# -*- python -*-
#                           Package   : omniidl
# marshal.py                Created on: 1999/12/1
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
#   Produce the main header alignment and marshal function definitions
#   for the C++ backend

# $Id$
# $Log$
# Revision 1.8.2.3  2003/11/06 11:56:56  dgrisby
# Yet more valuetype. Plain valuetype and abstract valuetype are now working.
#
# Revision 1.8.2.2  2003/10/23 11:25:55  dgrisby
# More valuetype support.
#
# Revision 1.8.2.1  2003/03/23 21:02:37  dgrisby
# Start of omniORB 4.1.x development branch.
#
# Revision 1.6.2.4  2001/10/29 17:42:40  dpg1
# Support forward-declared structs/unions, ORB::create_recursive_tc().
#
# Revision 1.6.2.3  2001/06/08 17:12:17  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.6.2.2  2000/10/12 15:37:51  sll
# Updated from omni3_1_develop.
#
# Revision 1.7.2.1  2000/08/21 11:35:17  djs
# Lots of tidying
#
# Revision 1.7  2000/07/13 15:26:00  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.4.2.2  2000/04/26 18:22:30  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.4.2.1  2000/02/14 18:34:55  dpg1
# New omniidl merged in.
#
# Revision 1.4  2000/01/19 11:23:28  djs
# Moved most C++ code to template file
#
# Revision 1.3  2000/01/07 20:31:28  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.2  1999/12/24 18:14:30  djs
# Fixed handling of #include'd .idl files
#
# Revision 1.1  1999/12/01 17:02:11  djs
# Moved ancillary marshalling and alignment code to this module from header.opers
#


"""Produce the main header alignment and marshal function definitions
  for the C++ backend"""

from omniidl import idlast, idltype, idlutil
from omniidl_be.cxx import config, id, ast
from omniidl_be.cxx.header import template

import marshal

self = marshal

def __init__(stream):
    marshal.stream = stream
    return marshal

# Control arrives here
#
def visitAST(node):
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

def visitModule(node):
    for n in node.definitions():
        n.accept(self)

def visitStruct(node):
    for n in node.members():
        n.accept(self)

def visitStructForward(node):
    pass

def visitUnion(node):
    # *** Recurse?
    pass

def visitUnionForward(node):
    pass

def visitMember(node):
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    pass

def visitInterface(node):
    # interfaces act as containers for other declarations
    # output their operators here
    for d in node.declarations():
        d.accept(self)

    name = id.Name(node.scopedName())
    cxx_name = name.fullyQualify()
    idLen = len(node.repoId()) + 1

    stream.out(template.interface_marshal_forward,
               name = cxx_name, idLen = str(idLen))        

def visitTypedef(node):
    pass
        
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitException(node):
    pass


def visitValue(node):
    # ***
    pass

def visitValueForward(node):
    # ***
    pass

def visitValueAbs(node):
    # ***
    pass

def visitValueBox(node):
    # ***
    pass
