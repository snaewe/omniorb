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
//    omni_thread style mutex and condition variables with checks.
//

/*
  $Log$
  Revision 1.1.2.4  2000/02/10 18:28:22  djr
  omni_tracedmutex/condition now survive if omni_thread::self() returns 0.

  Revision 1.1.2.3  2000/02/09 12:06:37  djr
  Additional checks for tracedmutex/conditions.
  Removed superflouous member of omni_tracedmutex.

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

static const char* bug_msg =
  " This is a bug in omniORB. Please submit a report (with stack\n"
  " trace if possible) to <omniorb@uk.research.att.com>.\n";


omni_tracedmutex::omni_tracedmutex()
  : pd_cond(&pd_lock),
    pd_holder(0),
    pd_n_conds(0)
{
}


omni_tracedmutex::~omni_tracedmutex()
{
  if( pd_holder ) {
    omniORB::log <<
      "omniORB: Assertion failed -- mutex destroyed whilst held.\n" <<
      bug_msg;
    omniORB::log.flush();

    BOMB_OUT();
  }
  if( pd_n_conds != 0 ) {
    omniORB::log <<
      "omniORB: Assertion failed -- mutex destroyed whilst still being used\n"
      " by a condition variable.\n" << bug_msg;
    omniORB::log.flush();

    BOMB_OUT();
  }
}


void
omni_tracedmutex::lock()
{
  omni_thread* me = omni_thread::self();

  omni_mutex_lock sync(pd_lock);

  if( me && pd_holder == me ) {
    omniORB::log <<
      "omniORB: Assertion failed -- attempt to lock mutex when already held.\n"
      << bug_msg;
    omniORB::log.flush();

    BOMB_OUT();
  }

  while( pd_holder )  pd_cond.wait();

  pd_holder = me ? me : (omni_thread*) 1;
}


void
omni_tracedmutex::unlock()
{
  omni_thread* me = omni_thread::self();

  {
    omni_mutex_lock sync(pd_lock);

    if( !pd_holder || me && pd_holder != me ) {
      omniORB::log <<
	"omniORB: Assertion failed -- attempt to unlock mutex not held.\n" <<
	bug_msg;
      omniORB::log.flush();

      BOMB_OUT();
    }

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

    if(  yes && pd_holder == me || !yes && pd_holder != me || !me )
      return;
  }

  omniORB::log << "omniORB: Assertion failed -- " <<
    (yes ? "mutex is not held.\n" : "mutex should not be held.\n") <<
    bug_msg <<
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
      " a nil mutex argument!\n" << bug_msg;
    omniORB::log.flush();

    BOMB_OUT();
  }
  pd_mutex.pd_n_conds++;
}


omni_tracedcondition::~omni_tracedcondition()
{
  if( pd_n_waiters ) {
    omniORB::log <<
      "omniORB: WARNING -- an omni_tracedcondition was destroyed,\n"
      " but there are still threads waiting on it!\n";
    omniORB::log.flush();
  }
  pd_mutex.pd_n_conds--;
}


void
omni_tracedcondition::wait()
{
  omni_thread* me = omni_thread::self();

  omni_mutex_lock sync(pd_mutex.pd_lock);

  if( me && pd_mutex.pd_holder != me ) {
    omniORB::log <<
      "omniORB: Assertion failed -- attempt to wait on condition variable,\n"
      " but the calling thread does not hold the associated mutex.\n" <<
      bug_msg;
    omniORB::log.flush();

    BOMB_OUT();
  }

  pd_mutex.pd_holder = 0;
  pd_mutex.pd_cond.signal();
  pd_n_waiters++;
  pd_cond.wait();
  pd_n_waiters--;
  while( pd_mutex.pd_holder )  pd_mutex.pd_cond.wait();
  pd_mutex.pd_holder = me ? me : (omni_thread*) 1;
}


int
omni_tracedcondition::timedwait(unsigned long secs, unsigned long nanosecs)
{
  omni_thread* me = omni_thread::self();

  omni_mutex_lock sync(pd_mutex.pd_lock);

  if( me && pd_mutex.pd_holder != me ) {
    omniORB::log <<
      "omniORB: Assertion failed -- attempt to wait on condition variable,\n"
      " but the calling thread does not hold the associated mutex.\n" <<
      bug_msg;
    omniORB::log.flush();

    BOMB_OUT();
  }

  pd_mutex.pd_holder = 0;
  pd_mutex.pd_cond.signal();
  pd_n_waiters++;
  int ret = pd_cond.timedwait(secs, nanosecs);
  pd_n_waiters--;
  while( pd_mutex.pd_holder )  pd_mutex.pd_cond.wait();
  pd_mutex.pd_holder = me ? me : (omni_thread*) 1;

  return ret;
}


void
omni_tracedcondition::signal()
{
  pd_cond.signal();
}


void
omni_tracedcondition::broadcast()
{
  pd_cond.broadcast();
}


#endif  // ifdef OMNIORB_ENABLE_LOCK_TRACES
