#!/bin/sh

die() {
    echo "$@"
    exit 1
}

source=$(dirname $(dirname $(readlink -f $0)))

# path to git-svn checkout
VERSION=$(grep versionName $source/port/android/AndroidManifest.xml |sed -r 's/.*versionName="(.*)".*/\1/g')
checkout=/tmp/chaos-$VERSION
mkdir -p $checkout

tmpfile=$(mktemp)

git archive --format=tar HEAD > $tmpfile

cd $checkout || die "Unable to change directories"
rm * -rf || die "Cannot remove all files"
tar --exclude=tools/push_to_googlecode.sh --exclude=.gitmodules --exclude=.ditz-config --exclude=bugs -xf $tmpfile

tar -C /tmp -czf $source/chaos-src-$VERSION.tar.gz chaos-$VERSION
