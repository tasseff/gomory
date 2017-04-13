#!/bin/bash

CURRENT_DIR="$(pwd)"
CURRENT_TIME=$(date +%s)
FILE1=$1
FILE2=$2

./../cmake-build-debug/output/gmi naive.json $1 solution.sol > $2/naive.txt
./../cmake-build-debug/output/gmi rounds.json $1 solution.sol > $2/rounds.txt
./../cmake-build-debug/output/gmi purging.json $1 solution.sol > $2/purging.txt
./../cmake-build-debug/output/gmi lex.json $1 solution.sol > $2/lex.txt
./../cmake-build-debug/output/gmi rounds_purging.json $1 solution.sol > $2/rounds_purging.txt
./../cmake-build-debug/output/gmi lex_rounds.json $1 solution.sol > $2/lex_rounds.txt
./../cmake-build-debug/output/gmi lex_purging.json $1 solution.sol > $2/lex_purging.txt
./../cmake-build-debug/output/gmi lex_rounds_purging.json $1 solution.sol > $2/lex_rounds_purging.txt
