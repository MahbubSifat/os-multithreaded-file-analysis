#!/bin/bash
# Test suite: normal, boundary ar failure case
cd "$(dirname "$0")/.." || exit 1      # project er root e jao

PASS=0
FAIL=0
FIX=tests/fixtures

check() {      # check <test er nam> <expected> <actual>
    if [ "$2" == "$3" ]; then
        echo "PASS: $1"; PASS=$((PASS+1))
    else
        echo "FAIL: $1 (expected=$2 got=$3)"; FAIL=$((FAIL+1))
    fi
}

expect_fail() {   # expect_fail <test er nam> <command...> (command fail howa dorkar)
    local name=$1; shift
    if "$@" > /dev/null 2>&1; then
        echo "FAIL: $name (should have failed)"; FAIL=$((FAIL+1))
    else
        echo "PASS: $name"; PASS=$((PASS+1))
    fi
}

# program er output theke total=... ongsho ta kete ney
total_of() { sed -E 's/.*total=([0-9]+).*/\1/'; }

# ek folder e shob version chalay
run_all() {   # run_all <folder> <expected> <label>
    local dir=$1 exp=$2 label=$3 t
    check "$label [sequential]" "$exp" "$(./seq_analyzer "$dir" error | total_of)"
    for t in 1 3 8; do
        check "$label [thread-local t=$t]" "$exp" "$(./mt_analyzer "$dir" error $t | total_of)"
        check "$label [mutex t=$t]"        "$exp" "$(./mt_mutex "$dir" error $t | total_of)"
        check "$label [dynamic t=$t]"      "$exp" "$(./mt_dynamic "$dir" error $t | total_of)"
    done
}

# ---------- test er file banai ----------
rm -rf "$FIX"
mkdir -p "$FIX"/{basic,empty,single,nonl,nofiles,unreadable}

# basic: a.txt te 3 ta error, b.txt te 4 ta error => mot 7
# ("errors" ar "Error" gona hobe na, karon whole word, case-sensitive)
printf 'error info error\nwarning error\nerrors Error\n' > "$FIX/basic/a.txt"
printf 'error\nerror error error\n' > "$FIX/basic/b.txt"
: > "$FIX/empty/empty.txt"                          # 0 byte er file
printf 'error\n' > "$FIX/single/one.txt"            # matro 1 ta file
printf 'error error' > "$FIX/nonl/x.txt"            # shesh e newline nai
printf 'error\n' > "$FIX/nofiles/notes.log"         # .txt na
printf 'error\n' > "$FIX/unreadable/ok.txt"
printf 'error\n' > "$FIX/unreadable/bad.txt"
chmod 000 "$FIX/unreadable/bad.txt"                 # pora jabe na

echo "=== NORMAL CASES ==="
run_all "$FIX/basic" 7 "basic (hand-counted)"
for d in small uneven; do
    if [ -d "data/$d" ]; then
        run_all "data/$d" "$(grep -ow error data/$d/*.txt | wc -l)" "data/$d (vs grep)"
    fi
done

echo "=== BOUNDARY CASES ==="
run_all "$FIX/empty"  0 "empty file"
run_all "$FIX/single" 1 "one file, threads > files"
run_all "$FIX/nonl"   2 "no trailing newline"

echo "=== FAILURE CASES ==="
expect_fail "no arguments"                  ./seq_analyzer
expect_fail "missing directory"             ./seq_analyzer /no/such/dir error
expect_fail "directory with no .txt files"  ./mt_analyzer "$FIX/nofiles" error 2
expect_fail "zero threads"                  ./mt_analyzer "$FIX/basic" error 0
expect_fail "threads not a number"          ./mt_dynamic "$FIX/basic" error abc
expect_fail "too many threads"              ./mt_mutex "$FIX/basic" error 1000

# pora jay na emon file: baki file gulo thikmoto gona hobe, ar error message ashbe
out=$(./mt_analyzer "$FIX/unreadable" error 2 2> "$FIX/err.log")
check "unreadable file: other files still counted" 1 "$(echo "$out" | total_of)"
check "unreadable file: error message printed" yes \
      "$(grep -q 'Permission denied' "$FIX/err.log" && echo yes || echo no)"
chmod 644 "$FIX/unreadable/bad.txt"

echo "=== REPEATABILITY ==="
EXP=$(grep -ow error data/medium/*.txt | wc -l)
bad=0
for i in $(seq 1 20); do
    r=$(./mt_analyzer data/medium error 8 | total_of)
    [ "$r" != "$EXP" ] && bad=$((bad+1))
done
check "20 runs x 8 threads, every result correct" 0 "$bad"

echo "=== RACE DEMO (information only, not a pass/fail test) ==="
echo "correct=$EXP   racy=$(./mt_racy data/medium error 8 | total_of)"

echo "-------------------------------------"
echo "PASSED: $PASS   FAILED: $FAIL"
[ "$FAIL" -eq 0 ]
