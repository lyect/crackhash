#!/bin/bash

cd "$(dirname "$0")"
SCRIPT_DIR="$(pwd)"

######################################
#                                    #
#   INITIALIZE MONGODB REPLICA SET   #
#                                    #
######################################

# Create nodes' directories
mkdir -p $SCRIPT_DIR/test/mongors/data1
mkdir -p $SCRIPT_DIR/test/mongors/data2
mkdir -p $SCRIPT_DIR/test/mongors/data3
mkdir -p $SCRIPT_DIR/test/mongors/config1
mkdir -p $SCRIPT_DIR/test/mongors/config2
mkdir -p $SCRIPT_DIR/test/mongors/config3

##########################
#                        #
#   INITIALIZE MANAGER   #
#                        #
##########################

# Clear directories
mkdir -p $SCRIPT_DIR/test/manager
sudo rm -rf $SCRIPT_DIR/test/manager/*

# Copy files
cp -r $SCRIPT_DIR/src                   $SCRIPT_DIR/test/manager/
cp    $SCRIPT_DIR/manager-init.sh       $SCRIPT_DIR/test/manager/
cp    $SCRIPT_DIR/manager-config.json   $SCRIPT_DIR/test/manager/

##########################
#                        #
#   INITIALIZE WORKERS   #
#                        #
##########################

# Clear directories
mkdir -p $SCRIPT_DIR/test/worker1
mkdir -p $SCRIPT_DIR/test/worker2

sudo rm -rf $SCRIPT_DIR/test/worker1/*
sudo rm -rf $SCRIPT_DIR/test/worker2/*

# Copy files
cp -r $SCRIPT_DIR/src                  $SCRIPT_DIR/test/worker1/
cp    $SCRIPT_DIR/worker-init.sh       $SCRIPT_DIR/test/worker1/
cp    $SCRIPT_DIR/worker-config.json   $SCRIPT_DIR/test/worker1/

# Copy files
cp -r $SCRIPT_DIR/src                  $SCRIPT_DIR/test/worker2/
cp    $SCRIPT_DIR/worker-init.sh       $SCRIPT_DIR/test/worker2/
cp    $SCRIPT_DIR/worker-config.json   $SCRIPT_DIR/test/worker2/

cd $SCRIPT_DIR
docker-compose up --force-recreate