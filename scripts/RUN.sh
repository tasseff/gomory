#!/bin/bash

CURRENT_DIR="$(pwd)"
CURRENT_TIME=$(date +%s)
FILE1=$1

./../cmake-build-debug/output/gmi_rounds ${1}/temp.json > $1/rounds.txt
./../cmake-build-debug/output/gmi_lex ${1}/temp.json > $1/lex.txt
./../cmake-build-debug/output/gmi_rounds_lex ${1}/temp.json > $1/rounds_lex.txt
