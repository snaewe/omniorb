#ifndef __omniAMI_h__
#define __omniAMI_h__

#include <omniORB3/callDescriptor.h>
#include <omniORB3/omniObjRef.h>
#include "queue.h"

class omniAMICall;
class omni_AMI_initialiser;

class AMI{

 protected:
  static void initialise_private_POA();


 public:
  typedef Queue<omniAMICall*> Queue;
  
  class Worker: public omni_thread{
  private:
    //~Worker(){ }
    omniAMICall *first_task;
    //void reply_with_exception(omniAMICall&, CORBA::Exception&, CORBA::Boolean);
    

  public:
    ~Worker() { } // stupid warning
    Worker(omniAMICall *cd);
    void process(omniAMICall& call);
    void shutdown();

    void *run_undetached(void *arg);
  };

  static void enqueue(omniAMICall *cd);

  static void shutdown();

  static CORBA::ORB_ptr ORB;
  static PortableServer::POA_ptr POA;

  friend class omni_AMI_initialiser;
};


#endif /* __omniAMI_h__ */


