#ifndef _OMNI_AMI_CALL_DESCRIPTOR_H
#define _OMNI_AMI_CALL_DESCRIPTOR_H

#include <omniORB3/callDescriptor.h>
#include <omniORB3/omniObjRef.h>
#include "queue.h"

// interface used by the AMI subsystem to control individual AMI requests.
// (objects of this type are stored in a queue by the system)
class omniAMICall{
 public:
  virtual ~omniAMICall() { }
  // invoke operation on target interface
  virtual void sendRequest()   = 0;

  // process the non-exceptional reply from the server
  virtual void sendReply()     = 0;

  // process the exceptional reply from the server
  virtual void sendException(CORBA::Exception &e, 
			     CORBA::Boolean is_system_exception) = 0; 
};

// Each AMI Call Descriptor object can either make private copies of operation
// arguments, in which case it needs to delete them when it finishes or it
// can use references to data owned by someone else. Similarly when replies
// come back from the target interface they are owned by this descriptor, but 
// ownership could be passed on to another entity (specifically a Poller)
class omniAMICallStorage{
 protected:
  CORBA::Boolean own_sent_arguments; // true if I own the operation arguments
  CORBA::Boolean own_replies;        // true if I own the replies
 public:
  omniAMICallStorage(CORBA::Boolean own_sent, CORBA::Boolean own_replies):
    own_sent_arguments(own_sent), own_replies(own_replies){ }
  virtual ~omniAMICallStorage() { }

  // called by the derived destructor to delete any storage still owned by
  // this descriptor after the call has completed.
  void delete_owned_storage(){
    if (own_sent_arguments) delete_sent_arguments();
    if (own_replies) delete_replies();
  }

  // overriden on an operation by operation basis to properly deallocate
  // heap storage.
  virtual void delete_sent_arguments() = 0;
  virtual void delete_replies() = 0;
};

// common code for a normal operation invocation, used irrespective of
// how the results are handled
class omniAMINormalCall: public omniAMICall{
 protected:
  omniCallDescriptor *request;
 public:
  omniAMINormalCall(): omniAMICall(), request(NULL) { }
  virtual ~omniAMINormalCall(){ delete request; }
  
  virtual omniCallDescriptor *get_request_cd() = 0;
  virtual omniObjRef         *get_target() = 0;

  void sendRequest(){
    get_target()->_invoke(*get_request_cd());
  }
};

// Actual descriptor class which gets overriden by the operation-specific 
// class in the generated stubs.
class omniAMIDescriptor: public omniAMINormalCall, public omniAMICallStorage{
 protected:
  omniCallDescriptor *reply;
  omniCallDescriptor *exc;
  CORBA::Boolean _is_polling; // true if handing results to a local Poller
                              // object rather than sending them on via
                              // a ReplyHandler call
 public:
  omniAMIDescriptor(CORBA::Boolean own_sent, 
		    CORBA::Boolean own_replies,
		    CORBA::Boolean is_polling):
    omniAMINormalCall(), omniAMICallStorage(own_sent, own_replies),
    reply(NULL), exc(NULL), _is_polling(is_polling) { }
  virtual ~omniAMIDescriptor(){
    if (reply) delete reply;
    if (exc)   delete exc;
  }

  virtual omniCallDescriptor *get_reply_cd()                          = 0;
  virtual omniCallDescriptor *get_exc_cd(Messaging::ExceptionHolder&) = 0;
  virtual Messaging::ExceptionHolder *get_exception_holder()          = 0;
  virtual omniObjRef *get_handler()                                   = 0;
  
  void sendReply();
  void sendException(CORBA::Exception &e, CORBA::Boolean is_system_exception);
  virtual void giveReplyToPoller() = 0;
};

// Convenience _var type for the call descriptor. Could probably use one
// of the standard templates?
class omniAMICall_var{
  omniAMICall *data;
 public:
  omniAMICall_var(): data(NULL) { }
  omniAMICall_var(omniAMICall *d): data(d) { }
  ~omniAMICall_var() { 
    delete data; 
  }
  omniAMICall_var operator=(omniAMICall* call){
    if (data) delete data;
    data = call;
    return *this;
  }
};
#endif /*_OMNI_AMI_CALL_DESCRIPTOR_H */
