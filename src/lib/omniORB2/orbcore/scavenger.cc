// -*- Mode: C++; -*-
//                            Package   : omniORB
// scavenger.cc               Created on: 5/8/97
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//      
 
/*
  $Log$
  Revision 1.17  2001/02/21 14:12:11  dpg1
  Merge from omni3_develop for 3.0.3 release.

  Revision 1.10.6.4  2000/01/07 14:51:14  djr
  Call timeouts are now disabled by default.

  Revision 1.10.6.3  1999/09/27 11:01:11  djr
  Modifications to logging.

  Revision 1.10.6.2  1999/09/24 15:01:36  djr
  Added module initialisers, and sll's new scavenger implementation.

  Revision 1.10.2.2  1999/09/23 13:47:25  sll
  Fixed a race condition which causes simple programs that exit quickly to
  delay for a maximum of one scan period.

  Revision 1.10.2.1  1999/09/21 20:37:17  sll
  -Simplified the scavenger code and the mechanism in which connections
   are shutdown. Now only one scavenger thread scans both incoming
   and outgoing connections. A separate thread do the actual shutdown.
  -omniORB::scanGranularity() now takes only one argument as there is
   only one scan period parameter instead of 2.
  -Trace messages in various modules have been updated to use the logger
   class.
  -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                 -ORBscanIncomingPeriod.

  Revision 1.10  1999/08/31 19:22:37  sll
  Revert back to single inheritance. The bug that causes occasional thread
  exit on startup has been identified. start_undetached() should be called
  from the most derived type. Previously it was called in _Scavenger.

  Revision 1.9  1999/08/30 16:49:00  sll
  Scavenger threads now scan for idle connections and stuck remote calls.
  Another thread Ripper_t is used to do the actual shutdown.

  Revision 1.8  1999/08/16 19:27:20  sll
  Added a per-compilation unit initialiser object.
  This object is called by ORB_init and ORB::shutdown.

  Revision 1.7  1999/05/26 11:55:33  sll
  Use WrTestLock instead of the obsoluted WrTimedLock.

  Revision 1.6  1999/03/11 16:25:55  djr
  Updated copyright notice

  Revision 1.5  1999/02/11 17:54:19  djr
  Added class OutScavengerThreadKiller which kills the out scavenger
  when global destructors are called.

  Revision 1.4  1998/08/14 13:51:58  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.3  1998/04/07 19:37:14  sll
  Replace cerr with omniORB::log.

// Revision 1.2  1998/01/22  11:38:19  sll
// Set the incoming and outgoing scan period to 30 seconds.
//
  Revision 1.1  1997/12/09 18:43:11  sll
  Initial revision

  */


#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <scavenger.h>
#include <ropeFactory.h>
#include <objectAdapter.h>
#include <initialiser.h>
#include <limits.h>


#define LOGMESSAGE(level,prefix,message)  \
  omniORB::logs(level, "scavenger " prefix ": " message)


static CORBA::ULong ScanPeriod = 5;  // seconds

// The following are in scan periods.
static int serverCallTimeLimit_ = INT_MAX;
static int clientCallTimeLimit_ = INT_MAX;
static int outIdleTimeLimit_    = 24;
static int inIdleTimeLimit_     = 36;


/////////////////////////////////////////////////////////////////////////////
// omniORB_Scavenger
//
// Instance of this class scan the strands periodically. It calls shutdown
// on those strands that has been idle for a period of time or
// a call using such a strand has made no progress for a period of time.
// The length of both periods are controlled by the application.
//
class omniORB_Scavenger : public omni_thread {
public:
  omniORB_Scavenger() : pd_cond(&pd_mutex),pd_isdying(0),pd_ropefactories(2) {
    
    start_undetached();
  }

  virtual ~omniORB_Scavenger() {}

  void poke() { pd_cond.signal(); }
  void kill() { 
    {
      omni_mutex_lock sync(pd_mutex);
      pd_isdying = 1;
      pd_cond.signal();
    }
    join(0);
  }
  
