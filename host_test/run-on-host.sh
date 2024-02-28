#!/bin/sh

idf.py --preview set-target linux && \
idf.py build && \
rm -f test_buffer.dat && \
clear && \
./build/host_test.elf