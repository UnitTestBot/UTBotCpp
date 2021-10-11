/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "multi_arrays.h"

#include <string.h>

struct MyStructMult {
    int a[2][3][2];
};

struct IntArray {
    int ints[2][5];
};

struct PointStruct {
    int x;
    int y;
};


int kek(int a[1][2]) {
    return 1;
}


int sumSign(int a[2][2]) {
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            sum += a[i][j];
        }
    }
    if (sum == 0) {
        return 0;
    } else if (sum > 0) {
        return 1;
    } else {
        return -1;
    }
}

int value(int a[2][3]) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            if (a[i][j] > 0) {
                return 3 * i + j;
            }
        }
    }
    return -1;
}

int value2(int (* a)[3]) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            if (a[i][j] > 0) {
                return 3 * i + j;
            }
        }
    }
    return -1;
}


int some_method(int ** pointer2d) {
    int x = 2;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if (pointer2d[i][j] > 0) {
                return i * 2 + j;
            }
        }
    }
    return -1;
}

int return_sign_sum(struct MyStructMult st) {
    int res = 0;
    for (int i = 0; i < 2; i++) {
        for(int j = 0; j < 3; j++) {
            for (int k = 0; k < 2; k++) {
                res += st.a[i][j][k];
            }
        }  
    }
    
    if (res > 0) {
        return 1;
    } else if (res == 0) {
        return 0;
    } else {
        return -1;
    }
}

long long return_sign_sum_of_struct_array(struct PointStruct arr[2][2]) {
    long long sumDiffs = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if (arr[i][j].x > 0) {
                sumDiffs += arr[i][j].x;
            } else {
                 sumDiffs += -arr[i][j].x;
            }

            if (arr[i][j].y > 0) {
                sumDiffs += arr[i][j].y;
            } else {
                sumDiffs += -arr[i][j].y;
            }
        }
    }
    return sumDiffs;
}

int point_quart(struct PointStruct ** point) {
    if ((**point).x > 0) {
         if ((**point).y > 0) {
             return 1;
         } else {
             return 4;
         }
    } else {
         if ((**point).y > 0) {
             return 2;
         } else {
             return 3;
         }
    }
}

struct IntArray return_struct_with_2d_array(int a) {
    if (a > 0) {
        struct IntArray st = {{{1,2,3,4,5}, {1,2,3,4,5}}};
        return st;
    } else if (a < 0) {
        struct IntArray st = {{{-1,-2,-3,-4,-5}, {-1,-2,-3,-4,-5}}};
        return st;
    } else {
        struct IntArray st = {{{0,0,0,0,0}, {0,0,0,0,0}}};
        return st;
    }
}

#define W 3
#define H 2
static int matrix_a[W][H];
static int matrix_b[W][H];
static int matrix_c[W][H];

int sum_matrix() {
    int sum = 0;
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < H; j++) {
            matrix_c[i][j] = matrix_a[i][j] + matrix_b[i][j];
            sum += matrix_c[i][j];
        }
    }
    if (sum < 0) {
        return sum;
    }
    if (sum == 0) {
        return 0;
    }
    return sum;
}

int argc;
char **argv;

int count_dashes() {
    char **cur = argv;
    int cnt = 0;
    while (argc--) {
        if (**cur == '-') {
            cnt++;
        }
        cur++;
    }
    if (cnt == 0) {
        return -1;
    }
    return cnt;
}