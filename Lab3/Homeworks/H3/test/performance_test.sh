#!/bin/bash
set -euo pipefail

SRC_MAIN="../src/main.c"
SRC_FUN="../src/functions.c"
INC_DIR="../include"

BUILD_DIR="../build"
BIN="$BUILD_DIR/elevators"
TIMEOUT_SEC=15

NUM_WORKERS=4
NUM_MANAGER=1
NUM_THREADS_EXPECTED=$((NUM_WORKERS + NUM_MANAGER))   # 5 threads creados
NUM_ELEVATORS=2

NUM_TASKS=12
FLOORS=8

SCORE=0
TOTAL=100

OUT_FILE="perf_stdout.txt"
ERR_FILE="perf_stderr.txt"

GREEN='\033[0;32m'; RED='\033[0;31m'; YEL='\033[0;33m'; NC='\033[0m'

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

# --- Output patterns (your program) ---
EXEC_RE='^WORKER [0-9]+ EXECUTING TASK [0-9]+ \| PICK [0-9]+ -> DROP [0-9]+ \| ELEVATOR [0-9]+$'
FIN_RE='^WORKER [0-9]+ FINISHED TASK [0-9]+ \| PICKED [0-9]+ \| DROPPED [0-9]+ \| ELEVATOR [0-9]+$'

executing_lines() { grep -E "$EXEC_RE" "$OUT_FILE" || true; }
finished_lines()  { grep -E "$FIN_RE"  "$OUT_FILE" || true; }

# Fields (FINISHED line):
# 1 WORKER
# 2 worker_id
# 3 FINISHED
# 4 TASK
# 5 task_id
# 6 |
# 7 PICKED
# 8 pick
# 9 |
# 10 DROPPED
# 11 drop
# 12 |
# 13 ELEVATOR
# 14 elev
worker_ids_from_finished() { finished_lines | awk '{print $2}'; }
task_ids_from_finished()   { finished_lines | awk '{print $5}'; }
pick_from_finished()       { finished_lines | awk '{print $8}'; }
drop_from_finished()       { finished_lines | awk '{print $11}'; }
elev_from_finished()       { finished_lines | awk '{print $14}'; }

# ---------------- Test 1: Build (5) ----------------
mkdir -p "$BUILD_DIR"
if gcc -O2 -Wall -Wextra -std=c11 -pthread -I"$INC_DIR" "$SRC_MAIN" "$SRC_FUN" -o "$BIN"; then
  SCORE=$((SCORE+5))
  echo -e "Test1: Build OK ${GREEN}(+5)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test1: Build FAIL ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
  exit 1
fi

# Run once to generate output
if ! run_with_timeout_capture "$TIMEOUT_SEC" "$BIN"; then
  echo -e "${RED}Runtime FAIL/timeout${NC}"
  echo "TOTAL_SCORE: $SCORE / $TOTAL"
  exit 1
fi

# ---------------- Test 2: Observe exactly 5 created threads (50) ----------------
( "$BIN" >/dev/null 2>/dev/null ) &
pid=$!
max_threads_seen=0

# sample ~2.5s total, fast sampling
for ((i=0; i<250; i++)); do
  kill -0 "$pid" 2>/dev/null || break
  nlwp=$(thread_count "$pid"); [[ "$nlwp" =~ ^[0-9]+$ ]] || nlwp=0
  (( nlwp > max_threads_seen )) && max_threads_seen="$nlwp"
  sleep 0.01
done

# clean stop (in case still running)
kill -TERM "$pid" 2>/dev/null || true
sleep 0.1
kill -KILL "$pid" 2>/dev/null || true
wait "$pid" 2>/dev/null || true

created_threads=$((max_threads_seen > 0 ? max_threads_seen - 1 : 0))

if [ "$created_threads" -eq 5 ]; then
  SCORE=$((SCORE+50))
  echo -e "Test2: observed 5 created threads (4 workers + 1 manager) ${GREEN}(+50)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test2: expected 5 created threads, got $created_threads ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

