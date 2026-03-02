#!/bin/bash

SCORE=0
TOTAL=100

echo "--- EVALUATING HOMEWORK 2: Space Scanner ---"
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

# 1. Compilation command_center and drone
cd ../drone
rm -rf build && mkdir build && cd build
cmake .. && make
echo -e "${YELLOW} Test 1: drone Compilation (+5)"
if [ $? -eq 0 ]; then
    echo -e "${GREEN} drone Compilation: PASS${NC} "
    SCORE=$((SCORE + 5))
else
    echo "1. Compilation: FAIL (+0)"
    echo "FINAL GRADE: 0 / 100"
    exit 1
fi
# 2. Compilation command_center and drone
cd ../../command_center
rm -rf build && mkdir build && cd build
cmake .. && make
echo -e "${YELLOW} Test 2: command_center Compilation (+5)${NC} "
if [ $? -eq 0 ]; then
    echo -e "${GREEN} command_center Compilation: PASS${NC} "
    SCORE=$((SCORE + 5))
else
    echo "2. Compilation: FAIL (+0)"
    echo "FINAL GRADE: 0 / 100"
    exit 1
fi


SCANNER_EXE="./center"
"./$SCANNER_EXE"  > output.log 2>&1 &
SHELL_PID=$!

SHM_CREATED=0
SHM_NAME="space_terrain_shm"
FOUND=1
for i in {1..5}; do
    CHILDREN=$(pgrep -P $SHELL_PID)
    if [ -n "$CHILDREN" ]; then
        FOUND=$((FOUND + 1))
    fi
    if ls /dev/shm | grep -q "$SHM_NAME"; then
        SHM_CREATED=1
    fi
    sleep 0.5
done

#Test 3. Validate child process::::
echo -e "${YELLOW}Test 2: Validating execution and child processes (+30)...${NC}"
if [ $FOUND -eq 3 ]; then
    echo -e "${GREEN}   - Process creation: PASS${NC}"
    SCORE=$((SCORE + 30))
else
    echo -e "${RED}   - Process creation: FAIL (No children detected)${NC}"
fi

# Test: Validate shared memory creation
echo -e "${YELLOW}Test 3: Validating shared memory creation (+30)...${NC}"
if [ $SHM_CREATED -eq 1 ]; then
    echo -e "${GREEN}   - Shared memory detected in /dev/shm: PASS${NC}"
    SCORE=$((SCORE + 30))
else
    echo -e "${RED}   - Shared memory NOT found: FAIL (+0)${NC}"
    echo "     (Tip: Asegúrate de usar shm_open con el nombre correcto)"
fi

#Test 4: Validate Scanner:
echo -e "${YELLOW}Test 4: Validating Scanner complete (+20)...${NC}"
UNIQUE_PIDS=$(tail -10 output.log | awk -F'[][]' '{print $2}' | sort -u | wc -l)
if [ $UNIQUE_PIDS -eq 3 ]; then
    echo -e "${GREEN} 3 Drones complete mission PASS (+20)${NC}"
    SCORE=$((SCORE + 20))
else
    echo "Incomplete Drones $UNIQUE_PIDS"
fi

#Test 5: Validate signals:
MAIN_SRC="../src/main.c"
echo -e "${YELLOW}Test 3: Checking signals usage command_center (+20)...${NC}"
if grep -qE "signal\s*\(" "$MAIN_SRC"; then
    echo -e "${GREEN}   - 'signal' usage found: PASS (+20)${NC}"
    SCORE=$((SCORE + 10))
else
    echo -e "${RED}   - 'signal' usage NOT found in source${NC}"
fi



# Cleanup: kill compilator if still running
if ps -p $COMP_PID > /dev/null 2>&1; then
    kill $COMP_PID 2>/dev/null || true
    wait $COMP_PID 2>/dev/null || true
fi

echo "---------------------------"
echo "FINAL GRADE: $SCORE / $TOTAL"
echo "---------------------------"

echo "---------------------------"
echo "FINAL GRADE: $SCORE / 100"
echo "---------------------------"
