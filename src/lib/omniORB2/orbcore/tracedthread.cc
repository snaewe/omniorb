// -*- Mode: C++; -*-
//                            Package   : omniORB
// tracedthread.cc            Created on: 15/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996, 1999 AT&T Research Cambridge
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
//    Implementation of methods defined in class omni.
//      
 
/*
  $Log$
  Revision 1.1.2.2  1999/10/13 12:46:02  djr
  Work-around for dodgy threads implementation.

  Revision 1.1.2.1  1999/09/22 14:27:12  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB3/tracedthread.h>


//////////////////////////////////////////////////////////////////////
////////////////////////// omni_tracedmutex //////////////////////////
//////////////////////////////////////////////////////////////////////

#define BOMB_OUT()  throw *(int*)0

#ifdef OMNIORB_ENABLE_LOCK_TRACES

omni_tracedmutex::omni_tracedmutex()
  : pd_cond(&pd_lock),
    pd_holder(0)
{
}


omni_tracedmutex::~omni_tracedmutex()
{
  if( pd_holder ) {
    omniORB::log <<
      "omniORB: Assertion failed -- mutex destroyed whilst held.\n"
      " This is a bug in omniORB. Please submit a report (with stack\n"
      " trace if possible) to <omniorb@uk.research.att.com>.\n";
    omniORB::log.flush();

    BOMB_OUT();
  }
}


void
omni_tracedmutex::lock()
{
  omni_thread* me = omni_thread::self();

  omni_mutex_lock sync(pd_lock);

  if( pd_holder && pd_holder == me ) {
    omniORB::log <<
      "omniORB: Assertion failed -- attempt to lock mutex when already held.\n"
      " This is a bug in omniORB. Please submit a report (with stack\n"
      " trace if possible) to <omniorb@uk.research.att.com>.\n";
    omniORB::log.flush();

    BOMB_OUT();
  }

  while( pd_holder )  pd_cond.wait();

  pd_m.lock();
  pd_holder = me;
}


void
omni_tracedmutex::unlock()
{
  omni_thread* me = omni_thread::self();

  {
    omni_mutex_lock sync(pd_lock);

    if( pd_holder != me ) {
      omniORB::log <<
	"omniORB: Assertion failed -- attempt to unlock mutex not held.\n"
	" This is a bug in omniORB. Please submit a report (with stack\n"
	" trace if possible) to <omniorb@uk.research.att.com>.\n";
      omniORB::log.flush();

      BOMB_OUT();
    }

    pd_m.unlock();
    pd_holder = 0;
  }
  pd_cond.signal();
}


void
omni_tracedmutex::assert_held(const char* file, int line, int yes)
{
  {
    omni_mutex_lock sync(pd_lock);

    omni_thread* me = omni_thread::self();

    if(  yes && pd_holder == me || !yes && pd_holder != me )
      return;

    // This is needed for VMS Alpha.  omni_thread::self() is unreliable
    // once we have returned from main, and returns 0.
    if( !me )  return;
  }

  omniORB::log << "omniORB: Assertion failed -- " <<
    (yes ? "mutex is not held.\n" : "mutex should not be held.\n") <<
    " This is a bug in omniORB. Please submit a report (with stack\n"
    " trace if possible) to <omniorb@uk.research.att.com>.\n"
    "   file: " << file << "\n"
    "   line: " << line << "\n";
  omniORB::log.flush();

  BOMB_OUT();
}

//////////////////////////////////////////////////////////////////////
//////////////////////// omni_tracedcondition ////////////////////////
//////////////////////////////////////////////////////////////////////

omni_tracedcondition::omni_tracedcondition(omni_tracedmutex* m)
  : pd_mutex(*m), pd_cond(&m->pd_lock), pd_n_waiters(0)
{
  if( !m ) {
    omniORB::log <<
      "omniORB: Assertion failed -- omni_tracedcondition initialised with\n"
      " a nil mutex argument!\n"
      " This is a bug in omniORB. Please submit a report (with stack\n"
      " trace if possible) to <omniorb@uk.research.att.com>.\n";
    omniORB::log.flush();

    BOMB_OUT();
  }
}


omni_tracedcondition::~omni_tracedcondition()
{
  if( pd_n_waiters ) {
    omniORB::log <<
      "omniORB: WARNING -- an omni_tracedcondition was destroyed,\n"
      " but there are still threads waiting on it!\n";
    omniORB::log.flush();
  }
}


void
omni_tracedcondition::wait()
{
  omni_thread* me = omni_thread::self();

  omni_mutex_lock sync(pd_mutex.pd_lock);

  if( pd_mutex.pd_holder != me ) {
    omniORB::log <<
      "omniORB: Assertion failed -- attempt to wait on condition variable,\n"
      " but the calling thread does not hold the associated mutex.\n"
      " This is a bug in omniORB. Please submit a report (with stack\n"
      " trace if possible) to <omniorb@uk.research.att.com>.\n";
    omniORB::log.flush();

    BOMB_OUT();
  }

  pd_mutex.pd_m.unlock();
  pd_mutex.pd_holder = 0;
  pd_mutex.pd_cond.signal();
  pd_n_waiters++;
  pd_cond.wait();
  pd_n_waiters--;
  while( pd_mutex.pd_holder )  pd_mutex.pd_cond.wait();
  pd_mutex.pd_m.lock();
  pd_mutex.pd_holder = me;
}


int
omni_tracedcondition::timedwait(unsigned long secs, unsigned long nanosecs)
{
  omni_thread* me = omni_thread::self();

  omni_mutex_lock sync(pd_mutex.pd_lock);

  if( pd_mutex.pd_holder != me ) {
    omniORB::log <<
      "omniORB: Assertion failed -- attempt to wait on condition variable,\n"
      " but the calling thread does not hold the associated mutex.\n"
      " This is a bug in omniORB. Please submit a report (with stack\n"
      " trace if possible) to <omniorb@uk.research.att.com>.\n";
    omniORB::log.flush();

    BOMB_OUT();
  }

  pd_mutex.pd_m.unlock();
  pd_mutex.pd_holder = 0;
  pd_mutex.pd_cond.signal();
  pd_n_waiters++;
  int ret = pd_cond.timedwait(secs, nanosecs);
  pd_n_waiters--;
  while( pd_mutex.pd_holder )  pd_mutex.pd_cond.wait();
  pd_mutex.pd_m.lock();
  pd_mutex.pd_holder = me;

  return ret;
}


void
omni_tracedcondition::signal()
{
#if 0
  // This is not generally desirable, since it
  // bombs out even for perfectly legal code!

  if( omniORB::traceLevel > 5 ) {
    // Check to see if we hold the mutex, and issue a
    // warning if so, since it is inefficient.
    CORBA::Boolean mutex_held = 0;
    { omni_mutex_lock sync(pd_mutex.pd_lock);
      if( pd_mutex.pd_holder == omni_thread::self() )
	mutex_held = 1;
    }
    if( mutex_held ) {
      omniORB::log <<
	"omniORB: WARNING -- signalling a condition variable whilst holding\n"
	" the associated mutex may be inefficient.\n"
      omniORB::log.flush();
      BOMB_OUT();
    }
  }
#endif

  pd_cond.signal();
}


void
omni_tracedcondition::broadcast()
{
  pd_cond.broadcast();
}

#endif
