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
#   Produce the main skeleton definitions

# $Id$
# $Log$
# Revision 1.1  1999/11/12 17:18:58  djs
# Struct skeleton code added
#

"""Produce the main skeleton definitions"""
# similar to o2be_root::produce_skel in the old C++ BE

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

def __init__(stream):
    self.stream = stream
    return self

# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    name = tyutil.mapID(node.identifier())
#    enter(name)
    scope = currentScope()

    for n in node.definitions():
        n.accept(self)

#    leave()

def visitInterface(node):
    name = tyutil.mapID(node.identifier())
#    enter(name)
    scope = currentScope()
    

#    leave()

def visitTypedef(node):
    pass

def visitEnum(node):
    pass

def visitMember(node):
    memberType = node.memberType()
    if node.constrType():
        # if the type was declared here, it must be an instance
        # of idltype.Declared!
        assert isinstance(memberType, idltype.Declared)
        memberType.decl().accept(self)
        
def visitStruct(node):

    environment = self.__environment
    
    name = map(tyutil.mapID, node.scopedName())
    name = util.delimitedlist(name, "::")

    size_calculation = "omni::align_to(_msgsize, omni::ALIGN_4) + 4"

    marshall = util.StringStream()
    unmarshall = util.StringStream()
    msgsize = util.StringStream()
    
    for n in node.members():
        n.accept(self)

        for d in n.declarators():
            # marshall and unmarshall the struct members
            member_name = tyutil.name(d.scopedName())
            marshall.out("""\
  @member_name@ >>= _n;""", member_name = member_name)
            unmarshall.out("""\
  @member_name@ <<= _n;""", member_name = member_name)

            # computation of aligned size
            memberType = n.memberType()
            size = tyutil.sizeCalculation(environment, memberType, d,
                                          "_msgsize", member_name)
            msgsize.out("""\
  @size_calculation@;""", size_calculation = size)
            
            
            
    stream.out("""\
size_t
@name@::_NP_alignedSize(size_t _initialoffset) const
{
  CORBA::ULong _msgsize = _initialoffset;
  @size_calculation@
  return _msgsize;
}
""", name = name, size_calculation = str(msgsize))
    
    for where_to in ["NetBufferedStream", "MemBufferedStream"]:
        stream.out("""\
void
@name@::operator>>= (@where_to@ &_n) const
{
  @marshall_code@
}

void
@name@::operator<<= (@where_to@ &_n)
{
  @unmarshall_code@
}
""", name = name, where_to = where_to,
                   marshall_code = str(marshall),
                   unmarshall_code = str(unmarshall))

    
def visitUnion(node):
    pass
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitException(node):
    pass

