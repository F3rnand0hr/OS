#!/bin/bash

# --- Configuration ---
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'
EXECUTABLE="./memory_mapper"
TEST_LOG="memory_test_results.csv"
OUTPUT_FILE="test_output.log"
FINAL_SCORE=0
TOTAL_CHECKS=4 # One check per memory block
EXPECTED_DATA=(100 200 300 400)

# --- Cleanup & Compile ---
# rm -f $EXECUTABLE $TEST_LOG
echo "--- Compiling Memory Mapper ---"
gcc -Wall -Werror ../src/main.c -o $EXECUTABLE
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation FAILED. Exiting.${NC}"
    exit 1
fi
echo -e "${GREEN}Compilation successful!${NC}"

# --- Execution ---
echo -e "\n--- Running Memory Mapper (Background) ---"
# Run the program in the background and capture all output immediately
stdbuf -o0 "$EXECUTABLE" $NICE_INPUT $CPU_INPUT > $OUTPUT_FILE 2>&1 &
PID=$!
sleep 1 # Give it a moment to run and map the memory

# --- Extract Log Data ---
LOG_LINE=$(grep "^LOG_START|" "$OUTPUT_FILE")

echo -e "\n--- Extracted Log Line ---"
echo "$LOG_LINE"    

if [ -z "$LOG_LINE" ]; then
    echo -e "${RED}ERROR: Could not find required LOG_START line in output. Exiting.${NC}"
    kill -9 $PID 2>/dev/null
    exit 1
fi

# Extract PID from the log line
LOG_PID=$(echo "$LOG_LINE" | awk -F'|' '{print $2}' | awk -F'=' '{print $2}')

echo "PID: ${PID}  LOG_PID: ${LOG_PID}"
if [ "$PID" -ne "$LOG_PID" ]; then
    echo -e "${RED}ERROR: PID mismatch. Exiting.${NC}"
    kill -9 $PID 2>/dev/null
    exit 1
fi

# --- Validation and Scoring ---
echo -e "\n--- Validating Mapped Addresses against /proc/$PID/maps ---"
echo "Name,Status,Reported_Addr,Mapped_Range,Size_Check(KB),Score" > $TEST_LOG
CURRENT_SCORE=0

