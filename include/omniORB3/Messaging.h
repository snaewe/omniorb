// -*- Mode: C++; -*-
//                            Package   : omniORB
// Messaging.h                Created on: 21/8/2000
//                            Author    : David Scott (djs)
//
//    Copyright (C) 1996-2000 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//    02111-1307, USA
//
//
// Description: Valuetypes in the Messaging module
//    
//
/*
 $Id$
 $Log$
 Revision 1.1.2.5  2000/09/28 23:18:14  djs
 Reordered initialiser list to match declaration order

 Revision 1.1.2.4  2000/09/28 18:28:36  djs
 Bugfixes in Poller (wrt timout behaviour and is_ready function)
 Removed traces of Private POA/ internal ReplyHandler servant for Poller
 strategy
 General comment tidying

*/

// Converts code generated from AMI_PseudoIDL.idl into the CORBA Messaging
// module

#ifndef __Messaging_hh__
#define __Messaging_hh__

#ifndef USE_omniORB_logStream
#define USE_omniORB_logStream
#endif

// for ULONG_MAX
#include <limits.h>

#ifndef __CORBA_H_EXTERNAL_GUARD__
#include <omniORB3/CORBA.h>
#endif

#ifdef USE_stub_in_nt_dll
#ifndef USE_core_stub_in_nt_dll
#define USE_core_stub_in_nt_dll
#endif
#ifndef USE_dyn_stub_in_nt_dll
#define USE_dyn_stub_in_nt_dll
#endif
#endif

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#else
# ifdef  USE_core_stub_in_nt_dll
#  define _core_attr _OMNIORB_NTDLL_IMPORT
# else
#  define _core_attr
# endif
#endif

#ifdef _dyn_attr
# error "A local CPP macro _dyn_attr has already been defined."
#else
# ifdef  USE_dyn_stub_in_nt_dll
#  define _dyn_attr _OMNIORB_NTDLL_IMPORT
# else
#  define _dyn_attr
# endif
#endif

#include <omniORB3/omniAMI.h>

_CORBA_MODULE Messaging

_CORBA_MODULE_BEG

// Import the closest IDL we can (structs rather than valuetypes)
#ifndef __Messaging_mReplyHandler__
# include "omniMessaging_defs.hh"
#endif

// Note that valuetypes decend from CORBA::ValueBase and are reference
// counted objects (using _add_ref() and _remove_ref) They should never
// be delete()d directly, the system will delete them when their reference
// counts drop to zero.

/////////////////////////////////////////////////////////////////////////
// Messaging::ExceptionHolder valuetype /////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Note this can be marshalled and sent across a network, but it will
// look like a simple struct and not a valuetype. Not much can be done
// about this until omniORB supports valuetypes.

// When instances of this object are created (on catching an exception)
// the exception is stored as a simple local pointer and not marshalled 
// into a sequence<octet> buffer unless it is necessary to actually transmit 
// it.)
struct ExceptionHolder: ExceptionHolder_base, CORBA::ValueBase{
  ExceptionHolder(): local_exception_object(NULL){ } 
  virtual ~ExceptionHolder();

  // Exception can either be a pointer to a local exception object
  // kept here, or it can exit in marshalled form inside the 
  // sequeence<octet>
  CORBA::Exception *local_exception_object;

  // Fields in a valuetype look like fields in a union, using accessor
  // functions.
  CORBA::Boolean is_system_exception(){ return pd_is_system_exception; }
  CORBA::Boolean byte_order(){ return pd_byte_order; }
  ExceptionHolder_base::_pd_marshaled_exception_seq marshaled_exception(){ 
    return pd_marshaled_exception;
  }

  // Functions to handle insertion and extraction of exception from
  // the sequence<octet>
  void _NP_marshal_exception_to_sequence(const CORBA::Exception& e);
  virtual void _NP_unmarshal_sequence_to_exception() { }

  // Custom marshalling and unmarshalling depending on the actual
  // location of the exception
  void operator>>= (NetBufferedStream &);
  void operator<<= (NetBufferedStream &);
  void operator>>= (MemBufferedStream &);
  void operator<<= (MemBufferedStream &);
};

typedef _CORBA_ConstrType_Variable_Var<ExceptionHolder> ExceptionHolder_var;
typedef _CORBA_ConstrType_Variable_OUT_arg<ExceptionHolder, ExceptionHolder_var> ExceptionHolder_out;

_CORBA_MODULE_VAR _dyn_attr const CORBA::TypeCode_ptr _tc_ExceptionHolder;


/////////////////////////////////////////////////////////////////////////
// Messaging::Poller valuetype //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Again this valuetype is implemented as a struct with methods. It cannot
// really be transmitted meaningfully on the network- if the request hasn't
// been received then a strict value-copy will never be useful. Methods to
// marshal type specific pollers aren't generated.

