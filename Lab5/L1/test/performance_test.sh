#!/bin/bash

# RUTAS
INPUT_IMG="../build/input.bmp"
MASTER_IMG="../test/master.bmp"
STUDENT_OUT="../build/output.bmp" 
EXECUTABLE="../pixel_inverter"

SCORE=0

echo "--- STARTING SYSTEM I/O & PERFORMANCE TEST ---"

# 0. PRE-REQUISITO: ¿Existe el input en build?
if [ ! -f "$INPUT_IMG" ]; then
    echo "ERROR: $INPUT_IMG the build folder does not exists."
    echo "Copy the image to ../build/input.bmp before running the performance test."
    exit 1
fi

# 1. COMPILACIÓN (5 pts)
echo -n "1. Compiling... "
mkdir -p ../build
gcc -Wall -Werror ../src/main.c -I../include -o $EXECUTABLE &> /dev/null
if [ $? -eq 0 ]; then
    echo "PASSED (+5)"
    SCORE=$((SCORE + 5))
else
    echo "FAILED"; exit 1
fi

# 2. VALIDACIÓN DE APERTURA (Lectura Real) (25 pts)
# Evaluamos que el programa sea capaz de abrir el archivo real
echo -n "2. Evaluating fopen(argv[1], \"rb\") on existing file... "
$EXECUTABLE "$INPUT_IMG" "$STUDENT_OUT" &> /dev/null
if [ $? -eq 0 ]; then
    echo "PASSED (+25)"
    SCORE=$((SCORE + 25))
else
    echo "FAILED (Program could not open or read $INPUT_IMG)"
fi

# 3. VALIDACIÓN DE GUARDADO (Escritura Real) (25 pts)
# Evaluamos si el archivo se creó y tiene un tamaño mayor a cero (no está vacío)
echo -n "3. Evaluating fopen(argv[2], \"wb\") and write success... "
if [ -s "$STUDENT_OUT" ]; then
    echo "PASSED (+25)"
    SCORE=$((SCORE + 25))
else
    echo "FAILED (Output file is missing or empty)"
fi

# 4. VALIDACIÓN SEMÁNTICA (MD5) (45 pts)
echo -n "4. Validating pixel inversion accuracy (MD5)... "
# Saltamos el header de 54 bytes
MASTER_HASH=$(tail -c +55 "$MASTER_IMG" | md5sum | cut -d' ' -f1)
STUDENT_HASH=$(tail -c +55 "$STUDENT_OUT" | md5sum | cut -d' ' -f1)

if [ "$MASTER_HASH" == "$STUDENT_HASH" ]; then
    echo "PASSED (+45)"
    SCORE=$((SCORE + 45))
else
    echo "FAILED"
    echo "   Master:  $MASTER_HASH"
    echo "   Student: $STUDENT_HASH"
fi

echo "---------------------------------------"
echo "TOTAL SCORE: $SCORE / 100"
echo "---------------------------------------"