// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpConnection.cc           Created on: 19 Mar 2001
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
  Revision 1.1.4.8  2006/10/09 13:08:58  dgrisby
  Rename SOCKADDR_STORAGE define to OMNI_SOCKADDR_STORAGE, to avoid
  clash on Win32 2003 SDK.

  Revision 1.1.4.7  2006/03/25 18:54:03  dgrisby
  Initial IPv6 support.

  Revision 1.1.4.6  2005/09/01 14:52:12  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.5  2005/03/02 12:39:17  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.4  2005/03/02 12:10:48  dgrisby
  setSelectable / Peek fixes.

  Revision 1.1.4.3  2005/01/13 21:10:03  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.2  2005/01/06 23:10:55  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:01:58  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.11  2003/01/06 11:11:55  dgrisby
  New AddrInfo instead of gethostbyname.

  Revision 1.1.2.10  2001/12/03 13:39:55  dpg1
  Explicit socket shutdown flag for Windows.

  Revision 1.1.2.9  2001/11/26 10:51:04  dpg1
  Wrong endpoint address when getsockname() fails.

  Revision 1.1.2.8  2001/08/24 15:56:44  sll
  Fixed code which made the wrong assumption about the semantics of
  do { ...; continue; } while(0)

  Revision 1.1.2.7  2001/08/23 16:47:01  sll
  Fixed missing cleanup in the switch to use orbParameters to store all
   configuration parameters.

  Revision 1.1.2.6  2001/07/31 16:16:17  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.5  2001/07/13 15:32:50  sll
  Enter a mapping from a socket to a giopConnection in the endpoint's hash
  table.

  Revision 1.1.2.4  2001/06/29 16:26:01  dpg1
  Reinstate tracing messages for new connections and handling locate
  requests.

  Revision 1.1.2.3  2001/06/20 18:35:16  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.2  2001/06/13 20:13:49  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.1.2.1  2001/04/18 18:10:44  sll
  Big checkin with the brand new internal APIs.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <omniORB4/omniURI.h>
#include <orbParameters.h>
#include <SocketCollection.h>
#include <tcp/tcpConnection.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>

#if defined(NTArchitecture)
#  include <ws2tcpip.h>
#endif

#if defined(__vxWorks__)
#  include "selectLib.h"
#endif

