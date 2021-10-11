/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

char *passthrough(__attribute__((align_value(0x8000))) char *x) {
  return x;
}
