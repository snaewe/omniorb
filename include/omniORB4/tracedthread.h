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
  Revision 1.5.2.1  2003/03/23 21:03:59  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.2.2.4  2002/04/28 20:39:46  dgrisby
  autoconf, install location tweaks.

  Revision 1.2.2.3  2001/12/03 18:47:39  dpg1
  Detect use after deletion in traced mutex and condition.

  Revision 1.2.2.2  2001/08/17 13:49:08  dpg1
  Optional logging for traced mutexes and condition variables.

  Revision 1.2.2.1  2000/07/17 10:35:38  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:04  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.2  2000/02/09 12:06:37  djr
  Additional checks for tracedmutex/conditions.
  Removed superflouous member of omni_tracedmutex.

  Revision 1.1.2.1  1999/09/24 09:51:57  djr
  Moved from omniORB2 + some new files.

*/

#ifndef __OMNI_TRACEDTHREAD_H__
#define __OMNI_TRACEDTHREAD_H__


// Lock tracing is controlled by autoconf, or in the dummy
// omniconfig.h.  You can override it here if you wish.

//#define OMNIORB_ENABLE_LOCK_TRACES


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

  void log(const char* name);
  // Call this to cause logging messages whenever the mutex is locked
  // or unlocked. The storage associated with name must exist until
  // the mutex is deleted, or log(0) is called.

private:
  friend class omni_tracedcondition;

  omni_tracedmutex(const omni_tracedmutex&);
  omni_tracedmutex& operator=(const omni_tracedmutex&);

  omni_mutex     pd_lock;    // protects other members
  omni_condition pd_cond;    // so can wait for mutex to unlock
  omni_thread*   pd_holder;  // the thread holding pd_m, or 0
  int            pd_n_conds; // number of dependent condition vars
  int            pd_deleted; // set true on deletion, may catch later use
  const char*    pd_logname; // if non-zero, name to use for logging
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

  void log(const char* name);
  // Call this to cause logging messages whenever the condition is
  // signalled or waited upon. The storage associated with name must
  // exist until the mutex is deleted, or log(0) is called.

private:
  omni_tracedcondition(const omni_tracedcondition&);
  omni_tracedcondition& operator=(const omni_tracedcondition&);

  omni_tracedmutex& pd_mutex;
  omni_condition    pd_cond;
  int               pd_n_waiters;
  int               pd_deleted;
  const char*       pd_logname;
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
