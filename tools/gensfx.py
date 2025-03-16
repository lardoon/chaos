#!/usr/bin/env python
import sys
import os

_BANNER = '''
/* AUTO-GENERATED FILE.  DO NOT MODIFY.
 *
 * This file was automatically generated by the
 * gensfx tool from the sfx/ *.wav files.
 * It should not be modified by hand.
 */
'''

def write_header(wavs, header):
    hf = open(header, 'w')
    idx = 0
    hf.write(_BANNER + '''
#ifndef SOUNDDATA_H
#define SOUNDDATA_H
''')
    for w in wavs:
        name = os.path.basename(w).split('.')[0].replace(' ', '_').upper()
        hf.write('#define SND_%s %d\n' % (name, idx))
        idx += 1
    hf.write('#endif\n')

def write_impl(wavs, impl, header):
    hf = open(impl, 'w')
    hf.write(_BANNER)
    hf.write('#include "%s"\n' % os.path.basename(header))

def main():
    wavs = []
    header = []
    impl = []
    for filename in sys.argv[1:]:
        if filename.endswith('.wav'):
            wavs.append(filename)
        if filename.endswith('.c'):
            impl.append(filename)
        if filename.endswith('.h'):
            header.append(filename)
    if len(header) > 1:
        print("Only one header is needed")
        sys.exit(1)
    if len(impl) > 1:
        print("Only one implementation .c file is needed")
        sys.exit(1)

    wavs.sort()
    write_header(wavs, header[0])
    write_impl(wavs, impl[0], header[0])


if __name__ == '__main__':
    main()
