// -*- Mode: C++; -*-
//                            Package   : omniORB
// SocketCollection.cc        Created on: 23 Jul 2001
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
  Revision 1.1.2.14  2003/02/17 10:39:52  dgrisby
  Fix inevitable Windows problem.

  Revision 1.1.2.13  2003/02/17 01:46:23  dgrisby
  Pipe to kick select thread (on Unix).

  Revision 1.1.2.12  2003/01/28 12:17:09  dgrisby
  Bug with Select() ignoring data in buffer indications.

  Revision 1.1.2.11  2002/10/14 15:27:41  dgrisby
  Typo in fcntl error check.

  Revision 1.1.2.10  2002/08/21 06:23:15  dgrisby
  Properly clean up bidir connections and ropes. Other small tweaks.

  Revision 1.1.2.9  2002/03/18 16:50:18  dpg1
  New threadPoolWatchConnection parameter.

  Revision 1.1.2.8  2002/03/14 12:21:49  dpg1
  Undo accidental scavenger period change, remove invalid assertion.

  Revision 1.1.2.7  2002/03/13 16:05:38  dpg1
  Transport shutdown fixes. Reference count SocketCollections to avoid
  connections using them after they are deleted. Properly close
  connections when in thread pool mode.

  Revision 1.1.2.6  2002/02/26 14:06:45  dpg1
  Recent changes broke Windows.

  Revision 1.1.2.5  2002/02/13 16:02:38  dpg1
  Stability fixes thanks to Bastiaan Bakker, plus threading
  optimisations inspired by investigating Bastiaan's bug reports.

  Revision 1.1.2.4  2001/08/24 15:56:44  sll
  Fixed code which made the wrong assumption about the semantics of
  do { ...; continue; } while(0)

  Revision 1.1.2.3  2001/08/02 13:00:53  sll
  Do not use select(0,0,0,0,&timeout), it doesn't work on win32.

  Revision 1.1.2.2  2001/08/01 15:56:07  sll
  Workaround MSVC++ bug. It generates wrong code with FD_ISSET and FD_SET
  under certain conditions.

  Revision 1.1.2.1  2001/07/31 16:16:26  sll
  New transport interface to support the monitoring of active connections.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <SocketCollection.h>

#if defined(__vxWorks__)
#  include "pipeDrv.h"
#  include "iostream.h"
#endif


OMNI_NAMESPACE_BEGIN(omni)

#define GDB_DEBUG

/////////////////////////////////////////////////////////////////////////
void
SocketSetTimeOut(unsigned long abs_sec,
		 unsigned long abs_nsec,struct timeval& t)
{
  unsigned long now_sec, now_nsec;
  omni_thread::get_time(&now_sec,&now_nsec);

  if ((abs_sec <= now_sec) && ((abs_sec < now_sec) || (abs_nsec < now_nsec))) {
    t.tv_sec = t.tv_usec = 0;
  }
  else {
    t.tv_sec = abs_sec - now_sec;
    if (abs_nsec >= now_nsec) {
      t.tv_usec = (abs_nsec - now_nsec) / 1000;
    }
    else {
      t.tv_usec = (1000000000 + abs_nsec - now_nsec) / 1000;
      t.tv_sec -= 1;
    }
  }
}

