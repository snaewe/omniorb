# -*- python -*-
#                           Package   : omniidl
# poller.py                 Created on: 2000/07/20
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
#   Create AMI Poller 'valuetype' for an interface
#
# $Id$
# $Log$
# Revision 1.1.2.5  2000/09/28 18:29:22  djs
# Bugfixes in Poller (wrt timout behaviour and is_ready function)
# Removed traces of Private POA/ internal ReplyHandler servant for Poller
# strategy
# Fixed nameclash problem in Call Descriptor, Poller etc
# Uses reference counting internally rather than calling delete()
# General comment tidying
#
#

import string

from omniidl import idlvisitor, idlast, idltype
from omniidl_be.cxx import header, id, types, output, cxx, ast, iface
from omniidl_be.cxx.ami import ami, exholder
import poller

# Poller valuetype interface #########################################
#
# Note: The spec defines one Poller valuetype per IDL interface, with
# methods corresponding to each IDL operation and attribute in that
# interface. The poller is returned from a sendp_operation call
# and contains data _specific to that call_. So either:
#  * have one class with all the data possibilities (use enum & union?)
#  * have a derived class for each op with the ops implemented
# (both probably implemented the same way, second seems cleaner)

# The interface-specific base class, which contains an exceptionholder
# pointer since that is also a per-interface thing.
# All the operation methods are set to raise CORBA::WRONG_TRANSACTION
# by default (behaviour overriden by the derived class)
class_t = """\
// Interface-specific AMI Poller valuetype
class @name@: public Messaging::Poller{
public:
  @exceptionholder@ *_exholder;

public:
  @name@(@target_ptr@ _target, const char* _name):
      Messaging::Poller(_target, _name), _exholder(NULL){ }
  virtual ~@name@(){ if (_exholder) _exholder->_remove_ref(); }

  @methods@
};
"""

default_operation_t = """\
virtual void @op@(@args@){
  _NP_throw_wrong_transaction();
}
"""

# The operation-specific final derived class can store arguments specific
# to this operation.
# This class is only visible inside the SK.cc file, so it doesn't matter
# that all of it's members are public.
specific_t = """\
// Operation specific AMI Poller valuetype
class @name@: public @inherits@{
public:
  @arg_storage@

  @name@(@target_ptr@ _target): @inherits@(_target, "@opname@"){ }
  @methods@
};
"""

# For every operation:
operation_t = """\
void @op@(@args@) {
  _NP_wait_and_throw_exception(timeout);
  if (_exholder) _exholder->@raise_fn@();

  @return_arguments@
}
"""

# Poller valuetype generation ########################################
#

# The base interface-specific class exists only in the header and comes
# complete with a load of pure virtual functions.
class Poller(iface.Class):
    def __init__(self, interface):
        iface.Class.__init__(self, interface)

        # Cache the scopedNames of the ExceptionHolder and Poller for this
        # IDL interface
        self._ehname = id.Name(interface._node.ExceptionHolder.scopedName())
        self._name = id.Name(interface._node.Poller.scopedName())

    def hh(self, stream):

        methods = []
        for callable in self.interface().callables():
            args = [ "CORBA::ULong timeout" ]
            rType = types.Type(callable.returnType())
            if not(rType.void()):
                args.append(rType.op(types.OUT, self._environment) +\
                            " _ami_return_val")
            for parameter in callable.parameters():
                if parameter.is_out():
                    # in the signature it's treated like an
                    # -out- parameter (even if it was originally
                    # -inout-)
                    pType = types.Type(parameter.paramType())
                    args.append(pType.op(types.OUT, self._environment))

            body = output.StringStream()
            body.out(default_operation_t, op = callable.method_name(),
                     args = string.join(args, ", "))

            methods.append(str(body))
            
        stream.out(class_t, name = self.name().simple(),
                   target_ptr = self.interface().name().simple() + "_ptr",
                   exceptionholder = self._ehname.simple(),
                   methods = string.join(methods, "\n"))

    def cc(self, stream): return


# The derived operation-specific class exists only in the SK.cc file and
# overrides the appropriate virtual function as well as providing operation
# specific argument storage.
class PollerOpSpecific(iface.Class):
    def __init__(self, interface, callable):
        iface.Class.__init__(self, interface)
        self._callable = callable

        self._poller = id.Name(interface._node.Poller.scopedName())
        #self._name = self._poller.suffix("_" + callable.operation_name())

    def cc(self, stream):
        # somewhere to put the received operation arguments
        arg_storage = output.StringStream()
        methods = output.StringStream()
        args = [ "CORBA::ULong timeout" ]
        copy = output.StringStream()
        
        rType = types.Type(self._callable.returnType())
        if not(rType.void()):
            arg_storage.out(rType._var() + " _ami_return_val;")
            copy.out(ami.copy_to_OUT(rType, "this->_ami_return_val",
                                     "_ami_return_val"))
            args.append(rType.op(types.OUT) +\
                        " _ami_return_val")

        for parameter in self._callable.parameters():
            ident = ami.paramID(parameter)
            if parameter.is_out():
                pType = types.Type(parameter.paramType())
                arg_storage.out(pType._var() + " " + ident + ";")
                copy.out(ami.copy_to_OUT(pType, "this->" + ident, ident))
                args.append(pType.op(types.OUT) + " " + ident)

        # the specific operation
        methods.out(operation_t,
                    op = self._callable.method_name(),
                    raise_fn = exholder.callable_raise_name(self._callable),
                    args = string.join(args, ", "),
                    return_arguments = copy)
        

        poller_descriptor = ami.poller_descriptor(self.interface()._node,
                                                  self._callable)

        stream.out(specific_t,
                   name = poller_descriptor,
                   target_ptr = self.interface().name().fullyQualify()+ "_ptr",
                   inherits = self._poller.fullyQualify(),
                   arg_storage = arg_storage,
                   methods = methods,
                   opname = self._callable.operation_name())





