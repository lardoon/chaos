#!/bin/bash

# Run semi-automatic tests
# These are pre-recorded saved games in critical situations
# Follow the instructions to test things.
print_usage() {
    echo "Usage: run_semi_auto_tests.sh [OPTIONS] EXECUTABLE TESTS"
    echo
    echo "OPTIONS:"
    echo " -m,--manual        do not autorun tests (manual mode)"
    echo " -h,--help          print this message and exit"
    echo
    echo "EXAMPLE:"
    echo "  run_semi_auto_tests.sh ./port/linux/chaos disbelieve"
}

manualmode=no
TEMP=$(getopt -o hm --long help,manual -- "$@")
eval set -- "$TEMP"
while true ; do
    case $1 in
        -m|--manual) manualmode=yes; shift ;;
        -h|--help ) print_usage ; exit 0 ;;
        --) shift ;  break ;;
        *) print_usage;  exit 1 ;;
    esac
done

if [ $# -eq 0 ] ; then
    print_usage
    echo
    echo "ERROR: Expected an argument"
    exit 1
fi

if ! which xdotool > /dev/null && [ $manualmode = no ] ; then
    echo "You need to install xdotool to run the automatic tests"
    echo " or you can use the --manual flag to run them manually"
    exit 1
fi

EXECUTABLE=$1
if ! test -x "$EXECUTABLE" ; then
    echo "Chaos executable '$EXECUTABLE' doesn't work"
    exit 1
fi
TEST_DIR=$(dirname $(readlink -f $0))

clicksquare()
{
    test $manualmode = yes && return
    x=$(expr \( 16 \* $1 \) + 12)
    y=$(expr \( 16 \* $2 \) + 12)
    skipexit=$3

    # bring to the front (wait for the window to appear first)
    sleep 1
    id=$(xdotool search --maxdepth 2 --name '^Chaos' 2>/dev/null)
    xdotool windowactivate "$id"
    sleep 0.5
    xdotool mousemove --window $id $x $y
    sleep 0.5
    xdotool mousedown 1
    sleep 0.5
    xdotool mouseup 1
    if test "$skipexit" = "" ; then
        exit_test
    fi
}

down()
{
    test $manualmode = yes && return
    id=$(xdotool search --maxdepth 2 --name '^Chaos' 2>/dev/null)
    xdotool windowactivate "$id"
    sleep 0.5
    xdotool key Down
}

exit_test()
{
    test $manualmode = yes && return
    sleep 3
    id=$(xdotool search --maxdepth 2 --name '^Chaos' 2>/dev/null )
    if test "$id" = "" ; then
        # it has gone
        return
    fi
    xdotool windowactivate $id
    while test $id -ne $(xdotool getactivewindow) ; do
        sleep 0.2
    done
    xdotool key Escape
}

# SPELL_DISBELIEVE 1
disbelieve() {
    echo "Check that DISBELIEVE works (cast it on the spectre)"
    "$EXECUTABLE" -l "$TEST_DIR/saves/disbelieve-works.txt" 2>/dev/null &
    clicksquare 2 4
    wait

    echo "Check that DISBELIEVE fails (cast it on the wraith)"
    "$EXECUTABLE" -l "$TEST_DIR/saves/disbelieve-fail.txt" 2>/dev/null &
    clicksquare 9 4
    wait

    echo "Check that DISBELIEVE works (CPU)"
    "$EXECUTABLE" -l "$TEST_DIR/saves/disbelieve-works-cpu.txt" 2>/dev/null &
    clicksquare 2 4
    wait
}
# SPELL_MEDITATE 2
meditate() {
    echo "Check that MEDITATE works"
    "$EXECUTABLE" -d1 -l "$TEST_DIR/saves/meditate.txt" 2>/dev/null &
    clicksquare 1 1
    wait

    echo "Check that MEDITATE fails"
    "$EXECUTABLE" -d0 -l "$TEST_DIR/saves/meditate.txt" 2>/dev/null &
    clicksquare 1 1
    wait

    echo "Check that MEDITATE works for cpu"
    "$EXECUTABLE" -d1 -l "$TEST_DIR/saves/meditate-cpu.txt" 2>/dev/null &
    clicksquare 1 1
    wait

}
# SPELL_KING_COBRA 3
creature() {
    echo "Check that creature cast works"
    "$EXECUTABLE" -d1 -l "$TEST_DIR/saves/creature.txt" 2>/dev/null &
    clicksquare 0 3
    wait

    echo "Check that creature cast fails"
    "$EXECUTABLE" -d0 -l "$TEST_DIR/saves/creature.txt" 2>/dev/null &
    clicksquare 1 3
    wait

    echo "Check that CPU creature cast works"
    cpu=$(mktemp)
    awk '{
    if (/SPELLC 13/ && !spellc) { print "\tSPELLC 1"; spellc=1;}
    else if (/TYPE 0/ && !type0) { print "\tTYPE 65"; type0=1;}
    else { print }
    }' "$TEST_DIR/saves/creature.txt" > $cpu
    sed 's/SPELLS.*,5,$/SPELLS 10,9/g' -i $cpu

    "$EXECUTABLE" -d1 -l $cpu 2>/dev/null &
    clicksquare 0 3
    wait
    rm $cpu
}
# SPELL_GOOEY_BLOB 36
fire() {
    echo "Check that FIRE/BLOB cast works"
    "$EXECUTABLE" -d1 -l "$TEST_DIR/saves/magic_fire.txt" 2>/dev/null &
    clicksquare 3 3
    wait

    echo "Check that FIRE/BLOB cast fails"
    "$EXECUTABLE" -d0 -l "$TEST_DIR/saves/magic_fire.txt" 2>/dev/null &
    clicksquare 3 4
    wait

    echo "Check that FIRE/BLOB cast works cpu"
    "$EXECUTABLE" -d1 -l "$TEST_DIR/saves/magic_fire-cpu.txt" 2>/dev/null &
    clicksquare 3 4
    wait
}

