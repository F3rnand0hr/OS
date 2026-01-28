#include <math.h>
#include <stdio.h>

#include "../include/functions.h"

int main() {
  int passed = 0;

  // Test 1: Kelvin to Celsius
  if (fabs(KelvinToCelsius(300.0) - 26.85) < 0.01) {
    printf("[OK] KelvinToCelsius\n");
    passed++;

  } else {
    printf("[FAIL] KelvinToCelsius logic error\n");
  }

  // Test 2: Celsius to Fahrenheit
  if (fabs(CelsiusToFahrenheit(20.0) - 68.0) < 0.01) {
    printf("[OK] CelsiusToFahrenheit\n");
    passed++;
  } else {
    printf("[FAIL] CelsiusToFahrenheit logic error\n");
  }

  printf("Grade: %d / 30\n", passed * 15);
  return 0;
}