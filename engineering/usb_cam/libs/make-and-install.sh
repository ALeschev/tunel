#!/bin/bash

ARCH="mv"
HOST=arm-mv5sft-linux-gnueabi

if [ x"$1" = x"mv" ]; then
    ARCH="mv"
    HOST=arm-mv5sft-linux-gnueabi

elif [ x"$1" = x"ppc" ]; then
    ARCH="ppc"
    HOST=ppc_4xxFP
elif [ x"$1" = x"2016" ]; then
    ARCH="2016"
    HOST=arm-marvell-linux-gnueabi
elif [ x"$1" = x"smg4" ]; then
    ARCH="smg4"
    HOST=arm-marvell-linux-gnueabi
else
    echo "ERROR! Target plaform not specified! Use 'mv' by default"
fi

INSTALL_PATH=`pwd`/$ARCH


[ ! -e ./info ] && echo "ERROR! no info-file found!" && exit 1

DIR=`cat ./info`

cd ./$DIR

make -j8
make install DESTDIR=$INSTALL_PATH
