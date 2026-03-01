#include <stdio.h>
#include <stdlib.h>

#include "../include/functions.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        return 1;
    }

    float n1 = (float)atof(argv[1]);
    char op = argv[2][0];
    float n2 = (float)atof(argv[3]);

    float result = 0.0f;
    int rc = Calculate(n1, n2, op, &result);

    if (rc == 0) {
        printf("%.2f %c %.2f = %.2f\n", n1, op, n2, result);
        return 0;
    }

    if (op == '/' && n2 == 0.0f) {
        printf("Error: Division by zero\n");
    } else {
        printf("Error: Invalid operator.\n");
    }

    return 1;
}