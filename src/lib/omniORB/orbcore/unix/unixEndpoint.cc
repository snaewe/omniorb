// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixEndpoint.cc            Created on: 6 Aug 2001
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
  Revision 1.1.2.2  2001/08/07 15:42:17  sll
  Make unix domain connections distinguishable on both the server and client
  side.

  Revision 1.1.2.1  2001/08/06 15:47:44  sll
  Added support to use the unix domain socket as the local transport.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <SocketCollection.h>
#include <unix/unixConnection.h>
#include <unix/unixAddress.h>
#include <unix/unixEndpoint.h>
#include <sys/un.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixEndpoint);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
unixEndpoint::unixEndpoint(const char* filename) :
  pd_socket(RC_INVALID_SOCKET),
  pd_new_conn_socket(RC_INVALID_SOCKET), pd_callback_func(0),
  pd_callback_cookie(0) {

  pd_filename = filename;
  pd_address_string = unixConnection::unToString(filename);
}

/////////////////////////////////////////////////////////////////////////
unixEndpoint::~unixEndpoint() {
  if (pd_socket != RC_INVALID_SOCKET) {
    unlink(pd_filename);
    CLOSESOCKET(pd_socket);
    pd_socket = RC_INVALID_SOCKET;
  }
}

/////////////////////////////////////////////////////////////////////////
const char*
unixEndpoint::type() const {
  return "giop:unix";
}

/////////////////////////////////////////////////////////////////////////
const char*
unixEndpoint::address() const {
  return pd_address_string;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixEndpoint::Bind() {

  OMNIORB_ASSERT(pd_socket == RC_INVALID_SOCKET);

  if ((pd_socket = socket(AF_LOCAL,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return 0;
  }

  unlink(pd_filename);

  struct sockaddr_un addr;

  memset((void*)&addr,0,sizeof(addr));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, pd_filename, sizeof(addr.sun_path) - 1);

  if (::bind(pd_socket,(struct sockaddr *)&addr,
	               sizeof(addr)) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }

  if (listen(pd_socket,5) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }

  pd_address_string = unixConnection::unToString(pd_filename);
  return 1;
}

/////////////////////////////////////////////////////////////////////////
void
unixEndpoint::Poke() {

  unixAddress* target = new unixAddress(pd_address_string);
  giopActiveConnection* conn;
  if ((conn = target->Connect()) == 0) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Warning: Fail to connect to myself ("
	  << (const char*) pd_address_string << ") via tcp!\n";
      log << "Warning: ATM this is ignored but this may cause the ORB shutdown to hang.\n";
    }
  }
  else {
    delete conn;
  }
  delete target;
}

/////////////////////////////////////////////////////////////////////////
void
unixEndpoint::Shutdown() {
  SHUTDOWNSOCKET(pd_socket);
}

/////////////////////////////////////////////////////////////////////////
giopConnection*
unixEndpoint::AcceptAndMonitor(giopConnection::notifyReadable_t func,
			      void* cookie) {

  OMNIORB_ASSERT(pd_socket != RC_INVALID_SOCKET);

  pd_callback_func = func;
  pd_callback_cookie = cookie;
  setSelectable(pd_socket,1,0,0);

  while (1) {
    pd_new_conn_socket = RC_INVALID_SOCKET;
    if (!Select()) break;
    if (pd_new_conn_socket != RC_INVALID_SOCKET) {
      return  new unixConnection(pd_new_conn_socket,this,pd_filename,0);
    }
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixEndpoint::notifyReadable(SocketHandle_t fd) {

  if (fd == pd_socket) {
    SocketHandle_t sock;
    sock = ::accept(pd_socket,0,0);
    if (sock == RC_SOCKET_ERROR) {
      return 0;
    }
    pd_new_conn_socket = sock;
    setSelectable(pd_socket,1,0,1);
    return 1;
  }
  SocketLink* conn = findSocket(fd,1);
  if (conn) {
    pd_callback_func(pd_callback_cookie,(unixConnection*)conn);
  }
  return 1;
}

OMNI_NAMESPACE_END(omni)
