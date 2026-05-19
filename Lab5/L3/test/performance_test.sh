#!/bin/bash

# Configuración de colores
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'
SCORE=0

BIN="../build/l3"

echo -e "--- Performance Test ---\n"

# 1. Test de Compilación (5 pts)
echo -n "[TEST 1] Checking compilation... "
if [ -f "$BIN" ]; then
    SCORE=$((SCORE + 5))
    echo -e "${GREEN}PASSED (+5 pts)${NC}"
else
    echo -e "${RED}FAILED${NC}"
    echo "Ensure you have compiled in the build folder before running the test."
    exit 1
fi

# 2. Test de Creación y Notificación (40 pts)
echo -n "[TEST 2] Verifying Timer Lifecycle & Signal Handling... "
TEST_OUT=$( $BIN 0 0.1 2>&1 )

if echo "$TEST_OUT" | grep -qiE "EXECUTING|EJECUTANDO" && echo "$TEST_OUT" | grep -q "completed"; then
    SCORE=$((SCORE + 40))
    echo -e "${GREEN}PASSED (+40 pts)${NC}"
else
    echo -e "${RED}FAILED${NC}"
    echo "  -> Error: Task execution not detected or system shutdown failed."
fi

# 3. Test de Precisión Temporal (35 pts)
echo -n "[TEST 3] Testing decimal precision (Delta 0.5s)... "
# Tarea 1 (3.5s) y Tarea 0 (4.0s) -> Diferencia 0.5s
OUTPUT=$( $BIN 1 3.5 0 4.0 | grep "EXECUTING" )
T1=$(echo "$OUTPUT" | sed -n '1p' | cut -d'[' -f2 | cut -d' ' -f1 | tr -d 's]')
T2=$(echo "$OUTPUT" | sed -n '2p' | cut -d'[' -f2 | cut -d' ' -f1 | tr -d 's]')

# Cálculo de diferencia con python3
DIFF=$(python3 -c "print(abs(float($T2) - float($T1)))")

if (( $(echo "$DIFF > 0.45 && $DIFF < 0.55" | bc -l) )); then
    SCORE=$((SCORE + 35))
    echo -e "${GREEN}PASSED (+35 pts) - Delta: ${DIFF}s${NC}"
else
    echo -e "${RED}FAILED - Delta detectado: ${DIFF}s (Se esperaba ~0.5s)${NC}"
fi

# 4. Test de Mapeo de Diccionario (20 pts)
echo -n "[TEST 4] Verifying ID to Task Name mapping... "
if $BIN 1 0.1 | grep -iq "Update routing tables"; then
    SCORE=$((SCORE + 20))
    echo -e "${GREEN}PASSED (+20 pts)${NC}"
else
    echo -e "${RED}FAILED (Incorrect name mapping)${NC}"
fi

echo -e "\n--------------------------------------------"
echo -e "FINAL SCORE: ${SCORE} / 100"
echo -e "--------------------------------------------"