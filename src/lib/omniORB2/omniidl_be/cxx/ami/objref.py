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
#   Responsible for adding sendc_ and sendp_ method calls

from omniidl import idlast, idltype
from omniidl_be.cxx import iface, id, types, call, cxx, output
from omniidl_be.cxx.ami import ami, calldesc, rhandler, poller

import string

sendc_cc_t = """\
omniAMICall *call_desc = new @name@(@args@);
AMI::enqueue(call_desc);
"""

sendp_cc_t = """\
// return this "valuetype" to the user
@poller_descriptor@ *poller = new @poller_descriptor@(this);

omniAMICall *call_desc = new @name@(@args@);
AMI::enqueue(call_desc);

return poller;
"""



# _objref class has a couple of extra methods for each operation invocation
class _objref_I(iface._objref_I):
    def __init__(self, I):
        iface._objref_I.__init__(self, I)

        # The name of the type-specific ReplyHandler
        self.rh_name = id.Name(I._node.ReplyHandler.scopedName())
        # .. and the type-specific Poller
        self.poller_name = id.Name(I._node.Poller.scopedName())

        self.build_extra_callables()

    def build_extra_callables(self):
        # For callback objref methods we add
        #     void         sendc_op(in AMI_IHANDLER, in args) 
        #     AMI_IPoller *sendp_op(in args)
        # For all operations and attributes

        self.sendc        = []
        self.sendc_nocopy = []
        self.sendp        = []
        self.sendp_nocopy = []
        
        voidType = idltype.Base(idltype.tk_void)
        
        # Get the type specific replyhandler        
        tsrh_decl = idlast.findDecl(self.rh_name.fullName())
        tsrh_type = idltype.declaredType(tsrh_decl, self.rh_name.fullName(),
                                         idltype.tk_objref)
        tsrh_param = idlast.Parameter("<pseudo>", 0, 0, [], [],
                                      0, tsrh_type, "ami_handler")
        # And the type specific poller
        tsp_decl = idlast.findDecl(self.poller_name.fullName())
        tsp_type = idltype.declaredType(tsp_decl, self.poller_name.fullName(),
                                        idltype.tk_struct)

        for callable in self.interface().callables():
            # all -in- and -inout- params are involved, but with a direction
            # of -in-
            in_params  = []
            for param in callable.parameters():
                if not(param.is_in()): continue
                new_param = ami.decl(idlast.Parameter, param, [ 0,
                                     param.paramType(), param.identifier() ])
                in_params.append(new_param)

            # sendc_ methods
            operation_name = "sendc_" + callable.operation_name()
            method_name = id.mapID(operation_name)
            c = call.Callable(self.interface(), operation_name,
                              method_name, voidType,
                              [tsrh_param] + in_params, 0, [],
                              callable.contexts())
            c.original = callable
            self.sendc.append(c)

            operation_name = "nocopy_sendc_" + callable.operation_name()
            method_name = id.mapID(operation_name)
            c_nocopy = call.Callable(self.interface(), operation_name,
                                     method_name, voidType,
                                     [tsrh_param] + in_params, 0, [],
                                     callable.contexts())
            c.nocopy = c_nocopy
            self.sendc_nocopy.append(c_nocopy)
        
            # sendp_ method
            operation_name = "sendp_" + callable.operation_name()
            method_name = id.mapID(operation_name)
            c = call.Callable(self.interface(), operation_name,
                              method_name, tsp_type,
                              in_params, 0, [], callable.contexts())
            c.original = callable
            self.sendp.append(c)

            operation_name = "nocopy_" + operation_name
            method_name = id.mapID(operation_name)
            c_nocopy = call.Callable(self.interface(), operation_name,
                                     method_name, tsp_type,
                                     in_params, 0, [], callable.contexts())
            c.nocopy = c_nocopy
            self.sendp_nocopy.append(c_nocopy)

        return


    def hh(self, stream):
        # Add the pseudo IDL methods in temporarily (signature generating
        # code is still useful)
        old_methods = self._methods[:]
        for extra in (self.sendc + self.sendc_nocopy +\
                      self.sendp + self.sendp_nocopy):
            method = iface._objref_Method(extra, self)
            self._methods.append(method)
            self._callables[method] = extra

        iface._objref_I.hh(self, stream)
            
        # turn state back to normal again
        self._methods = old_methods

    def cc(self, stream):
        iface._objref_I.cc(self, stream)

        # Generate all the op-specific Poller classes
        for callable in self.interface().callables():
            specific = poller.PollerOpSpecific(self.interface(), callable)
            specific.cc(stream)
            
        
        # Generate all required Call Descriptors (needed by sendc_, which
        # are themselves needed by sendp_)
        handler = rhandler.IHandler(self.interface()._node)
        for callable in self.sendc:
            name = callable.original.operation_name()
            reply_callable = handler.callable_by_name(name)
            exc_callable = handler.callable_by_name(name + "_excep")

            cd = calldesc._AMI_Call_Descriptor(self.interface(), handler,
                                               callable.original,
                                               reply_callable,
                                               exc_callable)
            callable.original.cd = cd
            
            cd.cc(stream)

        for callable in self.sendc:

            # sendc_ signatures take -in- arguments, we need to store these in
            # _var types inside the object. Depending on available operators
            # and the argument passing convention, we might need to * or & some
            # of these quantities
            op_arg_names = []
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

                op_arg_names.append(op + ident)

            arg_names = [ op_arg_names[0] ] + ["1", "this"] + op_arg_names[1:]
            method = iface._objref_Method(callable, self)
            body = output.StringStream()
            body.out(sendc_cc_t, name = callable.original.cd.descriptor,
                     args = string.join(arg_names, ", "))
            method.cc(stream, body)

            # the zero copy method as well
            arg_names = [ op_arg_names[0] ] + ["0", "this"] + op_arg_names[1:]
            method = iface._objref_Method(callable.nocopy, self)
            body = output.StringStream()
            body.out(sendc_cc_t, name = callable.original.cd.descriptor,
                     args = string.join(arg_names, ", "))
            method.cc(stream, body)
            

        for callable in self.sendp:
            method = iface._objref_Method(callable, self)
            Poller = id.Name(self.interface()._node.Poller.scopedName())
            Handler = id.Name(self.interface()._node.ReplyHandler.scopedName())

            # We get given parameters according to the C++ mapping, we
            # store _ptr types in the call descriptor.
            op_arg_names = []
            for p in callable.parameters():
                ident = id.mapID(parameter.identifier())
                pType = types.Type(parameter.paramType())
                op = ami.pointer(pType.op_is_pointer(types.direction(p)),
                                 pType._ptr_is_pointer())
                op_arg_names.append(op + ident)

            poller_descriptor = ami.poller_descriptor(self.interface()._node,
                                                      callable.original)
            servant = ami.servant(self.interface()._node)
            arg_names = [ "poller", "1", "this"] + op_arg_names
            body = output.StringStream()
            body.out(sendp_cc_t,
                     poller_t = Poller.simple(),
                     poller_descriptor = poller_descriptor,
                     name = callable.original.cd.descriptor,
                     args = string.join(arg_names, ", "))

            method.cc(stream, body)

            # and the zero copy method as well
            arg_names = [ "poller", "0", "this"] + op_arg_names
            method = iface._objref_Method(callable.nocopy, self)
            body = output.StringStream()
            body.out(sendp_cc_t, poller_t = Poller.simple(),
                     poller_descriptor = poller_descriptor,
                     name = callable.original.cd.descriptor,
                     args = string.join(arg_names, ", "))
            method.cc(stream, body)
        return
