#ifndef _SELECT_H
#define _SELECT_H

#ifdef __UNIX__
#include <unistd.h>
#define PIPE(X) pipe(X)
#else
#if defined(_MSC_VER)
#include <io.h>
#define PIPE(X) _pipe(X, 16, 0)

#endif /* _MSC_VER */
#endif /* __UNIX__ */
 
#include <sys/types.h>
#include <iostream.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#include "omnithread.h"
#include "event.h"

class Poller;

class PollSet_Iterator;
class PollSet_Active_Iterator;
class PollSet: public SocketSet{
 private:
  int maxfds;

#ifdef __WIN32__
  HANDLE *handles;
  socket_t *fds;
  int next;
#else 
  struct fd_set rfds;
#endif
  int maxFD;

  mutable omni_mutex guard;

 public:

  PollSet(const int maxfds);
  PollSet(const PollSet&);
  ~PollSet();

  void add(const socket_t fd);
  void remove(const socket_t fd);

  friend ostream &operator << (ostream& s, const PollSet& set);
  PollSet& operator = (const PollSet &set);

  friend PollSet_Iterator;
  friend PollSet_Active_Iterator;
  friend Poller;

};

class PollSet_Iterator: public SocketSet_Iterator{
 protected:
#ifdef __WIN32__
  socket_t active;
#else
  PollSet &set;
  int index;
#endif
 public:
#ifdef __WIN32__
  PollSet_Iterator(socket_t active);
#else
  PollSet_Iterator(PollSet&);
#endif
  virtual socket_t operator() ();
};

#define PollSet_Active_Iterator PollSet_Iterator


class Poller: public EventMonitor{
  PollSet ps;
  PollSet active_ps;

  omni_mutex ps_guard;

  enum State { awake, asleep };
  State state;
  omni_mutex state_guard;

  int nthreads_waiting;
  omni_condition wakeUP_cond;

#ifdef __WIN32__
  // Event notification is achieved under win32 with an
  // "Event Object"
  HANDLE wakeUpEvent;
#else
  // On Unixoid systems a loopback pipe will suffice
  socket_t *pipe_fd;
#endif

  void wakeUp();
 public:
  Poller(const int maxFDs);

  void add(socket_t);
  void remove(socket_t);

  PollSet_Active_Iterator wait();
  friend ostream &operator <<(ostream& s, const Poller& ps);
};



#endif /* _POLL_H */
