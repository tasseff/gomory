#!/bin/bash

CURRENT_DIR="$(pwd)"
CURRENT_TIME=$(date +%s)
FILE1=$1

./../cmake-build-debug/output/gmi_naive_mixed temp.json > $1/naive_mixed.txt
./../cmake-build-debug/output/gmi_rounds_mixed temp.json > $1/rounds_mixed.txt
./../cmake-build-debug/output/gmi_lex_mixed temp.json > $1/lex._mixedtxt
./../cmake-build-debug/output/gmi_rounds_lex_mixed temp.json > $1/rounds_lex_mixed.txt