// -*- Mode: C++; -*-
//                            Package   : omniORB2
// deferredRequest.cc         Created on: 10/1998
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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


DeferredRequest::DeferredRequest(CORBA::Request_ptr request)
  : pd_readyCondition(&pd_readyMutex)
{
  if( CORBA::is_nil(request) )
    throw omniORB::fatalException(__FILE__,__LINE__,
         "DeferredRequest::DeferredRequest(CORBA::Request_ptr)");

  pd_request = request;
  pd_ready = 0;
  pd_exception = 0;
  start_undetached();
}


DeferredRequest::~DeferredRequest()
{
  if( pd_exception )  delete pd_exception;
}


void*
DeferredRequest::run_undetached(void* arg)
{
  // Invoke the requested operation. If a (system) exception is
  // thrown, it is saved so that it can be thrown to the thread
  // which calls poll_response() or get_response().

  try{
    try{
      pd_request->invoke();
    }
    catch(CORBA::SystemException& ex){
      if( omniORB::diiThrowsSysExceptions )
	// Store the exception so that it can be thrown later
	pd_exception = CORBA::Exception::_duplicate(&ex);
      else{
	if( omniORB::traceLevel > 0 ){
	  omniORB::log <<
	    "omniORB2 Bug: file <" << __FILE__ << ">, line <" << __LINE__ <<
	    "\n Request->invoke() raised a system exception.\n"
	    " omni::diiThrowsSysExceptions = " <<
	    omniORB::diiThrowsSysExceptions << ".\n";
	  omniORB::log.flush();
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
    omni_mutex_lock lock(pd_readyMutex);
    pd_ready = 1;
  }
  pd_readyCondition.signal();

  return 0;
}


void
DeferredRequest::done()
{
  if( !pd_ready )
    throw omniORB::fatalException(__FILE__,__LINE__,
        "DeferredRequest::done() when not ready");

  // Grab the exception to prevent it being deleted when <this> is
  // deleted. Ensure that it is freed after we have thrown a copy of it.
  CORBA::Exception* ex = pd_exception;
  pd_exception = 0;

  this->join(0);  // 'this' will now have been 'delete'd

  try {
    if( ex && omniORB::diiThrowsSysExceptions )
      ex->_raise();
  }
  catch(...) {
    // I'm not sure if this is strictly speaking legal ... but it
    // seems to work!!
    if( ex )  delete ex;
    throw;
  }
}
