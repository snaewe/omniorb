// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpSocketMTfactory.cc      Created on: 18/3/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//	Implementation of the Strand using TCP/IP and BSD socket interface
//	

/*
  $Log$
  Revision 1.31  2000/08/18 14:09:12  dpg1
  Merge from omni3_develop for 3.0.1 release.

  Revision 1.30  2000/07/13 15:25:54  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.22.6.18  2000/08/17 15:37:52  sll
  Merged RTEMS port.

  Revision 1.22.6.17  2000/08/10 10:10:56  sll
  For those platforms which cannot be unblocked from a recv() by a
  shutdown(), now do poll() or select() for both incoming and outgoing
  strands.  This is necessary especialy for win32 or else the server side
  socket will not shutdown until the client side close the socket. This
  wasn't done previously as it was thought that shutdown() does have an
  effect on recv() if this is a passive socket. This turns out to be wrong.

  Revision 1.22.6.16  2000/08/08 15:22:17  sll
  In realConnect(), now checks the exception fd_set in the select() call for an
  error condition. This is necessary because win32 differs from other
  platforms in that it reports a connect() failure via the exception fd_set.

  Revision 1.22.6.15  2000/08/07 10:34:19  sll
  Fixed bug with Win32 tcp socket being left in non-blocking state.

  Revision 1.22.6.14  2000/07/03 15:38:20  dpg1
  Support for FreeBSD 4.0. FreeBSD 3.2 hopefully works too.

  Revision 1.22.6.13  2000/06/27 15:18:15  sll
  On some platforms, e.g. HPUX 11 and Windows NT, shutdown() does not unblock
  a thread in recv() when the socket is of the active type, i.e.  a connect()
  was called on the socket previously.  This is now fixed by using select or
  poll before calling into recv on these platforms.  This is only done when
  the strand is outgoing. shutdown() seems to work fine on incoming strands.

  connect() is now non-blocking, except on OpenVMS. Previously, this is only
  done on Solaris. The effect is that client call timeout also works even
  when a connection cannot be established and the kernel takes a long time to
  report this as an error.

  Revision 1.22.6.12  2000/06/22 10:37:51  dpg1
  Transport code now throws omniConnectionBroken exception rather than
  CORBA::COMM_FAILURE when things go wrong. This allows the invocation
  code to distinguish between transport problems and COMM_FAILURES
  propagated from the server side.

  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.22.6.11  2000/03/16 14:28:34  djr
  Fixed signed/unsigned comparison warning.

  Revision 1.22.6.10  2000/01/28 15:57:10  djr
  Removed superflouous ref counting in Strand_iterator.
  Removed flags to indicate that Ropes and Strands are heap allocated.
  Improved allocation of client requests to strands.

  Revision 1.22.6.9  1999/12/06 14:02:47  djr
  Fixed bug in tcpSocketMTfactory.cc:dumpbuf().

  Revision 1.22.6.8  1999/10/27 18:20:40  sll
  Fixed the ctor of tcpSocketWorker so that if thread create fails, the
  exception raised by omnithread does not cause assertion failure in the
  ORB.

  Revision 1.22.6.7  1999/10/16 13:22:55  djr
  Changes to support compiling on MSVC.

  Revision 1.22.6.6  1999/10/14 16:22:18  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.22.6.5  1999/09/30 12:26:19  djr
  Minor change to remove compiler warning.

  Revision 1.22.6.4  1999/09/27 13:26:23  djr
  Updates to loggin to ensure prefix is always omniORB:

  Revision 1.22.6.3  1999/09/27 11:01:13  djr
  Modifications to logging.

  Revision 1.22.6.2  1999/09/24 15:01:39  djr
  Added module initialisers, and sll's new scavenger implementation.

  Revision 1.22.2.3  1999/09/22 19:56:52  sll
  Correct typo.

  Revision 1.22.2.2  1999/09/22 12:10:01  sll
  Merged port to SCO Unixware 7.

  Revision 1.22.2.1  1999/09/21 20:37:18  sll
  -Simplified the scavenger code and the mechanism in which connections
   are shutdown. Now only one scavenger thread scans both incoming
   and outgoing connections. A separate thread do the actual shutdown.
  -omniORB::scanGranularity() now takes only one argument as there is
   only one scan period parameter instead of 2.
  -Trace messages in various modules have been updated to use the logger
   class.
  -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                 -ORBscanIncomingPeriod.

  Revision 1.22  1999/08/30 16:54:24  sll
  Wait much less time in tcpSocketStrand::shutdown. Added trace message.

  Revision 1.21  1999/07/09 21:03:58  sll
  removeIncoming() now waits until all tcpSocketWorker threads have exited
  before returning.

  Revision 1.20  1999/06/28 17:38:01  sll
  Added packet dump routines in ll_send and ll_recv. Enabled at traceLevel 25.
  Added openvms change.

  Revision 1.19  1999/06/26 18:10:52  sll
  Use WSAGetLastError() on win32 to get the errno correctly.

  Revision 1.18  1999/05/31 14:03:06  sll
  Remove set_terminate workaround when compiled with SUN C++ 5.0.

  Revision 1.17  1999/05/22 17:40:11  sll
  Added #ifdef for ciao so that CCia would not complain about gnu/linux
  specifics.

  Revision 1.16  1999/03/11 16:25:57  djr
  Updated copyright notice

  Revision 1.15  1999/02/19 12:22:33  djr
  Fixed typo: sunsos -> sunos

  Revision 1.14  1998/11/09 10:56:57  sll
  Removed the use of the reserved keyword "export".

  Revision 1.13  1998/09/23 15:31:15  sll
  Previously, tcpSocketStrand::shutdown sends an incomplete GIOP
  CloseConnection message (the message length field is missing). Fixed.

  Revision 1.12  1998/09/23 08:48:34  sll
  Use config variable omniORB::maxTcpConnectionPerServer to determine the
  maximum number of outgoing per tcpSocketOutgoingRope.

  Revision 1.11  1998/08/14 13:54:38  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.10  1998/06/29 17:13:30  sll
  Fixed Solaris specific code in realConnect. Now switch the socket back
  to blocking mode after connect() until all circumstance.

  Revision 1.9  1998/04/22 16:39:50  sll
  Added try-catch loop to guard against exception raised by the thread library
  when it cannot create a new thread for tcpSocketWorker.

  Revision 1.8  1998/04/08 16:06:49  sll
  Added support for Reliant UNIX 5.43

  Revision 1.7  1998/04/07 19:39:40  sll
  Replace cerr with omniORB::log.

  Revision 1.6  1998/03/19 19:53:14  sll
  Delay connect to the remote address space until the first send or recv.
  Previously, connect was made inside the ctor of tcpSocketStrand.

  Revision 1.5  1998/03/04 14:44:36  sll
  Updated to use omniORB::giopServerThreadWrapper.

// Revision 1.4  1998/01/20  17:32:38  sll
// Added support for OpenVMS.
//
  Revision 1.3  1997/12/18 17:27:39  sll
  Updated to work under glibc-2.0.

  Revision 1.2  1997/12/12 18:44:11  sll
  Added call to gateKeeper.

  Revision 1.1  1997/12/09 18:43:18  sll
  Initial revision

  */

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <tcpSocket.h>
#include <exceptiondefs.h>

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//           Platform dependent includes and macros                       //
//                                                                        //
//  The ORB uses shutdown(2) to asynchronously notify a thread that is    //
//  blocking on receiving data from a socket to give up on the wait.      //
//  On some platforms, the shutdown call only has an effect on subsequent //
//  recv() but not the ones that are in progress. For these platforms, we //
//  have to block on a timeout so that the shutdown call will have a      //
//  chance to notify the blocking thread.                                 //
//  We can either use poll(2) or select(2), define either of the          //
//  following macro to choose one of them iff the platform exhibits the   //
//  above behaviour.                                                      //
//    #define USE_POLL_ON_RECV                                            //
//    #define USE_SELECT_ON_RECV                                          //
//                                                                        //
//  To put an upper bound on the time to wait for a connect() to succeed, //
//  define this macro to enable non-blocking connect. Make sure that      //
//  the platform supports this.                                           //
//    #define USE_NONBLOCKING_CONNECT                                     //
//
//  The maximum number of seconds to wait for a connect() to succeed:
#     define MAX_CONNECTION_WAIT   30
////////////////////////////////////////////////////////////////////////////

