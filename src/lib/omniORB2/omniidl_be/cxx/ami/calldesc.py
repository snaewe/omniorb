# -*- python -*-
#                           Package   : omniidl
# calldesc.py               Created on: 2000/07/25
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
#   Internal AMI call descriptor used to package up the state of an
#   AMI request

import string

from omniidl_be.cxx import id, types, output, cxx, ast, call, iface
from omniidl_be.cxx.ami import ami

# Operation-specific AMI call descriptor  ############################
#                                                                    #
call_descriptor_t = """\
class @classname@: public omniAMICall{
private:
  @target_t@_var  target;
  @handler_t@_var handler;

  // Argument storage
  @arg_storage@

public:
  @classname@(@constructor_args@){
    @constructor@
  }
  
  ~@classname@() { } // FIXME: delete things?

  omniCallDescriptor* get_request_cd(){
    request = new @request_calldesc@
       (@request_lcallfn@,
        @request_args@);
    return request;
  }
  omniCallDescriptor* get_reply_cd(){
    @request_calldesc@ *derived_request = (@request_calldesc@*)request;

    reply = new @reply_calldesc@
       (@reply_lcallfn@,
        @reply_args@);
    return reply;
  }
  omniCallDescriptor* get_exc_cd(Messaging::ExceptionHolder &holder){
    exc = new @exc_calldesc@
       (@exc_lcallfn@,
        @exc_args@);
    return exc;
  }
  inline omniObjRef* get_target(){ return @target_t@::_duplicate(target); }
  inline omniObjRef* get_handler(){ return @handler_t@::_duplicate(handler); }

  inline Messaging::ExceptionHolder* get_exception_holder(){
    return new @exceptionholder@();
  }
};
"""


class _AMI_Call_Descriptor(iface.Class):
    def __init__(self, target, handler, request, reply, exception):
        assert isinstance(target, iface.Interface)
        assert isinstance(handler, iface.Interface)
        assert isinstance(request, call.Callable)
        assert isinstance(reply, call.Callable)
        assert isinstance(exception, call.Callable)
        self.target = target
        self.handler = handler
        self.request = request
        self.reply = reply
        self.exception = exception


    def cc(self, stream):
        stream.out("// Call Descriptor goes here somewhere")

        handler_t = self.handler.name().fullyQualify()
        target_t = self.target.name().fullyQualify()

        # need to have storage for all arguments
        arg_storage = output.StringStream()
        for parameter in self.request.parameters():
            pType = types.Type(parameter.paramType())
            ident = id.mapID(parameter.identifier())
            arg_storage.out(pType._var() + " " + ident + ";")
        rType = types.Type(self.request.returnType())
                            
        constructor_args = [] # constructor argument list
        request_args = []     # args sent on request

        constructor_args.append(target_t + "_ptr _target")
        constructor_args.append(handler_t + "_ptr _handler")

        constructor = output.StringStream() # copies of arguments
        constructor.out("target = " + self.target.name().fullyQualify() +\
                        "::_duplicate(_target);")
        constructor.out("handler = " + self.handler.name().fullyQualify() +\
                        "::_duplicate(_handler);")        
        
        request_args.append("\"" + self.request.operation_name() + "\"")
        request_args.append(str(len(self.request.operation_name()) + 1))
        request_args.append("0")
        
        for parameter in self.request.parameters():
            pType = types.Type(parameter.paramType())
            ident = id.mapID(parameter.identifier())
            constructor_args.append(pType._ptr() + " _" + ident)
            constructor.out(pType.copy("_" + ident, ident))
            request_args.append(ident)
            
        reply_args = []       # args sent to reply handler
        reply_args.append("\"" + self.reply.operation_name() + "\"")
        reply_args.append(str(len(self.reply.operation_name()) + 1))
        reply_args.append("0")
        if not(rType.void()): # grab the return value
            reply_args.append("derived_request->result()")
        for parameter in self.reply.parameters():
            if parameter.identifier() != "ami_return_val": # hack!
                reply_args.append(id.mapID(parameter.identifier()))

        exc_args = []
        exc_args.append("\"" + self.exception.operation_name() + "\"")
        exc_args.append(str(len(self.exception.operation_name()) + 1))
        exc_args.append("0")
        # need to narrow to the type specific exception holder
        tseh = ami.unique_name(self.request.interface().name(),
                               "ExceptionHolder",
                               self.request.interface().environment())
        exc_args.append("*((" + tseh.fullyQualify() + "*)&holder)")

        request_cd = call.proxy_call_descriptor(self.request, stream)
        reply_cd = call.proxy_call_descriptor(self.reply, stream)
        exc_cd = call.proxy_call_descriptor(self.exception, stream)
        request_lcall = call.local_callback_function\
                        (stream, self.target._node, name = self.target.name(),
                         callable = self.request)
        reply_lcall = call.local_callback_function\
                      (stream, self.handler._node, name = self.handler.name(),
                       callable = self.reply)
        exc_lcall = call.local_callback_function\
                    (stream, self.handler._node,
                     name = self.handler.name(), callable = self.exception)
        
                                                     
        stream.out(call_descriptor_t,
                   classname = "temp",
                   handler_t = handler_t, target_t = target_t,
                   arg_storage = arg_storage,
                   constructor_args = string.join(constructor_args, ", "),
                   constructor = constructor,
                   request_args = string.join(request_args, ", "),
                   reply_args = string.join(reply_args, ", "),
                   exc_args = string.join(exc_args, ", "),
                   request_calldesc = request_cd,
                   request_lcallfn = request_lcall,
                   reply_calldesc = reply_cd, reply_lcallfn = reply_lcall,
                   exc_calldesc = exc_cd, exc_lcallfn = exc_lcall,
                   exceptionholder = tseh.fullyQualify())
                   
                   
                            

