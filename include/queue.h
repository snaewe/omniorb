#ifndef _QUEUE_H
#define _QUEUE_H

#include <omnithread.h>

#define QTRACE(prefix, message) omniORB::logs(10, prefix ": " message)

// Should the queue be bounded? If an enqueue() call fails then the calling
// thread will have to be blocked indefinitely.
const unsigned int max_queue_size = 1048576;

// A bounded-length queue built out of a dynamic array which provides
// a timedwait-based dequeue method.
template<class T> class TimedQueue{
 protected:
  // Queue can resize it's internal array
  unsigned long next_size_up(){
    return num_slots * 2;
  }
  unsigned long next_size_down(){
    return num_slots / 2;
  }
  inline unsigned long empty_slots(){
    return num_slots - used_slots;
  }
  // Make the internal array the next size up (must be holding the
  // state lock)
  void grow(){
    QTRACE("Q", "growing");
    unsigned long new_size = next_size_up();
    T *new_items = new T[new_size];

    // copy all existing data into new array
    unsigned long new_write_index = 0;
    unsigned long n_to_copy = used_slots;
    while (n_to_copy-- > 0){
      new_items[new_write_index ++] = items[next_read_index];
      next_read_index = (next_read_index + 1) % num_slots;
    }

    delete[] items;
    items = new_items;
    num_slots = new_size;

    next_read_index = 0;
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
      // queue is full- either resize or block the caller.
      if (next_size_up() > max_queue_size){
	QTRACE("Q", "enqueue:  +-having to block caller");
	while (!empty_slots()) pd_add_cond.wait();
      }else{
	grow();
	OMNIORB_ASSERT(empty_slots());
      }
    }
    items[next_write_index] = thing;
    next_write_index = (next_write_index + 1) % num_slots;
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
      thing = items[next_read_index];
      next_read_index = (next_read_index + 1) % num_slots;      
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
