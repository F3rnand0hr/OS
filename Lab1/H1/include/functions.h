#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

typedef struct {
  char name[50];
  float grades[3];
} Student;

/**
 * @brief Allocates memory for a specific number of students.
 * @param[in] n The number of students to allocate.
 * @return A pointer to the allocated array of Student structures.
 */
Student* CreateCourse(int n);

/**
 * @brief Initializes a student's data.
 * @param[out] s Pointer to the specific Student to populate.
 * @param[in] name Student name.
 * @param[in] g1 Grade 1, @param[in] g2 Grade 2, @param[in] g3 Grade 3.
 */
void AddStudent(Student* s, char* name, float g1, float g2, float g3);

/**
 * @brief Finds the student with the highest average in an array.
 * @param[in] student_list Array of students.
 * @param[in] count Number of students in the array.
 * @return The index of the student with the highest average.
 */
int FindTopStudent(Student* student_list, int count);

#endif  // INCLUDE_FUNCTIONS_H_