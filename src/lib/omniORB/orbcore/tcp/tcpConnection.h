// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpConnection.h            Created on: 19 Mar 2001
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
  Revision 1.1.2.2  2001/06/13 20:13:49  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.1.2.1  2001/04/18 18:10:44  sll
  Big checkin with the brand new internal APIs.

*/

#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__


////////////////////////////////////////////////////////////////////////
//  Platform feature selection

#define USE_NONBLOCKING_CONNECT

#if defined(__linux__)
#   define USE_POLL
#endif

#if defined(__sunos__)
#   define USE_POLL
#endif

#if defined(__hpux__)
#   if __OSVERSION__ >= 11
#       define USE_POLL
#   endif
#   define USE_FAKE_INTERRUPTABLE_RECV
#endif

#if defined(__win32__)
#   define USE_FAKE_INTERRUPTABLE_RECV
#endif

#if defined(__irix__)
#   define USE_POLL
#endif

////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//             win32 API
//
#if defined(__WIN32__)

#include <winsock.h>
#include <sys/types.h>
#include <libcWrapper.h>

#define RC_INADDR_NONE     INADDR_NONE
#define RC_INVALID_SOCKET  INVALID_SOCKET
#define RC_SOCKET_ERROR    SOCKET_ERROR
#define INETSOCKET         PF_INET
#define CLOSESOCKET(sock)  closesocket(sock)
#define SHUTDOWNSOCKET(sock) ::shutdown(sock,2)
#define ERRNO              ::WSAGetLastError()
#define EINPROGRESS        WSAEWOULDBLOCK
#define RC_EINTR           WSAEINTR
#define SOCKNAME_SIZE_T    int

OMNI_NAMESPACE_BEGIN(omni)

typedef SOCKET tcpSocketHandle_t;

OMNI_NAMESPACE_END(omni)

#else

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//             unix
//
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <libcWrapper.h>

#if defined(USE_POLL)
#include <poll.h>
#endif

#if !defined(__VMS)
#include <fcntl.h>
#endif

#if defined (__uw7__)
#ifdef shutdown
#undef shutdown
#endif
#endif

#if defined(__VMS) && defined(USE_tcpSocketVaxRoutines)
#include "tcpSocketVaxRoutines.h"
#undef accept
#undef recv
#undef send
#define accept(a,b,c) tcpSocketVaxAccept(a,b,c)
#define recv(a,b,c,d) tcpSocketVaxRecv(a,b,c,d)
#define send(a,b,c,d) tcpSocketVaxSend(a,b,c,d)
#endif

#ifdef __rtems__
extern "C" int select (int,fd_set*,fd_set*,fd_set*,struct timeval *);
#endif

#define RC_INADDR_NONE     ((CORBA::ULong)-1)
#define RC_INVALID_SOCKET  (-1)
#define RC_SOCKET_ERROR    (-1)
#define INETSOCKET         AF_INET
#define CLOSESOCKET(sock)   close(sock)
#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#elif defined(__osf1__) && defined(__alpha__)
#define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#else
  // XXX none of the above, calling shutdown() may not have the
  // desired effect.
#define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#endif
#define ERRNO              errno
#define RC_EINTR           EINTR

#if (defined(__GLIBC__) && __GLIBC__ >= 2) || (defined(__freebsd__) && __OSVERSION__ >= 4)
#  define SOCKNAME_SIZE_T  socklen_t
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
#  define SOCKNAME_SIZE_T  size_t
#else
#  define SOCKNAME_SIZE_T  int
# endif

OMNI_NAMESPACE_BEGIN(omni)

typedef int    tcpSocketHandle_t;

OMNI_NAMESPACE_END(omni)

#endif

#if defined(NEED_GETHOSTNAME_PROTOTYPE)
extern "C" int gethostname(char *name, int namelen);
#endif

OMNI_NAMESPACE_BEGIN(omni)

class tcpConnection : public giopConnection {
 public:

  int send(void* buf, size_t sz,
	   unsigned long deadline_secs = 0,
	   unsigned long deadline_nanosecs = 0);

  int recv(void* buf, size_t sz,
	   unsigned long deadline_secs = 0,
	   unsigned long deadline_nanosecs = 0);

  void shutdown();

  const char* myaddress();

  const char* peeraddress();

  tcpSocketHandle_t handle() const;

  tcpConnection(tcpSocketHandle_t);

  ~tcpConnection();

  static void setTimeOut(unsigned long abs_sec,
			 unsigned long abs_nsec,struct timeval& t);

  static int setnonblocking(tcpSocketHandle_t sock);

  static int setblocking(tcpSocketHandle_t sock);

  static char* ip4ToString(CORBA::ULong);
  static char* ip4ToString(CORBA::ULong,CORBA::UShort,const char* prefix=0);

 private:
  tcpSocketHandle_t pd_socket;
  CORBA::String_var pd_myaddress;
  CORBA::String_var pd_peeraddress;
};

OMNI_NAMESPACE_END(omni)

#endif __TCPCONNECTION_H__
