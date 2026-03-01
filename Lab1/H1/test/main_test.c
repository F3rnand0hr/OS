#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"

int main() {
  int score = 0;
  int student_count = 3;

  // --- TEST 1: Memory Allocation (30 pts) ---
  Student* course = CreateCourse(student_count);
  if (course != NULL) {
    printf("PASS: CreateCourse allocated memory correctly.\n");
    score += 30;
  } else {
    printf("FAIL: CreateCourse returned NULL.\n");
    // Cannot proceed if memory wasn't allocated
    printf("RESULT: %d\n", score);
    return 0;
  }

  // --- TEST 2: Adding Students (30 pts) ---
  // We populate the array and then verify the data exists in memory
  AddStudent(&course[0], "Alice", 70.0, 70.0, 70.0);
  AddStudent(&course[1], "Bob", 90.0, 95.0, 100.0);  // Avg: 95.0
  AddStudent(&course[2], "Charlie", 50.0, 60.0, 70.0);

  // Verify Bob's data was stored correctly
  if (strcmp(course[1].name, "Bob") == 0 && course[1].grades[2] == 100.0) {
    printf("PASS: AddStudent stored data correctly.\n");
    score += 30;
  } else {
    printf("FAIL: AddStudent did not store data correctly.\n");
  }

  // --- TEST 3: Logic / Find Top Student (40 pts) ---
  int top_index = FindTopStudent(course, student_count);

  // Check if valid pointer returned
  if (top_index > 3 || top_index < 0) {
    printf("FAIL: FindTopStudent returned index out of bounds.\n");
  }
  // Check if it points to the correct student (Bob)
  else if (strcmp(course[top_index].name, "Bob") == 0) {
    printf("PASS: FindTopStudent identified the correct top student (Bob).\n");
    score += 40;
  } else {
    printf("FAIL: FindTopStudent returned %s (Expected Bob).\n",
           course[top_index].name);
  }

  // Cleanup
  free(course);

  // Output final score for the bash script
  printf("RESULT: %d\n", score);
  return 0;
}