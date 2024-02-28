#!/bin/sh

idf.py --preview set-target linux && \
idf.py build && \
clear && \
idf.py monitor