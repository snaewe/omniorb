# -*- python -*-
#                           Package   : omniidl
# main.py                   Created on: 1999/11/12
#			    Author1   : David Scott (djs)
#                           Author2   : Duncan Grisby (dgrisby)
#
#  Copyright (C) 2004 Apasphere Ltd.
#  Copyright (C) 1999 AT&T Laboratories Cambridge
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
# Revision 1.16.2.5  2004/07/23 10:29:58  dgrisby
# Completely new, much simpler Any implementation.
#
# Revision 1.16.2.4  2004/07/04 23:53:38  dgrisby
# More ValueType TypeCode and Any support.
#
# Revision 1.16.2.3  2004/02/16 10:10:31  dgrisby
# More valuetype, including value boxes. C++ mapping updates.
#
# Revision 1.16.2.2  2003/10/23 11:25:55  dgrisby
# More valuetype support.
#
# Revision 1.16.2.1  2003/03/23 21:02:40  dgrisby
# Start of omniORB 4.1.x development branch.
#
# Revision 1.14.2.10  2001/11/27 14:37:25  dpg1
# long double TC descriptor.
#
# Revision 1.14.2.9  2001/10/29 17:42:38  dpg1
# Support forward-declared structs/unions, ORB::create_recursive_tc().
#
# Revision 1.14.2.8  2001/08/22 13:29:47  dpg1
# Re-entrant Any marshalling.
#
# Revision 1.14.2.7  2001/08/17 13:47:31  dpg1
# Small bug fixes.
#
# Revision 1.14.2.6  2001/06/08 17:12:14  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.14.2.5  2001/03/13 10:32:07  dpg1
# Fixed point support.
#
# Revision 1.14.2.4  2000/11/20 14:43:24  sll
# Added support for wchar and wstring.
#
# Revision 1.14.2.3  2000/11/03 19:21:35  sll
# idltype.Declared now takes an extra argument.
#
# Revision 1.14.2.2  2000/10/12 15:37:49  sll
# Updated from omni3_1_develop.
#
# Revision 1.15.2.2  2000/08/21 11:35:06  djs
# Lots of tidying
#
# Revision 1.15.2.1  2000/08/04 17:10:28  dpg1
# Long long support
#
# Revision 1.15  2000/07/13 15:26:00  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.12.2.10  2000/06/27 16:15:09  sll
# New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
# _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
# sequence of string and a sequence of object reference.
#
# Revision 1.12.2.9  2000/06/26 16:23:26  djs
# Refactoring of configuration state mechanism.
#
# Revision 1.12.2.8  2000/06/05 13:03:04  djs
# Removed union member name clash (x & pd_x, pd__default, pd__d)
# Removed name clash when a sequence is called "pd_seq"
#
# Revision 1.12.2.7  2000/05/31 18:02:50  djs
# Better output indenting (and preprocessor directives now correctly output at
# the beginning of lines)
#
# Revision 1.12.2.6  2000/04/26 18:22:20  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.12.2.5  2000/04/05 10:58:36  djs
# Missing function declaration when a union has a switch type declared in
# another file
#
# Revision 1.12.2.4  2000/03/24 22:30:18  djs
# Major code restructuring:
#   Control flow is more recursive and obvious
#   Properly distinguishes between forward declarations and externs
#   Only outputs definitions once
#   Lots of assertions to check all is well
#
# Revision 1.12.2.3  2000/03/20 11:48:16  djs
# Better handling of unions whose switch types are declared externally
#
# Revision 1.12.2.2  2000/03/15 20:49:18  djs
# Problem with typedefs to sequences or array declarators defined externally
# and used within a local struct or union.
# Refactoring of this code is now required....
#
# Revision 1.12.2.1  2000/02/14 18:34:56  dpg1
# New omniidl merged in.
#
# Revision 1.12  2000/01/20 18:26:44  djs
# Moved large C++ output strings into an external template file
#
# Revision 1.11  2000/01/19 11:23:48  djs
# *** empty log message ***
#
# Revision 1.10  2000/01/17 17:06:56  djs
# Better handling of recursive and constructed types
#
# Revision 1.9  2000/01/13 15:56:35  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.8  2000/01/13 14:16:24  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.7  2000/01/11 12:02:38  djs
# More tidying up
#
# Revision 1.6  2000/01/11 11:33:55  djs
# Tidied up
#
# Revision 1.5  2000/01/07 20:31:24  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.4  1999/12/24 18:16:39  djs
# Array handling and TypeCode building fixes (esp. across multiple files)
#
# Revision 1.3  1999/12/10 18:26:36  djs
# Moved most #ifdef buildDesc code into a separate module
# General tidying up
#
# Revision 1.2  1999/12/09 20:40:14  djs
# TypeCode and Any generation option performs identically to old compiler for
# all current test fragments.
#
# Revision 1.1  1999/11/12 17:18:07  djs
# Skeleton of dynamic skeleton code :)
#

