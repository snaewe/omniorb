#!/usr/bin/env python

# Python script to process a Unicode mapping table for an 8-bit
# character set into an omniORB code set object definition.

# Lines beginning # are comments
# Data lines are of the form:
#
#   0xXX <tab> 0xXXXX <tab> # <unicode name>

import sys, string

if len(sys.argv) >= 4:
    file = open(sys.argv[1], "r")
    csid   = sys.argv[2]
    name = sys.argv[3]
    if len(sys.argv) == 4:
        dceid = "omniCodeSet::ID_" + csid
    else:
        dceid = sys.argv[4]
else:
    sys.stderr.write("Usage: %s <file> <csid> <name> [dce id]\n" % sys.argv[0])
    sys.exit(1)

to_u = [0] * 256
fr_u = {}
bank = [0] * 256

while 1:
    line = file.readline()
    if not line:
        break
    
    if line[0] == "#":
        continue

    sl = string.split(line)
    if (len(sl) < 2 or sl[0][:2] != "0x" or sl[1][:2] != "0x"):
        sys.stderr.write("Don't understand line: " +
                         string.join(sl, "\t") + "\n")
        continue

    c = string.atoi(sl[0], 0)
    u = string.atoi(sl[1], 0)

    to_u[c]      = u
    fr_u[u]      = c
    bank[u >> 8] = 1

sys.stdout.write("""\
// -*- Mode: C++; -+-
//
// Code set table automatically generated from:
//
//  %s
//

#include <omniORB4/CORBA.h>
#include <codeSetUtil.h>

static const omniCodeSet::UniChar toUCS[] = {""" % sys.argv[1])

for i in range(256):
    if i % 8 == 0:
        sys.stdout.write("\n ")

    sys.stdout.write(" 0x%04x," % to_u[i])

print "\n};\n"

for i in range(256):
    if bank[i]:
        sys.stdout.write("static const _CORBA_Char frUCS%02x[] = {" % i)

        add = i << 8

        for j in range(256):
            if j % 8 == 0:
                sys.stdout.write("\n ")

            sys.stdout.write(" 0x%02x," % fr_u.get(add + j, 0))

        print "\n};\n"

sys.stdout.write("""\
#ifdef E_T
#  undef E_T
#endif

#define E_T omniCodeSet::empty8BitTable

static const _CORBA_Char* frUCS[] = {""")

for i in range(256):
    if i % 8 == 0:
        sys.stdout.write("\n ")

    if bank[i]:
        sys.stdout.write(" frUCS%02x," % i)
    else:
        sys.stdout.write(" E_T,")

print "\n};\n"


spacer = " " * len(csid)

print """\
static omniCodeSet::NCS_C_8bit _NCS_C_%(csid)s(%(dceid)s,
                                       %(spacer)s"%(name)s",
                                       %(spacer)stoUCS, frUCS);

static omniCodeSet::TCS_C_8bit _TCS_C_%(csid)s(%(dceid)s,
                                       %(spacer)s"%(name)s",
                                       %(spacer)somniCodeSetUtil::GIOP12,
                                       %(spacer)stoUCS, frUCS);

class CS_%(csid)s_init {
public:
  CS_%(csid)s_init() {
    omniCodeSet::registerNCS_C(&_NCS_C_%(csid)s);
    omniCodeSet::registerTCS_C(&_TCS_C_%(csid)s);
  }
};
static CS_%(csid)s_init _CS_%(csid)s_init_;
""" % vars()
