#!/bin/sh

opts="--foreign-user --omit-header"
# skip header + none lines
lines=$(expr $(wc -l data/spelldata.csv | cut -d ' ' -f1) - 2)
xgettext -k_ -kT chaos/*c
tail -n$lines data/spelldata.csv | cut -d',' -f1 | tr '\n' ',' | xgettext $opts -Lc -a -ospellnames.po -
tail -n$lines data/spelldata.csv | cut -d',' -f15 | tr '\n' ',' | xgettext $opts -Lc -a -odesc.po -
echo >> messages.po
echo >> spellnames.po

cat messages.po spellnames.po desc.po > chaos.pot
