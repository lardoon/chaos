#!/bin/sh

if [ $# -ne 1 ] ; then
    echo "Usage: $0 IN.pcx"
    exit 1
fi
in=$1
out=$(basename $in .pcx)

convert -crop 16x16 +repage $in ${out}%02d.gif

convert  -page +0+0 -delay 50   -loop 0 ${out}0[0-3].gif  ${out}.gif

sed "s/anim.gif/${out}.gif/g" anim.html > $out.html

echo "Created file://$(pwd)/$out.html"
