# -*- python -*-
#                           Package   : omniidl
# poa.py                    Created on: 1999/11/12
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
#   Produce the main POA skeleton definitions

# $Id$
# $Log$
# Revision 1.11.2.2  2003/10/23 11:25:55  dgrisby
# More valuetype support.
#
# Revision 1.11.2.1  2003/03/23 21:02:34  dgrisby
# Start of omniORB 4.1.x development branch.
#
# Revision 1.9.2.4  2001/10/29 17:42:42  dpg1
# Support forward-declared structs/unions, ORB::create_recursive_tc().
#
# Revision 1.9.2.3  2001/06/08 17:12:20  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.9.2.2  2000/10/12 15:37:53  sll
# Updated from omni3_1_develop.
#
# Revision 1.10.2.1  2000/08/21 11:35:34  djs
# Lots of tidying
#
# Revision 1.10  2000/07/13 15:25:59  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.7.2.3  2000/06/26 16:24:18  djs
# Refactoring of configuration state mechanism.
#
# Revision 1.7.2.2  2000/04/26 18:22:56  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
# Removed superfluous externs in front of function definitions
#
# Revision 1.7.2.1  2000/02/14 18:34:53  dpg1
# New omniidl merged in.
#
# Revision 1.7  2000/01/19 17:05:16  djs
# Modified to use an externally stored C++ output template.
#
# Revision 1.6  2000/01/13 14:16:35  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.5  2000/01/10 18:42:22  djs
# Removed redundant code, tidied up.
#
# Revision 1.4  1999/12/25 21:47:19  djs
# Better TypeCode support
#
# Revision 1.3  1999/11/23 18:48:26  djs
# Bugfixes, more interface operations and attributes code
#
# Revision 1.2  1999/11/19 20:09:40  djs
# Added trivial POA interface code
#
# Revision 1.1  1999/11/12 17:18:58  djs
# Struct skeleton code added
#

"""Produce the main POA skeleton definitions"""
# similar to o2be_root::produce_poa_skel in the old C++ BE

from omniidl import idlast, idltype, idlutil
from omniidl_be.cxx import ast, cxx, id
from omniidl_be.cxx.skel import template

import poa

self = poa

def __init__(stream):
    poa.stream = stream

    self.__nested = 0
    return poa

def POA_prefix():
    if not self.__nested:
        return "POA_"
    return ""

# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

def visitModule(node):
    name = id.mapID(node.identifier())

    for n in node.definitions():
        nested = self.__nested
        self.__nested = 1
        
        n.accept(self)

        self.__nested = nested


def visitInterface(node):
    name = id.mapID(node.identifier())
    fqname = id.Name(node.scopedName()).fullyQualify()
    stream.out(template.interface_POA,
               POA_prefix = POA_prefix(),
               name = name,
               fqname = fqname)

def visitTypedef(node):
    pass

def visitEnum(node):
    pass
def visitStruct(node):
    pass
def visitStructForward(node):
    pass
def visitUnion(node):
    pass
def visitUnionForward(node):
    pass
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitMember(node):
    pass
def visitException(node):
    pass


def visitValue(node):
    # ***
    pass
