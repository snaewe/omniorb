// Converts code generated from AMI_PseudoIDL.idl into the CORBA Messaging
// module

#ifndef __Messaging_hh__
#define __Messaging_hh__

#ifndef USE_omniORB_logStream
#define USE_omniORB_logStream
#endif

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

_CORBA_MODULE Messaging

_CORBA_MODULE_BEG

#ifndef __Messaging_mReplyHandler__
# include "omniMessaging_defs.hh"
#endif

// ExceptionHolder valuetype
struct ExceptionHolder: ExceptionHolder_base{
  ExceptionHolder(): local_exception_object(NULL){ } 
  virtual ~ExceptionHolder(){ }

  // Exception can either be a pointer to a local exception object
  // kept here, or it can exit in marshalled form inside the 
  // sequeence<octet>
  CORBA::Exception *local_exception_object;

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

typedef ExceptionHolder_base_var ExceptionHolder_var;

typedef ExceptionHolder_base_out ExceptionHolder_out;

_CORBA_MODULE_VAR _dyn_attr const CORBA::TypeCode_ptr _tc_ExceptionHolder;

// Poller valuetype
struct Poller: public POA_CORBA::Pollable{
protected:
  Poller(): pd_state_cond(&pd_state_lock),
	    reply_received(0),
	    reply_already_read(0),
            pd_is_from_poller(0){ }

  void _NP_wrong_transaction(){
    pd_is_from_poller = 1;
    throw CORBA::WRONG_TRANSACTION();
  }
public:
  virtual ~Poller() { }

  // readonly attribute Object operation_target;
  //  6.6.1: The target of the asynchronous invocation is accessible from
  //         any Poller.
  virtual CORBA::Object_ptr operation_target() = 0;
  // readonly attribute string opeartion_name;
  //  6.6.2: The name of the operation that was invoked asynchronously is
  //         accessible from any Poller. The returned string is identical
  //         to the operation name from the target interface's InterfaceDef.
  virtual const char* operation_name() = 0;
  // attribute ReplyHandler associated_handler;
  //  6.6.3: If the associated_handler is set to nil, the pollinb model is
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
  CORBA::Object_Member target;
  // string op_name
  CORBA::String_member op_name;

public:
  // store the state of the request- has a reply been received?
  omni_mutex      pd_state_lock;
  omni_condition  pd_state_cond;
  CORBA::Boolean  reply_received;
  CORBA::Boolean  reply_already_read;
  CORBA::Boolean  pd_is_from_poller;

  // Are we using the ReplyHandler callback mode
  ReplyHandler_ptr pd_associated_handler;
  

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

