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
# Revision 1.10  2000/07/13 15:26:00  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.7.2.4  2000/06/26 16:23:59  djs
# Better handling of #include'd files (via new commandline options)
# Refactoring of configuration state mechanism.
#
# Revision 1.7.2.3  2000/04/26 18:22:30  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.7.2.2  2000/03/20 11:50:20  djs
# Removed excess buffering- output templates have code attached which is
# lazily evaluated when required.
#
# Revision 1.7.2.1  2000/02/14 18:34:54  dpg1
# New omniidl merged in.
#
# Revision 1.7  2000/01/19 11:23:28  djs
# Moved most C++ code to template file
#
# Revision 1.6  2000/01/17 17:03:14  djs
# Support for constructed types in typedefs and unions
#
# Revision 1.5  2000/01/13 15:56:39  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.4  2000/01/07 20:31:29  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.3  1999/12/24 18:14:30  djs
# Fixed handling of #include'd .idl files
#
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
from omniidl_be.cxx import tyutil, util, config, id, types
from omniidl_be.cxx.header import template

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
    # again, check what happens with reopened modules spanning
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

    # TypeCode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()

        stream.out(template.any_struct,
                   fqname = fqname)

def visitUnion(node):
    if not(node.mainFile()):
        return

    # deal with constructed switch type
    if node.constrType():
        node.switchType().decl().accept(self)

    # deal with constructed member types
    for n in node.cases():
        if n.constrType():
            n.caseType().decl().accept(self)
    
    # TypeCode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()
    
        stream.out(template.any_union,
                   fqname = fqname)


def visitMember(node):
    if not(node.mainFile()):
        return
    
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    if not(node.mainFile()):
        return

    cxx_fqname = id.Name(node.scopedName()).fullyQualify()
    
    # build the cases
    def cases(stream = stream, node = node):
        for d in node.enumerators():
            labelname = id.Name(d.scopedName()).fullyQualify()
            stream.out("case " + labelname + ":\n")

    stream.out(template.enum_operators,
               name = cxx_fqname,
               private_prefix = config.state['Private Prefix'],
               cases = cases)

    # Typecode and Any
    if config.state['Typecode']:
        stream.out(template.any_enum,
                   name = cxx_fqname)

def visitInterface(node):
    if not(node.mainFile()):
        return
    
    # interfaces act as containers for other declarations
    # output their operators here
    for d in node.declarations():
        d.accept(self)


    # Typecode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()
    
        stream.out(template.any_interface,
                   fqname = fqname)
        

def visitTypedef(node):
    if not(node.mainFile()):
        return
    
    aliasType = types.Type(node.aliasType())

    if node.constrType():
        aliasType.type().decl().accept(self)

    # don't need to do anything unless generating TypeCodes and Any
    if not(config.state['Typecode']):
        return
    
    for d in node.declarators():
        decl_dims = d.sizes()
        fqname = id.Name(d.scopedName()).fullyQualify()

        array_declarator = decl_dims != []

        if array_declarator:
            stream.out(template.any_array_declarator,
                       fqname = fqname)
        # only need to generate these operators if the typedef
        # introduces a new sequence- they already exist for a simple
        # typedef. Hence aliasType rather than deref_aliasType.
        elif aliasType.sequence():
            stream.out(template.any_sequence,
                       fqname = fqname)
            
            
        
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitException(node):
    if not(node.mainFile()):
        return

    for m in node.members():
        if m.constrType():
            m.memberType().decl().accept(self)
    
    # don't need to do anything unless generating TypeCodes and Any
    if not(config.state['Typecode']):
        return

    fqname = id.Name(node.scopedName()).fullyQualify()
    stream.out(template.any_exception,
               fqname = fqname)

