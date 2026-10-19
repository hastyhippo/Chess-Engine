"""Neural network training script for the engine's evaluation.

Streams positions from training_data.binpack using the nnue-pytorch C++
data loader (fast binpack decoding + shuffling + filtering), and will
write trained weights to the weights/ storage directory.

Run with the venv python:  .venv/bin/python train.py
"""

import json
import math
import os
import sys
import time

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import torch
import torch.nn as nn

ROOT = os.path.dirname(os.path.abspath(__file__))
NNUE_DIR = os.path.join(ROOT, "nnue-pytorch")
DATA_FILE = os.path.join(ROOT, "training_data.binpack")
WEIGHTS_DIR = os.path.join(ROOT, "weights")
TRAINING_RUNS_DIR = os.path.join(ROOT, "training_runs")

sys.path.insert(0, NNUE_DIR)
# The data_loader package locates ./build/libtraining_data_loader.dylib
# relative to the current working directory.
os.chdir(NNUE_DIR)

from data_loader import DataloaderSkipConfig, SparseBatchProvider

FEATURE_SET = "HalfKP" 
BATCH_SIZE = 8000
NUM_WORKERS = 4
NUM_SQUARES = 64
PIECE_TYPE_CNT = 5


REAL_INPUT_SIZE = NUM_SQUARES * NUM_SQUARES * PIECE_TYPE_CNT * 2
VIRTUAL_INPUT_SIZE = NUM_SQUARES * PIECE_TYPE_CNT * 2
TOTAL_INPUT_SIZE = VIRTUAL_INPUT_SIZE + REAL_INPUT_SIZE
MAX_ACTIVE_FEATURES = 30  # HalfKP, kings excluded: at most 30 pieces per perspective

L0_SIZE = 256           # accumulator width, per perspective
L1_SIZE = L0_SIZE * 2   # both perspectives concatenated
L2_SIZE = 32
L3_SIZE = 32

SAVE_EVERY = 10000

CP_CONV = 400
EVAL_WEIGHT = 0.9
EWMA_BETA = 0.99

# Quantization scheme the C++ engine will use
QUANTIZED_ONE = 127.0        # clipped-relu output [0,1] maps to int8 [0,127]
WEIGHT_SCALE_L1 = 128.0      # l1 weights -> int8, accumulate int32, then >>7
WEIGHT_SCALE_L2 = 64.0       # l2 weights -> int8, then >>6
WEIGHT_SCALE_OUT = 64.0      # output layer weight scale
FT_SCALE = QUANTIZED_ONE     # feature transformer weights/bias -> int16
NNUE2SCORE = 128.0           # network units -> centipawns
INT16_MAX = 32767
FAKE_QUANT_EPS = 1e-3        # absorbs float32 matmul noise so floor() matches `>>`

# int8 holds [-127, 127], so a weight at scale s can be at most 127/s.
MAX_L1_WEIGHT = QUANTIZED_ONE / WEIGHT_SCALE_L1
MAX_L2_WEIGHT = QUANTIZED_ONE / WEIGHT_SCALE_L2
MAX_OUT_WEIGHT = QUANTIZED_ONE / WEIGHT_SCALE_OUT

CHECKPOINT_FORMAT = 3  # 1 = Sequential tower, cp output; 2 = NNUE2SCORE 64
V2_NNUE2SCORE = 64.0

RUN_DESC = "Virtual Input Space, 8000 batch sizes"

DEVICE = "mps" if torch.backends.mps.is_available() else "cpu"

def fq_round(t, scale):
    """Round onto the quantization grid, straight-through gradient."""
    return (t.mul(scale).round().div(scale) - t).detach() + t


def fq_floor(t, scale):
    """Truncate onto the grid, as `>>` does. Straight-through gradient."""
    return (t.mul(scale).add(FAKE_QUANT_EPS).floor().div(scale) - t).detach() + t


