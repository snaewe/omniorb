# -*- python -*-
#                           Package   : omniidl
# __init__.py               Created on: 1999/11/3
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
#   Entrypoint to the C++ backend

# $Id$
# $Log$
# Revision 1.10  1999/12/09 20:41:24  djs
# Now runs typecode and any generator
#
# Revision 1.9  1999/12/01 17:05:13  djs
# Backend now supports command line arguments
#
# Revision 1.8  1999/11/29 19:26:59  djs
# Code tidied and moved around. Some redundant code eliminated.
#
# Revision 1.7  1999/11/17 20:37:09  djs
# General util functions
#
# Revision 1.6  1999/11/15 19:10:54  djs
# Added module for utility functions specific to generating skeletons
# Union skeletons working
#
# Revision 1.5  1999/11/12 17:17:45  djs
# Creates output files rather than using stdout
# Utility functions useful for skeleton generation added
#
# Revision 1.4  1999/11/08 19:28:56  djs
# Rewrite of sequence template code
# Fixed lots of typedef problems
#
# Revision 1.3  1999/11/04 19:05:01  djs
# Finished moving code from tmp_omniidl. Regression tests ok.
#
# Revision 1.2  1999/11/03 17:35:06  djs
# Brought more of the old tmp_omniidl code into the new tree
#
# Revision 1.1  1999/11/03 11:09:49  djs
# General module renaming
#

# From http://www-i3.informatik.rwth-aachen.de/funny/babbage.html:
# ...
# C. A. R. Hoare, in his 1980 ACM Turing Award lecture, told of two
# ways of constructing a software design: "One way is to make it so
# simple that there are obviously no deficiencies and the other way
# is to make it so complicated that there are no obvious deficiencies." 
#

# -----------------------------
# Output generation functions
from omniidl.be.cxx import header
from omniidl.be.cxx import skel
from omniidl.be.cxx import dynskel

from omniidl.be.cxx import config

import re, sys

cpp_args = ["-D__OMNIIDL_CXX__"]
usage_string = """\
  -Wba            Generate code for TypeCodes and Any
  -Wbtp           Generate 'tie' implementation skeletons
  -Wbtf           Generate flattened 'tie' implementation skeletons"""

# -----------------------------
# Process back end specific arguments

def typecode_any():
    config.setTypecodeFlag(1)

def tie():
    config.setTieFlag(1)

def flat_tie():
    config.setFlatTieFlag(1)

arguments = {
    "a":  typecode_any,
    "tp": tie,
    "tf": flat_tie
    }

def process_args(args):
    for arg in args:
        if arguments.has_key(arg):
            arguments[arg]()
        else:
            print "Argument \"" + str(arg) + "\" is unknown to the " +\
                  "C++ backend\n"
            print "omniidl -bcxx -u for usage"
            sys.exit(1)
            

def run(tree, args):
    """Entrypoint to the C++ backend"""


    filename = tree.file()
    regex = re.compile(r"(.*/|)(.+)\.idl")
    match = regex.search(filename)
    if match:
        config.setBasename(match.group(2))
    else:
        raise "Unable to work out basename of input file"

    # set the default behaviour
    config.setTypecodeFlag(0)
    config.setTieFlag(0)
    config.setFlatTieFlag(0)

    process_args(args)
       
    header.run(tree)
    
    skel.run(tree)

    # if we're generating code for Typecodes and Any then
    # we need to create the DynSK.cc file
    if config.TypecodeFlag():
        dynskel.run(tree)
