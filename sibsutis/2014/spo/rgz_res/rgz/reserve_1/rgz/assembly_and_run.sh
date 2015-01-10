#!/bin/bash
echo "./analyzer $1.t $1.asm"
./analyzer $1.t $1.asm
echo "nasm -f elf $1.asm"
nasm -f elf $1.asm
echo "gcc $1.o -o run_me"
gcc $1.o -o $1