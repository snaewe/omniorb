// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniAMI.cc                 Created on: 21/08/2000
//                            Author    : David Scott (djs)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
// Description:
//      Runtime AMI support
//

/* 
  $Log$
  Revision 1.1.2.7  2000/10/10 15:07:13  djs
  Improved comments & tracing
  Moved code from header file to here

  Revision 1.1.2.6  2000/09/28 18:29:40  djs
  Bugfixes in Poller (wrt timout behaviour and is_ready function)
  Removed traces of Private POA/ internal ReplyHandler servant for Poller
  strategy
  General comment tidying

  Revision 1.1.2.5  2000/09/27 17:13:08  djs
  Struct member renaming
  Added command line options
  Added CORBA::ValueBase (just to do reference counting)
  General refactoring

  Revision 1.1.2.4  2000/09/21 16:36:24  djs
  Set max queue length to 65536 (arbitrary)

  Revision 1.1.2.3  2000/09/20 13:25:29  djs
  Added Messaging::ExceptionHolder destructor
  Added omniAMICall_var type

  Revision 1.1.2.2  2000/09/14 16:04:16  djs
  Added module initialiser for AMI module

  $Id$
*/

#include <omniORB3/CORBA.h>
#include <omniORB3/Messaging.h>
#include <omniORB3/omniAMICallDescriptor.h>
#include <omniORB3/omniAMI.h>

#include "initialiser.h"


#define AMITRACE(prefix, message) omniORB::logs(10, prefix ": " message)


// As async calls come in, they are added to a queue served by a dynamic
// pool of threads. Initially the pool is empty, but it grows as requests
// are added up to a maximum size (maxWorkers). The threads block for a
// set (timeout) number of seconds, and if no work is available they shut
// themselves down.

// Uses the system variables:
//   omniORB::AMIWorkerTimeout    (seconds of idle time before worker threads
//                                 shut themselves down)
//   omniORB::AMIMaxWorkerThreads (limit on the number of spawned worker
//                                 threads)
//   omniORB::AMIMaxQueueSize     (max elements in the queue, or 0 for 
//                                 unbounded)

static omni_mutex     state_lock;
static omni_condition shutdown_cond(&state_lock);

static unsigned int   nWorkers      = 0; // number of active worker threads
static CORBA::Boolean shutting_down = 0; 

static AMI::Queue     *queue        = NULL;
static unsigned long  queue_length  = 0; // number of call descriptors in q 


///////////////////////////////////////////////////////////////////////
// Call Descriptor code ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

// Called by the AMI runtime when the invocation on the target has completed
// and the results are available.
void omniAMIDescriptor::sendReply(){
  AMITRACE("omniAMIDescriptor", "sendReply()");
  if (_is_polling) return giveReplyToPoller();

  omniObjRef *handler = get_handler();

  // A nil reply handler means don't send the reply anywhere.
  if (handler->_is_nil()) return;

  get_handler()->_invoke(*get_reply_cd());
}

// Called whenever an exceptional reply is available
void omniAMIDescriptor::sendException(CORBA::Exception &e, 
				      CORBA::Boolean is_system_exception){
  AMITRACE("omniAMIDescriptror", "sendException");
  // package up the caught exception object into a Type-Specific
  // ExceptionHolder valuetype
  Messaging::ExceptionHolder *holder = get_exception_holder();
  holder->pd_is_system_exception = is_system_exception;
  holder->local_exception_object = CORBA::Exception::_duplicate(&e);

  if (_is_polling) return giveReplyToPoller();
  
  get_handler()->_invoke(*get_exc_cd(*holder));
}


///////////////////////////////////////////////////////////////////////
// AMI::Worker (implementation of worker thread) //////////////////////
///////////////////////////////////////////////////////////////////////

AMI::Worker::Worker(omniAMICall *cd): first_task(cd){
  AMITRACE("AMI::Worker", "constructed");
  
  start_undetached();
}

void *AMI::Worker::run_undetached(void *arg){
  AMITRACE("AMI::Worker", "thread start");

  // First request of this thread can avoid the queue
  if (first_task){
    omniAMICall_var task = first_task;
    process(*first_task);
  }

  while (1){
    AMITRACE("AMI::Worker", "dequeue");
    omniAMICall *call = queue->dequeue(omniORB::AMIWorkerTimeout);
    omniAMICall_var store = call;
    
    if (call){
      {
	omni_mutex_lock lock(state_lock);
	queue_length --;
      }
      process(*call);
      continue;
    }

    AMITRACE("AMI::Worker", "possible timeout?");
    // call == NULL means either a timeout or a signal to shutdown
    {
      omni_mutex_lock lock(state_lock);
      if ((queue_length == 0) || shutting_down){
	nWorkers --;
	if (nWorkers == 0) shutdown_cond.signal();
	shutdown();
	return NULL;
      }
    }
  }
  return NULL;
}
void AMI::Worker::process(omniAMICall& call){
  AMITRACE("AMI::Worker", "processing request");

  // Actual sending and receiving requests is done in the call descriptor
  // class itself.
  try{
    call.sendRequest();

    // received a non-exceptional reply from server
    call.sendReply();

  }catch(CORBA::UserException &e){
    call.sendException(e, 0);
  }catch(CORBA::SystemException &e){
    call.sendException(e, 1);
  }

  AMITRACE("AMI::Worker", "Request completed");
}


void AMI::Worker::shutdown(){
  AMITRACE("AMI::Worker", "shutting down");
}


///////////////////////////////////////////////////////////////////////
// AMI subsystem //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

// Called by the new stub code, _should not block_.
void AMI::enqueue(omniAMICall *cd){
  {
    omni_mutex_lock lock(state_lock);

    // Probably an error- this code is executed by a user thread, and if
    // the ORB is shutting down while user threads are using it...
    if (shutting_down) return;

    if ((nWorkers < omniORB::AMIMaxWorkerThreads)){
      // spawn a new worker thread directly for this request
      // (skipping the queue)
      AMITRACE("AMI", "Spawning a new worker");
      Worker *newWorker = new Worker(cd);
      
      nWorkers++;
      return;
    }
  }
  // queue request (potentially a blocking call if the queue
  // gets full. Mustn't block holding state_lock)
  queue->enqueue(cd);  
  queue_length ++;
    
}

void AMI::shutdown(){
  // tell all the waiting worker threads to shutdown
  omni_mutex_lock lock(state_lock);

  // stop anyone else adding things to the queue
  shutting_down = 1;

  // wake them all up
  for (unsigned int i = 0; i < nWorkers; i++){
    omniAMICall *empty = NULL;
    queue->enqueue(empty);
  }

  // wait for them to complete
  while (nWorkers > 0) shutdown_cond.wait();
  
}


///////////////////////////////////////////////////////////////////////
// initialiser// //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

class omni_AMI_initialiser : public omniInitialiser {
public:

  void attach() {
    AMITRACE("AMI Initialiser", "Creating request Queue");
    queue = new AMI::Queue(omniORB::AMIMaxQueueSize);
  }
  void detach() {
    AMITRACE("AMI Initialiser", "Signalling Worker threads to shut down");
    AMI::shutdown();
    AMITRACE("AMI Initaliser", "Destroying request Queue");
    delete queue;
  }

};

static omni_AMI_initialiser initialiser;
omniInitialiser& omni_AMI_initialiser_ = initialiser;