#if defined(__WIN32__)

#include <winsock.h>
#include <sys/types.h>

#define RC_INADDR_NONE     INADDR_NONE
#define RC_INVALID_SOCKET  INVALID_SOCKET
#define RC_SOCKET_ERROR    SOCKET_ERROR
#define INETSOCKET         PF_INET
#define CLOSESOCKET(sock)  closesocket(sock)
#define SHUTDOWNSOCKET(sock) ::shutdown(sock,2)
#define USE_SELECT_ON_RECV
#define USE_NONBLOCKING_CONNECT

#else

#include <sys/time.h>

#if !defined(__linux__) || !defined(__CIAO__)
#include <sys/socket.h>
#else
// This bit of ugly work around is to hide away the non ansi
// syntax in the headers that causes CCia to complain loudly.
#  ifndef __STRICT_ANSI__
#    define __STRICT_ANSI__
#    include <sys/socket.h>
#    undef  __STRICT_ANSI__
#  else
#    include <sys/socket.h>
#  endif
#endif

#if defined (__uw7__)
#ifdef shutdown
#undef shutdown
#endif
#endif

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#include <sys/types.h>
#endif


#if defined(__hpux__)
#include <poll.h>
#define USE_POLL_ON_RECV
#endif

#if !defined(__VMS)
#include <fcntl.h>
#define USE_NONBLOCKING_CONNECT
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
#endif

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>

#include <libcWrapper.h>
#include <gatekeeper.h>


#if defined(NEED_GETHOSTNAME_PROTOTYPE)
extern "C" int gethostname(char *name, int namelen);
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
extern "C" 
int select (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *tv);
#endif

#define PTRACE(prefix,message)  \
  omniORB::logs(15, "tcpSocketMTfactory " prefix ": " message)


class tcpSocketRendezvouser : public omni_thread {
public:
  tcpSocketRendezvouser(tcpSocketIncomingRope *r,
			tcpSocketMTincomingFactory *f) : 
        omni_thread(r), pd_factory(f)
    {
      start_undetached();
    }
  virtual ~tcpSocketRendezvouser() { }
  virtual void* run_undetached(void *arg);

private:
  tcpSocketMTincomingFactory* pd_factory;
  tcpSocketRendezvouser();
};

class tcpSocketWorker : public omni_thread {
public:
  tcpSocketWorker(tcpSocketStrand* s, tcpSocketMTincomingFactory* f) : 
          omni_thread(s), pd_factory(f), pd_sync(s,0,0) 
    {
      start();
      s->decrRefCount();
    }
  virtual ~tcpSocketWorker() { 
    omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
    OMNIORB_ASSERT(pd_factory->pd_shutdown_nthreads != 0);
    if (pd_factory->pd_shutdown_nthreads > 0) {
      pd_factory->pd_shutdown_nthreads--;
    }
    else {
      pd_factory->pd_shutdown_nthreads++;
      pd_factory->pd_shutdown_cond.signal();
    }
  }
  virtual void run(void *arg);
  static void _realRun(void* arg);

private:
  tcpSocketMTincomingFactory* pd_factory;
  Strand::Sync    pd_sync;
};

/////////////////////////////////////////////////////////////////////////////

tcpSocketMTincomingFactory::~tcpSocketMTincomingFactory()  {}


tcpSocketMTincomingFactory::tcpSocketMTincomingFactory() 
      : pd_state(IDLE), pd_shutdown_cond(&pd_shutdown_lock),
        pd_shutdown_nthreads(0)
{
  tcpSocketFactoryType::init();
  
}
 
CORBA::Boolean
tcpSocketMTincomingFactory::isIncoming(Endpoint* addr) const
{
  Rope_iterator next_rope(&pd_anchor);
  Rope* r;
  while ((r = next_rope()))
    {
      if (r->this_is(addr))
	return 1;
    }
  return 0;
}

