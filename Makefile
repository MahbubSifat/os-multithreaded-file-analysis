CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu11 -pthread -Isrc
.RECIPEPREFIX = >

BINS = seq_analyzer mt_analyzer mt_racy mt_mutex mt_dynamic

all: $(BINS)

%: src/%.c src/common.c src/common.h
> $(CC) $(CFLAGS) $< src/common.c -o $@

data:
> python3 gen_data.py
> python3 gen_uneven.py

test: all
> ./tests/run_tests.sh

clean:
> rm -f $(BINS)

.PHONY: all data test clean
