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
# Revision 1.15  2000/01/20 18:26:13  djs
# Tie template output order problem
#
# Revision 1.14  2000/01/20 12:46:40  djs
# Renamed a function to avoid a name clash with a module.
#
# Revision 1.13  2000/01/19 11:23:28  djs
# Moved most C++ code to template file
#
# Revision 1.12  2000/01/17 17:03:37  djs
# Support for module continuations
#
# Revision 1.11  2000/01/13 14:16:30  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.10  2000/01/11 11:34:28  djs
# Added support for fragment generation (-F) mode
#
# Revision 1.9  2000/01/10 17:18:14  djs
# Removed redundant code.
#
# Revision 1.8  2000/01/10 15:38:55  djs
# Better name and scope handling.
#
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
from omniidl.be.cxx import tyutil, name, env, config, util
from omniidl.be.cxx.header import tie
from omniidl.be.cxx.header import template

import poa

self = poa

def __init__(stream):
    self.__nested = 0
    poa.stream = stream
    return poa


def POA_prefix():
    if not(self.__nested):
        return "POA_"
    return ""


# Control arrives here
#
def visitAST(node):
    self.__completedModules = {}
    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    # again, check what happens with reopened modules spanning
    # multiple files
    if not(node.mainFile()):
        return

    slash_scopedName = string.join(node.scopedName(), '/')
    if self.__completedModules.has_key(slash_scopedName):
        return
    self.__completedModules[slash_scopedName] = 1
    
    name = tyutil.mapID(node.identifier())

    if not(config.FragmentFlag()):
        stream.out(template.POA_module_begin,
                   name = name,
                   POA_prefix = POA_prefix())
        stream.inc_indent()

    nested = self.__nested
    self.__nested = 1
    for n in node.definitions():
        n.accept(self)
    for c in node.continuations():
        slash_scopedName = string.join(c.scopedName(), '/')
        self.__completedModules[slash_scopedName] = 1
        for n in c.definitions():
            n.accept(self)

    self.__nested = nested

    if not(config.FragmentFlag()):
        stream.dec_indent()
        stream.out(template.POA_module_end)
    return

def visitInterface(node):
    if not(node.mainFile()):
        return
    
    iname = tyutil.mapID(node.identifier())
    environment = env.lookup(node)
    scope = tyutil.scope(node.scopedName())

    scopedName = scope + [iname]
    scopedID = idlutil.ccolonName(scopedName)
    impl_scopedID = name.prefixName(scopedName, "_impl_")

    POA_name = POA_prefix() + iname

    # build the normal POA class first
    stream.out(template.POA_interface,
               POA_name = POA_name,
               scopedID = scopedID,
               impl_scopedID = impl_scopedID)

    if config.TieFlag():
        tie.__init__(stream)
        tie.write_template(environment, node, self.__nested)
    return

def visitTypedef(node):
    pass
def visitEnum(node):
    pass
def visitStruct(node):
    pass
def visitUnion(node):
    pass
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitMember(node):
    pass
def visitException(node):
    pass
