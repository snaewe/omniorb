// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpEndpoint.cc             Created on: 19 Mar 2001
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
  Revision 1.1.2.24  2004/10/17 22:27:23  dgrisby
  Handle errors in accept() properly. Thanks Kamaldeep Singh Khanuja and
  Jeremy Van Grinsven.

  Revision 1.1.2.23  2004/10/17 20:14:33  dgrisby
  Updated support for OpenVMS. Many thanks to Bruce Visscher.

  Revision 1.1.2.22  2003/11/12 16:04:17  dgrisby
  Set sockets to close on exec.

  Revision 1.1.2.21  2003/07/25 16:04:57  dgrisby
  vxWorks patches.

  Revision 1.1.2.20  2003/05/04 11:22:14  dgrisby
  Set the poked flag in the right place.

  Revision 1.1.2.19  2003/05/02 09:58:13  dgrisby
  Ensure shutdown happens even if the TCP stack has broken.

  Revision 1.1.2.18  2003/02/17 02:03:10  dgrisby
  vxWorks port. (Thanks Michael Sturm / Acterna Eningen GmbH).

  Revision 1.1.2.17  2003/01/06 11:11:55  dgrisby
  New AddrInfo instead of gethostbyname.

  Revision 1.1.2.16  2002/10/04 11:11:46  dgrisby
  Transport fixes: ENOTSOCK for Windows, SOMAXCONN in listen().

  Revision 1.1.2.15  2002/08/27 10:32:27  dgrisby
  FreeBSD fixes.

  Revision 1.1.2.14  2002/08/16 16:00:55  dgrisby
  Bugs accessing uninitialised String_vars with [].

  Revision 1.1.2.13  2002/05/07 12:54:44  dgrisby
  Fix inevitable Windows header brokenness.

  Revision 1.1.2.12  2002/05/07 00:46:26  dgrisby
  Different define for TCP protocol number.

  Revision 1.1.2.11  2002/05/07 00:28:32  dgrisby
  Turn off Nagle's algorithm. Fixes odd Linux loopback behaviour.

  Revision 1.1.2.10  2002/04/29 11:52:51  dgrisby
  More fixes for FreeBSD, Darwin, Windows.

  Revision 1.1.2.9  2002/03/19 15:42:04  dpg1
  Use list of IP addresses to pick a non-loopback interface if there is one.

  Revision 1.1.2.8  2002/03/13 16:05:40  dpg1
  Transport shutdown fixes. Reference count SocketCollections to avoid
  connections using them after they are deleted. Properly close
  connections when in thread pool mode.

  Revision 1.1.2.7  2002/03/11 12:24:39  dpg1
  Restrict bind to specified host, if any.

  Revision 1.1.2.6  2001/07/31 16:16:17  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.5  2001/07/13 15:34:24  sll
  Added the ability to monitor connections and callback to the giopServer when
  data has arrived at a connection.

  Revision 1.1.2.4  2001/06/20 18:35:15  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.3  2001/06/13 20:13:49  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.1.2.2  2001/05/01 16:04:42  sll
  Silly use of sizeof() on const char*. Should use strlen().

  Revision 1.1.2.1  2001/04/18 18:10:43  sll
  Big checkin with the brand new internal APIs.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <SocketCollection.h>
#include <tcp/tcpConnection.h>
#include <tcp/tcpAddress.h>
#include <tcp/tcpEndpoint.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(tcpEndpoint);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
tcpEndpoint::tcpEndpoint(const IIOP::Address& address) :
  pd_socket(RC_INVALID_SOCKET), pd_address(address),
  pd_new_conn_socket(RC_INVALID_SOCKET), pd_callback_func(0),
  pd_callback_cookie(0), pd_poked(0) {

  pd_address_string = (const char*) "giop:tcp:255.255.255.255:65535";
  // address string is not valid until bind is called.
}

