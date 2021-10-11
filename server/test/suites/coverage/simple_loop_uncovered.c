/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "simple_loop_uncovered.h"

int simple_loop_uncovered(unsigned int n) {
  int i = 0;
  while (i < n) {
    i = i + 1;
    if (i % n == 37)
      return 1;
    else if (i == 50)
      return 2;
  }
  return 0;
}