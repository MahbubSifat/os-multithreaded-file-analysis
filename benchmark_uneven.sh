#!/bin/bash
# uneven dataset e: sequential vs static vs dynamic (median of 5)
REPS=5
WORD=error
OUT=results/uneven.csv
THREADS="1 2 4 8"

EXPECTED=$(grep -ow "$WORD" data/uneven/*.txt | wc -l)
echo "expected total = $EXPECTED"
echo "version,threads,median_time,total" > "$OUT"

# run_case <version> <threads> <command...>
run_case() {
    local version=$1 threads=$2
    shift 2
    local times="" total="" line t med r

    "$@" > /dev/null                      # warm-up

    for ((r = 1; r <= REPS; r++)); do
        line=$("$@")
        t=$(echo "$line" | sed -E 's/.*time=([0-9.]+).*/\1/')
        total=$(echo "$line" | sed -E 's/.*total=([0-9]+).*/\1/')
        times="$times$t"$'\n'
        if [ "$total" != "$EXPECTED" ]; then
            echo "MISMATCH: $version t=$threads got=$total expected=$EXPECTED" >&2
        fi
    done

    med=$(printf "%s" "$times" | sort -n | awk '{a[NR]=$1} END{print a[int((NR+1)/2)]}')
    echo "$version,$threads,$med,$total" | tee -a "$OUT"
}

run_case seq 1 ./seq_analyzer data/uneven $WORD
for t in $THREADS; do
    run_case static  "$t" ./mt_analyzer data/uneven $WORD $t
    run_case dynamic "$t" ./mt_dynamic  data/uneven $WORD $t
done

echo "Done. Saved in $OUT"
