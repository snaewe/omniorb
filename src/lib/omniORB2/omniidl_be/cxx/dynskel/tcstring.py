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
# Revision 1.4.2.1  2000/07/17 10:35:45  sll
# Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
#
# Revision 1.5  2000/07/13 15:26:00  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.2.2.3  2000/06/26 16:23:27  djs
# Refactoring of configuration state mechanism.
#
# Revision 1.2.2.2  2000/04/26 18:22:20  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.2.2.1  2000/02/14 18:34:56  dpg1
# New omniidl merged in.
#
# Revision 1.2  2000/01/20 18:26:44  djs
# Moved large C++ output strings into an external template file
#
# Revision 1.1  2000/01/17 17:06:30  djs
# Added tcParser #ifdefs for bounded strings
#

"""Produce bounded string #ifdefs for .hh"""

from omniidl import idlast, idltype, idlutil
from omniidl_be.cxx import tyutil, util, id, types
from omniidl_be.cxx.dynskel import template

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
    stream.out(template.tc_string,
               n = str(type.bound()))    

def visitAttribute(node):
    attrType = types.Type(node.attrType())
    if attrType.string():
        attrType.type().accept(self)

def visitOperation(node):
    returnType = types.Type(node.returnType())
    if returnType.string():
        returnType.type().accept(self)

    for p in node.parameters():
        paramType = types.Type(p.paramType())
        if paramType.string():
            paramType.type().accept(self)
            
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


