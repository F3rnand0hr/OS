#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

typedef struct {
  char name[50];
  float price;
  int quantity;
} Item;

/**
 * @brief Allocates memory for the inventory array.
 * @param[in] size Number of items to store.
 * @return Pointer to the allocated Item array.
 */
Item* CreateInventory(int size);

/**
 * @brief Adds an item to a specific slot in the inventory.
 * @param[in] inv Pointer to the inventory array.
 * @param[in] index Index in the array.
 * @param[in] name Name of the item.
 * @param[in] price Price per unit.
 * @param[in] quantity Number of units.
 */
void AddItem(Item* inv, int index, char* name, float price, int quantity);

/**
 * @brief Calculates the total monetary value of the inventory.
 * @param[in] inv Pointer to the inventory array.
 * @param[in] size Number of items in the array.
 * @return Total value (sum of price * quantity for all items).
 */
float CalculateTotalValue(Item* inv, int size);

#endif  // INCLUDE_FUNCTIONS_H_