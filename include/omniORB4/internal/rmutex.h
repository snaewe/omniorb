// -*- Mode: C++; -*-
//                            Package   : omniORB
// rmutex.h                   Created on: 2001/06/27
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Research Cambridge
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
//    Recursive mutex, to be used by single-thread policy POAs
//

#ifndef __OMNI_RMUTEX_H_
#define __OMNI_RMUTEX_H_

#include <omnithread.h>


// omni_rmutex is similar to omni_tracedmutex, except that it permits
// a recursive lock rather than complaining about it.

class omni_rmutex {
public:
  omni_rmutex();
  ~omni_rmutex();

  void lock();
  void unlock();
  inline void acquire(void) { lock();   }
  inline void release(void) { unlock(); }

private:
  omni_rmutex(const omni_rmutex&);
  omni_rmutex& operator=(const omni_rmutex&);
  // Not implemented

  omni_mutex     pd_lock;   // protects other members
  omni_condition pd_cond;   // so can wait for mutex to unlock
  omni_thread*   pd_holder; // the thread holding pd_m, or 0
  int            pd_depth;  // number of recursive calls by locking thread
  int            pd_dummy;  // true if a dummy thread was created
};


class omni_rmutex_lock {
public:
  inline omni_rmutex_lock(omni_rmutex& m) : pd_m(m) { m.lock(); }
  inline ~omni_rmutex_lock() { pd_m.unlock(); }

private:
  omni_rmutex_lock(const omni_rmutex_lock&);
  omni_rmutex_lock& operator = (const omni_rmutex_lock&);

  omni_rmutex& pd_m;
};


// omni_optional_rlock differs from omni_optional_lock in that it
// takes a pointer to an rlock, not a reference. This is so it can be
// used with a zero call lock in the POA.

class omni_optional_rlock {
public:
  inline omni_optional_rlock(omni_rmutex* m, int locked,
			     int locked_on_exit)
    : pd_locked(locked_on_exit), pd_m(m)
    { if( !locked ) pd_m->lock(); }

  inline ~omni_optional_rlock() { if( !pd_locked )  pd_m->unlock(); }

private:
  omni_optional_rlock(const omni_optional_rlock&);
  omni_optional_rlock& operator = (const omni_optional_rlock&);

  int          pd_locked;
  omni_rmutex* pd_m;
};



#endif // __OMNI_RMUTEX_H_
