#!/usr/bin/env python
# -*- python -*-
#                           Package   : omniidl
# omniidlrun.py             Created on: 1999/10/29
#			    Author    : Duncan Grisby (dpg1)
#
#    Copyright (C) 1999 AT&T Laboratories Cambridge
#
#  This file is part of omniidl.
#
#  omniidl is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA.
#
# Description:
#   
#   IDL compiler command

import sys

if sys.version[:6] != "1.5.2 ":
    sys.stderr.write("\n\n")
    sys.stderr.write("omniidl: WARNING!!\n\n")
    sys.stderr.write("omniidl: Python version 1.5.2 is required.\n")
    sys.stderr.write("omniidl: " + sys.executable + " is version " + \
                     sys.version + "\n")
    sys.stderr.write("omniidl: Execution is likely to fail.\n")
    sys.stderr.write("\n\n\n")
    sys.stderr.flush()

import os, os.path

pylibdir   = archlibdir = None
binarchdir = os.path.abspath(os.path.dirname(sys.argv[0]))

if binarchdir != "":
    sys.path.insert(0, binarchdir)
    bindir, archname = os.path.split(binarchdir)
    treedir, bin     = os.path.split(bindir)
    if bin == "bin":
        pylibdir   = os.path.join(treedir, "lib", "python")
        archlibdir = os.path.join(treedir, "lib", archname)

        if os.path.isdir(pylibdir):
            sys.path.insert(0, pylibdir)

        if os.path.isdir(archlibdir):
            sys.path.insert(0, archlibdir)

try:
    import _omniidl
except ImportError, msg:
    sys.stderr.write("\n\n")
    sys.stderr.write("omniidl: ERROR!\n\n")
    sys.stderr.write("omniidl: Could not find IDL compiler module " \
                     "_omniidlmodule.so\n")
    sys.stderr.write("omniidl: Please make sure it is in directory ")
    sys.stderr.write((archlibdir or binarchdir) + "\n")
    sys.stderr.write("omniidl: (or set the PYTHONPATH environment variable)\n")
    sys.stderr.write("\n")
    sys.stderr.write("omniidl: (The error was `" + str(msg) + "')\n")
    sys.stderr.write("\n\n")
    sys.stderr.flush()
    sys.exit(1)

try:
    import omniidl.main
except ImportError, msg:
    sys.stderr.write("\n\n")
    sys.stderr.write("omniidl: ERROR!\n\n")
    sys.stderr.write("omniidl: Could not find Python files for IDL compiler\n")
    sys.stderr.write("omniidl: Please put them in directory " + \
                     (pylibdir or binarchdir) + "\n")
    sys.stderr.write("omniidl: (or set the PYTHONPATH environment variable)\n")
    sys.stderr.write("\n")
    sys.stderr.write("omniidl: (The error was `" + str(msg) + "')\n")
    sys.stderr.write("\n\n")
    sys.stderr.flush()
    sys.exit(1)

omniidl.main.main()
