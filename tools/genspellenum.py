#!/usr/bin/env python
import sys

_BANNER = '''
/* AUTO-GENERATED FILE.  DO NOT MODIFY.
 *
 * This file was automatically generated by the
 * genspellenum tool from the spelldata.csv file.
 * It should not be modified by hand.
 */
'''
def create_define_file(infile, outfile):
    f = open(infile)
    out = open(outfile, 'w')
    out.write(_BANNER)
    out.write('''
#ifndef SPELL_NAMES_H
#define SPELL_NAMES_H

''')

    idx = 0
    for line in f.readlines()[1:]:
        out.write('#define SPELL_')
        out.write(line.split(',')[0].strip('"').upper().replace(' ', '_'))
        first = False
        out.write(' %d\n' % idx)
        idx += 1

    out.write('\n')
    out.write('#endif\n\n')

def gfxname(name):
    n = name.strip('"').replace(' ', '_').lower()
    return '%sTiles' % n

def mapname(name):
    n = name.strip('"').replace(' ', '_').lower()
    return '%sMap' % n

def create_spelldata_file(infile, outfile):
    f = open(infile)
    out_c = open(outfile, 'w')
    out_c.write(_BANNER)
    out_c.write('''
/* for struct spell_data */
#include "chaos/spelldata.h"
/* for magic spell functions */
#include "chaos/magic.h"
''')
    lines = f.readlines()
    for line in lines[4:]:
        val = line.split(',')[0]
        pal = line.split(',')[5]
        out_c.write('#include "gfx/palette%s/%s.h"\n' % (pal, val.strip('"').replace(' ', '_').lower()))
        if val == '"Wall"':
            break

    out_c.write('const struct spell_data CHAOS_SPELLS[] = {\n')
    should_add_gfx = 0
    names = [e.strip('"') for e in lines[0].strip().split(',')]
    for line in lines[1:]:
        line = line.strip()
        out_c.write('\t')
        out_c.write('{')
        vals = line.split(',')
        entry_idx = 0
        for val in vals:
            name = names[entry_idx]
            entry_idx += 1
            if val.startswith('"'):
                # string entries
                if name == 'spellFunction' or name == 'flags':
                    # these are symbols, not strings
                    val = val.strip('"')
                is_creature = should_add_gfx >= 3 and should_add_gfx <= 35
                if name == 'description' and is_creature:
                    val = '0' # no description for creatures
                out_c.write(val)
            else:
                if val:
                    val = int(val)
                    if name == 'castRange' and val > 0:
                        val = val * 2 + 1
                    out_c.write("%d" % val)
                else:
                    out_c.write('0')
            out_c.write(',')
        if should_add_gfx >= 3 and should_add_gfx <= 42:
            out_c.write(gfxname(vals[0]))
            out_c.write(',')
            out_c.write(mapname(vals[0]))
        else:
            out_c.write('0, 0')
        out_c.write('},\n')
        should_add_gfx += 1
    out_c.write('};\n')

def main(args):
    infile = args[0]
    outfile = args[1]
    create_define_file(infile, outfile)

    outfile = args[2]
    create_spelldata_file(infile, outfile)

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print "Usage: genspellenum infile.csv outenum.h outdata.c"
        sys.exit(1)
    main(sys.argv[1:])