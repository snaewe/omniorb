# -*- python -*-
#                           Package   : omniidl
# poa.py                    Created on: 1999/11/4
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
#   Produce the main header POA definitions for the C++ backend

# $Id$
# $Log$
# Revision 1.1  1999/11/04 19:05:09  djs
# Finished moving code from tmp_omniidl. Regression tests ok.
#

"""Produce the main header POA definitions for the C++ backend"""

from omniidl import idlast, idltype, idlutil

from omniidl.be.cxx import tyutil, util

import poa

self = poa

def __init__(stream):
    poa.stream = stream
    return poa


self.__nested = 0
self.__scope = []

def enter(scope):
    self.__scope.append(scope)
def leave():
    self.__scope = self.__scope[0:-1]
def currentScope():
    return self.__scope

def POA_prefix():
    if not(self.__nested):
        return "POA_"
    return ""


# Control arrives here
#
def visitAST(node):
    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    name = tyutil.mapID(node.identifier())
    enter(name)
    scope = currentScope()
    
    stream.out("""\
_CORBA_MODULE @POA_prefix@@name@
_CORBA_MODULE_BEG
""",
               name = name,
               POA_prefix = POA_prefix())
    stream.inc_indent()

    for n in node.definitions():
        nested = self.__nested
        self.__nested = 1

        n.accept(self)

        self.__nested = nested

    stream.dec_indent()
    stream.out("""\
_CORBA_MODULE_END

""")
    leave()

def visitInterface(node):
    name = tyutil.mapID(node.identifier())
#    enter(name)
    scope = currentScope()
    
    scopedID = idlutil.ccolonName(scope + [name])
    scopedImplID = idlutil.ccolonName(scope + ["_impl_" + name])
    stream.out("""\
class @POA_prefix@@name@ :
  public virtual @scopedImplID@,
  public virtual PortableServer::ServantBase
{
public:
  virtual ~@POA_prefix@@name@();

  inline @scopedID@_ptr _this() {
    return (@scopedID@_ptr) _do_this(@scopedID@::_PD_repoId);
  }
};
""",
               name = name,
               scopedID = scopedID,
               scopedImplID = scopedImplID,
               POA_prefix = POA_prefix())
#    leave()

def visitTypedef(node):
    return
    scope = currentScope()
    
    aliasType = node.aliasType()
    derefType = tyutil.deref(aliasType)
    base = tyutil.principalID(aliasType, scope)
    for d in node.declarators():
        name = tyutil.mapID(d.identifier())
        if tyutil.isObjRef(derefType):
            stream.out("""\
typedef @POA_prefix@@base@ @POA_prefix@@name@;""",
                       POA_prefix = POA_prefix(),
                       base = base,
                       name = name)    
    

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