/////////////////////////////////////////////////////////////////////////
tcpEndpoint::tcpEndpoint(const char* address) :
  pd_socket(RC_INVALID_SOCKET),
  pd_new_conn_socket(RC_INVALID_SOCKET), pd_callback_func(0),
  pd_callback_cookie(0) {

  pd_address_string = address;
  // OMNIORB_ASSERT(strncmp(address,"giop:tcp:",9) == 0);
  const char* host = strchr(address,':');
  host = strchr(host+1,':') + 1;
  const char* port = strchr(host,':') + 1;
  CORBA::ULong hostlen = port - host - 1;
  // OMNIORB_ASSERT(hostlen);
  pd_address.host = CORBA::string_alloc(hostlen);
  strncpy(pd_address.host,host,hostlen);
  ((char*)pd_address.host)[hostlen] = '\0';
  int rc;
  unsigned int v;
  rc = sscanf(port,"%u",&v);
  // OMNIORB_ASSERT(rc == 1);
  // OMNIORB_ASSERT(v > 0 && v < 65536);
  pd_address.port = v;
}

/////////////////////////////////////////////////////////////////////////
tcpEndpoint::~tcpEndpoint() {
  if (pd_socket != RC_INVALID_SOCKET) {
    CLOSESOCKET(pd_socket);
    pd_socket = RC_INVALID_SOCKET;
  }
}

/////////////////////////////////////////////////////////////////////////
const char*
tcpEndpoint::type() const {
  return "giop:tcp";
}

