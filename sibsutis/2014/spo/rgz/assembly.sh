#!/bin/bash
./analyzer $1.t $1.asm
nasm -f elf $1.asm
gcc $1.o -o $1