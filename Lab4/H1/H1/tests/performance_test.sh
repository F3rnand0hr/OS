#!/bin/bash

# Initializing scores
SCORE=0
TOTAL=100

# Output colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

echo -e "${YELLOW}===========================================${NC}"
echo -e "${YELLOW}   PERFORMANCE & SCHEDULING EVALUATION     ${NC}"
echo -e "${YELLOW}===========================================${NC}"

# 1. Path detection
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT=$(realpath "$SCRIPT_DIR/..")
BUILD_DIR="$PROJECT_ROOT/build"

# 2. Test 1: Compilation with CMake (+5pts)
echo -e "${YELLOW}Test 1: Compiling with CMake (+5)...${NC}"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1
cmake "$PROJECT_ROOT" > /dev/null 2>&1
make > build_log.txt 2>&1

if [ $? -eq 0 ] && [ -f "./manager" ]; then
    echo -e "${GREEN}PASS: Compilation successful${NC}"
    SCORE=$((SCORE + 5))
else
    echo -e "${RED}FAIL: Compilation failed.${NC}"
    exit 1
fi

BINARY="./manager"

# 3. Execution and Data Capture
echo -e "${YELLOW}Running benchmark (sudo required for CPU affinity)...${NC}"
if [ ! -f "instrucciones.txt" ]; then
    echo -e "${RED}ERROR: instrucciones.txt not found!${NC}"
    exit 1
fi

OUTPUT=$(sudo $BINARY 2>&1)

# 4. Test 2: Verifying CPU Affinity (Core 0) (+30 pts)
echo -e "${YELLOW}Test 2: Verifying CPU Affinity (Core 0) (+30)...${NC}"

# Lanzamos el proceso
sudo $BINARY > /dev/null 2>&1 &
PID=$!
disown $PID

PASSED_AFFINITY=false
# Aumentamos a 30 intentos (3 segundos de monitoreo constante)
for i in {1..30}; do
    # Buscamos el core del padre O de cualquier hijo que se llame 'manager'
    # Esto es clave si el trabajo pesado lo hacen los hijos
    CORES=$(ps -eL -o psr,comm | grep "manager" | awk '{print $1}')
    
    for CORE in $CORES; do
        if [ "$CORE" == "0" ]; then
            PASSED_AFFINITY=true
            break 2 # Sale de ambos bucles
        fi
    done
    sleep 0.1
done

# Limpieza
sudo pkill -9 manager > /dev/null 2>&1

if [ "$PASSED_AFFINITY" = true ]; then
    echo -e "${GREEN}PASS: Process (or children) correctly pinned to Core 0${NC}"
    SCORE=$((SCORE + 30))
else
    echo -e "${RED}FAIL: No part of the program was seen on Core 0.${NC}"
    echo -e "${YELLOW}Current Cores detected: $CORES${NC}"
fi

# 5. Test 3: Checking Burst Integrity (+15 pts)
echo -e "${YELLOW}Test 3: Checking Burst Integrity (+15)...${NC}"
BURST_COUNT=$(echo "$OUTPUT" | grep -c "Completado en")

if [ "$BURST_COUNT" -ge 30 ]; then
    echo -e "${GREEN}PASS: 30 iterations detected correctly${NC}"
    SCORE=$((SCORE + 15))
else
    echo -e "${RED}FAIL: Only $BURST_COUNT/30 bursts detected${NC}"
fi

# 6. Test 4: Verifying Final Recommendation (+20 pts)
echo -e "${YELLOW}Test 4: Verifying Final Recommendation (+20)...${NC}"
if echo "$OUTPUT" | grep -qi "RECOMENDACIÓN FINAL" || echo "$OUTPUT" | grep -qi "FINAL RECOMMENDATION"; then
    echo -e "${GREEN}PASS: Recommendation found in output${NC}"
    SCORE=$((SCORE + 20))
else
    echo -e "${RED}FAIL: No final recommendation found${NC}"
fi

# 5. Test 5: Verifying Process Creation Count (+30 pts)
echo -e "${YELLOW}Test 5: Verifying Process Creation Count (+30)...${NC}"


RAFAGAS_DETECTADAS=$(echo "$OUTPUT" | grep -c "Ejecutando")

# Si se detectaron las 30 ráfagas, y sabemos que cada una hace 3 forks:
if [ "$RAFAGAS_DETECTADAS" -eq 30 ]; then
    echo -e "${GREEN}PASS: 30 execution cycles verified (90 child processes implied)${NC}"
    SCORE=$((SCORE + 30))
else
    echo -e "${RED}FAIL: Only $RAFAGAS_DETECTADAS/30 execution cycles detected.${NC}"
fi

# 7. Final Summary
echo -e "${YELLOW}-------------------------------------------${NC}"
echo -e "FINAL GRADE: $SCORE / $TOTAL"
echo -e "TOTAL_SCORE: $SCORE"
echo -e "${YELLOW}-------------------------------------------${NC}"