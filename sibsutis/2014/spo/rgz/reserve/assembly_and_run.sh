#!/bin/bash
echo "nasm -f elf tst.asm"
nasm -f elf tst.asm
echo "gcc tst.o -o run_me"
gcc tst.o -o run_me
echo "./run_me"
./run_me