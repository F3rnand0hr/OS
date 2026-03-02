#!/bin/bash

SCORE=0
TOTAL=100

echo "--- EVALUATING HOMEWORK 1: MINISHELL ---"
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

# 1. Preparación y Compilación
cd ..
rm -rf build && mkdir build && cd build
cmake .. && make > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo -e "${GREEN}1. Compilation: PASS (+30)${NC}"
    SCORE=30
else
    echo -e "${RED}1. Compilation: FAIL (+0)${NC}"
    echo "TOTAL_SCORE: 0"
    exit 1
fi


MAIN_SRC="../src/main.c"
MINISHELL_EXE="./minishell"


echo -e "ls &\nxxxx\nexit\n" > input.txt

"./$MINISHELL_EXE" < input.txt > output.log 2>&1 &
SHELL_PID=$!

FOUND=0
for i in {1..5}; do
    CHILDREN=$(pgrep -P $SHELL_PID)
    if [ -n "$CHILDREN" ]; then
        FOUND=1
        echo -e "${GREEN}   - Found child process(es) created by minishell: $CHILDREN${NC}"
        break
    fi
    sleep 0.5
done

# 1. Test de Código Fuente (Análisis Estático)
echo -e "${YELLOW}Test 1: Validating execution and child processes...${NC}"
if [ $FOUND -eq 1 ]; then
    echo -e "${GREEN}   - Process creation: PASS (+30)${NC}"
    SCORE=$((SCORE + 30))
else
    echo -e "${RED}   - Process creation: FAIL (No children detected)${NC}"
fi

sleep 2

if ps -p $SHELL_PID > /dev/null; then
    kill -9 $SHELL_PID 2>/dev/null
fi

# 2. Test de Código Fuente 
echo -e "${YELLOW}Test 2: Checking fork usage...${NC}"
if grep -qE "fork\s*\(" "$MAIN_SRC"; then
    echo -e "${GREEN}   - 'fork' usage found: PASS (+20)${NC}"
    SCORE=$((SCORE + 10))
else
    echo -e "${RED}   - 'fork' usage NOT found in source${NC}"
fi

# 3. Test de Código Fuente 
echo -e "${YELLOW}Test 3: Checking execvp usage...${NC}"
if grep -qE "execvp\s*\(" "$MAIN_SRC"; then
    echo -e "${GREEN}   - 'execvp' usage found: PASS (+20)${NC}"
    SCORE=$((SCORE + 10))
else
    echo -e "${RED}   - 'execvp' usage NOT found in source${NC}"
fi

# Test 4: Validate Error Message for invalid commands
echo -e "${YELLOW} Test 4: Validate 'command not found' message${NC}"
if grep -qi "command not found" output.log; then
    echo -e "${GREEN}   - Error message validation: PASS (+10)${NC}"
    SCORE=$((SCORE + 10))
else
    echo -e "${RED}   - Error message validation: FAIL (+0)${NC}"
    echo "     (Expected: 'command not found: xxxx')"

fi
# Test 4: Validate Async mode for invalid commands
echo -e "${YELLOW} Test 5: Validate Async mode{NC}"
if grep -qi ">>>>>> minishell: >>>>>> minishell:" output.log; then
    echo -e "${GREEN}   - Async message validation: PASS (+10)${NC}"
    SCORE=$((SCORE + 10))
else
    echo -e "${RED}   - Async message validation: FAIL (+0)${NC}"
fi

rm -f input.txt output.log

echo "---------------------------"
echo "FINAL GRADE: $SCORE / $TOTAL"
echo -e "${GREEN}TOTAL_SCORE: $SCORE${NC}"
echo "---------------------------"