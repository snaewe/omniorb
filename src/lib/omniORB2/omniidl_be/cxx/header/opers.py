# -*- python -*-
#                           Package   : omniidl
# opers.py                  Created on: 1999/11/4
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
#   Produce the main header operator definitions for the C++ backend

# $Id$
# $Log$
# Revision 1.1  1999/11/04 19:05:09  djs
# Finished moving code from tmp_omniidl. Regression tests ok.
#

"""Produce the main header operator definitions"""
# similar to o2be_root::produce_hdr_operators in the old C++ BE

from omniidl import idlast, idltype, idlutil

from omniidl.be.cxx import tyutil, util

import opers

self = opers

def __init__(stream):
    opers.stream = stream
    return opers

# Control arrives here
#
def visitAST(node):
    orderedList = node.declarations()[:]
    orderedList.sort(partialOrder)
    for n in orderedList:
        n.accept(self)

# The old back end generates the code for all enums at a level
# before the interfaces.
# partialOrder defines a partial ordering over the set of AST node
# types. Any topological sort of such an ordered set will produce
# the same output as the old BE.
def partialOrder(a, b):
    if isinstance(a, idlast.Enum) and isinstance(b, idlast.Interface): return -1
    if isinstance(a, idlast.Interface) and isinstance(b, idlast.Enum): return 1
    return 0


def visitModule(node):
    orderedList = node.definitions()[:]
    orderedList.sort(partialOrder)
    for n in orderedList:
        n.accept(self)

def visitStruct(node):
    for n in node.members():
        n.accept(self)

def visitMember(node):
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    for s in ["NetBufferedStream", "MemBufferedStream"]:
        name = idlutil.ccolonName(map(tyutil.mapID, node.scopedName()))
        stream.out("""\
inline void operator >>=(@name@ _e, @stream@& s) {
  ::operator>>=((CORBA::ULong)_e, s);
}

inline void operator <<= (@name@& _e, @stream@& s) {
  CORBA::ULong _0RL_e;
  ::operator<<=(_0RL_e,s);
  switch (_0RL_e) {""", name = name, stream = s)
        stream.inc_indent()
        for d in node.enumerators():
            labelname = idlutil.ccolonName(map(tyutil.mapID,
                                               d.scopedName()))
            stream.out("""\
     case @label@:""", label = labelname)
        stream.inc_indent()
        stream.out("""\
        _e = (@name@) _0RL_e;
        break;
     default:
        _CORBA_marshal_error();""", name = name)
        stream.dec_indent()
        stream.dec_indent()
        stream.out("""\
  }
}
""")

def visitInterface(node):
    # interfaces act as containers for other declarations
    # output their operators here
    for d in node.declarations():
        d.accept(self)
            
    name = idlutil.ccolonName(map(tyutil.mapID, node.scopedName()))
    idLen = len(node.repoId()) + 1
    stream.out("""\
inline size_t
@name@::_alignedSize(@name@_ptr obj, size_t offset) {
  return CORBA::AlignedObjRef(obj, _PD_repoId, @idLen@, offset);
}

inline void
@name@::_marshalObjRef(@name@_ptr obj, NetBufferedStream& s) {
  CORBA::MarshalObjRef(obj, _PD_repoId, @idLen@, s);
}

inline void
@name@::_marshalObjRef(@name@_ptr obj, MemBufferedStream& s) {
  CORBA::MarshalObjRef(obj, _PD_repoId, @idLen@, s);
}

""", name = name, idLen = str(idLen))        
               
        
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
def visitTypedef(node):
    pass
