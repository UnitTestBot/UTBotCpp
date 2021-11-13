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
    return 17;
}

int len_bound(struct Node *head, int bound) {
    int len = 0;
    while (head != NULL && bound > 0) {
        ++len;
        --bound;
        head = head->next;
    }
    return head != NULL ? -1 : len;
}

#define SIZE 4

int sort_list(struct Node *head) {
    int n = len_bound(head, SIZE);
    if (n == SIZE) {
        for (int i = 0; i < n - 2; i++) {
            struct Node *cop = head;
            while (cop->next != NULL) {
                if (cop->x > cop->next->x) {
                    int t = cop->x;
                    cop->x = cop->next->x;
                    cop->next->x = t;
                }
                cop = cop->next;
            }
        }
        int fl = 1;
        struct Node *cop = head;
        while (cop->next != NULL) {
            if (cop->x > cop->next->x) {
                fl = -1;
            }
            cop = cop->next;
        }
        if (fl == 1) {
            return 1;
        } else {
            return -1;
        }
    }
    return 0;
}
