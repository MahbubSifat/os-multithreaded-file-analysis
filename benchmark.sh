#!/bin/bash
# Repeatable benchmark: results/timings.csv e median somoy lekhe
REPS=5
WORD=error
OUT=results/timings.csv
SIZES="small medium large"
THREADS="1 2 4 8"

echo "version,size,threads,median_time,total" > "$OUT"

# run_case <version> <size> <threads> <command...>
run_case() {
    local version=$1 size=$2 threads=$3
    shift 3
    local times="" total="" line t med

    "$@" > /dev/null                      # warm-up (gona hobe na)

    for ((r = 1; r <= REPS; r++)); do
        line=$("$@")
        t=$(echo "$line" | sed -E 's/.*time=([0-9.]+).*/\1/')
        total=$(echo "$line" | sed -E 's/.*total=([0-9]+).*/\1/')
        times="$times$t"$'\n'
        if [ "$total" != "$EXPECTED" ]; then
            echo "MISMATCH: $version $size t=$threads got=$total expected=$EXPECTED" >&2
        fi
    done

    med=$(printf "%s" "$times" | sort -n | awk '{a[NR]=$1} END{print a[int((NR+1)/2)]}')
    echo "$version,$size,$threads,$med,$total" | tee -a "$OUT"
}

for size in $SIZES; do
    # grep = sotti uttor (independent tool)
    EXPECTED=$(grep -ow "$WORD" data/$size/*.txt | wc -l)
    echo "== $size : expected total = $EXPECTED =="

    run_case seq "$size" 1 ./seq_analyzer data/$size $WORD
    for t in $THREADS; do
        run_case threadlocal "$size" "$t" ./mt_analyzer data/$size $WORD $t
        run_case mutex       "$size" "$t" ./mt_mutex    data/$size $WORD $t
    done
done

echo "Done. Results saved in $OUT"
