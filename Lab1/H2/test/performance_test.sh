#!/bin/bash

SCORE=0
TOTAL=100

echo "--- EVALUATING EXERCISE 3: CALCULATOR ---"

# 1. Compilation (5 pts)
cd ../build
rm -r *
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
# 2. Functional/Logic Unit Test (50 pts)
# Compiles main_test.c with the student's functions.c
echo "compiling functional tests..."
gcc -I./include src/functions.c test/main_test.c -o build/func_test -lm
if [ $? -eq 0 ]; then
    RESULT=$(./build/func_test | grep "RESULT:" | cut -d' ' -f2)
    # The C test returns up to 50 points
    SCORE=$((SCORE + RESULT))
    echo "2. Logic Unit Test: $RESULT/50"
else
    echo "2. Logic Unit Test: FAIL (+0) - Compilation Error"
fi

# 3. Integration/Argv Test (45 pts)
# We test the actual executable created by CMake
echo "3. Testing Command Line Parsing..."
PASSED_INTEGRATION=0

# Case A: Addition Operation
OUT1=$(./build/console_calculator 10 + 5)
if [[ "$OUT1" == *"15.00"* ]]; then
    PASSED_INTEGRATION=$((PASSED_INTEGRATION + 15))
    echo "   - Addition: PASS"
else
    echo "   - Addition: FAIL (Output: $OUT1)"
fi

# Case A: Power Operation
OUT1=$(./build/console_calculator 2 ^ 5)
if [[ "$OUT1" == *"32.00"* ]]; then
    PASSED_INTEGRATION=$((PASSED_INTEGRATION + 15))
    echo "   - Power: PASS"
else
    echo "   - Power: FAIL (Output: $OUT1)"
fi

# Case B: Error Handling via CLI
OUT2=$(./build/console_calculator 5 / 0 2>&1)
if [[ "$OUT2" == *"Error"* ]] || [[ "$OUT2" == *"Division by zero"* ]]; then
    PASSED_INTEGRATION=$((PASSED_INTEGRATION + 15))
    echo "   - DivByZero: PASS"
else
    echo "   - DivByZero: FAIL (Output: $OUT2)"
fi

SCORE=$((SCORE + PASSED_INTEGRATION))

echo "---------------------------"
echo "FINAL GRADE: $SCORE / 100"
echo "---------------------------"