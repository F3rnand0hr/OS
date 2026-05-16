#!/bin/bash

# --- CONFIGURACIÓN DE COLORES ---
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SCORE=0
LOG_FILE="grading_session.log"
# Asegurarnos de que el log esté limpio y en la carpeta correcta
rm -f "$LOG_FILE"
touch "$LOG_FILE"

echo -e "${BLUE}==================================================${NC}"
echo -e "${BLUE}    OPERATING SYSTEMS - THREAD SCHEDULER GRADER   ${NC}"
echo -e "${BLUE}==================================================${NC}\n"

# --- 1. ORGANIZATION & COMPILATION (5 pts) ---
echo -e "${YELLOW}[1/4] Checking Compilation...${NC}"
mkdir -p ../build && cd ../build
cmake .. > /dev/null 2>&1 && make > /dev/null 2>&1
if [ $? -eq 0 ] && [ -f "./scheduler" ] && [ -f "./emisor" ]; then
    echo -e "  ${GREEN}[OK] Compilation successful (+5 pts)${NC}"
    SCORE=$((SCORE + 5))
else
    echo -e "  ${RED}[FAIL] Compilation failed.${NC}"
    exit 1
fi
# Regresamos a la carpeta de tests para que las rutas relativas funcionen
cd ../tests

# --- 2. COMUNICACIÓN IPC Y LANZAMIENTO (35 pts) ---
echo -e "${YELLOW}[2/4] Testing IPC & Message Queue... (35 pts)${NC}"
QUEUE_NAME="/scheduler_queue"
pkill -9 scheduler > /dev/null 2>&1
rm -f /dev/mqueue$QUEUE_NAME

# Lanzar scheduler REDIRIGIENDO AL LOG para que la sección 3 pueda leerlo
../build/scheduler > "$LOG_FILE" 2>&1 &
SCHED_PID=$!
sleep 1

# TEST A: Existencia del nodo (15 pts)
if [ -f "/dev/mqueue$QUEUE_NAME" ]; then
    echo -e "  ${GREEN}[OK] Message Queue node created (+15 pts)${NC}"
    SCORE=$((SCORE + 15))
else
    echo -e "  ${RED}[FAIL] Message Queue node not found (+0 pts)${NC}"
fi

# TEST B: Consumo de mensajes (20 pts)
../build/emisor > /dev/null 2>&1
sleep 2

# Verificamos si el scheduler tiene la cola abierta (criterio de sistema)
if lsof -p $SCHED_PID 2>/dev/null | grep -q "mqueue"; then
    echo -e "  ${GREEN}[OK] Scheduler is connected to the queue (+20 pts)${NC}"
    SCORE=$((SCORE + 20))
else
    # Si lsof no está disponible, verificamos que el log tenga actividad
    if [ -s "$LOG_FILE" ] && grep -q "Tarea" "$LOG_FILE"; then
        echo -e "  ${GREEN}[OK] Messages consumed (verified via log) (+20 pts)${NC}"
        SCORE=$((SCORE + 20))
    else
        echo -e "  ${RED}[FAIL] No communication detected (+0 pts)${NC}"
    fi
fi

# --- 3. MULTICORE AFFINITY (35 pts) ---
echo -e "${YELLOW}[3/4] Testing CPU Affinity... (30 pts)${NC}"

# No necesitamos lanzar el emisor de nuevo, ya lo hicimos arriba y los hilos duran 1s
sleep 2

CONFIRMADOS=$(grep -c "CONFIRMED" "$LOG_FILE")
CORE0=$(grep -c "Core Real: 0" "$LOG_FILE")
CORE1=$(grep -c "Core Real: 1" "$LOG_FILE")

if [ "$CORE0" -gt 0 ] && [ "$CORE1" -gt 0 ]; then
    echo -e "  ${GREEN}[OK] Kernel verified affinity for both cores (+35 pts)${NC}"
    SCORE=$((SCORE + 35))
else
    echo -e "  ${RED}[FAIL] Affinity mismatch or Kernel too slow (+0 pts)${NC}"
    echo -e "  [DEBUG] Cores detectados: Core0($CORE0), Core1($CORE1)"
fi

# --- 4. THREAD INTEGRITY & CLEANUP (25 pts) ---
echo -e "${YELLOW}[4/4] Checking Thread Integrity...${NC}"
if ! grep -qiE "error|failed|segfault" "$LOG_FILE"; then
    echo -e "  ${GREEN}[OK] No execution errors detected (+25 pts)${NC}"
    SCORE=$((SCORE + 25))
else
    echo -e "  ${RED}[FAIL] Errors found in scheduler logs.${NC}"
fi

# --- RESULTADO FINAL ---
pkill -9 scheduler > /dev/null 2>&1
echo -e "\n${BLUE}==================================================${NC}"
if [ $SCORE -eq 100 ]; then
    echo -e "${GREEN} FINAL GRADE: $SCORE / 100 ${NC}"
else
    echo -e "${YELLOW} FINAL GRADE: $SCORE / 100 ${NC}"
fi
echo -e "${BLUE}==================================================${NC}"