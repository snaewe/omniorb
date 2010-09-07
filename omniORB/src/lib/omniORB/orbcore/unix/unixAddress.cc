// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixAddress.cc             Created on: 6 Aug 2001
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
  Revision 1.1.4.4  2006/06/22 13:53:49  dgrisby
  Add flags to strand.

  Revision 1.1.4.3  2006/04/28 18:40:46  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.2  2006/03/25 18:54:03  dgrisby
  Initial IPv6 support.

  Revision 1.1.4.1  2003/03/23 21:01:58  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.4  2002/01/15 16:38:14  dpg1
  On the road to autoconf. Dependencies refactored, configure.ac
  written. No makefiles yet.

  Revision 1.1.2.3  2001/08/23 10:11:15  sll
  Use AF_UNIX if AF_LOCAL is not defined.

  Revision 1.1.2.2  2001/08/07 15:42:17  sll
  Make unix domain connections distinguishable on both the server and client
  side.

  Revision 1.1.2.1  2001/08/06 15:47:44  sll
  Added support to use the unix domain socket as the local transport.


*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <unix/unixConnection.h>
#include <unix/unixAddress.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>
#include <sys/un.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixAddress);

#ifndef AF_LOCAL
#  ifdef  AF_UNIX
#    define AF_LOCAL AF_UNIX
#  endif
#endif

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
unixAddress::unixAddress(const char* filename) {

  pd_filename = (const char*) filename;
  pd_address_string = unixConnection::unToString(filename);
}

/////////////////////////////////////////////////////////////////////////
const char*
unixAddress::type() const {
  return "giop:unix";
}

/////////////////////////////////////////////////////////////////////////
const char*
unixAddress::address() const {
  return pd_address_string;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
unixAddress::duplicate() const {
  return new unixAddress(pd_filename);
}

/////////////////////////////////////////////////////////////////////////
giopActiveConnection*
unixAddress::Connect(unsigned long deadline_secs,
		     unsigned long deadline_nanosecs,
		     CORBA::ULong) const {

  struct sockaddr_un raddr;
  int  rc;
  SocketHandle_t sock;

  if ((sock = socket(AF_LOCAL,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return 0;
  }

  memset((void*)&raddr,0,sizeof(raddr));
  raddr.sun_family = AF_LOCAL;
  strncpy(raddr.sun_path, pd_filename, sizeof(raddr.sun_path) - 1);

#if !defined(USE_NONBLOCKING_CONNECT)

  if (::connect(sock,(struct sockaddr *)&raddr,
                     sizeof(raddr)) == RC_SOCKET_ERROR) {
    omniORB::logs(25, "Failed to connect to Unix socket.");
    CLOSESOCKET(sock);
    return 0;
  }

#else

  if (SocketSetnonblocking(sock) == RC_INVALID_SOCKET) {
    omniORB::logs(25, "Failed to set Unix socket to non-blocking mode.");
    CLOSESOCKET(sock);
    return 0;
  }

  if (::connect(sock,(struct sockaddr *)&raddr,
                     sizeof(raddr)) == RC_SOCKET_ERROR) {
    if (ERRNO != EINPROGRESS) {
      omniORB::logs(25, "Failed to connect to Unix socket.");
      CLOSESOCKET(sock);
      return 0;
    }
  }

  do {

    struct timeval t;

    if (deadline_secs || deadline_nanosecs) {
      SocketSetTimeOut(deadline_secs,deadline_nanosecs,t);
      if (t.tv_sec == 0 && t.tv_usec == 0) {
	// Already timed out.
	omniORB::logs(25, "Timed out connecting to Unix socket.");
	CLOSESOCKET(sock);
	return 0;
      }
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      if (t.tv_sec > orbParameters::scanGranularity) {
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

#if defined(USE_POLL)
    struct pollfd fds;
    fds.fd = sock;
    fds.events = POLLOUT;
    int timeout = t.tv_sec*1000+((t.tv_usec+999)/1000);
    if (timeout == 0) timeout = -1;
    int rc = poll(&fds,1,timeout);
    if (rc > 0 && fds.revents & POLLERR) {
      rc = RC_SOCKET_ERROR;
    }
#else
    fd_set fds, efds;
    FD_ZERO(&fds);
    FD_ZERO(&efds);
    FD_SET(sock,&fds);
    FD_SET(sock,&efds);
    struct timeval* tp = &t;
    if (t.tv_sec == 0 && t.tv_usec == 0) tp = 0;
    int rc = select(sock+1,0,&fds,&efds,tp);
#endif
    if (rc == 0) {
      // Time out!
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      continue;
#else
      omniORB::logs(25, "Timed out connecting to Unix socket.");
      CLOSESOCKET(sock);
      return 0;
#endif
    }
    if (rc != RC_SOCKET_ERROR) {
      // Check to make sure that the socket is connected.

      OMNI_SOCKADDR_STORAGE peer;
      SOCKNAME_SIZE_T len = sizeof(peer);
      rc = getpeername(sock, (struct sockaddr*)&peer, &len);
    }
    if (rc == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EINTR) {
	continue;
      }
      else {
	omniORB::logs(25, "Failed to connect to Unix socket (no peer name).");
	CLOSESOCKET(sock);
	return 0;
      }
    }
    break;

  } while (1);

  if (SocketSetblocking(sock) == RC_INVALID_SOCKET) {
    omniORB::logs(25, "Failed to set Unix socket to blocking mode.");
    CLOSESOCKET(sock);
    return 0;
  }
#endif

  return new unixActiveConnection(sock,pd_filename);
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixAddress::Poke() const {

  struct sockaddr_un raddr;
  int  rc;
  SocketHandle_t sock;

  if ((sock = socket(AF_LOCAL,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return 0;
  }

  memset((void*)&raddr,0,sizeof(raddr));
  raddr.sun_family = AF_LOCAL;
  strncpy(raddr.sun_path, pd_filename, sizeof(raddr.sun_path) - 1);

  if (::connect(sock,(struct sockaddr *)&raddr,
                     sizeof(raddr)) == RC_SOCKET_ERROR) {
    CLOSESOCKET(sock);
    return 0;
  }
  CLOSESOCKET(sock);
  return 1;
}


OMNI_NAMESPACE_END(omni)
