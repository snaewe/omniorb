// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniServer.h               Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.1.4.1  2003/03/23 21:04:09  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.1  2001/04/18 17:26:28  sll
  Big checkin with the brand new internal APIs.

*/

#ifndef __OMNISERVER_H__
#define __OMNISERVER_H__

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class orbServer {
public:
  orbServer() {}
  virtual ~orbServer() {}

  virtual const char* instantiate(const char* endpoint_uri,
				  _CORBA_Boolean no_publish,
				  _CORBA_Boolean no_listen) = 0;
  // Accept to serve requests coming in from this endpoint .
  // If start() has already been called, incoming
  // requests from this endpoint will be served immediately.
  //
  // If <no_publish> is TRUE(1), the endpoint should not be published
  // in the IOR of the local objects.
  //
  // If <no_listen> is TRUE(1), the endpoint do not even need to be served.
  // For instance, this may be the address of another process which serve
  // as this process's backup. Nevertheless, the endpoint should be
  // published in the IOR of the local objects. Even though the endpoint will
  // not served by this object, only return TRUE(1) if it is indeed a valid
  // endpoint that this server recognise.
  //
  // If the instantiation is successful, returns the established uri,
  // otherwise returns 0.
  //
  // This function does not raise an exception.
  // 
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object is synchronised 
  //    internally.

  virtual void start() = 0;
  // When this function returns, the server will service requests.
  //
  // This function does not raise an exception.
  //
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object is synchronised 
  //    internally.

  virtual void stop() = 0;
  // When this function returns, the server will no longer serve the requests.
  // Existing strands will be shutdown. 
  // However, the endpoints will stay.
  // This server will serve incoming requests again when start() is called.
  //
  // This function does not raise an exception.
  //
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object is synchronised 
  //    internally.

  virtual void remove() = 0;
  // When this function returns, all endpoints will be removed.
  //
  // This function does not raise an exception.
  //
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object is synchronised 
  //    internally.

};

OMNI_NAMESPACE_END(omni)

#endif // __OMNISERVER_H__
