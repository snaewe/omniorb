// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniServer.h               Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2006 Apasphere Ltd
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
  Revision 1.1.4.4  2009/05/06 16:16:12  dgrisby
  Update lots of copyright notices.

  Revision 1.1.4.3  2006/10/09 09:47:12  dgrisby
  Only delete giopServer if all threads are successfully shut down.

  Revision 1.1.4.2  2006/04/09 19:52:31  dgrisby
  More IPv6, endPointPublish parameter.

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

protected:
  // Only deleted by remove().
  virtual ~orbServer() {}

public:

  typedef _CORBA_Unbounded_Sequence_String PublishSpecs;
  typedef _CORBA_Unbounded_Sequence_String EndpointList;

  virtual _CORBA_Boolean instantiate(const char*    endpoint_uri,
				     _CORBA_Boolean no_publish,
				     EndpointList&  listening_endpoints) = 0;
  // Accept to serve requests coming in from this endpoint. If
  // start() has already been called, incoming requests from this
  // endpoint will be served immediately.
  //
  // If <no_publish> is TRUE(1), the endpoint should not be published
  // in the IORs of the local objects.
  //
  // The URIs for all endpoints that can be used to contact the
  // instantiated server are added to <listening_endpoints>.
  //
  // If the instantiation is successful, returns TRUE(1) otherwise
  // returns FALSE(0).
  //
  // This function does not raise any exceptions.
  // 
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object are synchronised 
  //    internally.

  virtual _CORBA_Boolean publish(const PublishSpecs& publish_specs,
				 _CORBA_Boolean      all_specs,
				 _CORBA_Boolean      all_eps,
				 EndpointList&       published_endpoints) = 0;
  // For each endpoint instantiated in the server, publish its details
  // in IORs according to the <publish_specs> and flags, and add the
  // endpoint URIs to <published_endpoints>.
  //
  // Any URIs that are already in <published_endpoints> are not added
  // again.
  //
  // Returns TRUE(1) if the <publish_specs> were understood; FALSE(0)
  // if not.
  //
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object are synchronised 
  //    internally.

  virtual void start() = 0;
  // When this function returns, the server will service requests.
  //
  // This function does not raise any exceptions.
  //
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object are synchronised 
  //    internally.

  virtual void stop() = 0;
  // When this function returns, the server will no longer serve the requests.
  // Existing strands will be shutdown. 
  // However, the endpoints will stay.
  // This server will serve incoming requests again when start() is called.
  //
  // This function does not raise any exceptions.
  //
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object are synchronised 
  //    internally.

  virtual void remove() = 0;
  // When this function returns, all endpoints will be removed and
  // this server object will be deleted.
  //
  // This function does not raise any exceptions.
  //
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object are synchronised 
  //    internally.

};

OMNI_NAMESPACE_END(omni)

#endif // __OMNISERVER_H__