# ---------------- Test 3: Condition variables usage (15) ----------------
# Require: pthread_cond_wait AND (pthread_cond_signal OR pthread_cond_broadcast)
if bin_has_symbol "pthread_cond_wait" && ( bin_has_symbol "pthread_cond_signal" || bin_has_symbol "pthread_cond_broadcast" ); then
  SCORE=$((SCORE+15))
  echo -e "Test3: condition variables detected ${GREEN}(+15)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test3: condition variables missing/incomplete ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

# ---------------- Test 4: Mutex usage (10) ----------------
# Require: lock/unlock and trylock (because elevators)
if bin_has_symbol "pthread_mutex_lock" && bin_has_symbol "pthread_mutex_unlock" && bin_has_symbol "pthread_mutex_trylock"; then
  SCORE=$((SCORE+10))
  echo -e "Test4: mutex lock/unlock/trylock detected ${GREEN}(+10)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test4: mutex usage missing/incomplete ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
fi

# ---------------- Test 5: Each task done exactly once by one worker (20) ----------------
ok=1

# Must have exactly NUM_TASKS finished lines
fin_count=$(finished_lines | wc -l | tr -d ' ')
if [ "$fin_count" -ne "$NUM_TASKS" ] 2>/dev/null; then
  ok=0
fi

# Duplicates in task ids not allowed
dup_tasks=$(task_ids_from_finished | sort -n | uniq -d | wc -l | tr -d ' ')
if [ "$dup_tasks" -ne 0 ] 2>/dev/null; then
  ok=0
fi

# Must cover tasks 0..NUM_TASKS-1
missing_tasks=$(comm -23 <(seq 0 $((NUM_TASKS-1))) <(task_ids_from_finished | sort -n | uniq) | wc -l | tr -d ' ')
if [ "$missing_tasks" -ne 0 ] 2>/dev/null; then
  ok=0
fi

# Validate worker ids 0..3 and numeric
bad_worker=$(worker_ids_from_finished | awk -v n="$NUM_WORKERS" '($1 !~ /^[0-9]+$/) || ($1<0) || ($1>=n) {print "bad"; exit 0}')
if [ -n "${bad_worker:-}" ]; then
  ok=0
fi

# Validate elevator ids 0..1 and numeric
bad_elev=$(elev_from_finished | awk -v n="$NUM_ELEVATORS" '($1 !~ /^[0-9]+$/) || ($1<0) || ($1>=n) {print "bad"; exit 0}')
if [ -n "${bad_elev:-}" ]; then
  ok=0
fi

# Validate floors 0..7 and numeric (pick/drop)
bad_pick=$(pick_from_finished | awk -v f="$FLOORS" '($1 !~ /^[0-9]+$/) || ($1<0) || ($1>=f) {print "bad"; exit 0}')
bad_drop=$(drop_from_finished | awk -v f="$FLOORS" '($1 !~ /^[0-9]+$/) || ($1<0) || ($1>=f) {print "bad"; exit 0}')
if [ -n "${bad_pick:-}" ] || [ -n "${bad_drop:-}" ]; then
  ok=0
fi

# Optional: ensure there are also NUM_TASKS executing lines (not required, but helps)
exec_count=$(executing_lines | wc -l | tr -d ' ')
if [ "$exec_count" -ne "$NUM_TASKS" ] 2>/dev/null; then
  ok=0
fi

if [ "$ok" -eq 1 ]; then
  SCORE=$((SCORE+20))
  echo -e "Test5: tasks executed exactly once (EXECUTING+FINISHED) ${GREEN}(+20)${NC} | SCORE=$SCORE/$TOTAL"
else
  echo -e "Test5: task uniqueness/coverage failed ${RED}(+0)${NC} | SCORE=$SCORE/$TOTAL"
  echo -e "${YEL}Hint: need exactly $NUM_TASKS EXECUTING lines and $NUM_TASKS FINISHED lines, tasks 0..$((NUM_TASKS-1)) once each, workers 0..3, elevators 0..1.${NC}"
fi

# ---------------- Final score ----------------
echo "TOTAL_SCORE: $SCORE / $TOTAL"