"""Produce the main dynamic skeleton definitions"""

import string

from omniidl import idlast, idltype, idlutil
from omniidl_be.cxx import ast, cxx, output, util, id, types, config, skutil
from omniidl_be.cxx.skel import mangler
from omniidl_be.cxx.dynskel import template

import main

self = main

def __init__(stream):
    self.stream = stream
    return self


# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

# ------------------------------------

def visitModule(node):
    for n in node.definitions():
        n.accept(self)

# -----------------------------------

def visitConst(node):
    pass

# -----------------------------------

def visitInterface(node):
    for n in node.declarations():
        n.accept(self)

    scopedName = id.Name(node.scopedName())
    fqname     = scopedName.fullyQualify()
    guard_name = scopedName.guard()
    tc_name    = scopedName.prefix("_tc_").fullyQualify()
    prefix     = config.state['Private Prefix']

    if node.abstract():
        stream.out(template.abstract_interface,
                   guard_name = guard_name,
                   fqname = fqname, tc_name = tc_name,
                   private_prefix = prefix)
    else:
        stream.out(template.interface,
                   guard_name = guard_name,
                   fqname = fqname, tc_name = tc_name,
                   private_prefix = prefix)


def visitForward(node):
    pass

# -----------------------------------

def visitEnum(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    stream.out(template.enum,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)


# -----------------------------------

def visitStruct(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    # output code for constructed members (eg nested structs)
    for m in node.members():
        memberType = m.memberType()
        if m.constrType():
            memberType.decl().accept(self)

    stream.out(template.struct,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)

def visitStructForward(node):
    pass


# -----------------------------------

def visitException(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    # output code for constructed members (eg nested structs)
    for m in node.members():
        memberType = m.memberType()
        if m.constrType():
            memberType.decl().accept(self)

    stream.out(template.exception,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)

# -----------------------------------

def visitUnion(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    if node.constrType():
        node.switchType().decl().accept(self)

    for n in node.cases():
        if n.constrType():
            n.caseType().decl().accept(self)

    stream.out(template.union,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)

def visitUnionForward(node):
    pass

# -----------------------------------


def visitTypedef(node):
    if node.constrType():
        node.aliasType().decl().accept(self)

    aliasType = types.Type(node.aliasType())
    prefix    = config.state['Private Prefix']

    for d in node.declarators():
        scopedName = id.Name(d.scopedName())
        guard_name = scopedName.guard()
        fqname     = scopedName.fullyQualify()

        if d.sizes():
            # Array
            marshal = output.StringStream()
            skutil.marshall(marshal, None,
                            aliasType, d, "_a", "_s")

            unmarshal = output.StringStream()
            skutil.unmarshall(unmarshal, None,
                              aliasType, d, "_a", "_s")

            stream.out(template.array,
                       guard_name     = guard_name,
                       fqname         = fqname,
                       marshal        = marshal,
                       unmarshal      = unmarshal,
                       private_prefix = prefix)

        elif aliasType.sequence():
            stream.out(template.sequence,
                       guard_name     = guard_name,
                       fqname         = fqname,
                       private_prefix = prefix)


def visitValueForward(node):
    pass

def visitValue(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    for d in node.declarations():
        d.accept(self)
    for s in node.statemembers():
        memberType = s.memberType()
        if s.constrType():
            memberType.decl().accept(self)

    stream.out(template.value,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)

def visitValueBox(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    stream.out(template.value,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)

def visitValueAbs(node):
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname     = scopedName.fullyQualify()
    prefix     = config.state['Private Prefix']
    
    for d in node.declarations():
        d.accept(self)

    stream.out(template.value,
               guard_name     = guard_name,
               fqname         = fqname,
               private_prefix = prefix)