class InputLayer(nn.Module):
    def __init__(self):
        super().__init__()
        self.W = nn.Parameter((torch.randn(TOTAL_INPUT_SIZE, L0_SIZE) * (np.sqrt(1.0/60.0))))
        self.b = nn.Parameter((torch.zeros(size = (L0_SIZE,))))

    def coalesced(self):
        """The 40960 rows the engine stores: real + its virtual (factorizer) row."""
        return self.W[:REAL_INPUT_SIZE] + self.W[REAL_INPUT_SIZE:].repeat(NUM_SQUARES, 1)

    def accumulate(self, W, bias, indices):
        """Sum the rows for one perspective's active features."""
        mask = (indices >= 0).unsqueeze(-1)
        return (W[indices.clamp(min=0)] * mask).sum(dim=1) + bias  # -1 pads, masked off

    def forward(self, white_indices, black_indices, white_to_move, fake_quant=False):
        # Quantize the table once, not once per active row: same numbers, ~12x
        # fewer elements through fq_round.
        W, bias = self.coalesced(), self.b
        if fake_quant:
            W = fq_round(W, FT_SCALE)
            bias = fq_round(bias, FT_SCALE)
        w_ret = self.accumulate(W, bias, white_indices)
        b_ret = self.accumulate(W, bias, black_indices)

        us = w_ret * (white_to_move) + b_ret * (1 - white_to_move)
        them = w_ret * (1 - white_to_move) + b_ret * (white_to_move)
        out = torch.cat((us, them), dim = 1)
        return out

class NNUE(nn.Module):
    def __init__(self, fake_quant=True):
        super().__init__()
        self.input = InputLayer()
        # Named, not a Sequential, so the clipper and exporter can address each
        # layer. Order matches the old tower so optimiser state still lines up.
        self.crelu = nn.Hardtanh(0.0, 1.0)
        self.l1 = nn.Linear(L1_SIZE, L2_SIZE)
        self.l2 = nn.Linear(L2_SIZE, L3_SIZE)
        self.out = nn.Linear(L3_SIZE, 1)
        self.it = 0
        self.fake_quant = fake_quant

    def _dense(self, layer, x, scale, fake_quant, activate=True):
        w, b = layer.weight, layer.bias
        if fake_quant:
            w = fq_round(w, scale)
            b = fq_round(b, scale * QUANTIZED_ONE)  # bias lives in the int32 acc
        y = nn.functional.linear(x, w, b)
        if activate:
            if fake_quant:
                y = fq_floor(y, QUANTIZED_ONE)  # the >> back to activation scale
            y = self.crelu(y)
        return y

    def forward(self, white_indices, black_indices, white_to_move, fake_quant=None):
        self.it = self.it + 1
        fq = self.fake_quant if fake_quant is None else fake_quant

        acc = self.input(white_indices, black_indices, white_to_move, fq)
        # Quantized FT weights land the accumulator exactly on the 1/127 grid,
        # so this clipped relu is a pure clamp with nothing to round.
        x = self.crelu(acc)
        x = self._dense(self.l1, x, WEIGHT_SCALE_L1, fq)
        x = self._dense(self.l2, x, WEIGHT_SCALE_L2, fq)
        x = self._dense(self.out, x, WEIGHT_SCALE_OUT, fq, activate=False)
        return x * NNUE2SCORE

    @torch.no_grad()
    def clip_weights(self):
        """Keep every weight inside what its quantized dtype can represent.

        Biases are exempt: they land in int32 accumulators with room to spare.
        The feature transformer is exempt too (int16 at scale 127 spans +-258);
        quantization_report checks it for overflow instead of clamping it.
        """
        self.l1.weight.clamp_(-MAX_L1_WEIGHT, MAX_L1_WEIGHT)
        self.l2.weight.clamp_(-MAX_L2_WEIGHT, MAX_L2_WEIGHT)
        self.out.weight.clamp_(-MAX_OUT_WEIGHT, MAX_OUT_WEIGHT)

    @torch.no_grad()
    def coalesced_ft_weights(self):
        """What the exporter writes: real row i plus virtual row i % 640."""
        return self.input.coalesced()


_V1_KEY_MAP = {
    "tower.1.weight": "l1.weight", "tower.1.bias": "l1.bias",
    "tower.3.weight": "l2.weight", "tower.3.bias": "l2.bias",
    "tower.5.weight": "out.weight", "tower.5.bias": "out.bias",
}


def migrate_v1_state_dict(sd):
    """Rename the old Sequential tower keys and rescale the output layer.

    Format 1 nets emitted centipawns straight out of the last Linear; format 2
    emits network units that forward() multiplies by NNUE2SCORE. Same function,
    but the weights are now small enough to quantize.
    """
    out = {}
    for k, v in sd.items():
        new_k = _V1_KEY_MAP.get(k, k)
        if new_k in ("out.weight", "out.bias"):
            v = v / NNUE2SCORE
        out[new_k] = v
    return out


