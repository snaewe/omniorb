// -*- Mode: C++; -*-
//                            Package   : omniORB2
// tracedthread.h             Created on: 15/6/99
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
//    omni_thread style mutex and condition variables with checks.
//

/*
  $Log$
  Revision 1.2  2000/07/04 15:23:29  dpg1
  Merge from omni3_develop.

  Revision 1.1.2.2  2000/02/09 12:06:37  djr
  Additional checks for tracedmutex/conditions.
  Removed superflouous member of omni_tracedmutex.

  Revision 1.1.2.1  1999/09/24 09:51:57  djr
  Moved from omniORB2 + some new files.

*/

#ifndef __OMNI_TRACEDTHREAD_H__
#define __OMNI_TRACEDTHREAD_H__


#define OMNIORB_ENABLE_LOCK_TRACES


//////////////////////////////////////////////////////////////////////
////////////////////////// omni_tracedmutex //////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef OMNIORB_ENABLE_LOCK_TRACES

#define ASSERT_OMNI_TRACEDMUTEX_HELD(m, yes)
typedef omni_mutex      omni_tracedmutex;
typedef omni_mutex_lock omni_tracedmutex_lock;
typedef omni_condition  omni_tracedcondition;

#else

class omni_tracedcondition;


class omni_tracedmutex {
public:
  omni_tracedmutex();
  ~omni_tracedmutex();

  void lock();
  void unlock();
  inline void acquire(void) { lock();   }
  inline void release(void) { unlock(); }

  void assert_held(const char* file, int line, int yes);

private:
  friend class omni_tracedcondition;

  omni_tracedmutex(const omni_tracedmutex&);
  omni_tracedmutex& operator=(const omni_tracedmutex&);

  omni_mutex     pd_lock;    // protects other members
  omni_condition pd_cond;    // so can wait for mutex to unlock
  omni_thread*   pd_holder;  // the thread holding pd_m, or 0
  int            pd_n_conds; // number of dependent condition vars
};

//////////////////////////////////////////////////////////////////////
//////////////////////// omni_tracedcondition ////////////////////////
//////////////////////////////////////////////////////////////////////

class omni_tracedcondition {
public:
  omni_tracedcondition(omni_tracedmutex* m);
  ~omni_tracedcondition();

  void wait();
  int timedwait(unsigned long secs, unsigned long nanosecs = 0);
  void signal();
  void broadcast();

private:
  omni_tracedcondition(const omni_tracedcondition&);
  omni_tracedcondition& operator=(const omni_tracedcondition&);

  omni_tracedmutex& pd_mutex;
  omni_condition    pd_cond;
  int               pd_n_waiters;
};

//////////////////////////////////////////////////////////////////////
//////////////////////// omni_tracedmutex_lock ///////////////////////
//////////////////////////////////////////////////////////////////////

class omni_tracedmutex_lock {
public:
  inline omni_tracedmutex_lock(omni_tracedmutex& m) :pd_m(m) { m.lock(); }
  inline ~omni_tracedmutex_lock() { pd_m.unlock(); }

private:
  omni_tracedmutex_lock(const omni_tracedmutex_lock&);
  omni_tracedmutex_lock& operator = (const omni_tracedmutex_lock&);

  omni_tracedmutex& pd_m;
};


#define ASSERT_OMNI_TRACEDMUTEX_HELD(m, yes)  \
  (m).assert_held(__FILE__, __LINE__, (yes))

// #ifndef OMNIORB_ENABLE_LOCK_TRACES
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////// omni_optional_lock /////////////////////////
//////////////////////////////////////////////////////////////////////

class omni_optional_lock {
public:
  inline omni_optional_lock(omni_tracedmutex& m, int locked,
			    int locked_on_exit)
    : pd_locked(locked_on_exit), pd_m(m)
    { if( !locked ) pd_m.lock(); }

  inline ~omni_optional_lock() { if( !pd_locked )  pd_m.unlock(); }

private:
  omni_optional_lock(const omni_optional_lock&);
  omni_optional_lock& operator = (const omni_optional_lock&);

  int               pd_locked;
  omni_tracedmutex& pd_m;
};


#endif  // __OMNITRACEDTHREAD_H__