void
tcpSocketMTincomingFactory::instantiateIncoming(Endpoint* addr,
						CORBA::Boolean exportflag)
{
  tcpSocketEndpoint* te = tcpSocketEndpoint::castup(addr);
  if (!te)
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "wrong endpoint argument type");

  Rope_iterator rope_lock(&pd_anchor);

  if (pd_state == ZOMBIE) {
    throw omniORB::fatalException(__FILE__,__LINE__,
			    "cannot instantiate incoming in ZOMBIE state");
  }

  tcpSocketIncomingRope* r = new tcpSocketIncomingRope(this,0,te,exportflag);
  r->incrRefCount(1);

  if (pd_state == ACTIVE) {
    r->rendezvouser = new tcpSocketRendezvouser(r,this);
  }
}

void
tcpSocketMTincomingFactory::startIncoming()
{
  Rope_iterator next_rope(&pd_anchor);
  tcpSocketIncomingRope* r;

  switch (pd_state) {
  case IDLE:
    try {
      pd_state = ACTIVE;
      while ((r = (tcpSocketIncomingRope*)next_rope())) {
	if (r->pd_shutdown == tcpSocketIncomingRope::NO_THREAD) {
	  r->pd_shutdown = tcpSocketIncomingRope::ACTIVE;
	  r->rendezvouser = new tcpSocketRendezvouser(r,this);
	}
      }
    }
    catch(...) { }
    break;
  default:
    break;
  }
}

void
tcpSocketMTincomingFactory::stopIncoming()
{
  Rope_iterator next_rope(&pd_anchor);
  tcpSocketIncomingRope* r;

  switch (pd_state) {
  case ACTIVE:
    try {
      while ((r = (tcpSocketIncomingRope*)next_rope())) {
	r->cancelThreads();
      }
      pd_state = IDLE;
    }
    catch(...) {}
    break;
  default:
    break;
  }
}

void
tcpSocketMTincomingFactory::removeIncoming()
{
  {
    Rope_iterator next_rope(&pd_anchor);
    tcpSocketIncomingRope* r;

    switch (pd_state) {
    case ACTIVE:
      try {
	while ((r = (tcpSocketIncomingRope*)next_rope())) {
	  if (pd_state == ACTIVE) {
	    r->cancelThreads();
	  }
	  if (r->pd_shutdown != tcpSocketIncomingRope::NO_THREAD) {
	    // rendezvouser has not been shutdown properly
	    continue;
	  }
	}
      }
      catch(...) {}
    case IDLE:
      pd_state = ZOMBIE;
      break;
    default:
      return;
    }
  }
  {
    omni_mutex_lock sync(pd_shutdown_lock);
    while (pd_shutdown_nthreads != 0) {
      if (pd_shutdown_nthreads > 0) {
	pd_shutdown_nthreads = -pd_shutdown_nthreads;
      }
      PTRACE("removeIncoming","blocks waiting for worker threads to exit");
      pd_shutdown_cond.wait();
    }
  }
  {
    Rope_iterator next_rope(&pd_anchor);
    tcpSocketIncomingRope* r;

    try {
      while ((r = (tcpSocketIncomingRope*)next_rope())) {
	if (r->pd_shutdown != tcpSocketIncomingRope::NO_THREAD) {
	  // rendezvouser has not been shutdown properly
	  continue;
	}
	if (r->pd_rendezvous != RC_INVALID_SOCKET) {
	  CLOSESOCKET(r->pd_rendezvous);
	  r->pd_rendezvous = RC_INVALID_SOCKET;
	  r->decrRefCount(1);
	}
      }
    }
    catch(...) {}
  }
  PTRACE("removeIncoming","Done");
}

Rope*
tcpSocketMTincomingFactory::findIncoming(Endpoint* addr) const
{
  tcpSocketEndpoint* te = tcpSocketEndpoint::castup(addr);
  if (!te) return 0;

  Rope_iterator next_rope(&pd_anchor);
  Rope* r;
  while ((r = next_rope()))
    {
      if (r->this_is(addr)) {
	r->incrRefCount(1);
	return r;
      }
    }
  return 0;
}

void 
tcpSocketMTincomingFactory::getIncomingIOPprofiles(const CORBA::Octet* objkey,
						   const size_t objkeysize,
			      IOP::TaggedProfileList& profilelist) const
{
  Rope_iterator next_rope(&pd_anchor);
  tcpSocketIncomingRope* r;
  while ((r = (tcpSocketIncomingRope*) next_rope()) && r->pd_export) {
    CORBA::ULong index = profilelist.length();
    profilelist.length(index+1);
    tcpSocketFactoryType::singleton->encodeIOPprofile(r->me,objkey,objkeysize,
						      profilelist[index]);
  }
}

tcpSocketIncomingRope::tcpSocketIncomingRope(tcpSocketMTincomingFactory* f,
					     unsigned int maxStrands,
					     tcpSocketEndpoint *e,
					     CORBA::Boolean exportflag)
  : Rope(f->anchor(),maxStrands), pd_export(exportflag), 
    pd_shutdown(NO_THREAD), rendezvouser(0)
{
  struct sockaddr_in myaddr;

  // For the moment, we do not impose a restriction on the maximum
  // no. of strands that can be accepted. In other words, <maxStrands> is 
  // ignored.

  if ((pd_rendezvous = socket(INETSOCKET,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
# ifndef __WIN32__
    OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
# else
    OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),CORBA::COMPLETED_NO);
# endif
  }
  myaddr.sin_family = INETSOCKET;
  myaddr.sin_addr.s_addr = INADDR_ANY;
  myaddr.sin_port = htons(e->port());
  
  if (e->port()) {
    int valtrue = 1;
    if (setsockopt(pd_rendezvous,SOL_SOCKET,
		   SO_REUSEADDR,(char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR)
      {
	CLOSESOCKET(pd_rendezvous);
#     ifndef __WIN32__
	OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
#     else
	OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),
					CORBA::COMPLETED_NO);
#     endif
      }
  }

  if (bind(pd_rendezvous,(struct sockaddr *)&myaddr,
	   sizeof(struct sockaddr_in)) == RC_SOCKET_ERROR) 
  {
    CLOSESOCKET(pd_rendezvous);
# ifndef __WIN32__
    OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
# else
    OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),CORBA::COMPLETED_NO);