def migrate_v2_state_dict(sd):
    """Format 2 stored the output layer at NNUE2SCORE 64; rescale to the current one."""
    out = dict(sd)
    for k in ("out.weight", "out.bias"):
        out[k] = sd[k] * (V2_NNUE2SCORE / NNUE2SCORE)
    return out


def save_checkpoint(model, optimiser, path):
    torch.save({"format": CHECKPOINT_FORMAT,
                "model": model.state_dict(),
                "optim": optimiser.state_dict(),
                "step": model.it}, path)


def load_checkpoint(model, optimiser, path):
    """Load a checkpoint, migrating format 1 nets on the way in."""
    checkpoint = torch.load(path, map_location=DEVICE)
    fmt = checkpoint.get("format", 1)

    sd = checkpoint["model"]
    if fmt == 1:
        sd = migrate_v1_state_dict(sd)
    elif fmt == 2:
        sd = migrate_v2_state_dict(sd)
    model.load_state_dict(sd)
    optimiser.load_state_dict(checkpoint["optim"])
    model.it = checkpoint.get("step", 0)

    if fmt < CHECKPOINT_FORMAT:
        # Only the output layer moved, so only its Adam moments are stale.
        for p in (model.out.weight, model.out.bias):
            st = optimiser.state.get(p)
            if st:
                st["exp_avg"].zero_()
                st["exp_avg_sq"].zero_()
        model.clip_weights()
        print(f"migrated a format {fmt} checkpoint: output layer rescaled to "
              f"NNUE2SCORE {NNUE2SCORE:.0f}, weights clipped to the quantized ranges.\n"
              "fine-tune at a lower LR (try 1e-4) before exporting.")


@torch.no_grad()
def pinned_pct(model):
    """Percent of each int8 layer sitting on its clipping bound."""
    return tuple((l.weight.abs() >= b * 0.999).float().mean().item() * 100
                 for l, b in ((model.l1, MAX_L1_WEIGHT),
                              (model.l2, MAX_L2_WEIGHT),
                              (model.out, MAX_OUT_WEIGHT)))


@torch.no_grad()
def quantization_report(model):
    """Check whether the current weights survive the engine's integer format.

    Run this before exporting. Anything flagged here is strength the C++ net
    would silently lose relative to what the loss curve says it has.
    """
    ft = model.coalesced_ft_weights()
    ft_q = ft * FT_SCALE
    bias_q = model.input.b * FT_SCALE

    # Worst case accumulator: bias plus the MAX_ACTIVE_FEATURES most extreme
    # weights in that neuron's column. No real position need reach it, but if
    # this fits in int16 then no position can overflow.
    pos = ft_q.clamp(min=0).topk(MAX_ACTIVE_FEATURES, dim=0).values.sum(dim=0)
    neg = ft_q.clamp(max=0).topk(MAX_ACTIVE_FEATURES, dim=0, largest=False).values.sum(dim=0)
    acc_hi = (bias_q + pos).max().item()
    acc_lo = (bias_q + neg).min().item()

    print("\n--- quantization report ---")
    print(f"ft weight     max |w*{FT_SCALE:.0f}| = {ft_q.abs().max().item():8.1f}  (int16 limit {INT16_MAX})")
    print(f"accumulator   worst case [{acc_lo:.0f}, {acc_hi:.0f}]  (int16 limit +-{INT16_MAX})")
    if max(abs(acc_hi), abs(acc_lo)) > INT16_MAX:
        print("  OVERFLOW: the int16 accumulator cannot hold this net.")

    for name, layer, bound, scale in (
        ("l1 ", model.l1, MAX_L1_WEIGHT, WEIGHT_SCALE_L1),
        ("l2 ", model.l2, MAX_L2_WEIGHT, WEIGHT_SCALE_L2),
        ("out", model.out, MAX_OUT_WEIGHT, WEIGHT_SCALE_OUT),
    ):
        w = layer.weight
        at_bound = (w.abs() >= bound * 0.999).float().mean().item() * 100
        err = (w - w.mul(scale).round().div(scale)).abs().max().item()
        print(f"{name}  max |w| = {w.abs().max().item():.4f} / {bound:.4f}"
              f" | {at_bound:5.2f}% pinned at the bound"
              f" | max rounding error {err:.5f}")
        if at_bound > 5.0:
            print(f"  {at_bound:.1f}% of {name.strip()} is clipped: the layer wants more range "
                  "than int8 has. Widen the layer or accept the loss.")
    print("---------------------------\n")


