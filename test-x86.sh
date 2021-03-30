#!/usr/bin/env bash

set -e

blasm2x86 -I lib examples/halt.blasm > /tmp/halt.basm.asm
nasm -felf64 /tmp/halt.basm.asm
ld -o /tmp/halt /tmp/halt.basm.o
