// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslEndpoint.cc             Created on: 29 May 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001      AT&T Laboratories Cambridge
//    Copyright (C) 2002-2008 Apasphere Ltd
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
  Revision 1.1.4.20  2009/02/17 14:32:18  dgrisby
  Some platforms return the IPv4 wildcard address before the IPv6 one
  when given PF_UNSPEC. Make sure the IPv6 one is used if available.

  Revision 1.1.4.19  2008/04/19 18:25:33  dgrisby
  HPUX returns an invalid protocol from getaddrinfo() with PF_UNSPEC.

  Revision 1.1.4.18  2007/10/29 12:41:46  dgrisby
  Handle SSL_ERROR_ZERO_RETURN. Thanks Jan Lennartsson.

  Revision 1.1.4.17  2007/07/31 14:23:43  dgrisby
  If the platform does not accept IPv4 connections on IPv6 sockets by
  default, try to enable it by turning the IPV6_V6ONLY socket option
  off. Should work for BSDs and Windows Vista.

  Revision 1.1.4.16  2007/03/28 16:29:04  dgrisby
  Always wake up SocketCollection in Poke in case connect seems to work
  but does not actually wake the thread.

  Revision 1.1.4.15  2007/02/28 15:55:57  dgrisby
  setsockopt expects a char* on some platforms.

  Revision 1.1.4.14  2007/02/26 15:16:31  dgrisby
  New socketSendBuffer parameter, defaulting to 16384 on Windows.
  Avoids a bug in Windows where select() on send waits for all sent data
  to be acknowledged.

  Revision 1.1.4.13  2006/11/20 14:16:21  dgrisby
  FreeBSD doesn't listen for IPv4 on IPv6 sockets. Thanks Arno Klaassen.

  Revision 1.1.4.12  2006/10/09 13:08:58  dgrisby
  Rename SOCKADDR_STORAGE define to OMNI_SOCKADDR_STORAGE, to avoid
  clash on Win32 2003 SDK.

  Revision 1.1.4.11  2006/07/03 11:18:57  dgrisby
  If Poke() fails to connect to itself, wake up the SocketCollection in
  case it is idle.

  Revision 1.1.4.10  2006/04/28 18:40:46  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.9  2006/04/24 14:26:43  dgrisby
  Include both IPv4 and IPv6 loopbacks in address lists.

  Revision 1.1.4.8  2006/04/10 12:50:35  dgrisby
  More endPointPublish; support for deprecated endPointNoListen,
  endPointPublishAllIFs.

  Revision 1.1.4.7  2006/04/09 19:52:31  dgrisby
  More IPv6, endPointPublish parameter.

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
#include <objectAdapter.h>
#include <orbParameters.h>
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
  return pd_addresses[0];
}

/////////////////////////////////////////////////////////////////////////
const _CORBA_Unbounded_Sequence_String*
sslEndpoint::addresses() const {
  return &pd_addresses;
}

