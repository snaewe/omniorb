#ifndef __omniAMI_h__
#define __omniAMI_h__

#include <omniORB3/callDescriptor.h>
#include <omniORB3/omniObjRef.h>
#include "queue.h"

// Analogue of an omniCallDescriptor: 
//   * may provide storage for operation arguments between the request op
//     and the reply op (or just external pointers to them)
//   * stores the invocation target and reply handler objrefs
//   * can produce omniCallDescriptors for the request/ reply/ exception
class omniAMICall{
protected:
  omniCallDescriptor *request;
  omniCallDescriptor *reply;
  omniCallDescriptor *exc;
public:
  omniAMICall(): request(NULL), reply(NULL), exc(NULL) { }
  virtual ~omniAMICall() {
    delete request;
    if (reply) delete reply;
    if (exc)   delete exc;
  }
  // These are implemented by the IDL compiler, dealing with type-specific
  // entities.
  virtual omniCallDescriptor *get_request_cd() = 0;
  virtual omniCallDescriptor *get_reply_cd() = 0;
  virtual omniCallDescriptor *get_exc_cd(Messaging::ExceptionHolder&) = 0;

  virtual omniObjRef *get_target() = 0;
  virtual omniObjRef *get_handler() = 0;

  virtual Messaging::ExceptionHolder* get_exception_holder() = 0;
};

//?
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


class AMI{

public:
  typedef Queue<omniAMICall*> Queue;

  class Worker: public omni_thread{
  private:
    //~Worker(){ }
    omniAMICall *first_task;
    void reply_with_exception(omniAMICall&, CORBA::Exception&, CORBA::Boolean);
    

  public:
    ~Worker() { } // stupid warning
    Worker(omniAMICall *cd);
    void process(omniAMICall *cd);
    void shutdown();

    void *run_undetached(void *arg);
  };

  static void enqueue(omniAMICall *cd);

  static void shutdown();
};


#endif /* __omniAMI_h__ */


