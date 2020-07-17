#!/bin/sh
# create build directories for all platforms, pass in suitable configure args

top=$(readlink -f $(dirname $(readlink -f "$0"))/..)

if test "x$ANDROID_NDK" = x ; then
    echo "ANDROID_NDK variable not set"
    exit 1
fi

if test "x$ANDROID_SDK" = x ; then
    echo "ANDROID_SDK variable not set"
    exit 1
fi

hasemcmake=$(which emcmake 2> /dev/null)
if [ "x$hasemcmake" = "x" ] ; then
    echo "No EMSCRIPTEN Cmake wrapper found in path. Try 'source emsdk_env.sh'?"
    exit 1
fi


if test "x$DEVKITPRO" = x ; then
    echo "DEVKITPRO variable not set"
    exit 1
fi

if test "x$DEVKITARM" = x ; then
    echo "DEVKITARM variable not set"
    exit 1
fi

mkdir -p android
cd android
env PATH="$PATH:$DEVKITARM/bin" cmake -DANDROID=1 \
    -DANDROID_NDK="$ANDROID_NDK" \
    -DANDROID_SDK="$ANDROID_SDK" "$top" || exit 1
cd ..

mkdir -p gba
cd gba
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE="$top/cmake/GBA.cmake" "$top" || exit 1
cd ..

mkdir -p nds
cd nds
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE="$top/cmake/NDS.cmake" "$top" || exit 1
cd ..

mkdir -p linux
cd linux
cmake -G Ninja "$top" || exit 1
cd ..

mkdir -p js
cd js || exit 1
emcmake cmake -G Ninja "$top" || exit 1
cd ..

cat >Makefile <<EOF
all:
	ninja -C gba
	ninja -C nds
	\$(MAKE) -C android
	ninja -C linux
	ninja -C js
EOF
