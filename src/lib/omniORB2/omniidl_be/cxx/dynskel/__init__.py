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
from omniidl_be.cxx import tyutil, util

#import omniidl_be.cxx.dynskel.tcstring
import omniidl_be.cxx.dynskel.typecode
import omniidl_be.cxx.dynskel.main
import omniidl_be.cxx.dynskel.template

def monolithic(stream, tree):
    stream.out(template.header,
               Config = config)

    # This is the bit shared with the header file?
    tcstring = omniidl_be.cxx.dynskel.tcstring.__init__(stream)
    #tree.accept(tcstring)

    Typecode = omniidl_be.cxx.dynskel.typecode.__init__(stream)
    tree.accept(Typecode)

    Main = omniidl_be.cxx.dynskel.main.__init__(stream)
    tree.accept(Main)

def run(tree):
    # create somewhere to put the output
    header_filename = config.basename() + config.dynskelsuffix()
    
    stream = util.Stream(open(header_filename, "w"), 2)

    # generate one big chunk of header
    monolithic(stream, tree)

