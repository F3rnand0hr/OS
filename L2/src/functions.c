#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <functions.h>

Item* CreateInventory(int size) {
    return (Item*)malloc(size * sizeof(Item));
}

void AddItem(Item* inv, int index, char* name, float price, int quantity) {
    strncpy(inv[index].name, name, sizeof(inv[index].name) - 1);
    inv[index].name[sizeof(inv[index].name) - 1] = '\0';
    inv[index].price = price;
    inv[index].quantity = quantity;
}

float CalculateTotalValue(Item* inv, int size) {
    float total = 0.0f;
    for (int i = 0; i < size; i++) {
        total += inv[i].price * inv[i].quantity;
    }
    return total;
}