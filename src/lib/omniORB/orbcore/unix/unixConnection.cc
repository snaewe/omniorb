// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixConnection.cc          Created on: 6 Aug 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2005 Apasphere Ltd
//    Copyright (C) 2001      AT&T Laboratories Cambridge
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
  Revision 1.1.4.7  2009/05/06 16:14:44  dgrisby
  Update lots of copyright notices.

  Revision 1.1.4.6  2005/09/01 14:52:12  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.5  2005/03/02 12:39:16  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.4  2005/03/02 12:10:48  dgrisby
  setSelectable / Peek fixes.

  Revision 1.1.4.3  2005/01/13 21:10:16  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.2  2005/01/06 23:10:57  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:01:57  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.6  2002/03/21 10:59:13  dpg1
  HPUX fixes.

  Revision 1.1.2.5  2002/01/15 16:38:14  dpg1
  On the road to autoconf. Dependencies refactored, configure.ac
  written. No makefiles yet.

  Revision 1.1.2.4  2001/11/28 20:33:43  dpg1
  Minor Unix transport bugs.

  Revision 1.1.2.3  2001/08/24 15:56:45  sll
  Fixed code which made the wrong assumption about the semantics of
  do { ...; continue; } while(0)

  Revision 1.1.2.2  2001/08/07 15:42:17  sll
  Make unix domain connections distinguishable on both the server and client
  side.

  Revision 1.1.2.1  2001/08/06 15:47:44  sll
  Added support to use the unix domain socket as the local transport.


*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <orbParameters.h>
#include <SocketCollection.h>
#include <unix/unixConnection.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>
#include <sys/un.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixConnection);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
int
unixConnection::Send(void* buf, size_t sz,
		    unsigned long deadline_secs,
		    unsigned long deadline_nanosecs) {

  if (sz > orbParameters::maxSocketSend)
    sz = orbParameters::maxSocketSend;

  int tx;

  do {

    struct timeval t;

    if (deadline_secs || deadline_nanosecs) {
      SocketSetTimeOut(deadline_secs,deadline_nanosecs,t);
      if (t.tv_sec == 0 && t.tv_usec == 0) {
	// Already timeout.
	return 0;
      }
      else {
#if defined(USE_POLL)
	struct pollfd fds;
	fds.fd = pd_socket;
	fds.events = POLLOUT;
	tx = poll(&fds,1,t.tv_sec*1000+(t.tv_usec/1000));
#else
	fd_set fds, efds;
	FD_ZERO(&fds);
	FD_ZERO(&efds);
	FD_SET(pd_socket,&fds);
	FD_SET(pd_socket,&efds);
	tx = select(pd_socket+1,0,&fds,&efds,&t);
#endif
	if (tx == 0) {
	  // Time out!
	  return 0;
	}
	else if (tx == RC_SOCKET_ERROR) {
	  if (ERRNO == RC_EINTR)
	    continue;
	  else {
	    return -1;
	  }
	}
      }
    }

    // Reach here if we can write without blocking or we don't
    // care if we block here.
    if ((tx = ::send(pd_socket,(char*)buf,sz,0)) == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EINTR)
	continue;
      else
	return -1;
    }
    else if (tx == 0)
      return -1;

    break;

  } while(1);

  return tx;

}

