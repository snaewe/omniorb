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
#
# $Id$
# $Log$
# Revision 1.1.2.8  2000/10/22 00:33:51  djs
# Fixed bug deleting non-existent exception holder valuetype
# Accidentally set the AMI queue length to 0, rather than -1 (zero length
# vs unbounded)
#
# Revision 1.1.2.7  2000/10/10 15:07:34  djs
# Bug in storing the invocation target
#
# Revision 1.1.2.6  2000/09/28 18:29:21  djs
# Bugfixes in Poller (wrt timout behaviour and is_ready function)
# Removed traces of Private POA/ internal ReplyHandler servant for Poller
# strategy
# Fixed nameclash problem in Call Descriptor, Poller etc
# Uses reference counting internally rather than calling delete()
# General comment tidying
#
#
import string

from omniidl_be.cxx import id, types, output, cxx, ast, call, iface
from omniidl_be.cxx.ami import ami

# Operation-specific AMI call descriptor  ############################
#
# Descriptor is common to both callback and polling AMI requests. It
# keeps track of who owns both the arguments sent to the remote server
# and who owns the received replies, to minimised the number of data
# copies.
# If it is instantiated with _own_sent set to 1, it will duplicate the
# calling arguments and release its own copies on destruction. If _own_sent
# is set to 0 it will sneakily store pointers to the calling arguments,
# trusting they won't be deleted until after it's finished. It won't
# attempt to delete them when it finishes.
# Instead of invoke()ing on an internal ReplyHandler servant to deliver
# replies when Polling (as suggested in the Messaging spec) it keeps
# a direct pointer, since the Poller is a valuetype and therefore local.
# If it hands results to a Poller, it gives up ownership and will not
# try to delete them itself.
#
# One problem is that if we take store pointers to the arguments then
# we have to cast away their const-ness. We guarantee dynamically not to
# free or otherwise modify the data, but the compiler cannot tell that
# statically (it's stored in the boolean _own_sent flag)
#
# We could solve this by splitting this class into two- more output but
# the cast could end up hiding a nasty bug.

call_descriptor_t = """\
class @classname@: public omniAMIDescriptor{
private:
  @target_t@_var  _pd_target;
  @handler_t@_var _pd_handler;
  @exceptionholder@ *_pd_holder;
  @tspoller_t@ *poller;
  
  // Argument storage
  @arg_storage@

protected:
  void init(@common_constructor_args@){
    @constructor_common@
    if (_own_sent){
      @constructor_duplicate_args@
    }else{
      @constructor_simple_assign_args@
    }
  }

public:
  @classname@(@handler_ctor_args@):
      omniAMIDescriptor(_own_sent, 1, 0), _pd_holder(NULL){
    init(@common_constructor_ids@);
    @duplicate_handler@
  }
  @classname@(@poller_ctor_args@):
      omniAMIDescriptor(_own_sent, 1, 1), _pd_holder(NULL), poller(poller){
    init(@common_constructor_ids@);
  }

  virtual ~@classname@(){
    delete_owned_storage();
  }

  virtual void delete_sent_arguments(){
    @delete_sent_arguments@
  }
  virtual void delete_replies(){
    if (_pd_holder) _pd_holder->_remove_ref();
    @delete_replies@
  }
  
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
  inline omniObjRef* get_target(){ return _pd_target; }
  inline omniObjRef* get_handler(){ return _pd_handler; }

  inline Messaging::ExceptionHolder* get_exception_holder(){
    _pd_holder = new @exceptionholder@();
    return _pd_holder;
  }
  void giveReplyToPoller(){
    @poller_get_result@

    @copy_to_poller@
    poller->_exholder = _pd_holder;
    own_replies = 0; // poller owns all the storage now
    poller->_NP_tell_poller_reply_received();
  }
};
"""


