#!/usr/bin/env python

# Script to extract the distribution date from update.log

import sys, string

line = sys.stdin.readline()

while line == "\n":
    line = sys.stdin.readline()

line = string.strip(line)

output = """\
// distdate.hh -- Automatically generated file

#define OMNIORB_DIST_DATE "%s"
"""

print output % line
