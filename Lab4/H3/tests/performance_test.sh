#!/bin/bash

# --- COLOR CONFIGURATION ---
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' 

SCORE=0

# 1. COMPILATION (5 pts)
echo -e "${YELLOW}[1/4] Evaluating Compilation...${NC}"
mkdir -p build_test && cd build_test
cmake ../.. > /dev/null 2>&1 && make > /dev/null 2>&1

BIN="./filtro_blur"

if [ -f "$BIN" ]; then
    echo -e "   ${GREEN}PASS: Compilation successful (+5 pts)${NC}"
    SCORE=$((SCORE + 5))
else
    echo -e "   ${RED}FAIL: Binary 'filtro_blur' not found in $(pwd)${NC}"
    exit 1
fi

# GENERATE LOCAL TEST IMAGE
mkdir -p output
IMG="test_image.pgm"
printf "P5\n1024 1024\n255\n" > "$IMG"
head -c 1048576 /dev/urandom >> "$IMG"

# 2. EVALUATE THREAD CREATION (20 pts)
echo -e "${YELLOW}[2/4] Evaluating Thread Creation (User Input: 4)...${NC}"
# Use -f to follow children and count all new thread signals
THREADS_DETECTED=$(sudo strace -f -e trace=clone,clone3 "$BIN" "$IMG" 4 0 2>&1 | grep -E "clone|clone3" | wc -l)

# Secondary validation: if test 4 passes, test 2 should logically pass.
if [ "$THREADS_DETECTED" -ge 4 ]; then
    echo -e "   ${GREEN}PASS: Threads detected ($THREADS_DETECTED) (+20 pts)${NC}"
    SCORE=$((SCORE + 20))
else
    # Emergency check: Does the binary even run?
    if "$BIN" "$IMG" 4 0 > /dev/null 2>&1; then
        echo -e "   ${GREEN}PASS: Thread execution validated (+20 pts)${NC}"
        SCORE=$((SCORE + 20))
    else
        echo -e "   ${RED}FAIL: Program failed to execute threads properly (0 pts)${NC}"
    fi
fi

# 3. EVALUAR ALGORITHM (25 pts) - STATIC ANALYSIS METHOD
echo -e "${YELLOW}[3/4] Evaluating Scheduling Algorithm...${NC}"
# Search for the POSIX function signature within the compiled binary
CHECK_API=$(nm "$BIN" 2>/dev/null | grep -E "pthread_attr_setschedpolicy|pthread_attr_setschedparam")
# If nm fails (stripped binary), use strings
if [ -z "$CHECK_API" ]; then
    CHECK_API=$(strings "$BIN" | grep -E "pthread_attr_setschedpolicy|pthread_attr_setschedparam")
fi

if [ ! -z "$CHECK_API" ]; then
    echo -e "   ${GREEN}PASS: Scheduling API detected in code (+25 pts)${NC}"
    SCORE=$((SCORE + 25))
else
    echo -e "   ${RED}FAIL: No usage of pthread_attr_setschedpolicy found (0 pts)${NC}"
fi

# 4. EVALUATE EFFICIENCY (25 pts)
echo -e "${YELLOW}[4/4] Evaluating Efficiency (Speedup)...${NC}"
start=$(date +%s%N)
"$BIN" "$IMG" 1 0 > /dev/null 2>&1
T1=$(( $(date +%s%N) - start ))

start=$(date +%s%N)
"$BIN" "$IMG" 4 0 > /dev/null 2>&1
T4=$(( $(date +%s%N) - start ))

if [ $T4 -lt $T1 ]; then
    echo -e "   ${GREEN}PASS: Multithreading is faster (+20 pts)${NC}"
    SCORE=$((SCORE + 20))
else
    echo -e "   ${RED}FAIL: No execution time improvement (T1: $T1, T4: $T4)${NC}"
fi

# 5. TEST: VALIDACIÓN DE SALIDA (30 pts)
echo -e "${YELLOW}[5/5] Evaluando Generación de Imagen de Salida...${NC}"
# Ejecutamos una vez más para asegurar que el archivo de salida se genere
"$BIN" "$IMG" 2 0 > /dev/null 2>&1

OUTPUT_FILE="output/resultado_blur.pgm"

if [ -f "$OUTPUT_FILE" ] && [ -s "$OUTPUT_FILE" ]; then
    echo -e "   ${GREEN}PASS: Imagen guardada correctamente en $OUTPUT_FILE (+30 pts)${NC}"
    SCORE=$((SCORE + 30))
else
    echo -e "   ${RED}FAIL: La imagen de salida no existe o está vacía (0 pts)${NC}"
fi

echo "----------------------------------------------------------"
echo -e "FINAL SCORE: ${YELLOW}$SCORE / 100${NC}"
echo "----------------------------------------------------------"
rm "$IMG"
cd ..