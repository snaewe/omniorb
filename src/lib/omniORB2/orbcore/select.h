#ifndef _SELECT_H
#define _SELECT_H

#include <unistd.h>
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

  struct fd_set rfds;
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
  PollSet &set;
  int index;
 public:
  PollSet_Iterator(PollSet&);
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
