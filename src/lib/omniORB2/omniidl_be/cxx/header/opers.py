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
# Revision 1.2  1999/12/01 17:01:09  djs
# Moved ancillary marshalling and alignment code to another module
# Added operator overloads for Typecodes and Anys
#
# Revision 1.1  1999/11/04 19:05:09  djs
# Finished moving code from tmp_omniidl. Regression tests ok.
#

"""Produce the main header operator definitions"""
# similar to o2be_root::produce_hdr_operators in the old C++ BE

from omniidl import idlast, idltype, idlutil

from omniidl.be.cxx import tyutil, util, config, name

import opers

self = opers

def __init__(stream):
    opers.stream = stream
    return opers

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

    # TypeCode and Any
    if config.TypecodeFlag():
        env = name.Environment()
        fqname = env.nameToString(node.scopedName())
    
        stream.out("""\
extern void operator<<=(CORBA::Any& _a, const @fqname@& _s);
extern void operator<<=(CORBA::Any& _a, @fqname@* _sp);
extern CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@*& _sp);
extern CORBA::Boolean operator>>=(const CORBA::Any& _a, const @fqname@*& _sp);""",
                   fqname = fqname)

def visitUnion(node):

    # TypeCode and Any
    if config.TypecodeFlag():
        env = name.Environment()
        fqname = env.nameToString(node.scopedName())
    
        stream.out("""\
void operator<<=(CORBA::Any& _a, const @fqname@& _s);
void operator<<=(CORBA::Any& _a, @fqname@* _sp);
CORBA::Boolean operator>>=(const CORBA::Any& _a, const @fqname@*& _sp);
CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@*& _sp);""",
                   fqname = fqname)


def visitMember(node):
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    for s in ["NetBufferedStream", "MemBufferedStream"]:
        cxxname = idlutil.ccolonName(map(tyutil.mapID, node.scopedName()))
        stream.out("""\
inline void operator >>=(@name@ _e, @stream@& s) {
  ::operator>>=((CORBA::ULong)_e, s);
}

inline void operator <<= (@name@& _e, @stream@& s) {
  CORBA::ULong _0RL_e;
  ::operator<<=(_0RL_e,s);
  switch (_0RL_e) {""", name = cxxname, stream = s)
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
        _CORBA_marshal_error();""", name = cxxname)
        stream.dec_indent()
        stream.dec_indent()
        stream.out("""\
  }
}
""")
    # Typecode and Any
    if config.TypecodeFlag():
        stream.out("""\
void operator<<=(CORBA::Any& _a, @name@ _s);
CORBA::Boolean operator>>=(const CORBA::Any& _a, @name@& _s);""",
                   name = cxxname)

def visitInterface(node):
    # interfaces act as containers for other declarations
    # output their operators here
    for d in node.declarations():
        d.accept(self)


    # Typecode and Any
    if config.TypecodeFlag():
        env = name.Environment()
        fqname = env.nameToString(node.scopedName())
    
        stream.out("""\
void operator<<=(CORBA::Any& _a, @fqname@_ptr _s);
void operator<<=(CORBA::Any& _a, @fqname@_ptr* _s);
CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@_ptr& _s);
""", fqname = fqname)
        

def visitTypedef(node):
    # don't need to do anything unless generating TypeCodes and Any
    if not(config.TypecodeFlag()):
        return
    
    aliasType = node.aliasType()
    deref_aliasType = tyutil.deref(aliasType)
    type_dims = tyutil.typeDims(aliasType)

    env = name.Environment()
    for d in node.declarators():
        decl_dims = d.sizes()
        fqname = env.nameToString(d.scopedName())

        array_declarator = decl_dims != []

        if array_declarator:
            stream.out("""\
void operator<<=(CORBA::Any& _a, const @fqname@_forany& _s);
CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@_forany& _s);""",
                       fqname = fqname)
        # only need to generate these operators if the typedef
        # introduces a new sequence- they already exist for a simple
        # typedef. Hence aliasType rather than deref_aliasType.
        elif tyutil.isSequence(aliasType):
            stream.out("""\
extern void operator <<= (CORBA::Any& a, const @fqname@& s);
inline void operator <<= (CORBA::Any& a, @fqname@* sp) {
  a <<= *sp;
  delete sp;
}
extern _CORBA_Boolean operator >>= (const CORBA::Any& a, @fqname@*& sp);
extern _CORBA_Boolean operator >>= (const CORBA::Any& a, const @fqname@*& sp);
""", fqname = fqname)
            
            
        
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitException(node):
    # don't need to do anything unless generating TypeCodes and Any
    if not(config.TypecodeFlag()):
        return

    env = name.Environment()
    fqname = env.nameToString(node.scopedName())

    stream.out("""\
void operator<<=(CORBA::Any& _a, const @fqname@& _s);
void operator<<=(CORBA::Any& _a, const @fqname@* _sp);
CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@*& _sp);
""", fqname = fqname)

