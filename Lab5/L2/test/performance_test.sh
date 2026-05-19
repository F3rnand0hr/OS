#!/bin/bash

# Configuración de rutas y archivos
PROJECT_DIR="$(dirname "$0")/.."
BUILD_DIR="$PROJECT_DIR/build"
EXE_NAME="l2"
LOG_FILE="test_result.log"
EXPECTED_SUM="550005309568.5097656250"

# Inicialización de puntaje
TOTAL_SCORE=0

echo "===================================================="
echo "      PERFORMANCE TEST - GRADING SYSTEM (100 PTS)   "
echo "===================================================="

# --- TEST 1: COMPILACIÓN (5 PTS) ---
echo -n "[TEST 1] Compiling with CMake... "
cd "$BUILD_DIR" || { echo "Error: Build folder not found"; exit 1; }
cmake .. > /dev/null 2>&1 && make > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "PASS (+5 pts)"
    TOTAL_SCORE=$((TOTAL_SCORE + 5))
else
    echo "FAIL (Check your code or CMakeLists.txt)"
    exit 1
fi

# Verificar si el ejecutable existe
if [ ! -f "./$EXE_NAME" ]; then
    echo "Error: Executable $EXE_NAME not found in build/."
    exit 1
fi

# --- EJECUCIÓN INICIAL PARA CAPTURAR DATOS ---
./$EXE_NAME > "$LOG_FILE" 2>&1
cat "$LOG_FILE"

# --- TEST 2: INTEGRIDAD DE DATOS (35 PTS) ---
echo -n "[TEST 2] Verifying Data Integrity... "
if grep -qiE "Verification: (OK|EXITO|ÉXITO)" "$LOG_FILE"; then
    echo "PASS (+35 pts)"
    TOTAL_SCORE=$((TOTAL_SCORE + 35))
else
    echo "FAIL (Sums do not match)"
fi

# --- TEST 3: EFICIENCIA DE MMAP / SPEEDUP (30 PTS) ---
echo -n "[TEST 3] Evaluating Performance Speedup... "
SPEEDUP=$(grep "Performance:" "$LOG_FILE" | awk '{print $6}' | sed 's/x//')

if (( $(echo "$SPEEDUP > 10.0" | bc -l) )); then
    echo "PASS: ${SPEEDUP}x (+30 pts)"
    TOTAL_SCORE=$((TOTAL_SCORE + 30))
elif (( $(echo "$SPEEDUP > 1.0" | bc -l) )); then
    echo "MARGINAL: ${SPEEDUP}x (+15 pts)"
    TOTAL_SCORE=$((TOTAL_SCORE + 15))
else
    echo "FAIL: Binary is too slow (+0 pts)"
fi



# --- TEST 4: VALIDACIÓN CONTRA VALOR MAESTRO (30 PTS) ---
echo -n "[TEST 4] Validating results against 'Master Value'... "


REAL_SUM_TXT=$(grep "Sum .txt:" "$LOG_FILE" | awk '{print $3}')
REAL_SUM_BIN=$(grep "Sum .bin:" "$LOG_FILE" | awk '{print $3}')

# Margen de error permitido por precisión de punto flotante
EPSILON="1.0"

# Función interna para comparar con el Expected Sum
check_master() {
    local val=$1
    local master=$2
    # Calculamos la diferencia absoluta y comparamos con EPSILON
    echo "if ((${val} - ${master}) > 0) d=${val}-${master} else d=${master}-${val}; if (d < ${EPSILON}) 1 else 0" | bc -l
}

# Realizar las comparaciones
MATCH_TXT=$(check_master "$REAL_SUM_TXT" "$EXPECTED_SUM")
MATCH_BIN=$(check_master "$REAL_SUM_BIN" "$EXPECTED_SUM")

if [ "$MATCH_TXT" -eq 1 ] && [ "$MATCH_BIN" -eq 1 ]; then
    echo "PASS (+30 pts)"
    TOTAL_SCORE=$((TOTAL_SCORE + 30))
else
    echo "FAIL"
    echo "    - Expected: $EXPECTED_SUM"
    echo "    - Got TXT:  $REAL_SUM_TXT"
    echo "    - Got BIN:  $REAL_SUM_BIN"
fi

# --- RESULTADO FINAL ---
echo "===================================================="
echo " FINAL SCORE: $TOTAL_SCORE / 100"
echo "===================================================="