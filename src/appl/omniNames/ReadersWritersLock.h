//
// This class implements a multiple-readers, single-writer lock.  It has
// a slight alteration from a standard such lock.  Any thread which has already
// called writerIn() may call writerIn() or readerIn() again multiple times.
//


#ifndef _ReadersWritersLock_h_
#define _ReadersWritersLock_h_

#include <omnithread.h>

class ReadersWritersLock {

public:

  omni_mutex m;
  omni_condition c;
  int n;	// 0 means no-one active, > 0 means n readers, < 0 means writer
		// (-n times).
  int writerId;

  ReadersWritersLock(void) : c(&m), n(0), writerId(0) {}

  void readerIn(void) {
    m.lock();
    if ((n < 0) && (writerId == omni_thread::self()->id())) {
      // this thread already has lock as writer, simply decrement n
      n--;
      m.unlock();
      return;
    }
    while (n < 0)
      c.wait();
    n++;
    m.unlock();
  }

  void readerOut(void) {
    m.lock();
    if (n < 0) {
      // this thread already had lock as writer, simply increment n
      n++;
      m.unlock();
      return;
    }
    n--;
    if (n == 0)
      c.signal();
    m.unlock();
  }

  void writerIn(void) {
    m.lock();
    if ((n < 0) && (writerId == omni_thread::self()->id())) {
      // this thread already has lock as writer, simply decrement n
      n--;
      m.unlock();
      return;
    }
    while (n != 0)
      c.wait();
    n--;
    writerId = omni_thread::self()->id();
    m.unlock();
  }

  void writerOut(void) {
    m.lock();
    n++;
    if (n == 0)
      c.broadcast();	// might as well wake up all readers
    m.unlock();
  }

};


//
// As an alternative to:
// {
//   lock.readerIn();
//   .....
//   lock.readerOut();
// }
//
// you can use a single instance of the ReaderLock class:
//
// {
//   ReaderLock r(lock);
//   ....
// }
//
// This has the advantage that lock.readerOut() will be called automatically
// when an exception is thrown.
//

class ReaderLock {
  ReadersWritersLock& rwl;
public:
  ReaderLock(ReadersWritersLock& l) : rwl(l) { rwl.readerIn(); }
  ~ReaderLock(void) { rwl.readerOut(); }
};


//
// Similarly the WriterLock class can be used instead of lock.writerIn() and
// lock.writerOut().
//

class WriterLock {
  ReadersWritersLock& rwl;
public:
  WriterLock(ReadersWritersLock& l) : rwl(l) { rwl.writerIn(); }
  ~WriterLock(void) { rwl.writerOut(); }
};

#endif
