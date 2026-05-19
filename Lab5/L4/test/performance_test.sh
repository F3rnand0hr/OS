#!/bin/bash

# ==========================================================
# AUTOMATED EVALUATION SCRIPT - OS LABORATORY
# TOTAL SCORE: 100 PTS
# ==========================================================

# Terminal Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

TOTAL_SCORE=0

echo -e "--- STARTING TECHNICAL EVALUATION ---\n"

# 1. ARCHITECTURE AND COMPILATION (5 PTS)
echo -n "1. Verifying directory structure and CMake... "
if [ -d "../src" ] && [ -d "../include" ] && [ -f "../CMakeLists.txt" ]; then
    mkdir -p ../build
    cd ../build
    cmake .. &> /dev/null
    make &> /dev/null
    if [ $? -eq 0 ] && [ -f "./l4" ]; then
        echo -e "${GREEN}PASSED 5 pts${NC}"
        TOTAL_SCORE=$((TOTAL_SCORE + 5))
    else
        echo -e "${RED}FAILED (Compilation error)${NC}"
    fi
else
    echo -e "${RED}FAILED (Incorrect directory structure)${NC}"
fi

# 2. RACE CONDITIONS / ATOMIC TYPES VALIDATION (30 PTS)
# Adjusted to reach a 100-point scale based on your previous breakdown
echo -n "2. Verifying use of volatile sig_atomic_t... "
ATOMIC_CHECK=$(grep -c "volatile sig_atomic_t" ../src/main.c)
if [ "$ATOMIC_CHECK" -ge 3 ]; then
    echo -e "${GREEN}PASSED 30 pts${NC}"
    TOTAL_SCORE=$((TOTAL_SCORE + 30))
else
    echo -e "${RED}FAILED (Insufficient atomic types detected)${NC}"
fi
# 3. SIGNAL HANDLING AND ASYNC-SAFE HANDLERS (30 PTS)
echo -e "3. Running program for signal testing (SIGINT)..."
./l4 500 100 > test_output.txt &
PID=$!
sleep 2

# Sending SIGINT (Simulated Ctrl+C)
kill -SIGINT $PID
sleep 1

# Check if program is still running (it should not die with SIGINT)
if ps -p $PID > /dev/null; then
    SIGINT_OK=$(grep -ciE "Snapshot" test_output.txt)
    if [ "$SIGINT_OK" -ge 1 ]; then
        echo -e "${GREEN}   [OK] SIGINT handler captured correctly. 30 pts${NC}"
        SIG_PTS=30
    else
        echo -e "${RED}   [ERROR] SIGINT did not print the expected message.${NC}"
        SIG_PTS=0
    fi
else
    echo -e "${RED}   [ERROR] Program died on SIGINT (Handler not configured).${NC}"
    SIG_PTS=0
fi
TOTAL_SCORE=$((TOTAL_SCORE + SIG_PTS))

# 4. TIMER & SIGNAL HANDLER CONFIGURATION (30 PTS)
# This checks if both ITIMER_REAL and ITIMER_VIRTUAL are referenced in the code
echo -n "4. Verifying Timer and Signal Handler configuration... "
TIMER_REAL_CHECK=$(grep -c "ITIMER_REAL" ../src/main.c)
TIMER_VIRT_CHECK=$(grep -c "ITIMER_VIRTUAL" ../src/main.c)
SIGACTION_CHECK=$(grep -c "sigaction" ../src/main.c)

if [ "$TIMER_REAL_CHECK" -ge 1 ] && [ "$TIMER_VIRT_CHECK" -ge 1 ] && [ "$SIGACTION_CHECK" -ge 2 ]; then
    echo -e "${GREEN}PASSED 35 pts${NC}"
    TOTAL_SCORE=$((TOTAL_SCORE + 35))
else
    echo -e "${RED}FAILED (Missing ITIMER_REAL, ITIMER_VIRTUAL, or sigaction)${NC}"
fi

# FINAL SUMMARY
echo -e "\n--------------------------------------------------"
echo -e "  ESTIMATED FINAL GRADE: ${TOTAL_SCORE} / 100${NC}"
echo -e "--------------------------------------------------"