#!/usr/bin/env bash

set -e

blasm2nasm -I lib $1 > $(basename $1).asm
nasm -g -wall -felf64 $(basename $1).asm
ld -o $(basename ${1%.*}) $(basename $1).o
