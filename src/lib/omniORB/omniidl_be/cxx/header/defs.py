# -*- python -*-
#                           Package   : omniidl
# defs.py                   Created on: 1999/11/2
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
#   Produce the main header definitions for the C++ backend

# $Id$
# $Log$
# Revision 1.1  1999/11/03 11:09:56  djs
# General module renaming
#

"""Produce the main header definitions"""
# similar to o2be_root::produce_hdr_defs in the old C++ BE

from omniidl import idlast, idltype, idlutil, output

import defs

self = defs

# Not implemented yet:
# Flags which control the behaviour of the backend
isFragment = 0


def __init__(stream):
    defs.stream = stream
    return defs

#
# Control arrives here
#
def visitAST(node):
    print "visitAST(" + repr(node) + ")"
    print "stream = " + repr(stream)

    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    # o2be_module::produce_hdr
    name = node.identifier()
    if not(isFragment):
        stream.out("""\
_CORBA_MODULE @name@
_CORBA_MODULE_BEG""", name = name)
        stream.inc_indent()
  
    for n in node.definitions():
        pass

    if not(isFragment):
        stream.dec_indent()
        stream.out("""\
_CORBA_MODULE_END
""")
    node.written = name;


