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
# Revision 1.22  2000/08/18 14:09:15  dpg1
# Merge from omni3_develop for 3.0.1 release.
#
# Revision 1.18.2.10  2000/08/14 19:34:44  djs
# Performs a quick scan of the AST looking for unsupported IDL constructs
# before doing anything else.
#
# Revision 1.18.2.9  2000/08/07 15:34:34  dpg1
# Partial back-port of long long from omni3_1_develop.
#
# Revision 1.18.2.8  2000/07/18 15:34:17  djs
# Added -Wbvirtual_objref option to make attribute and operation _objref
# methods virtual
#
# Revision 1.18.2.7  2000/06/26 16:23:09  djs
# Added new backend arguments.
# Better error handling when encountering unsupported IDL (eg valuetypes)
# Refactoring of configuration state mechanism.
#
# Revision 1.18.2.6  2000/05/31 15:11:11  dpg1
# C++ back-end properly handles Windows paths.
#
# Revision 1.18.2.5  2000/05/24 17:16:34  dpg1
# Minor omniidl usage message fix.
#
# Revision 1.18.2.4  2000/05/04 14:34:50  djs
# Added new flag splice-modules which causes all continuations to be output
# as one lump. Default is now to output them in pieces following the IDL.
#
# Revision 1.18.2.3  2000/04/26 18:22:12  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.18.2.2  2000/02/18 23:01:19  djs
# Updated example implementation code generating module
#
# Revision 1.18.2.1  2000/02/14 18:34:57  dpg1
# New omniidl merged in.
#
# Revision 1.18  2000/01/13 11:45:47  djs
# Added option to customise C++ reserved word name escaping
#
# Revision 1.17  2000/01/13 10:52:04  djs
# Rewritten argument handling
# Added options to specify the header and stubs file prefix, mirroring omniidl3
#
# Revision 1.16  2000/01/12 19:54:47  djs
# Added option to generate old CORBA 2.1 signatures for skeletons
#
# Revision 1.15  2000/01/12 17:48:27  djs
# Added option to create BOA compatible skeletons (same as -BBOA in omniidl3)
#
# Revision 1.14  2000/01/11 14:12:41  djs
# Updated commandline option help to include Fragment generation
#
# Revision 1.13  2000/01/11 11:35:36  djs
# Added support for fragment generation (-F) mode
#
# Revision 1.12  2000/01/10 15:39:34  djs
# Better name and scope handling.
#
# Revision 1.11  1999/12/24 18:20:12  djs
# Builds list of IDL files #included by walking the AST and examining the
# file() of each Decl node.
#
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
from omniidl_be.cxx import header
from omniidl_be.cxx import skel
from omniidl_be.cxx import dynskel
from omniidl_be.cxx import impl

from omniidl_be.cxx import id

from omniidl_be.cxx import support
from omniidl_be.cxx import config

import re, sys, os.path

cpp_args = ["-D__OMNIIDL_CXX__"]
usage_string = """\
  -Wbh=<suffix>     Specify suffix for generated header files
  -Wbs=<suffix>     Specify suffix for generated stub files
  -Wbd=<suffix>     Specify suffix for generated dynamic files
  -Wba              Generate code for TypeCodes and Any
  -Wbtp             Generate 'tie' implementation skeletons
  -Wbtf             Generate flattened 'tie' implementation skeletons
  -Wbsplice-modules Splice together multiply opened modules into one 
  -Wbexample        Generate example implementation code
  -WbF              Generate code fragments (for expert only)
  -WbBOA            Generate BOA compatible skeletons
  -Wbold            Generate old CORBA 2.1 signatures for skeletons
  -Wbold_prefix     Map C++ reserved words with prefix _
  -Wbkeep_inc_path  Preserve IDL #include path in header #includes
  -Wbuse_quotes     Use quotes in #includes: \"foo\" rather than <foo>"""

# Encountering an unknown AST node will cause an AttributeError exception
# to be thrown in one of the visitors. Store a list of those not-supported
# so we can produce a friendly error message later.
AST_unsupported_nodes = [ "Native", "StateMember", "Factory", "ValueForward",
                          "ValueBox", "ValueAbs", "Value" ]

def process_args(args):
    for arg in args:
        if arg == "a":
            config.state['Typecode']          = 1
        elif arg == "tp":
            config.state['Normal Tie']        = 1
        elif arg == "tf":
            config.state['Flattened Tie']     = 1
        elif arg == "splice-modules":
            config.state['Splice Modules']    = 1
        elif arg == "example":
            config.state['Example Code']      = 1
        elif arg == "F":
            config.state['Fragment']          = 1
        elif arg == "BOA":
            config.state['BOA Skeletons']     = 1
        elif arg == "old":
            config.state['Old Signatures']    = 0
        elif arg == "old_prefix":
            config.state['Reserved Prefix']   = "_"
        elif arg == "keep_inc_path":
            config.state['Keep Include Path'] = 1
        elif arg == "use_quotes":
            config.state['Use Quotes']        = 1
        elif arg == "virtual_objref":
            config.state['Virtual Objref Methods'] = 1
        elif arg == "debug":
            config.state['Debug']             = 1
        elif arg[:2] == "h=":
            config.state['HH Suffix']         = arg[2:]
        elif arg[:2] == "s=":
            config.state['SK Suffix']         = arg[2:]
        elif arg[:2] == "d=":
            config.state['DYNSK Suffix']      = arg[2:]
        else:
            util.fatalError("Argument \"" + str(arg) + "\" is unknown")

def run(tree, args):
    """Entrypoint to the C++ backend"""

    dirname, filename = os.path.split(tree.file())
    basename,ext      = os.path.splitext(filename)
    config.state['Basename']  = basename
    config.state['Directory'] = dirname

    process_args(args)

    try:
        # Check the input tree only contains stuff we understand
        support.checkIDL(tree)
        
        # build the list of include files
        walker = config.WalkTreeForIncludes()
        tree.accept(walker)
        
        environments = id.WalkTree()
        tree.accept(environments)

        header.run(tree)
        
        skel.run(tree)
        
        # if we're generating code for Typecodes and Any then
        # we need to create the DynSK.cc file
        if config.state['Typecode']:
            dynskel.run(tree)

        if config.state['Example Code']:
            impl.run(tree)

    except AttributeError, e:
        name = e[0]
        unsupported_visitors = map(lambda x:"visit" + x,
                                   AST_unsupported_nodes[:])
        if name in unsupported_visitors:
            util.unsupportedIDL()
            
        util.fatalError("An AttributeError exception was caught")
    except SystemExit, e:
        # fatalError function throws SystemExit exception
        # *** Should delete partial output files here
        raise e
    except:
        util.fatalError("An internal exception was caught")
