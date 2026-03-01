#include <math.h>
#include <stdio.h>

#include "functions.h"

int main() {
  int score = 0;
  float res;
  int status;

  // TEST 1: Addition (10 pts)
  status = Calculate(10.0, 5.5, '+', &res);
  if (status == 0 && fabs(res - 15.5) < 0.01) score += 10;

  // TEST 2: Multiplication 'x' (10 pts)
  status = Calculate(3.0, 4.0, 'x', &res);
  if (status == 0 && fabs(res - 12.0) < 0.01) score += 10;

  // TEST 3: Power (10 pts)
  status = Calculate(2.0, 3.0, '^', &res);
  if (status == 0 && fabs(res - 8.0) < 0.01) score += 10;

  // TEST 4: Division by Zero (10 pts)
  status = Calculate(10.0, 0.0, '/', &res);
  if (status == -1)
    score += 10;
  else
    printf("FAIL: Division by zero did not return error status.\n");

  // TEST 5: Invalid Operator (10 pts)
  status = Calculate(10, 10, '$', &res);
  if (status == -1) score += 10;

  printf("RESULT: %d\n", score);
  return 0;
}