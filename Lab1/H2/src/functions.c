#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/functions.h"

int Calculate(float n1, float n2, char op, float* result) {
    switch (op) {
        case '+':
            *result = n1 + n2;
            break;
        case '-':
            *result = n1 - n2;
            break;
        case 'x':
        case '*':
            *result = n1 * n2;
            break;
        case '/':
            if (n2 == 0.0f) {
                return -1;
            }
            *result = n1 / n2;
            break;
        case '^':
            *result = pow(n1, n2);
            break;
        default:
            return -1;
    }
    return 0;
}
