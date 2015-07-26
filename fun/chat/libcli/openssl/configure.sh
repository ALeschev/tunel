#!/bin/bash

[ ! -e ./info ] && echo "ERROR! no info-file found!" && exit 1

DIR=`cat ./info`

ARCH="mv"
HOST=arm-mv5sft-linux-gnueabi-
MODE=$1
OPTS=

case "$MODE" in
    "x86_64" | "host" )
        ARCH="x86_64"
        HOST=x86_64-linux-gnu-

        export MACHINE="x86_64"
        export RELEASE="3.13.0-35-generic"
        export SYSTEM="Linux"
        export BUILD="#62-Ubuntu SMP Fri Aug 15 01:58:42 UTC 2014"
    ;;
    "mv" | "smg1016m" | "smgm" )
        ARCH="mv"
        HOST=arm-mv5sft-linux-gnueabi-

        export MACHINE="armv5tejl"
        export RELEASE="2.6.22.18"
        export SYSTEM="Linux"
        export BUILD="#139 Mon Jun 10 15:41:46 NOVT 2013"
    ;;
    "ppc" | "smg1016" | "smg" )
        ARCH="ppc"
        #HOST=powerpc-440-linux-gnu-
        HOST=ppc_4xxFP-

        export MACHINE="ppc"
        export RELEASE="2.6.32-rc5"
        export SYSTEM="Linux"
        export BUILD="#289 Thu Nov 29 16:04:08 NOVT 2012"
    ;;
    "2016" | "smg2016" )
        ARCH="2016"
        HOST=arm-marvell-linux-gnueabi-
        OPTS="-march=armv7-a"

        if [ "${DIR}" != "openssl-1.0.1e" ]; then
            DIR="openssl-1.0.1e"
        fi;

        export MACHINE="armv7l"
        export RELEASE="3.2.27"
        export SYSTEM="Linux"
        export BUILD="#2 SMP Thu Jul 11 10:25:19 NOVT 2013"
    ;;
    "smg4" )
        ARCH="smg4"
        HOST=arm-marvell-linux-gnueabi-
        OPTS="-march=armv7-a"

        if [ "${DIR}" != "openssl-1.0.1e" ]; then
            DIR="openssl-1.0.1e"
        fi;

        export MACHINE="armv7l"
        export RELEASE="3.2.52"
        export SYSTEM="Linux"
        export BUILD="#2 SMP Thu Jul 11 10:25:19 NOVT 2013"
    ;;

    * )
        echo "ERROR! Target plaform not specified! Use 'mv' by default"

        export MACHINE="armv5tejl"
        export RELEASE="2.6.22.18"
        export SYSTEM="Linux"
        export BUILD="#139 Mon Jun 10 15:41:46 NOVT 2013"
    ;;
esac

INSTALL_PATH=`pwd`/$ARCH

echo "entering $DIR"
cd ./$DIR

[ -e ./Makefile ] && make clean

export CROSS_COMPILE=$HOST

./config shared

#./config --openssldir="/openssl" shared

#./Configure --openssldir=${INSTALL_PATH} --cross-compile-prefix=$HOST no-asm linux-armv5 $OPTS

exit 0;
