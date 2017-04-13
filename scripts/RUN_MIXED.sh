#!/bin/bash

CURRENT_DIR="$(pwd)"
CURRENT_TIME=$(date +%s)
FILE1=$1
FILE2=$2

./../cmake-build-debug/output/gmi naive_mixed.json $1 solution.sol > $2/naive_mixed.txt
./../cmake-build-debug/output/gmi rounds_mixed.json $1 solution.sol > $2/rounds_mixed.txt
./../cmake-build-debug/output/gmi purging_mixed.json $1 solution.sol > $2/purging_mixed.txt
./../cmake-build-debug/output/gmi lex_mixed.json $1 solution.sol > $2/lex_mixed.txt
./../cmake-build-debug/output/gmi rounds_purging_mixed.json $1 solution.sol > $2/rounds_purging_mixed.txt
./../cmake-build-debug/output/gmi lex_rounds_mixed.json $1 solution.sol > $2/lex_rounds_mixed.txt
./../cmake-build-debug/output/gmi lex_purging_mixed.json $1 solution.sol > $2/lex_purging_mixed.txt
./../cmake-build-debug/output/gmi lex_rounds_purging_mixed.json $1 solution.sol > $2/lex_rounds_purging_mixed.txt
