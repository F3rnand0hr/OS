#include "../include/functions.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

float CelsiusToFahrenheit(float celsius) {
  float conversion = ((9.0f / 5.0f) * celsius + 32.0f);
  return conversion;
}

float CelsiusToKelvin(float celsius) { return celsius + 273.15f; }
float FahrenheitToCelsius(float fahrenheit) {
  float conversion = (fahrenheit - 32.0f) * (5.0f / 9.0f);
  return conversion;
}

float KelvinToCelsius(float kelvin) { return kelvin - 273.15f; }