# -*- python -*-
#                           Package   : omniidl
# ami.py                    Created on: 2000/07/20
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
#   General AMI utility functions

import string

from omniidl import idlast, idltype
from omniidl_be.cxx import id, ast, types

# The spec says that the names for typespecific entities should
# be AMI_nameSUFFIX with AMI_ prefixes added until the name is
# unique
def unique_name(name, suffix, environment):
    prefix = "AMI_"
    ami_name = name.suffix(suffix).prefix(prefix)
    while (len(ami_name.relName(environment)) != 1):
        # not unique yet
        prefix = prefix + "AMI_"
        ami_name.prefix(prefix)

    return ami_name

# The Pseudo IDL things have their own repoId (first class entities)
def unique_name_repoId(name, repoId):
    (scope, version) = ast.splitRepoId(repoId)
    return ast.joinRepoId((scope[0:-1] + [name], version))

# Make a list of all the user exceptions that can be thrown by the
# operations of an IDL interface
def list_exceptions(node):
    assert isinstance(node, idlast.Interface)
    exceptions = []
    for callable in node.callables():
        if isinstance(callable, idlast.Operation):
            exceptions = exceptions + callable.raises()
    return exceptions

# Code to register new IDL entities:
# Helper function to make new built in IDL types:
def new_builtin(class_type, extra_arguments):
    args = ["<built in>", 0, 0, [], []] + extra_arguments
    return apply(class_type, args)

# Helper function to make new pseudo-IDL types, `inheriting' their
# file, line, mainFile, pragmas and comments fields from the parent.
def new_inherits_from(class_type, parent, extra_arguments):
    args = [ parent.file(), parent.line(), parent.mainFile(),
             parent.pragmas(), parent.comments() ] + extra_arguments

    return apply(class_type, args)

# Registers the Messaging::ExceptionHolder valuetype with the system (as an
# IDL struct)
#              value ExceptionHolder{
#                boolean          is_system_exception;
#                boolean          byte_order;
#                sequence<octet>  marshalled_exception;
#              };
def register_Messaging_ExceptionHolder():
    MEH = new_builtin(idlast.Struct,
                      [ "ExceptionHolder", ["Messaging", "ExceptionHolder"],
                        "IDL:omg.org/Messaging/ExceptionHolder:1.0", 0 ])
    m = []
    for name in ["is_system_exception", "byte_order"]:
        decl = new_builtin(idlast.Declarator,
                           [name, ["Messaging", "ExceptionHolder", name],
                            None, None])
        member = new_builtin(idlast.Member,
                             [ idltype.Base(idltype.tk_boolean), 0, decl])
        m.append(member)

    decl = new_builtin(idlast.Declarator,
                       ["marshaled_exception", ["Messaging", "ExceptionHolder",
                                                "marshaled_exception"],
                        None, None])
    sequence_octet = idltype.Sequence(idltype.Base(idltype.tk_octet), 0)
    
    marshalled_exception = new_builtin(idlast.Member,[sequence_octet, 0, decl])
    m.append(marshalled_exception)
           
    MEH._setMembers(m)
    idlast.registerDecl(["Messaging", "ExceptionHolder"], MEH)
    id.predefine_decl(MEH)

# Registers the Messaging::ReplyHandler interface with the system
def register_Messaging_ReplyHandler():
    rh = idlast.Interface("<built in>", 0, 0, [], [],
                          "ReplyHandler", ["Messaging", "ReplyHandler"],
                          "IDL:omg.org/Messaging/ReplyHandler:1.0", 0, [])
    idlast.registerDecl(["Messaging", "ReplyHandler"], rh)

    id.predefine_decl(rh)