# endif
  }

  // Make it a passive socket
  if (listen(pd_rendezvous,5) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_rendezvous);
# ifndef __WIN32__
    OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
# else
    OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),CORBA::COMPLETED_NO);
# endif
  }
  
  {
# if (defined(__GLIBC__) && __GLIBC__ >= 2) || (defined(__freebsd__) && __OSVERSION__ >= 4)
    // GNU C library uses socklen_t * instead of int* in getsockname().
    // This is suppose to be compatible with the upcoming POSIX standard.
    // FreeBSD 4.0 uses it too.
    socklen_t l;
# elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
    size_t l;
# else
    int l;
# endif



    l = sizeof(struct sockaddr_in);
    if (getsockname(pd_rendezvous,
		    (struct sockaddr *)&myaddr,&l) == RC_SOCKET_ERROR) {
      CLOSESOCKET(pd_rendezvous);
# ifndef __WIN32__
      OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
# else
      OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),CORBA::COMPLETED_NO);
# endif
    }

    e->port(ntohs(myaddr.sin_port));

    if (e->host() == 0 || strlen((const char*)e->host()) == 0) {

      char self[64];
      if (gethostname(&self[0],64) == RC_SOCKET_ERROR) {
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "Cannot get the name of this host");
      }

      LibcWrapper::hostent_var h;
      int rc;

      if (LibcWrapper::gethostbyname(self,h,rc) < 0) {
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "Cannot get the address of this host");
      }
      memcpy((void *)&myaddr.sin_addr,
	     (void *)h.hostent()->h_addr_list[0],
	     sizeof(myaddr.sin_addr));
      char ipaddr[16];
      // To prevent purify from generating UMR warnings, use the following temp
      // variables to store the IP address fields.
      int ip1 = (int)((ntohl(myaddr.sin_addr.s_addr) & 0xff000000) >> 24);
      int ip2 = (int)((ntohl(myaddr.sin_addr.s_addr) & 0x00ff0000) >> 16);
      int ip3 = (int)((ntohl(myaddr.sin_addr.s_addr) & 0x0000ff00) >> 8);
      int ip4 = (int)(ntohl(myaddr.sin_addr.s_addr) & 0x000000ff);
      sprintf(ipaddr,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
      e->host((const CORBA::Char *) ipaddr);

    }
    else {
      // The caller has already specified the host name, we are not going to
      // override it here. However, it may be possible that the host name does
      // not resolve to one of the IP addresses that identified the network
      // interfaces of this machine. There is no way to guard against this
      // mistake.
      // Do nothing here.
    }
  }

  me = new tcpSocketEndpoint(e);
}

tcpSocketIncomingRope::~tcpSocketIncomingRope()
{
  PTRACE("~tcpSocketIncomingRope","called");
  if (me) {
    delete me;
    me = 0;
  }
  if (pd_rendezvous != RC_INVALID_SOCKET) {
    CLOSESOCKET(pd_rendezvous);
    pd_rendezvous = RC_INVALID_SOCKET;
  }
}

void
tcpSocketIncomingRope::cancelThreads()
{
  if (rendezvouser) {
      pd_lock.lock();
      pd_shutdown = SHUTDOWN;
      pd_lock.unlock();
  }

  CutStrands();
  
  if (rendezvouser) {
    // Unblock the rendezvouser from its accept() call.
    // Cannot shutdown the rendezvous socket because we want to keep it open
    // and could be served later by another rendezvouser.
    // 
    // Instead, unblock the rendezvouser by making a connect() call.

    struct sockaddr_in myaddr;

    {
      Endpoint* e = 0;
      this_is(e);
      tcpSocketEndpoint* te = (tcpSocketEndpoint*)e;
      myaddr.sin_family = INETSOCKET;
      myaddr.sin_port   = htons(te->port());
      if (LibcWrapper::isipaddr((char*)te->host())) 
	{
	  CORBA::ULong ip_p = inet_addr((char*) te->host());
	  memcpy((void*) &myaddr.sin_addr, (void*) &ip_p, sizeof(myaddr.sin_addr));
	}
      else
	{
	  LibcWrapper::hostent_var h;
	  int  rc;
	  LibcWrapper::gethostbyname((char*)te->host(),h,rc);
	  memcpy((void*)&myaddr.sin_addr,
		 (void*)h.hostent()->h_addr_list[0],
		 sizeof(myaddr.sin_addr));
	}
      delete te;
    }

    tcpSocketHandle_t  tmp_sock;

    if ((tmp_sock = socket(INETSOCKET,SOCK_STREAM,0)) == RC_INVALID_SOCKET) 
      {
	// If we cannot create a socket, we cannot shutdown the rendezvouser
	omniORB::logs(1,
	      "tcpSocketIncomingRope::cancelThreads() cannot create\n"
	      " a socket to connect to the rendezvous socket.  The\n"
	      " rendezvous thread may or may not have exited.  If this is\n"
	      " a temporary resource exhaustion, try again later.");
	return;
      }
    if (connect(tmp_sock,(struct sockaddr *)&myaddr,
		sizeof(struct sockaddr_in)) == RC_SOCKET_ERROR) 
      {
	// OK, so we cannot connect to the rendezvouser, it would have
	// unblock from accept() anyway. That is fine as well.
      }
    else 
      {
	// Now we have unblock the rendezvouser, just close the socket.
	CLOSESOCKET(tmp_sock);
      }

    // Now we have got the rendezvouser's attention. We acknowlege that
    // we have seen it and instructs the rendezvouser to exit.
    pd_lock.lock();
    pd_shutdown = NO_THREAD;
    pd_lock.unlock();

    PTRACE("stopIncoming","Waiting for tcpSocketMT Rendezvouser to exit...");
    rendezvouser->join(0); // Wait till the rendezvouser to come back
    PTRACE("stopIncoming","tcpSocketMT Rendezvouser has exited");
    rendezvouser = 0;
  }
}

Strand *
tcpSocketIncomingRope::newStrand()
{
  throw omniORB::fatalException(__FILE__,__LINE__,
				"newStrand should not be called.");
# ifdef NEED_DUMMY_RETURN
  return 0; // dummy return to keep some compilers happy
# endif
}

