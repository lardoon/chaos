#!/bin/sh -x

DEVKITPRO=$(pwd)/devkitPro
mkdir -p $DEVKITPRO
WGET=wget --timeout=30

die() {
  echo >&2 "$@"
  exit 1
}

g() {
    python <<EOF
import ConfigParser
c = ConfigParser.ConfigParser()
c.readfp(open("$DEVKITPRO/" + 'devkitProUpdate.ini'))
print c.get("$1", "$2")
EOF
}

url="http://devkitpro.sourceforge.net/devkitProUpdate.ini"
$WGET $url || die "Unable to download devkitProUpdate.ini"

url=$(g devkitProUpdate URL)
platform=$(uname -m)-linux.tar.bz2

for item in devkitARM libnds maxmodds defaultarm7 libgba maxmodgba
do
    dl=$(g $item File | sed "s/win32\.exe/$platform/g")
    v=$(g $item Version)
    if [ -e $dl ] ; then
        echo "Skipping $item $v. Already installed"
        continue
    fi
    $WGET $url/$dl

    case $item in
        devkitARM)
            touch $item-$v
            tar xf $dl
            ;;
        libnds|libndsfat|maxmodds|dswifi|filesystem|defaultarm7)
            touch $item-$v
            mkdir -p libnds
            cd libnds
            tar xf ../$dl
            cd ..
            ;;
        libgba|maxmodgba)
            touch $item-$v
            mkdir -p libgba
            cd libgba
            tar xf ../$dl
            cd ..
            ;;
    esac
done

# download android deps TODO(richq) test this works
wget http://dl.google.com/android/android-sdk_r21-linux.tgz
wget http://dl.google.com/android/android-ndk_r8c-linux.tgz

tar -xf android-sdk_r21-linux.tgz
tar -xf android-ndk_r8c-linux.tgz

./android-sdk-linux/tools/android update --no-ui --filter android-8,platform

# outside this script, set DEVKITPRO, DEVKITARM, ANDROID_NDK, and ANDROID_SDK
echo export DEVKITPRO=$(pwd)/devkitPro
echo 'export DEVKITARM=$DEVKITPRO/devkitARM'
echo export ANDROID_NDK=$(pwd)/android-ndk-r8c
echo export ANDROID_SDK=$(pwd)/android-sdk-linux
