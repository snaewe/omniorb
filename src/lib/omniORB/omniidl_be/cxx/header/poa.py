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
# Revision 1.17.2.4  2001/10/29 17:42:40  dpg1
# Support forward-declared structs/unions, ORB::create_recursive_tc().
#
# Revision 1.17.2.3  2001/06/08 17:12:17  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.17.2.2  2000/10/12 15:37:51  sll
# Updated from omni3_1_develop.
#
# Revision 1.18.2.2  2000/08/21 11:35:18  djs
# Lots of tidying
#
# Revision 1.18.2.1  2000/08/02 10:52:02  dpg1
# New omni3_1_develop branch, merged from omni3_develop.
#
# Revision 1.18  2000/07/13 15:26:00  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.15.2.9  2000/07/17 09:36:40  djs
# Now handles the case where an interface inherits from a typedef to another
# interface.
#
# Revision 1.15.2.8  2000/07/12 17:16:12  djs
# Minor bugfix to option -Wbsplice-modules
#
# Revision 1.15.2.7  2000/06/26 16:23:59  djs
# Better handling of #include'd files (via new commandline options)
# Refactoring of configuration state mechanism.
#
# Revision 1.15.2.6  2000/06/05 13:03:57  djs
# Removed union member name clash (x & pd_x, pd__default, pd__d)
# Removed name clash when a sequence is called "pd_seq"
# Nested union within union fix
# Actually generates BOA non-flattened tie templates
#
# Revision 1.15.2.5  2000/05/30 15:59:25  djs
# Removed inheritance ambiguity in generated BOA _sk_ and POA_ classes
#
# Revision 1.15.2.4  2000/05/04 14:35:04  djs
# Added new flag splice-modules which causes all continuations to be output
# as one lump. Default is now to output them in pieces following the IDL.
#
# Revision 1.15.2.3  2000/04/26 18:22:30  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.15.2.2  2000/04/05 10:57:39  djs
# Minor source tidying (removing commented out blocks)
#
# Revision 1.15.2.1  2000/02/14 18:34:54  dpg1
# New omniidl merged in.
#
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
from omniidl_be.cxx import id,  config, ast
from omniidl_be.cxx.header import tie, template

import poa

self = poa

def __init__(stream):
    self.__nested = 0
    poa.stream = stream
    return poa


def POA_prefix():
    if not self.__nested:
        return "POA_"
    return ""


# Control arrives here
#
def visitAST(node):
    self.__completedModules = {}
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

def visitModule(node):
    if self.__completedModules.has_key(node):
        return
    self.__completedModules[node] = 1
    
    name = id.mapID(node.identifier())

    if not config.state['Fragment']:
        stream.out(template.POA_module_begin,
                   name = name,
                   POA_prefix = POA_prefix())
        stream.inc_indent()

    nested = self.__nested
    self.__nested = 1
    for n in node.definitions():
        n.accept(self)

    # Splice the continuations together if splice-modules flag is set
    # (This might be unnecessary as there (seems to be) no relationship
    #  between things in the POA module- they all point back into the main
    #  module?)
    if config.state['Splice Modules']:
        for c in node.continuations():
            for n in c.definitions():
                n.accept(self)
            self.__completedModules[c] = 1

    self.__nested = nested

    if not config.state['Fragment']:
        stream.dec_indent()
        stream.out(template.POA_module_end)
    return

def visitInterface(node):
    iname = id.mapID(node.identifier())
    environment = id.lookup(node)
    scopedName = id.Name(node.scopedName())
    impl_scopedName = scopedName.prefix("_impl_")
    scopedID = scopedName.fullyQualify()
    impl_scopedID = impl_scopedName.fullyQualify()

    POA_name = POA_prefix() + iname

    # deal with inheritance
    inherits = []
    for i in map(ast.remove_ast_typedefs, node.inherits()):
        name = id.Name(i.scopedName())
        i_POA_name = name.unambiguous(environment)

        if name.relName(environment) == None:
            # we need to fully qualify from the root
            i_POA_name = "::POA_" + name.fullyQualify(environment)
            
        elif name.relName(environment) == i.scopedName():
            # fully qualified (but not from root) POA name has a POA_ on the
            # front
            i_POA_name = "POA_" + i_POA_name
            
        inherits.append("public virtual " + i_POA_name)

    # Note that RefCountServantBase is a mixin class specified by the
    # implementor, not generated by the idl compiler.
    if node.inherits() == []:
        inherits.append("public virtual PortableServer::ServantBase")

    inherits_str = string.join(inherits, ",\n  ")

    # build the normal POA class first
    stream.out(template.POA_interface,
               POA_name = POA_name,
               scopedID = scopedID,
               impl_scopedID = impl_scopedID,
               inherits = inherits_str)

    if config.state['Normal Tie']:
        # Normal tie templates, inline (so already in relevant POA_
        # module)
        poa_name = ""
        if len(scopedName.fullName()) == 1:
            poa_name = "POA_"
        poa_name = poa_name + scopedName.simple()
        tie_name = poa_name + "_tie"

        tie.write_template(tie_name, poa_name, node, stream)

    return

def visitTypedef(node):
    pass
def visitEnum(node):
    pass
def visitStruct(node):
    pass
def visitStructForward(node):
    pass
def visitUnion(node):
    pass
def visitUnionForward(node):
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