/////////////////////////////////////////////////////////////////////////
int
SocketSetnonblocking(SocketHandle_t sock) {
# if !defined(__WIN32__)
  int fl = O_NONBLOCK;
  if (fcntl(sock,F_SETFL,fl) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# elif defined(__vxWorks__)
  int fl = TRUE;
  if (ioctl(sock, FIONBIO, (int)&fl) == ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# else
  u_long v = 1;
  if (ioctlsocket(sock,FIONBIO,&v) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# endif
}

/////////////////////////////////////////////////////////////////////////
int
SocketSetblocking(SocketHandle_t sock) {
# if !defined(__WIN32__)
  int fl = 0;
  if (fcntl(sock,F_SETFL,fl) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# elif defined(__vxWorks__)
  int fl = FALSE;
  if (ioctl(sock, FIONBIO, (int)&fl) == ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# else
  u_long v = 0;
  if (ioctlsocket(sock,FIONBIO,&v) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# endif
}


/////////////////////////////////////////////////////////////////////////
unsigned long SocketCollection::scan_interval_sec  = 0;
unsigned long SocketCollection::scan_interval_nsec = 50*1000*1000;
CORBA::ULong  SocketCollection::hashsize           = 103;

/////////////////////////////////////////////////////////////////////////
SocketCollection::SocketCollection() :
  pd_n_fdset_1(0), pd_n_fdset_2(0), pd_n_fdset_dib(0),
  pd_select_cond(&pd_fdset_lock),
  pd_abs_sec(0), pd_abs_nsec(0),
  pd_pipe_read(-1), pd_pipe_write(-1), pd_pipe_full(0),
  pd_refcount(1)
{
  FD_ZERO(&pd_fdset_1);
  FD_ZERO(&pd_fdset_2);
  FD_ZERO(&pd_fdset_dib);

#ifdef UnixArchitecture
#  ifdef __vxWorks__
    if (pipeDrv() == OK) {
      if (pipeDevCreate("/pipe/SocketCollection",10,sizeof(int)) == OK) {
	pd_pipe_read = pd_pipe_write = open("/pipe/SocketCollection",
					    O_RDWR,0);
      }
    }
    if (pd_pipe_read <= 0) {
      omniORB::logs(5, "Unable to create pipe for SocketCollection.");
    }
#  else
    int filedes[2];
    int r = pipe(filedes);
    if (r != -1) {
      pd_pipe_read  = filedes[0];
      pd_pipe_write = filedes[1];
    }
    else {
      omniORB::logs(5, "Unable to create pipe for SocketCollection.");
    }
#  endif
#endif

  if (pd_pipe_read > 0) {
    pd_n_fdset_1++;
    pd_n_fdset_2++;
    FD_SET(pd_pipe_read, &pd_fdset_1);
    FD_SET(pd_pipe_read, &pd_fdset_2);
  }

  pd_hash_table = new SocketLink* [hashsize];
  for (CORBA::ULong i=0; i < hashsize; i++)
    pd_hash_table[i] = 0;
}


/////////////////////////////////////////////////////////////////////////
SocketCollection::~SocketCollection()
{
  pd_refcount = -1;
  delete [] pd_hash_table;

#ifdef UnixArchitecture
#  ifdef __vxWorks__
  // *** How do we clean up on vxWorks?
#  else
  close(pd_pipe_read);
  close(pd_pipe_write);
#  endif
#endif
}


/////////////////////////////////////////////////////////////////////////
void
SocketCollection::setSelectable(SocketHandle_t sock, 
				CORBA::Boolean now,
				CORBA::Boolean data_in_buffer,
				CORBA::Boolean hold_lock) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_fdset_lock, hold_lock);

  if (!hold_lock) pd_fdset_lock.lock();

  if (data_in_buffer && !FD_ISSET(sock,&pd_fdset_dib)) {
    pd_n_fdset_dib++;
    FD_SET(sock,&pd_fdset_dib);
  }

  if (!FD_ISSET(sock,&pd_fdset_1)) {
    pd_n_fdset_1++;
    FD_SET(sock,&pd_fdset_1);
  }
  if (now || data_in_buffer) {
    if (!FD_ISSET(sock,&pd_fdset_2)) {
      pd_n_fdset_2++;
      FD_SET(sock,&pd_fdset_2);
    }
    // Wake up the thread blocked in select() if we can.
    if (pd_pipe_write > 0) {
#ifdef UnixArchitecture
      if (!pd_pipe_full) {
	char data = '\0';
	pd_pipe_full = 1;
	write(pd_pipe_write, &data, 1);
      }
#endif
    }
    else {
      pd_select_cond.signal();
    }
  }
  if (!hold_lock) pd_fdset_lock.unlock();
}

/////////////////////////////////////////////////////////////////////////
void
SocketCollection::clearSelectable(SocketHandle_t sock) {

  omni_tracedmutex_lock sync(pd_fdset_lock);

  if (FD_ISSET(sock,&pd_fdset_1)) {
    pd_n_fdset_1--;
    FD_CLR(sock,&pd_fdset_1);
  }
  if (FD_ISSET(sock,&pd_fdset_2)) {
    pd_n_fdset_2--;
    FD_CLR(sock,&pd_fdset_2);
  }
  if (FD_ISSET(sock,&pd_fdset_dib)) {
    pd_n_fdset_dib--;
    FD_CLR(sock,&pd_fdset_dib);
  }
}

#ifdef GDB_DEBUG

static
int
do_select(int maxfd,fd_set* r, fd_set* w, fd_set* e, struct timeval* t) {
  return select(maxfd,r,w,e,t);
}

#endif

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
SocketCollection::Select() {

  struct timeval timeout;
  fd_set         rfds;
  int            total;

 again:

  // (pd_abs_sec,pd_abs_nsec) define the absolute time when we switch fdset
  SocketSetTimeOut(pd_abs_sec,pd_abs_nsec,timeout);

  if (timeout.tv_sec == 0 && timeout.tv_usec == 0) {

    omni_thread::get_time(&pd_abs_sec,&pd_abs_nsec,
			  scan_interval_sec,scan_interval_nsec);
    timeout.tv_sec  = scan_interval_sec;
    timeout.tv_usec = scan_interval_nsec / 1000;

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

  int nready;

  if (fd != 0) {
#ifndef GDB_DEBUG
    nready = select(maxfd+1,&rfds,0,0,&timeout);
#else
    nready = do_select(maxfd+1,&rfds,0,0,&timeout);
#endif
  }
  else {
    omni_tracedmutex_lock sync(pd_fdset_lock);
    pd_select_cond.timedwait(pd_abs_sec,pd_abs_nsec);
    // The condition variable should be poked so we are woken up
    // immediately when there is something to monitor.  We cannot use
    // select(0,0,0,0,&timeout) because win32 doesn't like it.
    nready = 0; // simulate a timeout
  }

  if (nready == RC_SOCKET_ERROR) {
    if (ERRNO == RC_EBADF) {
      omniORB::logs(20, "select() returned EBADF, retrying");
      goto again;
    }
    else if (ERRNO != RC_EINTR) {
      return 0;
    }
    else {
      return 1;
    }
  }

  {
    omni_tracedmutex_lock sync(pd_fdset_lock);
    SocketHandle_t fd = 0;

    if (nready > 0) {
      // Process the result from the select.
      while (nready) {
	if (FD_ISSET(fd,&rfds)) {
	  nready--;

          if (fd == pd_pipe_read) {
#ifdef UnixArchitecture
            char data;
            read(pd_pipe_read, &data, 1);
            pd_pipe_full = 0;
#endif
          }
	  else {
	    if (FD_ISSET(fd,&pd_fdset_2)) {
	      pd_n_fdset_2--;
	      FD_CLR(fd,&pd_fdset_2);
	      if (FD_ISSET(fd,&pd_fdset_1)) {
		pd_n_fdset_1--;
		FD_CLR(fd,&pd_fdset_1);
	      }
	      if (FD_ISSET(fd,&pd_fdset_dib)) {
		pd_n_fdset_dib--;
		FD_CLR(fd,&pd_fdset_dib);
	      }
	      if (!notifyReadable(fd)) return 0;
	    }
	  }
	}
	fd++;
      }
    }

    // Process pd_fdset_dib. Those sockets with their bit set have
    // already got data in buffer. We do a call back for these sockets if
    // their entries in pd_fdset_2 is also set.
    fd = 0;
    nready = pd_n_fdset_dib;
    while (nready) {
      if (FD_ISSET(fd,&pd_fdset_dib)) {
	if (FD_ISSET(fd,&pd_fdset_2)) {
	  pd_n_fdset_2--;
	  FD_CLR(fd,&pd_fdset_2);
	  pd_n_fdset_dib--;
	  FD_CLR(fd,&pd_fdset_dib);
	  if (FD_ISSET(fd,&pd_fdset_1)) {
	    pd_n_fdset_1--;
	    FD_CLR(fd,&pd_fdset_1);
	  }
	  if (!notifyReadable(fd)) return 0;
	}
	nready--;
      }
      fd++;
    }
  }
  return 1;

}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
SocketCollection::Peek(SocketHandle_t sock) {

  {
    omni_tracedmutex_lock sync(pd_fdset_lock);
   
    // Do nothing if this socket is not set to be monitored.
    if (!FD_ISSET(sock,&pd_fdset_1))
      return 0;

    // If data in buffer is set, do callback straight away.
    if (FD_ISSET(sock,&pd_fdset_dib)) {
      if (FD_ISSET(sock,&pd_fdset_1)) {
	pd_n_fdset_1--;
	FD_CLR(sock,&pd_fdset_1);
      }
      if (FD_ISSET(sock,&pd_fdset_2)) {
	pd_n_fdset_2--;
	FD_CLR(sock,&pd_fdset_2);
      }
      pd_n_fdset_dib--;
      FD_CLR(sock,&pd_fdset_dib);
      return 1;
    }
  }

  struct timeval timeout;
  // select on the socket for half the time of scan_interval, if no request
  // arrives in this interval, we just let AcceptAndMonitor take care
  // of it.
  timeout.tv_sec  = scan_interval_sec / 2;
  timeout.tv_usec = scan_interval_nsec / 1000 / 2;
  if (scan_interval_sec % 2) timeout.tv_usec += 500000;
  fd_set rfds;

  do {
    FD_ZERO(&rfds);
    FD_SET(sock,&rfds);
#ifndef GDB_DEBUG
    int nready = select(sock+1,&rfds,0,0,&timeout);
#else
    int nready = do_select(sock+1,&rfds,0,0,&timeout);
#endif

    if (nready == RC_SOCKET_ERROR) {
      if (ERRNO != RC_EINTR) {
	break;
      }
      else {
	continue;
      }
    }

    // Reach here if nready >= 0

    if (FD_ISSET(sock,&rfds)) {
      omni_tracedmutex_lock sync(pd_fdset_lock);

      // Are we still interested?
      if (FD_ISSET(sock,&pd_fdset_1)) {
	if (FD_ISSET(sock,&pd_fdset_2)) {
	  pd_n_fdset_2--;
	  FD_CLR(sock,&pd_fdset_2);
	}
	pd_n_fdset_1--;
	FD_CLR(sock,&pd_fdset_1);
	if (FD_ISSET(sock,&pd_fdset_dib)) {
	  pd_n_fdset_dib--;
	  FD_CLR(sock,&pd_fdset_dib);
	}
	return 1;
      }
    }
    break;

  } while(1);

  return 0;
}


/////////////////////////////////////////////////////////////////////////
void
SocketCollection::incrRefCount()
{
  omni_tracedmutex_lock sync(pd_fdset_lock);
  OMNIORB_ASSERT(pd_refcount > 0);
  pd_refcount++;
}

/////////////////////////////////////////////////////////////////////////
void
SocketCollection::decrRefCount()
{
  int refcount;
  {
    omni_tracedmutex_lock sync(pd_fdset_lock);
    OMNIORB_ASSERT(pd_refcount > 0);
    refcount = --pd_refcount;
  }
  if (refcount == 0) delete this;
}

/////////////////////////////////////////////////////////////////////////
void
SocketCollection::addSocket(SocketLink* conn)
{
  omni_tracedmutex_lock sync(pd_fdset_lock);
  SocketLink** head = &(pd_hash_table[conn->pd_socket % hashsize]);
  conn->pd_next = *head;
  *head = conn;
  OMNIORB_ASSERT(pd_refcount > 0);
  pd_refcount++;
}

/////////////////////////////////////////////////////////////////////////
SocketLink*
SocketCollection::removeSocket(SocketHandle_t sock)
{
  int refcount  = 0; // Initialise to stop over-enthusiastic compiler warnings
  SocketLink* l = 0;
  {
    omni_tracedmutex_lock sync(pd_fdset_lock);
    SocketLink** head = &(pd_hash_table[sock % hashsize]);
    while (*head) {
      if ((*head)->pd_socket == sock) {
	l = *head;
	*head = (*head)->pd_next;
	OMNIORB_ASSERT(pd_refcount > 0);
	refcount = --pd_refcount;
	break;
      }
      head = &((*head)->pd_next);
    }
  }
  if (l && refcount == 0) delete this;
  return l;
}

/////////////////////////////////////////////////////////////////////////
SocketLink*
SocketCollection::findSocket(SocketHandle_t sock,
				CORBA::Boolean hold_lock) {

  if (!hold_lock) pd_fdset_lock.lock();

  SocketLink* l = 0;
  SocketLink** head = &(pd_hash_table[sock % hashsize]);
  while (*head) {
    if ((*head)->pd_socket == sock) {
      l = *head;
      break;
    }
    head = &((*head)->pd_next);
  }

  if (!hold_lock) pd_fdset_lock.unlock();

  return l;
}

OMNI_NAMESPACE_END(omni)
