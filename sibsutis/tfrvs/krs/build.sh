#!/bin/bash

echo "build..."

gcc -g zxc.c -o zxc -lm

if [ $? -eq 0 ]
then
    echo "Done"
else
    echo "Failed"
fi
