#!/usr/bin/env python3
"""Generate repeatable text datasets for the file-analysis assignment."""
import os, random, sys

SEED = 42
WORDS = ["error", "warning", "info", "debug", "login", "logout",
         "timeout", "connect", "disk", "memory", "thread", "process"]

def make_file(path, n_lines, rng):
    with open(path, "w") as f:
        for _ in range(n_lines):
            line = " ".join(rng.choice(WORDS) for _ in range(rng.randint(6, 14)))
            f.write(line + "\n")

def make_set(name, n_files, lines_each):
    rng = random.Random(SEED)          # fixed seed = repeatable
    d = os.path.join("data", name)
    os.makedirs(d, exist_ok=True)
    for i in range(n_files):
        make_file(os.path.join(d, f"file_{i:03d}.txt"), lines_each, rng)

def make_skewed(name):
    """Few huge files + many tiny files (for the load-balancing challenge)."""
    rng = random.Random(SEED)
    d = os.path.join("data", name)
    os.makedirs(d, exist_ok=True)
    for i in range(4):
        make_file(os.path.join(d, f"big_{i}.txt"), 400000, rng)
    for i in range(60):
        make_file(os.path.join(d, f"tiny_{i:02d}.txt"), 500, rng)

if __name__ == "__main__":
    make_set("small", 20, 5000)       # ~20 files x 5k lines
    make_set("medium", 40, 50000)     # ~40 files x 50k lines
    make_set("large", 80, 200000)     # ~80 files x 200k lines
    make_skewed("skewed")
    print("Datasets created in data/")
