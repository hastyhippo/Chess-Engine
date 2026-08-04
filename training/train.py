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

L1_SIZE = 512
L2_SIZE = 32
L3_SIZE = 32

SAVE_EVERY = 10000

CP_CONV = 400
EVAL_WEIGHT = 0.9
EWMA_BETA = 0.98

RUN_DESC = "Virtual Input Space, 8000 batch sizes"

DEVICE = "mps" if torch.backends.mps.is_available() else "cpu"

class InputLayer(nn.Module):
    def __init__(self):
        super().__init__()
        self.W = nn.Parameter((torch.randn(TOTAL_INPUT_SIZE, 256) * (np.sqrt(1.0/60.0))))
        self.b = nn.Parameter((torch.zeros(size = (256,))))

    def forward(self, white_indices, black_indices, white_to_move):
        w_mask = (white_indices >= 0)
        b_mask = (black_indices >= 0)

        w_res = self.W[white_indices.clamp(min = 0)] # clamps = 0 to process -1 vals safely
        w_res = w_res * w_mask.unsqueeze(-1) # multiplies the rows by a mask with dim (B, 30, 1) which zeros -1 rows
        w_ret = w_res.sum(dim = 1) + self.b 

        b_res = self.W[black_indices.clamp(min = 0)]
        b_res = b_res * b_mask.unsqueeze(-1)
        b_ret = b_res.sum(dim = 1) + self.b

        us = w_ret * (white_to_move) + b_ret * (1 - white_to_move)
        them = w_ret * (1 - white_to_move) + b_ret * (white_to_move)
        out = torch.cat((us, them), dim = 1)
        return out

class NNUE(nn.Module):
    def __init__(self):
        super().__init__()
        self.input = InputLayer()
        self.tower = nn.Sequential(
            nn.Hardtanh(0.0, 1.0),
            nn.Linear(L1_SIZE, L2_SIZE), nn.Hardtanh(0.0, 1.0),
            nn.Linear(L2_SIZE, L3_SIZE), nn.Hardtanh(0.0, 1.0),
            nn.Linear(L3_SIZE, 1)
        )
        self.it = 0
    def forward(self, white_indices, black_indices, white_to_move):
        self.it = self.it + 1
        return self.tower(self.input(white_indices, black_indices, white_to_move))


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
        # Assumes orient() is a vertical flip (sq ^ 56) for black.
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
            black_set.add((sq ^ 56, ptype, "white" if theirs else "black"))
            bksq = ksq ^ 56

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

def append_virtual_indices(indices):
    v_indices = REAL_INPUT_SIZE + indices % (NUM_SQUARES * PIECE_TYPE_CNT * 2)
    empty_mask = (indices == -1).to(torch.int32)
    v_indices = v_indices * (1 - empty_mask) + (-1) * (empty_mask)
    return torch.cat((indices, v_indices), dim = 1) 

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
    torch.save({"model": model.state_dict(), "optim": optimiser.state_dict(), "step": model.it}, weights_path)
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

        white_indices = append_virtual_indices(white_indices)
        black_indices = append_virtual_indices(black_indices)

        y_pred = model(white_indices, black_indices, us)

        # Use sigmoid to convert centipawn 
        p_pred = torch.sigmoid(y_pred / CP_CONV)
        p_target = torch.sigmoid(score / CP_CONV) * EVAL_WEIGHT + (1 - EVAL_WEIGHT) * outcome
        loss = loss_fn(p_pred, p_target)

        # recalculate gradients and perform gradient descent
        optimiser.zero_grad()
        loss.backward()
        optimiser.step()

        losses[it] = loss.item()
        ewma = loss.item() if ewma is None else EWMA_BETA * ewma + (1 - EWMA_BETA) * loss.item()
        ewma_losses[it] = ewma

        if it % 100 == 0:
            print(f"Loss: {loss.item():.6f} | EWMA: {ewma:.6f}")

        if model.it % SAVE_EVERY == 0:
            path = os.path.join(WEIGHTS_DIR, f"nnue_step{model.it:07d}.pt")
            torch.save({"model": model.state_dict(), "optim": optimiser.state_dict(), "step": it}, path)

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
    opt = torch.optim.Adam(model.parameters(), lr = 1e-3)

    while(True):
        ch = input("Command (l: load | t: train | s: save | q: quit): ")
        if ch == 'l':
            file_name = input("File name: ")
            try:
                path = os.path.join(WEIGHTS_DIR, file_name)
                checkpoint = torch.load(path)
                model.load_state_dict(checkpoint['model'])
                opt.load_state_dict(checkpoint['optim'])
                print(f"Loading: {file_name} succesful! \n\n")
            except Exception as e:
                print("Error loading file\n\n")
        elif ch == 't':
            cnt = int(input("How many iterations: "))
            execute_training_loop(model, provider, opt, cnt)
        elif ch == 's':
            file_name = input("File name: ") +  ".pt"
            try:
                path = os.path.join(WEIGHTS_DIR, file_name)
                torch.save({"model": model.state_dict(),
                            "optim": opt.state_dict(),
                            "step": model.it}, path)
            except Exception as e:
                print("Error saving file\n\n")
        elif  ch == 'q':
            return 0


            

if __name__ == "__main__":
    main()
