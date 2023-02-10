#include "hard-linked-list.h"
#include <stddef.h>

int hard_list_and_pointers(struct Node *node) {
    if (node == NULL) {
        return 0;
    }
    if (node->x == NULL || node->data == NULL) {
        return -1;
    }
    if (node->x == &(node->data->x) && *node->x == node->data->x) {
        return 1;
    }
    if (node->next == NULL) {
        return -2;
    }
    if (node->next->x == NULL || node->next->data == NULL) {
        return -3;
    }
    if (node->next->data == node->data && &(node->next->data->y) == node->x && node->next->data->y == *node->x) {
        return 2;
    }
    return 3;
}
