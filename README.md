# Multithreaded File Analysis and Performance Evaluation

CSE 203 Operating System and Unix Programming - Assignment (Topic 3)

Counts how many times a word appears across a directory of text files,
using POSIX threads, and measures how the design affects performance.

## Versions

| Binary | Description |
|---|---|
| `seq_analyzer` | Sequential baseline (one thread) |
| `mt_analyzer` | Multithreaded, thread-local counts, static round-robin file split (correct, fast) |
| `mt_racy` | INTENTIONALLY WRONG: all threads increment one shared counter with no lock (race condition demo) |
| `mt_mutex` | Race fixed with a mutex around every increment (correct, but slow) |
| `mt_dynamic` | Thread-local counts plus a dynamic work queue (best for files of very different sizes) |

## Environment used for all measurements

- CPU: Intel Core i5-8365U, 4 physical cores / 8 logical threads
- OS: Ubuntu Linux (kernel 7.0.0-31-generic, x86_64)
- Compiler: gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- Build flags: `-Wall -Wextra -O2 -std=gnu11 -pthread`

## Dependencies

- gcc and make (`sudo apt install build-essential`)
- python3 (dataset generation)
- python3-matplotlib (graphs only): `sudo apt install python3-matplotlib`
- git (optional)

## Build

    make          # builds all five programs
    make clean    # removes the binaries

## Generate the datasets

The datasets are not stored in the repository (about 1.4 GB). They are
generated deterministically (fixed random seed), so everyone gets the same files.

    python3 gen_data.py      # data/small, data/medium, data/large, data/skewed
    python3 gen_uneven.py    # data/uneven (every 8th file is huge, others tiny)

| Dataset | Files | Size |
|---|---|---|
| small | 20 | about 6.5 MB |
| medium | 40 | about 129 MB |
| large | 80 | about 1.1 GB |
| uneven | 64 | about 160 MB (8 huge files, 56 tiny ones) |

## Usage

    ./seq_analyzer <directory> <word>
    ./mt_analyzer  <directory> <word> <threads>
    ./mt_racy      <directory> <word> <threads>
    ./mt_mutex     <directory> <word> <threads>
    ./mt_dynamic   <directory> <word> <threads> [-v]

Example:

    ./mt_dynamic data/medium error 4 -v

`-v` prints how many files each thread processed and how long it was busy.
A word matches only when it is a whole, case-sensitive,
whitespace-separated token ("errors" and "Error" do not match "error").

## Tests

    make test
    # or: ./tests/run_tests.sh 2>&1 | tee results/test_output.txt

The suite covers:
- normal cases: a hand-counted dataset and comparison against `grep -ow`
- boundary cases: empty file, one file with more threads than files, no trailing newline
- failure cases: missing directory, no `.txt` files, zero / non-numeric / too many threads,
  an unreadable file (other files are still counted and an error is printed)
- repeatability: 20 runs with 8 threads must all give the correct total

## Experiments

    ./benchmark.sh           # 3 sizes x 1,2,4,8 threads, median of 5 runs -> results/timings.csv
    python3 plot.py          # results/speedup.png, results/time.png, results/speedup.csv
    ./benchmark_uneven.sh    # static vs dynamic on uneven files -> results/uneven.csv
    python3 plot_uneven.py   # results/uneven.png

Method: one warm-up run (not counted), then 5 timed runs, median reported.
Every run's total is compared with `grep -ow`; a mismatch is printed as `MISMATCH`.
Run on mains power with other applications closed.

## Summary of findings (this machine)

- Thread-local counting gave about 2.9x speedup on `large` with 8 threads.
  Gains flatten after 4 threads because there are only 4 physical cores.
- The racy version lost roughly 30% of the counts and gave a different wrong
  total on every run. With 1 thread it was correct.
- The mutex version is correct but only about 1.1x to 1.3x faster, because
  every match takes the same lock (about 1.7 million lock operations on `medium`).
- On `uneven`, static splitting gave about 1.0x for every thread count
  (one thread received all huge files). The dynamic queue gave about 3.0x with 8 threads.

Full numbers are in `results/*.csv`; analysis is in the technical report.

## Limitations

- Counts one word per run; no regular expressions or multi-word patterns.
- A single huge file is processed by one thread only (work is divided per file,
  not per byte range), so one dominant file limits the speedup.
- Results are specific to this hardware. Timings vary run to run
  (CPU frequency scaling), hence the median of 5.
- `mt_racy` is deliberately incorrect and exists only for demonstration.

## Possible improvement

Split large files into byte ranges aligned to line boundaries, so that even
a single huge file can be shared by several threads.
