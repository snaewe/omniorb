# -*- python -*-
#                           Package   : omniidl
# main.py                   Created on: 1999/11/12
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
#   Produce the main dynamic skeleton definitions

# $Id$
# $Log$
# Revision 1.1  1999/11/12 17:18:07  djs
# Skeleton of dynamic skeleton code :)
#

"""Produce the main dynamic skeleton definitions"""
# similar to o2be_root::produce_dynskel in the old C++ BE

from omniidl import idlast, idltype, idlutil

from omniidl.be.cxx import tyutil, util, name, config

import main

self = main

# ------------------------------------
# environment handling functions

self.__environment = name.Environment()

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
    

    leave()

def visitInterface(node):
    name = tyutil.mapID(node.identifier())
    enter(name)
    scope = currentScope()
    

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

