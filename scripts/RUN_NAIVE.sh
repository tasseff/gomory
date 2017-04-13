#!/bin/bash

CURRENT_DIR="$(pwd)"
CURRENT_TIME=$(date +%s)
FILE1=$1
FILE2=$2
./../cmake-build-debug/output/gmi naive.json $2 solution.sol > $1/naive.txt
