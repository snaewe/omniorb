// -*- Mode: C++; -*-
//                            Package   : omniORB
// SocketCollection.h         Created on: 23 Jul 2001
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
  Revision 1.1.2.2  2001/08/23 10:13:14  sll
  Cope with the different length type for getsockname and friends on
  different solaris versions.

  Revision 1.1.2.1  2001/07/31 16:16:26  sll
  New transport interface to support the monitoring of active connections.

*/

#ifndef __SOCKETCOLLECTION_H__
#define __SOCKETCOLLECTION_H__

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

typedef SOCKET SocketHandle_t;
typedef fd_set SocketHandleSet_t;

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
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__) || ( defined(__sunos__) && defined(_XPG4_2) )
#  define SOCKNAME_SIZE_T  size_t
#else
#  define SOCKNAME_SIZE_T  int
# endif

OMNI_NAMESPACE_BEGIN(omni)

typedef int    SocketHandle_t;
typedef fd_set SocketHandleSet_t;

OMNI_NAMESPACE_END(omni)

#endif

#if defined(NEED_GETHOSTNAME_PROTOTYPE)
extern "C" int gethostname(char *name, int namelen);
#endif

OMNI_NAMESPACE_BEGIN(omni)

class SocketCollection;

extern void SocketSetTimeOut(unsigned long abs_sec,
			     unsigned long abs_nsec,struct timeval& t);

extern int SocketSetnonblocking(SocketHandle_t sock);

extern int SocketSetblocking(SocketHandle_t sock);


class SocketLink {

public:
  SocketLink(SocketHandle_t s) : pd_socket(s), pd_next(0) {}
  ~SocketLink() {}

  friend class SocketCollection;

protected:
  SocketHandle_t pd_socket;

private:
  SocketLink*    pd_next;
};

class SocketCollection {
public:

  SocketCollection();
  virtual ~SocketCollection();

protected:
  virtual CORBA::Boolean notifyReadable(SocketHandle_t) = 0;
  // Callback used by Select(). This method is called while holding
  // pd_fdset_lock.

public:
  void setSelectable(SocketHandle_t sock, CORBA::Boolean now,
		     CORBA::Boolean data_in_buffer,
		     CORBA::Boolean hold_lock=0);
  // Indicates that this socket should be watched by a select()
  // so that any new data arriving on the connection will be noted.
  // If now == 1, immediately make this socket part of the select
  // set.
  // If data_in_buffer == 1, treat this socket as if there are
  // data available from the connection already.

  void clearSelectable(SocketHandle_t);
  // Indicates that this connection need not be watched any more.

  CORBA::Boolean Select();
  // Returns TRUE(1) if the Select() has successfully done a scan.
  // otherwise returns false(0) to indicate that an error has been
  // detected and this function should not be called again.
  //
  // For each of the sockets that has been marked watchable and indeed
  // has become readable, call notifyReadable() with the socket no.
  // as the argument.

  CORBA::Boolean Peek(SocketHandle_t sock);
  // Do nothing and returns immediately if the socket has not been
  // set to be watched by a previous setSelectable().
  // Otherwise, monitor the socket's status for a short time.
  // Returns TRUE(1) if the socket becomes readable.
  // otherwise returns FALSE(0).


  void addSocket(SocketLink* conn);
  // Add this socket to the collection. <conn> is associated with the
  // socket and should be added to the table hashed by the socket number.

  SocketLink* removeSocket(SocketHandle_t sock);
  // Remove the socket from this collection. Return the socket which has
  // been removed. Return 0 if the socket is not found.

  SocketLink* findSocket(SocketHandle_t sock, 
			 CORBA::Boolean hold_lock=0);
  // Returns the connection that is associated with this socket.
  // Return 0 if this socket cannot be found in the hash table.
  // if hold_lock == 1, the caller has already got the lock on pd_fdset_lock.
  // (use purely by member functions.)

  static unsigned long scan_interval_sec;
  static unsigned long scan_interval_nsec;

  static CORBA::ULong  hashsize;

private:
  SocketHandleSet_t pd_fdset_1;
  SocketHandleSet_t pd_fdset_2;
  SocketHandleSet_t pd_fdset_dib; // data in buffer
  int               pd_n_fdset_1;
  int               pd_n_fdset_2;
  int               pd_n_fdset_dib;
  omni_tracedmutex  pd_fdset_lock;
  unsigned long     pd_abs_sec;
  unsigned long     pd_abs_nsec;

protected:
  SocketLink**      pd_hash_table;      

};

OMNI_NAMESPACE_END(omni)

#endif // __SOCKETCOLLECTION_H__
