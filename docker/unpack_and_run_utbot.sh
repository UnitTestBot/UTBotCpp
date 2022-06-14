#!/bin/bash

#This script unpacks and runs UTBot server
tar -xvf utbot_distr.tar.gz
cd utbot_distr
chmod +x *.sh
./utbot_server_restart.sh
