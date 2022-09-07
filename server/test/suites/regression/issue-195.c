#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node *next;
} Node;

int list_sum(Node *head) {
    int sum = 0;
    while (head != NULL) {
        sum += head->value;
        head = head->next;
    }
    return sum;
}
