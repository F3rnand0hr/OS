#!/bin/bash
set -euo pipefail

SRC_MAIN="../src/main.c"
SRC_FUN="../src/functions.c"
INC_DIR="../include"

BUILD_DIR="../build"
BIN="$BUILD_DIR/keyword_counter"
TIMEOUT_SEC=10

TEXT1="../build/text1.txt"
TEXT2="../build/text2.txt"

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

# Uses LD_PRELOAD delay so threads live long enough to be observed
observe_workers() {
  local file="$1"
  ( LD_PRELOAD="$DELAY_SO" "$BIN" "$file" >/dev/null 2>/dev/null ) &
  local pid=$!
  local max_threads_seen=0

  for ((i=0; i<1200; i++)); do
    kill -0 "$pid" 2>/dev/null || break
    nlwp=$(thread_count "$pid"); [[ "$nlwp" =~ ^[0-9]+$ ]] || nlwp=0
    (( nlwp > max_threads_seen )) && max_threads_seen="$nlwp"
    sleep 0.002
  done

  wait "$pid" 2>/dev/null || true
  local workers=$((max_threads_seen > 0 ? max_threads_seen - 1 : 0))
  echo "$workers"
}

extract_count_for_paragraph() {
  local pid="$1"
  grep -E "^PARAGRAPH[[:space:]]+${pid}[[:space:]]+\|" "$OUT_FILE" \
    | head -n 1 \
    | sed -E 's/.*COUNT:[[:space:]]*([0-9]+).*/\1/'
}

extract_keyword_for_paragraph() {
  local pid="$1"
  grep -E "^PARAGRAPH[[:space:]]+${pid}[[:space:]]+\|" "$OUT_FILE" \
    | head -n 1 \
    | sed -E 's/.*KEYWORD:[[:space:]]*([^|]+)\|.*/\1/' \
    | sed -E 's/[[:space:]]+$//'
}

check_run_correctness() {
  local file="$1"; shift
  local expected_paragraphs="$1"; shift

  # Ensure preload does NOT affect correctness runs
  if ! LD_PRELOAD= run_with_timeout_capture "$TIMEOUT_SEC" "$BIN" "$file"; then
    echo -e "${RED}Runtime FAIL/timeout for $file${NC}"
    return 1
  fi

  local ok=1
  for ((i=0; i<expected_paragraphs; i++)); do
    local exp_kw="$1"; shift
    local exp_ct="$1"; shift

    got_kw="$(extract_keyword_for_paragraph "$i")"
    got_ct="$(extract_count_for_paragraph "$i")"

    if [ -z "${got_kw:-}" ] || [ -z "${got_ct:-}" ]; then
      ok=0
      continue
    fi

    if [ "$got_kw" != "$exp_kw" ] || [ "$got_ct" != "$exp_ct" ]; then
      ok=0
    fi
  done

  [ "$ok" -eq 1 ]
}

# ---------------- Test 1: Build (5) ----------------
mkdir -p "$BUILD_DIR"
if gcc -O2 -Wall -Wextra -std=c11 -pthread -I"$INC_DIR" "$SRC_MAIN" "$SRC_FUN" -o "$BIN"; then
  SCORE=$((SCORE+5))
  echo -e "Test1: Build OK ${GREEN}(+5)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test1: Build FAIL ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
  exit 1
fi

# ---- Build preload to slow down worker start (test-side sleep) ----
DELAY_SRC="../test/pthread_start_delay.c"
DELAY_SO="$BUILD_DIR/pthread_start_delay.so"
gcc -shared -fPIC -O2 -Wall -Wextra "$DELAY_SRC" -o "$DELAY_SO" -ldl -pthread

# ---------------- Test 2: pthread_create (5) ----------------
if bin_has_symbol "pthread_create"; then
  SCORE=$((SCORE+5))
  echo -e "Test2: pthread_create found ${GREEN}(+5)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test2: pthread_create missing ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

# ---------------- Test 3: pthread_join (10) ----------------
if bin_has_symbol "pthread_join"; then
  SCORE=$((SCORE+10))
  echo -e "Test3: pthread_join found ${GREEN}(+10)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test3: pthread_join missing ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

# ---------------- Test 4: Threads match paragraphs for TEXT1 (20) ----------------
workers1="$(observe_workers "$TEXT1")"
if [ "$workers1" -eq 3 ]; then
  SCORE=$((SCORE+20))
  echo -e "Test4: text1 workers=3 observed ${GREEN}(+20)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test4: text1 expected 3 workers, got $workers1 ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

# ---------------- Test 5: Threads match paragraphs for TEXT2 (20) ----------------
workers2="$(observe_workers "$TEXT2")"
if [ "$workers2" -eq 4 ]; then
  SCORE=$((SCORE+20))
  echo -e "Test5: text2 workers=4 observed ${GREEN}(+20)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test5: text2 expected 4 workers, got $workers2 ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

# ---------------- Test 6: Correctness for TEXT1 (20) ----------------
# Expected: P0=8, P1=6, P2=10
if check_run_correctness "$TEXT1" 3  thread 8  mutex 6  join 10; then
  SCORE=$((SCORE+20))
  echo -e "Test6: text1 counts correct ${GREEN}(+20)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test6: text1 counts incorrect ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

# ---------------- Test 7: Correctness for TEXT2 (20) ----------------
# Expected: P0=7, P1=5, P2=11, P3=4
if check_run_correctness "$TEXT2" 4  lock 7  race 5  thread 11  join 4; then
  SCORE=$((SCORE+20))
  echo -e "Test7: text2 counts correct ${GREEN}(+20)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test7: text2 counts incorrect ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

echo "TOTAL_SCORE: $SCORE / $TOTAL"