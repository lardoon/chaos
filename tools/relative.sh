#!/bin/sh

# returns $2 relative to $1
absolute=$(readlink -f "$2")
current=$(readlink -f "$1")

perl -MFile::Spec -e 'print File::Spec->abs2rel("'"$absolute"'","'"$current"'")'
