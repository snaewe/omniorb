# -*- python -*-
#                           Package   : omniidl
# mangler.py                Created on: 1999/11/16
#			    Author    : David Scott (djs)
#
#    Copyright (C) 1999-2000 AT&T Laboratories Cambridge
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
#   Produce mangled names for types and operation signatures
#   

# $Id$
# $Log$
# Revision 1.15.2.9  2002/11/21 16:12:52  dgrisby
# Oneway call descriptor bug.
#
# Revision 1.15.2.8  2001/11/27 14:37:25  dpg1
# long double TC descriptor.
#
# Revision 1.15.2.7  2001/11/13 15:23:52  dpg1
# Bug in forward declared structs/unions.
#
# Revision 1.15.2.6  2001/10/29 17:42:42  dpg1
# Support forward-declared structs/unions, ORB::create_recursive_tc().
#
# Revision 1.15.2.5  2001/06/08 17:12:20  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.15.2.4  2001/03/13 10:32:09  dpg1
# Fixed point support.
#
# Revision 1.15.2.3  2000/11/20 14:43:26  sll
# Added support for wchar and wstring.
#
# Revision 1.15.2.2  2000/10/12 15:37:53  sll
# Updated from omni3_1_develop.
#
# Revision 1.16.2.2  2000/09/14 16:03:59  djs
# Remodularised C++ descriptor name generator
#
# Revision 1.16.2.1  2000/08/21 11:35:33  djs
# Lots of tidying
#
# Revision 1.16  2000/07/13 15:25:59  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.13.2.5  2000/06/26 16:24:18  djs
# Refactoring of configuration state mechanism.
#
# Revision 1.13.2.4  2000/04/26 18:22:56  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
# Removed superfluous externs in front of function definitions
#
# Revision 1.13.2.3  2000/03/09 15:21:57  djs
# Better handling of internal compiler exceptions (eg attempts to use
# wide string types)
#
# Revision 1.13.2.2  2000/02/16 16:30:03  djs
# Fix to proxy call descriptor code- failed to handle special case of
#   Object method(in string x)
#
# Revision 1.13.2.1  2000/02/14 18:34:53  dpg1
# New omniidl merged in.
#
# Revision 1.13  2000/01/13 17:02:05  djs
# Added support for operation contexts.
#
# Revision 1.12  2000/01/13 15:56:44  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.11  2000/01/13 14:16:35  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.10  1999/12/17 10:48:11  djs
# Typedef to a sequence<sequence< name mangling bug
#
# Revision 1.9  1999/12/14 17:38:25  djs
# Fixed anonymous sequences of sequences bug
#
# Revision 1.8  1999/12/14 11:53:23  djs
# Support for CORBA::TypeCode and CORBA::Any
# Exception member bugfix
#
# Revision 1.7  1999/12/13 10:50:07  djs
# Treats the two call descriptors associated with an attribute separately,
# since it can happen that one needs to be generated but not the other.
#
# Revision 1.6  1999/12/10 18:27:05  djs
# Fixed bug to do with marshalling arrays of things, mirrored in the old
# compiler
#
# Revision 1.5  1999/12/09 20:40:58  djs
# Bugfixes and integration with dynskel/ code
#
# Revision 1.4  1999/12/01 16:59:01  djs
# Fixed name generation for attributes with user exceptions.
#
# Revision 1.3  1999/11/23 18:48:26  djs
# Bugfixes, more interface operations and attributes code
#
# Revision 1.2  1999/11/19 20:09:03  djs
# Fixed signature generating bug when return type is void
#
# Revision 1.1  1999/11/17 20:37:23  djs
# Code for call descriptors and proxies
#


from omniidl import idlast, idltype
from omniidl_be.cxx import types, id, skutil, util

import string

#######################################################################
## Produce cannonical names for types and operation signatures

# Separator constants
SCOPE_SEPARATOR         =    "_m"
ARRAY_SEPARATOR         =    "_a"
SEQ_SEPARATOR           =    "_s"
FORWARD_SEQ_SEPARATOR   =    "_f"
CANNON_NAME_SEPARATOR   =    "_c"
ONEWAY_SEPARATOR        =    "_w"
IN_SEPARATOR            =    "_i"
OUT_SEPARATOR           =    "_o"
INOUT_SEPARATOR         =    "_n"
EXCEPTION_SEPARATOR     =    "_e"

# Canonical names for basic types
name_map = {
    idltype.tk_short:       "short",
    idltype.tk_long:        "long",
    idltype.tk_longlong:    "longlong",
    idltype.tk_ushort:      "unsigned_pshort",
    idltype.tk_ulong:       "unsigned_plong",
    idltype.tk_ulonglong:   "unsigned_plonglong",
    idltype.tk_float:       "float",
    idltype.tk_double:      "double",
    idltype.tk_longdouble:  "longdouble",
    idltype.tk_char:        "char",
    idltype.tk_wchar:       "wchar",
    idltype.tk_boolean:     "boolean",
    idltype.tk_octet:       "octet",
    idltype.tk_void:        "void",
    idltype.tk_any:         "any",
    idltype.tk_TypeCode:    "TypeCode",
    }

