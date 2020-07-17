#!/bin/sh

# prepare a build env for android in the android-build directory

top=$(dirname $(dirname $(readlink -f $0)))
cd $top

# download devkitARM - this is needed for grit and bin2s
if [ ! -e tmp/devkitARM.tar.bz2 ] ; then
    dka=http://downloads.sourceforge.net/project/devkitpro/devkitARM/devkitARM_r41-i686-linux.tar.bz2
    mkdir -p tmp
    wget "$dka" -O tmp/devkitARM.tar.bz2
    tar -C tmp -xjf tmp/devkitARM.tar.bz2
fi

autoreconf -i

if [ $# -ne 2 ] ; then
    echo "prepare-clean-build.sh SDK NDK"
    exit 1
fi

mkdir android-build
cd android-build

../configure --host=android --with-grit=../tmp/devkitARM/bin/grit --with-bin2s=../tmp/devkitARM/bin/bin2s \
    --with-sdk=$1 \
    --with-ndk=$2
