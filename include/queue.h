
#include <omnithread.h>

// Queue implementation is a doubly-linked list of Elements supporting
// the following interface:
//
//   Queue<T>::enqueue(T datum)  // adds "datum" to the queue
//                               // guaranteed non-blocking if queue is
//                               // unbounded. Otherwise might block.
//   T Queue<T>::dequeue(timeout)// attempts to remove an item from the 
//                               // queue, will block if the queue is empty.
//                               // "timeout" bounds the wait time, returning
//                               // NULL if nothing was removed.

template<class T> class Element{
  T           pd_data;
  Element<T> *pd_next;
  Element<T> *pd_prev;
public:
  Element(T d, Element<T> *prev, Element<T> *next): 
    pd_data(d), pd_next(next), pd_prev(prev) {}
  T            data(){ return pd_data; }
  Element<T>  *next(){ return pd_next; }
  Element<T>  *prev(){ return pd_prev; }
  void        prev(Element<T> *prev){ pd_prev = prev; }
};

template<class T> class Queue{
  // all accesses to the internals are protected with a mutex
  omni_mutex pd_lock;  

  Element<T> *pd_hd; // pointer to the head of the queue
  Element<T> *pd_tl; // pointer to the tail

  int pd_length;     // # elements in the queue
  int pd_bound;      // max # elements in the queue (or -1 for unbounded)
  omni_condition  pd_readers; // used to wake up threads blocked in dequeue()
  omni_condition  pd_writers; // used to wake up threads blocked in enqueue()


public:
  Queue(const int bound = -1): pd_hd(NULL), pd_tl(NULL),
    pd_length(0), pd_bound(bound), 
    pd_readers(&pd_lock), pd_writers(&pd_lock){ }
  
  void enqueue(T d){
    // If queue is at the bound, wait for an element to be removed.
    omni_mutex_lock lock(pd_lock);
    while (pd_length == pd_bound) pd_writers.wait();

    Element<T> *e = new Element<T>(d, pd_hd, NULL);
    if (pd_length == 0){
      pd_hd = pd_tl = e;
    }else{
      pd_hd->prev(e);
      pd_hd = e;
    }
    pd_length++;

    pd_readers.signal();
  }

  T dequeue(const unsigned int timeout){
    omni_mutex_lock lock(pd_lock);

    while (pd_length == 0) {
      unsigned long secs, nanosecs;
      omni_thread::get_time(&secs, &nanosecs, timeout, 0);
      if (!pd_readers.timedwait(secs, nanosecs)){
	return NULL;
      }
    }

    Element<T> *e = pd_tl;
    T d = e->data();
    if (pd_length == 1){
      pd_tl = pd_hd = NULL;
    }else{
      pd_tl = e->prev();
    }
    delete e;
    pd_length--;

    pd_writers.signal();
    return d;
  }
};
