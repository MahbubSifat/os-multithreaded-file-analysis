#!/usr/bin/env python3
"""timings.csv theke speedup table ar graph banay."""
import csv
import matplotlib
matplotlib.use("Agg")          # screen chhara file e chhobi save korbe
import matplotlib.pyplot as plt

data = {}
with open("results/timings.csv") as f:
    for r in csv.DictReader(f):
        data[(r["version"], r["size"], int(r["threads"]))] = float(r["median_time"])

sizes = ["small", "medium", "large"]
threads = [1, 2, 4, 8]
versions = [("threadlocal", "Thread-local"), ("mutex", "Mutex")]

# 1) speedup table (CSV)
with open("results/speedup.csv", "w") as f:
    f.write("version,size,threads,speedup\n")
    for s in sizes:
        base = data[("seq", s, 1)]
        for v, _ in versions:
            for t in threads:
                f.write(f"{v},{s},{t},{base / data[(v, s, t)]:.2f}\n")

# 2) speedup graph
fig, axes = plt.subplots(1, 3, figsize=(15, 4.5))
for ax, s in zip(axes, sizes):
    base = data[("seq", s, 1)]
    for v, label in versions:
        ax.plot(threads, [base / data[(v, s, t)] for t in threads],
                marker="o", label=label)
    ax.plot(threads, threads, "--", color="gray", label="Ideal")
    ax.axvline(4, color="red", linestyle=":", label="4 physical cores")
    ax.set_title(f"{s} dataset")
    ax.set_xlabel("Threads")
    ax.set_ylabel("Speedup vs sequential")
    ax.set_xticks(threads)
    ax.grid(True, alpha=0.3)
    ax.legend()
fig.suptitle("Speedup (median of 5 runs, i5-8365U 4C/8T)")
fig.tight_layout()
fig.savefig("results/speedup.png", dpi=150)

# 3) execution time graph
fig, axes = plt.subplots(1, 3, figsize=(15, 4.5))
for ax, s in zip(axes, sizes):
    for v, label in versions:
        ax.plot(threads, [data[(v, s, t)] for t in threads],
                marker="o", label=label)
    ax.axhline(data[("seq", s, 1)], color="black", linestyle="--",
               label="Sequential")
    ax.set_title(f"{s} dataset")
    ax.set_xlabel("Threads")
    ax.set_ylabel("Time (seconds)")
    ax.set_xticks(threads)
    ax.grid(True, alpha=0.3)
    ax.legend()
fig.suptitle("Execution time (median of 5 runs)")
fig.tight_layout()
fig.savefig("results/time.png", dpi=150)

print("Saved: results/speedup.csv, results/speedup.png, results/time.png")