# SPELL_MAGIC_WOOD 38
magic_wood() {
    wood=$(mktemp)
    sed 's/,16,/,38,/g' "$TEST_DIR/saves/creature.txt" > $wood
    echo "Check that MAGIC WOOD cast works"
    "$EXECUTABLE" -d1 -l $wood 2>/dev/null &
    clicksquare 1 1 x
    sleep 7
    exit_test
    wait

    echo "Check that MAGIC WOOD cast fails"
    "$EXECUTABLE" -d0 -l $wood 2>/dev/null &
    clicksquare 1 1
    wait

    echo "Check MAGIC WOOD for cpus"
    "$EXECUTABLE" -d1 -l "$TEST_DIR/saves/mwood-cpu.txt" 2>/dev/null &
    clicksquare 1 1
    sleep 7
    exit_test
    wait

    rm $wood
}

# SPELL_SHADOW_WOOD 39
shadow_wood() {
    wood=$(mktemp)
    sed 's/,16,/,39,/g' "$TEST_DIR/saves/creature.txt" > $wood
    echo "Check that SHADOW WOOD cast works"
    "$EXECUTABLE" -d1 -l $wood 2>/dev/null &
    clicksquare 5 1 x
    clicksquare 3 1 x
    clicksquare 1 1 x

    clicksquare 5 7 x
    clicksquare 3 7 x
    clicksquare 1 7 x

    clicksquare 7 4 x
    clicksquare 5 4
    wait

    echo "Check that SHADOW WOOD cast fails"
    "$EXECUTABLE" -d0 -l $wood 2>/dev/null &
    clicksquare 5 4
    wait

    sed 's/,38,/,39,/g' "$TEST_DIR/saves/mwood-cpu.txt" > $wood
    echo "Check SHADOW WOOD for cpus"
    "$EXECUTABLE" -d1 -l $wood 2>/dev/null &
    clicksquare 1 1
    sleep 7
    exit_test
    wait
    rm $wood
}

