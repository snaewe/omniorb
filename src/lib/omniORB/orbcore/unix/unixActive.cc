// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixActive.cc              Created on: 6 Aug 2001
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
  Revision 1.1.2.1  2001/08/06 15:47:43  sll
  Added support to use the unix domain socket as the local transport.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <SocketCollection.h>
#include <unix/unixConnection.h>
#include <unix/unixEndpoint.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixActive);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
static unixActiveCollection myCollection;

/////////////////////////////////////////////////////////////////////////
unixActiveCollection::unixActiveCollection() : pd_n_sockets(0) {}

/////////////////////////////////////////////////////////////////////////
unixActiveCollection::~unixActiveCollection() {}

/////////////////////////////////////////////////////////////////////////
const char*
unixActiveCollection::type() const {
  return "giop:unix";
}

/////////////////////////////////////////////////////////////////////////
void
unixActiveCollection::Monitor(giopConnection::notifyReadable_t func,
			     void* cookie) {

  pd_callback_func = func;
  pd_callback_cookie = cookie;

  CORBA::Boolean doit;
  while (!isEmpty()) {
    if (!Select()) break;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixActiveCollection::notifyReadable(SocketHandle_t fd) {

  SocketLink* conn = findSocket(fd,1);
  if (conn) {
    pd_callback_func(pd_callback_cookie,(unixConnection*)conn);
  }
  return 1;
}


/////////////////////////////////////////////////////////////////////////
void
unixActiveCollection::addMonitor(SocketHandle_t) {
  omni_tracedmutex_lock sync(pd_lock);
  pd_n_sockets++;
}

/////////////////////////////////////////////////////////////////////////
void
unixActiveCollection::removeMonitor(SocketHandle_t) {
  omni_tracedmutex_lock sync(pd_lock);
  pd_n_sockets--;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixActiveCollection::isEmpty() const {
  omni_tracedmutex_lock sync((omni_tracedmutex&)pd_lock);
  return (pd_n_sockets == 0);
}

/////////////////////////////////////////////////////////////////////////
unixActiveConnection::unixActiveConnection(SocketHandle_t sock) : 
  unixConnection(sock,&myCollection), pd_registered(0) {
}

/////////////////////////////////////////////////////////////////////////
unixActiveConnection::~unixActiveConnection() {
  if (pd_registered) {
    myCollection.removeMonitor(pd_socket);
  }
}


/////////////////////////////////////////////////////////////////////////
giopActiveCollection*
unixActiveConnection::registerMonitor() {

  if (pd_registered) return &myCollection;

  pd_registered = 1;
  myCollection.addMonitor(pd_socket);
  return &myCollection;
}

/////////////////////////////////////////////////////////////////////////
giopConnection&
unixActiveConnection::getConnection() {
  return *this;
}


OMNI_NAMESPACE_END(omni)
