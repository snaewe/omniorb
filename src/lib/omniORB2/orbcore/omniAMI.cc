// System support for the AMI interface

#include <omniORB3/CORBA.h>
#include <omniORB3/Messaging.h>
#include <omniORB3/omniAMI.h>

#undef DEBUG

#ifdef DEBUG
# define AMITRACE(prefix, message) \
    cerr << prefix << ": " << message << endl;
#else
# define AMITRACE(prefix, message)
#endif


AMI::Worker::Worker(omniAMICall *cd): first_task(cd){
  // process this call then wait for others
  AMITRACE("AMI::Worker", "constructed");
  
  start_undetached();
}

void AMI::Worker::process(omniAMICall *cd){
  AMITRACE("AMI::Worker", "processing request");

  Messaging::ExceptionHolder *holder;
  CORBA::Boolean exception_occurred = 0;

  // execute request
  {
    omniObjRef *target = cd->get_target();
    omniCallDescriptor *calldesc = cd->get_request_cd();
    
    try{
      // synchronous invocation
      target->_invoke(*calldesc, 1);
    }catch(CORBA::UserException &e){
      exception_occurred = 1;
      
      holder = cd->get_exception_holder();
      holder->is_system_exception = 0;
      holder->local_exception_object = CORBA::Exception::_duplicate(&e);
      //holder->_NP_marshal_exception_to_sequence(e);
      
    }catch(CORBA::SystemException &e){
      exception_occurred = 1;
      
      holder = cd->get_exception_holder();
      holder->is_system_exception = 1;
      holder->local_exception_object = CORBA::Exception::_duplicate(&e);
      //holder->_NP_marshal_exception_to_sequence(e);
      
    }
  }
  if (exception_occurred){
    
    AMITRACE("AMI::Worker", "sending exception");
    {
      omniObjRef *handler = cd->get_handler();
      omniCallDescriptor *calldesc = cd->get_exc_cd(*holder);
      handler->_invoke(*calldesc, 1);
    }
    delete holder;
  }else{
    AMITRACE("AMI::Worker", "sending results");
    // results are now available
    {
      omniObjRef *handler = cd->get_handler();
      omniCallDescriptor *calldesc = cd->get_reply_cd();
      
      // synchronous again
      handler->_invoke(*calldesc, 1);
    }
  }
  // finished processing, delete storage
  delete cd;
  AMITRACE("AMI::Worker", "Request completed");
}

void AMI::Worker::shutdown(){
  AMITRACE("AMI::Worker", "shutting down");
}


void *AMI::Worker::run_undetached(void *arg){
  AMITRACE("AMI::Worker", "thread start");

  omniAMICall *call;

  // avoid the queue for the first one
  if (first_task){
    process(first_task);
    first_task = NULL;
    //gettimeofday(&last_service_time, NULL);
  }

  while (1){
    AMITRACE("AMI::Worker", "dequeue");
    call = queue.dequeue(timeout);
    if (!call){
      AMITRACE("AMI::Worker", "got timeout");
      // we call this code after <timeout> seconds and check
      // how long this thread has been idle.
      //struct timeval current_time;
      //gettimeofday(&current_time, NULL);
      //unsigned long idle_time = current_time.tv_sec - 
      //	last_service_time.tv_sec;
      //if (idle_time > thread_lifetime){
	// only shutdown if no work is pending.
	bool must_shutdown = 0;
	{
	  omni_mutex_lock lock(AMI::state_lock);
	  if (jobs_todo == 0){
	    AMI::nWorkers --;
	    must_shutdown = 1;
	  }
	}
	if (must_shutdown){
	  shutdown();
	  return NULL;
	}
	//}
    }else{
      process(call);
      
      AMITRACE("AMI::Worker", "looping-|");
      // timestamp last use of this thread
      //gettimeofday(&last_service_time, NULL);
    }
  }
  return NULL;
}



void AMI::enqueue(omniAMICall *cd){
  {
    omni_mutex_lock lock(state_lock);
    jobs_todo ++;
    
    if ((nWorkers < maxWorkers)){
      // spawn a new worker thread
      AMITRACE("AMI", "Spawning a new worker");
      Worker *newWorker = new Worker(cd);
      
      nWorkers++;
      nBusy++;
      return;
    }
  }
  // queue request (potentially a blocking call if the queue
  // gets full. Mustn't block holding state_lock)
  queue.enqueue(cd);
  
}


omni_mutex AMI::state_lock;
int AMI::nWorkers = 0;
int AMI::nBusy = 0;
AMI::Queue AMI::queue(128);
unsigned long AMI::jobs_todo = 0;
const unsigned int AMI::maxWorkers = 5;
const unsigned int AMI::Worker::timeout = 3;