# SPELL_MAGIC_CASTLE 40
castle() {
    echo "Check that CITADEL cast works"
    "$EXECUTABLE" -d1 -l "$TEST_DIR/saves/castle.txt" 2>/dev/null &
    clicksquare 3 4
    wait

    echo "Check that CITADEL cast fails"
    "$EXECUTABLE" -d0 -l "$TEST_DIR/saves/castle.txt" 2>/dev/null &
    clicksquare 3 4
    wait

    echo "CPU Castle test - should go under player"
    "$EXECUTABLE" -l "$TEST_DIR/saves/castle-cpu.txt" 2>/dev/null &
    clicksquare 2 4
    wait

}

# SPELL_WALL 42
wall() {
    wall=$(mktemp)
    sed 's/,16,/,42,/g' "$TEST_DIR/saves/creature.txt" > $wall
    echo "Check that WALL cast works"
    "$EXECUTABLE" -d1 -l $wall 2>/dev/null &
    clicksquare 4 2 x
    clicksquare 4 3 x
    clicksquare 4 4 x
    clicksquare 4 5
    wait

    echo "Check that WALL cast fails"
    "$EXECUTABLE" -d0 -l $wall 2>/dev/null &
    clicksquare 4 5
    wait

    rm $wall
}

# SPELL_MAGIC_BOLT 43
bolt() {
    bolt=$(mktemp)
    sed 's/,63,/,43,/g' "$TEST_DIR/saves/mutation.txt" > $bolt
    sed -i 's/SEED 0x76C46310/SEED 0x76C46311/g' $bolt

    echo "Check that MAGIC BOLT works (cast on wizard)"
    "$EXECUTABLE" -l $bolt 2>/dev/null &
    clicksquare 6 3
    wait

    echo "Check that MAGIC BOLT works (cast on creature)"
    "$EXECUTABLE" -l $bolt 2>/dev/null &
    clicksquare 6 4
    wait

    sed 's/,63,/,43,/g' "$TEST_DIR/saves/mutation.txt" > $bolt

    echo "Check that MAGIC BOLT fails (cast on wizard or creature)"
    "$EXECUTABLE" -l $bolt 2>/dev/null &
    clicksquare 6 4
    wait

    rm -f $bolt

    echo "Check that MAGIC BOLT cpu works"
    "$EXECUTABLE" -d1 -l "$TEST_DIR/saves/mbolt.txt" 2>/dev/null &
    clicksquare 6 4
    wait
}

# SPELL_BLIND 44
blind() {
    blind=$(mktemp)
    sed 's/,63,/,44,/g' "$TEST_DIR/saves/mutation.txt" > $blind

    echo "Check that BLIND works (cast on wizard)"
    "$EXECUTABLE" -l $blind 2>/dev/null &
    clicksquare 6 3 x
    sleep 2
    down # look and see if blind after cast
    exit_test
    wait

    echo "Check that BLIND works (cast on creature)"
    "$EXECUTABLE" -l $blind 2>/dev/null &
    clicksquare 6 4
    wait

    sed -i 's/SEED 0x76C46310/SEED 0x76C46311/g' $blind

    echo "Check that BLIND fails (cast on wizard or creature)"
    "$EXECUTABLE" -l $blind 2>/dev/null &
    clicksquare 6 3
    wait


    sed 's/,43,/,44,/g' "$TEST_DIR/saves/mbolt.txt" > $blind
    echo "Check that BLIND cpu works"
    "$EXECUTABLE" -d1 -l $blind 2>/dev/null &
    clicksquare 6 4
    wait

    rm -f $blind
}

# SPELL_MAGIC_SLEEP 46
msleep() {
    msleep=$(mktemp)
    sed 's/,63,/,46,/g' "$TEST_DIR/saves/mutation.txt" > $msleep

    echo "Check that MAGIC SLEEP works (cast on wizard)"
    "$EXECUTABLE" -l $msleep 2>/dev/null &
    clicksquare 6 3
    wait

    echo "Check that MAGIC SLEEP works (cast on creature)"
    "$EXECUTABLE" -l $msleep 2>/dev/null &
    clicksquare 6 4
    wait

    sed -i 's/SEED 0x76C46310/SEED 0x76C46311/g' $msleep

    echo "Check that MAGIC SLEEP fails (cast on wizard or creature)"
    "$EXECUTABLE" -l $msleep 2>/dev/null &
    clicksquare 6 3
    wait

    rm -f $msleep
}

