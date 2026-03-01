#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../include/functions.h"

static float Average3(const Student* s) {
  return (s->grades[0] + s->grades[1] + s->grades[2]) / 3.0f;
}

static int ParseNumStudents(const char* s, int* out_n) {
  if (!s || !out_n) return 0;

  char* end = NULL;
  int64_t n64 = (int64_t)strtoll(s, &end, 10);
  if (!end || *end != '\0') return 0;
  if (n64 <= 0 || n64 > 1000000) return 0;

  *out_n = (int)n64;
  return 1;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <num_students>\n", argv[0]);
    return 2;
  }

  int n = 0;
  if (!ParseNumStudents(argv[1], &n)) {
    fprintf(stderr, "Invalid num_students: %s\n", argv[1]);
    return 2;
  }

  Student* course = CreateCourse(n);
  if (!course) {
    fprintf(stderr, "Failed to allocate course for %d students.\n", n);
    return 1;
  }

  for (int i = 0; i < n; i++) {
    char name[50] = {0};
    float g1 = 0.0f, g2 = 0.0f, g3 = 0.0f;

    printf("Student %d name: ", i + 1);
    if (scanf("%49s", name) != 1) {
      fprintf(stderr, "\nFailed to read student name.\n");
      free(course);
      return 1;
    }

    printf("Student %d grades (g1 g2 g3): ", i + 1);
    if (scanf("%f %f %f", &g1, &g2, &g3) != 3) {
      fprintf(stderr, "\nFailed to read 3 grades.\n");
      free(course);
      return 1;
    }

    AddStudent(&course[i], name, g1, g2, g3);
  }

  int top = FindTopStudent(course, n);
  if (top < 0) {
    fprintf(stderr, "Could not determine top student.\n");
    free(course);
    return 1;
  }

  printf("\nTop student: %s\n", course[top].name);
  printf("Average: %.2f\n", Average3(&course[top]));

  free(course);
  return 0;
}