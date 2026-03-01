#include <stdlib.h>
#include <string.h>

#include "../include/functions.h"

static float Average3(const Student* s) {
  return (s->grades[0] + s->grades[1] + s->grades[2]) / 3.0f;
}

Student* CreateCourse(int n) {
  if (n <= 0) return NULL;
  return (Student*)calloc((size_t)n, sizeof(Student));
}

void AddStudent(Student* s, char* name, float g1, float g2, float g3) {
  if (!s) return;

  if (name) {
    strncpy(s->name, name, sizeof(s->name) - 1);
    s->name[sizeof(s->name) - 1] = '\0';
  } else {
    s->name[0] = '\0';
  }

  s->grades[0] = g1;
  s->grades[1] = g2;
  s->grades[2] = g3;
}

int FindTopStudent(Student* student_list, int count) {
  if (!student_list || count <= 0) return -1;

  int top_idx = 0;
  float top_avg = Average3(&student_list[0]);

  for (int i = 1; i < count; i++) {
    float avg = Average3(&student_list[i]);
    if (avg > top_avg) {
      top_avg = avg;
      top_idx = i;
    }
  }

  return top_idx;
}