  void* run_undetached(void*);

  void addRopeFactoryList(ropeFactoryList* l) {
    omni_mutex_lock sync(pd_mutex);
    CORBA::ULong index = pd_ropefactories.length();
    pd_ropefactories.length(index + 1);
    pd_ropefactories[index] = l;
  }

  void removeRopeFactoryList(ropeFactoryList* l) {
    omni_mutex_lock sync(pd_mutex);
    CORBA::ULong index;
    for (index = 0; index < pd_ropefactories.length(); index++)
      if (pd_ropefactories[index] == l) break;
    if (index != pd_ropefactories.length()) {
      for (index++ ; index < pd_ropefactories.length(); index++)
	pd_ropefactories[index-1] = pd_ropefactories[index];
      pd_ropefactories.length(pd_ropefactories.length()-1);
    }
  }

private:
  omni_mutex       pd_mutex;
  omni_condition   pd_cond;
  CORBA::Boolean   pd_isdying;
  _CORBA_PseudoValue_Sequence<ropeFactoryList*> pd_ropefactories;
};

static omniORB_Scavenger* scavenger;

/////////////////////////////////////////////////////////////////////////////
// Internal interface to other parts of the ORB

int
StrandScavenger::clientCallTimeLimit() { return clientCallTimeLimit_; }

int 
StrandScavenger::serverCallTimeLimit() { return serverCallTimeLimit_; }

int
StrandScavenger::outIdleTimeLimit() { return outIdleTimeLimit_; }

int
StrandScavenger::inIdleTimeLimit() { return inIdleTimeLimit_; }

void 
StrandScavenger::addRopeFactories(ropeFactoryList* l) {
  scavenger->addRopeFactoryList(l);
}

void
StrandScavenger::removeRopeFactories(ropeFactoryList* l) {
  scavenger->removeRopeFactoryList(l);
}


/////////////////////////////////////////////////////////////////////////////
void 
omniORB::idleConnectionScanPeriod(omniORB::idleConnType direction,
				  CORBA::ULong sec)
{
  switch (direction)
    {
    case omniORB::idleIncoming:
      if (sec && ScanPeriod)
	inIdleTimeLimit_ = ((sec >= ScanPeriod) ? sec : ScanPeriod) / 
                           ScanPeriod;
      else
	inIdleTimeLimit_ = INT_MAX;
      break;
    case omniORB::idleOutgoing:
      if (sec && ScanPeriod)
	outIdleTimeLimit_ = ((sec >= ScanPeriod) ? sec : ScanPeriod)
                             / ScanPeriod;
      else
	outIdleTimeLimit_ = INT_MAX;
      break;
    }
}

/////////////////////////////////////////////////////////////////////////////
CORBA::ULong 
omniORB::idleConnectionScanPeriod(omniORB::idleConnType direction)
{
  switch (direction)
    {
    case omniORB::idleIncoming:
      return ((inIdleTimeLimit_ != INT_MAX) ? 
	      (inIdleTimeLimit_ * ScanPeriod) : 0);
    case omniORB::idleOutgoing:
    default:  // stop MSVC complaining
      return ((outIdleTimeLimit_ != INT_MAX) ? 
	      (outIdleTimeLimit_ * ScanPeriod) : 0);
    }
}

/////////////////////////////////////////////////////////////////////////////
void 
omniORB::callTimeOutPeriod(omniORB::callTimeOutType direction,
			   CORBA::ULong sec)
{
  switch (direction)
    {
    case omniORB::serverSide:
      if (sec && ScanPeriod)
	serverCallTimeLimit_ = ((sec >= ScanPeriod) ? sec : ScanPeriod) / 
                               ScanPeriod;
      else
	serverCallTimeLimit_ = INT_MAX;
      break;
    case omniORB::clientSide:
      if (sec && ScanPeriod)
	clientCallTimeLimit_ = ((sec >= ScanPeriod) ? sec : ScanPeriod)
                                / ScanPeriod;
      else
	clientCallTimeLimit_ = INT_MAX;
      break;
    }
}

