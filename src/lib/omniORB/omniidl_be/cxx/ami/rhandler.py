# -*- python -*-
#                           Package   : omniidl
# rhandler.py               Created on: 2000/07/25
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
#   Generate AMI ReplyHandler for an IDL interface

import string
from omniidl import idlast, idltype
from omniidl_be.cxx import id, iface, output, call
from omniidl_be.cxx.ami import ami
import rhandler

self = rhandler

  
# Problem: The exceptionholder is a valuetype. It has been implemented
# as a base IDL struct (Messaging::ExceptionHolder) with methods added to
# the _type-specific_ valuetype by inheritance. For the _impl_ methods of
# the reply handler interface we have an internal function to cast the
# exceptionholder to the exact type.

impl_internal_method_t = """\
void _@op@(const Messaging::ExceptionHolder& _e)
  { @op@(*((const @exceptionholder@*)&_e)); }
"""


# AMI Type-Specific ReplyHandler class

class IHandler(iface.Interface):
    def __init__(self, node):
        iface.Interface.__init__(self, node)

        environment = id.lookup(node)
        
        # Grab the generic base ExceptionHolder
        meh_name = ["Messaging", "ExceptionHolder"]
        base_exception_holder = idlast.findDecl(meh_name)
        meh_type = idltype.Declared(base_exception_holder, meh_name,
                                    idltype.tk_struct)
        # Define the type specific one (as a struct)
        holder_name = ami.unique_name(self.name(), "ExceptionHolder",
                                      self._environment)
        holder_repoID = ami.unique_name_repoId(holder_name.simple(),
                                               base_exception_holder.repoId())
        tseh = ami.register_type_specific_exceptionholder(holder_name,
                                                          holder_repoID)
        tseh_type = idltype.Declared(tseh, holder_name.fullName(),
                                     idltype.tk_struct)
        environment.addName(holder_name.fullName(), allow_already_exists = 1)

        # this node inherits from Messaging::ReplyHandler
        self._inherits = [ idlast.findDecl(["Messaging", "ReplyHandler"]) ]

        # Set and register our name
        self._node_name = ami.unique_name(self.name(), "Handler",
                                          self._environment)
        environment.addName(self._node_name.fullName(),
                            allow_already_exists = 1)

        # The replyhandler has two callables per original IDL interface
        # callable. One for exceptional and one for normal replies.
        # The normal reply is ok, but we need to treat the ExceptionHolder
        # reply differently to cope with the lack of valuetypes.
        normal_callables = []
        exceptional_callables = []
        for c in self.callables():
            # normal reply
            out_params = filter(lambda x:x.is_out(), c.parameters())
            new_params = []
            
            returnType = c.returnType()
            voidType = idltype.Base(idltype.tk_void)
            
            if returnType.kind() != idltype.tk_void:
                new_params.append(ami.new_inherits_from(
                    idlast.Parameter, node, [0, returnType, "ami_return_val"]))

            # (changing all the parameters to -out-)
            for parameter in out_params:
                new_params.append(ami.new_inherits_from(
                    idlast.Parameter, node, [0, parameter.paramType(),
                                             parameter.identifier()]))
            operation = c.operation_name()

            normal_callables.append(
                call.Callable(self, operation, id.mapID(operation),
                              voidType, new_params, 0, [], []))

            # exceptional reply
            param = ami.new_inherits_from(idlast.Parameter, node,
                                          [0, tseh_type, "_eh"])
            operation = operation + "_excep"
            exceptional_callables.append(
                call.Callable(self, operation, id.mapID(operation),
                              voidType, [ param ], 0, [], []))


        self.normal_callables = normal_callables
        self.exceptional_callables = exceptional_callables
        self.exception_holder = holder_name

        # set the callable lists
        self._callables = normal_callables + exceptional_callables

        self.callable_map = {}
        for callable in self._callables:
            self.callable_map[callable.operation_name()] = callable

    def inherits(self):
        return map(lambda x:iface.Interface(x), self._inherits)

    def allInherits(self):
        return self.inherits()

    def callable_by_name(self, operation):
        return self.callable_map[operation]


class _objref_IHandler(iface._objref_I):
    def __init__(self, I):
        iface._objref_I.__init__(self, I)


class _impl_IHandler(iface._impl_I):
    def __init__(self, I):
        iface._impl_I.__init__(self, I)

        class FakeMethod:
            def __init__(self, name, exceptionholder):
                self.exceptionholder = exceptionholder
                self.name = name
            def hh(self, virtual = 1, pure = 1):
                fake = output.StringStream()
                fake.out(impl_internal_method_t, op = self.name,
                         exceptionholder = self.exceptionholder)
                return str(fake)
            def cc(self, stream, body):
                pass # it was inline

        # and for the exceptional replies, add an extra internal glue function
        for callable in self.interface().exceptional_callables:
            method = FakeMethod(callable.method_name(),
                                self.interface().exception_holder.simple())
            self._methods.append(method)
            self._callables[method] = callable