/////////////////////////////////////////////////////////////////////////
static CORBA::Boolean
publish_one(const char*    	     publish_spec,
	    const char*    	     ep,
	    CORBA::Boolean 	     no_publish,
	    orbServer::EndpointList& published_eps)
{
  OMNIORB_ASSERT(!strncmp(ep, "giop:ssl:", 9));

  CORBA::String_var to_add;
  CORBA::String_var ep_host;
  CORBA::UShort     ep_port;

  ep_host = omniURI::extractHostPort(ep+9, ep_port);

  if (!strncmp(publish_spec, "giop:ssl:", 9)) {
    CORBA::UShort port;
    CORBA::String_var host = omniURI::extractHostPort(publish_spec+9, port, 0);
    if (!(char*)host) {
      if (omniORB::trace(1)) {
	omniORB::logger l;
	l << "Invalid endpoint '" << publish_spec
	  << "' in publish specification.";
      }
      OMNIORB_THROW(INITIALIZE,
		    INITIALIZE_EndpointPublishFailure,
		    CORBA::COMPLETED_NO);
    }
    if (strlen(host) == 0)
      host = ep_host;

    if (!port)
      port = ep_port;

    to_add = omniURI::buildURI("giop:ssl:", host, port);
  }
  else if (no_publish) {
    // Suppress all the other options
    return 0;
  }
  else if (omni::strMatch(publish_spec, "addr")) {
    to_add = ep;
  }
  else if (omni::strMatch(publish_spec, "ipv6")) {
    if (!LibcWrapper::isip6addr(ep_host))
      return 0;
    to_add = ep;
  }
  else if (omni::strMatch(publish_spec, "ipv4")) {
    if (!LibcWrapper::isip4addr(ep_host))
      return 0;
    to_add = ep;
  }
  else if (omni::strMatch(publish_spec, "name")) {
    LibcWrapper::AddrInfo_var ai = LibcWrapper::getAddrInfo(ep_host, 0);
    if (!ai.in())
      return 0;

    CORBA::String_var name = ai->name();
    if (!(char*)name)
      return 0;

    to_add = omniURI::buildURI("giop:ssl:", name, ep_port);
  }
  else if (omni::strMatch(publish_spec, "hostname")) {
    char self[OMNIORB_HOSTNAME_MAX];

    if (gethostname(&self[0],OMNIORB_HOSTNAME_MAX) == RC_SOCKET_ERROR)
      return 0;

    to_add = omniURI::buildURI("giop:ssl:", self, ep_port);
  }
  else if (omni::strMatch(publish_spec, "fqdn")) {
    char self[OMNIORB_HOSTNAME_MAX];

    if (gethostname(&self[0],OMNIORB_HOSTNAME_MAX) == RC_SOCKET_ERROR)
      return 0;

    LibcWrapper::AddrInfo_var ai = LibcWrapper::getAddrInfo(self, 0);
    if (!ai.in())
      return 0;

    char* name = ai->name();
    if (name && !(omni::strMatch(name, "localhost") ||
		  omni::strMatch(name, "localhost.localdomain"))) {
      to_add = omniURI::buildURI("giop:ssl:", name, ep_port);
    }
    else {
      to_add = omniURI::buildURI("giop:ssl:", self, ep_port);
    }
  }
  else {
    // Don't understand the spec.
    return 0;
  }

  if (!omniObjAdapter::endpointInList(to_add, published_eps)) {
    if (omniORB::trace(20)) {
      omniORB::logger l;
      l << "Publish endpoint '" << to_add << "'\n";
    }
    giopEndpoint::addToIOR(to_add);
    published_eps.length(published_eps.length() + 1);
    published_eps[published_eps.length() - 1] = to_add._retn();
  }
  return 1;
}

CORBA::Boolean
sslEndpoint::publish(const orbServer::PublishSpecs& publish_specs,
		     CORBA::Boolean 	      	    all_specs,
		     CORBA::Boolean 	      	    all_eps,
		     orbServer::EndpointList& 	    published_eps)
{
  CORBA::ULong i, j;
  CORBA::Boolean result = 0;

  if (publish_specs.length() == 1 &&
      omni::strMatch(publish_specs[0], "fail-if-multiple") &&
      pd_addresses.length() > 1) {

    omniORB::logs(1, "SSL endpoint has multiple addresses. "
		  "You must choose one to listen on.");
    OMNIORB_THROW(INITIALIZE, INITIALIZE_TransportError,
		  CORBA::COMPLETED_NO);
  }
  for (i=0; i < pd_addresses.length(); ++i) {

    CORBA::Boolean ok = 0;
    
    for (j=0; j < publish_specs.length(); ++j) {
      if (omniORB::trace(25)) {
	omniORB::logger l;
	l << "Try to publish '" << publish_specs[j]
	  << "' for endpoint " << pd_addresses[i] << "\n";
      }
      ok = publish_one(publish_specs[j], pd_addresses[i], no_publish(),
		       published_eps);
      result |= ok;

      if (ok && !all_specs)
	break;
    }
    if (result && !all_eps)
      break;
  }
  return result;
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
#if defined(OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS) || defined(IPV6_V6ONLY)
    host = 0;
    passive_host = 1;
#else
    host = "0.0.0.0";
    passive_host = 2;
#endif
  }

  LibcWrapper::AddrInfo_var aiv;
  LibcWrapper::AddrInfo*    ai;

  do {
    aiv = LibcWrapper::getAddrInfo(host, pd_address.port);
    ai  = aiv;

#if defined(OMNI_SUPPORT_IPV6)
    if (passive_host == 1) {
      // Search through addresses to find an IPv6 one
      LibcWrapper::AddrInfo* aip = ai;
      while (aip) {
	if (aip->addrFamily() == PF_INET6) {
	  ai = aip;
	  break;
	}
	aip = aip->next();
      }
    }
#endif

    if (ai == 0) {
      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Cannot get the address of host " << host << ".\n";
      }
      CLOSESOCKET(pd_socket);
      return 0;
    }

    pd_socket = socket(ai->addrFamily(), SOCK_STREAM, 0);

    if (pd_socket == RC_INVALID_SOCKET) {

      if (passive_host == 1) {
	omniORB::logs(2, "Unable to open socket for unspecified passive host. "
		      "Fall back to IPv4.");
	host = "0.0.0.0";
	passive_host = 2;
	continue;
      }
      else {
	omniORB::logs(1, "Unable to open required socket.");
	return 0;
      }
    }

