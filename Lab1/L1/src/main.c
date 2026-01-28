#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Error: Invalid number of arguments.\n");
    return 1;
  }

  if (argv[2][0] != 'C' && argv[2][0] != 'F' && argv[2][0] != 'K') {
    printf("Error: Invalid unit.\n");
    return 1;
  }
  float value = atof(argv[1]);
  char unit = argv[2][0];

  if (unit == 'C') {
    float f = CelsiusToFahrenheit(value);
    float k = CelsiusToKelvin(value);
    printf("%.2f %c is %.2f F and %.2f K\n", value, unit, f, k);
  } else if (unit == 'F') {
    float c = FahrenheitToCelsius(value);
    float k = CelsiusToKelvin(c);
    printf("%.2f %c is %.2f C and %.2f K\n", value, unit, c, k);
  } else if (unit == 'K') {
    float c = KelvinToCelsius(value);
    float f = CelsiusToFahrenheit(c);
    printf("%.2f %c is %.2f C and %.2f F\n", value, unit, c, f);
  }

  return 0;
}