// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpEndpoint.h              Created on: 19 Mar 2001
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
  Revision 1.1.2.4  2001/07/31 16:16:16  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.3  2001/07/13 15:34:24  sll
  Added the ability to monitor connections and callback to the giopServer when
  data has arrived at a connection.

  Revision 1.1.2.2  2001/06/20 18:35:15  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.1  2001/04/18 18:10:43  sll
  Big checkin with the brand new internal APIs.

*/

#ifndef __TCPENDPOINT_H__
#define __TCPENDPOINT_H__

OMNI_NAMESPACE_BEGIN(omni)

class tcpConnection;

class tcpEndpoint : public giopEndpoint, public SocketCollection {
public:

  tcpEndpoint(const IIOP::Address& address);
  tcpEndpoint(const char* address);

  // The following implement giopEndpoint abstract functions
  const char* type() const;
  const char* address() const;
  CORBA::Boolean Bind();
  giopConnection* AcceptAndMonitor(giopConnection::notifyReadable_t,void*);
  void Poke();
  void Shutdown();

  ~tcpEndpoint();

protected:
  CORBA::Boolean notifyReadable(SocketHandle_t);
  // implement SocketCollection::notifyReadable
  

private:
  SocketHandle_t       pd_socket;
  IIOP::Address        pd_address;
  CORBA::String_var    pd_address_string;

  SocketHandle_t                   pd_new_conn_socket;
  giopConnection::notifyReadable_t pd_callback_func;
  void*                            pd_callback_cookie;

  tcpEndpoint();
  tcpEndpoint(const tcpEndpoint&);
  tcpEndpoint& operator=(const tcpEndpoint&);
};


class tcpActiveConnection;

class tcpActiveCollection : public giopActiveCollection, 
			    public SocketCollection {
public:
  const char* type() const;
  // implement giopActiveCollection::type

  void Monitor(giopConnection::notifyReadable_t func, void* cookie);
  // implement giopActiveCollection::Monitor

  CORBA::Boolean isEmpty() const;
  // implement giopActiveCollection::isEmpty

  tcpActiveCollection();
  ~tcpActiveCollection();

  friend class tcpActiveConnection;

protected:
  CORBA::Boolean notifyReadable(SocketHandle_t);
  // implement SocketCollection::notifyReadable

  void addMonitor(SocketHandle_t);
  void removeMonitor(SocketHandle_t);

private:
  CORBA::ULong      pd_n_sockets;
  omni_tracedmutex  pd_lock;

  giopConnection::notifyReadable_t pd_callback_func;
  void*                            pd_callback_cookie;

  tcpActiveCollection(const tcpActiveCollection&);
  tcpActiveCollection& operator=(const tcpActiveCollection&);
};

OMNI_NAMESPACE_END(omni)

#endif // __TCPENDPOINT_H__