# Produce a cannonical type name
def canonTypeName(type, decl = None, useScopedName = 0):
    assert isinstance(type, types.Type)

    type_dims = type.dims()
    decl_dims = []
    if decl != None:
        assert isinstance(decl, idlast.Declarator)
        decl_dims = decl.sizes()

    # flatten a list of dimensions into a string
    def dims(d):
        if d == []: return ""
        d_str = map(lambda x:ARRAY_SEPARATOR + str(x), d)
        return string.join(d_str, "")

    full_dims = decl_dims + type_dims
    is_array = full_dims != []
    dims_string = dims(full_dims)

    # The canonical type name always has the full dimensions
    # prepended to it.
    canon_name = dims_string

    d_type = type.deref()

    # consider anonymous sequence<sequence<....
    if type.sequence() and types.Type(type.type().seqType()).sequence():
        bound = type.type().bound()
        canon_name = canon_name + SEQ_SEPARATOR + str(bound) +\
                     canonTypeName(types.Type(type.type().seqType()), None,
                                   useScopedName)
        
        return canon_name
        

    # sometimes we don't want to call a sequence a sequence
    # (operation signatures)
    if useScopedName and not is_array and \
       type.typedef() and d_type.sequence():
        # find the last typedef in the chain
        while types.Type(type.type().decl().alias().aliasType()).typedef():
            type = types.Type(type.type().decl().alias().aliasType())
        scopedName = id.Name(type.type().scopedName()).guard()
        return canon_name + CANNON_NAME_SEPARATOR + scopedName

    # _arrays_ of sequences seem to get handled differently
    # to simple aliases to sequences
    if d_type.sequence():
        bound = d_type.type().bound()
        seqType = types.Type(d_type.type().seqType())

        if seqType.structforward() or seqType.unionforward():
            canon_name = canon_name + FORWARD_SEQ_SEPARATOR + str(bound)
        else:
            canon_name = canon_name + SEQ_SEPARATOR + str(bound)


        while seqType.sequence():
            bound = seqType.type().bound()
            canon_name = canon_name + SEQ_SEPARATOR + str(bound)
            seqType = types.Type(seqType.type().seqType())

        # straight forward sequences of sequences use their
        # flattened scoped name
        dkd_seqType = seqType.deref(keep_dims = 1)
        if not dkd_seqType.sequence():
            canon_name = canon_name + canonTypeName(dkd_seqType)
            return canon_name
        type = seqType
        d_type = type.deref()
        

    # add in the name for the most dereferenced type
    def typeName(type):
        assert isinstance(type, types.Type)
        d_type = type.deref()
        # dereference the type, until just -before- it becomes a
        # sequence. Since a sequence doesn't have a scopedName(),
        # we use the scopedName() of the immediately preceeding
        # typedef which is an instance of idltype.Declared
        while type.typedef() and \
              not types.Type(type.type().decl().alias().aliasType()).sequence():
            type = types.Type(type.type().decl().alias().aliasType())

        if name_map.has_key(type.type().kind()):
            return name_map[type.type().kind()]
        if type.string():
            bound = ""
            if type.type().bound() != 0:
                bound = str(type.type().bound())
            return bound + "string"
        if type.wstring():
            bound = ""
            if type.type().bound() != 0:
                bound = str(type.type().bound())
            return bound + "wstring"

        if isinstance(type.type(), idltype.Fixed):
            return str(type.type().digits()) + "_" + \
                   str(type.type().scale()) + "fixed"

        if isinstance(type.type(), idltype.Declared):
            return id.Name(type.type().scopedName()).guard()

        util.fatalError("Error generating mangled name")

    canon_name = canon_name + CANNON_NAME_SEPARATOR + typeName(type)
    return canon_name


def produce_signature(returnType, parameters, raises, oneway):

    returnType = types.Type(returnType)
    d_returnType = returnType.deref()

    # return type
    if d_returnType.void():
        sig = "void"
    else:
        sig = canonTypeName(returnType, useScopedName = 1)

    if oneway:
        # Can only validly happen with void return, but you never know
        # what the future may hold.
        sig = ONEWAY_SEPARATOR + sig
        
    # parameter list
    for param in parameters:
        if param.is_in() and param.is_out():
            sig = sig + INOUT_SEPARATOR
        elif param.is_in():
            sig = sig + IN_SEPARATOR
        elif param.is_out():
            sig = sig + OUT_SEPARATOR

        sig = sig + canonTypeName(types.Type(param.paramType()),
                                  useScopedName = 1)

    # exception list
    raises = skutil.sort_exceptions(raises)

    def exception_signature(exception):
        cname = CANNON_NAME_SEPARATOR +\
                id.Name(exception.scopedName()).guard()
        return EXCEPTION_SEPARATOR + cname
    
    raises_sigs = map(exception_signature, raises)
    raises_str = string.join(raises_sigs, "")

    sig = sig + raises_str
    return sig
            

