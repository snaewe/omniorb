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
  @target_t@_var  _PD_target;
  @handler_t@_var _PD_handler;
  @exceptionholder@_var _PD_holder;
  
  // Argument storage
  @arg_storage@

public:
  @classname@(@constructor_args@):_PD_holder(NULL){
    @constructor@
  }
  
  ~@classname@() { }

  omniCallDescriptor* get_request_cd(){
    request = new @request_calldesc@
       (@request_lcallfn@,
        @request_args@);
    return request;
  }
  omniCallDescriptor* get_reply_cd(){
    @request_calldesc@ *derived_request =
       (@request_calldesc@*)request;

    @result_store@

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
  inline omniObjRef* get_target(){ return _PD_target; }
  inline omniObjRef* get_handler(){ return _PD_handler; }

  inline Messaging::ExceptionHolder* get_exception_holder(){
    _PD_holder = new @exceptionholder@();
    return _PD_holder.operator->();
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

        self.descriptor = ami.call_descriptor(target._node,
                                              request.operation_name(),
                                              request.signature())


    def cc(self, stream):
        handler_t = self.handler.name().fullyQualify()
        target_t = self.target.name().fullyQualify()

        # need to have storage for all arguments
        arg_storage = output.StringStream()
        for parameter in self.request.parameters():
            pType = types.Type(parameter.paramType())
            ident = id.mapID(parameter.identifier())
            arg_storage.out(pType._var() + " _" + ident + ";")
        rType = types.Type(self.request.returnType())

        # ... and the result value
        if not(rType.void()):
            arg_storage.out(rType._var() + " _result;")
                            
        constructor_args = [] # constructor argument list
        request_args = []     # args sent on request

        # Note: since we copy everything, we can make everything const
        constructor_args.append("const " + target_t + "_ptr _PD_target")
        constructor_args.append("const " + handler_t + "_ptr _PD_handler")

        constructor = output.StringStream() # copies of arguments
        constructor.out("this->_PD_target = " + \
                        self.target.name().fullyQualify() +\
                        "::_duplicate(_PD_target);")
        constructor.out("this->_PD_handler = " + \
                        self.handler.name().fullyQualify() +\
                        "::_duplicate(_PD_handler);")        
        
        request_args.append("\"" + self.request.operation_name() + "\"")
        request_args.append(str(len(self.request.operation_name()) + 1))
        request_args.append("0")

        # constructor call and request op only concern -in- and -inout- params
        for parameter in self.request.parameters():
            pType = types.Type(parameter.paramType())
            ident = "_" + id.mapID(parameter.identifier())
            if parameter.is_in():
                op = ""
                if pType._ptr_is_pointer(): op = "*"
                constructor_args.append("const " + pType._ptr() + " " + ident)
                constructor.out(pType.copy(op + ident, "this->" + ident))
                request_args.append("_" + ami.parameter(parameter))

            if parameter.direction() == 1: # -out-
                request_args.append("_" + ami.parameter(parameter))
            
        reply_args = []       # args sent to reply handler
        reply_args.append("\"" + self.reply.operation_name() + "\"")
        reply_args.append(str(len(self.reply.operation_name()) + 1))
        reply_args.append("0")

        store_result = ""
        if not(rType.void()): # grab the return value
            reply_args.append("_result")
            store_result = "_result = derived_request->result();"
            
        # reply only concerns -inout- and -out- params
        for parameter in self.reply.parameters():
            if parameter.identifier() != "ami_return_val": # hack!
                reply_args.append("_" + ami.parameter(parameter))
                #reply_args.append(id.mapID(parameter.identifier()))
                
        exc_args = []
        exc_args.append("\"" + self.exception.operation_name() + "\"")
        exc_args.append(str(len(self.exception.operation_name()) + 1))
        exc_args.append("0")
        # need to narrow to the type specific exception holder
        tseh = id.Name(self.target._node.ExceptionHolder.scopedName())
        exc_args.append("*((" + tseh.fullyQualify() + "*)&holder)")

        request_cd = call.proxy_call_descriptor(self.request, stream)
        reply_cd = call.proxy_call_descriptor(self.reply, stream)
        exc_cd = call.proxy_call_descriptor(self.exception, stream)
        request_lcall = call.local_callback_function\
                        (stream, self.target.name(),
                         callable = self.request)
        reply_lcall = call.local_callback_function\
                      (stream, self.handler.name(),
                       callable = self.reply)
        exc_lcall = call.local_callback_function\
                    (stream, self.handler.name(), callable = self.exception)
        
                                                     
        stream.out(call_descriptor_t,
                   classname = self.descriptor,
                   handler_t = handler_t, target_t = target_t,
                   arg_storage = arg_storage,
                   result_store = store_result,
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
                   
                   
                            

