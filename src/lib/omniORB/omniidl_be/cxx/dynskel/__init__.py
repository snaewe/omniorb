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
#   Entrypoint to dynamic skeleton generation code

# $Id$
# $Log$
# Revision 1.8.2.3  2001/06/08 17:12:14  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.8.2.2  2000/10/12 15:37:49  sll
# Updated from omni3_1_develop.
#
# Revision 1.9.2.1  2000/08/21 11:35:06  djs
# Lots of tidying
#
# Revision 1.9  2000/07/13 15:26:01  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.6.2.6  2000/06/26 16:23:26  djs
# Refactoring of configuration state mechanism.
#
# Revision 1.6.2.5  2000/05/31 18:02:50  djs
# Better output indenting (and preprocessor directives now correctly output at
# the beginning of lines)
#
# Revision 1.6.2.4  2000/04/26 18:22:19  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.6.2.3  2000/03/24 22:30:17  djs
# Major code restructuring:
#   Control flow is more recursive and obvious
#   Properly distinguishes between forward declarations and externs
#   Only outputs definitions once
#   Lots of assertions to check all is well
#
# Revision 1.6.2.2  2000/02/16 18:34:49  djs
# Fixed problem generating fragments in DynSK.cc file
#
# Revision 1.6.2.1  2000/02/14 18:34:56  dpg1
# New omniidl merged in.
#
# Revision 1.6  2000/01/20 18:26:43  djs
# Moved large C++ output strings into an external template file
#
# Revision 1.5  2000/01/17 17:06:30  djs
# Added tcParser #ifdefs for bounded strings
#
# Revision 1.4  2000/01/13 15:56:34  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.3  2000/01/12 11:52:09  djs
# Added important comment :)
#
# Revision 1.2  1999/12/09 20:40:13  djs
# TypeCode and Any generation option performs identically to old compiler for
# all current test fragments.
#
# Revision 1.1  1999/11/12 17:18:06  djs
# Skeleton of dynamic skeleton code :)
#

# All you really need to know for the moment is that the universe
# is a lot more complicated than you might think, even if you
# start from a position of thinking it's pretty damn complicated
# in the first place.       Douglas Adams, "Mostly Harmless"
#

# -----------------------------
# Configuration data
from omniidl_be.cxx import config

# -----------------------------
# Utility functions
from omniidl_be.cxx import ast, cxx, output, util

#import omniidl_be.cxx.dynskel.tcstring
import omniidl_be.cxx.dynskel.typecode
import omniidl_be.cxx.dynskel.main
import omniidl_be.cxx.dynskel.template

def generate(stream, tree):
    stream.out(template.header_comment,
               program = config.state['Program Name'],
               library = config.state['Library Version'])
    if not config.state['Fragment']:
        stream.out(template.header,
                   basename = config.state['Basename'],
                   hh = config.state['HH Suffix'],
                   library = config.state['Library Version'],
                   prefix = config.state['Private Prefix'])

    # This is the bit shared with the header file?
    tcstring = omniidl_be.cxx.dynskel.tcstring.__init__(stream)

    Typecode = omniidl_be.cxx.dynskel.typecode.__init__(stream)
    tree.accept(Typecode)

    Main = omniidl_be.cxx.dynskel.main.__init__(stream)
    tree.accept(Main)

def run(tree):
    # create somewhere to put the output
    header_filename = config.state['Basename'] +\
                      config.state['DYNSK Suffix']
    
    stream = output.Stream(output.createFile(header_filename), 2)

    generate(stream, tree)
