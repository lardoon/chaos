#!/bin/sh

echo "Create a build directory at the same level as the project"

if [ "$1" = "--dry-run" ] ; then
    DRYRUN=echo
else
    DRYRUN=
fi

PROGDIR=$(dirname $(dirname $(readlink -f $0)))
PROGDIR_NAME=$(basename $PROGDIR)
BUILD_ROOT=$PROGDIR/../$PROGDIR_NAME-build
mkdir -p $BUILD_ROOT
BUILD_ROOT=$(readlink -f $BUILD_ROOT)
for d in linux android gba nds
do
    $DRYRUN mkdir -p $BUILD_ROOT/$d
done

CONF=$PROGDIR/configure

$DRYRUN cd $BUILD_ROOT/linux
$DRYRUN $CONF

$DRYRUN cd $BUILD_ROOT/android
$DRYRUN $CONF --host=android

$DRYRUN cd $BUILD_ROOT/gba
$DRYRUN $CONF --host=arm-eabi --with-libgba=$DEVKITPRO/libgba

$DRYRUN cd $BUILD_ROOT/nds
$DRYRUN $CONF --host=arm-eabi --with-libnds=$DEVKITPRO/libnds
