// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniAsyncInvoker.h         Created on: 20 Dec 2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//	*** PROPRIETORY INTERFACE ***
// 

/*
  $Log$
  Revision 1.1.4.1  2001/04/19 09:47:54  sll
  New library omniAsyncInvoker.

  Revision 1.1.2.1  2001/02/23 16:47:09  sll
  Added new files.

*/

#ifndef __OMNIASYNCINVOKER_H__
#define __OMNIASYNCINVOKER_H__

// Usage:
//
//    An omniAsyncInvoker, or an invoker in short, is an object that
//    executes commands asynchronously using a pool of threads it manages
//    internally.
//
//    The command to be executed is an omniTask instance. The omniTask
//    class is based on the Command Pattern (Design Patterns, chapter 5).
// 
//    The client creates an instance of the omniTask and registers it with
//    the omniAsyncInvoker using the insert() method. The invoker will
//    dispatch a thread to call the execute() method in the omniTask object.
//    Notice that the call to the execute() method is always done by
//    another thread, hence giving the invoker its asynchronous nature.
//
//    Depend on the category of an omniTask, the invoker will choose a
//    thread to execute the task in one of the following ways:
//
//       Anytime: the task will be executed by one of the threads in the
//                pool. If no thread is available, the task may be queued
//                indefinitely until a thread is available
//
//       DedicatedThread: the task is executed by a particular thread. This
//                        is particularly useful if the task involves calling
//                        into a non-thread safe library and it is desirable
//                        to always use the same thread to do so.
//
//                        The thread that will execute the command is always
//                        the thread that calls the invoker's perform()
//                        method.
//
//                        Notice that the task may never be dispatched if
//                        no external thread calls perform().
//
//       ImmediateDispatch: A thread will be dispatched to execute the task
//                          immediately.
//
//                          To do so the invoker may have to spawn a new
//                          thread to do the work. If the invoker fails to
//                          spawn a thread for various reasons, the insert()
//                          call fails.
//
//   Once the execute() method is called on a task, the invoker will forget
//   about it. If the task object is heap allocated, it has to be garbage
//   collected by some external means. The simplist approach is to delete
//   the task object before the execute() method returns.
//
//   Once inserted, a task may be cancelled by calling the cancel() method.
//   However, this call only has an effect if the task is still sitting in a
//   queue waiting for its turn to be executed.

#include <omnithread.h>


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
class omniTaskLink {
public:
  omniTaskLink* next;
  omniTaskLink* prev;

  omniTaskLink() : next(this), prev(this) {}

  void enq(omniTaskLink& head);
  void deq();
  static unsigned int is_empty(omniTaskLink& head);

private:
  omniTaskLink(const omniTaskLink&);
  omniTaskLink& operator=(const omniTaskLink&);
};

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
class omniTask : public omniTaskLink {
 public:

  enum Category { AnyTime, 
		  ImmediateDispatch, 
		  DedicatedThread
  };

  virtual void execute() = 0;

  omniTask(Category cat = AnyTime) : pd_category(cat) {}
  virtual ~omniTask() {}

  Category category() { return pd_category; }
  void category(Category c) { pd_category = c; }

 private:
  Category  pd_category;

  omniTask();
  omniTask(const omniTask&);
  omniTask& operator=(const omniTask&);
};

class omniAsyncInvoker;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
class omniAsyncInvoker {
 public:
  omniAsyncInvoker(unsigned int max=10000);
  // <max> specifies the maximum number of threads the object should
  // spawn to perform tasks in the Anytime category.

  ~omniAsyncInvoker();
  // Returns only when all the threads doing Anytime tasks have exited.
  // Notice that any tasks still sitting on the pending queue will be
  // discarded quietly.

  int insert(omniTask*);
  // insert the task into the pending queue. The task will be dispatched
  // according to its category.
  //
  // returns 0 if the task cannot be inserted.
  // returns 1 if the task has been inserted successfully.

  int cancel(omniTask*);
  // returns 0 if the task is not found in the pending queue
  // returns 1 if the task is successfully removed from the pending queue.


  void perform(int polling = 0);
  // Execute the tasks that is in the DedicatedThread category.
  // if polling is non-zero, blocks within this method until a task is 
  // executed.
  // else returns immediately if there is no task to execute.

  friend class omniAsyncWorker;

  static unsigned int idle_timeout; // No. of seconds before an idle thread
                                    // has to wait before it exits.
                                    // default is 10 seconds.

  static unsigned int traceLevel;   // control how much trace message is
                                    // emitted to stderr. The higher the no.
                                    // the more verbose it is. Default is 1.

 private:

  unsigned int      pd_keep_working; // 0 means all threads should exit.
  omni_mutex*       pd_lock;
  omni_condition*   pd_cond;        // signal this conditional when all 
                                    // the threads serving Anytime tasks are
                                    // exiting.
  omniTaskLink      pd_anytime_tq;  // Anytime tasks 
  omniTaskLink      pd_dedicate_tq; // Dedicated thread tasks
  omniAsyncWorker*  pd_idle_threads; // idle threads ready for Anytime tasks
  unsigned int      pd_nthreads;   // No. of threads serving Anytime tasks
  unsigned int      pd_maxthreads; // Max. no. of threads serving Anytime tasks
  unsigned int      pd_totalthreads; // total no. of threads.
  unsigned int      pd_dthreads;   // no. of dedicated threads.
};

#endif // __OMNIASYNCINVOKER_H__
