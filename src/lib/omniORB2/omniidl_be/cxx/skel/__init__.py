# -*- python -*-
#                           Package   : omniidl
# __init__.py               Created on: 1999/11/11
#			    Author    : David Scott (djs)
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
#   Entrypoint to skeleton generation code

# $Id$
# $Log$
# Revision 1.5  2000/01/13 15:56:43  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.4  2000/01/13 14:16:34  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.3  2000/01/11 11:34:49  djs
# Added support for fragment generation (-F) mode
#
# Revision 1.2  1999/11/19 20:10:13  djs
# Now runs the poa generating code after the main code
#
# Revision 1.1  1999/11/12 17:18:57  djs
# Struct skeleton code added
#

# -----------------------------
# Configuration data
from omniidl.be.cxx import config

# -----------------------------
# Utility functions
from omniidl.be.cxx import tyutil, util

from omniidl.be.cxx.skel import main
from omniidl.be.cxx.skel import poa
from omniidl.be.cxx.skel import mangler

def monolithic(stream, tree):
    """Creates one large skeleton with all code inside"""
    stream.out("""\
// This file is generated by @Config.program_Name()@- @Config.omniORB_Library_Version()@. Do not edit.

#include "@Config.basename()@@Config.hdrsuffix()@"
#include <omniORB3/callDescriptor.h>

static const char* @Config.privatePrefix()@_library_version = @Config.omniORB_Library_Version()@;


""", Config = config)

    skel = main.__init__(stream)
    tree.accept(skel)

    poa_skel = poa.__init__(stream)
    tree.accept(poa_skel)

def fragment(stream, tree):
    """Used in fragment mode"""

    stream.out("""\
// This file is generated by @Config.program_Name()@- @Config.omniORB_Library_Version()@. Do not edit.
""", Config = config)
    
    skel = main.__init__(stream)
    tree.accept(skel)

    poa_skel = poa.__init__(stream)
    tree.accept(poa_skel)


def run(tree):
    # create somewhere to put the output
    skel_filename = config.basename() + config.skelsuffix()
    stream = util.Stream(open(skel_filename, "w"), 2)

    # clear all state
    mangler.__init__()

    if config.FragmentFlag():
        fragment(stream, tree)
    else:
        # generate one big chunk of code
        monolithic(stream, tree)





