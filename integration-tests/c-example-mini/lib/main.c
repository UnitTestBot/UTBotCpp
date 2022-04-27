/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

static int exit_code(int argc) {
    if (argc == 1) {
        return 0;
    }
    return 1;
}

int main2(int argc, char** argv) {
    return exit_code(argc);
} 

int main(int argc, char** argv) {
    return 0;
}
