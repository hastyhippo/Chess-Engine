"""Neural network training script for the engine's evaluation.

Streams positions from training_data.binpack using the nnue-pytorch C++
data loader (fast binpack decoding + shuffling + filtering), and will
write trained weights to the weights/ storage directory.

Run with the venv python:  .venv/bin/python train.py
"""

import math  # noqa: F401
import os
import sys
import time

import numpy as np  # noqa: F401
import torch  # noqa: F401

ROOT = os.path.dirname(os.path.abspath(__file__))
NNUE_DIR = os.path.join(ROOT, "nnue-pytorch")
DATA_FILE = os.path.join(ROOT, "training_data.binpack")
WEIGHTS_DIR = os.path.join(ROOT, "weights")

sys.path.insert(0, NNUE_DIR)
# The data_loader package locates ./build/libtraining_data_loader.dylib
# relative to the current working directory.
os.chdir(NNUE_DIR)

from data_loader import DataloaderSkipConfig, SparseBatchProvider  # noqa: E402

FEATURE_SET = "HalfKAv2_hm"
BATCH_SIZE = 16384
NUM_WORKERS = 4


def main():
    os.makedirs(WEIGHTS_DIR, exist_ok=True)

    provider = SparseBatchProvider(
        FEATURE_SET,
        [DATA_FILE],
        BATCH_SIZE,
        cyclic=True,
        num_workers=NUM_WORKERS,
        config=DataloaderSkipConfig(),
    )

    # Each batch is a tuple of torch tensors:
    #   us            (batch, 1)  1.0 if white to move else 0.0
    #   them          (batch, 1)  1.0 - us
    #   white_indices (batch, max_active)  active HalfKAv2_hm feature indices
    #   black_indices (batch, max_active)  (padded with -1)
    #   outcome       (batch, 1)  game result from side to move: 1 / 0.5 / 0
    #   score         (batch, 1)  engine eval in centipawns from side to move
    #   piece_count   (batch,)
    #
    # TODO: define the network and train on these batches, then serialize
    # weights to a file under WEIGHTS_DIR for the engine to load.
    n_batches = 20
    start = time.time()
    for i, batch in zip(range(n_batches), provider):
        us, them, white_indices, black_indices, outcome, score, piece_count = batch
        if i == 0:
            print(f"batch size: {us.shape[0]}, max active features: {white_indices.shape[1]}")
        print(
            f"batch {i + 1:>3}: mean score {score.mean():>8.2f} cp | "
            f"mean outcome {outcome.mean():.3f} | white-to-move {us.mean():.3f}"
        )
    elapsed = time.time() - start
    positions = n_batches * BATCH_SIZE
    print(f"\nstreamed {positions:,} positions in {elapsed:.2f}s "
          f"({positions / elapsed:,.0f} positions/sec)")


if __name__ == "__main__":
    main()