# AMI call descriptor generator ######################################
#
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

        # It would be nice to use friendly _var types and always copy
        # arguments, but for efficiency we'll have to perform the memory
        # management ourselves.
        # Declare a reference (_ptr) to each request parameter (or just
        # basic types for basic types etc)
        arg_storage = output.StringStream()
        for parameter in self.request.parameters():
            pType = types.Type(parameter.paramType())
            ident = ami.paramID(parameter)
            arg_storage.out(pType._ptr() + " " + ident + ";")

        rType = types.Type(self.request.returnType())

        # ... and the result value
        if not(rType.void()):
            arg_storage.out(rType._ptr() + " _result;")

        # Certain arguments are common to both of the public constructors.
        # CORBA::Boolean _own_sent : true if this object must copy the args
        # target_t_ptr   _target   : target interface
        # <op specific list>
        
        common_ctor_args = ["CORBA::Boolean _own_sent",
                            target_t + "_ptr _target"]
        common_ctor_ids  = ["_own_sent", "_target"]

        # The common code is factored out into an init() fn
        common_ctor = output.StringStream()
        common_ctor.out("this->_pd_target = " + \
                        self.target.name().fullyQualify() +\
                        "::_duplicate(_target);")

        # This is specific to the ReplyHandler constructor
        duplicate_handler = output.StringStream()
        duplicate_handler.out("this->_pd_handler = " + \
                              self.handler.name().fullyQualify() +\
                              "::_duplicate(_handler);")
        

        # request call descriptor has the wirename as a (ptr, len) string
        request_args = []  
        request_args.append("\"" + self.request.operation_name() + "\"")
        request_args.append(str(len(self.request.operation_name()) + 1))
        request_args.append("0")

        # Depending on the value of _own_sent, we either simply assign the
        # pointers or do a full duplicate.
        ctor_copy   = output.StringStream()   
        ctor_assign = output.StringStream()
        dtor_sent   = output.StringStream()
        dtor_reply  = output.StringStream()

        # if in polling mode, have to hand over arguments
        copy_to_poller = output.StringStream()

        for parameter in self.request.parameters():
            pType = types.Type(parameter.paramType())
            ident = ami.paramID(parameter)

            # Consider args going --server-> (in and inout)
            if parameter.is_in():
                op = ""
                if pType._ptr_is_pointer(): op = "*"
                common_ctor_args.append("const " + pType._ptr()+ " "+ ident)
                common_ctor_ids.append(ident)
                ctor_copy.out(pType.copy( op + ident, "this->" + ident))
                # DANGEROUS: we always use a non-const pointer but we treat
                # it as const (never modify, never release) unless it has
                # been allocated by us through a copy.
                # A more verbose but type-safe thing would be to make a union
                # of both versions so every assignment & release would be safe?
                ctor_assign.out("this->" + ident + " = " +
                                 "(" + pType._ptr() + ")" + ident + ";")
                # pointer conversion
                op = ami.pointer(pType._ptr_is_pointer(),
                        pType.op_is_pointer(types.direction(parameter)))
                request_args.append(op + ident)

                # free copied arguments
                dtor_sent.out(pType.free(ident))

            if parameter.direction() == 1: # -out-
                op = ""
                if not(pType.array()): # everything is a slice
                    op = ami.pointer(pType._ptr_is_pointer(),
                       pType.op_is_pointer(types.direction(parameter)))
                request_args.append(op  + ident)

                dtor_reply.out(pType.free(ident))

            # Consider args coming <--server-- (inout and out)
            if parameter.is_out():
                copy_to_poller.out("poller->" + ident + " = " + ident + ";")

        tspoller_t = ami.poller_descriptor(self.target._node, self.request)

        poller_ctor_args = [ tspoller_t + "*poller" ] + common_ctor_args 
        handler_ctor_args = [ handler_t + "_ptr _handler"] + common_ctor_args
                
        reply_args = []       # args sent to reply handler
        reply_args.append("\"" + self.reply.operation_name() + "\"")
        reply_args.append(str(len(self.reply.operation_name()) + 1))
        reply_args.append("0")

        request_cd = call.proxy_call_descriptor(self.request, stream)
        reply_cd = call.proxy_call_descriptor(self.reply, stream)
        exc_cd = call.proxy_call_descriptor(self.exception, stream)

        store_result = ""
        poller_get_result = ""
        if not(rType.void()): # grab the return value
            op = ami.pointer(rType._ptr_is_pointer(),
                             rType.op_is_pointer(types.IN))
            
            reply_args.append(op + "_result")
            store_result = "_result = derived_request->result();"

            dtor_reply.out(rType.free("_result"))

            poller_get_result = "_result = ((" + request_cd + "*)request)" +\
                                "->result();\n" +\
                                "poller->_ami_return_val = _result;"

            
        # reply only concerns -inout- and -out- params
        for parameter in self.reply.parameters():
            ident = ami.paramID(parameter)
            if parameter.identifier() != "ami_return_val": # hack!
                #reply_args.append("_" + ami.parameter(parameter))
                pType = types.Type(parameter.paramType())
                op = ami.pointer(pType._ptr_is_pointer(),
                        pType.op_is_pointer(types.direction(parameter)))
                reply_args.append(op + ident)


                
        exc_args = []
        exc_args.append("\"" + self.exception.operation_name() + "\"")
        exc_args.append(str(len(self.exception.operation_name()) + 1))
        exc_args.append("0")
        # need to narrow to the type specific exception holder
        tseh = id.Name(self.target._node.ExceptionHolder.scopedName())
        exc_args.append("*((" + tseh.fullyQualify() + "*)&holder)")

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
                   tspoller_t = tspoller_t,
                   arg_storage = arg_storage,
                   poller_ctor_args = string.join(poller_ctor_args,","),
                   handler_ctor_args = string.join(handler_ctor_args,","),
                   common_constructor_args = string.join(common_ctor_args,","),
                   constructor_common = common_ctor,
                   constructor_duplicate_args = ctor_copy,
                   constructor_simple_assign_args = ctor_assign,
                   common_constructor_ids = string.join(common_ctor_ids,","),
                   duplicate_handler = duplicate_handler,
                   result_store = store_result,
                   poller_get_result = poller_get_result,
                   delete_sent_arguments = dtor_sent,
                   delete_replies = dtor_reply,
                   request_args = string.join(request_args, ", "),
                   reply_args = string.join(reply_args, ", "),
                   exc_args = string.join(exc_args, ", "),
                   request_calldesc = request_cd,
                   request_lcallfn = request_lcall,
                   reply_calldesc = reply_cd, reply_lcallfn = reply_lcall,
                   exc_calldesc = exc_cd, exc_lcallfn = exc_lcall,
                   exceptionholder = tseh.fullyQualify(),
                   copy_to_poller = copy_to_poller)
                   
                   
