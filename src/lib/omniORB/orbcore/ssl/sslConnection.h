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
  Revision 1.1.2.2  2001/06/18 20:28:31  sll
  Remove garbage after #endif

  Revision 1.1.2.1  2001/06/11 18:11:06  sll
  *** empty log message ***


*/

#ifndef __SSLCONNECTION_H__
#define __SSLCONNECTION_H__


#include <tcp/tcpConnection.h>
#include <openssl/ssl.h>

OMNI_NAMESPACE_BEGIN(omni)

class sslConnection : public giopConnection {
 public:

  int send(void* buf, size_t sz,
	   unsigned long deadline_secs = 0,
	   unsigned long deadline_nanosecs = 0);

  int recv(void* buf, size_t sz,
	   unsigned long deadline_secs = 0,
	   unsigned long deadline_nanosecs = 0);

  void shutdown();

  const char* myaddress();

  const char* peeraddress();

  tcpSocketHandle_t handle() const { return pd_socket; }
  ::SSL*            ssl_handle() const { return pd_ssl; }

  sslConnection(tcpSocketHandle_t,::SSL*);

  ~sslConnection();

 private:
  tcpSocketHandle_t pd_socket;
  ::SSL*            pd_ssl;
  CORBA::String_var pd_myaddress;
  CORBA::String_var pd_peeraddress;
};

OMNI_NAMESPACE_END(omni)

#endif //__SSLCONNECTION_H__
