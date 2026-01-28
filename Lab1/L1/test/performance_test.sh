#!/bin/bash

# Configuration
SCORE=0
TOTAL=100

echo "--- STARTING EVALUATION ---"

# 1. Compilation Test (5 Points)
echo -n "1. Compiling Project... "
cd ../build
rm -rf *
gcc -I../include ../src/*.c -o temperature_converter -lm > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "PASS (+5)"
    SCORE=$((SCORE + 5))
else
    echo "FAIL (+0)"
fi


# 2. Argument Validation (20 Points)
echo -n "2. Testing Error Handling... "
ERR_OUT=$(./temperature_converter 10 C extra_arg 2>&1)
if [[ "$ERR_OUT" == *"Error"* ]]; then
    echo "PASS (+20)"
    SCORE=$((SCORE + 20))
else
    echo "FAIL (+0) - Program didn't report error for invalid arguments"
fi

# 3. Functional/Logic Test (30 Points)
echo "3. Testing Individual Functions (grader_main)... "
cd ..
gcc -I../include ./src/functions.c ./test/main_test.c -o ./build/functional_test -lm
if [ $? -eq 0 ]; then
    FUNC_SCORE=$(./build/functional_test)
    echo "$FUNC_SCORE"
    FUNC_SCORE=$(echo "$FUNC_SCORE" | tail -n 1 | cut -d' ' -f2)
    # Assumes grader_main outputs a number 0-30
    SCORE=$((SCORE + FUNC_SCORE))
else
    echo "FAIL (+0) - Could not compile grader_main"
fi

# 4. Input/Output Accuracy (45 Points)
echo "4. Testing Conversion Accuracy:"
# Format: "Input_Val Input_Unit | Expected_Part1 | Expected_Part2"
test_cases=(
    "100.0 C | 212.00 F | 373.15 K"
    "32.0 F | 0.00 C | 273.15 K"
    "0.0 K | -273.15 C | -459.67 F"
)

CASE_POINTS=15
for test in "${test_cases[@]}"; do
    IFS="|" read -r input part1 part2 <<< "$test"
    OUTPUT=$(./build/temperature_converter $input 2>&1)

    if [[ "$OUTPUT" == *"$part1"* ]] && [[ "$OUTPUT" == *"$part2"* ]]; then
        echo "   - Input $input: PASS (+$CASE_POINTS)"
        SCORE=$((SCORE + CASE_POINTS))
    else
        echo "   - Input $input: FAIL"
    fi
done

echo "---------------------------"
echo "FINAL GRADE: $SCORE / $TOTAL"
echo "---------------------------"