// -*- Mode: C++; -*-
//                            Package   : omniORB
// SocketCollection.h         Created on: 23 Jul 2001
//                            Author    : Sai Lai Lo (sll)
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//    Copyright (C) 2005 Apasphere Ltd.
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
  Revision 1.1.4.3  2005/01/13 21:09:57  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.2  2005/01/06 23:08:25  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:03:55  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.16  2003/02/17 01:45:50  dgrisby
  Pipe to kick select thread (on Unix).

  Revision 1.1.2.15  2002/10/04 11:11:45  dgrisby
  Transport fixes: ENOTSOCK for Windows, SOMAXCONN in listen().

  Revision 1.1.2.14  2002/09/08 22:12:22  dgrisby
  Last checkin broke it.

  Revision 1.1.2.13  2002/09/08 21:58:49  dgrisby
  Support for MSVC 7. (Untested.)

  Revision 1.1.2.12  2002/08/21 06:23:15  dgrisby
  Properly clean up bidir connections and ropes. Other small tweaks.

  Revision 1.1.2.11  2002/05/07 12:54:38  dgrisby
  Fix inevitable Windows header brokenness.

  Revision 1.1.2.10  2002/04/28 20:43:25  dgrisby
  Windows, FreeBSD, ETS fixes.

  Revision 1.1.2.9  2002/03/13 16:05:38  dpg1
  Transport shutdown fixes. Reference count SocketCollections to avoid
  connections using them after they are deleted. Properly close
  connections when in thread pool mode.

  Revision 1.1.2.8  2002/02/26 14:06:44  dpg1
  Recent changes broke Windows.

  Revision 1.1.2.7  2002/02/13 16:02:38  dpg1
  Stability fixes thanks to Bastiaan Bakker, plus threading
  optimisations inspired by investigating Bastiaan's bug reports.

  Revision 1.1.2.6  2002/01/15 16:38:11  dpg1
  On the road to autoconf. Dependencies refactored, configure.ac
  written. No makefiles yet.

  Revision 1.1.2.5  2001/12/03 13:39:54  dpg1
  Explicit socket shutdown flag for Windows.

  Revision 1.1.2.4  2001/08/24 16:43:25  sll
  Switch to use Winsock 2. Removed reference to winsock.h. Let the pre-processor
  define _WIN32_WINNT=0x0400 to select the right header.

  Revision 1.1.2.3  2001/08/23 16:03:29  sll
  Corrected typo __win32__, should be __WIN32__.

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

#define SOCKNAME_SIZE_T OMNI_SOCKNAME_SIZE_T

#define USE_NONBLOCKING_CONNECT

#ifdef HAVE_POLL
#   define USE_POLL
#endif

#if defined(__hpux__)
#   if __OSVERSION__ < 11
#       undef USE_POLL
#   endif
#   define USE_FAKE_INTERRUPTABLE_RECV
#endif

#if defined(__WIN32__)
#   define USE_FAKE_INTERRUPTABLE_RECV
#endif

////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//             win32 API
//
#if defined(__WIN32__)

#  include <sys/types.h>
#  include <libcWrapper.h>

#  define RC_INADDR_NONE     INADDR_NONE
#  define RC_INVALID_SOCKET  INVALID_SOCKET
#  define RC_SOCKET_ERROR    SOCKET_ERROR
#  define INETSOCKET         PF_INET
#  define CLOSESOCKET(sock)  closesocket(sock)
#  define SHUTDOWNSOCKET(sock) ::shutdown(sock,2)
#  define ERRNO              ::WSAGetLastError()
#  define EINPROGRESS        WSAEWOULDBLOCK
#  define RC_EINTR           WSAEINTR
#  define RC_EBADF           WSAENOTSOCK

OMNI_NAMESPACE_BEGIN(omni)

typedef SOCKET SocketHandle_t;

OMNI_NAMESPACE_END(omni)

#else

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//             unix(ish)
//
#  if defined(__vxWorks__)
#    include <sockLib.h>
#    include <hostLib.h>
#    include <ioLib.h>
#    include <netinet/tcp.h>
#  else
#    include <sys/time.h>
#  endif
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <arpa/inet.h>
#  include <unistd.h>
#  include <sys/types.h>
#  include <errno.h>
#  include <libcWrapper.h>

#  if defined(USE_POLL)
#    include <poll.h>
#  endif

#  include <fcntl.h>

#  if defined (__uw7__)
#    ifdef shutdown
#      undef shutdown
#    endif
#  endif

#  if defined(__VMS) && defined(USE_tcpSocketVaxRoutines)
#    include "tcpSocketVaxRoutines.h"
#    undef accept
#    undef recv
#    undef send
#    define accept(a,b,c) tcpSocketVaxAccept(a,b,c)
#    define recv(a,b,c,d) tcpSocketVaxRecv(a,b,c,d)
#    define send(a,b,c,d) tcpSocketVaxSend(a,b,c,d)
#  endif

#  ifdef __rtems__
extern "C" int select (int,fd_set*,fd_set*,fd_set*,struct timeval *);
#  endif

#  define RC_INADDR_NONE     ((CORBA::ULong)-1)
#  define RC_INVALID_SOCKET  (-1)
#  define RC_SOCKET_ERROR    (-1)
#  define INETSOCKET         AF_INET
#  define CLOSESOCKET(sock)  close(sock)

