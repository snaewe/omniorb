// -*- Mode: C++; -*-
//                            Package   : omniORB
// SocketCollection.cc        Created on: 23 Jul 2001
//                            Author 1  : Sai Lai Lo (sll)
//                            Author 2  : Duncan Grisby (dgrisby)
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
//	*** PROPRIETARY INTERFACE ***
// 

/*
  $Log$
  Revision 1.1.4.17  2006/11/16 14:07:40  dgrisby
  Peek failed to clear pd_fd_index, which could lead to a race condition
  where the wrong socket was removed from the poll list.

  Revision 1.1.4.16  2006/05/16 13:32:04  dgrisby
  Silly bug in setting Peek timeouts with conflicting Peeks.

  Revision 1.1.4.15  2006/05/15 17:11:15  dgrisby
  Limit select timeout in case time goes backwards.

  Revision 1.1.4.14  2006/05/02 13:07:13  dgrisby
  Idle giopMonitor SocketCollections would not exit at shutdown.

  Revision 1.1.4.13  2006/01/19 16:05:02  dgrisby
  Windows build fixes.

  Revision 1.1.4.12  2005/11/19 17:33:28  dgrisby
  Previous fix for the race condition was stupidly commented out...

  Revision 1.1.4.11  2005/11/18 18:25:57  dgrisby
  Race condition between connection deletion and Select.

  Revision 1.1.4.10  2005/08/03 09:43:13  dgrisby
  setSelectable could cause a readable socket to be handled twice.

  Revision 1.1.4.9  2005/08/02 09:42:53  dgrisby
  Two threads could be dispatched for one call, one by Peek, one by Select.

  Revision 1.1.4.8  2005/06/24 14:31:30  dgrisby
  Allow multiple threads to Peek() without clashing. Not yet tested on
  Windows.

  Revision 1.1.4.7  2005/04/26 17:08:06  dgrisby
  Handle all error returns from poll().

  Revision 1.1.4.6  2005/03/02 12:10:50  dgrisby
  setSelectable / Peek fixes.

  Revision 1.1.4.5  2005/01/25 16:43:37  dgrisby
  Properly handle data_in_buffer indicator.

  Revision 1.1.4.4  2005/01/17 14:46:19  dgrisby
  Windows SocketCollection implementation.

  Revision 1.1.4.3  2005/01/13 21:09:59  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.2  2005/01/06 23:10:12  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:02:31  dgrisby
  Start of omniORB 4.1.x development branch.

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
#  include "selectLib.h"
#  include "iostream.h"
#endif

#if defined(__VMS)
#  include <stropts.h>
#endif

#if !defined(__WIN32__)
#  define SELECTABLE_FD_LIMIT FD_SETSIZE
#  define IS_SELECTABLE(fd) (fd < FD_SETSIZE)
#else
#  define IS_SELECTABLE(fd) (1)
#endif


#ifdef USE_POLL
#  define INITIAL_POLLFD_LEN 64
#endif


OMNI_NAMESPACE_BEGIN(omni)

#define GDB_DEBUG

/////////////////////////////////////////////////////////////////////////
void
SocketSetTimeOut(unsigned long abs_sec, unsigned long abs_nsec,
		 struct timeval& t)
{
  if (abs_sec == 0 && abs_nsec == 0) {
    // Avoid get_time call which is expensive on some platforms.
    t.tv_sec = t.tv_usec = 0;
    return;
  }

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
# if defined(__vxWorks__)
  int fl = TRUE;
  if (ioctl(sock, FIONBIO, (int)&fl) == ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# elif defined(__VMS)
  int fl = 1;
  if (ioctl(sock, FIONBIO, &fl) == RC_INVALID_SOCKET) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# elif defined(__WIN32__)
  u_long v = 1;
  if (ioctlsocket(sock,FIONBIO,&v) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# else
  int fl = O_NONBLOCK;
  if (fcntl(sock,F_SETFL,fl) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# endif
}

/////////////////////////////////////////////////////////////////////////
int
SocketSetblocking(SocketHandle_t sock) {
# if defined(__vxWorks__)
  int fl = FALSE;
  if (ioctl(sock, FIONBIO, (int)&fl) == ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# elif defined(__VMS)
  int fl = 0;
  if (ioctl(sock, FIONBIO, &fl) == RC_INVALID_SOCKET) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# elif defined(__WIN32__)
  u_long v = 0;
  if (ioctlsocket(sock,FIONBIO,&v) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# else
  int fl = 0;
  if (fcntl(sock,F_SETFL,fl) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# endif
}


/////////////////////////////////////////////////////////////////////////
int
SocketSetCloseOnExec(SocketHandle_t sock) {
# if defined(__vxWorks__) || defined(__ETS_KERNEL__)
  // Not supported on vxWorks or ETS
  return 0;
# elif defined(__WIN32__)
  SetHandleInformation((HANDLE)sock, HANDLE_FLAG_INHERIT, 0);
  return 0;
# else
  int fl = FD_CLOEXEC;
  if (fcntl(sock,F_SETFD,fl) == RC_SOCKET_ERROR) {
    return RC_INVALID_SOCKET;
  }
  return 0;
# endif
}


/////////////////////////////////////////////////////////////////////////
unsigned long SocketCollection::scan_interval_sec  = 0;
unsigned long SocketCollection::scan_interval_nsec = 50*1000*1000;
unsigned      SocketCollection::idle_scans         = 20;

/////////////////////////////////////////////////////////////////////////
// pipe creation

#ifdef UnixArchitecture
#  ifdef __vxWorks__
static void initPipe(int& pipe_read, int& pipe_write)
{
  if (pipeDrv() == OK) {
    if (pipeDevCreate("/pipe/SocketCollection",10,sizeof(int)) == OK) {
      pipe_read = pipe_write = open("/pipe/SocketCollection", O_RDWR,0);
    }
  }
  if (pipe_read <= 0) {
    omniORB::logs(5, "Unable to create pipe for SocketCollection.");
    pipe_read = pipe_write = -1;
  }
}
static void closePipe(int pipe_read, int pipe_write)
{
  // *** How do we clean up on vxWorks?
}
#  else
static void initPipe(int& pipe_read, int& pipe_write)
{
  int filedes[2];
  int r = pipe(filedes);
  if (r != -1) {
    pipe_read  = filedes[0];
    pipe_write = filedes[1];
  }
  else {
    omniORB::logs(5, "Unable to create pipe for SocketCollection.");
    pipe_read = pipe_write = -1;
  }
}
static void closePipe(int pipe_read, int pipe_write)
{
  if (pipe_read  > 0) close(pipe_read);
  if (pipe_write > 0) close(pipe_write);
}
#  endif
#else
static void initPipe(int& pipe_read, int& pipe_write)
{
  pipe_read = pipe_write = -1;
}
static void closePipe(int pipe_read, int pipe_write)
{
}
#endif


#if defined(USE_POLL)

/////////////////////////////////////////////////////////////////////////
// poll() based implementation

SocketCollection::SocketCollection()
  : pd_refcount(1),
    pd_abs_sec(0), pd_abs_nsec(0),
    pd_pipe_full(0),
    pd_idle_count(idle_scans),
    pd_pollfd_n(0),
    pd_pollfd_len(INITIAL_POLLFD_LEN),
    pd_collection(0),
    pd_changed(1)
{
  pd_pollfds     = new struct pollfd[pd_pollfd_len];
  pd_pollsockets = new SocketHolder*[pd_pollfd_len];

  initPipe(pd_pipe_read, pd_pipe_write);
}

SocketCollection::~SocketCollection()
{
  pd_refcount = -1;
  delete [] pd_pollsockets;
  delete [] pd_pollfds;
  closePipe(pd_pipe_read, pd_pipe_write);
}

CORBA::Boolean
SocketCollection::isSelectable(SocketHandle_t sock)
{
  // All sockets are selectable with poll()
  return 1;
}

void
SocketCollection::growPollLists()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_collection_lock, 1);

  // Grow the lists
  struct pollfd* new_pollfds     = new struct pollfd[pd_pollfd_len*2];
  SocketHolder** new_pollsockets = new SocketHolder*[pd_pollfd_len*2];

  for (unsigned i=0; i < pd_pollfd_len; i++) {
    new_pollfds[i]     = pd_pollfds[i];
    new_pollsockets[i] = pd_pollsockets[i];
  }
  delete [] pd_pollfds;
  delete [] pd_pollsockets;

  pd_pollfds     = new_pollfds;
  pd_pollsockets = new_pollsockets;
  pd_pollfd_len  = pd_pollfd_len * 2;
}


CORBA::Boolean
SocketCollection::Select() {

  struct timeval timeout;
  int timeout_millis;
  int count;
  unsigned index;

  // (pd_abs_sec,pd_abs_nsec) defines the absolute time at which we
  // process the socket list.
  SocketSetTimeOut(pd_abs_sec,pd_abs_nsec,timeout);

  if ((timeout.tv_sec == 0 && timeout.tv_usec == 0) ||
      timeout.tv_sec > scan_interval_sec) {

    // Time to scan the socket list...

    omni_thread::get_time(&pd_abs_sec,&pd_abs_nsec,
			  scan_interval_sec,scan_interval_nsec);
    timeout.tv_sec  = scan_interval_sec;
    timeout.tv_usec = scan_interval_nsec / 1000;

    index = 0;
    {
      omni_tracedmutex_lock sync(pd_collection_lock);
      if (pd_changed) {

	// Add our pipe
	if (pd_pipe_read >= 0) {
	  pd_pollfds[index].fd      = pd_pipe_read;
	  pd_pollfds[index].events  = POLLIN;
	  pd_pollfds[index].revents = 0;
	  pd_pollsockets[index]     = 0;
	  index++;
	}

	// Add all the selectable sockets
	for (SocketHolder* s = pd_collection; s; s = s->pd_next) {
	  if (s->pd_selectable) {
	    if (s->pd_data_in_buffer) {
	      // Socket is readable now
	      s->pd_selectable = s->pd_data_in_buffer = 0;
              s->pd_fd_index = -1;
	      notifyReadable(s);
	    }
	    else {
	      // Add to the pollfds
	      if (index == pd_pollfd_len)
		growPollLists();

	      pd_pollfds[index].fd      = s->pd_socket;
	      pd_pollfds[index].events  = POLLIN;
	      pd_pollfds[index].revents = 0;
	      pd_pollsockets[index]     = s;
	      s->pd_fd_index            = index;
	      index++;
	    }
	  }
          else {
            s->pd_fd_index = -1;
          }
	}
	pd_pollfd_n = index;
	pd_changed  = 0;
      }
      if (pd_idle_count > 0 || pd_pipe_read < 0) {
	timeout_millis = timeout.tv_sec * 1000 + timeout.tv_usec / 1000;
      }
      else {
	omniORB::logs(25, "SocketCollection idle. Sleeping.");
	timeout_millis = -1;
      }
    }
  }
  else {
    timeout_millis = timeout.tv_sec * 1000 + timeout.tv_usec / 1000;
  }

  count = poll(pd_pollfds, pd_pollfd_n, timeout_millis);

  if (count > 0) {
    // Some sockets are readable
    omni_tracedmutex_lock sync(pd_collection_lock);

    pd_idle_count = idle_scans;

    index = 0;
    while (count) {
      OMNIORB_ASSERT(index < pd_pollfd_n);

      short revents = pd_pollfds[index].revents;

      if (revents) {
	count--;

	if (revents & POLLIN) {
	  SocketHolder* s = pd_pollsockets[index];

	  if (s) {
	    // Remove from pollfds by swapping in the last item in the array
	    pd_pollfd_n--;
	    s->pd_fd_index                     = -1;
	    pd_pollfds[index]     	       = pd_pollfds[pd_pollfd_n];
	    pd_pollsockets[index] 	       = pd_pollsockets[pd_pollfd_n];
	    if (pd_pollsockets[index])
	      pd_pollsockets[index]->pd_fd_index = index;

	    if (s->pd_peeking) {
	      // A thread is monitoring the socket with Peek(). We do
	      // not notify from this thread, otherwise a thread would
	      // be dispatched to handle the data, and either that
	      // thread or the peeker would find itself with nothing
	      // to do. To avoid a race condition where the Peek()
	      // thread has timed out, we set a flag to tell it there
	      // is data available.
	      s->pd_peek_go = 1;
	    }
	    else {
	      if (s->pd_selectable) {
		s->pd_selectable = s->pd_data_in_buffer = 0;
		notifyReadable(s);
	      }
	    }
	    continue; // without incrementing index
	  }
	  else if (pd_pollfds[index].fd == pd_pipe_read) {
#ifdef UnixArchitecture
	    char data;
	    read(pd_pipe_read, &data, 1);
	    pd_pipe_full = 0;
#endif
	  }
	  else {
	    // Socket is no longer selectable -- remove from pollfds.
	    pd_pollfd_n--;
	    pd_pollfds[index]     = pd_pollfds[pd_pollfd_n];
	    pd_pollsockets[index] = pd_pollsockets[pd_pollfd_n];
	    if (pd_pollsockets[index])
	      pd_pollsockets[index]->pd_fd_index = index;
	    continue; // without incrementing index
	  }
	}
	else {
	  // Force a list scan next time
	  pd_changed = 1;
	  pd_abs_sec = pd_abs_nsec = 0;
	}
      }
      index++;
    }
  }
  else if (count == 0) {
    // Nothing to read.
    omni_tracedmutex_lock sync(pd_collection_lock);
    if (pd_idle_count > 0)
      pd_idle_count--;
  }
  else {
    // Negative return means error
    if (ERRNO == RC_EBADF) {
      omniORB::logs(20, "poll() returned EBADF.");

      // Force a list scan next time
      pd_changed = 1;
      pd_abs_sec = pd_abs_nsec = 0;
    }
    else if (ERRNO != RC_EINTR) {
      if (omniORB::trace(20)) {
	omniORB::logger l;
	l << "Error return from poll(). errno = " << (int)ERRNO << "\n";
      }
      return 0;
    }
  }
  return 1;
}

void SocketCollection::wakeUp()
{
  omni_tracedmutex_lock sync(pd_collection_lock);

  if (pd_pipe_write >= 0 && !pd_pipe_full) {
    char data = '\0';
    pd_pipe_full = 1;
    write(pd_pipe_write, &data, 1);
  }
}


void
SocketHolder::setSelectable(int            now,
			    CORBA::Boolean data_in_buffer,
			    CORBA::Boolean hold_lock)
{
  OMNIORB_ASSERT(pd_belong_to);
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_belong_to->pd_collection_lock, hold_lock);

  omni_optional_lock l(pd_belong_to->pd_collection_lock, hold_lock, hold_lock);

  if (now == 2 && !pd_selectable)
    return;

  if (now && pd_fd_index == -1) {
    // Add socket to the list of pollfds
    unsigned index = pd_belong_to->pd_pollfd_n;

    if (index == pd_belong_to->pd_pollfd_len)
      pd_belong_to->growPollLists();

    pd_belong_to->pd_pollfds[index].fd      = pd_socket;
    pd_belong_to->pd_pollfds[index].events  = POLLIN;
    pd_belong_to->pd_pollfds[index].revents = 0;
    pd_belong_to->pd_pollsockets[index]     = this;
    pd_belong_to->pd_pollfd_n = index + 1;
    pd_fd_index = index;
  }

  pd_selectable      = 1;
  pd_data_in_buffer |= data_in_buffer;

  pd_belong_to->pd_changed = 1;

  if (pd_data_in_buffer) {
    // Force Select() to scan through the connections right away
    pd_belong_to->pd_abs_sec = pd_belong_to->pd_abs_nsec = 0;
  }

#ifdef UnixArchitecture
  if (!hold_lock &&
      (now || pd_data_in_buffer || pd_belong_to->pd_idle_count == 0)) {

    // Wake up the Select thread by writing to the pipe.
    if (pd_belong_to->pd_pipe_write >= 0 && !pd_belong_to->pd_pipe_full) {
      char data = '\0';
      pd_belong_to->pd_pipe_full = 1;
      write(pd_belong_to->pd_pipe_write, &data, 1);
    }
  }
#endif
  // Wake up a thread waiting to peek, if there is one
  if (pd_peek_cond)
    pd_peek_cond->signal();
}

void
SocketHolder::clearSelectable()
{
  OMNIORB_ASSERT(pd_belong_to);
  omni_tracedmutex_lock l(pd_belong_to->pd_collection_lock);
  pd_selectable = 0;

  if (pd_fd_index >= 0) {
    pd_belong_to->pd_pollsockets[pd_fd_index] = 0;
    pd_fd_index = -1;
  }

#ifdef UnixArchitecture
  if (pd_belong_to->pd_idle_count == 0) {
    // Wake up the Select thread by writing to the pipe.
    if (pd_belong_to->pd_pipe_write >= 0 && !pd_belong_to->pd_pipe_full) {
      char data = '\0';
      pd_belong_to->pd_pipe_full = 1;
      write(pd_belong_to->pd_pipe_write, &data, 1);
    }
  }
#endif
}

CORBA::Boolean
SocketHolder::Peek()
{
  {
    omni_tracedmutex_lock l(pd_belong_to->pd_collection_lock);

    unsigned long s, ns;
    s = ns = 0;

    while (1) {
      if (pd_selectable && !pd_peeking) {
	break;
      }
      else {
	if (omniORB::trace(25)) {
	  omniORB::logger l;
	  l << "Socket " << (int)pd_socket << " in Peek() is "
	    << (pd_selectable ? "being peeked" : "not selectable")
	    << ". Waiting...\n";
	}
	if (!pd_peek_cond)
	  pd_peek_cond =
	    new omni_tracedcondition(&pd_belong_to->pd_collection_lock);
	
	if (s == 0 && ns == 0) {
	  // Set timeout for condition wait
	  unsigned long rs, rns;
	  rs  = SocketCollection::scan_interval_sec;
	  rns = SocketCollection::scan_interval_nsec;
	  omni_thread::get_time(&s, &ns, rs, rns);
	}
	int signalled = pd_peek_cond->timedwait(s, ns);
	
	if (pd_selectable && !pd_peeking) {
	  // OK to go ahead and peek
	  omniORB::logs(25, "Peek can now go ahead.");
	  break;
	}
	if (!signalled) {
	  // Timed out. Give up.
	  omniORB::logs(25, "Timed out waiting to be able to peek.");
	  return 0;
	}
      }
    }
    if (pd_data_in_buffer) {
      pd_data_in_buffer = 0;
      pd_selectable = 0;
      return 1;
    }
    pd_peeking = 1;
    pd_peek_go = 0;
  }

  int timeout = (SocketCollection::scan_interval_sec * 1000 +
		 SocketCollection::scan_interval_nsec / 1000000);

  pollfd pfd;
  pfd.fd = pd_socket;
  pfd.events = POLLIN;

  int retval = -1;

  while (1) {
    int r = poll(&pfd, 1, timeout);

    {
      omni_tracedmutex_lock l(pd_belong_to->pd_collection_lock);

      if (pd_data_in_buffer) {
	pd_data_in_buffer = 0;
	retval = 1;
      }
      else if (r > 0) {
	if (pfd.revents & POLLIN && pd_selectable) {
	  retval = 1;
	}
	else {
	  retval = 0;
	}
      }
      else if (r == 0) {
	// Timed out
	if (pd_peek_go) {
	  // Select thread has seen that we should return true
	  retval = 1;
	}
	else {
	  retval = 0;
	}
      }
      else {
	// Error return
	if (ERRNO != RC_EINTR)
	  retval = 0;
      }

      if (retval != -1) {
        if (retval) {
          pd_selectable = 0;
          if (pd_fd_index >= 0) {
            pd_belong_to->pd_pollsockets[pd_fd_index] = 0;
            pd_fd_index = -1;
          }
        }
	pd_peeking = 0;
	if (pd_peek_cond)
	  pd_peek_cond->signal();
	break;
      }
    }
  }
  return (CORBA::Boolean)retval;
}

#elif defined(__WIN32__)

/////////////////////////////////////////////////////////////////////////
// Windows select() based implementation

// On Windows, fd_set is not a bit mask like on Unix platforms.
// Instead, it is a struct containing an array of fds. Adding an fd to
// select with FD_SET is O(n) on the number of items already in the
// set, meaning that adding n fds to an empty set is O(n^2).
//
// To avoid that overhead, we directly access the array inside the
// fd_set. This is risky since one day the implementation might
// change, but it makes a big difference to the performance with large
// sets.

SocketCollection::SocketCollection()
  : pd_refcount(1),
    pd_select_cond(&pd_collection_lock),
    pd_abs_sec(0), pd_abs_nsec(0),
    pd_pipe_full(0),
    pd_idle_count(idle_scans),
    pd_collection(0),
    pd_changed(1)
{
  FD_ZERO(&pd_fd_set);
}

SocketCollection::~SocketCollection()
{
  pd_refcount = -1;
}

CORBA::Boolean
SocketCollection::isSelectable(SocketHandle_t sock)
{
  return 1;
}

CORBA::Boolean
SocketCollection::Select() {

  struct timeval  timeout;
  int count;
  unsigned index;

  // (pd_abs_sec,pd_abs_nsec) defines the absolute time at which we
  // process the socket list.
  SocketSetTimeOut(pd_abs_sec,pd_abs_nsec,timeout);

  if ((timeout.tv_sec == 0 && timeout.tv_usec == 0) ||
      timeout.tv_sec > scan_interval_sec) {

    // Time to scan the socket list...

    omni_thread::get_time(&pd_abs_sec,&pd_abs_nsec,
			  scan_interval_sec,scan_interval_nsec);
    timeout.tv_sec  = scan_interval_sec;
    timeout.tv_usec = scan_interval_nsec / 1000;

    {
      omni_tracedmutex_lock sync(pd_collection_lock);
      if (pd_changed) {

	FD_ZERO(&pd_fd_set);

	// Add all the selectable sockets
	for (SocketHolder* s = pd_collection; s; s = s->pd_next) {
	  if (s->pd_selectable) {
	    if (s->pd_data_in_buffer) {
	      // Socket is readable now
	      s->pd_selectable = s->pd_data_in_buffer = 0;
	      notifyReadable(s);
	    }
	    else {
	      // Add to the fd_set
	      if (pd_fd_set.fd_count == FD_SETSIZE) {
		omniORB::logs(1, "Reached the limit of selectable file "
			      "descriptors. Some connections may be ignored.");
		break;
	      }
	      s->pd_fd_index = pd_fd_set.fd_count;
	      pd_fd_set.fd_array[pd_fd_set.fd_count] = s->pd_socket;
	      pd_fd_sockets[pd_fd_set.fd_count] = s;
	      pd_fd_set.fd_count++;
	    }
	  }
	}
	pd_changed = 0;
      }
    }
  }

  fd_set rfds = pd_fd_set;

  if (rfds.fd_count) {
    // Windows select() ignores its first argument.
    count = select(0, &rfds, 0, 0, &timeout);
  }
  else {
    // Windows doesn't let us select on an empty fd_set, so we sleep
    // on a condition variable instead.
    omni_tracedmutex_lock sync(pd_collection_lock);
    pd_select_cond.timedwait(pd_abs_sec,pd_abs_nsec);
    count = 0;
  }

  if (count > 0) {
    // Some sockets are readable
    omni_tracedmutex_lock sync(pd_collection_lock);

    for (SocketHolder* s = pd_collection; s && count; s = s->pd_next) {

      if (s->pd_selectable) {

	// The call to FD_ISSET here is a linear search through all
	// the readable sockets, but that should usually be a
	// reasonably small number, so it's probably OK.

	if (FD_ISSET(s->pd_socket, &rfds)) {
	  // Remove socket from pd_fd_set by swapping last item.
	  // We don't use FD_CLR since that is incredibly inefficient
	  // with large sets.
	  OMNIORB_ASSERT(s->pd_fd_index >= 0);
	  int to_i   = s->pd_fd_index;
	  int from_i = --pd_fd_set.fd_count;
	  
	  pd_fd_set.fd_array[to_i] = pd_fd_set.fd_array[from_i];
	  pd_fd_sockets[to_i] = pd_fd_sockets[from_i];
	  pd_fd_sockets[to_i]->pd_fd_index = to_i;

	  s->pd_fd_index = -1;

	  if (s->pd_peeking) {
	    // A thread is monitoring the socket with Peek(). We do
	    // not notify from this thread, otherwise a thread would
	    // be dispatched to handle the data, and either that
	    // thread or the peeker would find itself with nothing
	    // to do. To avoid a race condition where the Peek()
	    // thread has timed out, we set a flag to tell it there
	    // is data available.
	    s->pd_peek_go = 1;
	  }
	  else {
	    s->pd_selectable = s->pd_data_in_buffer = 0;
	    notifyReadable(s);
	  }
	  count--;
	}
      }
    }
  }
  else if (count == 0) {
    // Nothing to read.
  }
  else {
    // Negative return means error
    if (ERRNO == RC_EBADF) {
      omniORB::logs(20, "select() returned EBADF.");

      // Force a list scan next time
      pd_changed = 1;
      pd_abs_sec = pd_abs_nsec = 0;
    }
    else if (ERRNO != RC_EINTR) {
      if (omniORB::trace(1)) {
	omniORB::logger l;
	l << "Error return from select(). errno = " << (int)ERRNO << "\n";
      }
      return 0;
    }
  }
  return 1;
}

void SocketCollection::wakeUp()
{
  // Not possible on Win32
}


void
SocketHolder::setSelectable(int            now,
			    CORBA::Boolean data_in_buffer,
			    CORBA::Boolean hold_lock)
{
  OMNIORB_ASSERT(pd_belong_to);
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_belong_to->pd_collection_lock, hold_lock);

  omni_optional_lock l(pd_belong_to->pd_collection_lock, hold_lock, hold_lock);

  if (now == 2 && !pd_selectable)
    return;

  if (now && pd_fd_index == -1) {
    // Add socket to the fd_set
    if (pd_belong_to->pd_fd_set.fd_count == FD_SETSIZE) {
      omniORB::logs(1, "Reached the limit of selectable file "
		    "descriptors. Some connections may be ignored.");
    }
    else {
      pd_fd_index = pd_belong_to->pd_fd_set.fd_count;
      pd_belong_to->pd_fd_set.fd_array[pd_fd_index] = pd_socket;
      pd_belong_to->pd_fd_sockets[pd_fd_index] = this;
      pd_belong_to->pd_fd_set.fd_count++;

      if (!hold_lock && pd_belong_to->pd_fd_set.fd_count == 1) {
	// Select() thread may be waiting on the condition variable.
	pd_belong_to->pd_select_cond.signal();
      }
    }
  }

  pd_selectable      = 1;
  pd_data_in_buffer |= data_in_buffer;

  pd_belong_to->pd_changed = 1;

  if (pd_data_in_buffer) {
    // Force Select() to scan through the connections right away
    pd_belong_to->pd_abs_sec = pd_belong_to->pd_abs_nsec = 0;
  }

  // Wake up a thread waiting to peek, if there is one
  if (pd_peek_cond)
    pd_peek_cond->signal();
}

void
SocketHolder::clearSelectable()
{
  OMNIORB_ASSERT(pd_belong_to);
  omni_tracedmutex_lock l(pd_belong_to->pd_collection_lock);
  pd_selectable = 0;
}


CORBA::Boolean
SocketHolder::Peek()
{
  {
    omni_tracedmutex_lock l(pd_belong_to->pd_collection_lock);

    unsigned long s, ns;
    s = ns = 0;

    while (1) {
      if (pd_selectable && !pd_peeking) {
	break;
      }
      else {
	if (omniORB::trace(25)) {
	  omniORB::logger l;
	  l << "Socket " << (int)pd_socket << " in Peek() is "
	    << (pd_selectable ? "being peeked" : "not selectable")
	    << ". Waiting...\n";
	}
	if (!pd_peek_cond)
	  pd_peek_cond =
	    new omni_tracedcondition(&pd_belong_to->pd_collection_lock);
	
	if (s == 0 && ns == 0) {
	  // Set timeout for condition wait
	  unsigned long rs, rns;
	  rs  = SocketCollection::scan_interval_sec;
	  rns = SocketCollection::scan_interval_nsec;
	  omni_thread::get_time(&s, &ns, rs, rns);
	}
	int signalled = pd_peek_cond->timedwait(s, ns);
	
	if (pd_selectable && !pd_peeking) {
	  // OK to go ahead and peek
	  omniORB::logs(25, "Peek can now go ahead.");
	  break;
	}
	if (!signalled) {
	  // Timed out. Give up.
	  omniORB::logs(25, "Timed out waiting to be able to peek.");
	  return 0;
	}
      }
    }
    if (pd_data_in_buffer) {
      pd_data_in_buffer = 0;
      pd_selectable = 0;
      return 1;
    }
    pd_peeking = 1;
    pd_peek_go = 0;
  }

  struct timeval timeout;
  timeout.tv_sec  = SocketCollection::scan_interval_sec;
  timeout.tv_usec = SocketCollection::scan_interval_nsec / 1000;
  fd_set rfds;

  int retval = -1;

  while (1) {
    FD_ZERO(&rfds);
    FD_SET(pd_socket, &rfds);

    int r = select(pd_socket + 1, &rfds, 0, 0, &timeout);

    {
      omni_tracedmutex_lock l(pd_belong_to->pd_collection_lock);

      if (pd_data_in_buffer) {
	pd_data_in_buffer = 0;
	pd_selectable = 0;
	retval = 1;
      }
      else if (r > 0) {
	if (FD_ISSET(pd_socket, &rfds) && pd_selectable) {
	  pd_selectable = 0;
	  retval = 1;
	}
	else {
	  retval = 0;
	}
      }
      else if (r == 0) {
	// Timed out
	if (pd_peek_go) {
	  // Select thread has seen that we should return true
	  pd_selectable = 0;
	  retval = 1;
	}
	else {
	  retval = 0;
	}
      }
      else {
	if (ERRNO != RC_EINTR)
	  retval = 0;
      }

      if (retval != -1) {
	pd_peeking = 0;
	if (pd_peek_cond)
	  pd_peek_cond->signal();
	break;
      }
    }
  }
  return (CORBA::Boolean)retval;
}

#else

/////////////////////////////////////////////////////////////////////////
// Posix select() based implementation

SocketCollection::SocketCollection()
  : pd_refcount(1),
    pd_abs_sec(0), pd_abs_nsec(0),
    pd_pipe_full(0),
    pd_idle_count(idle_scans),
    pd_fd_set_n(0),
    pd_collection(0),
    pd_changed(1)
{
  FD_ZERO(&pd_fd_set);
  initPipe(pd_pipe_read, pd_pipe_write);
}

SocketCollection::~SocketCollection()
{
  pd_refcount = -1;
  closePipe(pd_pipe_read, pd_pipe_write);
}

CORBA::Boolean
SocketCollection::isSelectable(SocketHandle_t sock)
{
  return IS_SELECTABLE(sock);
}

#ifdef GDB_DEBUG
static
int
do_select(int maxfd,fd_set* r, fd_set* w, fd_set* e, struct timeval* t) {
  return select(maxfd,r,w,e,t);
}
#else
#  define do_select select
#endif


CORBA::Boolean
SocketCollection::Select() {

  struct timeval  timeout;
  struct timeval* timeout_p;
  int count;
  unsigned index;

  // (pd_abs_sec,pd_abs_nsec) defines the absolute time at which we
  // process the socket list.
  SocketSetTimeOut(pd_abs_sec,pd_abs_nsec,timeout);

  if ((timeout.tv_sec == 0 && timeout.tv_usec == 0) ||
      timeout.tv_sec > scan_interval_sec) {

    // Time to scan the socket list...

    omni_thread::get_time(&pd_abs_sec,&pd_abs_nsec,
			  scan_interval_sec,scan_interval_nsec);
    timeout.tv_sec  = scan_interval_sec;
    timeout.tv_usec = scan_interval_nsec / 1000;

    {
      omni_tracedmutex_lock sync(pd_collection_lock);
      if (pd_changed) {

	FD_ZERO(&pd_fd_set);
	pd_fd_set_n = 0;

	// Add our pipe
	if (pd_pipe_read >= 0) {
	  FD_SET(pd_pipe_read, &pd_fd_set);
	  pd_fd_set_n = pd_pipe_read + 1;
	}

	// Add all the selectable sockets
	for (SocketHolder* s = pd_collection; s; s = s->pd_next) {
	  if (s->pd_selectable) {
	    if (s->pd_data_in_buffer) {
	      // Socket is readable now
	      s->pd_selectable = s->pd_data_in_buffer = 0;
	      notifyReadable(s);
	    }
	    else {
	      // Add to the fd_set
	      s->pd_fd_index = 1;

	      if (IS_SELECTABLE(s->pd_socket)) {
		FD_SET(s->pd_socket, &pd_fd_set);
		if (pd_fd_set_n <= s->pd_socket)
		  pd_fd_set_n = s->pd_socket + 1;
	      }
	    }
	  }
	}
	pd_changed = 0;
      }
      if (pd_idle_count > 0 || pd_pipe_read < 0) {
	timeout_p = &timeout;
      }
      else {
	omniORB::logs(25, "SocketCollection idle. Sleeping.");
	timeout_p = 0;
      }
    }
  }
  else {
    timeout_p = &timeout;
  }

  fd_set rfds = pd_fd_set;
  count = do_select(pd_fd_set_n, &rfds, 0, 0, timeout_p);

  if (count > 0) {
    // Some sockets are readable
    omni_tracedmutex_lock sync(pd_collection_lock);

    pd_idle_count = idle_scans;

    pd_fd_set_n = 0;

#ifdef UnixArchitecture
    if (pd_pipe_read >= 0 && FD_ISSET(pd_pipe_read, &rfds)) {
      char data;
      read(pd_pipe_read, &data, 1);
      pd_pipe_full = 0;
      pd_fd_set_n = pd_pipe_read + 1;
    }
#endif

    for (SocketHolder* s = pd_collection; s; s = s->pd_next) {

      if (s->pd_selectable) {
	if (FD_ISSET(s->pd_socket, &rfds)) {
	  s->pd_fd_index = -1;
	  FD_CLR(s->pd_socket, &pd_fd_set);
	  
	  if (s->pd_peeking) {
	    // A thread is monitoring the socket with Peek(). We do
	    // not notify from this thread, otherwise a thread would
	    // be dispatched to handle the data, and either that
	    // thread or the peeker would find itself with nothing
	    // to do. To avoid a race condition where the Peek()
	    // thread has timed out, we set a flag to tell it there
	    // is data available.
	    s->pd_peek_go = 1;
	  }
	  else {
	    s->pd_selectable = s->pd_data_in_buffer = 0;
	    notifyReadable(s);
	  }
	}
	else if (pd_fd_set_n <= s->pd_socket) {
	  pd_fd_set_n = s->pd_socket + 1;
	}
      }
    }
  }
  else if (count == 0) {
    // Nothing to read.
    omni_tracedmutex_lock sync(pd_collection_lock);
    if (pd_idle_count > 0)
      pd_idle_count--;
  }
  else {
    // Negative return means error
    if (ERRNO == RC_EBADF) {
      omniORB::logs(20, "select() returned EBADF.");

      // Force a list scan next time
      pd_changed = 1;
      pd_abs_sec = pd_abs_nsec = 0;
    }
    else if (ERRNO != RC_EINTR) {
      if (omniORB::trace(1)) {
	omniORB::logger l;
	l << "Error return from select(). errno = " << (int)ERRNO << "\n";
      }
      return 0;
    }
  }
  return 1;
}

void SocketCollection::wakeUp()
{
  omni_tracedmutex_lock sync(pd_collection_lock);

  if (pd_pipe_write >= 0 && !pd_pipe_full) {
    char data = '\0';
    pd_pipe_full = 1;
    write(pd_pipe_write, &data, 1);
  }
}

void
SocketHolder::setSelectable(int            now,
			    CORBA::Boolean data_in_buffer,
			    CORBA::Boolean hold_lock)
{
  OMNIORB_ASSERT(pd_belong_to);
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_belong_to->pd_collection_lock, hold_lock);

  omni_optional_lock l(pd_belong_to->pd_collection_lock, hold_lock, hold_lock);

  if (now == 2 && !pd_selectable)
    return;

  if (now && pd_fd_index == -1) {
    // Add socket to the fd_set
    pd_fd_index = 1;

    if (IS_SELECTABLE(pd_socket)) {
      FD_SET(pd_socket, &pd_belong_to->pd_fd_set);
      if (pd_belong_to->pd_fd_set_n <= pd_socket)
	pd_belong_to->pd_fd_set_n = pd_socket + 1;
    }
  }

  pd_selectable      = 1;
  pd_data_in_buffer |= data_in_buffer;

  pd_belong_to->pd_changed = 1;

  if (pd_data_in_buffer) {
    // Force Select() to scan through the connections right away
    pd_belong_to->pd_abs_sec = pd_belong_to->pd_abs_nsec = 0;
  }

#ifdef UnixArchitecture
  if (!hold_lock &&
      (now || pd_data_in_buffer || pd_belong_to->pd_idle_count == 0)) {

    // Wake up the Select thread by writing to the pipe.
    if (pd_belong_to->pd_pipe_write >= 0 && !pd_belong_to->pd_pipe_full) {
      char data = '\0';
      pd_belong_to->pd_pipe_full = 1;
      write(pd_belong_to->pd_pipe_write, &data, 1);
    }
  }
#endif
  // Wake up a thread waiting to peek, if there is one
  if (pd_peek_cond)
    pd_peek_cond->signal();
}

void
SocketHolder::clearSelectable()
{
  OMNIORB_ASSERT(pd_belong_to);
  omni_tracedmutex_lock l(pd_belong_to->pd_collection_lock);
  pd_selectable = 0;

#ifdef UnixArchitecture
  if (pd_belong_to->pd_idle_count == 0) {
    // Wake up the Select thread by writing to the pipe.
    if (pd_belong_to->pd_pipe_write >= 0 && !pd_belong_to->pd_pipe_full) {
      char data = '\0';
      pd_belong_to->pd_pipe_full = 1;
      write(pd_belong_to->pd_pipe_write, &data, 1);
    }
  }
#endif
}


CORBA::Boolean
SocketHolder::Peek()
{
  {
    omni_tracedmutex_lock l(pd_belong_to->pd_collection_lock);

    unsigned long s, ns;
    s = ns = 0;

    while (1) {
      if (pd_selectable && !pd_peeking) {
	break;
      }
      else {
	if (omniORB::trace(25)) {
	  omniORB::logger l;
	  l << "Socket " << (int)pd_socket << " in Peek() is "
	    << (pd_selectable ? "being peeked" : "not selectable")
	    << ". Waiting...\n";
	}
	if (!pd_peek_cond)
	  pd_peek_cond =
	    new omni_tracedcondition(&pd_belong_to->pd_collection_lock);
	
	if (s == 0 && ns == 0) {
	  // Set timeout for condition wait
	  unsigned long rs, rns;
	  rs  = SocketCollection::scan_interval_sec  / 2;
	  rns = SocketCollection::scan_interval_nsec / 2;
	  if (SocketCollection::scan_interval_sec % 2)
	    rns += 500000000;
	  omni_thread::get_time(&s, &ns, rs, rns);
	}
	int signalled = pd_peek_cond->timedwait(s, ns);
	
	if (pd_selectable && !pd_peeking) {
	  // OK to go ahead and peek
	  omniORB::logs(25, "Peek can now go ahead.");
	  break;
	}
	if (!signalled) {
	  // Timed out. Give up.
	  omniORB::logs(25, "Timed out waiting to be able to peek.");
	  return 0;
	}
      }
    }
    if (pd_data_in_buffer) {
      pd_data_in_buffer = 0;
      pd_selectable = 0;
      return 1;
    }
    pd_peeking = 1;
    pd_peek_go = 0;
  }

  struct timeval timeout;
  timeout.tv_sec  = SocketCollection::scan_interval_sec / 2;
  timeout.tv_usec = SocketCollection::scan_interval_nsec / 1000 / 2;
  if (SocketCollection::scan_interval_sec % 2) timeout.tv_usec += 500000;
  fd_set rfds;

  int retval = -1;

  while (1) {
    FD_ZERO(&rfds);
    FD_SET(pd_socket, &rfds);

    int r = do_select(pd_socket + 1, &rfds, 0, 0, &timeout);

    {
      omni_tracedmutex_lock l(pd_belong_to->pd_collection_lock);

      if (pd_data_in_buffer) {
	pd_data_in_buffer = 0;
	pd_selectable = 0;
	retval = 1;
      }
      else if (r > 0) {
	if (FD_ISSET(pd_socket, &rfds) && pd_selectable) {
	  pd_selectable = 0;
	  retval = 1;
	}
	else {
	  retval = 0;
	}
      }
      else if (r == 0) {
	// Timed out
	if (pd_peek_go) {
	  // Select thread has seen that we should return true
	  pd_selectable = 0;
	  retval = 1;
	}
	else {
	  retval = 0;
	}
      }
      else {
	if (ERRNO != RC_EINTR)
	  retval = 0;
      }

      if (retval != -1) {
	pd_peeking = 0;
	if (pd_peek_cond)
	  pd_peek_cond->signal();
	break;
      }
    }
  }
  return (CORBA::Boolean)retval;
}

#endif


/////////////////////////////////////////////////////////////////////////
// Common functions


/////////////////////////////////////////////////////////////////////////
SocketHolder::~SocketHolder()
{
}


/////////////////////////////////////////////////////////////////////////
void
SocketCollection::incrRefCount()
{
  omni_tracedmutex_lock sync(pd_collection_lock);
  OMNIORB_ASSERT(pd_refcount > 0);
  pd_refcount++;
}

/////////////////////////////////////////////////////////////////////////
void
SocketCollection::decrRefCount()
{
  int refcount;
  {
    omni_tracedmutex_lock sync(pd_collection_lock);
    OMNIORB_ASSERT(pd_refcount > 0);
    refcount = --pd_refcount;
  }
  if (refcount == 0) delete this;
}

/////////////////////////////////////////////////////////////////////////
void
SocketCollection::addSocket(SocketHolder* s)
{
  omni_tracedmutex_lock sync(pd_collection_lock);

  OMNIORB_ASSERT(pd_refcount > 0);
  pd_refcount++;

  s->pd_belong_to = this;

  if (pd_collection)
    pd_collection->pd_prev = &s->pd_next;

  s->pd_next = pd_collection;
  s->pd_prev = &pd_collection;
  pd_collection = s;
}

/////////////////////////////////////////////////////////////////////////
void
SocketCollection::removeSocket(SocketHolder* s)
{
  OMNIORB_ASSERT(s->pd_belong_to == this);

  int refcount = 0; // Initialise to stop over-enthusiastic compiler warnings
  {
    omni_tracedmutex_lock sync(pd_collection_lock);

    OMNIORB_ASSERT(pd_refcount > 0);
    refcount = --pd_refcount;

    *(s->pd_prev) = s->pd_next;

    if (s->pd_next)
      s->pd_next->pd_prev = s->pd_prev;

    s->pd_belong_to = 0;
  }
  if (refcount == 0) delete this;
}


OMNI_NAMESPACE_END(omni)
