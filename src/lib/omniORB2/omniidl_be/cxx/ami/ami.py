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

from omniidl import idlast, idltype, idlvisitor
from omniidl_be.cxx import id, ast, types, config, descriptor

# We need to augment the AST tree with extra definitions before the
# naming environments are calculated so that later phases are aware of the
# new names we've added for AMI support.

# PseudoDecl and PseudoDeclRepoId represent Pseudo-IDL declarations stemming
# from an existing Decl node (eg an interface reply handler from an interface)
class PseudoDecl(idlast.Decl):
    """Pseudo-Decl object (derived from a real AST Decl"""
    def __init__(self, decl):
        idlast.Decl.__init__(self, decl.file(), decl.line(), decl.mainFile(),
                             decl.pragmas(), decl.comments())

class PseudoDeclRepoId(idlast.DeclRepoId):
    """AMI Pseudo IDL mixin"""
    def __init__(self, derived_from, suffix):
        environment = id.lookup(derived_from)
        # have to generate a unique name of the form
        # (AMI_)+nameSUFFIX (with sufficient prefixes to avoid a clash)
        prefix = "AMI_"
        name = id.Name(derived_from.scopedName()).suffix(suffix).prefix(prefix)
        while (len(name.relName(environment)) != 1):
            prefix = prefix + "AMI_"
            name.prefix(prefix)

        (p, scope, version) = ast.splitRepoId(derived_from.repoId())
        repoId = ast.joinRepoId((p, scope[0:-1] + [name.simple(cxx = 0)],
                                 version))
        idlast.DeclRepoId.__init__(self, name.simple(cxx = 0), name.fullName(),
                                   repoId)
        idlast.registerDecl(name.fullName(), self)

                                   
# ExceptionHolder, Poller, ReplyHandler are the three actual pseudo-IDL
# constructs required by AMI.
class ExceptionHolder(PseudoDecl, PseudoDeclRepoId, idlast.Struct):
    def __init__(self, interface):
        PseudoDecl.__init__(self, interface)
        PseudoDeclRepoId.__init__(self, interface, "ExceptionHolder")
        idlast.Struct.__init__(self, self.file(), self.line(), self.mainFile(),
                               self.pragmas(), self.comments(),
                               self.identifier(), self.scopedName(),
                               self.repoId(), 0)

        eh = idlast.findDecl(["Messaging", "ExceptionHolder"])
        self._setMembers(eh.members())

    def accept(self, visitor): pass


# Pretend the Poller is a variable-length struct
class Poller(PseudoDecl, PseudoDeclRepoId, idlast.Struct):
    def __init__(self, interface):
        PseudoDecl.__init__(self, interface)
        PseudoDeclRepoId.__init__(self, interface, "Poller")
        idlast.Struct.__init__(self, self.file(), self.line(),
                               self.mainFile(), self.pragmas(),
                               self.comments(), self.identifier(),
                               self.scopedName(), self.repoId(), 0)
        # Placing an exceptionholder inside will do the trick!
        # (contents not important, their variable-ness is)
        eh_members= idlast.findDecl(["Messaging", "ExceptionHolder"]).members()
        self._setMembers(eh_members)

    def accept(self, visitor):
        if hasattr(visitor, "visitDeclRepoId"):
            visitor.visitDeclRepoId(self)


class ReplyHandler(PseudoDecl, PseudoDeclRepoId, idlast.Interface):
    def __init__(self, interface):
        PseudoDecl.__init__(self, interface)
        PseudoDeclRepoId.__init__(self, interface, "Handler")
        idlast.Interface.__init__(self, self.file(), self.line(),
              self.mainFile(), self.pragmas(), self.comments(),
              self.identifier(), self.scopedName(), self.repoId(),
              0, [ idlast.findDecl(["Messaging", "ReplyHandler"]) ])


    def accept(self, visitor):
        if hasattr(visitor, "visitDeclRepoId"):
            visitor.visitDeclRepoId(self)
        

# Make a new declaration object using the same base Decl and DeclRepoId
def decl(decl_class, copy_from, new_args):
    args = [ copy_from.file(), copy_from.line(), copy_from.mainFile(),
             copy_from.pragmas(), copy_from.comments() ] + new_args
    return apply(decl_class, args)

# Helper function to make new "built in" IDL types:
def new_builtin(class_type, extra_arguments):
    args = ["<built in>", 0, 0, [], []] + extra_arguments
    return apply(class_type, args)

# Augmenting the tree cannot be done directly- we need to modify readonly
# internal data items. We must therefore copy the tree.

