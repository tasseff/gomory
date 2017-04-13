#!/bin/bash

CURRENT_DIR="$(pwd)"
CURRENT_TIME=$(date +%s)
FILE1=$1

./../cmake-build-debug/output/gmi ${1}/naive.json > $1/naive.txt
