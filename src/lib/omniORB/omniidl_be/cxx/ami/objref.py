# -*- python -*-
#                           Package   : omniidl
# objref.py                 Created on: 2000/08/08
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
#   Modified _objref_I for an IDL interface

from omniidl import idlast, idltype
from omniidl_be.cxx import iface, id, types, call, cxx, output
from omniidl_be.cxx.ami import ami, calldesc, rhandler

import string

sendc_cc_t = """\
omniAMICall *call_desc = new @name@(this, @args@);
AMI::enqueue(call_desc);
"""

# _objref class has a couple of extra methods for each operation invocation
class _objref_I(iface._objref_I):
    def __init__(self, I):
        iface._objref_I.__init__(self, I)

        # The name of the type-specific ReplyHandler
        self.ami_name = id.Name(I._node.ReplyHandler.scopedName())

        self.extra_callables = self.build_extra_callables()

    def build_extra_callables(self):
        # For callback objref methods we add
        #     void sendc_op(in AMI_IHANDLER, in args) for each operation
        # and
        #     void sendc_get_op(in AMI_IHANDLER,in arg)
        #     void sendc_set_op(in AMI_IHANDLER) for attributes

        extra = []
        # Get the type specific replyhandler
        voidType = idltype.Base(idltype.tk_void)
        
        tsrh_decl = idlast.findDecl(self.ami_name.fullName())
        tsrh_type = idltype.declaredType(tsrh_decl, self.ami_name.fullName(),
                                         idltype.tk_objref)
        tsrh_param = idlast.Parameter("<pseudo>", 0, 0, [], [],
                                      0, tsrh_type, "ami_handler")
        
        for callable in self.interface().callables():
            # all -in- and -inout- params are involved, but with a direction
            # of -in-
            in_params = [ tsrh_param ]
            for param in callable.parameters():
                if not(param.is_in()): continue
                new_param = ami.decl(idlast.Parameter, param, [ 0,
                                     param.paramType(), param.identifier() ])
                in_params.append(new_param)
            
            operation_name = "sendc_" + callable.operation_name()
            method_name = id.mapID(operation_name)
            c = call.Callable(self.interface(), operation_name,
                              method_name, voidType,
                              in_params, 0, [], callable.contexts())
            c.original = callable
            extra.append(c)

        return extra


    def hh(self, stream):
        # Add the pseudo IDL methods in temporarily
        old_methods = self._methods[:]
        for extra in self.extra_callables:
            method = iface._objref_Method(extra, self)
            self._methods.append(method)
            self._callables[method] = extra

        iface._objref_I.hh(self, stream)
            
        # turn state back to normal again
        self._methods = old_methods

    def cc(self, stream):
        iface._objref_I.cc(self, stream)
        
        # Generate all the sendc_ methods
        handler = rhandler.IHandler(self.interface()._node)
        for callable in self.extra_callables:
            name = callable.original.operation_name()
            reply_callable = handler.callable_by_name(name)
            exc_callable = handler.callable_by_name(name + "_excep")

            cd = calldesc._AMI_Call_Descriptor(self.interface(), handler,
                                               callable.original,
                                               reply_callable,
                                               exc_callable)
            cd.cc(stream)

            method = iface._objref_Method(callable, self)

            arg_names = []
            for parameter in callable.parameters():
                ident = id.mapID(parameter.identifier())
                pType = types.Type(parameter.paramType())
                # If we're using C++ pointers we must perform conversions
                # by hand (otherwise we can use _var, _ptr conversion ops)
                got  = pType.op_is_pointer(types.direction(parameter))
                need = pType._ptr_is_pointer()
                op = ""
                if got and not(need): op = "*"
                if need and not(got): op = "&"

                arg_names.append(op + ident)
                
            body = output.StringStream()
            body.out(sendc_cc_t, name = cd.descriptor,
                     args = string.join(arg_names, ", "))
            method.cc(stream, body)
            
        return
