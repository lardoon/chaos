#!/bin/sh

PROGDIR=$(dirname $(dirname $(readlink -f $0)))
cd $PROGDIR

xgettext --no-location -k_ -kT chaos/*c $(find port -name '*.c')
cut -d',' -f1 data/spelldata.csv | tr '\n' ',' | xgettext --omit-header --no-location -Lc -a -ospellnames.po -
cut -d',' -f16 data/spelldata.csv | tr '\n' ',' | xgettext --omit-header --no-location -Lc -a -odesc.po -
cat messages.po spellnames.po desc.po > chaos.pot
vi -d chaos.pot lang/chaos.pot
