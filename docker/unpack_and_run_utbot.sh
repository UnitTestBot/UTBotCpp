#!/bin/bash

#This script unpacks and runs UTBot server
tar -xf utbot_distr.tar.gz
cd utbot_distr || exit
chmod +x *.sh
./utbot_server_restart.sh
