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

from omniidl.be.cxx import tyutil, util, name, config

import poa

self = poa

def __init__(stream):
    poa.stream = stream
    return poa

# ------------------------------------
# environment handling functions

self.__environment = name.Environment()

self.__nested = 0

def POA_prefix():
    # is this different to the header?
    #return "POA_"
    if not(self.__nested):
        return "POA_"
    return ""

def enter(scope):
    self.__environment = self.__environment.enterScope(scope)
def leave():
    self.__environment = self.__environment.leaveScope()
def currentScope():
    return self.__environment.scope()


# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    name = tyutil.mapID(node.identifier())
    enter(name)
    scope = currentScope()

    for n in node.definitions():
        nested = self.__nested
        self.__nested = 1
        
        n.accept(self)

        self.__nested = nested

    leave()

def visitInterface(node):
    name = tyutil.mapID(node.identifier())
    environment = self.__environment
    fqname = environment.nameToString(node.scopedName())
    
    enter(name)
    scope = currentScope()
    
    stream.out("""\
POA_@fqname@::~@POA_prefix@@name@() {}""",
               POA_prefix = POA_prefix(),
               name = name,
               fqname = fqname)
        

    leave()

def visitTypedef(node):
    pass

def visitEnum(node):
    pass
def visitStruct(node):
    pass
def visitUnion(node):
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

