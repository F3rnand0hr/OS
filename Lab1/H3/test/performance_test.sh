#!/bin/bash

SCORE=0
TOTAL=100

echo "--- EVALUATING EXERCISE 4: DISTANCE CALCULATOR ---"

# 1. Compilation (5 pts)
cd ../build
rm -rf *
# Note: We must link the math library (-lm) in CMake, 
# but for safety, we often add it in the gcc commands below too.
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
# 2. Logic Unit Test (40 pts)
# We explicitly add -lm here to ensure the test runner links math.h correctly
echo "2. Running Logic Unit Tests..."
gcc -I./include src/functions.c test/main_test.c -o build/func_test -lm
if [ $? -eq 0 ]; then
    RESULT=$(./build/func_test | grep "RESULT:" | cut -d' ' -f2)
    SCORE=$((SCORE + RESULT))
    echo "2. Logic Unit Test: $RESULT/40"
else
    echo "2. Logic Unit Test: FAIL (+0) - Compilation Error"
fi

# 3. Integration/CLI Test (50 pts)
echo "3. Testing Command Line Interface..."
# Case A: 3-4-5 Triangle
OUTPUT_A=$(./build/point_distances 0 0 3 4 2>&1)
EXPECTED_A="Distance: 5.00"

if [[ "$OUTPUT_A" == *"$EXPECTED_A"* ]]; then
    echo "   - Case A (0,0)->(3,4): PASS (+20)"
    SCORE=$((SCORE + 25))
else
    echo "   - Case A: FAIL (Expected '$EXPECTED_A', got '$OUTPUT_A')"
fi

# Case B: Large Negative Coordinates
OUTPUT_B=$(./build/point_distances 20 0 -5 -12 2>&1)
EXPECTED_B="Distance: 27.73"

if [[ "$OUTPUT_B" == *"$EXPECTED_B"* ]]; then
    echo "   - Case B (20,0)->(-5,-12): PASS (+20)"
    SCORE=$((SCORE + 25))
else
    echo "   - Case B: FAIL (Expected '$EXPECTED_B', got '$OUTPUT_B')"
fi

echo "---------------------------"
echo "FINAL GRADE: $SCORE / 100"
echo "---------------------------"