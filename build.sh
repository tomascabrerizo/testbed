#!/bin/bash

if [ ! -d "./build" ]
then
  mkdir ./build
fi

gcc -std=c99 -Wall -Wextra -Werror -I./ *.c test/docker.c -g -o ./build/testbed -lm -lX11 -lGL
