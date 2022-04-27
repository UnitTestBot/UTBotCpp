/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

struct bpf_xdp_attach_opts {
    int sz;
    int : 0;
};

int bpf_xdp_attach(const struct bpf_xdp_attach_opts *opts) {
    return 0;
}