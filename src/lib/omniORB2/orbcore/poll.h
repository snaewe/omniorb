#ifndef _POLL_H
#define _POLL_H

#include <sys/poll.h>
#include <unistd.h>
#include <iostream.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#include "omnithread.h"
#include "event.h"

#define SOCKET_UNDEFINED -1

#define MAXFDS (getdtablesize())

class Poller;

class PollSet_Iterator;
class PollSet_Active_Iterator;
class PollSet: public SocketSet{
 private:
  int maxfds;
  int nactive; // number of active entries in the array
  struct pollfd *ufds;
  mutable omni_mutex guard;

  int where(socket_t);

  PollSet(const PollSet&);
 public:

  PollSet(const int maxfds);
  ~PollSet();

  void add(const socket_t fd);
  void remove(const socket_t fd);

  friend ostream &operator << (ostream& s, const PollSet& set);

  friend PollSet_Iterator;
  friend PollSet_Active_Iterator;
  friend Poller;

};

class PollSet_Iterator: public SocketSet_Iterator{
 protected:
  PollSet &set;
  int index;
 public:
  PollSet_Iterator(PollSet&);
  virtual socket_t operator() ();
};

class PollSet_Active_Iterator: public PollSet_Iterator{
 public:
  PollSet_Active_Iterator(PollSet&);
  socket_t operator() ();
};


class Poller: public EventMonitor{
  PollSet ps;
  omni_mutex ps_guard;

  enum State { awake, asleep };
  State state;
  omni_mutex state_guard;

  int nthreads_waiting;
  omni_condition wakeUP_cond;

  socket_t *pipe_fd;

  void wakeUp();
 public:
  Poller();

  void add(socket_t);
  void remove(socket_t);

  PollSet_Active_Iterator wait();
  friend ostream &operator <<(ostream& s, const Poller& ps);
};



#endif /* _POLL_H */
