#include <stdio.h>
#include <stdlib.h>
#include <functions.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <count>\n", argv[0]);
        return 1;
    }

    int count = atoi(argv[1]);
    Item* inventory = CreateInventory(count);

    if (inventory == NULL) {
        fprintf(stderr, "Failed to create inventory\n");
        return 1;
    }

    for (int i = 0; i < count; i++) {
        char name[50];
        float price;
        int quantity;

        if (scanf("%49s %f %d", name, &price, &quantity) != 3) {
            fprintf(stderr, "Failed to read item data\n");
            free(inventory);
            return 1;
        }

        AddItem(inventory, i, name, price, quantity);
    }

    float total = CalculateTotalValue(inventory, count);
    printf("%.2f\n", total);

    free(inventory);
    return 0;
}