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
#include <ssl/sslConnection.h>
#include <ssl/sslAddress.h>
#include <ssl/sslEndpoint.h>
#include <tcp/tcpEndpoint.h>
#include <openssl/err.h>

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
sslEndpoint::sslEndpoint(const IIOP::Address& address, sslContext* ctx) : 
  pd_socket(RC_INVALID_SOCKET), pd_address(address), pd_ctx(ctx),
  pd_n_fdset_1(0), pd_n_fdset_2(0), pd_n_fdset_dib(0),
  pd_abs_sec(0), pd_abs_nsec(0) {

  pd_address_string = (const char*) "giop:ssl:255.255.255.255:65535";
  // address string is not valid until bind is called.

  FD_ZERO(&pd_fdset_1);
  FD_ZERO(&pd_fdset_2);
  FD_ZERO(&pd_fdset_dib);

  pd_hash_table = new sslConnection* [tcpEndpoint::hashsize];
  for (CORBA::ULong i=0; i < tcpEndpoint::hashsize; i++)
    pd_hash_table[i] = 0;
}

/////////////////////////////////////////////////////////////////////////
sslEndpoint::sslEndpoint(const char* address, sslContext* ctx) : 
  pd_socket(RC_INVALID_SOCKET), pd_ctx(ctx),
  pd_n_fdset_1(0), pd_n_fdset_2(0), pd_n_fdset_dib(0),
  pd_abs_sec(0), pd_abs_nsec(0) {

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

  FD_ZERO(&pd_fdset_1);
  FD_ZERO(&pd_fdset_2);
  FD_ZERO(&pd_fdset_dib);

  pd_hash_table = new sslConnection* [tcpEndpoint::hashsize];
  for (CORBA::ULong i=0; i < tcpEndpoint::hashsize; i++)
    pd_hash_table[i] = 0;
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
    
  addr.sin_family = INETSOCKET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(pd_address.port);

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

  {
    char self[64];
    if (gethostname(&self[0],64) == RC_SOCKET_ERROR) {
      if (omniORB::trace(0)) {
	omniORB::logger log;
	log << "Cannot get the name of this host\n";
      }
      CLOSESOCKET(pd_socket);
      return 0;
    }

    LibcWrapper::hostent_var h;
    int rc;

    if (LibcWrapper::gethostbyname(self,h,rc) < 0) {
      if (omniORB::trace(0)) {
	omniORB::logger log;
	log << "Cannot get the address of this host\n";
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
  if (strcmp(pd_address.host,"127.0.0.1") == 0 && omniORB::trace(1)) {
    omniORB::logger log;
    log << "Warning: the local loop back interface (127.0.0.1) is used as this server's address.\n";
    log << "Warning: only clients on this machine can talk to this server.\n";
  }

  const char* format = "giop:ssl:%s:%d";
  pd_address_string = CORBA::string_alloc(strlen(pd_address.host)+strlen(format)+6);
  sprintf((char*)pd_address_string,format,
	  (const char*)pd_address.host,(int)pd_address.port);

  FD_SET(pd_socket,&pd_fdset_1);
  FD_SET(pd_socket,&pd_fdset_2);
  pd_n_fdset_1++;
  pd_n_fdset_2++;
  return 1;

}

/////////////////////////////////////////////////////////////////////////
void
sslEndpoint::Poke() {

  sslAddress* target = new sslAddress(pd_address,pd_ctx);
  sslConnection* conn;
  if ((conn = (sslConnection*)target->Connect()) == 0) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Warning: Fail to connect to myself (" 
	  << (const char*) pd_address_string << ") via ssl!\n";
      log << "Warning: ATM this is ignored but this may cause the ORB shutdown to hang.\n";
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
}


/////////////////////////////////////////////////////////////////////////
giopConnection*
sslEndpoint::AcceptAndMonitor(giopEndpoint::notifyReadable_t func,
			      void* cookie) {

  OMNIORB_ASSERT(pd_socket != RC_INVALID_SOCKET);

  struct timeval timeout;
  fd_set         rfds;
  tcpSocketHandle_t sock;
  int            total;

 again:
  
  while (1) {

    // (pd_abs_sec,pd_abs_nsec) define the absolute time when we switch fdset
    tcpConnection::setTimeOut(pd_abs_sec,pd_abs_nsec,timeout);

    if (timeout.tv_sec == 0 && timeout.tv_usec == 0) {

      omni_thread::get_time(&pd_abs_sec,&pd_abs_nsec,
			    tcpEndpoint::scan_interval_sec,
			    tcpEndpoint::scan_interval_nsec);
      timeout.tv_sec = tcpEndpoint::scan_interval_sec;
      timeout.tv_usec = tcpEndpoint::scan_interval_nsec / 1000;

      omni_tracedmutex_lock sync(pd_fdset_lock);
      rfds  = pd_fdset_2;
      total = pd_n_fdset_2;
      pd_fdset_2 = pd_fdset_1;
      pd_n_fdset_2 = pd_n_fdset_1;
    }
    else {

      omni_tracedmutex_lock sync(pd_fdset_lock);
      rfds  = pd_fdset_2;
      total = pd_n_fdset_2;
    }

    int maxfd = 0;
    int fd = 0;
    while (total) {
      if (FD_ISSET(fd,&rfds)) {
	maxfd = fd;
	total--;
      }
      fd++;
    }

    int nready = select(maxfd+1,&rfds,0,0,&timeout);

    if (nready == RC_SOCKET_ERROR) {
      if (ERRNO != RC_EINTR) {
	sock = RC_SOCKET_ERROR;
	break;
      }
      else {
	continue;
      }
    }

    // Reach here if nready >= 0

    if (FD_ISSET(pd_socket,&rfds)) {
      // Got a new connection
      sock = ::accept(pd_socket,0,0);
      break;
    }

    {
      omni_tracedmutex_lock sync(pd_fdset_lock);

      // Process the result from the select.
      tcpSocketHandle_t fd = 0;
      while (nready) {
	if (FD_ISSET(fd,&rfds)) {
	  notifyReadable(fd,func,cookie);
	  if (FD_ISSET(fd,&pd_fdset_1)) {
	    pd_n_fdset_1--;
	    FD_CLR(fd,&pd_fdset_1);
	  }
	  if (FD_ISSET(fd,&pd_fdset_dib)) {
	    pd_n_fdset_dib--;
	    FD_CLR(fd,&pd_fdset_dib);
	  }
	  nready--;
	}
	fd++;
      }

      // Process pd_fdset_dib. Those sockets with their bit set have
      // already got data in buffer. We do a call back for these sockets if
      // their entries in pd_fdset_1 is also set.
      fd = 0;
      nready = pd_n_fdset_dib;
      while (nready) {
	if (FD_ISSET(fd,&pd_fdset_dib)) {
	  if (FD_ISSET(fd,&pd_fdset_2)) {
	    notifyReadable(fd,func,cookie);
	    if (FD_ISSET(fd,&pd_fdset_1)) {
	      pd_n_fdset_1--;
	      FD_CLR(fd,&pd_fdset_1);
	    }
	    pd_n_fdset_dib--;
	    FD_CLR(fd,&pd_fdset_dib);
	  }
	  nready--;
	}
	fd++;
      }
    }
  }

  if (sock == RC_SOCKET_ERROR) {
    return 0;
  }

  ::SSL* ssl = SSL_new(pd_ctx->get_SSL_CTX());
  SSL_set_fd(ssl, sock);
  SSL_set_accept_state(ssl);

  while(1) {
    int result = SSL_accept(ssl);
    int code = SSL_get_error(ssl, result);

    switch(code) {
    case SSL_ERROR_NONE:
      return new sslConnection(sock,ssl,this);

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
	CLOSESOCKET(sock);
	//break;
	// XXX We should be able to go back to accept again. But for
	//     some reason the SSL library SEGV if we do. For the time
	//     being, we returns 0 which effectively shutdown the endpoint.
	return 0;
      }
    }
  }
  goto again;
}

/////////////////////////////////////////////////////////////////////////
void
sslEndpoint::notifyReadable(tcpSocketHandle_t fd,
			    giopEndpoint::notifyReadable_t func,
			    void* cookie) {

  // Caller hold pd_fdset_lock

  if (FD_ISSET(fd,&pd_fdset_2)) {
    pd_n_fdset_2--;
    FD_CLR(fd,&pd_fdset_2);
    giopConnection* conn = 0;
    sslConnection** head = &(pd_hash_table[fd%tcpEndpoint::hashsize]);
    while (*head) {
      if ((*head)->handle() == fd) {
	conn = (giopConnection*)(*head);
	break;
      }
      head = &((*head)->pd_next);
    }
    if (conn) {
      // Note: do the callback while holding pd_fdset_lock
      func(cookie,conn);
    }
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslConnection::setSelectable(CORBA::Boolean now,
			     CORBA::Boolean data_in_buffer) {

  if (!pd_endpoint) return;

  omni_tracedmutex_lock sync(pd_endpoint->pd_fdset_lock);

  if ((data_in_buffer || SSL_pending(ssl_handle())) && 
      !FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_dib))) {
    pd_endpoint->pd_n_fdset_dib++;
    FD_SET(pd_socket,&(pd_endpoint->pd_fdset_dib));
  }

  if (!FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_1)))
    pd_endpoint->pd_n_fdset_1++;
  FD_SET(pd_socket,&(pd_endpoint->pd_fdset_1));
  if (now) {
    if (!FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_2)))
      pd_endpoint->pd_n_fdset_2++;
    FD_SET(pd_socket,&(pd_endpoint->pd_fdset_2));
    // XXX poke the thread doing accept to look at the fdset immediately.
  }
}


