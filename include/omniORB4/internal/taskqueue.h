// -*- Mode: C++; -*-
//                            Package   : omniORB
// taskqueue.h                Created on: 16/8/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//

/*
  $Log$
  Revision 1.1.2.1  2001/02/23 16:47:00  sll
  Added new files.

  Revision 1.2.2.1  2000/07/17 10:35:59  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:55  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.2  2000/02/07 15:26:59  dpg1
  Spurious \r characters removed.

  Revision 1.1.2.1  1999/09/22 14:27:09  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __OMNIORB_TASKQUEUE_H__
#define __OMNIORB_TASKQUEUE_H__

OMNI_NAMESPACE_BEGIN(omni)

class omniTaskQueue : public omni_thread {
public:
  class Task;
  typedef void (*TaskFn)(void*);

  virtual ~omniTaskQueue();
  omniTaskQueue();
  void insert(Task*);
  void insert(TaskFn, void*);
  void die();

  class Task {
  public:
    inline Task() : pd_next(0) {}
    virtual ~Task();
    virtual void doit() = 0;

  private:
    friend class omniTaskQueue;
    Task(const Task&);
    Task& operator=(const Task&);

    Task* pd_next;
  };

private:
  virtual void* run_undetached(void*);

  omni_tracedmutex     pd_lock;
  omni_tracedcondition pd_cond;
  Task*                pd_taskq;
  Task*                pd_taskqtail;
  int                  pd_dying;
};

OMNI_NAMESPACE_END(omni)


#endif  // __OMNIORB_TASKQUEUE_H__
