// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslConnection.h            Created on: 19 Mar 2001
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
  Revision 1.1.4.1  2003/03/23 21:01:59  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.5  2001/07/31 16:16:23  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.4  2001/07/13 15:36:24  sll
  Revised declaration to match the changes in giopConnection.

  Revision 1.1.2.3  2001/06/20 18:35:16  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.2  2001/06/18 20:28:31  sll
  Remove garbage after #endif

  Revision 1.1.2.1  2001/06/11 18:11:06  sll
  *** empty log message ***


*/

#ifndef __SSLCONNECTION_H__
#define __SSLCONNECTION_H__


#include <SocketCollection.h>
#include <openssl/ssl.h>

OMNI_NAMESPACE_BEGIN(omni)

class sslEndpoint;

class sslConnection : public giopConnection, public SocketLink {
 public:

  int Send(void* buf, size_t sz,
	   unsigned long deadline_secs = 0,
	   unsigned long deadline_nanosecs = 0);

  int Recv(void* buf, size_t sz,
	   unsigned long deadline_secs = 0,
	   unsigned long deadline_nanosecs = 0);

  void Shutdown();

  const char* myaddress();

  const char* peeraddress();

  void setSelectable(CORBA::Boolean now = 0,CORBA::Boolean data_in_buffer = 0);

  void clearSelectable();

  void Peek(giopConnection::notifyReadable_t func,void* cookie);

  SocketHandle_t handle() const { return pd_socket; }
  ::SSL*            ssl_handle() const { return pd_ssl; }

  sslConnection(SocketHandle_t,::SSL*,SocketCollection*);

  ~sslConnection();


 private:
  ::SSL*            pd_ssl;
  SocketCollection* pd_belong_to;
  CORBA::String_var pd_myaddress;
  CORBA::String_var pd_peeraddress;

};


class sslActiveConnection : public giopActiveConnection, public sslConnection {
public:
  giopActiveCollection* registerMonitor();
  giopConnection& getConnection();

  sslActiveConnection(SocketHandle_t,::SSL*);
  ~sslActiveConnection();

private:
  CORBA::Boolean pd_registered;

  sslActiveConnection(const sslActiveConnection&);
  sslActiveConnection& operator=(const sslActiveConnection&);
};


OMNI_NAMESPACE_END(omni)

#endif //__SSLCONNECTION_H__
