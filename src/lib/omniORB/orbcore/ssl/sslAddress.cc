// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslAddress.cc              Created on: 29 May 2001
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
  Revision 1.1.2.7  2001/09/07 11:27:14  sll
  Residual changes needed for the changeover to use orbParameters.

  Revision 1.1.2.6  2001/07/31 16:16:24  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.5  2001/07/26 16:37:21  dpg1
  Make sure static initialisers always run.

  Revision 1.1.2.4  2001/07/13 15:35:09  sll
   Error in setblocking and setnonblocking now causes the socket to be closed
   as well.

  Revision 1.1.2.3  2001/06/20 18:35:17  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.2  2001/06/18 20:27:56  sll
  Use strchr instead of index() for maximal portability.

  Revision 1.1.2.1  2001/06/11 18:11:07  sll
  *** empty log message ***

*/

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <orbParameters.h>
#include <omniORB4/sslContext.h>
#include <SocketCollection.h>
#include <ssl/sslConnection.h>
#include <ssl/sslAddress.h>
#include <openssl/err.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(sslAddress);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
sslAddress::sslAddress(const IIOP::Address& address, sslContext* ctx) : 
  pd_address(address), pd_ctx(ctx) {

  const char* format = "giop:ssl:%s:%d";
  pd_address_string = CORBA::string_alloc(strlen(address.host)+
					  strlen(format)+6);
  sprintf((char*)pd_address_string,format,(const char*)address.host,
	  address.port);
}

/////////////////////////////////////////////////////////////////////////
sslAddress::sslAddress(const char* address, sslContext* ctx) : pd_ctx(ctx) {

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
const char*
sslAddress::type() const {
  return "giop:ssl";
}

/////////////////////////////////////////////////////////////////////////
const char*
sslAddress::address() const {
  return pd_address_string;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
sslAddress::duplicate() const {
  return new sslAddress(pd_address,pd_ctx);
}

/////////////////////////////////////////////////////////////////////////
giopActiveConnection*
sslAddress::Connect(unsigned long deadline_secs,
		    unsigned long deadline_nanosecs) const {

  struct sockaddr_in raddr;
  LibcWrapper::hostent_var h;
  int  rc;
  SocketHandle_t sock;
    
  if (! LibcWrapper::isipaddr(pd_address.host)) {
    if (LibcWrapper::gethostbyname(pd_address.host,h,rc) < 0) {
      return 0;
    }
    // We just pick the first address in the list, may be we should go
    // through the list and if possible pick the one that is on the same
    // subnet.
    memcpy((void*)&raddr.sin_addr,
	   (void*)h.hostent()->h_addr_list[0],
	   sizeof(raddr.sin_addr));
  }
  else {
    // The machine name is already an IP address
    CORBA::ULong ip_p;
    if ( (ip_p = inet_addr(pd_address.host)) == RC_INADDR_NONE) {
      return 0;
    }
    memcpy((void*) &raddr.sin_addr, (void*) &ip_p, sizeof(raddr.sin_addr));
  }

  raddr.sin_family = INETSOCKET;
  raddr.sin_port   = htons(pd_address.port);

  if ((sock = socket(INETSOCKET,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return 0;
  }

  if (SocketSetnonblocking(sock) == RC_INVALID_SOCKET) {
    CLOSESOCKET(sock);
    return 0;
  }
    
  if (::connect(sock,(struct sockaddr *)&raddr,
		sizeof(struct sockaddr_in)) == RC_SOCKET_ERROR) {

    if (ERRNO != EINPROGRESS) {
      CLOSESOCKET(sock);
      return 0;
    }
  }

  do {

    struct timeval t;

    if (deadline_secs || deadline_nanosecs) {
      SocketSetTimeOut(deadline_secs,deadline_nanosecs,t);
      if (t.tv_sec == 0 && t.tv_usec == 0) {
	// Already timeout.
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
    int timeout = t.tv_sec*1000+(t.tv_usec/1000);
    if (timeout == 0) timeout = -1;
    int rc = poll(&fds,1,timeout);
    if (rc > 0 && fds.revents & POLLERR) {
      rc = 0;
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
      CLOSESOCKET(sock);
      return 0;
#endif
    }
    else if (rc == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EINTR)
	continue;
      else {
	CLOSESOCKET(sock);
	return 0;
      }
    }

  } while (0);

  if (SocketSetblocking(sock) == RC_INVALID_SOCKET) {
    CLOSESOCKET(sock);
    return 0;
  }

  ::SSL* ssl = SSL_new(pd_ctx->get_SSL_CTX());
  SSL_set_fd(ssl, sock);
  SSL_set_connect_state(ssl);

  while(1) {
    int result = SSL_connect(ssl);
    int code = SSL_get_error(ssl, result);

    switch(code) {
    case SSL_ERROR_NONE:
      return new sslActiveConnection(sock,ssl);

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
	  log << "openSSL error detected in sslAddress::connect. Reason: "
	      << (const char*) buf << "\n";
	}
	SSL_free(ssl);
	CLOSESOCKET(sock);
	return 0;
      }
    }
    OMNIORB_ASSERT(0);
  }
}


OMNI_NAMESPACE_END(omni)
