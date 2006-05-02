// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpActive.cc               Created on: 24 Jul 2001
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
  Revision 1.1.4.3  2006/05/02 13:07:13  dgrisby
  Idle giopMonitor SocketCollections would not exit at shutdown.

  Revision 1.1.4.2  2005/01/13 21:10:02  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.1  2003/03/23 21:01:58  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.2  2002/08/21 06:23:16  dgrisby
  Properly clean up bidir connections and ropes. Other small tweaks.

  Revision 1.1.2.1  2001/07/31 16:16:21  sll
  New transport interface to support the monitoring of active connections.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <SocketCollection.h>
#include <tcp/tcpConnection.h>
#include <tcp/tcpEndpoint.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(tcpActive);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
static tcpActiveCollection myCollection;

/////////////////////////////////////////////////////////////////////////
tcpActiveCollection::tcpActiveCollection() : pd_n_sockets(0), pd_shutdown(0) {}

/////////////////////////////////////////////////////////////////////////
tcpActiveCollection::~tcpActiveCollection() {}

/////////////////////////////////////////////////////////////////////////
const char*
tcpActiveCollection::type() const {
  return "giop:tcp";
}

/////////////////////////////////////////////////////////////////////////
void
tcpActiveCollection::Monitor(giopConnection::notifyReadable_t func,
			     void* cookie) {

  pd_callback_func = func;
  pd_callback_cookie = cookie;

  while (!isEmpty()) {
    if (!Select()) break;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpActiveCollection::notifyReadable(SocketHolder* conn) {

  pd_callback_func(pd_callback_cookie,(tcpConnection*)conn);
  return 1;
}


/////////////////////////////////////////////////////////////////////////
void
tcpActiveCollection::addMonitor(SocketHandle_t) {
  omni_tracedmutex_lock sync(pd_lock);
  pd_n_sockets++;
  pd_shutdown = 0;
}

/////////////////////////////////////////////////////////////////////////
void
tcpActiveCollection::removeMonitor(SocketHandle_t) {
  omni_tracedmutex_lock sync(pd_lock);
  pd_n_sockets--;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpActiveCollection::isEmpty() const {
  // Cast tracedmutex to escape this function's constness.
  omni_tracedmutex_lock sync((omni_tracedmutex&)pd_lock);
  return (pd_n_sockets == 0 || pd_shutdown);
}

/////////////////////////////////////////////////////////////////////////
void
tcpActiveCollection::deactivate() {
  omni_tracedmutex_lock sync(pd_lock);
  pd_shutdown = 1;
  wakeUp();
}

/////////////////////////////////////////////////////////////////////////
tcpActiveConnection::tcpActiveConnection(SocketHandle_t sock) : 
  tcpConnection(sock,&myCollection), pd_registered(0) {
}

/////////////////////////////////////////////////////////////////////////
tcpActiveConnection::~tcpActiveConnection() {
  if (pd_registered) {
    myCollection.removeMonitor(pd_socket);
  }
}


/////////////////////////////////////////////////////////////////////////
giopActiveCollection*
tcpActiveConnection::registerMonitor() {

  if (pd_registered) return &myCollection;

  pd_registered = 1;
  myCollection.addMonitor(pd_socket);
  return &myCollection;
}

/////////////////////////////////////////////////////////////////////////
giopConnection&
tcpActiveConnection::getConnection() {
  return *this;
}


OMNI_NAMESPACE_END(omni)
