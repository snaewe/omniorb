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
#include <omniORB4/sslContext.h>
#include <SocketCollection.h>
#include <ssl/sslConnection.h>
#include <ssl/sslAddress.h>
#include <ssl/sslEndpoint.h>
#include <tcp/tcpConnection.h>
#include <openssl/err.h>
#include <netinet/tcp.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(sslEndpoint);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
sslEndpoint::sslEndpoint(const IIOP::Address& address, sslContext* ctx) : 
  pd_socket(RC_INVALID_SOCKET), pd_address(address), pd_ctx(ctx),
  pd_new_conn_socket(RC_INVALID_SOCKET), pd_callback_func(0),
  pd_callback_cookie(0) {

  pd_address_string = (const char*) "giop:ssl:255.255.255.255:65535";
  // address string is not valid until bind is called.
}

/////////////////////////////////////////////////////////////////////////
sslEndpoint::sslEndpoint(const char* address, sslContext* ctx) : 
  pd_socket(RC_INVALID_SOCKET), pd_ctx(ctx),
  pd_new_conn_socket(RC_INVALID_SOCKET), pd_callback_func(0),
  pd_callback_cookie(0) {

  pd_address_string = address;
  // OMNIORB_ASSERT(strncmp(address,"giop:ssl:",9) == 0);
  const char* host = strchr(address,':');
  host = strchr(host+1,':') + 1;
  const char* port = strchr(host,':') + 1;
  CORBA::ULong hostlen = port - host - 1;
  // OMNIORB_ASSERT(hostlen);
  pd_address.host = CORBA::string_alloc(hostlen);
  strncpy(pd_address.host,host,hostlen);
  pd_address.host[hostlen] = '\0';
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

  struct sockaddr_in addr;
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

  addr.sin_family = INETSOCKET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(pd_address.port);

  if ((char*)pd_address.host && strlen(pd_address.host) != 0) {
    LibcWrapper::hostent_var h;
    int rc;

    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Explicit bind to host " << pd_address.host << "\n";
    }

    if (LibcWrapper::gethostbyname(pd_address.host,h,rc) < 0) {
      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Cannot get the address of host " << pd_address.host << "\n";
      }
      CLOSESOCKET(pd_socket);
      return 0;
    }
    memcpy((void *)&addr.sin_addr,
	   (void *)h.hostent()->h_addr_list[0],
	   sizeof(addr.sin_addr));
  }
  
  if (addr.sin_port) {
    int valtrue = 1;
    if (setsockopt(pd_socket,SOL_SOCKET,SO_REUSEADDR,
		   (char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR) {
      CLOSESOCKET(pd_socket);
      pd_socket = RC_INVALID_SOCKET;
      return 0;
    }
  }

  if (::bind(pd_socket,(struct sockaddr *)&addr,
	     sizeof(struct sockaddr_in)) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }

  if (listen(pd_socket,5) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }

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

    const char* selected_hostname;
    char self[64];

    const omnivector<const char*>* ifaddrs
      = giopTransportImpl::getInterfaceAddress("giop:ssl");

    if (ifaddrs && !ifaddrs->empty()) {
      // SSL transport successfully gave us a list of interface addresses

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
      selected_hostname = pd_address.host;
    }
    else {
      omniORB::logs(5, "No list of interface addresses; fall back to "
		    "system hostname.");

      if (gethostname(&self[0],64) == RC_SOCKET_ERROR) {
	if (omniORB::trace(1)) {
	  omniORB::logger log;
	  log << "Cannot get the name of this host\n";
	}
	CLOSESOCKET(pd_socket);
	return 0;
      }
      selected_hostname = self;
    }

    LibcWrapper::hostent_var h;
    int rc;
    if (LibcWrapper::gethostbyname(selected_hostname,h,rc) < 0) {
      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Cannot get the address of host " << pd_address.host << "\n";
      }
      CLOSESOCKET(pd_socket);
      return 0;
    }
    memcpy((void *)&addr.sin_addr,
	   (void *)h.hostent()->h_addr_list[0],
	   sizeof(addr.sin_addr));
  }
  if (!(char*)pd_address.host || strlen(pd_address.host) == 0) {
    pd_address.host = tcpConnection::ip4ToString(addr.sin_addr.s_addr);
  }
  if (omniORB::trace(1) && strcmp(pd_address.host,"127.0.0.1") == 0) {
    omniORB::logger log;
    log << "Warning: the local loop back interface (127.0.0.1) is used as\n"
	<< "this server's address. Only clients on this machine can talk to\n"
	<< "this server.\n";
  }

  const char* format = "giop:ssl:%s:%d";
  pd_address_string = CORBA::string_alloc(strlen(pd_address.host) +
					  strlen(format)+6);
  sprintf((char*)pd_address_string,format,
	  (const char*)pd_address.host,(int)pd_address.port);

  return 1;
}

/////////////////////////////////////////////////////////////////////////
void
sslEndpoint::Poke() {

  sslAddress* target = new sslAddress(pd_address,pd_ctx);
  giopActiveConnection* conn;
  if ((conn = target->Connect()) == 0) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Warning: Fail to connect to myself (" 
	  << (const char*) pd_address_string << ") via ssl!\n";
      log << "Warning: This is ignored but this may cause the ORB shutdown to hang.\n";
    }
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
  setSelectable(pd_socket,1,0,0);

  while (1) {
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
	      log << "openSSL error detected in sslEndpoint::accept. Reason: "
		  << (const char*) buf << "\n";
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
sslEndpoint::notifyReadable(SocketHandle_t fd) {

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
    pd_callback_func(pd_callback_cookie,(sslConnection*)conn);
  }
  return 1;
}

OMNI_NAMESPACE_END(omni)
