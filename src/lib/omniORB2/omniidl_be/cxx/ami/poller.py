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
#   Create AMI Poller 'valuetype' for an interface (including the
#   internal ReplyHandler servant)

import string

from omniidl import idlvisitor, idlast, idltype
from omniidl_be.cxx import header, id, types, output, cxx, ast, iface
from omniidl_be.cxx.ami import ami, exholder
import poller

# Poller valuetype interface #########################################
#                                                                    #
# Note: The spec defines one Poller valuetype per IDL interface, with
# methods corresponding to each IDL operation and attribute in that
# interface. The poller is returned from a sendp_operation call
# and contains data _specific to that call_. So either:
#  * have one class with all the data possibilities (use enum & union?)
#  * have a derived class for each op with the ops implemented
# (both probably implemented the same way, second seems cleaner)
class_t = """\
// Type-specific AMI Poller valuetype
class @name@: public Messaging::Poller{
public:
  @exceptionholder@ *_exholder;

public:
  @name@(@target_ptr@ _target, const char* _name):
      Messaging::Poller(_target, _name), _exholder(NULL){ }
  virtual ~@name@(){ delete _exholder; }

  @methods@
};
"""

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

operation_t = """\
void @op@(@args@) 
{
  _NP_do_timeout(timeout);
  _NP_reply_obtained();
  if (_exholder) _exholder->@raise_fn@();

  @return_arguments@
}
"""

operation_proto_t = """\
void @op@(@args@);
"""

# FIXME: need to avoid duplication of signature specific things
# signature |----> call descriptor
#                  ami call descriptor
#                  local callback function
#                  poller valuetype instance

class Poller(iface.Class):
    def __init__(self, interface):
        iface.Class.__init__(self, interface)

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
                    args.append(pType.op(types.OUT, self._environment) + " "+\
                                parameter.identifier())

            method = "virtual void " + callable.method_name() + "(" +\
                     string.join(args, ", ") + ")" +\
                     "\n  {  _NP_wrong_transaction(); }"

            methods.append(method)
            
        stream.out(class_t, name = self.name().simple(),
                   target_ptr = self.interface().name().simple() + "_ptr",
                   exceptionholder = self._ehname.simple(),
                   methods = string.join(methods, "\n"))

    def cc(self, stream):
        return

class PollerOpSpecific(iface.Class):
    def __init__(self, interface, callable):
        iface.Class.__init__(self, interface)
        self._callable = callable

        self._poller = id.Name(interface._node.Poller.scopedName())
        self._name = self._poller.suffix("_" + callable.operation_name())

    def hh(self, stream):
        # internal only
        return

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
            if parameter.is_out():
                pType = types.Type(parameter.paramType())
                ident = id.mapID(parameter.identifier())
                arg_storage.out(pType._var() + " _" + ident + ";")
                copy.out(ami.copy_to_OUT(pType, "this->_" + ident, ident))
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


# Poller ReplyHandler internal servant ###############################
#                                                                    #
# This servant is registered with the AMI system to receive callback
# events when a polling call is made. It sets state inside the poller
# when things happen.
ReplyHandler_t = """\
// Poller valuetype contains an internal callback replyhandler.
class @servant@:
  public @poa_replyhandler@,
  public PortableServer::RefCountServantBase{
protected:
  @poller_base@ *_PD_poller;
  @replyhandler@_ptr _NP_narrow_handler(){
    return @replyhandler@::_narrow(_PD_poller->associated_handler());
  }
public:
  @servant@(@poller_base@ *poller): _PD_poller(poller) { }
  virtual ~@servant@() { }

  @methods@
};
"""

ReplyHandler_op_t = """\
void @op@(@args@){
  @replyhandler@_ptr _handler = _NP_narrow_handler();
  if (!_handler->_is_nil()) return _handler->@op@(@arglist@);

  @ts_poller@ *poller = (@ts_poller@*)_PD_poller;
  @copy_args_to_poller@
  _PD_poller->_NP_reply_received();
}
"""

ReplyHandler_op_excep_t = """\
void @op@_excep(const struct @exceptionholder@ &excep_holder){
  @replyhandler@_ptr _handler = _NP_narrow_handler();
  if (!_handler->_is_nil()) return _handler->@op@_excep(excep_holder);

  @ts_poller@ *poller = (@ts_poller@*)_PD_poller;
  poller->_exholder = new @exceptionholder@(excep_holder);
  poller->_NP_reply_received();
}
"""

class Poller_internal_servant(iface.Class):
    def __init__(self, interface):
        iface.Class.__init__(self, interface)

        node = self.interface()._node
        self._rhname = id.Name(node.ReplyHandler.scopedName())
        self._ehname = id.Name(node.ExceptionHolder.scopedName())
        self._name = id.Name(node.Poller.scopedName())

    def hh(self, stream):
        return

    def cc(self, stream):
        methods = output.StringStream()

        for callable in self.interface().callables():
            copy = output.StringStream()
            rType = types.Type(callable.returnType())
            args = []
            arglist = []
            if not(rType.void()):
                copy.out(rType.copy("_ami_return_val",
                                    "poller->_ami_return_val"))
                args.append(rType.op(types.IN) + " _ami_return_val")
                arglist.append("_ami_return_val")
                

            for parameter in callable.parameters():
                if parameter.is_out():
                    ident = parameter.identifier()
                    pType = types.Type(parameter.paramType())
                    copy.out(pType.copy(ident, "poller->" + ident,))
                    args.append(pType.op(types.IN) + " " + ident)
                    arglist.append(ident)

            tsname = self._name.suffix("_" + callable.operation_name())

            descriptor = ami.poller_descriptor(self.interface()._node,
                                               callable)
            
            methods.out(ReplyHandler_op_t,
                        op = id.mapID(callable.operation_name()),
                        ts_poller = descriptor,
                        args = string.join(args, ", "),
                        replyhandler = self._rhname.fullyQualify(),
                        arglist = string.join(arglist, ", "),
                        copy_args_to_poller = copy)
            methods.out(ReplyHandler_op_excep_t,
                        op = callable.operation_name(),
                        ts_poller = descriptor,
                        exceptionholder = self._ehname.fullyQualify(),
                        replyhandler = self._rhname.fullyQualify())

        descriptor = ami.servant(self.interface()._node)

        stream.out(ReplyHandler_t,
                   poller_base = self.name().fullyQualify(),
                   servant = descriptor,
                   poa_replyhandler = "POA_" + self._rhname.fullyQualify(),
                   replyhandler = self._rhname.fullyQualify(),
                   methods = methods)
                   



