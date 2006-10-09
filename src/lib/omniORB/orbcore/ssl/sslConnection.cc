// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslConnection.cc           Created on: 19 Mar 2001
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
  Revision 1.1.4.9  2006/10/09 13:08:58  dgrisby
  Rename SOCKADDR_STORAGE define to OMNI_SOCKADDR_STORAGE, to avoid
  clash on Win32 2003 SDK.

  Revision 1.1.4.8  2006/03/25 18:54:03  dgrisby
  Initial IPv6 support.

  Revision 1.1.4.7  2005/09/05 17:12:20  dgrisby
  Merge again. Mainly SSL transport changes.

  Revision 1.1.4.6  2005/09/01 14:52:12  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.5  2005/03/02 12:39:17  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.4  2005/03/02 12:10:49  dgrisby
  setSelectable / Peek fixes.

  Revision 1.1.4.3  2005/01/13 21:10:01  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.2  2005/01/06 23:10:52  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:01:59  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.11  2001/12/03 13:39:55  dpg1
  Explicit socket shutdown flag for Windows.

  Revision 1.1.2.10  2001/11/26 10:51:04  dpg1
  Wrong endpoint address when getsockname() fails.

  Revision 1.1.2.9  2001/09/07 11:27:15  sll
  Residual changes needed for the changeover to use orbParameters.

  Revision 1.1.2.8  2001/08/24 15:56:44  sll
  Fixed code which made the wrong assumption about the semantics of
  do { ...; continue; } while(0)

  Revision 1.1.2.7  2001/07/31 16:16:23  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.6  2001/07/26 16:37:21  dpg1
  Make sure static initialisers always run.

  Revision 1.1.2.5  2001/07/13 15:35:57  sll
  Enter a mapping from a socket to a giopConnection in the endpoint's hash
  table.

  Revision 1.1.2.4  2001/06/29 16:26:01  dpg1
  Reinstate tracing messages for new connections and handling locate
  requests.

  Revision 1.1.2.3  2001/06/26 13:38:45  sll
  Make ssl compiles with pre-0.9.6a OpenSSL

  Revision 1.1.2.2  2001/06/20 18:35:16  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.1  2001/06/11 18:11:06  sll
  *** empty log message ***

  Revision 1.1.2.1  2001/04/18 18:10:44  sll
  Big checkin with the brand new internal APIs.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <orbParameters.h>
#include <omniORB4/sslContext.h>
#include <SocketCollection.h>
#include <ssl/sslConnection.h>
#include <ssl/sslEndpoint.h>
#include <tcp/tcpConnection.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(sslConnection);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
int
sslConnection::Send(void* buf, size_t sz,
		    unsigned long deadline_secs,
		    unsigned long deadline_nanosecs) {

  if (sz > orbParameters::maxSocketSend)
    sz = orbParameters::maxSocketSend;

  int tx;
  int rc;

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
#if OPENSSL_VERSION_NUMBER >= 0x0090601fL
    tx = SSL_write(pd_ssl,buf,sz);
#else
    tx = SSL_write(pd_ssl,(char*)buf,sz);
#endif

    rc = SSL_get_error(pd_ssl, tx);

    switch(rc) {
    case SSL_ERROR_NONE:
      break;

    case SSL_ERROR_SSL:
    case SSL_ERROR_ZERO_RETURN:
      return -1;

    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      continue;

    case SSL_ERROR_SYSCALL:
      if (ERRNO == RC_EINTR)
	continue;
      else
	return -1;
    default:
      OMNIORB_ASSERT(0);
    }

    OMNIORB_ASSERT(tx != 0);

    break;

  } while(1);

  return tx;
}

