#!/bin/bash
top=$(dirname $(dirname $(readlink -f $0)))
project=chaos
VERSION=$(grep versionName "$top/port/android/AndroidManifest.xml" |sed -r 's/.*versionName="(.*)".*/\1/g')
SRCDIST=no

die() {
  echo >&2 "$@"
  exit 1
}

TEMP=`getopt -o shb:v: --long source,build:,version:,help -- "$@"`

if [ $? != 0 ] ; then
  echo "Try '$0 --help' for more information"
  exit 1
fi

eval set -- "$TEMP"
BUILDDIR=None

while true ; do
  case $1 in
    -v|--ve|--ver|--vers|--versi|--versio|--version ) VERSION=$2 ; shift 2 ;;
    -s|--so|--sou|--sour|--sourc|--source ) SRCDIST="yes" ; shift ;;
    -b|--bu|--bui|--buil|--build) BUILDDIR=$2 ; shift 2 ;;
    -h|--help )
    echo "Create binary and source distribution files."
    echo ""
    echo "Usage: $(basename $0) [OPTION]... "
    echo "Options available are:"
    echo "-v, --version=VERSION    Set the distro version number"
    echo "-s, --source             Create a source distro too"
    echo "-b, --build=BUILD        Path to the build directory"
    echo "-h, --help               This message."
    exit 0
    ;;
    --) shift ;  break ;;
    *) echo "Internal error! " >&2 ; exit 1 ;;
  esac
done

create_bin_dist() {
  mkdir $project-$VERSION || die "Unable to mkdir $project-$VERSION"
  cp "$@" $project-$VERSION/
  zip -r $zipname $project-$VERSION
  rm -rf $project-$VERSION/
}

if test "$BUILDDIR" != None ; then
  test -e $BUILDDIR/gba/port/gba/chaos.gba && cp -av $BUILDDIR/gba/port/gba/chaos.gba bin
  test -e $BUILDDIR/nds/port/ds/chaos.nds && cp -av $BUILDDIR/nds/port/ds/chaos.nds bin
  test -e $BUILDDIR/android/port/android/bin/Chaos-release.apk && cp -av $BUILDDIR/android/port/android/bin/Chaos-release.apk bin/Chaos.apk
  test -e $BUILDDIR/js/port/linux/chaos.html && cp -av $BUILDDIR/js/port/linux/chaos.{html,data} bin
fi

bin="$top/bin"
zipname=$project-ds-$VERSION.zip
create_bin_dist "$bin/chaos.nds"
zipname=$project-gba-$VERSION.zip
create_bin_dist "$bin/chaos.gba"
zipname=$project-android-$VERSION.zip
create_bin_dist "$bin/Chaos.apk"

src=$project-src-$VERSION
if [ $SRCDIST = yes ] ; then
  $top/tools/create_source_tar.sh
  echo "Created $src.tar.gz"
fi