#  if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#    define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#  elif defined(__osf1__) && defined(__alpha__)
#    define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#  else
     // XXX none of the above, calling shutdown() may not have the
     // desired effect.
#    define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#  endif

#  define ERRNO              errno
#  define RC_EINTR           EINTR
#  define RC_EBADF           EBADF
#  define RC_EAGAIN          EAGAIN


OMNI_NAMESPACE_BEGIN(omni)

typedef int    SocketHandle_t;

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

extern int SocketSetCloseOnExec(SocketHandle_t sock);


//
// Class SocketHolder holds a socket inside a collection. It contains
// flags to indicate whether the socket is selectable, and so on.
// Connection classes (e.g. tcpConnection) derive from this class.

class SocketHolder {

public:
  SocketHolder(SocketHandle_t s)
    : pd_socket(s),
      pd_belong_to(0),
      pd_shutdown(0),
      pd_selectable(0),
      pd_data_in_buffer(0),
      pd_selected(0),
      pd_next(0),
      pd_prev(0) { }

  virtual ~SocketHolder();

  void setSelectable(CORBA::Boolean now,
		     CORBA::Boolean data_in_buffer,
		     CORBA::Boolean hold_lock=0);
  // Indicate that this socket should be watched for readability.
  //
  // If now is true, immediately make the socket selectable (if the
  // platform permits it), rather than waiting until the select loop
  // rescans.
  //
  // If data_in_buffer is true, the socket is considered to already
  // have data available to read.
  //
  // If hold_lock is true, the associated SocketCollection's lock is
  // already held.

  void clearSelectable();
  // Indicate that this socket should not be watched any more.

  CORBA::Boolean Peek();
  // Do nothing and return immediately if the socket has not been set
  // to be watched by a previous setSelectable().
  // Otherwise, monitor the socket's status for a short time. Return
  // true if the socket becomes readable, otherwise returns false.

  friend class SocketCollection;

protected:
  SocketHandle_t    pd_socket;
  SocketCollection* pd_belong_to;
  CORBA::Boolean    pd_shutdown;

private:
  CORBA::Boolean    pd_selectable;
  CORBA::Boolean    pd_data_in_buffer;
  CORBA::Boolean    pd_selected;
  SocketHolder*     pd_next;
  SocketHolder**    pd_prev;
};


//
// SocketCollection manages a collection of sockets.

class SocketCollection {
public:

  SocketCollection();

  virtual ~SocketCollection();

  virtual CORBA::Boolean notifyReadable(SocketHolder*) = 0;
  // Callback used by Select(). If it returns false, something has
  // gone very wrong with the collection and exits the Select loop.
  // This method is called while holding pd_collection_lock.

  CORBA::Boolean isSelectable(SocketHandle_t sock);
  // Indicates whether the given socket can be selected upon.

  CORBA::Boolean Select();
  // Returns true if the Select() has successfully done a scan.
  // otherwise returns false to indicate that an error has been
  // detected and this function should not be called again.
  //
  // For each of the sockets that has been marked watchable and indeed
  // has become readable, call notifyReadable() with the socket as the
  // argument.

  void incrRefCount();
  void decrRefCount();

  void addSocket(SocketHolder* sock);
  // Add this socket to the collection. Increments this collection's
  // refcount.

  void removeSocket(SocketHolder* sock);
  // Remove the socket from this collection. Returns the socket which
  // has been removed. Decrements this collection's refcount.

  static unsigned long scan_interval_sec;
  static unsigned long scan_interval_nsec;
  static unsigned      idle_scans;

private:
  int                  pd_refcount;
  omni_tracedmutex     pd_collection_lock;
  omni_tracedcondition pd_select_cond; // timedwait on if nothing to select

  // Absolute time at which we scan through the socket list choosing
  // the selectable ones.
  unsigned long        pd_abs_sec;
  unsigned long        pd_abs_nsec;

  // On platforms that support it, we use a pipe to wake up the select.
  int                  pd_pipe_read;
  int                  pd_pipe_write;
  CORBA::Boolean       pd_pipe_full;

  // On platforms that support pipes, after scanning a while with no
  // activity, we poll / select with an infinite timeout to prevent
  // unnecessary processing.
  unsigned             pd_idle_count;

#ifdef USE_POLL
  // On platforms where we use poll(), we maintain a pre-allocated
  // array of pollfd structures and a parallel array of pointers to
  // SocketHolders. pd_pollfd_n is the number of populated pollfds.
  // pd_pollfd_len is the current length of both arrays.
  struct pollfd*       pd_pollfds;
  SocketHolder**       pd_pollsockets;
  unsigned             pd_pollfd_n;
  unsigned             pd_pollfd_len;

  void growPollLists();
  // Expand the pd_pollfds and pd_pollsockets to fit more values.
  
#else
  // On platforms using select(), we maintain an fd_set and the
  // highest socket number set in it plus 1.
  fd_set               pd_fd_set;
  int                  pd_fd_set_n;
#endif

  // Linked list of registered sockets.
  SocketHolder*        pd_collection;
  CORBA::Boolean       pd_changed;

  friend class SocketHolder;
};

OMNI_NAMESPACE_END(omni)

#endif // __SOCKETCOLLECTION_H__
