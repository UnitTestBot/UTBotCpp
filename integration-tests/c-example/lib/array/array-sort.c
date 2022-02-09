/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "array-sort.h"

#define SIZE 4

int sort_array(int *arr, int n) {
    if (n >= SIZE) { 
        for (int i = 0; i < n - 2; i++) {
            for (int j = 0; j < n - 1; j++) {
                if (arr[j] > arr[j + 1]) {
                    int t = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = t;
                }
            }
        }

        int fl = 1;
        for (int i = 0; i < n - 1; i++) {
            if (arr[i] > arr[i + 1]) {
                fl = 0;
            }
        }

        if (fl) {
            return 1;
        } else {
            return -1;
        }
    }
    return 0;
}

int sort_array_with_comparator(int *arr, int n, int (*cmp) (int, int)) {
    if (n >= SIZE) { 
        for (int i = 0; i < n - 2; i++) {
            for (int j = 0; j < n - 1; j++) {
                if (!cmp(arr[j], arr[j + 1])) {
                    int t = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = t;
                }
            }
        }

        int fl = 1;
        for (int i = 0; i < n - 1; i++) {
            if (!cmp(arr[i], arr[i + 1])) {
                fl = 0;
            }
        }

        if (fl) {
            return 1;
        } else {
            return -1;
        }
    }
    return 0;
}
