#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "functions.h"

int main() {
  int score = 0;
  int count = 3;

  // TEST 1: Memory Allocation (20 pts)
  Item* myStock = CreateInventory(count);
  if (myStock != NULL) {
    score += 20;
  } else {
    printf("FAIL: CreateInventory returned NULL.\n");
    printf("RESULT: 0\n");
    return 0;
  }

  // TEST 2: Add Items (Verified via logic check below)
  // Item 1: 5 Laptop at 1000.00 = 5000.00
  AddItem(myStock, 0, "Laptop", 1000.00, 5);
  // Item 2: 10 Mouse at 50.50 = 505.00
  AddItem(myStock, 1, "Mouse", 50.50, 10);
  // Item 3: 100 Paper at 0.10 = 10.00
  AddItem(myStock, 2, "Paper", 0.10, 100);

  // TEST 3: Calculate Total (30 pts)
  // Expected: 5000 + 505 + 10 = 5515.00
  float total = CalculateTotalValue(myStock, count);

  if (fabs(total - 5515.00) < 0.01) {
    score += 20;
  } else {
    printf("FAIL: Expected 5515.00, got %.2f\n", total);
  }

  // TEST 4: Edge Case - Zero Quantity (10 pts)
  AddItem(myStock, 2, "Paper", 0.10, 0);  // Should add 0 to total
  float newTotal = CalculateTotalValue(myStock, count);
  if (fabs(newTotal - 5505.00) < 0.01) {
    score += 15;
  }

  free(myStock);
  printf("RESULT: %d\n", score);
  return 0;
}