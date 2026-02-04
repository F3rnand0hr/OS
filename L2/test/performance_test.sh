#!/bin/bash

SCORE=0
TOTAL=100

echo "--- EVALUATING EXERCISE 5: INVENTORY SYSTEM ---"

# 1. Compilation (5 pts)
cd ..
rm -rf build && mkdir build && cd build
cmake .. && make
if [ $? -eq 0 ]; then
    echo "1. Compilation: PASS (+5)"
    SCORE=$((SCORE + 5))
else
    echo "1. Compilation: FAIL (+0)"
    echo "FINAL GRADE: 0 / 100"
    exit 1
fi
cd ..

# 2. Logic Unit Test (55 pts)
echo "2. Running Logic Unit Tests..."
gcc -I./include src/functions.c test/main_test.c -o build/func_test -lm
if [ $? -eq 0 ]; then
    RESULT=$(./build/func_test | grep "RESULT:" | cut -d' ' -f2)
    SCORE=$((SCORE + RESULT))
    echo "2. Logic Unit Test: $RESULT/55"
else
    echo "2. Logic Unit Test: FAIL (+0) - Compilation Error"
fi

# 3. Integration/CLI Test (40 pts)
echo "3. Testing Command Line Interface..."

# Simulating input: 2 items
# GPU: 500.00 * 2 = 1000
# RAM: 100.00 * 4 = 400
# Expected Total: 1400.00
INPUT_DATA="GPU 500.00 2\nRAM 100.00 4"

# Run program with argument '2' (count) and pipe input
OUTPUT=$(echo -e "$INPUT_DATA" | ./build/inventory 2 2>&1)
EXPECTED="1400.00"

if [[ "$OUTPUT" == *"$EXPECTED"* ]]; then
    echo "   - CLI Calculation: PASS (+40)"
    SCORE=$((SCORE + 40))
else
    echo "   - CLI Calculation: FAIL (Expected '$EXPECTED', got output with: '$OUTPUT')"
fi

echo "---------------------------"
echo "FINAL GRADE: $SCORE / 100"
echo "---------------------------"