/////////////////////////////////////////////////////////////////////////////

tcpSocketMToutgoingFactory::~tcpSocketMToutgoingFactory()  {}


tcpSocketMToutgoingFactory::tcpSocketMToutgoingFactory()
{
  tcpSocketFactoryType::init();
}

CORBA::Boolean
tcpSocketMToutgoingFactory::isOutgoing(Endpoint* addr) const
{
  Rope_iterator next_rope(&pd_anchor);
  Rope* r;
  while ((r = next_rope()))
    {
      if (r->remote_is(addr))
	return 1;
    }
  return 0;
}

Rope*
tcpSocketMToutgoingFactory::findOrCreateOutgoing(Endpoint* addr)
{
  tcpSocketEndpoint* te = tcpSocketEndpoint::castup(addr);
  if (!te) return 0;

  Rope_iterator next_rope(&pd_anchor);
  Rope* r;
  while ((r = next_rope()))
    {
      if (r->remote_is(addr)) {
	r->incrRefCount(1);
	return r;
      }
    }
  r = new tcpSocketOutgoingRope(this,omniORB::maxTcpConnectionPerServer,te);
  r->incrRefCount(1);
  return r;
}


tcpSocketOutgoingRope::tcpSocketOutgoingRope(tcpSocketMToutgoingFactory* f,
					     unsigned int maxStrands,
					     tcpSocketEndpoint *e)
  : Rope(f->anchor(),maxStrands)
{
  remote = new tcpSocketEndpoint(e);
}

tcpSocketOutgoingRope::~tcpSocketOutgoingRope()
{
  PTRACE("~tcpSocketOutgoingRope","called");
  if (remote) {
    delete remote;
    remote = 0;
  }
}

Strand *
tcpSocketOutgoingRope::newStrand()
{
  return new tcpSocketStrand(this,remote);
}


/////////////////////////////////////////////////////////////////////////////



// Size of transmit and receive buffers
const 
unsigned int 
tcpSocketStrand::buffer_size = 8192 + (int)omni::max_alignment;


static tcpSocketHandle_t realConnect(tcpSocketEndpoint* r,
				     tcpSocketStrand* s);


tcpSocketStrand::tcpSocketStrand(tcpSocketOutgoingRope *rope,
				 tcpSocketEndpoint   *r)
  : reliableStreamStrand(tcpSocketStrand::buffer_size,rope),
    pd_send_giop_closeConnection(0), pd_delay_connect(0)
{
  // Do not try to connect to the remote host in this ctor.
  // This is to avoid holding the mutex on rope->pd_lock while the connect
  // is in progress. Holding the mutex for an extended period is bad as this 
  // can have ***serious*** side effect. 
  // One immediate consequence of holding the rope->pd_lock is that the
  // outScavenger will be blocked on rope->pd_lock when it is scanning
  // for idle strands. This in turn blockout any thread trying to lock
  // rope->pd_anchor->pd_lock. This is really bad because no new rope
  // can be added to the anchor.


  pd_socket = RC_INVALID_SOCKET;
  pd_delay_connect = new tcpSocketEndpoint(r);
  // Do the connect on first call to ll_recv or ll_send.
}

tcpSocketStrand::tcpSocketStrand(tcpSocketIncomingRope *r,
				 tcpSocketHandle_t sock)
  : reliableStreamStrand(tcpSocketStrand::buffer_size,r),
    pd_socket(sock), pd_send_giop_closeConnection(1), pd_delay_connect(0)
{
}


tcpSocketStrand::~tcpSocketStrand() 
{
  if (omniORB::trace(5)) {
    omniORB::logger l;
    l << "tcpSocketStrand::~Strand() close socket no. " << pd_socket << '\n';
  }
  if (pd_socket != RC_INVALID_SOCKET)
    CLOSESOCKET(pd_socket);
  pd_socket = RC_INVALID_SOCKET;
  if (pd_delay_connect)
    delete pd_delay_connect;
  pd_delay_connect = 0;
}


static inline char printable_char(char c) {
  return (c < 32 || c > 126) ? '.' : c;
}


static void dumpbuf(unsigned char* buf, size_t sz)
{
  static omni_mutex lock;
  omni_mutex_lock sync(lock);
  unsigned i;
  char row[80];

  // Do complete rows of 16 octets.
  while( sz >= 16u ) {
    sprintf(row, "%02x%02x %02x%02x %02x%02x %02x%02x "
	         "%02x%02x %02x%02x %02x%02x %02x%02x ",
            (int) buf[0], (int) buf[1], (int) buf[2], (int) buf[3],
            (int) buf[4], (int) buf[5], (int) buf[6], (int) buf[7],
            (int) buf[8], (int) buf[9], (int) buf[10], (int) buf[11],
            (int) buf[12], (int) buf[13], (int) buf[14], (int) buf[15]);
    fprintf(stderr, "%s", row);
    char* p = row;
    for( i = 0u; i < 16u; i++ )  *p++ = printable_char(*buf++);
    *p++ = '\0';
    fprintf(stderr,"%s\n", row);
    sz -= 16u;
  }

  if( sz ) {
    // The final part-row.
    for( i = 0u; i < sz; i++ )
      fprintf(stderr, (i & 1u) ? "%02x ":"%02x", (int) buf[i]);
    for( ; i < 16u; i++ )
      fprintf(stderr, (i & 1u) ? "   ":"  ");
    for( i = 0u; i < sz; i++ )
      fprintf(stderr, "%c", printable_char(buf[i]));
    fprintf(stderr,"\n");
  }
}


size_t
tcpSocketStrand::ll_recv(void* buf, size_t sz)
{
  if (pd_delay_connect) {
    // We have not connect to the remote host yet. Do the connect now.
    // Note: May block on connect for sometime if the remote host is down
    //
    if ((pd_socket = realConnect(pd_delay_connect,this)) == RC_INVALID_SOCKET) {
      _setStrandIsDying();
#   ifndef __WIN32__
      OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
#   else
      OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),CORBA::COMPLETED_NO);
#   endif
    }
    delete pd_delay_connect;
    pd_delay_connect = 0;
  }

  int rx;

