// -*- Mode: C++; -*-
//                            Package   : omniORB
// deferredRequest.cc         Created on: 10/1998
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//  A thread class (derived from omni_thread) used to implement
//  deferred requests for the Dynamic Invocation Interface.
//

#include <deferredRequest.h>
#include <orbParameters.h>
#include <invoker.h>

OMNI_NAMESPACE_BEGIN(omni)

DeferredRequest::DeferredRequest(RequestImpl* request)
  : omniTask(omniTask::AnyTime), pd_readyCondition(&pd_readyMutex)
{
  if( CORBA::is_nil(request) )
    throw omniORB::fatalException(__FILE__,__LINE__,
         "DeferredRequest::DeferredRequest(CORBA::Request_ptr)");

  pd_request = request;
  pd_ready = 0;
  pd_exception = 0;
  orbAsyncInvoker->insert(this);
}


DeferredRequest::~DeferredRequest()
{
  if( pd_exception )  delete pd_exception;
}


void
DeferredRequest::execute()
{
  // Invoke the requested operation. If a (system) exception is
  // thrown, it is saved so that it can be thrown to the thread
  // which calls poll_response() or get_response().

  try{
    try{
      pd_request->deferred_invoke();
    }
    catch(CORBA::SystemException& ex){
      if( orbParameters::diiThrowsSysExceptions )
	// Store the exception so that it can be thrown later
	pd_exception = CORBA::Exception::_duplicate(&ex);
      else{
	if( omniORB::traceLevel > 0 ){
	  omniORB::logger log;
	  log <<
	    "omniORB: BUG: file <" << __FILE__ << ">, line " << __LINE__ <<
	    "\n Request->invoke() raised a system exception.\n"
	    " omni::diiThrowsSysExceptions = " <<
	    orbParameters::diiThrowsSysExceptions << ".\n";
	}
      }
    }
  }
  catch(...){
    // If we get here then ex._duplicate() probably threw an exception
    // (NO_MEMORY). We cannot pass exceptions out of this function, so
    // we will just have to silently ignore it.
  }

  {
    omni_tracedmutex_lock lock(pd_readyMutex);
    pd_ready = 1;
  }
  pd_readyCondition.signal();
}

OMNI_NAMESPACE_END(omni)