/////////////////////////////////////////////////////////////////////////////
CORBA::ULong 
omniORB::callTimeOutPeriod(omniORB::callTimeOutType direction)
{
  switch (direction)
    {
    case omniORB::serverSide:
      return ((serverCallTimeLimit_ != INT_MAX) ? 
	      (serverCallTimeLimit_ * ScanPeriod) : 0);
    case omniORB::clientSide:
    default:  // stop MSVC complaining
      return ((clientCallTimeLimit_ != INT_MAX) ? 
	      (clientCallTimeLimit_ * ScanPeriod) : 0);
    }
}

/////////////////////////////////////////////////////////////////////////////
void 
omniORB::scanGranularity(CORBA::ULong sec)
{
  if (sec) {
    CORBA::ULong clin,ilin,clout,ilout;

    clin = omniORB::callTimeOutPeriod(omniORB::serverSide);
    ilin = omniORB::idleConnectionScanPeriod(omniORB::idleIncoming);
    clout = omniORB::callTimeOutPeriod(omniORB::clientSide);
    ilout = omniORB::idleConnectionScanPeriod(omniORB::idleOutgoing);

    ScanPeriod = sec;
    omniORB::callTimeOutPeriod(omniORB::serverSide,clin);
    omniORB::idleConnectionScanPeriod(omniORB::idleIncoming,ilin);
    omniORB::callTimeOutPeriod(omniORB::clientSide,clout);
    omniORB::idleConnectionScanPeriod(omniORB::idleOutgoing,ilout);
  }
  else {
    ScanPeriod = sec;
  }
  
  if (scavenger) {
    scavenger->poke();
  }
}

/////////////////////////////////////////////////////////////////////////////
CORBA::ULong 
omniORB::scanGranularity()
{
  return ScanPeriod;
}


/////////////////////////////////////////////////////////////////////////////
void*
omniORB_Scavenger::run_undetached(void*)
{
  LOGMESSAGE(15,"","start.");

  unsigned long abs_sec,abs_nsec;
  omni_thread::get_time(&abs_sec,&abs_nsec);

  if (ScanPeriod)
    abs_sec += ScanPeriod;

  omni_mutex_lock sync(pd_mutex);

  while (!pd_isdying) {

    int poke = 0;
    if (ScanPeriod) {
      poke = pd_cond.timedwait(abs_sec,abs_nsec);
      if (poke) {
	LOGMESSAGE(15,"","woken by poke()");
	omni_thread::get_time(&abs_sec,&abs_nsec);	
	abs_sec += ScanPeriod;
      }
    }
    else {
      // inScanPeriod == 0 implies stop the scan. Block here indefinitely.
      pd_cond.wait();
      omni_thread::get_time(&abs_sec,&abs_nsec);	
    }

    if (poke || pd_isdying) continue;
  
    LOGMESSAGE(15,"","scanning connections");

    for (CORBA::ULong i= 0; i < pd_ropefactories.length(); i++)
    {
      ropeFactory_iterator iter(pd_ropefactories[i]);
      ropeFactory* rp;

      while ((rp = (ropeFactory*)iter())) {
	// Scan all the outgoing rope
	Rope_iterator next_rope(rp->anchor());
	Rope *r;
	while ((r = next_rope())) {
	  // For each rope, scan all the strands
	  Strand_iterator next_strand(r);
	  Strand *s;
	  while ((s = next_strand())) {
	    if (!s->_strandIsDying() && 
		Strand::Sync::clicksDecrAndGet(s) < 0) {
	      s->shutdown();
	    }
	  }
	}
      }
    }

    abs_sec += ScanPeriod;
  }

  LOGMESSAGE(15,"","exit.");
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_scavenger_initialiser : public omniInitialiser {
public:

  void attach() {
    scavenger = new omniORB_Scavenger();
  }

  void detach() {
    scavenger->kill();
    scavenger = 0;
  }
};

static omni_scavenger_initialiser initialiser;

omniInitialiser& omni_scavenger_initialiser_ = initialiser;
