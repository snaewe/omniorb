#ifndef __omniAMI_h__
#define __omniAMI_h__

#include <omniORB3/callDescriptor.h>
#include <omniORB3/omniObjRef.h>
#include "queue.h"

#if 0
// Requires ability to time events
#if defined(__WIN32__)
# error "Need to port time functions to non-UNIX"
#else
# include <sys/time.h>
# include <unistd.h>
#endif
#endif

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
  virtual omniCallDescriptor *get_request_cd() = 0;
  virtual omniCallDescriptor *get_reply_cd() = 0;
  virtual omniCallDescriptor *get_exc_cd(Messaging::ExceptionHolder&) = 0;

  virtual omniObjRef *get_target() = 0;
  virtual omniObjRef *get_handler() = 0;

  virtual Messaging::ExceptionHolder* get_exception_holder() = 0;
};


class AMI{

public:
  typedef TimedQueue<omniAMICall*> Queue;

  class Worker: public omni_thread{
  private:
    //~Worker(){ }
    omniAMICall *first_task;
    
    //struct timeval last_service_time;

  public:
    ~Worker() { } // stupid warning
    Worker(omniAMICall *cd);
    void process(omniAMICall *cd);
    void shutdown();

    static const unsigned int timeout;
    //static const unsigned int thread_lifetime = 5;

    void *run_undetached(void *arg);
  };

  static omni_mutex state_lock;
  static int nWorkers;
  static int nBusy;
  
  static const unsigned int maxWorkers;
  
  static Queue queue;
  static unsigned long jobs_todo;
  
  static void enqueue(omniAMICall *cd);
};


#endif /* __omniAMI_h__ */
