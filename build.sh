#!/bin/bash

CFLAGS="-O2" #"-O0 -g" #
gcc $CFLAGS -c main.c -o main.o \
&& gcc $CFLAGS -c skinny_fs.c -o skinny_fs.o \
&& gcc skinny_fs.o main.o -o test_skinny_fs