/////////////////////////////////////////////////////////////////////////
int
sslConnection::Recv(void* buf, size_t sz,
		    unsigned long deadline_secs,
		    unsigned long deadline_nanosecs) {

  if (sz > orbParameters::maxSocketRecv)
    sz = orbParameters::maxSocketRecv;

  int rx;
  int rc;

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

    if ( (t.tv_sec || t.tv_usec) && SSL_pending(pd_ssl) <=0 ) {
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
#if OPENSSL_VERSION_NUMBER >= 0x0090601fL
    rx = SSL_read(pd_ssl,buf,sz);
#else
    rx = SSL_read(pd_ssl,(char*)buf,sz);
#endif

    rc = SSL_get_error(pd_ssl, rx);

    switch(rc) {
    case SSL_ERROR_NONE:
      break;

    case SSL_ERROR_SSL:
    case SSL_ERROR_ZERO_RETURN:
      return -1;

    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      continue;

    case SSL_ERROR_SYSCALL:
      if (ERRNO == RC_EINTR)
	continue;
      else
	return -1;
    default:
      OMNIORB_ASSERT(0);
    }

    OMNIORB_ASSERT(rx != 0);

    break;

  } while(1);

  return rx;
}

/////////////////////////////////////////////////////////////////////////
void
sslConnection::Shutdown() {
  SSL_set_shutdown(pd_ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
  SSL_shutdown(pd_ssl);
  SHUTDOWNSOCKET(pd_socket);
  pd_shutdown = 1;
}

/////////////////////////////////////////////////////////////////////////
const char*
sslConnection::myaddress() {
  return (const char*)pd_myaddress;
}

/////////////////////////////////////////////////////////////////////////
const char*
sslConnection::peeraddress() {
  return (const char*)pd_peeraddress;
}

const char*
sslConnection::peeridentity() {
  return (const char *)pd_peeridentity;
}

/////////////////////////////////////////////////////////////////////////
sslConnection::sslConnection(SocketHandle_t sock,::SSL* ssl, 
			     SocketCollection* belong_to) : 
  SocketHolder(sock), pd_ssl(ssl) {

  OMNI_SOCKADDR_STORAGE addr;
  SOCKNAME_SIZE_T l;

  l = sizeof(OMNI_SOCKADDR_STORAGE);
  if (getsockname(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    pd_myaddress = (const char*)"giop:ssl:255.255.255.255:65535";
  }
  else {
    pd_myaddress = tcpConnection::addrToURI((sockaddr*)&addr, "giop:ssl:");
  }

  l = sizeof(OMNI_SOCKADDR_STORAGE);
  if (getpeername(pd_socket,
		  (struct sockaddr *)&addr,&l) == RC_SOCKET_ERROR) {
    pd_peeraddress = (const char*)"giop:ssl:255.255.255.255:65535";
  }
  else {
    pd_peeraddress = tcpConnection::addrToURI((sockaddr*)&addr, "giop:ssl:");
  }
  SocketSetCloseOnExec(sock);

  belong_to->addSocket(this);

  // determine our peer identity, if there is one
  X509 *peer_cert = SSL_get_peer_certificate(pd_ssl);

  if (peer_cert) {
    if (SSL_get_verify_result(pd_ssl) != X509_V_OK)
      return;

    char buf[1024];

    X509_NAME_get_text_by_NID(X509_get_subject_name(peer_cert),
			      NID_commonName, buf, sizeof(buf));

    pd_peeridentity = CORBA::string_dup(buf);
    X509_free(peer_cert);
  }
}

/////////////////////////////////////////////////////////////////////////
sslConnection::~sslConnection() {

  clearSelectable();
  pd_belong_to->removeSocket(this);

  if(pd_ssl != 0) {
    if (SSL_get_shutdown(pd_ssl) == 0) {
      SSL_set_shutdown(pd_ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
      SSL_shutdown(pd_ssl);
    }
    SSL_free(pd_ssl);
    pd_ssl = 0;
  }

  CLOSESOCKET(pd_socket);
}

/////////////////////////////////////////////////////////////////////////
void
sslConnection::setSelectable(int now,
			     CORBA::Boolean data_in_buffer) {

  if (SSL_pending(ssl_handle())) data_in_buffer = 1;

  SocketHolder::setSelectable(now,data_in_buffer);
}


/////////////////////////////////////////////////////////////////////////
void
sslConnection::clearSelectable() {

  SocketHolder::clearSelectable();
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
sslConnection::isSelectable() {
  return pd_belong_to->isSelectable(pd_socket);
}


/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
sslConnection::Peek() {

  if (SSL_pending(ssl_handle())) {
    return 1;
  }
  return SocketHolder::Peek();
}


OMNI_NAMESPACE_END(omni)
