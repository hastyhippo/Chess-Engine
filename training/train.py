"""Neural network training script for the engine's evaluation.

Placeholder for now — will train on game data and write weights to the
weights/ storage directory.
"""

import os

WEIGHTS_DIR = os.path.join(os.path.dirname(__file__), "weights")


def main():
    os.makedirs(WEIGHTS_DIR, exist_ok=True)
    # TODO: load training data, train the network, then serialize weights
    # to a file under WEIGHTS_DIR for the engine to load.
    print(f"Training not implemented yet. Weights will be written to {WEIGHTS_DIR}")


if __name__ == "__main__":
    main()