#ifdef __VMS
  // OpenVMS socket library cannot handle more than 64K buffer.
  if (sz > 65535) sz = 65536-8;
#endif

  while (1) {

# if defined(USE_POLL_ON_RECV)
    struct pollfd fds;
    fds.fd = pd_socket;
    fds.events = POLLIN;

    while (!(rx = poll(&fds,1,omniORB::scanGranularity()*1000) > 0)) {
      if (rx == RC_SOCKET_ERROR && errno != EINTR) 
	break;
    }

# elif defined(USE_SELECT_ON_RECV)
    do {
      fd_set fds, efds;
      FD_ZERO(&fds);
      FD_ZERO(&efds);
      FD_SET(pd_socket,&fds);
      FD_SET(pd_socket,&efds);
      struct timeval t;
      t.tv_sec = omniORB::scanGranularity();
      t.tv_usec = 0;
      rx = select(pd_socket+1,&fds,0,&efds,&t);
#   ifndef __WIN32__
      if (rx == RC_SOCKET_ERROR && errno != EINTR)
	break;
#   else
      if (rx == RC_SOCKET_ERROR && ::WSAGetLastError() != WSAEINTR)
	break;

      // Unfortunately, select() in WIN32 does not return with an error even
      // when the socket has been shutdown. As a workaround, we
      // check also if the strand is dying. Notice that reading the
      // state of the strand is not synchronised. In the worst case,
      // we may wait for an extra omniORB::scanGranularity() period before
      // noticing that the strand is dying.
      if (_strandIsDying()) break;

#   endif
    } while (rx <= 0);
# endif
    
    if ((rx = ::recv(pd_socket,(char*)buf,sz,0)) == RC_SOCKET_ERROR) {
      if (errno == EINTR)
	continue;
      else
	{
	  _setStrandIsDying();
#       ifndef __WIN32__
	  OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
#       else
	  OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),
					  CORBA::COMPLETED_NO);
#       endif
	}
    }
    else
      if (rx == 0) {
	_setStrandIsDying();
#     ifndef __WIN32__
	OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
#     else
	OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),
					CORBA::COMPLETED_NO);
#     endif
      }
    break;
  }
  if (omniORB::trace(30)) {
      fprintf(stderr,"ll_recv: %d bytes\n",rx);
      ::dumpbuf((unsigned char*)buf,rx);
  }
  return (size_t)rx;
}

void
tcpSocketStrand::ll_send(void* buf,size_t sz) 
{
  if (pd_delay_connect) {
    // We have not connect to the remote host yet. Do the connect now.
    // Note: May block on connect for sometime if the remote host is down
    //
    if ((pd_socket = realConnect(pd_delay_connect,this)) == RC_INVALID_SOCKET) {
      _setStrandIsDying();
#   ifndef __WIN32__
      OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
#   else
      OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),CORBA::COMPLETED_NO);
#   endif
    }
    delete pd_delay_connect;
    pd_delay_connect = 0;
  }

  int tx;
  char *p = (char *)buf;

  if (omniORB::trace(30)) {
      fprintf(stderr,"ll_send: %d bytes\n",sz);
      ::dumpbuf((unsigned char*)buf,sz);
  }

  while (sz) {

    size_t ssz = sz;
#ifdef __VMS
  // OpenVMS socket library cannot handle more than 64K buffer.
  if (ssz > 65535) ssz = 65536-8;
#endif

    if ((tx = ::send(pd_socket,p,ssz,0)) == RC_SOCKET_ERROR) {
#   ifndef __WIN32__
      if (errno == EINTR)
	continue;
      else {
	_setStrandIsDying();
	OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
      }
#   else
      if (::WSAGetLastError() == WSAEINTR)
 	continue;
      else {
 	_setStrandIsDying();
	OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),
					CORBA::COMPLETED_MAYBE);
      }
#   endif
    }
    else
      if (tx == 0) {
	_setStrandIsDying();
#     ifndef __WIN32__
	OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
#     else
	OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),
					CORBA::COMPLETED_NO);
#     endif
      }
    sz -= tx;
    p += tx;
  }
  return;
}



void
tcpSocketStrand::real_shutdown()
{
  if (pd_send_giop_closeConnection)
    {
      // XXX This is a bit ugly. GIOP requires us to send a CloseConnection 
      // message before shutdown. Ideally, we do not want this abstraction to 
      // do any GIOP dependent stuff. If this a problem in future, we should 
      // perhap make it a parameter to decide whether or what to send on
      // shutdown.

      static char closeConnectionMessage[12] = {
	   'G','I','O','P',
	   1,0,
	   _OMNIORB_HOST_BYTE_ORDER_,
	   GIOP::CloseConnection,
	   0,0,0,0
      };

      size_t sz = sizeof(closeConnectionMessage);
      char* p = closeConnectionMessage;
      while (sz) {
	fd_set wrfds;
#     ifndef __CIAO__
	FD_ZERO(&wrfds);
	FD_SET(pd_socket,&wrfds);
#     endif
	struct timeval t = { 0,100000};
	int rc;
	if ((rc = select(pd_socket+1,0,&wrfds,0,&t)) <= 0) {
	  // Any timeout or error, we just don't border sending the message.
	  break;
	}
	int tx;
	if ((tx = ::send(pd_socket,p,sz,0)) == RC_SOCKET_ERROR) {
	  // Any error we just don't border sending anymore.
	  break;
	}
	sz -= tx;
	p += tx;
      }
    }
  _setStrandIsDying();
  SHUTDOWNSOCKET(pd_socket);
  if (omniORB::trace(15)) {
    omniORB::logger l;
    l << "tcpSocketStrand::real_shutdown() fd no. " << pd_socket << " Done\n";
  }
  return;
}

