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
  Revision 1.2.2.7  2002/03/14 15:12:33  dpg1
  More logging for tracedmutexes/conditions.

  Revision 1.2.2.6  2001/12/03 18:47:39  dpg1
  Detect use after deletion in traced mutex and condition.

  Revision 1.2.2.5  2001/09/19 17:29:04  dpg1
  Cosmetic changes.

  Revision 1.2.2.4  2001/08/17 17:07:06  sll
  Remove the use of omniORB::logStream.

  Revision 1.2.2.3  2001/08/17 13:49:08  dpg1
  Optional logging for traced mutexes and condition variables.

  Revision 1.2.2.2  2000/09/27 17:35:50  sll
  Updated include/omniORB3 to include/omniORB4

  Revision 1.2.2.1  2000/07/17 10:36:00  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:54  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.5  2000/06/02 14:18:26  dpg1
  Race condition in tracedcondition

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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/tracedthread.h>


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
    pd_n_conds(0),
    pd_deleted(0),
    pd_logname(0)
{
}


omni_tracedmutex::~omni_tracedmutex()
{
  if( pd_deleted ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- mutex deleted more than once.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  if (pd_logname) {
    omni_thread* me = omni_thread::self();
    omniORB::logger l;
    l << pd_logname << ": thread " << (me ? me->id() : -1)
      << " deletes mutex.\n";
  }
  if( pd_holder ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- mutex destroyed whilst held.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  if( pd_n_conds != 0 ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- mutex destroyed whilst still being used\n"
	" by " << pd_n_conds << " condition variable(s).\n" << bug_msg;
    }
    BOMB_OUT();
  }
  pd_deleted = 1;
}


void
omni_tracedmutex::lock()
{
  if( pd_deleted ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- attempt to lock deleted mutex.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  omni_thread* me = omni_thread::self();

  if (pd_logname) {
    omniORB::logger l;
    l << pd_logname << ": thread " << (me ? me->id() : -1)
      << " calls lock()\n";
  }

  omni_mutex_lock sync(pd_lock);

  if( me && pd_holder == me ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- attempt to lock mutex when already held.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  while( pd_holder )  pd_cond.wait();

  pd_holder = me ? me : (omni_thread*) 1;
}


void
omni_tracedmutex::unlock()
{
  if( pd_deleted ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- attempt to unlock deleted mutex.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  omni_thread* me = omni_thread::self();

  if (pd_logname) {
    omniORB::logger l;
    l << pd_logname << ": thread " << (me ? me->id() : -1)
      << " calls unlock()\n";
  }

  {
    omni_mutex_lock sync(pd_lock);

    if( !pd_holder || me && pd_holder != me ) {
      {
	omniORB::logger log;
	log << "Assertion failed -- attempt to unlock mutex not held.\n"
	    << bug_msg;
      }
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

  {
    omniORB::logger log;
    log << "Assertion failed -- "
	<< (yes ? "mutex is not held.\n" : "mutex should not be held.\n")
	<< bug_msg
	<< "   file: " << file << "\n"
	<< "   line: " << line << "\n";
  }

  BOMB_OUT();
}

void
omni_tracedmutex::log(const char* name)
{
  pd_logname = name;
}


//////////////////////////////////////////////////////////////////////
//////////////////////// omni_tracedcondition ////////////////////////
//////////////////////////////////////////////////////////////////////

omni_tracedcondition::omni_tracedcondition(omni_tracedmutex* m)
  : pd_mutex(*m), pd_cond(&m->pd_lock), pd_n_waiters(0),
    pd_deleted(0), pd_logname(0)
{
  if( !m ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- omni_tracedcondition initialised with\n"
	  << " a nil mutex argument!\n" << bug_msg;
    }
    BOMB_OUT();
  }
  pd_mutex.pd_lock.lock();
  pd_mutex.pd_n_conds++;
  pd_mutex.pd_lock.unlock();
}


omni_tracedcondition::~omni_tracedcondition()
{
  if( pd_deleted ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- condition deleted more than once.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  if (pd_logname) {
    omni_thread* me = omni_thread::self();
    omniORB::logger l;
    l << pd_logname << ": thread " << (me ? me->id() : -1)
      << " deletes condition.\n";
  }
  if( pd_n_waiters ) {
    {
      omniORB::logger log;
      log << "WARNING -- an omni_tracedcondition was destroyed,\n"
	  << " but there are still threads waiting on it!\n";
    }
  }
  pd_mutex.pd_lock.lock();
  pd_mutex.pd_n_conds--;
  pd_mutex.pd_lock.unlock();
  pd_deleted = 1;
}


void
omni_tracedcondition::wait()
{
  if( pd_deleted ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- attempt to wait on deleted condition.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  omni_thread* me = omni_thread::self();

  if (pd_logname) {
    omniORB::logger l;
    l << pd_logname << ": thread " << (me ? me->id() : -1)
      << " calls wait()\n";
  }

  omni_mutex_lock sync(pd_mutex.pd_lock);

  if( me && pd_mutex.pd_holder != me ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- attempt to wait on condition variable,\n"
	  << " but the calling thread does not hold the associated mutex.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  pd_mutex.pd_holder = 0;
  pd_mutex.pd_cond.signal();
  pd_n_waiters++;
  pd_cond.wait();
  pd_n_waiters--;
  while( pd_mutex.pd_holder )  pd_mutex.pd_cond.wait();
  pd_mutex.pd_holder = me ? me : (omni_thread*) 1;

  if (pd_logname) {
    omniORB::logger l;
    l << pd_logname << ": thread " << (me ? me->id() : -1)
      << " leaves wait()\n";
  }
}


int
omni_tracedcondition::timedwait(unsigned long secs, unsigned long nanosecs)
{
  if( pd_deleted ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- attempt to timedwait on deleted condition.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  omni_thread* me = omni_thread::self();

  if (pd_logname) {
    omniORB::logger l;
    l << pd_logname << ": thread " << (me ? me->id() : -1)
      << " calls timedwait()\n";
  }

  omni_mutex_lock sync(pd_mutex.pd_lock);

  if( me && pd_mutex.pd_holder != me ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- attempt to wait on condition variable,\n"
	  << " but the calling thread does not hold the associated mutex.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }

  pd_mutex.pd_holder = 0;
  pd_mutex.pd_cond.signal();
  pd_n_waiters++;
  int ret = pd_cond.timedwait(secs, nanosecs);
  pd_n_waiters--;
  while( pd_mutex.pd_holder )  pd_mutex.pd_cond.wait();
  pd_mutex.pd_holder = me ? me : (omni_thread*) 1;

  if (pd_logname) {
    omniORB::logger l;
    l << pd_logname << ": thread " << (me ? me->id() : -1)
      << " leaves timedwait()\n";
  }
  return ret;
}


void
omni_tracedcondition::signal()
{
  if( pd_deleted ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- attempt to signal on deleted condition.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  if (pd_logname) {
    omni_thread* me = omni_thread::self();
    omniORB::logger l;
    l << pd_logname << ": thread " << (me ? me->id() : -1)
      << " calls signal()\n";
  }
  pd_cond.signal();
}


void
omni_tracedcondition::broadcast()
{
  if( pd_deleted ) {
    {
      omniORB::logger log;
      log << "Assertion failed -- attempt to broadcast on deleted condition.\n"
	  << bug_msg;
    }
    BOMB_OUT();
  }
  if (pd_logname) {
    omni_thread* me = omni_thread::self();
    omniORB::logger l;
    l << pd_logname << ": thread " << (me ? me->id() : -1)
      << " calls broadcast()\n";
  }
  pd_cond.broadcast();
}

void
omni_tracedcondition::log(const char* name)
{
  pd_logname = name;
}


#endif  // ifdef OMNIORB_ENABLE_LOCK_TRACES
