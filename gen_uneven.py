#!/usr/bin/env python3
"""Uneven dataset: proti 8 number file bishal, baki gulo chhoto (repeatable)."""
import os, random

SEED = 42
WORDS = ["error", "warning", "info", "debug", "login", "logout",
         "timeout", "connect", "disk", "memory", "thread", "process"]

rng = random.Random(SEED)
d = os.path.join("data", "uneven")
os.makedirs(d, exist_ok=True)

for i in range(64):
    n_lines = 300000 if i % 8 == 0 else 500
    path = os.path.join(d, f"file_{i:03d}.txt")
    with open(path, "w") as f:
        for _ in range(n_lines):
            f.write(" ".join(rng.choice(WORDS)
                             for _ in range(rng.randint(6, 14))) + "\n")

print("Created data/uneven")
