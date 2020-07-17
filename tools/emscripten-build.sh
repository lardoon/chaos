#!/bin/sh

base=$(dirname $(dirname $(readlink -f $0)))
FLAGS="--closure 1 -O2"
~/src/emscripten/emscripten-master/emmake make V=1 &&
    cp port/linux/chaos port/linux/chaos.bc &&
    ~/src/emscripten/emscripten-master/emcc $FLAGS port/linux/chaos.bc -o chaos.js -s EXPORTED_FUNCTIONS="['_main', '_InterruptProcess']" \
    --preload-file sfx --js-library $base/port/linux/library.js &&
   cat $base/port/linux/header.html chaos.js $base/port/linux/footer.html > chaos.html
