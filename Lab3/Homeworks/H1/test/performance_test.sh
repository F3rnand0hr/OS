#!/bin/bash
set -euo pipefail

SRC_MAIN="../src/main.c"
SRC_FUN="../src/functions.c"
INC_DIR="../include"

BUILD_DIR="../build"
BIN="$BUILD_DIR/matrix_threads"
TIMEOUT_SEC=10

DELAY_SRC="../test/pthread_start_delay.c"
DELAY_SO="$BUILD_DIR/pthread_start_delay.so"

EXPECTED_SUM=266000
EXPECTED_PARTIALS_SUM=266000

SCORE=0
TOTAL=100

OUT_FILE="perf_stdout.txt"
ERR_FILE="perf_stderr.txt"

GREEN='\033[0;32m'; RED='\033[0;31m'; NC='\033[0m'

run_with_timeout_capture() {
  local t="$1"; shift
  set +e
  ( "$@" >"$OUT_FILE" 2>"$ERR_FILE" ) &
  local pid=$!
  local elapsed=0
  while kill -0 "$pid" 2>/dev/null; do
    sleep 1
    elapsed=$((elapsed+1))
    if [ "$elapsed" -ge "$t" ]; then
      kill -TERM "$pid" 2>/dev/null || true
      sleep 1
      kill -KILL "$pid" 2>/dev/null || true
      wait "$pid" 2>/dev/null || true
      echo -e "${RED}Timeout (${t}s)${NC}"
      return 124
    fi
  done
  wait "$pid"
  local rc=$?
  set -e
  return "$rc"
}

bin_has_symbol() {
  local sym="$1"
  if command -v nm >/dev/null 2>&1; then
    { nm -u "$BIN"; nm -D "$BIN"; } 2>/dev/null | grep -Eiq "\b${sym}\b" && return 0
  fi
  if command -v readelf >/dev/null 2>&1; then
    readelf -Ws "$BIN" 2>/dev/null | grep -Eiq "\b${sym}\b" && return 0
  fi
  if command -v strings >/dev/null 2>&1; then
    strings "$BIN" 2>/dev/null | grep -Eiq "\b${sym}\b" && return 0
  fi
  return 1
}

thread_count() {
  local _pid="$1"
  case "$(uname -s)" in
    Linux)
      [ -d "/proc/${_pid}/task" ] && ls -1 "/proc/${_pid}/task" 2>/dev/null | wc -l | tr -d ' ' || echo 0
      ;;
    Darwin)
      ps -M "$_pid" 2>/dev/null | tail -n +2 | wc -l | tr -d ' '
      ;;
    *)
      echo 0
      ;;
  esac
}

extract_value_after_label() {
  local label="$1"
  grep -E "^[[:space:]]*${label}[[:space:]]*" "$OUT_FILE" \
    | head -n 1 \
    | sed -E "s/^[[:space:]]*${label}[[:space:]]*//" \
    | tr -d ',.'
}

echo "=== OS Exam Grading Script (Matrix Threads) ==="

# ---------------- Test 1: Build (GATE, 0 pts) ----------------
mkdir -p "$BUILD_DIR"
if gcc -O2 -Wall -Wextra -std=c11 -pthread -I"$INC_DIR" "$SRC_MAIN" "$SRC_FUN" -o "$BIN"; then
  echo -e "Test1: Build OK ${GREEN}(+0)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test1: Build FAIL ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
  exit 1
fi

# ---- Build preload to slow down worker start (test-side sleep) ----
gcc -shared -fPIC -O2 -Wall -Wextra "$DELAY_SRC" -o "$DELAY_SO" -ldl -pthread

# ---------------- Test 2: Runtime (5 pts) ----------------
# Ensure LD_PRELOAD does NOT affect the normal correctness run
if LD_PRELOAD= run_with_timeout_capture "$TIMEOUT_SEC" "$BIN"; then
  SCORE=$((SCORE+5))
  echo -e "Test2: Runtime OK (no timeout) ${GREEN}(+5)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test2: Runtime FAIL/timeout ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
  echo "TOTAL_SCORE: $SCORE / $TOTAL"
  exit 1
fi

# ---------------- Test 3: pthread_create (5) ----------------
if bin_has_symbol "pthread_create"; then
  SCORE=$((SCORE+5))
  echo -e "Test3: pthread_create found ${GREEN}(+5)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test3: pthread_create missing ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

# ---------------- Test 4: pthread_join (10) ----------------
if bin_has_symbol "pthread_join"; then
  SCORE=$((SCORE+10))
  echo -e "Test4: pthread_join found ${GREEN}(+10)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test4: pthread_join missing ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

# ---------------- Test 5: Exactly 4 workers observed (60 pts) ----------------
# Run under LD_PRELOAD so worker threads live long enough to be observed
( LD_PRELOAD="$DELAY_SO" "$BIN" >/dev/null 2>/dev/null ) &
pid=$!

max_threads_seen=0
for ((i=0; i<1200; i++)); do
  kill -0 "$pid" 2>/dev/null || break
  nlwp=$(thread_count "$pid"); [[ "$nlwp" =~ ^[0-9]+$ ]] || nlwp=0
  (( nlwp > max_threads_seen )) && max_threads_seen="$nlwp"
  sleep 0.002
done

kill -TERM "$pid" 2>/dev/null || true
sleep 0.1
kill -KILL "$pid" 2>/dev/null || true
wait "$pid" 2>/dev/null || true

workers=$((max_threads_seen > 0 ? max_threads_seen - 1 : 0))
if [ "$workers" -eq 4 ]; then
  SCORE=$((SCORE+60))
  echo -e "Test5: 4 worker threads observed (max_threads=$max_threads_seen) ${GREEN}(+60)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test5: expected 4 workers, got $workers (max_threads=$max_threads_seen) ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

# ---------------- Test 6: SUM + PARTIALS_SUM (20 pts) ----------------
sum_val="$(extract_value_after_label "SUM:")"
partials_val="$(extract_value_after_label "PARTIALS_SUM:")"

if [ -n "${sum_val:-}" ] && [ -n "${partials_val:-}" ] \
   && [ "$sum_val" = "$EXPECTED_SUM" ] \
   && [ "$partials_val" = "$EXPECTED_PARTIALS_SUM" ]; then
  SCORE=$((SCORE+20))
  echo -e "Test6: SUM/PARTIALS_SUM correct ${GREEN}(+20)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test6: SUM/PARTIALS_SUM incorrect ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
  echo "  Got SUM:          '${sum_val:-EMPTY}'"
  echo "  Got PARTIALS_SUM: '${partials_val:-EMPTY}'"
  echo "  (See $OUT_FILE and $ERR_FILE for details.)"
fi

echo "TOTAL_SCORE: $SCORE / $TOTAL"