# The type specific reply handler for an IDL interface, I, is an interface
# AMI_IHandler: Messaging::ReplyHandler {}
def register_type_specific_replyhandler(node):
    assert isinstance(node, idlast.Interface)
    environment = id.lookup(node)
    node_name = id.Name(node.scopedName())
    ami_rhname = unique_name(node_name, "Handler", environment)

    # this doesn't exist in IDL, so make a fake variable length struct
    repoId = unique_name_repoId(ami_rhname.simple(), node.repoId())
    inherits = idlast.findDecl(["Messaging", "ReplyHandler"])
    interface = idlast.Interface("<built in>", 0, 0, [], [],
                                 ami_rhname.simple(), ami_rhname.fullName(),
                                 repoId, 0, [inherits])
    idlast.registerDecl(ami_rhname.fullName(), interface)

def register_type_specific_exceptionholder(name, repoId):
    # make effectively equivalent to the base type with a different name
    meh = ["Messaging", "ExceptionHolder"]
    meh_decl = idlast.findDecl(meh)
    meh_members = meh_decl.members()

    tseh = new_inherits_from(idlast.Struct, meh_decl,
                             [ name.simple(), name.fullName(), repoId, 0])
    tseh._setMembers(meh_members)
    idlast.registerDecl(name.fullName(), tseh)
    return tseh


# Return a C++ type declaration for a pointer to a thing of type T
def pointer(T, environment = None):
    assert isinstance(T, types.Type)
    d_T = T.deref() # grab the fundamental type
    if T.array():
        # handle by pointer to array slice
        name = id.Name(T.type().decl().scopedName()).suffix("_slice")
        return name.unambiguous(environment) + "*"

    elif d_T.objref():
        # handle by T_ptr type
        name = id.Name(d_T.type().decl().scopedName()).suffix("_ptr")
        return name.unambiguous(environment) + "*"

    elif d_T.sequence():
        # (not the sequence template)
        name = id.Name(T.type().decl().scopedName())
        return name.unambiguous(environment) + "*"

    elif d_T.string():
        return "CORBA::String_ptr"

    else:
        return T.base(environment) + "*"


# Return a C++ type declaration for a value of type T
def value(T, environment = None):
    assert isinstance(T, types.Type)
    d_T = T.deref() # grab the fundamental type
    if T.array():
        # arrays by value?
        raise "Cannot pass arrays by value"

    elif d_T.objref():
        # objrefs are always _refs_
        raise "Cannot pass objrefs by value either"

    elif d_T.sequence():
        # (not the sequence template)
        name = id.Name(T.type().decl().scopedName())
        return name.unambiguous(environment)

    elif d_T.string():
        return "char*"

    else:
        return T.base(environment)


# Generate code to delete a thing of type T called name
def delete(T, name, environment = None):
    assert isinstance(T, types.Type)
    d_T = T.deref() # grab the fundamental type
    if T.array():
        # use the T_free(T_slice* method)
        name = id.Name(T.type().decl().scopedName())
        return name.unambiguous(environment) + "_free(" + name + ");"

    elif d_T.objref():
        # need to CORBA::release() the object reference
        return "CORBA::release(" + name + ");"

    elif d_T.sequence():
        return "delete " + name + ";"

    elif d_T.string():
        return "CORBA::String_free(" + name + ");"
    
    return "delete " + name + ";"


# Generate code to copy an instance of a type T
def copy(T, src, dest, environment = None):
    assert isinstance(T, types.Type)
    d_T = T.deref()
    if T.array():
        # use the T_dup(T_slice*) method
        name = id.Name(T.type().decl().scopedName())
        return dest + " = " + name.unambiguous(environment) + "_dup" + \
               "(" + src + ");"

    elif d_T.objref():
        # need to use I::_duplicate(I_ptr)
        name = id.Name(T.type().decl().scopedName())
        return dest + " = " + name.unambiguous(environment) + "::_duplicate" +\
               "(" + src + ");"

    elif d_T.sequence():
        # use the default copy constructor
        name = id.Name(T.type().decl().scopedName())
        return dest + " = new " + name.unambiguous(environment) +\
               "(*" + src + ");"

    elif d_T.string():
        return dest + " = CORBA::String_dup(" + src + ");"

    return dest + " = new " + T.base(environment) + "(*" + src + ");"


