#!/bin/bash

HOME_DIR=/home

cd $HOME_DIR

mkdir -p $HOME_DIR/build
cd $HOME_DIR/build
cmake $HOME_DIR/src
cmake --build . -- -j8

cd $HOME_DIR
cp $HOME_DIR/build/worker/worker $HOME_DIR
