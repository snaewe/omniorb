#!/usr/bin/env python

# Script to extract the distribution date from update.log

import sys, string

if len(sys.argv) > 1:
    package = sys.argv[1]
else:
    package = "OMNIORB"

line = sys.stdin.readline()

while line == "\n":
    line = sys.stdin.readline()

try:
    line = string.strip(line)
except NameError:
    # Python 3 has no string.strip()
    line = line.strip()

output = """\
// distdate.hh -- Automatically generated file

#define %s_DIST_DATE "%s"
"""

sys.stdout.write(output % (package, line))
