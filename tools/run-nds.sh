#!/bin/sh

die() {
    echo >&2 "$@"
    exit 1
}

# make the fat system
# create a massive image for use with desmume --cflash option
# might have to dldi patch with gbamp?

DI=desmume_disk.image
if [ ! -e $DI ] ; then
    mount_point=mp

    mkdir -p mp

    dd if=/dev/zero of=$DI bs=20000 count=128
    /sbin/mkdosfs -F16 $DI

fi

desmume-cli "$@" --cflash-image=$DI port/ds/chaos.nds
