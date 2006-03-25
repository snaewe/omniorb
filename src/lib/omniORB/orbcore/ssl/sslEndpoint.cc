// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslEndpoint.cc             Created on: 29 May 2001
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
  Revision 1.1.4.6  2006/03/25 18:54:03  dgrisby
  Initial IPv6 support.

  Revision 1.1.4.5  2005/08/03 09:43:51  dgrisby
  Make sure accept() never blocks.

  Revision 1.1.4.4  2005/04/11 12:09:40  dgrisby
  Another merge.

  Revision 1.1.4.3  2005/01/13 21:10:01  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.2  2005/01/06 23:10:53  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:01:59  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.19  2003/02/17 02:03:10  dgrisby
  vxWorks port. (Thanks Michael Sturm / Acterna Eningen GmbH).

  Revision 1.1.2.18  2003/01/06 11:11:55  dgrisby
  New AddrInfo instead of gethostbyname.

  Revision 1.1.2.17  2002/10/04 11:11:45  dgrisby
  Transport fixes: ENOTSOCK for Windows, SOMAXCONN in listen().

  Revision 1.1.2.16  2002/09/09 22:11:51  dgrisby
  SSL transport cleanup even if certificates are wrong.

  Revision 1.1.2.15  2002/08/16 16:00:53  dgrisby
  Bugs accessing uninitialised String_vars with [].

  Revision 1.1.2.14  2002/05/07 12:54:43  dgrisby
  Fix inevitable Windows header brokenness.

  Revision 1.1.2.13  2002/05/07 00:46:26  dgrisby
  Different define for TCP protocol number.

  Revision 1.1.2.12  2002/05/07 00:28:32  dgrisby
  Turn off Nagle's algorithm. Fixes odd Linux loopback behaviour.

  Revision 1.1.2.11  2002/04/29 11:52:51  dgrisby
  More fixes for FreeBSD, Darwin, Windows.

  Revision 1.1.2.10  2002/04/16 12:44:27  dpg1
  Fix SSL accept bug, clean up logging.

  Revision 1.1.2.9  2002/03/19 15:42:04  dpg1
  Use list of IP addresses to pick a non-loopback interface if there is one.

  Revision 1.1.2.8  2002/03/13 16:05:40  dpg1
  Transport shutdown fixes. Reference count SocketCollections to avoid
  connections using them after they are deleted. Properly close
  connections when in thread pool mode.

  Revision 1.1.2.7  2002/03/11 12:24:39  dpg1
  Restrict bind to specified host, if any.

  Revision 1.1.2.6  2001/07/31 16:16:22  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.5  2001/07/26 16:37:21  dpg1
  Make sure static initialisers always run.

  Revision 1.1.2.4  2001/07/13 15:36:53  sll
  Added the ability to monitor connections and callback to the giopServer
  when data has arrived at a connection.

  Revision 1.1.2.3  2001/06/20 18:35:16  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.2  2001/06/18 20:27:56  sll
  Use strchr instead of index() for maximal portability.

  Revision 1.1.2.1  2001/06/11 18:11:06  sll
  *** empty log message ***

*/

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/sslContext.h>
#include <SocketCollection.h>
#include <libcWrapper.h>
#include <ssl/sslConnection.h>
#include <ssl/sslAddress.h>
#include <ssl/sslEndpoint.h>
#include <tcp/tcpConnection.h>
#include <openssl/err.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(sslEndpoint);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
sslEndpoint::sslEndpoint(const IIOP::Address& address, sslContext* ctx) : 
  SocketHolder(RC_INVALID_SOCKET), pd_address(address), pd_ctx(ctx),
  pd_new_conn_socket(RC_INVALID_SOCKET), pd_callback_func(0),
  pd_callback_cookie(0), pd_go(1) {

  pd_address_string = (const char*) "giop:ssl:255.255.255.255:65535";
  // address string is not valid until bind is called.
}

