# -*- python -*-
#                           Package   : omniidl
# obv.py                    Created on: 2003/10/08
#			    Author    : Duncan Grisby
#
#    Copyright (C) 2003 Apasphere Ltd.
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
#   Produce the main header OBV definitions for the C++ backend

# $Id$
# $Log$
# Revision 1.1.2.3  2004/10/13 17:58:24  dgrisby
# Abstract interfaces support; values support interfaces; value bug fixes.
#
# Revision 1.1.2.2  2003/11/06 11:56:56  dgrisby
# Yet more valuetype. Plain valuetype and abstract valuetype are now working.
#
# Revision 1.1.2.1  2003/10/23 11:25:55  dgrisby
# More valuetype support.
#

"""Produce the main header OBV definitions for the C++ backend"""

import string

from omniidl import idlast, idltype, idlutil
from omniidl_be.cxx import id,  config, ast
from omniidl_be.cxx.header import tie, template

import obv

self = obv

def __init__(stream):
    self.__nested = 0
    obv.stream = stream
    return obv


def OBV_prefix():
    if not self.__nested:
        return "OBV_"
    return ""


# Control arrives here
#
def visitAST(node):
    self.__completedModules = {}
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

def visitModule(node):
    if self.__completedModules.has_key(node):
        return
    self.__completedModules[node] = 1
    
    name = id.mapID(node.identifier())

    if not config.state['Fragment']:
        stream.out(template.OBV_module_begin,
                   name = name,
                   OBV_prefix = OBV_prefix())
        stream.inc_indent()

    nested = self.__nested
    self.__nested = 1
    for n in node.definitions():
        n.accept(self)

    # Splice the continuations together if splice-modules flag is set
    # (This might be unnecessary as there (seems to be) no relationship
    #  between things in the POA module- they all point back into the main
    #  module?)
    if config.state['Splice Modules']:
        for c in node.continuations():
            for n in c.definitions():
                n.accept(self)
            self.__completedModules[c] = 1

    self.__nested = nested

    if not config.state['Fragment']:
        stream.dec_indent()
        stream.out(template.OBV_module_end)
    return

def visitInterface(node):
    pass
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
    from omniidl_be.cxx import value
    v = value.getValueType(node)

    v.obv_module_decls(stream, self)

def visitValueForward(node):
    from omniidl_be.cxx import value
    v = value.getValueType(node)

    v.obv_module_decls(stream, self)

def visitValueAbs(node):
    from omniidl_be.cxx import value
    v = value.getValueType(node)

    v.obv_module_decls(stream, self)

def visitValueBox(node):
    from omniidl_be.cxx import value
    v = value.getValueType(node)

    v.obv_module_decls(stream, self)
