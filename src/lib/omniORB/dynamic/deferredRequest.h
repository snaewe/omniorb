// -*- Mode: C++; -*-
//                            Package   : omniORB2
// deferredRequest.h          Created on: 10/1998
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

#ifndef __DEFERREDREQUEST_H__
#define __DEFERREDREQUEST_H__

#include <omniORB2/CORBA.h>


class DeferredRequest : public omni_thread {

public:
  DeferredRequest(CORBA::Request_ptr request);
  // Spawns off a new thread which invokes the operation
  // given in the request.

  CORBA::Boolean poll_response() {
    CORBA::Boolean ready;
    { omni_mutex_lock lock(pd_readyMutex);
      ready = pd_ready;
    }
    if( ready )  done();
    return ready;
  }
  // Returns true if the operation has completed, false otherwise.
  // If the invocation generated a system exception it is thrown
  // out of this function.

  void get_response() {
    { omni_mutex_lock lock(pd_readyMutex);
      while( !pd_ready )  pd_readyCondition.wait();
    }
    done();
  }
  // Blocks until the operation has completed, or if a system exception
  // is generated it is thrown.

protected:
  virtual ~DeferredRequest();

private:
  virtual void* run_undetached(void* arg);
  void done();

  CORBA::Request_ptr      pd_request;
  CORBA::Boolean          pd_ready;
  omni_mutex              pd_readyMutex;
  omni_condition          pd_readyCondition;
  CORBA::Exception*       pd_exception;
};


#endif  // __DEFERREDREQUEST_H__