# SPELL_JUSTICE 48
justice() {
    justice=$(mktemp)
    sed 's/,63,/,48,/g' "$TEST_DIR/saves/mutation.txt" > $justice
    sed -i 's/SEED 0x76C46310/SEED 0x76C46311/g' $justice

    echo "Check that JUSTICE works (cast on wizard)"
    "$EXECUTABLE" -l $justice 2>/dev/null &
    clicksquare 6 3
    wait

    echo "Check that JUSTICE works (cast on creature)"
    "$EXECUTABLE" -l $justice 2>/dev/null &
    clicksquare 6 4
    wait

    sed 's/,63,/,48,/g' "$TEST_DIR/saves/mutation.txt" > $justice

    echo "Check that JUSTICE fails (cast on wizard or creature)"
    "$EXECUTABLE" -l $justice 2>/dev/null &
    clicksquare 6 4
    wait

    echo "Check that JUSTICE for cpu is ok"
    "$EXECUTABLE" -l "$TEST_DIR/saves/justice.txt" 2>/dev/null &
    clicksquare 6 4
    wait

    rm -f $justice
}
# SPELL_DECREE 50
decree() {
    decree=$(mktemp)
    sed 's/,63,/,50,/g' "$TEST_DIR/saves/mutation.txt" > $decree
    sed -i 's/SEED 0x76C46310/SEED 0x76C46317/g' $decree

    echo "Check that DECREE works (cast on wizard a couple of times)"
    "$EXECUTABLE" -l $decree 2>/dev/null &
    clicksquare 6 3 x
    sleep 2
    clicksquare 6 3 x
    sleep 2
    clicksquare 7 8
    wait

    sed 's/,63,/,50,/g' "$TEST_DIR/saves/mutation.txt" > $decree
    sed -i 's/SEED 0x76C46310/SEED 0x76C46306/g' $decree

    echo "Check that DECREE fails (cast on wizard or creature)"
    "$EXECUTABLE" -l $decree 2>/dev/null &
    clicksquare 7 8 x
    sleep 2
    clicksquare 6 3 x
    sleep 2
    clicksquare 6 3
    wait

    rm -f $decree
}

wizspell()
{
    name="$1"
    id="$2"
    tmp=$(mktemp)
    sed "s/,16,/,$id,/g" "$TEST_DIR/saves/creature.txt" > $tmp

    echo "Check that $name works"
    "$EXECUTABLE" -d1 -l $tmp 2>/dev/null &
    clicksquare 1 1
    wait

    echo "Check that $name fails"
    "$EXECUTABLE" -d0 -l $tmp 2>/dev/null &
    clicksquare 1 1
    wait

    rm -f $tmp
}
# SPELL_MAGIC_SHIELD 51
shield() {
    wizspell "MAGIC SHIELD" 51
}

# SPELL_MAGIC_ARMOUR 52
armour() {
    wizspell "MAGIC ARMOUR" 52
}
# SPELL_MAGIC_SWORD 53
sword() {
    wizspell "MAGIC SWORD" 53
}
# SPELL_MAGIC_KNIFE 54
knife() {
    wizspell "MAGIC KNIFE" 54
}
# SPELL_MAGIC_BOW 55
bow() {
    wizspell "MAGIC BOW" 55
}
# SPELL_MAGIC_WINGS 56
wings() {
    wizspell "MAGIC WINGS" 56
}
# SPELL_LAW_1 57
law() {
    wizspell "LAW_1" 57
}
# SPELL_SHADOW_FORM 61
shadow_form() {
    wizspell "SHADOW FORM" 61
}
# SPELL_SUBVERSION 62
subversion() {
    tmp=$(mktemp)

    sed 's/,63,/,62,/g' "$TEST_DIR/saves/mutation.txt" > $tmp

    # seed is such that subversion on the bat works...
    echo "Check that SUBVERSION works (cast on the bat)"
    "$EXECUTABLE" -l $tmp 2>/dev/null &
    clicksquare 7 8
    wait

    # create a failing cast by tweaking the random seed
    sed -i 's/SEED 0x76C46310/SEED 0x76C46311/g' $tmp

    echo "Check that SUBVERSION fails (cast on any creature)"
    "$EXECUTABLE" -l $tmp 2>/dev/null &
    clicksquare 6 4
    wait

    echo "CPU SUBVERSION"
    "$EXECUTABLE" -d1 -l "$TEST_DIR/saves/subversion-cpu.txt" 2>/dev/null &
    clicksquare 6 4
    wait

    rm -f $tmp

}

