"""Export a trained checkpoint to the flat binary the C++ engine reads.

Usage:  .venv/bin/python serialize.py <checkpoint.pt> [out.bin]

Layout is final on disk: the engine fread()s each block straight into its arrays
with no transformation. ft_weight is feature-major so one accumulator update
reads 512 contiguous bytes; l1/l2/out are output-major so one output neuron
reads a contiguous row.
"""

import os
import struct
import sys

import numpy as np
import torch

import train as T

MAGIC = b"NNUE"
VERSION = 1
INT8_MAX, INT16_MAX, INT32_MAX = 127, 32767, 2**31 - 1


def q(tensor, scale, dtype, limit, name):
    """Scale, round and bounds-check one weight block."""
    v = torch.round(tensor.detach().float() * scale)
    lo, hi = v.min().item(), v.max().item()
    if lo < -limit or hi > limit:
        raise SystemExit(f"{name}: {lo:.0f}..{hi:.0f} exceeds +-{limit} at scale {scale:g}")
    return v.numpy().astype(dtype)


def accumulator_bound(ftw, ftb):
    """Worst-case accumulator over any legal set of active features."""
    pos = np.sort(np.maximum(ftw, 0), axis=0)[-T.MAX_ACTIVE_FEATURES:].sum(0)
    neg = np.sort(np.minimum(ftw, 0), axis=0)[:T.MAX_ACTIVE_FEATURES].sum(0)
    return int((ftb + neg).min()), int((ftb + pos).max())


def serialize(ckpt_path, out_path):
    ck = torch.load(ckpt_path, map_location="cpu")
    fmt = ck.get("format", 1)
    sd = ck["model"]
    if fmt == 1:
        sd = T.migrate_v1_state_dict(sd)
    elif fmt == 2:
        sd = T.migrate_v2_state_dict(sd)

    model = T.NNUE()
    model.load_state_dict(sd)
    model.clip_weights()

    ftw = q(model.coalesced_ft_weights(), T.FT_SCALE, np.int16, INT16_MAX, "ft_weight")
    ftb = q(model.input.b, T.FT_SCALE, np.int16, INT16_MAX, "ft_bias")

    lo, hi = accumulator_bound(ftw.astype(np.int64), ftb.astype(np.int64))
    if min(lo, -hi) < -INT16_MAX or max(hi, -lo) > INT16_MAX:
        raise SystemExit(f"accumulator can reach [{lo}, {hi}], outside int16")

    blocks = [("ft_weight", ftw), ("ft_bias", ftb)]
    for name, layer, scale in (("l1", model.l1, T.WEIGHT_SCALE_L1),
                               ("l2", model.l2, T.WEIGHT_SCALE_L2),
                               ("out", model.out, T.WEIGHT_SCALE_OUT)):
        blocks.append((f"{name}_weight",
                       q(layer.weight, scale, np.int8, INT8_MAX, f"{name}_weight")))
        blocks.append((f"{name}_bias",
                       q(layer.bias, scale * T.QUANTIZED_ONE, np.int32, INT32_MAX,
                         f"{name}_bias")))

    header = struct.pack(
        "<4s11I", MAGIC, VERSION,
        T.REAL_INPUT_SIZE, T.L0_SIZE, T.L1_SIZE, T.L2_SIZE, T.L3_SIZE,
        int(T.FT_SCALE), int(T.WEIGHT_SCALE_L1), int(T.WEIGHT_SCALE_L2),
        int(T.WEIGHT_SCALE_OUT), int(T.NNUE2SCORE))
    assert len(header) == 48, len(header)

    with open(out_path, "wb") as f:
        f.write(header)
        for _, arr in blocks:
            f.write(np.ascontiguousarray(arr).tobytes())

    print(f"{os.path.basename(ckpt_path)} (format {fmt}) -> {out_path}")
    print(f"  accumulator worst case [{lo}, {hi}] of +-{INT16_MAX}")
    off = len(header)
    print(f"  {'block':<11}{'dtype':>7}{'shape':>14}{'bytes':>12}{'offset':>11}")
    for name, arr in blocks:
        print(f"  {name:<11}{arr.dtype.name:>7}{str(arr.shape):>14}{arr.nbytes:>12,}{off:>11,}")
        off += arr.nbytes
    print(f"  total {off:,} bytes ({off / 2**20:.2f} MiB)")
    return out_path


if __name__ == "__main__":
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    ckpt = sys.argv[1]
    if not os.path.isabs(ckpt) and not os.path.exists(ckpt):
        ckpt = os.path.join(T.WEIGHTS_DIR, ckpt)   # train.py chdir's on import
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(T.ROOT, os.pardir, "nnue.bin")
    serialize(ckpt, out)
