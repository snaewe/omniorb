# -*- python -*-
#                           Package   : omniidl
# tcstring.py               Created on: 2000/1/17
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
#   Produce bounded string #ifdefs for .hh 

# $Id$
# $Log$
# Revision 1.1  2000/01/17 17:06:30  djs
# Added tcParser #ifdefs for bounded strings
#

"""Produce bounded string #ifdefs for .hh"""

from omniidl import idlast, idltype, idlutil

from omniidl.be.cxx import tyutil, util, config, name

import tcstring

self = tcstring

def __init__(stream):
    self.stream = stream
    return self

# Control arrives here
#
def visitAST(node):
    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    # again check what happens here wrt reopening modules spanning
    # multiple files
    if not(node.mainFile()):
        return
    
    for n in node.definitions():
        n.accept(self)


def visitStruct(node):
    if not(node.mainFile()):
        return
    
    for n in node.members():
        n.accept(self)

def visitUnion(node):
    if not(node.mainFile()):
        return


def visitStringType(type):
    if type.bound() == 0:
        return
    stream.out("""\
#if !defined(___tc_string_@n@_value__) && !defined(DISABLE_Unnamed_Bounded_String_TC)
#define ___tc_string_@n@_value__
const CORBA::TypeCode_ptr _tc_string_@n@ = CORBA::TypeCode::PR_string_tc(@n@);
#endif
""", n = str(type.bound()))    

def visitAttribute(node):
    attrType = node.attrType()
    if tyutil.isString(attrType):
        attrType.accept(self)

def visitOperation(node):
    returnType = node.returnType()
    if tyutil.isString(returnType):
        returnType.accept(self)

    for p in node.parameters():
        paramType = p.paramType()
        if tyutil.isString(paramType):
            paramType.accept(self)
            
def visitInterface(node):
    if not(node.mainFile()):
        return

    for n in node.declarations():
        n.accept(self)

    for c in node.callables():
        c.accept(self)



def visitException(node):
    if not(node.mainFile()):
        return
    
    for n in node.members():
        n.accept(self)
        
def visitMember(node):
    if not(node.mainFile()):
        return
    
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    pass


def visitTypedef(node):
    pass
        
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass


