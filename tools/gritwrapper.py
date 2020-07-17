#!/usr/bin/env python
import os
import glob
import sys
import subprocess

# wrap grit command to avoid chdir issues
# arg1 = new path
# arg2 = grit cmd
# arg3+ = grit args
if __name__ == '__main__':
    try:
        args = []
        palettenames = []
        # convert filenames to absolute file names
        for arg in sys.argv[2:]:
            if arg.startswith('..'):
                arg = os.path.abspath(arg)
            elif arg.startswith('-O'):
                palettenames.append(arg[2:])
            args.append(arg)

        dirname = sys.argv[1]
        if not os.path.exists(dirname):
            print "Create dir" , dirname
            os.mkdir(dirname)

        os.chdir(dirname)
        for p in palettenames:
            for pal in glob.glob(p + '.*'):
                os.unlink(pal)
        try:
            proc = subprocess.Popen(args)
            proc.wait()
        except OSError, e:
            sys.stderr.write(str(e) + ' running:\n')
            sys.stderr.write(str(' '.join(args)) + '\n')
            sys.exit(1)
    except Exception, e:
        sys.stderr.write(str(e) + '\n')
        sys.exit(1)
    except KeyboardInterrupt, e:
        sys.stderr.write('Interrupted\n')
        sys.exit(1)