/////////////////////////////////////////////////////////////////////////
const char*
tcpEndpoint::address() const {
  return pd_address_string;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpEndpoint::Bind() {

  OMNIORB_ASSERT(pd_socket == RC_INVALID_SOCKET);

  if ((pd_socket = socket(INETSOCKET,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return 0;
  }

  {
    // Prevent Nagle's algorithm
    int valtrue = 1;
    if (setsockopt(pd_socket,IPPROTO_TCP,TCP_NODELAY,
		   (char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR) {
      CLOSESOCKET(pd_socket);
      pd_socket = RC_INVALID_SOCKET;
      return 0;
    }
  }

  SocketSetCloseOnExec(pd_socket);

  const char* host;
  if ((char*)pd_address.host && strlen(pd_address.host) != 0) {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Explicit bind to host " << pd_address.host << ".\n";
    }
    host = pd_address.host;
  }
  else {
    host = 0;
  }

  LibcWrapper::AddrInfo_var ai;
  ai = LibcWrapper::getAddrInfo(host, pd_address.port);

  if ((LibcWrapper::AddrInfo*)ai == 0) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Cannot get the address of host " << host << ".\n";
    }
    CLOSESOCKET(pd_socket);
    return 0;
  }
  if (pd_address.port) {
    int valtrue = 1;
    if (setsockopt(pd_socket,SOL_SOCKET,SO_REUSEADDR,
		   (char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR) {
      CLOSESOCKET(pd_socket);
      pd_socket = RC_INVALID_SOCKET;
      return 0;
    }
  }
  if (omniORB::trace(25)) {
    omniORB::logger l;
    CORBA::String_var addr(ai->asString());
    l << "Bind to address " << addr << ".\n";
  }
  if (::bind(pd_socket, ai->addr(), ai->addrSize()) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }

  if (listen(pd_socket,SOMAXCONN) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }

  // Now figure out the details to put in IORs

  struct sockaddr_in addr;
  SOCKNAME_SIZE_T l;
  l = sizeof(struct sockaddr_in);
  if (getsockname(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }
  pd_address.port = ntohs(addr.sin_port);

  if (!(char*)pd_address.host || strlen(pd_address.host) == 0) {

    // Try to find the first interface that isn't the loopback

    const omnivector<const char*>* ifaddrs
      = giopTransportImpl::getInterfaceAddress("giop:tcp");

    if (ifaddrs && !ifaddrs->empty()) {
      // TCP transport successfully gave us a list of interface addresses

      omnivector<const char*>::const_iterator i;
      for (i = ifaddrs->begin(); i != ifaddrs->end(); i++) {
	if (strcmp(*i, "127.0.0.1"))
	  break;
      }
      if (i == ifaddrs->end()) {
	// Only interface was the loopback -- we'll have to use that
	i = ifaddrs->begin();
      }
      pd_address.host = CORBA::string_dup(*i);
    }
    else {
      omniORB::logs(5, "No list of interface addresses; fall back to "
		    "system hostname.");
      char self[64];

      if (gethostname(&self[0],64) == RC_SOCKET_ERROR) {
	omniORB::logs(1, "Cannot get the name of this host.");
	CLOSESOCKET(pd_socket);
	return 0;
      }
      if (omniORB::trace(10)) {
	omniORB::logger l;
	l << "My hostname is " << self << ".\n";
      }
      LibcWrapper::AddrInfo_var ai;
      ai = LibcWrapper::getAddrInfo(self, pd_address.port);
      if ((LibcWrapper::AddrInfo*)ai == 0) {
	if (omniORB::trace(1)) {
	  omniORB::logger log;
	  log << "Cannot get the address of my hostname " << self << ".\n";
	}
	CLOSESOCKET(pd_socket);
	return 0;
      }
      pd_address.host = ai->asString();
    }
  }
  if (omniORB::trace(1) && strcmp(pd_address.host,"127.0.0.1") == 0) {
    omniORB::logger log;
    log << "Warning: the local loop back interface (127.0.0.1) is used as\n"
	<< "this server's address. Only clients on this machine can talk to\n"
	<< "this server.\n";
  }

  const char* format = "giop:tcp:%s:%d";
  pd_address_string = CORBA::string_alloc(strlen(pd_address.host) +
					  strlen(format)+6);
  sprintf((char*)pd_address_string,format,
	  (const char*)pd_address.host,(int)pd_address.port);

  return 1;
}

/////////////////////////////////////////////////////////////////////////
void
tcpEndpoint::Poke() {

  tcpAddress* target = new tcpAddress(pd_address);
  giopActiveConnection* conn;
  if ((conn = target->Connect()) == 0) {
    if (omniORB::trace(5)) {
      omniORB::logger log;
      log << "Warning: fail to connect to myself ("
	  << (const char*) pd_address_string << ") via tcp.\n";
    }
    pd_poked = 1;
  }
  else {
    delete conn;
  }
  delete target;
}

/////////////////////////////////////////////////////////////////////////
void
tcpEndpoint::Shutdown() {
  SHUTDOWNSOCKET(pd_socket);
  decrRefCount();
  omniORB::logs(20, "TCP endpoint shut down.");
}

/////////////////////////////////////////////////////////////////////////
giopConnection*
tcpEndpoint::AcceptAndMonitor(giopConnection::notifyReadable_t func,
			      void* cookie) {

  OMNIORB_ASSERT(pd_socket != RC_INVALID_SOCKET);

  pd_callback_func = func;
  pd_callback_cookie = cookie;
  setSelectable(pd_socket,1,0,0);

  while (1) {
    pd_new_conn_socket = RC_INVALID_SOCKET;
    if (!Select()) break;
    if (pd_new_conn_socket != RC_INVALID_SOCKET) {
      return  new tcpConnection(pd_new_conn_socket,this);
    }
    if (pd_poked)
      return 0;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpEndpoint::notifyReadable(SocketHandle_t fd) {

again:

  if (fd == pd_socket) {
    SocketHandle_t sock;
    sock = ::accept(pd_socket,0,0);
    if (sock == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EBADF) {
        omniORB::logs(20, "accept() returned EBADF, unable to continue");
        return 0;
      }
      else if (ERRNO == RC_EINTR ) {
        omniORB::logs(20, "accept() returned EINTR, trying again");
        goto again;
      }
#ifdef UnixArchitecture
      else if (ERRNO == RC_ECONNABORTED || ERRNO == RC_EPROTO ) {
        omniORB::logs(20, "accept() returned ECONNABORTED, ignoring accept");
        goto ignore;
      }
      else if (ERRNO == RC_EAGAIN ) {
        omniORB::logs(20, "accept() returned EAGAIN, trying again");
        goto again;
      }
#endif
      if (omniORB::trace(1)) {
        omniORB::logger log;
        log << "Error: accept failed with unknown error " << ERRNO << "\n";
      }
      return 0;
    }
#if defined(__vxWorks__)
    // vxWorks "forgets" socket options
    static const int valtrue = 1;
    if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
		  (char*)&valtrue, sizeof(valtrue)) == ERROR) {
      return 0;
    }
#endif
    pd_new_conn_socket = sock;

ignore:

    setSelectable(pd_socket,1,0,1);
    return 1;
  }
  SocketLink* conn = findSocket(fd,1);
  if (conn) {
    pd_callback_func(pd_callback_cookie,(tcpConnection*)conn);
  }
  return 1;
}

OMNI_NAMESPACE_END(omni)