// Note that operation invocation results can be taken from this struct
// (as .out() arguments) and subsequent (or concurrent) calls should throw
// an exception. We need concurrency control to ensure this.

struct Poller: public CORBA::ValueBase{

protected:
  Poller(CORBA::Object_ptr target, const char *opname): 
    pd_state_cond(&pd_state_lock),
    reply_received(0),
    reply_already_read(0),
    pd_is_from_poller(0),
    pd_associated_handler(Messaging::ReplyHandler::_nil()),
    pd_target(CORBA::Object::_duplicate(target)),
    pd_op_name(opname) { }


  // lock internal data, allow for blocking waits
  omni_mutex      pd_state_lock;
  omni_condition  pd_state_cond;

  CORBA::Boolean  reply_received;     // true if reply arrived from server
  CORBA::Boolean  reply_already_read; // true if someone already looked at it
  CORBA::Boolean  pd_is_from_poller;  // true if exception came from the AMI
                                      // system and has nothing to do with
                                      // the remote server

  // If we have set an external ReplyHandler callback, then forward the
  // result to this object
  ReplyHandler_ptr pd_associated_handler;

  public:
  // Type-specific Pollers are defined _per IDL interface_ however they
  // are used _per OPERATION_. Therefore it is possible to ask for the 
  // wrong reply from a poller. Convenience function to throw the exception.
  void _NP_throw_wrong_transaction(){
    pd_is_from_poller = 1; // informs the client code that the exception
                           // originated from the AMI system and not from
                           // the remote server
    throw CORBA::WRONG_TRANSACTION();
  }

  // Called by the stubs to perform a blocking wait for each polling call.
  void _NP_wait_and_throw_exception(CORBA::ULong timeout){
    if (!is_ready(timeout)) {
      // NO_RESPONSE is generated for the no-timeout case
      if (timeout == 0) throw CORBA::NO_RESPONSE();
      throw CORBA::TIMEOUT();
    }

    // Data must have arrived within the timeout period.
    {
      omni_mutex_lock lock(pd_state_lock);
      // Has someone else got it?
      if (reply_already_read) throw CORBA::OBJECT_NOT_EXIST();
      // Claim it for ourselves
      reply_already_read = 1;
      // Everyone else who tries to get this data will get an exception instead
    }
  }

  // When the call descriptor is told to send on the reply and we are polling,
  // it fills in the pointers and then calls this function to wake up blocked
  // threads.
  void _NP_tell_poller_reply_received(){
    omni_mutex_lock lock(pd_state_lock);
    reply_received = 1;
    // The client could have lots of threads blocked on this event, better wake
    // them all up. They can fight over ownership of the data and the loser(s)
    // can return with OBJECT_NOT_EXIST
    pd_state_cond.broadcast();
  }

  public:
  virtual ~Poller() {
    CORBA::release(pd_associated_handler);
    CORBA::release(pd_target);
  }

  // readonly attribute Object operation_target;
  //  6.6.1: The target of the asynchronous invocation is accessible from
  //         any Poller.
  CORBA::Object_ptr operation_target(){ 
    return CORBA::Object::_duplicate(pd_target);
  }

  // readonly attribute string opeartion_name;
  //  6.6.2: The name of the operation that was invoked asynchronously is
  //         accessible from any Poller. The returned string is identical
  //         to the operation name from the target interface's InterfaceDef.
  const char* operation_name() { return pd_op_name; }

  // attribute ReplyHandler associated_handler;
  //  6.6.3: If the associated_handler is set to nil, the polling model is
  //         used to obtain the reply to the request. If it is non-nil, the
  //         associated ReplyHandler is invoked when a reply becomes available.
  ReplyHandler_ptr associated_handler();
  void associated_handler(ReplyHandler_ptr);

  // readonly attribute boolean is_from_poller
  //  6.6.4: The attribute returns the value TRUE if and only if the poller
  //         operation itself raised a system exception. If the Poller has
  //         not yet returned a response to the client, the BAD_INV_ORDER
  //         system exception is raised.
  CORBA::Boolean is_from_poller();
  
  CORBA::Boolean is_ready(CORBA::ULong timeout);
  CORBA::PollableSet_ptr create_pollable_set();

  // *** Question: what is the point in having a
  //    readonly attribute Object Messaging::Poller::operation_target
  //    readonly attribute string Messaging::Poller::operation_name
  // as well as the (writable?)
  //    Object Messaging::Poller::target
  //    string Messaging::Poller::string
  // what would be the point in writing to either of these fields?!

  CORBA::Object_ptr pd_target;
  const char* pd_op_name;
};

_CORBA_MODULE_END

_CORBA_MODULE POA_Messaging
_CORBA_MODULE_BEG

#include "omniMessaging_poa.hh"

_CORBA_MODULE_END

#undef _core_attr
#undef _dyn_attr

#include "omniMessaging_operators.hh"

#endif  // __Messaging_hh__

