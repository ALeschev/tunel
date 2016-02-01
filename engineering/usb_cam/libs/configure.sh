#!/bin/bash

[ ! -e ./info ] && echo "ERROR! no info-file found!" && exit 1

DIR=`cat ./info`

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

cd ./$DIR

./configure --host=$HOST
# --prefix=${INSTALL_PATH}
#--with-termlib \
#--enable-termcap

#sed -i 's/\/home\/bokrenok\/work\/projects\/smg\/srcgit\/sigtran-gw\/env\/smg-env\/smg1016m\/packages\/ncurses\/mv\/share\/terminfo/\/share\/terminfo/' Makefile
#sed -i 's/\/home\/bokrenok\/work\/projects\/smg\/srcgit\/sigtran-gw\/env\/smg-env\/smg1016m\/packages\/ncurses\/mv\/share\/terminfo/\/share\/terminfo/' include/ncurses_cfg.h