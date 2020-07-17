#!/bin/bash

IN=$1
OUT=$2

# read in in and spit out an array of strings
# if it is a pot file, use msgid
# if it is a po file, use msgstr

msgtype=msgstr
if echo $IN | grep '\.pot' >/dev/null ; then
    msgtype=msgid
fi

char2cstr() {
    printf "$1" |xxd | cut -f2 -d' ' | sed 's/../\\x&/g'
}

aacute=$(char2cstr 'á')
eacute=$(char2cstr 'é')
iacute=$(char2cstr 'í')
oacute=$(char2cstr 'ó')
aacutecaps=$(char2cstr 'Á')
eacutecaps=$(char2cstr 'É')
iacutecaps=$(char2cstr 'Í')
oacutecaps=$(char2cstr 'Ó')
nye=$(char2cstr 'ñ')
upsidedownqm=$(char2cstr '¿')

echo "const char * translation_$(basename "$IN" | sed 's/\./_/g')[] = {"
IFS=$'\n'
for msg in $(grep $msgtype $IN | sed 's/^msg[^ ]\+ \(.*\)/\1/g') ; do


    escaped=$(echo "$msg" |
    sed -e "s/$aacute/\\\\x7c\"\"/g" \
        -e "s/$eacute/\\\\x7d\"\"/g" \
        -e "s/$iacute/\\\\x7e\"\"/g" \
        -e "s/$oacute/\\\\x7f\"\"/g" \
        -e "s/$aacutecaps/\\\\x80\"\"/g" \
        -e "s/$eacutecaps/\\\\x81\"\"/g" \
        -e "s/$iacutecaps/\\\\x82\"\"/g" \
        -e "s/$oacutecaps/\\\\x83\"\"/g" \
        -e "s/$nye/\\\\x84\"\"/g" \
        -e "s/$upsidedownqm/\\\\x85\"\"/g"
    )
    printf '\t%s,\n' "$escaped"
done
printf '\t0,\n'
echo "};"

