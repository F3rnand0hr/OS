#!/bin/bash

SCORE=0
TOTAL=100

echo "--- EVALUATING L2: Prime Validator ---"
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

# 1. Compilation validator and generator
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

# 2. Compilation validator and generator
cd ../../validator
rm -rf build && mkdir build && cd build

cmake .. && make
echo -e "${YELLOW} Test 2: validator Compilation (+5)${NC} "
if [ $? -eq 0 ]; then
    echo -e "${GREEN} validator Compilation: PASS${NC} "
    SCORE=$((SCORE + 5))
else
    echo "2. Compilation: FAIL (+0)"
    echo "FINAL GRADE: 0 / 100"
    exit 1
fi



VALIDATOR_EXE="./validator"
"./$VALIDATOR_EXE"  > output_v.log 2>&1 &
SHELL_PID_V=$!

QUEUE_CREATED=0
SHM_NAME="space_terrain_shm"
FOUND=1
for i in {1..5}; do
    if ls /dev/mqueue| grep -q "$validator_queue"; then
        QUEUE_CREATED=1
    fi
done

echo -e "${YELLOW} Waitin the proces finish...${NC}"
wait

# Test 3: Validate message queue creation
echo -e "${YELLOW}Test 3: Validating message queue creation (+30)...${NC}"
if [ $QUEUE_CREATED -eq 1 ]; then
    echo -e "${GREEN}   - Message queue created: PASS${NC}"
    SCORE=$((SCORE + 30))
else
    echo -e "${RED}   - Message queue NOT found in /dev/mqueue: FAIL (+0)${NC}"
    echo "     (Tip: Verify mq_open(\"/test_queue\", ...) is used)"
fi

SUMMARY_FINAL=$(tail -n 7 output_v.log)
# Test 4: Validate 30 numbers generated and validated
echo -e "${YELLOW}Test 4: Validating total count (30 numbers) (+20)...${NC}"
TOTAL_DETECTED=$(echo "$SUMMARY_FINAL" | grep "Total processed:" | awk '{print $3}')
if [ "$TOTAL_DETECTED" -eq "30" ]; then
    echo -e "${GREEN}   - 30 numbers processed: PASS (+20)${NC}"
    SCORE=$((SCORE + 20))
else
    echo -e "${RED}   - FAIL: Expected 30, but found '$TOTAL_DETECTED'${NC}"
fi


# Test 5: Validar veracidad de 3 números Primos
echo -e "${YELLOW}Test 5: Verifying 3 prime numbers logic (+20)...${NC}"
PRIME_LIST=$(echo "$SUMMARY_FINAL" | grep "Primes numbers:" | cut -d':' -f2 | tr ',' ' ')

ERRORS_P=0
COUNT_P=0
for num in $PRIME_LIST; do
    [ $COUNT_P -eq 3 ] && break 
    ((COUNT_P++))

    # Algoritmo de primalidad
    is_p=1
    [ "$num" -lt 2 ] && is_p=0
    for ((i=2; i*i<=num; i++)); do
        if (( num % i == 0 )); then is_p=0; break; fi
    done

    if [ $is_p -ne 1 ]; then
        echo -e "${RED}   - FAIL: $num está marcado como PRIMO pero no lo es${NC}"
        ERRORS_P=$((ERRORS_P + 1))
    fi
done

if [ $ERRORS_P -eq 0 ] && [ $COUNT_P -gt 0 ]; then
    echo -e "${GREEN}   - 3 prime numbers verified: PASS (+20)${NC}"
    SCORE=$((SCORE + 20))
else
    echo -e "${RED}   - FAIL in prime logic validation${NC}"
fi

# Test 6: Validar veracidad de 3 números No Primos
echo -e "${YELLOW}Test 6: Verifying 3 non-prime numbers logic (+20)...${NC}"
NON_PRIME_LIST=$(echo "$SUMMARY_FINAL" | grep "Non primes numbers:" | cut -d':' -f2 | tr ',' ' ')

ERRORS_NP=0
COUNT_NP=0
for num in $NON_PRIME_LIST; do
    [ $COUNT_NP -eq 3 ] && break
    ((COUNT_NP++))

    is_p=1
    [ "$num" -lt 2 ] && is_p=0
    for ((i=2; i*i<=num; i++)); do
        if (( num % i == 0 )); then is_p=0; break; fi
    done

    if [ $is_p -eq 1 ] && [ "$num" -gt 1 ]; then
        echo -e "${RED}   - FAIL: $num está marcado como NO PRIMO pero sí es primo${NC}"
        ERRORS_NP=$((ERRORS_NP + 1))
    fi
done

if [ $ERRORS_NP -eq 0 ] && [ $COUNT_NP -gt 0 ]; then
    echo -e "${GREEN}   - 3 non-prime numbers verified: PASS (+20)${NC}"
    SCORE=$((SCORE + 20))
else
    echo -e "${RED}   - FAIL in non-prime logic validation${NC}"
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
