# -*- python -*-
#                           Package   : omniidl
# opers.py                  Created on: 1999/11/4
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2003-2006 Apasphere Ltd
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
# Revision 1.11.2.7  2009/05/06 16:15:44  dgrisby
# Update lots of copyright notices.
#
# Revision 1.11.2.6  2006/09/04 11:40:06  dgrisby
# Remove crazy switch code in enum marshalling.
#
# Revision 1.11.2.5  2006/06/14 10:35:03  dgrisby
# Problems with nested types in valuetypes.
#
# Revision 1.11.2.4  2004/07/04 23:53:38  dgrisby
# More ValueType TypeCode and Any support.
#
# Revision 1.11.2.3  2003/11/06 11:56:56  dgrisby
# Yet more valuetype. Plain valuetype and abstract valuetype are now working.
#
# Revision 1.11.2.2  2003/10/23 11:25:55  dgrisby
# More valuetype support.
#
# Revision 1.11.2.1  2003/03/23 21:02:37  dgrisby
# Start of omniORB 4.1.x development branch.
#
# Revision 1.9.2.4  2001/10/29 17:42:40  dpg1
# Support forward-declared structs/unions, ORB::create_recursive_tc().
#
# Revision 1.9.2.3  2001/06/08 17:12:17  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.9.2.2  2000/10/12 15:37:51  sll
# Updated from omni3_1_develop.
#
# Revision 1.10.2.1  2000/08/21 11:35:17  djs
# Lots of tidying
#
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
from omniidl_be.cxx import config, id, types, ast
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
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

def visitModule(node):
    for n in node.definitions():
        n.accept(self)

def visitStruct(node):
    for n in node.members():
        n.accept(self)

    # TypeCode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()

        stream.out(template.any_struct,
                   fqname = fqname)

def visitStructForward(node):
    pass

def visitUnion(node):
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

def visitUnionForward(node):
    pass

def visitMember(node):
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    cxx_fqname = id.Name(node.scopedName()).fullyQualify()
    last_item  = id.Name(node.enumerators()[-1].scopedName()).fullyQualify()

    stream.out(template.enum_operators,
               name = cxx_fqname,
               private_prefix = config.state['Private Prefix'],
               last_item = last_item)

    # Typecode and Any
    if config.state['Typecode']:
        stream.out(template.any_enum,
                   name = cxx_fqname)

def visitInterface(node):
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
    aliasType = types.Type(node.aliasType())

    if node.constrType():
        aliasType.type().decl().accept(self)

    # don't need to do anything unless generating TypeCodes and Any
    if not config.state['Typecode']:
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
    for m in node.members():
        if m.constrType():
            m.memberType().decl().accept(self)
    
    # don't need to do anything unless generating TypeCodes and Any
    if not config.state['Typecode']:
        return

    fqname = id.Name(node.scopedName()).fullyQualify()
    stream.out(template.any_exception,
               fqname = fqname)


def visitValue(node):
    # Nested declarations
    for d in node.declarations():
        d.accept(self)

    # Typecode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()
    
        stream.out(template.any_value,
                   fqname = fqname)

def visitValueForward(node):
    pass

def visitValueAbs(node):
    # Nested declarations
    for d in node.declarations():
        d.accept(self)

    # Typecode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()
    
        stream.out(template.any_value,
                   fqname = fqname)

def visitValueBox(node):
    # Typecode and Any
    if config.state['Typecode']:
        fqname = id.Name(node.scopedName()).fullyQualify()
    
        stream.out(template.any_value,
                   fqname = fqname)