/////////////////////////////////////////////////////////////////////////
int
unixConnection::Recv(void* buf, size_t sz,
		    unsigned long deadline_secs,
		    unsigned long deadline_nanosecs) {

  if (sz > orbParameters::maxSocketRecv)
    sz = orbParameters::maxSocketRecv;

  int rx;

  do {

    if (pd_shutdown)
      return -1;

    struct timeval t;

    if (deadline_secs || deadline_nanosecs) {
      SocketSetTimeOut(deadline_secs,deadline_nanosecs,t);
      if (t.tv_sec == 0 && t.tv_usec == 0) {
	// Already timeout.
	return 0;
      }
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      if (orbParameters::scanGranularity > 0 && 
	  t.tv_sec > orbParameters::scanGranularity) {
	t.tv_sec = orbParameters::scanGranularity;
      }
#endif
    }
    else {
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      t.tv_sec = orbParameters::scanGranularity;
      t.tv_usec = 0;
#else
      t.tv_sec = t.tv_usec = 0;
#endif
    }

    if (t.tv_sec || t.tv_usec) {
#if defined(USE_POLL)
      struct pollfd fds;
      fds.fd = pd_socket;
      fds.events = POLLIN;
      rx = poll(&fds,1,t.tv_sec*1000+(t.tv_usec/1000));
#else
      fd_set fds, efds;
      FD_ZERO(&fds);
      FD_ZERO(&efds);
      FD_SET(pd_socket,&fds);
      FD_SET(pd_socket,&efds);
      rx = select(pd_socket+1,&fds,0,&efds,&t);
#endif
      if (rx == 0) {
	// Time out!
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
	continue;
#else
	return 0;
#endif
      }
      else if (rx == RC_SOCKET_ERROR) {
	if (ERRNO == RC_EINTR)
	  continue;
	else {
	  return -1;
	}
      }
    }

    // Reach here if we can read without blocking or we don't
    // care if we block here.
    if ((rx = ::recv(pd_socket,(char*)buf,sz,0)) == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EINTR)
	continue;
      else
	return -1;
    }
    else if (rx == 0)
      return -1;

    break;

  } while(1);

  return rx;
}

/////////////////////////////////////////////////////////////////////////
void
unixConnection::Shutdown() {
  SHUTDOWNSOCKET(pd_socket);
  pd_shutdown = 1;
}

/////////////////////////////////////////////////////////////////////////
const char*
unixConnection::myaddress() {
  return (const char*)pd_myaddress;
}

/////////////////////////////////////////////////////////////////////////
const char*
unixConnection::peeraddress() {
  return (const char*)pd_peeraddress;
}

/////////////////////////////////////////////////////////////////////////
unixConnection::unixConnection(SocketHandle_t sock, 
			       SocketCollection* belong_to,
			       const char* filename,
			       CORBA::Boolean isActive) : 
  SocketHolder(sock) {

  static CORBA::ULong suffix = 0;

  CORBA::String_var filename_1;
  filename_1 = CORBA::string_alloc(strlen(filename)+12);
  sprintf(filename_1,"%s %08x",filename,(unsigned int)++suffix);

  if (isActive) {
    pd_myaddress = unToString(filename_1);
    pd_peeraddress = unToString(filename);
  }
  else {
    pd_myaddress = unToString(filename);
    pd_peeraddress = unToString(filename_1);
  }

  SocketSetCloseOnExec(sock);

  belong_to->addSocket(this);
}

/////////////////////////////////////////////////////////////////////////
unixConnection::~unixConnection() {
  clearSelectable();
  pd_belong_to->removeSocket(this);
  CLOSESOCKET(pd_socket);
}

/////////////////////////////////////////////////////////////////////////
void
unixConnection::setSelectable(int now,
			      CORBA::Boolean data_in_buffer) {

  SocketHolder::setSelectable(now,data_in_buffer);
}


/////////////////////////////////////////////////////////////////////////
void
unixConnection::clearSelectable() {

  SocketHolder::clearSelectable();
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixConnection::isSelectable() {
  return pd_belong_to->isSelectable(pd_socket);
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixConnection::Peek() {
  return SocketHolder::Peek();
}

/////////////////////////////////////////////////////////////////////////
char*
unixConnection::unToString(const char* filename) {

  const char* format = "giop:unix:%s";

  CORBA::ULong len = strlen(filename);
  if (len == 0) {
    filename = "<not bound>";
    len = strlen(filename);
  }
  len += strlen(format);
  CORBA::String_var addrstr(CORBA::string_alloc(len));
  sprintf(addrstr,format,(const char*)filename);
  return addrstr._retn();
}


OMNI_NAMESPACE_END(omni)
