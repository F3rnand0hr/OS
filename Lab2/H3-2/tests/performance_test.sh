#!/bin/bash

SCORE=0
TOTAL=100

echo "--- EVALUATING HOMEWORK 3: Data Processing ---"
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

# 1. Compilation worker and generator
cd ../generator
rm -rf build && mkdir build && cd build
cmake .. && make
echo -e "${YELLOW} Test 1: generator Compilation (+5)"
if [ $? -eq 0 ]; then
    echo -e "${GREEN} generator Compilation: PASS${NC} "
    SCORE=$((SCORE + 5))
else
    echo "1. Compilation: FAIL (+0)"
    echo "FINAL GRADE: 0 / 100"
    exit 1
fi
GENERATOR_EXE="./generator"
"./$GENERATOR_EXE" > output_v.log >&1 &
SHELL_PID_G=$!

# 2. Compilation worker and generator
cd ../../worker
rm -rf build && mkdir build && cd build

cmake .. && make
echo -e "${YELLOW} Test 2: worker Compilation (+5)${NC} "
if [ $? -eq 0 ]; then
    echo -e "${GREEN} worker Compilation: PASS${NC} "
    SCORE=$((SCORE + 5))
else
    echo "2. Compilation: FAIL (+0)"
    echo "FINAL GRADE: 0 / 100"
    exit 1
fi



worker_EXE="./worker"
"./$worker_EXE"  > output_v.log 2>&1 &
SHELL_PID_V=$!

SHM_PID_CREATED=0
SHM_DATA_CREATED=0
SHM_DATA="shm_data"
SHM_PID="shm_pid"
FOUND=1
for i in {1..5}; do
    if ls /dev/shm| grep -q "$SHM_PID"; then
        SHM_PID_CREATED=1
    fi
    if ls /dev/shm| grep -q "$SHM_PID"; then
        SHM_DATA_CREATED=1
    fi
done

echo -e "${YELLOW} Waitin the proces finish...${NC}"
wait

# Test 3: Validate shm creation
echo -e "${YELLOW}Test 3: Validating shared memmory for pid creation (+30)...${NC}"
if [ $SHM_PID_CREATED -eq 1 ]; then
    echo -e "${GREEN}   - shared memmory: PASS${NC}"
    SCORE=$((SCORE + 20))
else
    echo -e "${RED}   - Message queue NOT found in /dev/mqueue: FAIL (+0)${NC}"
    echo "     (Tip: Verify mq_open(\"/test_queue\", ...) is used)"
fi

# Test 4: Validate shm creation
echo -e "${YELLOW}Test 4: Validating shared memmory for data creation (+30)...${NC}"
if [ $SHM_DATA_CREATED -eq 1 ]; then
    echo -e "${GREEN}   - shared memmory created: PASS${NC}"
    SCORE=$((SCORE + 20))
else
    echo -e "${RED}   - Message queue NOT found in /dev/mqueue: FAIL (+0)${NC}"
    echo "     (Tip: Verify mq_open(\"/test_queue\", ...) is used)"
fi

# --- Test 5: Validación de Lógica de Procesamiento (Ruido vs Imagen) ---
echo -e "${YELLOW}Test 5: Validating AI Processing Logic (+50)...${NC}"

WORKER_LOG=$(grep -A 50 "Señal recibida" ../../worker/build/output_v.log)
RUIDO_FAIL=0
IMAGEN_FAIL=0

for i in {1..5}; do
    DATA_LINE=$(echo "$WORKER_LOG" | grep -A 1 "\[WORKER\] IMAGE $i" | tail -n 1)
    TIPO=$(grep "IMAGE $i ->" ../../generator/build/output_v.log | awk '{print $NF}')

    if [ "$TIPO" == "RUIDO" ]; then
        NON_ZERO=$(echo "$DATA_LINE" | grep -oE "[1-9][0-9]*")
        if [ -n "$NON_ZERO" ]; then
            echo -e "${RED}   - Image $i (RUIDO) contains non-zero values: FAIL${NC}"
            RUIDO_FAIL=$((RUIDO_FAIL + 1))
        fi
    elif [ "$TIPO" == "IMAGEN" ]; then
        HAS_ZERO=$(echo "$DATA_LINE" | grep -w "0")
        if [ -n "$HAS_ZERO" ]; then
            echo -e "${RED}   - Image $i (IMAGEN) contains zero values: FAIL${NC}"
            IMAGEN_FAIL=$((IMAGEN_FAIL + 1))
        fi
    fi
done

if [ $RUIDO_FAIL -eq 0 ] && [ $IMAGEN_FAIL -eq 0 ]; then
    echo -e "${GREEN}   - AI Logic (Zero for Noise ): PASS (+25)${NC}"
    echo -e "${GREEN}   - AI Logic (Non-Zero for Image): PASS (+25)${NC}"
    SCORE=$((SCORE + 50))
else
    echo -e "${RED}   - AI Logic: FAIL (Noise errors: $RUIDO_FAIL, Image errors: $IMAGEN_FAIL)${NC}"
fi



# Cleanup: kill compilator if still running
if ps -p $COMP_PID > /dev/null 2>&1; then
    kill $COMP_PID 2>/dev/null || true
    wait $COMP_PID 2>/dev/null || true
fi

echo "---------------------------"
echo "FINAL GRADE: $SCORE / $TOTAL"
echo -e "TOTAL_SCORE: $SCORE"
echo "---------------------------"