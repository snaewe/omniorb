// Converts code generated from AMI_PseudoIDL.idl into the CORBA Messaging
// module

#ifndef __Messaging_hh__
#define __Messaging_hh__

#ifndef USE_omniORB_logStream
#define USE_omniORB_logStream
#endif

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

#ifndef __Messaging_mReplyHandler__
# include "omniMessaging_defs.hh"
#endif

// ExceptionHolder valuetype
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


// Poller "valuetype"
struct Poller: public CORBA::ValueBase{

protected:
  Poller(CORBA::Object_ptr target, const char *opname): 
    pd_state_cond(&pd_state_lock),
    reply_received(0),
    reply_already_read(0),
    pd_is_from_poller(0),
    pd_associated_handler(Messaging::ReplyHandler::_nil()),
    op_name(opname) { }


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
  void _NP_wrong_transaction(){
    pd_is_from_poller = 1;
    throw CORBA::WRONG_TRANSACTION();
  }
  void _NP_do_timeout(CORBA::ULong timeout){
    if (!is_ready(timeout)) throw CORBA::TIMEOUT();
  }
  void _NP_reply_received(){
    omni_mutex_lock lock(pd_state_lock);
    reply_received = 1;
    // ** Question- if two threads are blocked, should both be woken up and
    // one throw an exception?
    pd_state_cond.signal();
  }
  // Called when the reply is known to have been received (ie is_ready returned
  // TRUE) to prevent two threads simultaneously taking the results.
  void _NP_reply_obtained(){
    omni_mutex_lock lock(pd_state_lock);
    
    if (reply_already_read)
      throw CORBA::OBJECT_NOT_EXIST();
    
    reply_already_read = 1;
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
  const char* operation_name() { return op_name; }

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

  // Object target
  CORBA::Object_Member pd_target;
  // string op_name
  const char* op_name;

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

