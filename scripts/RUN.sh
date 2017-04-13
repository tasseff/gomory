#!/bin/bash

CURRENT_DIR="$(pwd)"
CURRENT_TIME=$(date +%s)
FILE1=$1

./../cmake-build-debug/output/gmi ${1}/rounds.json > $1/rounds.txt
./../cmake-build-debug/output/gmi ${1}/purging.json > $1/purging.txt
./../cmake-build-debug/output/gmi ${1}/lex.json > $1/lex.txt
./../cmake-build-debug/output/gmi ${1}/rounds_purging.json > $1/rounds_purging.txt
./../cmake-build-debug/output/gmi ${1}/lex_rounds.json > $1/lex_rounds.txt
./../cmake-build-debug/output/gmi ${1}/lex_purging.json > $1/lex_purging.txt
./../cmake-build-debug/output/gmi ${1}/lex_rounds_purging.json > $1/lex_rounds_purging.txt
