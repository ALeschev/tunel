#!/bin/bash

TOP_DIR=/tmp/disk/top_checker

mkdir -p $TOP_DIR/

while [ 1 ]
do
    TIME=`date +%X`
    top -n1 >> $TOP_DIR/top_stat_$TIME

    sleep 1800
done
