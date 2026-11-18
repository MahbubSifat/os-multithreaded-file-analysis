#!/usr/bin/env python3
"""uneven.csv theke static vs dynamic er speedup graph."""
import csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

data = {}
with open("results/uneven.csv") as f:
    for r in csv.DictReader(f):
        data[(r["version"], int(r["threads"]))] = float(r["median_time"])

threads = [1, 2, 4, 8]
base = data[("seq", 1)]
static = [base / data[("static", t)] for t in threads]
dynamic = [base / data[("dynamic", t)] for t in threads]

print("threads  static  dynamic")
for t, s, d in zip(threads, static, dynamic):
    print(f"{t:>7}  {s:>5.2f}x  {d:>6.2f}x")

x = range(len(threads))
w = 0.35
fig, ax = plt.subplots(figsize=(8, 4.8))
ax.bar([i - w / 2 for i in x], static, w, label="Static (round-robin)")
ax.bar([i + w / 2 for i in x], dynamic, w, label="Dynamic work queue")
ax.axhline(1, color="black", linestyle="--", label="Sequential")
ax.set_xticks(list(x))
ax.set_xticklabels(threads)
ax.set_xlabel("Threads")
ax.set_ylabel("Speedup vs sequential")
ax.set_title("Uneven file sizes: static vs dynamic distribution")
ax.grid(True, axis="y", alpha=0.3)
ax.legend()
fig.tight_layout()
fig.savefig("results/uneven.png", dpi=150)
print("Saved: results/uneven.png")
