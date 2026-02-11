#!/bin/bash

SCORE=0
TOTAL=100

echo "--- EVALUATING EXERCISE 5: INVENTORY SYSTEM ---"
# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

# 1. Compilation (3 pts)
rm -r manager output.log
cd ..
rm -rf build && mkdir build && cd build
cmake .. && make
if [ $? -eq 0 ]; then
    echo "1. Compilation: PASS"
else
    echo "1. Compilation: FAIL (+0)"
    echo "FINAL GRADE: 0 / 100"
    echo "TOTAL_SCORE: $SCORE"
    exit 1
fi

#############################

MAIN_SRC="../src/main.c"
COMPILATOR_EXE="./compilator"
if [ ! -x "$COMPILATOR_EXE" ]; then
    if [ -f "$MAIN_SRC" ]; then
        echo "Compiling main program from $MAIN_SRC"
        gcc -Wall -Wextra -o "$COMPILATOR_EXE" "$MAIN_SRC"
        if [ $? -ne 0 ]; then
            echo "   - Main compilation: FAIL"
            echo "FINAL GRADE: $SCORE / $TOTAL"
            exit 1
        fi
    else
        echo "   - Main source $MAIN_SRC missing: cannot run runtime checks"
        echo "FINAL GRADE: $SCORE / $TOTAL"
        exit 1
    fi
fi

"./$COMPILATOR_EXE" > output.log &
COMP_PID=$!
echo "Started compilator (pid=$COMP_PID), checking for child 'manager' processes..."
FOUND=0
for i in {1..10}; do
    # Check processes whose PPID is COMP_PID and whose command contains 'manager'
    MATCHES=$(ps -eo pid,ppid,cmd | awk -v p=$COMP_PID '$2==p && $3 ~ /manager/ {print $0}')
    if [ -n "$MATCHES" ]; then
        FOUND=1
        echo "   - Found manager child process(es):"
        echo "$MATCHES"
        break
    fi
    sleep 1
done

# Test 1: Validate manager program
echo -e "${YELLOW} Test 1: Validate manager program${NC}"
MANAGER_SRC="../build/manager"
if [ -x "$MANAGER_SRC" ]; then
    echo -e "${GREEN}   - Manager executable found: PASS (+30)${NC}"
    SCORE=$((SCORE + 30))
else
    echo "   - Manager executable NOT found: FAIL (+0)"
fi

# Test 2: Validate two child process
echo -e "${YELLOW} Test 2: Validate two child process${NC}"
if [ $FOUND -eq 1 ]; then
    echo -e "${GREEN}   - Process creation: PASS (+30)${NC}"
    SCORE=$((SCORE + 30))
else
    echo "   - Process creation: FAIL (+0)"
fi

#Test3: Check that the code uses fork and execlp
echo -e "${YELLOW} Test3: Check that the code uses fork${NC}"
if grep -qE "fork\s*\(" "$MAIN_SRC"; then
    echo -e "${GREEN}   - 'fork' usage found in source${NC}"
    SCORE=$((SCORE + 20))
else
    echo -e "${RED}   - 'fork' usage NOT found in $MAIN_SRC${NC}"
fi

#Test4: Check that the code uses execlp
echo -e "${YELLOW} Test4: Check that the code uses execlp${NC}"
if grep -qE "execlp\s*\(" "$MAIN_SRC"; then
    echo -e "${GREEN}   - 'execlp' usage found in source${NC}"
    echo "   - execlp check: PASS (+10)"
    SCORE=$((SCORE + 20))
else
    echo -e "${RED}   - 'execlp' usage NOT found in $MAIN_SRC${NC}"
fi

# Cleanup: kill compilator if still running
if ps -p $COMP_PID > /dev/null 2>&1; then
    kill $COMP_PID 2>/dev/null || true
    wait $COMP_PID 2>/dev/null || true
fi

echo "---------------------------"
echo "FINAL GRADE: $SCORE / $TOTAL"
echo -e "${GREEN}TOTAL_SCORE: $SCORE${NC}"
echo "---------------------------"