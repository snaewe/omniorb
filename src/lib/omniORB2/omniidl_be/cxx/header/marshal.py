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
from omniidl.be.cxx import tyutil, util, config, name
from omniidl.be.cxx.header import template

import marshal

self = marshal

def __init__(stream):
    marshal.stream = stream
    return marshal

# Control arrives here
#
def visitAST(node):
    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    for n in node.definitions():
        n.accept(self)

def visitStruct(node):
    for n in node.members():
        n.accept(self)

def visitUnion(node):
    pass


def visitMember(node):
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    pass

def visitInterface(node):
    if not(node.mainFile()):
        return
    
    # interfaces act as containers for other declarations
    # output their operators here
    for d in node.declarations():
        d.accept(self)

    cxxname = idlutil.ccolonName(map(tyutil.mapID, node.scopedName()))
    idLen = len(tyutil.mapRepoID(node.repoId())) + 1
    stream.out(template.interface_marshal_forward,
               name = cxxname, idLen = str(idLen))        

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

