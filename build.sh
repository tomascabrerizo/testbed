#!/bin/bash

if [ ! -d "./build" ]
then
  mkdir ./build
fi

gcc -std=c89 -Wall -Wextra -Werror *.c -g -o ./build/testbed -lX11 -lGL
