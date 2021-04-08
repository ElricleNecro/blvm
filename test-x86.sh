#!/usr/bin/env bash

set -e

OUTDIR=build/Debug
BINDIR="${OUTDIR}/examples"
OBJDIR="build/obj/Debug/asm"

mkdir -p "${BINDIR}"
mkdir -p "${OBJDIR}"

blasm2nasm -I lib $1 > ${OUTDIR}/$(basename $1).asm
nasm -g -wall -felf64 ${OUTDIR}/$(basename $1).asm -o ${OBJDIR}/$(basename $1).o
ld -o ${BINDIR}/$(basename ${1%.*}) ${OBJDIR}/$(basename $1).o
