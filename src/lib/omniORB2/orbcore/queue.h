#ifndef _QUEUE_H
#define _QUEUE_H

#include <omnithread.h>


#undef DEBUG

#ifndef PTRACE
#  ifdef DEBUG
#    define QTRACE(prefix,message)  \
  omniORB::logs(15, "Queue " prefix ": " message)
#  else
#    define QTRACE(x)
#  endif
#else
#  define QTRACE(x) PTRACE("Queue", x)
#endif

// Abstract Queue base type for defining the external
// interface behaviour

// The Queue returns actual things, copy constructed 
// from the Queue itself.

template<class T> class Queue {
 private:
  // forbid copy constructor for the Queue itself
  Queue(const Queue&) { }
 protected:
  // superclasses can make and delete instances, but otherwise
  // this is an abstract interface. Is there another way of
  // doing this?
  Queue()  { }
  virtual ~Queue() { }

 public:
  virtual void add(T&) = 0;
  virtual T remove() = 0;
};

// A fixed length, thread safe Queue template
// FIXME: for some (nodoubt silly) reason it wouldn't link properly
// when I defined add and remove out-of-line.
template<class T> class FixedQueue: public Queue<T> {
 public:
  FixedQueue(const int length): Queue(), 
    next_read_index(0), next_write_index(0),
    num_slots(length), used_slots(0),
    pd_cond(&pd_lock) {
      items = new T[length];
  };

  virtual ~FixedQueue() {
    delete[] items;
  };

  void add(T& thing){
    pd_lock.lock();
    {
      if (used_slots == num_slots){
	// queue is full
	QTRACE("Attempt to add item to a full queue\n");
	while (used_slots == num_slots) pd_cond.wait();
      }
      // else we can enter the data
      items[next_write_index++] = thing;
      if (next_write_index == num_slots) next_write_index = 0;
      
      used_slots ++;

      pd_cond.signal();
    }
    pd_lock.unlock();
  }
  T remove(){
    T thing;

    pd_lock.lock();
    {
      if (used_slots == 0){
	// queue is empty
	QTRACE("Attempt to remove item from an empty queue\n");
	while (used_slots == 0) pd_cond.wait();
      }
      thing = items[next_read_index ++];
      if (next_read_index == num_slots) next_read_index = 0;
      
      used_slots --;
      pd_cond.signal();
    }
    pd_lock.unlock();

    return thing;
  }


 private:
  // the queue is represented by an array and a pair of
  // read, write pointers

  T *items;
  int next_read_index, next_write_index;
  int num_slots;
  int used_slots;

  // used to protect internal state
  omni_mutex     pd_lock;
  omni_condition pd_cond;
};

#endif _QUEUE_H
