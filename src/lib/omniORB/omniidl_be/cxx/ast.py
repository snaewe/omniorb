# -*- python -*-
#                           Package   : omniidl
# ast.py                    Created on: 2000/8/10
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2000 AT&T Laboratories Cambridge
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
#   Routines for manipulating the AST

# $Id$
# $Log$
# Revision 1.1.4.3  2001/03/13 10:34:01  dpg1
# Minor Python clean-ups
#
# Revision 1.1.4.2  2000/12/05 17:44:10  dpg1
# Remove dead repoId checking code.
#
# Revision 1.1.4.1  2000/10/12 15:37:46  sll
# Updated from omni3_1_develop.
#
# Revision 1.1.2.3  2000/09/21 16:35:54  djs
# *** empty log message ***
#
# Revision 1.1.2.2  2000/09/14 16:03:01  djs
# Remodularised C++ descriptor name generator
# Bug in listing all inherited interfaces if one is a forward
# repoID munging function now handles #pragma ID in bootstrap.idl
# Naming environments generating code now copes with new IDL AST types
# Modified type utility functions
# Minor tidying
#
# Revision 1.1.2.1  2000/08/21 11:34:32  djs
# Lots of omniidl/C++ backend changes
#

"""Routines for mangipulating the AST"""

from omniidl import idlast, idltype, idlvisitor
from omniidl_be.cxx import util

import string, re

import ast
self = ast

self.__initialised = 0

def __init__(tree):
    assert isinstance(tree, idlast.AST)
    self.__initialised = 1
    self.__ast = tree
    walker = WalkTreeForIncludes()
    tree.accept(walker)

# Returns the main IDL filename
def mainFile():
    assert(self.__initialised)
    return self.__ast.file()

# Returns a list of all IDL files included from this one
def includes():
    assert(self.__initialised)
    return ast._includes

# Traverses the tree making a list of all IDL files declarations have come
# from.
class WalkTreeForIncludes(idlvisitor.AstVisitor):
    def __init__(self):
        ast._includes = []
    def add(self, node):
        file = node.file()
        if not(file in ast._includes) and (file != "<built in>"):
            ast._includes.append(file)

    def visitAST(self, node):
        self.add(node)
        for d in node.declarations(): d.accept(self)
    def visitModule(self, node):
        self.add(node)
        for d in node.definitions(): d.accept(self)
    def visitInterface(self, node): self.add(node)
    def visitForward(self, node):   self.add(node)
    def visitConst(self, node):     self.add(node)
    def visitTypedef(self, node):   self.add(node)
    def visitStruct(self, node):    self.add(node)
    def visitException(self, node): self.add(node)
    def visitUnion(self, node):     self.add(node)
    def visitEnum(self, node):      self.add(node) 

# Returns the list of all non-default union case labels (ie those which
# have an explicit value)
def allCaseLabels(union):
    assert isinstance(union, idlast.Union)
    list = []
    for case in union.cases():
        for label in case.labels():
            if not(label.default()):
                list.append(label)
    return list

# Returns the list of all case label values
def allCaseLabelValues(union):
    labels = allCaseLabels(union)
    return map(lambda x:x.value(), labels)

# Returns the default case of a union or None if it doesn't exist
def defaultCase(union):
    assert isinstance(union, idlast.Union)
    default = None
    for case in union.cases():
        for label in case.labels():
            if label.default():
                default = case
    return default

# Adds a new attribute to all the union cases:
#  isDefault: boolean, true if case is the default one
def markDefaultCase(union):
    assert isinstance(union, idlast.Union)
    for case in union.cases():
        case.isDefault = 0
        for label in case.labels():
            if label.default():
                case.isDefault = 1

# Returns the default label of a case
def defaultLabel(case):
    assert isinstance(case, idlast.UnionCase)
    for label in case.labels():
        if label.default(): return label
    assert(0)

# raise x to the power of y (integer only)
def power(x, y):
    return long(x) ** y

# from CORBA 2.3 spec 3.10.1.1 Integer types
SHORT_MIN     = - power(2, (16 - 1))
SHORT_MAX     =   power(2, (16 - 1)) - 1
USHORT_MAX    =   power(2, 16) - 1

LONG_MIN      = - power(2, (32 - 1)) 
LONG_MAX      =   power(2, (32 - 1)) - 1
ULONG_MAX     =   power(2, 32) - 1

LONGLONG_MIN  = - power(2, (64 - 1))
LONGLONG_MAX  =   power(2, (64 - 1)) - 1
ULONGLONG_MAX =   power(2, 64) - 1

integer_type_ranges = {
    idltype.tk_short:     (SHORT_MIN, SHORT_MAX),
    idltype.tk_ushort:    (0, USHORT_MAX),
    idltype.tk_long:      (LONG_MIN, LONG_MAX),
    idltype.tk_ulong:     (0, ULONG_MAX),
    idltype.tk_longlong:  (LONGLONG_MIN, LONGLONG_MAX),
    idltype.tk_ulonglong: (0, ULONGLONG_MAX)
    }

# Returns true if the case values represent all the possible values for
# the supplied type.
def exhaustiveMatch(type, values):
    type = type.deref()

    # Note that we can only enumerate values for the types:
    #  integers, chars, boolean, enumeration

    # Returns true if everything from low to high is in set
    def in_set(low, high, set):
        test = low
        while (test <= high):
            if not(test in set): return 0
            test = test + 1
        return 1

    if type.integer():
        (low, high) = integer_type_ranges[type.kind()]
        return in_set(low, high, values)

    if type.char():
        return in_set(0, 255, map(ord, values))

    if type.boolean():
        if (0 in values) and (1 in values): return 1
        return 0

    if type.enum():
        all_enums = type.type().decl().enumerators()
        for enum in all_enums:
            if not(enum in values): return 0
        return 1

    raise "exhaustiveMatch type="+repr(type)+ \
          " val="+repr(discrimvalue)


# Return the base AST node after following all the typedef chains
def remove_ast_typedefs_(node, recurse):
    if isinstance(node, idlast.Declarator):
        typedef = node.alias()
        return recurse(typedef.aliasType().decl(), recurse)
    return node

def remove_ast_typedefs(node, chain = remove_ast_typedefs_):
    return chain(node, chain)

# Returns _all_ inherited interfaces by performing a breadth-first search
# of the inheritance graph.
def allInherits(interface):
    assert isinstance(interface, idlast.Interface)
    
    # It is possible to inherit from an interface through a chain of
    # typedef nodes. These need to be removed...

    # breadth first search
    def bfs(current, bfs, remove_typedefs = remove_ast_typedefs):
        if current == []:
            return []
        
        # extend search one level deeper than current
        next = []
        for c in map(remove_typedefs, current):
            if isinstance(c, idlast.Forward):
                c = c.fullDecl()
            next = next + c.inherits()

        return next + bfs(next, bfs)

    start = map(remove_ast_typedefs, interface.inherits())
    
    list = start + bfs(start, bfs)

    return util.setify(list)
