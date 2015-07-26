#!/bin/bash

ARCH="mv"
HOST=arm-mv5sft-linux-gnueabi
MODE=$1

DIR=`cat ./info`

case "$MODE" in
    "x86_64" | "host" )
        ARCH="x86_64"
        HOST=x86_64-linux-gnu-
    ;;

    "mv" | "smg1016m" | "smgm")
        ARCH="mv"
        HOST=arm-mv5sft-linux-gnueabi-
    ;;
    "ppc" | "smg1016" | "smg" )
        ARCH="ppc"
        #HOST=powerpc-440-linux-gnu-
        HOST=ppc_4xxFP-
    ;;
    "2016" | "smg2016" )
        ARCH="2016"
        HOST=arm-marvell-linux-gnueabi-

        if [ "${DIR}" != "openssl-1.0.1e" ]; then
            DIR="openssl-1.0.1e"
        fi;
    ;;
    "smg4" )
        ARCH="smg4"
        HOST=arm-marvell-linux-gnueabi-

        if [ "${DIR}" != "openssl-1.0.1e" ]; then
            DIR="openssl-1.0.1e"
        fi;
    ;;
    * )
        echo "ERROR! Target plaform not specified! Use 'mv' by default"
    ;;
esac


INSTALL_PATH=`pwd`/$ARCH

[ ! -e ./info ] && echo "ERROR! no info-file found!" && exit 1

cd ./$DIR

export CROSS_COMPILE=$HOST

make
make install_sw INSTALL_PREFIX=$INSTALL_PATH

cd ..

#if [[ ( -d ./${ARCH} ) && ( -d ./${ARCH}/usr) ]]; then
#    cd ./${ARCH}
#    ln -s ./usr/lib ./lib
#    ln -s ./usr/include ./include
#fi;

exit 0;