static
tcpSocketHandle_t
realConnect(tcpSocketEndpoint* r,tcpSocketStrand* s)
{
  struct sockaddr_in raddr;
  LibcWrapper::hostent_var h;
  int  rc;
  tcpSocketHandle_t sock;

  if (! LibcWrapper::isipaddr( (char*) r->host()))
    {
      if (LibcWrapper::gethostbyname((char *)r->host(),h,rc) < 0) 
	{
	  // XXX look at rc to decide what to do or if to give up what errno
	  // XXX to return EINVAL.
	  //
	  return RC_INVALID_SOCKET;
	}
      // We just pick the first address in the list, may be we should go
      // through the list and if possible pick the one that is on the same
      // subnet.
      memcpy((void*)&raddr.sin_addr,
	     (void*)h.hostent()->h_addr_list[0],
	     sizeof(raddr.sin_addr));
    }
  else
    {
      // The machine name is already an IP address
      CORBA::ULong ip_p;
      if ( (ip_p = inet_addr( (char*) r->host() )) == RC_INADDR_NONE)
	{
	  return RC_INVALID_SOCKET;
	}
      memcpy((void*) &raddr.sin_addr, (void*) &ip_p, sizeof(raddr.sin_addr));
    }

  raddr.sin_family = INETSOCKET;
  raddr.sin_port   = htons(r->port());

  if ((sock = socket(INETSOCKET,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return RC_INVALID_SOCKET;
  }

#if defined(USE_NONBLOCKING_CONNECT)
  // Use non-blocking connect.
# if !defined(__WIN32__)
  int fl = O_NONBLOCK;
  if (fcntl(sock,F_SETFL,fl) < RC_SOCKET_ERROR) {
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }
# else
  u_long v = 1;
  if (ioctlsocket(sock,FIONBIO,&v) == RC_SOCKET_ERROR) {
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }
# endif
  if (connect(sock,(struct sockaddr *)&raddr,
	      sizeof(struct sockaddr_in)) == RC_SOCKET_ERROR) 
  {
# ifndef __WIN32__
    if (errno != EINPROGRESS) {
      CLOSESOCKET(sock);
      return RC_INVALID_SOCKET;
    }
# else
    if (::WSAGetLastError() != WSAEWOULDBLOCK) {
      CLOSESOCKET(sock);
      return RC_INVALID_SOCKET;
    }
# endif

    int tremain = MAX_CONNECTION_WAIT;
    int rc;
    while (tremain) {

      fd_set wrfds, exfds;
# ifndef __CIAO__
      FD_ZERO(&wrfds);
      FD_ZERO(&exfds);
      FD_SET(sock,&wrfds);
      FD_SET(sock,&exfds);
# endif
      struct timeval t;
      int tselect = omniORB::scanGranularity();
      if (tselect > tremain) tselect = tremain;
      t.tv_sec = tselect;
      t.tv_usec = 0;
    again:
      rc = select(sock+1,0,&wrfds,&exfds,&t);

      if (rc == 0) {
	tremain -= tselect;
      }
      else if (rc == RC_SOCKET_ERROR) {
#   ifndef __WIN32__
	if (errno == EINTR)
	  continue;
#   else 
	if (::WSAGetLastError() == WSAEINTR)
	  continue;
#   endif
	else
	  break;
      }
      else {
	if (FD_ISSET(sock,&exfds)) {
	  rc = RC_SOCKET_ERROR;
	}
	break;
      }
      if (s->_strandIsDying()) {
	tremain = 0;
	break;
      }
    }
    if (rc == RC_SOCKET_ERROR || !tremain) {
      CLOSESOCKET(sock);
      return RC_INVALID_SOCKET;
    }
  }
  // Set the socket back to blocking
# if !defined(__WIN32__)
  fl = 0;
  if (fcntl(sock,F_SETFL,fl) == RC_SOCKET_ERROR) {
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }
# else
  v = 0;
  if (ioctlsocket(sock,FIONBIO,&v) == RC_SOCKET_ERROR) {
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }
# endif

#else
  if (connect(sock,(struct sockaddr *)&raddr,
	      sizeof(struct sockaddr_in)) == RC_SOCKET_ERROR) 
  {
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }
#endif


  return sock;
}

/////////////////////////////////////////////////////////////////////////////


#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x420
#include <signal.h>
static void abort()
{
  kill (getpid(),SIGABRT);
  while (1) {
    sleep(1000000);
  }
}

typedef void (*PFV)();
extern PFV set_terminate(PFV);
#endif
#endif

void*
tcpSocketRendezvouser::run_undetached(void *arg)
{
  tcpSocketIncomingRope* r = (tcpSocketIncomingRope*) arg;

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x420
  set_terminate(abort);
#endif
#endif
  PTRACE("Rendezvouser","start.");

  tcpSocketStrand *newSt = 0;
  tcpSocketWorker *newthr = 0;
  CORBA::Boolean   die = 0;

  while (r->pd_shutdown == tcpSocketIncomingRope::ACTIVE  && !die) {

    try {

      tcpSocketHandle_t new_sock;
      struct sockaddr_in raddr;
 
#if (defined(__GLIBC__) && __GLIBC__ >= 2) || (defined(__freebsd__) && __OSVERSION__ >= 4)
      // GNU C library uses socklen_t * instead of int* in accept ().
      // This is suppose to be compatible with the upcoming POSIX standard.
      // FreeBSD 4.0 uses it too.
      socklen_t l;
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
      size_t l;
#else
      int l;
#endif

      l = sizeof(struct sockaddr_in);

      PTRACE("Rendezvouser","block on accept()");

      if ((new_sock = ::accept(r->pd_rendezvous,(struct sockaddr *)&raddr,&l)) 
	                          == RC_INVALID_SOCKET) {
#     ifndef __WIN32__
	OMNIORB_THROW_CONNECTION_BROKEN(errno,CORBA::COMPLETED_NO);
#     else
	OMNIORB_THROW_CONNECTION_BROKEN(::WSAGetLastError(),
					CORBA::COMPLETED_NO);
#     endif
      }

      PTRACE("Rendezvouser","unblock from accept()");

      {
	omni_mutex_lock sync(r->pd_lock);

	if (r->pd_shutdown != tcpSocketIncomingRope::ACTIVE) {
	  // It has been indicated that this thread should stop
	  // accepting connection request.
	  CLOSESOCKET(new_sock);
	  continue;
	}

	newSt = new tcpSocketStrand(r,new_sock);
	newSt->incrRefCount(1);
      }

      PTRACE("Rendezvouser","accept new strand.");

      omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
      if (pd_factory->pd_shutdown_nthreads >= 0) {
	pd_factory->pd_shutdown_nthreads++;
      }
      else {
	pd_factory->pd_shutdown_nthreads--;
      }

      try {
	newthr = new tcpSocketWorker(newSt,pd_factory);
      }
      catch(...) {
	newthr = 0;
      }
      if (!newthr) {
	// Cannot create a new thread to serve the strand
	// We have no choice but to shutdown the strand.
	// The long term solutions are:  start multiplexing the new strand
	// and the rendezvous; close down idle connections; reasign
	// threads to strands; etc.
	newSt->decrRefCount();
	newSt->real_shutdown();

	omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
	OMNIORB_ASSERT(pd_factory->pd_shutdown_nthreads != 0);
	if (pd_factory->pd_shutdown_nthreads > 0) {
	  pd_factory->pd_shutdown_nthreads--;
	}
	else {
	  pd_factory->pd_shutdown_nthreads++;
	  pd_factory->pd_shutdown_cond.signal();
	}

      }
    }
    catch(omniConnectionBroken&) {
      // XXX accepts failed. The probable cause is that the number of
      //     file descriptors opened has exceeded the limit.
      //     On unix, the value of this limit can be set and get using the
      //              ulimit command.
      //     On NT, if this is part of a DLL, the limit is 256(?)
      //            else the limit is 16(?)
      // The following is a temporary fix, this thread just wait for a while
      // and tries again. Hopfully, some connections might be freed by then.
      PTRACE("Rendezvouser","accept fails. Too many file descriptors opened?");
      omni_thread::sleep(1,0);
      continue;
    }
    catch(omniORB::fatalException& ex) {
      if (omniORB::trace(0)) {
	omniORB::logger l;
	l << "You have caught an omniORB bug, details are as follows:\n" <<
	  " file: " << ex.file() << "\n"
	  " line: " << ex.line() << "\n"
	  " mesg: " << ex.errmsg() << "\n"
	  " tcpSocketMT Rendezvouser thread will not accept new connection.\n";
      }
      die = 1;
    }
    catch(...) {
      omniORB::logs(0,
       "Unexpected exception caught by tcpSocketMT Rendezvouser\n"
       " tcpSocketMT Rendezvouser thread will not accept new connection.");
      die = 1;
    }
    if (die && newSt) {
      newSt->decrRefCount();
      newSt->real_shutdown();
      if (!newthr) {
	omniORB::logs(5, "tcpSocketMT Rendezvouser thread cannot spawn a"
		      " new server thread.");
      }
    }
  }
  if (die) {
    // Something very wrong has happened, before we quit, we have to
    // make sure that a future call by another thread to
    // tcpSocketIncomingRope::cancelThreads() would not wait indefinitely
    // on this thread to response to its connect() request to the
    // rendezous socket. 
    // Shutdown (not close) the rendezvous socket to make sure that the
    // connect() in cancelThreads() would fail.
    SHUTDOWNSOCKET(r->pd_rendezvous);
  }

  while (r->pd_shutdown != tcpSocketIncomingRope::NO_THREAD) {

    // We keep on accepting connection requests but close the new sockets
    // immediately until the state of pd_shutdown changes to NO_THREAD.
    // This is to make sure that the thread calling cancelThreads()
    // will be unblocked from the connect() call.

    {
      fd_set rdfds;
#   ifndef __CIAO__
      FD_ZERO(&rdfds);
      FD_SET(r->pd_rendezvous,&rdfds);
#   endif
      struct timeval t = { 1,0};
      int rc;
      if ((rc = select(r->pd_rendezvous+1,&rdfds,0,0,&t)) <= 0) {
#     ifndef __WIN32__
	if (rc < 0 && errno != EINTR) {
	  die = 1;
	}
#     else
 	if (rc < 0 && ::WSAGetLastError() != WSAEINTR) {
	  die = 1;
	}
#     endif
	PTRACE("Rendezvouser","waiting on shutdown state to change to NO_THREAD.");
	continue;
      }
    }

    tcpSocketHandle_t new_sock;
    struct sockaddr_in raddr;
#if (defined(__GLIBC__) && __GLIBC__ >= 2) || (defined(__freebsd__) && __OSVERSION__ >= 4)
    // GNU C library uses socklen_t * instead of int* in accept ().
    // This is suppose to be compatible with the upcoming POSIX standard.
    // FreeBSD 4.0 uses it too.
    socklen_t l;
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
    size_t l;
#else
    int l;
#endif

    l = sizeof(struct sockaddr_in);
    if ((new_sock = ::accept(r->pd_rendezvous,(struct sockaddr *)&raddr,&l)) 
	      == RC_INVALID_SOCKET) 
      {
	die = 1;
	continue;
      }
    CLOSESOCKET(new_sock);
  }

  PTRACE("Rendezvouser","exit.");
  return 0;
}

void
tcpSocketWorker::run(void *arg)
{
  omniORB::giopServerThreadWrapper::
        getGiopServerThreadWrapper()->run(tcpSocketWorker::_realRun,arg);
  // the wrapper run() method will pass back control to tcpSocketWorker
  // by calling  _realRun(arg) when it is ready.
}

void
tcpSocketWorker::_realRun(void *arg)
{
  tcpSocketStrand* s = (tcpSocketStrand*)arg;

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x420
  set_terminate(abort);
#endif
#endif
  
  PTRACE("Worker","start.");

#ifndef _MSC_VER
  //??
  if ( !gateKeeper::checkConnect(s)) {
    s->real_shutdown();
  }
  else
#endif
    {
    while (1) {
      try {
	GIOP_S::dispatcher(s);
      }
      catch (omniConnectionBroken&) {
	PTRACE("Worker","#### Connection closed.");
	break;
      }
      catch(const omniORB::fatalException& ex) {
	if( omniORB::trace(0) ) {
	  omniORB::logger l;
	  l << "You have caught an omniORB bug, details are as follows:\n"
	    " file: " << ex.file() << "\n"
	    " line: " << ex.line() << "\n"
	    " mesg: " << ex.errmsg() << "\n";
	}
	break;
      }
      catch (...) {
	omniORB::logs(0, "An exception has occured and was caught by"
		      " tcpSocketMT Worker thread.");
	break;
      }
    }
  }

  PTRACE("Worker","exit.");
}