#if defined(OMNI_SUPPORT_IPV6) && defined(IPV6_V6ONLY)
#  if !defined(OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS)
    if (passive_host == 1) {
      // Attempt to turn IPV6_V6ONLY option off
      int valfalse = 0;
      omniORB::logs(10, "Attempt to set socket to listen on IPv4 and IPv6.");

      if (setsockopt(pd_socket, IPPROTO_IPV6, IPV6_V6ONLY,
		     (char*)&valfalse, sizeof(valfalse)) == RC_SOCKET_ERROR) {
	omniORB::logs(2, "Unable to set socket to listen on IPv4 and IPv6. "
		      "Fall back to just IPv4.");
	CLOSESOCKET(pd_socket);
	pd_socket = RC_INVALID_SOCKET;
	host = "0.0.0.0";
	passive_host = 2;
      }
    }
#  endif
#endif
  } while (pd_socket == RC_INVALID_SOCKET);

  {
    // Prevent Nagle's algorithm
    int valtrue = 1;
    if (setsockopt(pd_socket,IPPROTO_TCP,TCP_NODELAY,
		   (char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR) {

      omniORB::logs(2, "Warning: failed to set TCP_NODELAY option.");
    }
  }

  if (orbParameters::socketSendBuffer != -1) {
    // Set the send buffer size
    int bufsize = orbParameters::socketSendBuffer;
    if (setsockopt(pd_socket, SOL_SOCKET, SO_SNDBUF,
		   (char*)&bufsize, sizeof(bufsize)) == RC_SOCKET_ERROR) {
      CLOSESOCKET(pd_socket);
      pd_socket = RC_INVALID_SOCKET;
      omniORB::logs(1, "Failed to set SO_SNDBUF option.");
      return 0;
    }
  }

  SocketSetCloseOnExec(pd_socket);

  if (pd_address.port) {
    int valtrue = 1;
    if (setsockopt(pd_socket,SOL_SOCKET,SO_REUSEADDR,
		   (char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR) {

      omniORB::logs(2, "Warning: failed to set SO_REUSEADDR option.");
    }
  }
  if (omniORB::trace(25)) {
    omniORB::logger log;
    CORBA::String_var addr(ai->asString());
    log << "Bind to address " << addr << " ";
    if (pd_address.port)
      log << "port " << pd_address.port << ".\n";
    else
      log << "ephemeral port.\n";
  }
  if (::bind(pd_socket, ai->addr(), ai->addrSize()) == RC_SOCKET_ERROR) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      CORBA::String_var addr(ai->asString());
      log << "Failed to bind to address " << addr << " ";
      if (pd_address.port)
	log << "port " << pd_address.port << ". Address in use?\n";
      else
	log << "ephemeral port.\n";
    }
    CLOSESOCKET(pd_socket);
    return 0;
  }

  if (listen(pd_socket,SOMAXCONN) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    omniORB::logs(1, "Failed to listen on socket.");
    return 0;
  }

  // Now figure out the details to put in IORs

  OMNI_SOCKADDR_STORAGE addr;
  SOCKNAME_SIZE_T l;
  l = sizeof(OMNI_SOCKADDR_STORAGE);
  if (getsockname(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    omniORB::logs(1, "Failed to get socket name.");
    return 0;
  }
  pd_address.port = tcpConnection::addrToPort((struct sockaddr*)&addr);

  if (passive_host) {
    // Ask the TCP transport for its list of interface addresses

    CORBA::ULong   addrs_len = 0;
    CORBA::Boolean set_host  = 0;

    const omnivector<const char*>* ifaddrs
      = giopTransportImpl::getInterfaceAddress("giop:tcp");

    if (ifaddrs && !ifaddrs->empty()) {
      // TCP transport successfully gave us a list of interface addresses

      const char* loopback4 = 0;
      const char* loopback6 = 0;

      omnivector<const char*>::const_iterator i;
      for (i = ifaddrs->begin(); i != ifaddrs->end(); i++) {

	if (passive_host == 2 && !LibcWrapper::isip4addr(*i))
	  continue;

	if (passive_host == 3 && !LibcWrapper::isip6addr(*i))
	  continue;

	if (omni::strMatch(*i, "127.0.0.1")) {
	  loopback4 = *i;
	  continue;
	}
	if (omni::strMatch(*i, "::1")) {
	  loopback6 = *i;
	  continue;
	}
	pd_addresses.length(addrs_len + 1);
	pd_addresses[addrs_len++] = omniURI::buildURI("giop:ssl:",
						      *i, pd_address.port);

	if (!set_host) {
	  pd_address.host = CORBA::string_dup(*i);
	  set_host = 1;
	}
      }
      if (!set_host) {
	// No suitable addresses other than the loopback.
	if (loopback4) {
	  pd_addresses.length(addrs_len + 1);
	  pd_addresses[addrs_len++] = omniURI::buildURI("giop:ssl:",
							loopback4,
							pd_address.port);
	  pd_address.host = CORBA::string_dup(loopback4);
	  set_host = 1;
	}
	if (loopback6) {
	  pd_addresses.length(addrs_len + 1);
	  pd_addresses[addrs_len++] = omniURI::buildURI("giop:ssl:",
							loopback6,
							pd_address.port);
	  if (!set_host) {
	    pd_address.host = CORBA::string_dup(loopback6);
	    set_host = 1;
	  }
	}
	if (!set_host) {
	  omniORB::logs(1, "No suitable address in the list of "
			"interface addresses.");
	  CLOSESOCKET(pd_socket);
	  return 0;
	}
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
      if (orbParameters::dumpConfiguration || omniORB::trace(10)) {
	omniORB::logger log;
	log << "My hostname is '" << self << "'.\n";
      }
      LibcWrapper::AddrInfo_var ai;
      ai = LibcWrapper::getAddrInfo(self, pd_address.port);
      if ((LibcWrapper::AddrInfo*)ai == 0) {
	if (omniORB::trace(1)) {
	  omniORB::logger log;
	  log << "Cannot get the address of my hostname '"
	      << self << "'.\n";
	}
	CLOSESOCKET(pd_socket);
	return 0;
      }
      pd_address.host = ai->asString();
      pd_addresses.length(1);
      pd_addresses[0] = omniURI::buildURI("giop:ssl:",
					  pd_address.host, pd_address.port);
    }
    if (omniORB::trace(1) &&
	(omni::strMatch(pd_address.host, "127.0.0.1") ||
	 omni::strMatch(pd_address.host, "::1"))) {

      omniORB::logger log;
      log << "Warning: the local loop back interface (" << pd_address.host
	  << ")\n"
	  << "is the only address available for this server.\n";
    }
  }
  else {
    // Specific host
    pd_addresses.length(1);
    pd_addresses[0] = omniURI::buildURI("giop:ssl:",
					pd_address.host, pd_address.port);
  }

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

  pd_go = 0;
  if (!target->Poke()) {
    if (omniORB::trace(5)) {
      omniORB::logger log;
      log << "Warning: fail to connect to myself (" 
	  << (const char*) pd_addresses[0] << ") via ssl.\n";
    }
  }
  // Wake up the SocketCollection in case the connect did not work and
  // it is idle and blocked with no timeout.
  wakeUp();

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

      int go = pd_go;
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
	case SSL_ERROR_ZERO_RETURN:
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
