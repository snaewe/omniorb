#ifndef _EVENT_H
#define _EVENT_H

#include <iostream.h>

typedef int socket_t;

class SocketSet_Iterator;
class SocketSet{ /* abstract interface */
 public:
  
  void add(socket_t);
  void remove(socket_t);
  
  SocketSet_Iterator operator () ();
  friend ostream &operator << (ostream &, const SocketSet&);

};

class SocketSet_Iterator{
 public:
  socket_t next();
  void setActive();
  void clrActive();

  friend SocketSet;
};

class EventMonitor{ 
 public:
  EventMonitor() { }

  void add(socket_t);
  void remove(socket_t);
  SocketSet_Iterator& wait();

  //friend ostream &operator << (ostream &, const EventMonitor&);

 private:
  EventMonitor(const EventMonitor&);
};

#endif /* _EVENT_H */





