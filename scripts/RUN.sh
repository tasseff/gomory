#!/bin/bash

CURRENT_DIR="$(pwd)"
CURRENT_TIME=$(date +%s)
FILE1=$1

../build/output/gmi_naive ${1}/temp.json > $1/naive.txt
../build/output/gmi_rounds ${1}/temp.json > $1/rounds.txt
../build/output/gmi_lex ${1}/temp.json > $1/lex.txt
../build/output/gmi_rounds_lex ${1}/temp.json > $1/rounds_lex.txt
