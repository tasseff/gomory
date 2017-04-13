#!/bin/bash

CURRENT_DIR="$(pwd)"
CURRENT_TIME=$(date +%s)
FILE1=$1
./../cmake-build-debug/output/gmi ${1}/naive_mixed.json > $1/naive_mixed.txt
./../cmake-build-debug/output/gmi ${1}/rounds_mixed.json > $1/rounds_mixed.txt
./../cmake-build-debug/output/gmi ${1}/purging_mixed.json > $1/purging_mixed.txt
./../cmake-build-debug/output/gmi ${1}/lex_mixed.json > $1/lex_mixed.txt
./../cmake-build-debug/output/gmi ${1}/rounds_purging_mixed.json > $1/rounds_purging_mixed.txt
./../cmake-build-debug/output/gmi ${1}/lex_rounds_mixed.json > $1/lex_rounds_mixed.txt
./../cmake-build-debug/output/gmi ${1}/lex_purging_mixed.json > $1/lex_purging_mixed.txt
./../cmake-build-debug/output/gmi ${1}/lex_rounds_purging_mixed.json > $1/lex_rounds_purging_mixed.txt
