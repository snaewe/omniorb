// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslEndpoint.h              Created on: 29 May 2001
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
  Revision 1.1.4.3  2006/04/09 19:52:31  dgrisby
  More IPv6, endPointPublish parameter.

  Revision 1.1.4.2  2005/01/13 21:10:01  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.1  2003/03/23 21:01:59  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.6  2002/09/09 22:11:51  dgrisby
  SSL transport cleanup even if certificates are wrong.

  Revision 1.1.2.5  2002/08/21 06:23:16  dgrisby
  Properly clean up bidir connections and ropes. Other small tweaks.

  Revision 1.1.2.4  2001/07/31 16:16:22  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.3  2001/07/13 15:36:53  sll
  Added the ability to monitor connections and callback to the giopServer
  when data has arrived at a connection.

  Revision 1.1.2.2  2001/06/20 18:35:16  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.1  2001/06/11 18:11:06  sll
  *** empty log message ***


*/

#ifndef __SSLENDPOINT_H__
#define __SSLENDPOINT_H__

#include <omniORB4/omniServer.h>

OMNI_NAMESPACE_BEGIN(omni)

class sslConnection;

class sslEndpoint : public giopEndpoint,
		    public SocketCollection,
		    public SocketHolder {
public:

  sslEndpoint(const IIOP::Address& address, sslContext* ctx);
  const char* type() const;
  const char* address() const;
  const orbServer::EndpointList* addresses() const;
  CORBA::Boolean publish(const orbServer::PublishSpecs& publish_specs,
			 CORBA::Boolean 	  	all_specs,
			 CORBA::Boolean 	  	all_eps,
			 orbServer::EndpointList& 	published_eps);
  CORBA::Boolean Bind();
  giopConnection* AcceptAndMonitor(giopConnection::notifyReadable_t,void*);
  void Poke();
  void Shutdown();

  ~sslEndpoint();

protected:
  CORBA::Boolean notifyReadable(SocketHolder*);
  // implement SocketCollection::notifyReadable

 private:
  IIOP::Address                    pd_address;
  orbServer::EndpointList          pd_addresses;
  sslContext*                      pd_ctx;

  SocketHandle_t                   pd_new_conn_socket;
  giopConnection::notifyReadable_t pd_callback_func;
  void*                            pd_callback_cookie;
  int                              pd_go;

  sslEndpoint();
  sslEndpoint(const sslEndpoint&);
  sslEndpoint& operator=(const sslEndpoint&);
};


class sslActiveConnection;

class sslActiveCollection : public giopActiveCollection, 
			    public SocketCollection {
public:
  const char* type() const;
  // implement giopActiveCollection::type

  void Monitor(giopConnection::notifyReadable_t func, void* cookie);
  // implement giopActiveCollection::Monitor

  CORBA::Boolean isEmpty() const;
  // implement giopActiveCollection::isEmpty

  void deactivate();
  // implement giopActiveCollection::deactivate

  sslActiveCollection();
  ~sslActiveCollection();

  friend class sslActiveConnection;

protected:
  CORBA::Boolean notifyReadable(SocketHolder*);
  // implement SocketCollection::notifyReadable

  void addMonitor(SocketHandle_t);
  void removeMonitor(SocketHandle_t);

private:
  CORBA::ULong      pd_n_sockets;
  CORBA::Boolean    pd_shutdown;
  omni_tracedmutex  pd_lock;

  giopConnection::notifyReadable_t pd_callback_func;
  void*                            pd_callback_cookie;

  sslActiveCollection(const sslActiveCollection&);
  sslActiveCollection& operator=(const sslActiveCollection&);
};

OMNI_NAMESPACE_END(omni)

#endif // __SSLENDPOINT_H__