# SPELL_MUTATION 63
mutation() {
    echo "Check that MUTATION works (cast on wizard)"
    "$EXECUTABLE" -l "$TEST_DIR/saves/mutation.txt" 2>/dev/null &
    clicksquare 6 3
    wait

    echo "Check that MUTATION works (cast on creature)"
    "$EXECUTABLE" -l "$TEST_DIR/saves/mutation.txt" 2>/dev/null &
    clicksquare 4 4
    wait

    # create a failing cast by tweaking the random seed
    fail=$(mktemp)
    sed 's/SEED 0x76C46310/SEED 0x76C46311/g' "$TEST_DIR/saves/mutation.txt" > $fail

    echo "Check that MUTATION fails (cast on wizard)"
    "$EXECUTABLE" -l $fail 2>/dev/null &
    clicksquare 6 3
    wait

    echo "CPU MUTATION"
    sed 's/,62/,63/g' "$TEST_DIR/saves/subversion-cpu.txt" > $fail
    "$EXECUTABLE" -d1 -l "$fail" 2>/dev/null &
    clicksquare 6 4
    wait

    rm -f $fail
}

# SPELL_RAISE_DEAD 64
raise_dead() {
    tmp=$(mktemp)

    sed 's/,63,/,64,/g' "$TEST_DIR/saves/mutation.txt" > $tmp

    # create a working cast by tweaking the random seed
    sed -i 's/SEED 0x76C46310/SEED 0x76C46311/g' $tmp

    echo "Check that RAISE DEAD works (cast on the lion)"
    "$EXECUTABLE" -l $tmp 2>/dev/null &
    clicksquare 2 7
    wait

    sed 's/,63,/,64,/g' "$TEST_DIR/saves/mutation.txt" > $tmp

    echo "Check that RAISE DEAD fails (cast on the lion)"
    "$EXECUTABLE" -l $tmp 2>/dev/null &
    clicksquare 2 7
    wait

    echo "CPU RAISE DEAD"
    sed 's/,62/,64/g' "$TEST_DIR/saves/subversion-cpu.txt" > $tmp
    "$EXECUTABLE" -d1 -l "$tmp" 2>/dev/null &
    clicksquare 6 4
    wait

    rm -f $tmp
}

# SPELL_TURMOIL 65
turmoil() {
    echo "Check that TURMOIL works..."
    "$EXECUTABLE" -l "$TEST_DIR/saves/turmoil.txt" 2>/dev/null &
    clicksquare 1 1 x
    # more like sleep 50... this takes ages
    sleep 5
    exit_test
    wait
}

# -------------------
shift
available()
{
    grep -h '^\w\+() {$' $1 | cut -f1 -d\( | sort
}
if test $# -eq 0 ; then
    echo "Available tests are:"
    available $0
    echo '... or * for all'
    exit 1
fi
if [ "$1" = '*' ] ; then
    tests=$(grep -h '^\w\+() {$' $0 | cut -f1 -d\()
else
    tests="$@"
fi
if test $(pgrep chaos | wc -l) -gt 0 ; then
    echo "Chaos is already running, this will mess up the tests"
    exit 1
fi
for t in $tests ; do
    if ! grep -q "\<$t\>" <(available $0) ; then
        echo "Skipping non-existent test '$t'"
        continue
    fi
    $t
done
