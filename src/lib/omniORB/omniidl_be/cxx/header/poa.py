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
# Revision 1.7  2000/01/10 11:01:56  djs
# Forgot to keep track of names already defined causing a scoping problem.
#
# Revision 1.6  2000/01/07 20:31:29  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.5  1999/12/24 18:14:30  djs
# Fixed handling of #include'd .idl files
#
# Revision 1.4  1999/12/13 15:40:27  djs
# Added generation of "flattened" tie templates
#
# Revision 1.3  1999/12/12 14:02:47  djs
# Support for tie-templates added.
#
# Revision 1.2  1999/11/10 20:19:43  djs
# Array struct element fix
# Union sequence element fix
#
# Revision 1.1  1999/11/04 19:05:09  djs
# Finished moving code from tmp_omniidl. Regression tests ok.
#

"""Produce the main header POA definitions for the C++ backend"""

import string

from omniidl import idlast, idltype, idlutil

from omniidl.be.cxx import tyutil, name, config, util

from omniidl.be.cxx.header import tie

import poa

self = poa

def __init__(stream):
    poa.stream = stream
    return poa


self.__nested = 0

self.__environment = name.Environment()

def addName(name):
    #print "add " + name + " to " + str(self.__environment)
    try:
        self.__environment.add(name)
    except KeyError:
        pass
def enter(scope):
    # the exception is thrown in the case of a forward declared interface
    # being properly defined. Needs tidying up?
    addName(scope)
    self.__environment = self.__environment.enterScope(scope)
def leave():
    self.__environment = self.__environment.leaveScope()
def currentScope():
    return self.__environment.scope()

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
    # again, check what happens with reopened modules spanning
    # multiple files
    if not(node.mainFile()):
        return
    
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
    if not(node.mainFile()):
        return
    
    iname = tyutil.mapID(node.identifier())
#    enter(name)
    scope = currentScope()
    env = self.__environment

    scopedName = scope + [iname]
    scopedID = idlutil.ccolonName(scopedName)
    impl_scopedID = name.prefixName(scopedName, "_impl_")

    POA_name = POA_prefix() + iname

    # build the normal POA class first
    stream.out("""\
class @POA_name@ :
  public virtual @impl_scopedID@,
  public virtual PortableServer::ServantBase
{
public:
  virtual ~@POA_name@();

  inline @scopedID@_ptr _this() {
    return (@scopedID@_ptr) _do_this(@scopedID@::_PD_repoId);
  }
};
""",
               POA_name = POA_name,
               scopedID = scopedID,
               impl_scopedID = impl_scopedID)

    enter(node.identifier())
    if config.TieFlag():
        tie.template(env, node, self.__nested)
    leave()
    
#    leave()

def visitTypedef(node):
    for d in node.declarators():
        addName(d.identifier())


    return
    #scope = currentScope()
    # 
    #aliasType = node.aliasType()
    #derefType = tyutil.deref(aliasType)
    #base = tyutil.principalID(aliasType, scope)
    #for d in node.declarators():
    #    name = tyutil.mapID(d.identifier())
    #    if tyutil.isObjRef(derefType):
    #        stream.out("""\
#typedef @POA_prefix@@base@ @POA_prefix@@name@;""",
    #                   POA_prefix = POA_prefix(),
    #                   base = base,
    #                   name = name)    
    

def visitEnum(node):
    pass
def visitStruct(node):
    addName(node.identifier())
    pass
def visitUnion(node):
    addName(node.identifier())
    pass
def visitForward(node):
    addName(node.identifier())
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitMember(node):
    pass
def visitException(node):
    addName(node.identifier())
    pass