/////////////////////////////////////////////////////////////////////////
sslEndpoint::sslEndpoint(const char* address, sslContext* ctx) : 
  SocketHolder(RC_INVALID_SOCKET), pd_ctx(ctx),
  pd_new_conn_socket(RC_INVALID_SOCKET), pd_callback_func(0),
  pd_callback_cookie(0), pd_go(1) {

  pd_address_string = address;
  // OMNIORB_ASSERT(strncmp(address,"giop:ssl:",9) == 0);
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
sslEndpoint::~sslEndpoint() {
  if (pd_socket != RC_INVALID_SOCKET) {
    CLOSESOCKET(pd_socket);
    pd_socket = RC_INVALID_SOCKET;
  }
}

/////////////////////////////////////////////////////////////////////////
const char*
sslEndpoint::type() const {
  return "giop:ssl";
}

/////////////////////////////////////////////////////////////////////////
const char*
sslEndpoint::address() const {
  return pd_address_string;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
sslEndpoint::Bind() {

  OMNIORB_ASSERT(pd_socket == RC_INVALID_SOCKET);

  const char* host;
  int passive_host; // 0 for explicit, 1 for unspecified passive, 2
		    // for IPv4 passive, 3 for IPv6 passive.

  if ((char*)pd_address.host && strlen(pd_address.host) != 0) {
    host = pd_address.host;

    if (omni::strMatch(pd_address.host, "0.0.0.0")) {
      passive_host = 2;
    }
#if defined(OMNI_SUPPORT_IPV6)
    else if (omni::strMatch(pd_address.host, "::")) {
      passive_host = 3;
    }
#endif
    else {
      if (omniORB::trace(25)) {
	omniORB::logger log;
	log << "Explicit bind to host " << pd_address.host << ".\n";
      }
      passive_host = 0;
    }
  }
  else {
    host = 0;
#ifdef OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS
    passive_host = 1;
#else
    passive_host = 2;
#endif
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

  if ((pd_socket = socket(ai->addrFamily(),
			  SOCK_STREAM, 0)) == RC_INVALID_SOCKET) {
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
    l << "Bind to address " << addr << "\n";
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

  SOCKADDR_STORAGE addr;
  SOCKNAME_SIZE_T l;
  l = sizeof(SOCKADDR_STORAGE);
  if (getsockname(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }
  pd_address.port = tcpConnection::addrToPort((struct sockaddr*)&addr);

  if (passive_host) {

    // Ask the TCP transport for its list of interface addresses

    const omnivector<const char*>* ifaddrs
      = giopTransportImpl::getInterfaceAddress("giop:tcp");

    if (ifaddrs && !ifaddrs->empty()) {
      // TCP transport successfully gave us a list of interface addresses

      // Use the first non-loopback IP address.
      const char* loopback = 0;

      omnivector<const char*>::const_iterator i;
      for (i = ifaddrs->begin(); i != ifaddrs->end(); i++) {

	if (passive_host == 2 && !LibcWrapper::isip4addr(*i))
	  continue;

	if (passive_host == 3 && !LibcWrapper::isip6addr(*i))
	  continue;

	if (omni::strMatch(*i, "127.0.0.1") || omni::strMatch(*i, "::1")) {
	  loopback = *i;
	  continue;
	}
	break;
      }
      if (i == ifaddrs->end()) {
	// Did not find a matching address. Fall back to the loopback.
	if (loopback) {
	  pd_address.host = CORBA::string_dup(loopback);
	}
	else {
	  omniORB::logs(1, "No suitable address in the list of "
			"interface addresses.");
	  CLOSESOCKET(pd_socket);
	  return 0;
	}
      }
      else {
	pd_address.host = CORBA::string_dup(*i);
      }
    }
    else {
      omniORB::logs(5, "No list of interface addresses; fall back to "
		    "system hostname.");
      char self[OMNIORB_HOSTNAME_MAX];

      if (gethostname(&self[0],OMNIORB_HOSTNAME_MAX) == RC_SOCKET_ERROR) {
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
  if (passive_host && omniORB::trace(1) &&
      (omni::strMatch(pd_address.host, "127.0.0.1") ||
       omni::strMatch(pd_address.host, "::1") ||
       omni::strMatch(pd_address.host, "localhost") ||
       omni::strMatch(pd_address.host, "localhost.localdomain"))) {

    omniORB::logger log;
    log << "Warning: the local loop back interface (" << pd_address.host
	<< ") is used as\n"
	<< "this server's address. Only clients on this machine can talk to\n"
	<< "this server.\n";
  }

  pd_address_string = omniURI::buildURI("giop:ssl:",
					pd_address.host,
					pd_address.port);

  // Never block in accept
  SocketSetnonblocking(pd_socket);

  // Add the socket to our SocketCollection.
  addSocket(this);

  return 1;
}

/////////////////////////////////////////////////////////////////////////
void
sslEndpoint::Poke() {

  sslAddress* target = new sslAddress(pd_address,pd_ctx);
  giopActiveConnection* conn;

  unsigned long timeout_sec, timeout_nsec;
  omni_thread::get_time(&timeout_sec, &timeout_nsec, 1);

  if ((conn = target->Connect(timeout_sec, timeout_nsec)) == 0) {
    if (omniORB::trace(5)) {
      omniORB::logger log;
      log << "Warning: Fail to connect to myself (" 
	  << (const char*) pd_address_string << ") via ssl!\n";
    }
    pd_go = 0;
    // No concurrency control on pd_go, but it should be safe to set
    // it to zero here. The worst that can happen is AcceptAndMonitor
    // goes one extra time around its loop before spotting the change.
  }
  else {
    delete conn;
  }
  delete target;
}

/////////////////////////////////////////////////////////////////////////
void
sslEndpoint::Shutdown() {
  SHUTDOWNSOCKET(pd_socket);
  removeSocket(this);
  decrRefCount();
  omniORB::logs(20, "SSL endpoint shut down.");
}

/////////////////////////////////////////////////////////////////////////
giopConnection*
sslEndpoint::AcceptAndMonitor(giopConnection::notifyReadable_t func,
			      void* cookie) {

  OMNIORB_ASSERT(pd_socket != RC_INVALID_SOCKET);

  pd_callback_func = func;
  pd_callback_cookie = cookie;
  setSelectable(1,0,0);

  while (pd_go) {
    pd_new_conn_socket = RC_INVALID_SOCKET;
    if (!Select()) break;
    if (pd_new_conn_socket != RC_INVALID_SOCKET) {

      ::SSL* ssl = SSL_new(pd_ctx->get_SSL_CTX());
      SSL_set_fd(ssl, pd_new_conn_socket);
      SSL_set_accept_state(ssl);

      int go = 1;
      while(go) {
	int result = SSL_accept(ssl);
	int code = SSL_get_error(ssl, result);

	switch(code) {
	case SSL_ERROR_NONE:
	  return new sslConnection(pd_new_conn_socket,ssl,this);

	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
	  continue;

	case SSL_ERROR_SYSCALL:
	  {
	    if (ERRNO == RC_EINTR)
	      continue;
	  }
	  // otherwise falls through
	case SSL_ERROR_SSL:
	  {
	    if (omniORB::trace(10)) {
	      omniORB::logger log;
	      char buf[128];
	      ERR_error_string_n(ERR_get_error(),buf,128);
	      log << "openSSL error detected in sslEndpoint::accept.\n"
		  << "Reason: " << (const char*) buf << "\n";
	    }
	    SSL_free(ssl);
	    CLOSESOCKET(pd_new_conn_socket);
	    go = 0;
	  }
	}
      }
    }
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
sslEndpoint::notifyReadable(SocketHolder* sh) {

  if (sh == (SocketHolder*)this) {
    // New connection
    SocketHandle_t sock;
again:
    sock = ::accept(pd_socket,0,0);
    if (sock == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EBADF) {
        omniORB::logs(20, "accept() returned EBADF, unable to continue");
        return 0;
      }
      else if (ERRNO == RC_EINTR) {
        omniORB::logs(20, "accept() returned EINTR, trying again");
        goto again;
      }
#ifdef UnixArchitecture
      else if (ERRNO == RC_EAGAIN) {
        omniORB::logs(20, "accept() returned EAGAIN, will try later");
      }
#endif
      if (omniORB::trace(20)) {
        omniORB::logger log;
        log << "accept() failed with unknown error " << ERRNO << "\n";
      }
    }
    else {
#if defined(__vxWorks__)
      // vxWorks "forgets" socket options
      static const int valtrue = 1;
      if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
		    (char*)&valtrue, sizeof(valtrue)) == ERROR) {
	return 0;
      }
#endif
      // On some platforms, the new socket inherits the non-blocking
      // setting from the listening socket, so we set it blocking here
      // just to be sure.
      SocketSetblocking(sock);

      pd_new_conn_socket = sock;
    }
    setSelectable(1,0,1);
    return 1;
  }
  else {
    // Existing connection
    pd_callback_func(pd_callback_cookie,(sslConnection*)sh);
    return 1;
  }
}

OMNI_NAMESPACE_END(omni)
