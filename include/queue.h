#ifndef _QUEUE_H
#define _QUEUE_H

#include <omnithread.h>


#undef DEBUG

#ifdef DEBUG
#  define QTRACE(prefix,message)  \
  cerr << prefix << ": " << message << endl;
#else
#  define QTRACE(x, y)
#endif

const unsigned int max_queue_size = 1048576;

template<class T> class TimedQueue{
 protected:
  unsigned long next_size_up(){
    return num_slots * 2;
  }
  unsigned long next_size_down(){
    return num_slots / 2;
  }
  inline unsigned long empty_slots(){
    return num_slots - used_slots;
  }
  void grow(){
    QTRACE("Q", "growing");
    unsigned long new_size = next_size_up();
    T *new_items = new T[new_size];
    
    unsigned long new_index = 0;
    // assumes full, will copy harmless NULL data if not
    unsigned long n_to_copy = used_slots;
    while (n_to_copy-- > 0){
      new_items[new_index ++] = items[next_read_index];
      next_read_index = (next_read_index + 1) % num_slots;
    }

    delete[] items;
    items = new_items;
    next_read_index = 0;
    next_write_index = new_index;
    num_slots = new_size;
  }


 public:
  TimedQueue(const int initialSize = 1): 
    next_read_index(0), next_write_index(0),
    num_slots(initialSize), used_slots(0),
    pd_add_cond(&pd_lock), pd_remove_cond(&pd_lock){
    QTRACE("Q", "constructed");
    items = new T[initialSize];
  }
  virtual ~TimedQueue(){
    delete[] items;
  }
  void enqueue(T& thing){
    QTRACE("Q", "enqueue");
    omni_mutex_lock lock(pd_lock);

    if (!empty_slots()){
      QTRACE("Q", "enqueue: queue full");
      // queue is full
      if (next_size_up() > max_queue_size){
	// have to block caller
	QTRACE("Q", "enqueue:  +-having to block caller");
	while (!empty_slots()) pd_add_cond.wait();
      }else{
	grow();
	OMNIORB_ASSERT(used_slots != num_slots);
      }
    }
    items[next_write_index] = thing;
    next_write_index = (next_write_index + 1) % num_slots;
    OMNIORB_ASSERT(next_write_index != next_read_index);
    used_slots ++;

    pd_remove_cond.signal();
  }
  // If timeout == 0 then block indefinitely
  // If a timeout occurs, return NULL
  T dequeue(const int timeout = 0){
    QTRACE("Q", "dequeue");
    T thing;
    {
      omni_mutex_lock lock(pd_lock);

      if (used_slots == 0){
	// queue is empty
	QTRACE("Q", "Attempt to remove item from an empty queue\n");
	while (used_slots == 0) {
	  if (!timeout) pd_remove_cond.wait();
	  else{
	    unsigned long secs, nanosecs;
	    omni_thread::get_time(&secs, &nanosecs, timeout, 0);
	    if (!pd_remove_cond.timedwait(secs, nanosecs)){
	      return NULL;
	    }
	  }
	}
      }
      QTRACE("Q", "Got item from queue");
      thing = items[next_read_index ++];
      if (next_read_index == num_slots) next_read_index = 0;
      
      used_slots --;
      // maybe shrink the queue again?
      pd_add_cond.signal();
    }

    return thing;
  }
 private:
  // the queue is represented by an array and a pair of
  // read, write pointers

  T *items;
  unsigned long next_read_index, next_write_index;
  unsigned long num_slots;
  unsigned long used_slots;

  // used to protect internal state
  omni_mutex     pd_lock;
  omni_condition pd_add_cond;    // wakes up people wanting to add items
  omni_condition pd_remove_cond; // wakes up people wanting to remove items

};


#endif _QUEUE_H
