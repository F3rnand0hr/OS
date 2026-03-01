#!/bin/bash

SCORE=0
TOTAL=100

echo "--- EVALUATING EXERCISE 2: STUDENT DATABASE ---"

# 1. Clean Build Directory
rm -rf build && mkdir build

# 2. Compile Test Runner (Linking student's logic with our main_test.c)
# Note: We do NOT use the student's main.c here, only their functions.c
echo -n "Compiling Test Runner... "
cd ..
gcc -I./include src/functions.c test/main_test.c -o build/grade_runner -lm 

if [ $? -eq 0 ]; then
    echo "PASS"
else
    echo "FAIL - Compilation Error in functions.c"
    echo "FINAL GRADE: 0 / 100"
    exit 1
fi

# 3. Execute Logic Tests
echo "Running Logic Tests..."
# We capture the output line starting with "RESULT:"
TEST_OUTPUT=$(./build/grade_runner)
echo "$TEST_OUTPUT"
RESULT_LINE=$(echo "$TEST_OUTPUT" | grep "RESULT:")
CALCULATED_SCORE=$(echo $RESULT_LINE | cut -d' ' -f2)

if [ -z "$CALCULATED_SCORE" ]; then
    echo "Error: Test runner did not produce a valid score."
    SCORE=0
else
    SCORE=$CALCULATED_SCORE
fi

echo "---------------------------"
echo "FINAL GRADE: $SCORE / 100"
echo "---------------------------"