#!/usr/bin/env python

# Python script to process a Unicode mapping table for a 16-bit
# character set into an omniORB code set object definition.

# Lines beginning # are comments
# Data lines are of the form:
#
#   0xXXXX <tab> 0xXXXX <tab> # <unicode name>

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

to_u    = {}
fr_u    = {}
to_bank = [0] * 256
fr_bank = [0] * 256

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

    to_u[c]         = u
    to_bank[c >> 8] = 1
    fr_u[u]         = c
    fr_bank[u >> 8] = 1

sys.stdout.write("""\
// -*- Mode: C++; -+-
//
// Code set table automatically generated from:
//
//  %s
//

#include <omniORB4/CORBA.h>
#include <codeSetUtil.h>

"""                 % sys.argv[1])


for i in range(256):
    if to_bank[i]:
        sys.stdout.write("static const omniCodeSet::UniChar toUCS%02x[] = {"%i)

        add = i << 8

        for j in range(256):
            if j % 8 == 0:
                sys.stdout.write("\n ")

            sys.stdout.write(" 0x%04x," % to_u.get(add + j, 0))

        print "\n};\n"

for i in range(256):
    if fr_bank[i]:
        sys.stdout.write("static const _CORBA_UShort frUCS%02x[] = {" % i)

        add = i << 8

        for j in range(256):
            if j % 8 == 0:
                sys.stdout.write("\n ")

            sys.stdout.write(" 0x%04x," % fr_u.get(add + j, 0))

        print "\n};\n"

sys.stdout.write("""\
#ifdef E_T
#  undef E_T
#endif

#define E_T (omniCodeSet::UniChar*)omniCodeSet::empty16BitTable

static const omniCodeSet::UniChar* toUCS[] = {""")

for i in range(256):
    if i % 8 == 0:
        sys.stdout.write("\n ")

    if to_bank[i]:
        sys.stdout.write(" toUCS%02x," % i)
    else:
        sys.stdout.write(" E_T,")
print "\n};\n"

sys.stdout.write("""\
#undef  E_T
#define E_T (_CORBA_UShort*)omniCodeSet::empty16BitTable

static const _CORBA_UShort* frUCS[] = {""")

for i in range(256):
    if i % 8 == 0:
        sys.stdout.write("\n ")

    if fr_bank[i]:
        sys.stdout.write(" frUCS%02x," % i)
    else:
        sys.stdout.write(" E_T,")

print "\n};\n"


spacer = " " * len(csid)

print """\
static omniCodeSet::NCS_W_16bit _NCS_W_%(csid)s(%(dceid)s,
                                        %(spacer)s"%(name)s",
                                        %(spacer)stoUCS, frUCS);

static omniCodeSet::TCS_W_16bit _TCS_W_%(csid)s(%(dceid)s,
                                        %(spacer)s"%(name)s",
                                        %(spacer)somniCodeSetUtil::GIOP12,
                                        %(spacer)stoUCS, frUCS);

class CS_%(csid)s_init {
public:
  CS_%(csid)s_init() {
    omniCodeSet::registerNCS_W(&_NCS_W_%(csid)s);
    omniCodeSet::registerTCS_W(&_TCS_W_%(csid)s);
  }
};
static CS_%(csid)s_init _CS_%(csid)s_init_;
""" % vars()
