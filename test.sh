#!/usr/bin/bash

set -e

make -C examples -B
for examp in build/Debug/examples/*
do
	BNAME="$(basename $examp)"
	EXPECTED_OUT="examples/tests/${BNAME%.*}.out"
	if [[ -f "${EXPECTED_OUT}" ]]
	then
		echo "--------------------------- Testing ${BNAME}"
		blvrec $examp -e "${EXPECTED_OUT}"
	fi
done
