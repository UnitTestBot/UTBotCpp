#include "linked_list.h"

int length_of_linked_list3(struct Node *head) {
    if (head == NULL) {
        return 0;
    }
    if (head->next == NULL) {
        return 1;
    }
    if (head->next->next == NULL) {
        return 2;
    }
    if (head->next->next->next == NULL) {
        return 3;
    }
    return -1;
}

int length_of_linked_list2(struct Node *head) {
    if (head == NULL) {
        return 0;
    }
    if (head->next == NULL) {
        return 1;
    }
    if (head->next->next == NULL) {
        return 2;
    }
    return -1;
}

int hard_length2(struct HardNode *head) {
    if (head == NULL) {
        return 0;
    }
    if (head->in.superIn.next == NULL) {
        return 1;
    }
    if (head->in.superIn.next->in.superIn.next == NULL) {
        return 2;
    }
    return -1;
}

int middle_length2(struct Kuku *head) {
    if (head == NULL) {
        return 0;
    }
    if (head->in.next == NULL) {
        return 1;
    }
    if (head->in.next->in.next == NULL) {
        return 2;
    }
    return -1;
}

int cycle_list3(struct Node *head) {
    if (head == NULL) {
        return 0;
    }
    if (head->next == head) {
        return 1;
    }
    if (head->next == NULL) {
        return -1;
    }
    if (head->next->next == head->next) {
        return 2;
    }
    if (head->next->next == head) {
        return 3;
    }
    if (head->next->next == NULL) {
        return -2;
    }
    if (head->next->next->next == head->next->next) {
        return 4;
    }
    if (head->next->next->next == head->next) {
        return 5;
    }
    if (head->next->next->next == head) {
        return 6;
    }
    if (head->next->next->next == NULL) {
        return -3;
    }
}