OMNI_EXPORT_LINK_FORCE_SYMBOL(tcpConnection);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
char*
tcpConnection::addrToString(sockaddr* addr)
{
#if defined(HAVE_INET_NTOP)

  char dest[80];
  const char* addrstr;

  if (addr->sa_family == AF_INET) {
    sockaddr_in* addr_in = (sockaddr_in*)addr;
    addrstr = inet_ntop(AF_INET, &addr_in->sin_addr, dest, sizeof(dest));
  }
#if defined(OMNI_SUPPORT_IPV6)
  else {
    OMNIORB_ASSERT(addr->sa_family == AF_INET6);
    sockaddr_in6* addr_in6 = (sockaddr_in6*)addr;
    addrstr = inet_ntop(AF_INET6, &addr_in6->sin6_addr, dest, sizeof(dest));
  }
#endif
  OMNIORB_ASSERT(addrstr);

  return CORBA::string_dup(addrstr);

#elif defined (HAVE_GETNAMEINFO)

  char dest[80];
  socklen_t addrlen = 0;

  if (addr->sa_family == AF_INET) {
    addrlen = sizeof(sockaddr_in);
  }
#if defined(OMNI_SUPPORT_IPV6)
  else {
    OMNIORB_ASSERT(addr->sa_family == AF_INET6);
    addrlen = sizeof(sockaddr_in6);
  }
#endif
  OMNIORB_ASSERT(addrlen);

  int result = getnameinfo(addr, addrlen, dest, sizeof(dest), 0, 0,
			   NI_NUMERICHOST);
  if (result != 0) {
    omniORB::logs(1, "Unable to convert IP address to a string!");
    return CORBA::string_dup("**invalid**");
  }
  return CORBA::string_dup(dest);

#else

  OMNIORB_ASSERT(addr->sa_family == AF_INET);
  CORBA::ULong ipv4 = ((sockaddr_in*)addr)->sin_addr.s_addr;
  CORBA::ULong hipv4 = ntohl(ipv4);
  int ip1 = (int)((hipv4 & 0xff000000) >> 24);
  int ip2 = (int)((hipv4 & 0x00ff0000) >> 16);
  int ip3 = (int)((hipv4 & 0x0000ff00) >> 8);
  int ip4 = (int)((hipv4 & 0x000000ff));

  char* result = CORBA::string_alloc(16);
  sprintf(result,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
  return result;

#endif
}

/////////////////////////////////////////////////////////////////////////
char*
tcpConnection::addrToURI(sockaddr* addr, const char* prefix)
{
#if defined(HAVE_INET_NTOP)

  char dest[80];
  int port;
  const char* addrstr;

  if (addr->sa_family == AF_INET) {
    sockaddr_in* addr_in = (sockaddr_in*)addr;
    port = ntohs(addr_in->sin_port);
    addrstr = inet_ntop(AF_INET, &addr_in->sin_addr, dest, sizeof(dest));
  }
#if defined(OMNI_SUPPORT_IPV6)
  else {
    OMNIORB_ASSERT(addr->sa_family == AF_INET6);
    sockaddr_in6* addr_in6 = (sockaddr_in6*)addr;
    port = ntohs(addr_in6->sin6_port);
    addrstr = inet_ntop(AF_INET6, &addr_in6->sin6_addr, dest, sizeof(dest));
  }
#endif
  OMNIORB_ASSERT(addrstr);

  return omniURI::buildURI(prefix, addrstr, port);

#elif defined (HAVE_GETNAMEINFO)

  char dest[80];
  int port;
  socklen_t addrlen = 0;

  if (addr->sa_family == AF_INET) {
    addrlen = sizeof(sockaddr_in);
    sockaddr_in* addr_in = (sockaddr_in*)addr;
    port = ntohs(addr_in->sin_port);
  }
#if defined(OMNI_SUPPORT_IPV6)
  else {
    OMNIORB_ASSERT(addr->sa_family == AF_INET6);
    addrlen = sizeof(sockaddr_in6);
    sockaddr_in6* addr_in6 = (sockaddr_in6*)addr;
    port = ntohs(addr_in6->sin6_port);
  }
#endif
  OMNIORB_ASSERT(addrlen);

  int result = getnameinfo(addr, addrlen, dest, sizeof(dest), 0, 0,
			   NI_NUMERICHOST);
  if (result != 0) {
    omniORB::logs(1, "Unable to convert IP address to a string!");
    return CORBA::string_dup("**invalid**");
  }
  return omniURI::buildURI(prefix, dest, port);

#else

  OMNIORB_ASSERT(addr->sa_family == AF_INET);
  sockaddr_in* addr_in = (sockaddr_in*)addr;

  CORBA::ULong ipv4 = addr_in->sin_addr.s_addr;
  CORBA::ULong hipv4 = ntohl(ipv4);
  int ip1 = (int)((hipv4 & 0xff000000) >> 24);
  int ip2 = (int)((hipv4 & 0x00ff0000) >> 16);
  int ip3 = (int)((hipv4 & 0x0000ff00) >> 8);
  int ip4 = (int)((hipv4 & 0x000000ff));
  int port = ntohs(addr_in->sin_port);

  char* result = CORBA::string_alloc(strlen(prefix) + 24);
  sprintf(result,"%s%d.%d.%d.%d:%d",prefix, ip1, ip2, ip3, ip4, port);
  return result;

#endif
}

/////////////////////////////////////////////////////////////////////////
CORBA::UShort
tcpConnection::addrToPort(sockaddr* addr)
{
  if (addr->sa_family == AF_INET) {
    sockaddr_in* addr_in = (sockaddr_in*)addr;
    return ntohs(addr_in->sin_port);
  }
#if defined(OMNI_SUPPORT_IPV6)
  else {
    OMNIORB_ASSERT(addr->sa_family == AF_INET6);
    sockaddr_in6* addr_in6 = (sockaddr_in6*)addr;
    return ntohs(addr_in6->sin6_port);
  }
#else
  OMNIORB_ASSERT(0);
  return 0;
#endif
}

/////////////////////////////////////////////////////////////////////////
int
tcpConnection::Send(void* buf, size_t sz,
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
tcpConnection::Recv(void* buf, size_t sz,
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
tcpConnection::Shutdown() {
  SHUTDOWNSOCKET(pd_socket);
  pd_shutdown = 1;
}

/////////////////////////////////////////////////////////////////////////
const char*
tcpConnection::myaddress() {
  return (const char*)pd_myaddress;
}

/////////////////////////////////////////////////////////////////////////
const char*
tcpConnection::peeraddress() {
  return (const char*)pd_peeraddress;
}

/////////////////////////////////////////////////////////////////////////
tcpConnection::tcpConnection(SocketHandle_t sock,
			     SocketCollection* belong_to) :
  SocketHolder(sock) {

  OMNI_SOCKADDR_STORAGE addr;
  SOCKNAME_SIZE_T l;

  l = sizeof(OMNI_SOCKADDR_STORAGE);
  if (getsockname(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    pd_myaddress = (const char*)"giop:tcp:255.255.255.255:65535";
  }
  else {
    pd_myaddress = addrToURI((sockaddr*)&addr, "giop:tcp:");
  }

  l = sizeof(OMNI_SOCKADDR_STORAGE);
  if (getpeername(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    pd_peeraddress = (const char*)"giop:tcp:255.255.255.255:65535";
  }
  else {
    pd_peeraddress = addrToURI((sockaddr*)&addr, "giop:tcp:");
  }

  SocketSetCloseOnExec(sock);

  belong_to->addSocket(this);
}

/////////////////////////////////////////////////////////////////////////
tcpConnection::~tcpConnection() {
  clearSelectable();
  pd_belong_to->removeSocket(this);
  CLOSESOCKET(pd_socket);
}

/////////////////////////////////////////////////////////////////////////
void
tcpConnection::setSelectable(int now,
			     CORBA::Boolean data_in_buffer) {

  SocketHolder::setSelectable(now, data_in_buffer);
}

/////////////////////////////////////////////////////////////////////////
void
tcpConnection::clearSelectable() {

  SocketHolder::clearSelectable();
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpConnection::isSelectable() {
  return pd_belong_to->isSelectable(pd_socket);
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpConnection::Peek() {
  return SocketHolder::Peek();
}


OMNI_NAMESPACE_END(omni)