def visualize_position(white_indices, black_indices, us, outcome, score):
    def sq_name(sq):
        return chr(ord('a') + sq % 8) + str(sq // 8 + 1)

    def decode(idx):
        ksq, rem = divmod(idx, 640)
        p_idx, sq = divmod(rem, 64)
        ptype, theirs = divmod(p_idx, 2)
        return ksq, sq, ptype, theirs

    print(f"white_indices {tuple(white_indices.shape)}:\n{white_indices}")
    print(f"black_indices {tuple(black_indices.shape)}:\n{black_indices}")

    for b in range(us.shape[0]):
        # Decode each POV into physical-board terms (white's frame).
        # HalfKP::orient() in halfkp-extractor.patch flips black both
        # vertically and horizontally, i.e. a 180 degree rotation: sq ^ 63.
        white_set, black_set = set(), set()
        wksq = bksq = None
        for idx in white_indices[b]:
            if idx.item() < 0:
                continue
            ksq, sq, ptype, theirs = decode(idx.item())
            white_set.add((sq, ptype, "black" if theirs else "white"))
            wksq = ksq
        for idx in black_indices[b]:
            if idx.item() < 0:
                continue
            ksq, sq, ptype, theirs = decode(idx.item())
            black_set.add((sq ^ 63, ptype, "white" if theirs else "black"))
            bksq = ksq ^ 63

        print(f"\nposition {b}: side to move: {'white' if us[b].item() else 'black'} | "
              f"score {score[b].item():+.0f} cp | outcome {outcome[b].item()}")
        # One grid, white's POV: uppercase = white, lowercase = black.
        grid = [["." for _ in range(8)] for _ in range(8)]
        for sq, ptype, colour in white_set | black_set:
            piece = "PNBRQ"[ptype]
            grid[sq // 8][sq % 8] = piece if colour == "white" else piece.lower()
        if wksq is not None:
            grid[wksq // 8][wksq % 8] = "K"
        if bksq is not None:
            grid[bksq // 8][bksq % 8] = "k"
        for rank in range(7, -1, -1):
            print("    " + str(rank + 1) + "  " + " ".join(grid[rank]))
        print("       a b c d e f g h")

def plot_run(run_id, losses, ewma_losses, path):
    """Plot raw loss (faint) with its EWMA (strong) and save as a PNG."""
    its = list(losses.keys())
    fig, ax = plt.subplots(figsize=(8, 4.5))
    ax.plot(its, [losses[i] for i in its], color="#9ca3af", linewidth=1, alpha=0.5)
    ax.plot(its, [ewma_losses[i] for i in its], color="#2563eb", linewidth=2)
    # direct labels instead of a legend box
    ax.annotate("raw", (its[-1], list(losses.values())[-1]), color="#6b7280", xytext=(6, 0),
                textcoords="offset points", va="center", fontsize=9)
    ax.annotate("EWMA", (its[-1], list(ewma_losses.values())[-1]), color="#2563eb", xytext=(6, 0),
                textcoords="offset points", va="center", fontsize=9)
    ax.set_xlabel("iteration")
    ax.set_ylabel("MSE loss (win-prob space)")
    ax.set_title(f"{run_id} — {RUN_DESC}", fontsize=10)
    ax.grid(True, color="#e5e7eb", linewidth=0.5)
    ax.spines[["top", "right"]].set_visible(False)
    ax.margins(x=0.02)
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)

def store_run(model, optimiser, run_id, losses, ewma_losses, total_batches, elapsed):
    """Store one run's record, loss plot + final weights in training_runs/."""
    positions = total_batches * BATCH_SIZE
    os.makedirs(TRAINING_RUNS_DIR, exist_ok=True)
    weights_path = os.path.join(TRAINING_RUNS_DIR, f"{run_id}_weights.pt")
    save_checkpoint(model, optimiser, weights_path)
    plot_run(run_id, losses, ewma_losses, os.path.join(TRAINING_RUNS_DIR, f"{run_id}_loss.png"))
    record = {
        "run_id": run_id,
        "description": RUN_DESC,
        "learning_rate": optimiser.param_groups[0]["lr"],
        "batch_size": BATCH_SIZE,
        "ewma_beta": EWMA_BETA,
        "total_batches": total_batches,
        "duration_seconds": round(elapsed, 2),
        "positions_per_second": round(positions / elapsed),
        "final_weights": weights_path,
        "losses": losses,
        "device": DEVICE,
        "losses_ewma": ewma_losses,
    }
    with open(os.path.join(TRAINING_RUNS_DIR, f"{run_id}.json"), "w") as f:
        json.dump(record, f, indent=2)

def execute_training_loop(model, provider, optimiser, total_batches):
    start = time.time()
    loss_fn = torch.nn.MSELoss()

    # One record per run, written to training_runs/ when the run finishes.
    run_id = time.strftime("run_%Y%m%d_%H%M%S")
    losses = {}
    ewma_losses = {}
    ewma = None

    for it, batch in zip(range(total_batches), provider):
        # provider has an __iter__/ __next__ dunder which provides samples when looped through
        
        us, them, white_indices, black_indices, outcome, score, piece_count = batch
        # visualize_position(white_indices, black_indices, us, outcome, score)

        y_pred = model(white_indices, black_indices, us)

        # Use sigmoid to convert centipawn 
        p_pred = torch.sigmoid(y_pred / CP_CONV)
        p_target = torch.sigmoid(score / CP_CONV) * EVAL_WEIGHT + (1 - EVAL_WEIGHT) * outcome
        loss = loss_fn(p_pred, p_target)

        # recalculate gradients and perform gradient descent
        optimiser.zero_grad()
        loss.backward()
        optimiser.step()
        model.clip_weights()  # quantization-aware: the net must stay exportable

        losses[it] = loss.item()
        ewma = loss.item() if ewma is None else EWMA_BETA * ewma + (1 - EWMA_BETA) * loss.item()
        ewma_losses[it] = ewma

        if it % 100 == 0:
            msg = f"Loss: {loss.item():.6f} | EWMA: {ewma:.6f}"
            if it % 1000 == 0:
                l1p, l2p, op = pinned_pct(model)
                msg += f" | pinned l1 {l1p:.1f}% l2 {l2p:.1f}% out {op:.1f}%"
            print(msg)

        # Trigger on the loop counter, not model.it: a resumed run starts at an
        # arbitrary step and may never hit a multiple of SAVE_EVERY.
        if (it + 1) % SAVE_EVERY == 0:
            path = os.path.join(WEIGHTS_DIR, f"nnue_step{model.it:07d}.pt")
            save_checkpoint(model, optimiser, path)

    if DEVICE == "mps":
        torch.mps.synchronize()  # GPU ops are async; wait before reading the clock
    elapsed = time.time() - start
    positions = total_batches * BATCH_SIZE

    store_run(model, optimiser, run_id, losses, ewma_losses, total_batches, elapsed)

    print(f"\nstreamed {positions:,} positions in {elapsed:.2f}s "
          f"({positions / elapsed:,.0f} positions/sec)")
    print(f"run record: training_runs/{run_id}.json")

    return


def main():
    os.makedirs(WEIGHTS_DIR, exist_ok=True)

    print(f"training on device: {DEVICE}")
    provider = SparseBatchProvider(
        FEATURE_SET, [DATA_FILE], BATCH_SIZE, cyclic=True,
        num_workers=NUM_WORKERS, config=DataloaderSkipConfig(),
        device=DEVICE,
    )

    model = NNUE().to(DEVICE)
    model.clip_weights()
    opt = torch.optim.Adam(model.parameters(), lr = 1e-3)

    while(True):
        ch = input("Command (l: load | t: train | s: save | r: lr | c: check | q: quit): ")
        if ch == 'l':
            file_name = input("File name: ")
            try:
                load_checkpoint(model, opt, os.path.join(WEIGHTS_DIR, file_name))
                print(f"Loading: {file_name} succesful! \n\n")
            except Exception as e:
                print(f"Error loading file: {e}\n\n")
        elif ch == 't':
            cnt = int(input("How many iterations: "))
            execute_training_loop(model, provider, opt, cnt)
        elif ch == 's':
            file_name = input("File name: ") +  ".pt"
            try:
                save_checkpoint(model, opt, os.path.join(WEIGHTS_DIR, file_name))
            except Exception as e:
                print(f"Error saving file: {e}\n\n")
        elif ch == 'r':
            lr = float(input(f"Learning rate (currently {opt.param_groups[0]['lr']:g}): "))
            for g in opt.param_groups:
                g["lr"] = lr
        elif ch == 'c':
            quantization_report(model)
        elif  ch == 'q':
            return 0


            

if __name__ == "__main__":
    main()
