#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

/**
 * @brief Performs an arithmetic operation on two numbers.
 * @param[in] n1 First operand.
 * @param[in] n2 Second operand.
 * @param[in] op The operator character (+, -, x, /, ^).
 * @param[out] result Pointer to store the calculation result.
 * @return 0 if successful, -1 if an error occurred (div by zero, invalid op).
 */
int Calculate(float n1, float n2, char op, float* result);

#endif  // INCLUDE_FUNCTIONS_H_