# Loop through the four blocks defined in the log
for i in 0 1 2 3; do
    # 1. Parse Block Data (Example: BLOCK_0_AUTO_A=0x7ff8f4a00000/65536/100/...)
    index=$((i + 3)) # Block data starts at field 3
    BLOCK_DATA=$(echo "$LOG_LINE" | awk -F'|' -v idx="$index" '{print idx ":" $idx}')
    echo "Processing Block Data: $BLOCK_DATA i=$i, index=$index"

    # Check if BLOCK_DATA is empty, meaning the LOG line was shorter than expected
    if [ -z "$BLOCK_DATA" ]; then continue; fi

    # Parse all required fields: NAME=ADDRESS/SIZE/DATA/OS_MAP_RANGE
    NAME=$(echo "$BLOCK_DATA" | awk -F'=' '{print $1}')
    REPORTED_ADDR_HEX=$(echo "$BLOCK_DATA" | awk -F'=' '{print $2}' | awk -F'/' '{print $1}')
    REPORTED_SIZE=$(echo "$BLOCK_DATA" | awk -F'/' '{print $2}')
    REPORTED_DATA=$(echo "$BLOCK_DATA" | awk -F'/' '{print $3}') # New: DATA value
    REPORTED_MAP_RANGE=$(echo "$BLOCK_DATA" | awk -F'/' '{print $4}' | awk -F'|' '{print $1}')
    
    # Convert reported address (0x...) to decimal for range check
    REPORTED_ADDR_DEC=$(printf "%d" "$REPORTED_ADDR_HEX")

    # Calculate expected size in KB (for easier reading)
    REPORTED_SIZE_KB=$((REPORTED_SIZE / 1024))

    # --- Check 4: Data Integrity ---
    DATA_STATUS="FAIL"
    DATA_CHECK_OK=0
    if [ "$REPORTED_DATA" -eq "${EXPECTED_DATA[$i]}" ]; then
        DATA_STATUS="${GREEN}OK (Value: ${EXPECTED_DATA[$i]})${NC}"
        DATA_CHECKS_PASSED=$((DATA_CHECKS_PASSED + 1))
        DATA_CHECK_OK=1
    else
        DATA_STATUS="${RED}FAIL (Expected: ${EXPECTED_DATA[$i]}) | Got: $REPORTED_DATA${NC}"
    fi

    # --- Proc Maps Search ---
    # Search the /proc/[PID]/maps file for the reported *RANGE* (as reported by the C program)
    MAPS_LINE=$(grep -m 1 "$REPORTED_MAP_RANGE" /proc/$PID/maps)

    STATUS="FAIL"
    MAPPED_RANGE="N/A"
    
    if [ -n "$MAPS_LINE" ]; then
        # Check 1: Did the reported OS range match a line in the actual maps file?
        MAPPED_RANGE=$(echo "$MAPS_LINE" | awk '{print $1}')

        # Extract start and end from the verified kernel map entry
        MAP_START_HEX=$(echo "$MAPPED_RANGE" | awk -F'-' '{print $1}')
        MAP_END_HEX=$(echo "$MAPPED_RANGE" | awk -F'-' '{print $2}')
        
        # Convert map addresses to decimal for numerical comparison
        MAP_START_DEC=$(printf "%d" 0x$MAP_START_HEX)
        MAP_END_DEC=$(printf "%d" 0x$MAP_END_HEX)

        # Check 2: Verify the reported address is *INSIDE* the kernel's recorded range
        IS_ADDR_IN_RANGE=false
        if [ "$REPORTED_ADDR_DEC" -ge "$MAP_START_DEC" ] && [ "$REPORTED_ADDR_DEC" -lt "$MAP_END_DEC" ]; then
            IS_ADDR_IN_RANGE=true
        fi

        # Check 3: Does the reported size match the map size?
        MAP_SIZE_DEC=$((MAP_END_DEC - MAP_START_DEC))
        
        # Final status calculation: Must pass Addr/Size check AND Data check
        if $IS_ADDR_IN_RANGE && [ "$MAP_SIZE_DEC" -ge "$REPORTED_SIZE" ] && [ "$DATA_CHECK_OK" -eq 1 ]; then
            STATUS="${GREEN}PASS (All Checks)${NC}"
            CURRENT_SCORE=$((CURRENT_SCORE + 1))
        elif $IS_ADDR_IN_RANGE && [ "$MAP_SIZE_DEC" -ge "$REPORTED_SIZE" ]; then
            STATUS="${YELLOW}PASS (Addr/Size OK, Data FAIL)${NC}"
        elif $IS_ADDR_IN_RANGE; then
            STATUS="${YELLOW}PASS (Addr OK, Size/Data FAIL)${NC}"
        else
            STATUS="${RED}FAIL (Addr/Range Error)${NC}"
        fi
    else
        STATUS="${RED}FAIL (Range not found in OS maps)${NC}"
    fi

    # Print log line
    echo -e "$NAME,$STATUS,$REPORTED_ADDR_HEX,$MAPPED_RANGE,Reported: $REPORTED_SIZE_KB KB,$DATA_STATUS" | tee -a $TEST_LOG
done

# --- Final Cleanup and Summary ---
echo -e "\n--- Summary ---"
# kill -9 $PID 2>/dev/null # Ensure the process is killed

if [ $CURRENT_SCORE -eq $TOTAL_CHECKS ]; then
    FINAL_SCORE=$(echo "100")
    echo -e "${GREEN}SUCCESS: All ${TOTAL_CHECKS} memory blocks were successfully validated!${NC}"
else
    FINAL_SCORE=$(echo "scale=2; $CURRENT_SCORE / $TOTAL_CHECKS * 100" | bc)
    echo -e "${YELLOW}WARNING: Only $CURRENT_SCORE out of $TOTAL_CHECKS blocks were correctly mapped.${NC}"
fi

echo -e "Final Grade: ${YELLOW}$FINAL_SCORE${NC} / 100"
echo "-------------------"
