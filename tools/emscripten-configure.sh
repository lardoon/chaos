#!/bin/sh

base=$(dirname $(dirname $(readlink -f $0)))

configure=$($base/tools/relative.sh "$(pwd)" "$base")/configure
~/src/emscripten/emscripten-master/emconfigure "$configure" \
--with-sdl-prefix=~/src/emscripten/emscripten-master/system/ \
--with-emscripten=true \
--disable-sdltest CFLAGS="-O2"
