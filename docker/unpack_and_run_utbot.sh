#!/bin/bash

#
# Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
#

#This script unpacks and runs UTBot server
tar -xvf utbot_distr.tar.gz
cd utbot_distr
chmod +x *.sh
./utbot_server_restart.sh