/////////////////////////////////////////////////////////////////////////
void
sslConnection::clearSelectable() {

  if (!pd_endpoint) return;

  omni_tracedmutex_lock sync(pd_endpoint->pd_fdset_lock);

  if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_1)))
    pd_endpoint->pd_n_fdset_1--;
  FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_1));
  if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_2)))
    pd_endpoint->pd_n_fdset_2--;
  FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_2));
  if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_dib)))
    pd_endpoint->pd_n_fdset_dib--;
  FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_dib));
}

/////////////////////////////////////////////////////////////////////////
void
sslConnection::Peek(giopEndpoint::notifyReadable_t func, void* cookie) {

  if (!pd_endpoint) return;

  {
    omni_tracedmutex_lock sync(pd_endpoint->pd_fdset_lock);
   
    // Do nothing if this connection is not set to be monitored.
    if (!FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_1)))
      return;

    // If data in buffer is set, do callback straight away.
    if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_dib)) ||
	SSL_pending(ssl_handle())) {

      func(cookie,this);
      if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_1))) {
	pd_endpoint->pd_n_fdset_1--;
	FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_1));
      }
      pd_endpoint->pd_n_fdset_2--;
      FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_2));
      pd_endpoint->pd_n_fdset_dib--;
      FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_dib));
    }
  }

  struct timeval timeout;
  // select on the socket for half the time of scan_interval, if no request
  // arrives in this interval, we just let AcceptAndMonitor take care
  // of it.
  timeout.tv_sec = tcpEndpoint::scan_interval_sec / 2;
  timeout.tv_usec = tcpEndpoint::scan_interval_nsec / 1000 / 2;
  fd_set         rfds;

  do {
    FD_SET(pd_socket,&rfds);
    int nready = select(pd_socket+1,&rfds,0,0,&timeout);

    if (nready == RC_SOCKET_ERROR) {
      if (ERRNO != RC_EINTR) {
	break;
      }
      else {
	continue;
      }
    }

    // Reach here if nready >= 0

    if (FD_ISSET(pd_socket,&rfds)) {
      omni_tracedmutex_lock sync(pd_endpoint->pd_fdset_lock);

      // Are we still interested?
      if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_1))) {
	func(cookie,this);
	if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_2))) {
	  pd_endpoint->pd_n_fdset_2--;
	  FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_2));
	}
	pd_endpoint->pd_n_fdset_1--;
	FD_CLR(pd_socket,&pd_endpoint->pd_fdset_1);
	if (FD_ISSET(pd_socket,&(pd_endpoint->pd_fdset_dib))) {
	  pd_endpoint->pd_n_fdset_dib--;
	  FD_CLR(pd_socket,&(pd_endpoint->pd_fdset_dib));
	}
      }
    }
  } while(0);
}


OMNI_NAMESPACE_END(omni)
