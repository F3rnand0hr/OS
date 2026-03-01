#include <stdio.h>
#include <stdlib.h>
#include "../include/functions.h"

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Error: Invalid number of arguments.\n");
        return 1;
    }

    Point p1 = {atof(argv[1]), atof(argv[2])};
    Point p2 = {atof(argv[3]), atof(argv[4])};

    float distance = CalculateDistance(p1, p2);
    printf("Distance: %.2f\n", distance);
    return 0;
}