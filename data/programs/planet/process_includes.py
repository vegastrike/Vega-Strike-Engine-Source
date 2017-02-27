#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys

if len(sys.argv) <= 1:
    print "Usage: process_includes.py <file1> .. <filen>"
    print "Will create <file1>.pp .. <filen>.pp"
    sys.exit(0)

def process(path, outfile, processed):
    if '/' in path:
        dirname = path[:path.rfind('/')]
    else:
        dirname = "."
    processed.add(path)
    infile = open(path, "r")
    for lineno,line in enumerate(infile):
        if line.startswith('#include "'):
            incpath = dirname + "/" + line[10:line.find('"',10)]
            if incpath not in processed:
                outfile.write("#line 1\n")
                print "  including", incpath
                process(incpath, outfile, processed)
                outfile.write("\n#line %d\n" % (lineno+1))
            else:
                outfile.write("\n")
        else:
            outfile.write(line)

for path in sys.argv[1:]:
    print "Processing", path
    outfile = open(path+".pp","w")
    process(path, outfile, set())
    