# make_new_decls_for: node -> node list, returns an expanded list of things
# for an interface, recursively processes everything else
def make_new_decls_for(node):
    if isinstance(node, idlast.Interface):
        eh = ExceptionHolder(node)
        p = Poller(node)
        rh = ReplyHandler(node)
        node.ExceptionHolder = eh
        node.Poller = p
        node.ReplyHandler = rh
        return [ eh, p, rh ] + [ augment(node) ]

    return [ augment(node) ]

# augment: node -> node, where modules have been supplimented with extra
# definition nodes if required.
def augment(node):
    if isinstance(node, idlast.AST):
        decls = [ Messaging_ExceptionHolder(), Messaging_ReplyHandler() ]
        for d in node.declarations():
            decls = decls + make_new_decls_for(d)
        return idlast.AST(node.file(), decls, node.pragmas(), node.comments())
    
    if isinstance(node, idlast.Module):
        defns = []
        for d in node.definitions():
            defns = defns + make_new_decls_for(d)
        return decl(idlast.Module, node, [ node.identifier(),
                                           node.scopedName(),
                                           node.repoId(),
                                           defns ])
    
    return node

def Messaging_ExceptionHolder():
    # Creates an IDL ExceptionHolder STRUCT, simulating the valuetype:
    #   value ExceptionHolder{
    #     boolean          is_system_exception;
    #     boolean          byte_order;
    #     sequence<octet>  marshalled_exception;
    #   };
    MEH = new_builtin(idlast.Struct,
                      [ "ExceptionHolder", ["Messaging", "ExceptionHolder"],
                        "IDL:omg.org/Messaging/ExceptionHolder:1.0", 0 ])
    m = []
    for name in ["is_system_exception", "byte_order"]:
        decl = new_builtin(idlast.Declarator,
                           [name, ["Messaging", "ExceptionHolder", name],
                            None, None])
        member = new_builtin(idlast.Member,
                             [ idltype.Base(idltype.tk_boolean), 0, [ decl ]])
        m.append(member)

    decl = new_builtin(idlast.Declarator,
                       ["marshaled_exception",
                        ["Messaging", "ExceptionHolder",
                         "marshaled_exception"],
                        None, None])
    sequence_octet = idltype.Sequence(idltype.Base(idltype.tk_octet), 0)
    
    marshalled_exception = new_builtin(idlast.Member,
                                       [sequence_octet, 0, [ decl ]])
    m.append(marshalled_exception)
           
    MEH._setMembers(m)

    idlast.registerDecl(MEH.scopedName(), MEH)

    return MEH

def Messaging_ReplyHandler():
    rh = idlast.Interface("<built in>", 0, 0, [], [],
                          "ReplyHandler", ["Messaging", "ReplyHandler"],
                          "IDL:omg.org/Messaging/ReplyHandler:1.0", 0, [])

    messaging = idlast.Module("<built in>", 0, 0, [], [], "Messaging",
                              ["Messaging"], "IDL:omg.org/Messaging:1.0",
                              [ rh ])

    idlast.registerDecl(rh.scopedName(), rh)
    idlast.registerDecl(messaging.scopedName(), messaging)

    return messaging


# Make a list of all the user exceptions that can be thrown by the
# operations of an IDL interface
def list_exceptions(node):
    assert isinstance(node, idlast.Interface)
    exceptions = []
    for callable in node.callables():
        if isinstance(callable, idlast.Operation):
            exceptions = exceptions + callable.raises()
    return exceptions



# Return an internal name for an AMI call descriptor
def call_descriptor(iface, opname, sig):
    iname = id.Name(iface.scopedName())
    return config.state["Private Prefix"] + "_amicd_" +\
           descriptor.get_interface_operation_descriptor(iname, opname, sig)


def poller_descriptor(iface, callable):
    iname = id.Name(iface.scopedName())
    opname = callable.operation_name()
    sig = callable.signature()
    return  config.state["Private Prefix"] + "_amipoller_" +\
           descriptor.get_interface_operation_descriptor(iname, opname, sig)

def servant(iface):
    name = string.join(iface.scopedName(), "_")
    return config.state["Private Prefix"] + "_amisvt_" +\
           descriptor.get_signature_descriptor(name)


def operation_argument(type, direction, ident):
    d_type = type.deref()
    
    if not(type.array()) and \
       (d_type.kind() in types.basic_map.keys() or d_type.enum()):
        return ident

    methods = [ ".in()", ".out()", ".inout()", "._retn()" ]
    
    d_type = type.deref()

    if type.kind() in types.basic_map.keys():
        return ident
    return ident + methods[direction]

def parameter(parameter):
    return operation_argument(types.Type(parameter.paramType()),
                              parameter.direction(),
                              id.mapID(parameter.identifier()))


def copy_to_OUT(pType, src, dest):
    out_type = operation_argument(pType, types.RET, src)
    return dest + " = " + out_type + ";"

