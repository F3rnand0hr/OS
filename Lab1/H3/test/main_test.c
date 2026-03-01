#include <math.h>
#include <stdio.h>

#include "functions.h"

int main() {
  int score = 0;

  // Test 1: 3-4-5 Triangle (Origin to 3,4) - 25 pts
  Point a = {0.0, 0.0};
  Point b = {3.0, 4.0};
  float d1 = CalculateDistance(a, b);

  if (fabs(d1 - 5.0) < 0.01) {
    score += 20;
  } else {
    printf("FAIL: Expected 5.00, got %f\n", d1);
  }

  // Test 2: Horizontal Distance (Negative coordinates) - 25 pts
  Point c = {-2.0, 1.0};
  Point d = {2.0, 1.0};  // Distance should be 4
  float d2 = CalculateDistance(c, d);

  if (fabs(d2 - 4.0) < 0.01) {
    score += 25;
  } else {
    printf("FAIL: Expected 4.00, got %f\n", d2);
  }

  // Output raw score for the script
  printf("RESULT: %d\n", score);
  return 0;
}