// -*- Mode: C++; -*-
//                            Package   : omniORB
// deferredRequest.h          Created on: 10/1998
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
//  An invoker task used to implement deferred requests for the
//  Dynamic Invocation Interface.
//

#ifndef __DEFERREDREQUEST_H__
#define __DEFERREDREQUEST_H__

#include <omniORB4/CORBA.h>
#include <request.h>


OMNI_NAMESPACE_BEGIN(omni)

class DeferredRequest : public omniTask {

public:
  DeferredRequest(RequestImpl* request);
  // Inserts task in the ORB invoker as an AnyTime task.

  inline CORBA::Boolean poll_response() {
    omni_tracedmutex_lock lock(pd_readyMutex);
    return pd_ready;
  }
  // Returns true if the operation has completed, false otherwise.

  inline void get_response() {
    omni_tracedmutex_lock lock(pd_readyMutex);
    while( !pd_ready )  pd_readyCondition.wait();
  }
  // Blocks until the operation has completed.

  inline CORBA::Exception* get_exception() {
    CORBA::Exception* e = pd_exception;
    pd_exception = 0;
    return e;
  }
  // If there is one, transfer ownership of a stored
  // exception to the caller.

  inline void die() { OMNIORB_ASSERT(pd_ready); delete this; }

  virtual void execute();

protected:
  virtual ~DeferredRequest();

  RequestImpl*            pd_request;
  CORBA::Boolean          pd_ready;
  omni_tracedmutex        pd_readyMutex;
  omni_tracedcondition    pd_readyCondition;
  CORBA::Exception*       pd_exception;
};

OMNI_NAMESPACE_END(omni)

#endif  // __DEFERREDREQUEST_H__
