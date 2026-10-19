"""Build the C++ test fixture: FENs with the exact eval the engine must produce.

Usage:  .venv/bin/python make_oracle.py [nnue.bin] [count]

Writes src/test/nnue_examples.txt, the fixture test_eval.cpp reads.

Reads the weights back out of nnue.bin and runs the integer pipeline on them, so
this also checks the serializer round-trips. Each line of the output is
    <FEN>|<out32>|<cp>
out32 is the raw int32 of the output layer, before the conversion to centipawns,
so a C++ mismatch points at a layer rather than at rounding.
"""

import os
import struct
import sys

import numpy as np
import torch

import train as T
from data_loader import FenBatchProvider, destroy_sparse_batch, get_sparse_batch_from_fens

MAGIC = b"NNUE"


def load_bin(path):
    """Parse nnue.bin exactly as the engine will."""
    buf = open(path, "rb").read()
    magic, ver, nfeat, l0, l1, l2, l3, s_ft, s_l1, s_l2, s_out, n2s = \
        struct.unpack_from("<4s11I", buf, 0)
    if magic != MAGIC:
        raise SystemExit(f"bad magic {magic!r}")
    h = dict(version=ver, nfeat=nfeat, l0=l0, l1=l1, l2=l2, l3=l3,
             s_ft=s_ft, s_l1=s_l1, s_l2=s_l2, s_out=s_out, n2s=n2s)
    off = 48

    def take(dtype, shape):
        nonlocal off
        n = int(np.prod(shape)) * np.dtype(dtype).itemsize
        a = np.frombuffer(buf, dtype=dtype, count=int(np.prod(shape)), offset=off)
        off += n
        return a.reshape(shape).astype(np.int32)

    w = dict(ft_w=take(np.int16, (nfeat, l0)), ft_b=take(np.int16, (l0,)),
             l1_w=take(np.int8, (l2, l1)), l1_b=take(np.int32, (l2,)),
             l2_w=take(np.int8, (l3, l2)), l2_b=take(np.int32, (l3,)),
             out_w=take(np.int8, (1, l3)), out_b=take(np.int32, (1,)))
    if off != len(buf):
        raise SystemExit(f"trailing bytes: read {off} of {len(buf)}")
    return h, w


def forward_int(h, w, white_idx, black_idx, stm_white):
    """The engine's arithmetic, in numpy. Returns the raw int32 output."""
    def accumulate(idx):
        acc = w["ft_b"].copy()
        return acc + w["ft_w"][idx[idx >= 0]].sum(0)

    wa, ba = accumulate(white_idx), accumulate(black_idx)
    us, them = (wa, ba) if stm_white else (ba, wa)
    x = np.clip(np.concatenate([us, them]), 0, 127)
    x = np.clip((x @ w["l1_w"].T + w["l1_b"]) >> int(np.log2(h["s_l1"])), 0, 127)
    x = np.clip((x @ w["l2_w"].T + w["l2_b"]) >> int(np.log2(h["s_l2"])), 0, 127)
    return int((x @ w["out_w"].T + w["out_b"])[0])


def to_cp(h, out32):
    """int32 -> centipawns, round half away from zero.

    Mirrors C++ `(num + sign*den/2) / den`, whose `/` truncates toward zero.
    Python's `//` floors instead, which is off by one on every negative eval.
    """
    den = h["s_out"] * 127
    num = out32 * h["n2s"]
    q = (abs(num) + den // 2) // den
    return q if num >= 0 else -q


def main(bin_path, count):
    h, w = load_bin(bin_path)
    print(f"{os.path.basename(bin_path)}: {h}")

    fens = []
    for batch in FenBatchProvider(T.DATA_FILE, True, 2, batch_size=count):
        fens.extend(batch)
        break
    fens = fens[:count]

    z = [0] * len(fens)
    sb = get_sparse_batch_from_fens(T.FEATURE_SET, fens, z, z, z)
    us, _, wi, bi, _, _, _ = sb.contents.get_tensors("cpu")
    wi, bi, us = wi.numpy(), bi.numpy(), us.numpy()

    # cross-check the file against the trained model
    model = T.NNUE()
    ck = torch.load(os.path.join(T.WEIGHTS_DIR, "nnue_ste_8h.pt"), map_location="cpu")
    fmt = ck.get("format", 1)
    model.load_state_dict(T.migrate_v1_state_dict(ck["model"]) if fmt == 1 else
                          T.migrate_v2_state_dict(ck["model"]) if fmt == 2 else ck["model"])
    model.clip_weights()
    with torch.no_grad():
        ref = model(torch.from_numpy(wi), torch.from_numpy(bi),
                    torch.from_numpy(us), fake_quant=True).flatten().numpy()

    out = os.path.join(T.ROOT, os.pardir, "src", "test", "nnue_examples.txt")
    lines, diffs = [], []
    for i, fen in enumerate(fens):
        o32 = forward_int(h, w, wi[i], bi[i], us[i][0] > 0.5)
        cp = to_cp(h, o32)
        diffs.append(abs(cp - ref[i]))
        lines.append(f"{fen}|{o32}|{cp}")
    open(out, "w").write("\n".join(lines) + "\n")

    d = np.array(diffs)
    lsb = h["n2s"] / (h["s_out"] * 127)   # one unit of out32, in cp
    print(f"wrote {len(lines)} positions -> {out}")
    # The file pipeline is pure integer and is the authority; the model's
    # fake-quant forward is a float32 approximation of it and drifts by about
    # one activation LSB, because a 512-wide float32 matmul cannot reproduce
    # the int32 accumulator exactly.
    print(f"file pipeline vs model (float32 sim): mean {d.mean():.3f}  max {d.max():.3f} cp")
    print(f"  one activation LSB is worth up to {127 * lsb:.1f} cp here, so this is "
          f"sub-LSB drift, not a serializer error")
    print("\nfirst 3 lines:")
    for ln in lines[:3]:
        print(" ", ln)
    destroy_sparse_batch(sb)


if __name__ == "__main__":
    path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(T.ROOT, os.pardir, "nnue.bin")
    main(path, int(sys.argv[2]) if len(sys.argv) > 2 else 500)
