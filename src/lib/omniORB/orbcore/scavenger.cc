// -*- Mode: C++; -*-
//                            Package   : omniORB2
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


#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <limits.h>

#include <ropeFactory.h>
#include <objectManager.h>
#include <scavenger.h>

class _Scavenger : public omni_thread {
public:
  _Scavenger() : pd_isdying(0), pd_cond(&pd_mutex)  {}
  void poke() { pd_cond.signal(); }
  void setIsDying() { omni_mutex_lock sync(pd_mutex); pd_isdying = 1; }
  CORBA::Boolean isDying() const { return pd_isdying; }

protected:
  virtual ~_Scavenger() {}
  CORBA::Boolean pd_isdying;
  omni_mutex     pd_mutex;
  omni_condition pd_cond;
};


class Ripper_t : public _Scavenger {
  // Instance of this class do the dirty work of calling shutdown on the
  // strand. The call shutdown may actually block for considerable time.
  // We do not want the in or out scavenger thread to lose track of
  // time because of this.
public:
  virtual void* run_undetached(void * arg);

  Ripper_t(const char* id,ropeFactoryList* l) : pd_id(id), 
                                                pd_scan(0), pd_l(l) {
    start_undetached();
  }
  virtual ~Ripper_t() {}

  void triggerScan() { omni_mutex_lock sync(pd_mutex); pd_scan = 1; poke(); }

private:
  const char*      pd_id;
  CORBA::Boolean   pd_scan;
  ropeFactoryList* pd_l;
  Ripper_t();
};

class inScavenger_t : public _Scavenger {
public:
  inScavenger_t () { start_undetached(); }
  virtual void* run_undetached(void *arg);
protected:
  virtual ~inScavenger_t() {}
};


class outScavenger_t : public _Scavenger {
public:
  outScavenger_t () { start_undetached(); }
  virtual void* run_undetached(void *arg);
protected:
  virtual ~outScavenger_t() {}
};


static CORBA::ULong inScanPeriod  = 5;		// seconds
static CORBA::ULong outScanPeriod = 5;          // seconds

static inScavenger_t* inScavenger   = 0;
static outScavenger_t* outScavenger = 0;

static int serverCallTimeLimit_ = 18;
static int clientCallTimeLimit_ = 12;
static int outIdleTimeLimit_    = 24;
static int inIdleTimeLimit_     = 36;

int StrandScavenger::clientCallTimeLimit() { return clientCallTimeLimit_; }
int StrandScavenger::serverCallTimeLimit() { return serverCallTimeLimit_; }
int StrandScavenger::outIdleTimeLimit() { return outIdleTimeLimit_; }
int StrandScavenger::inIdleTimeLimit() { return inIdleTimeLimit_; }

void
StrandScavenger::initInScavenger()
{
  if (!inScavenger) inScavenger = new inScavenger_t;
}

void
StrandScavenger::pokeInScavenger()
{
  if (inScavenger) {
    inScavenger->poke();
  }
}

void
StrandScavenger::killInScavenger()
{
  if (inScavenger) {
    inScavenger->setIsDying();
    inScavenger->poke();
    inScavenger->join(0);
    inScavenger = 0;
  }
}

void
StrandScavenger::initOutScavenger()
{
  if (!outScavenger) outScavenger = new outScavenger_t;
}

void
StrandScavenger::pokeOutScavenger()
{
  if (outScavenger) {
    outScavenger->poke();
  }
}

void
StrandScavenger::killOutScavenger()
{
  if (outScavenger) {
    outScavenger->setIsDying();
    outScavenger->poke();
    outScavenger->join(0);
    outScavenger = 0;
  }
}

void*
Ripper_t::run_undetached(void*)
{
  if (omniORB::traceLevel >= 15) {
    omniORB::log << pd_id << " Ripper: start.\n";
    omniORB::log.flush();
  }

  while (1) {

    Strand *s;
    {
      omni_mutex_lock sync(pd_mutex);

      while (!isDying() ) {
	if (!pd_scan) {
	  pd_cond.wait();
	  continue;
	}

	if (omniORB::traceLevel >= 15) {
	  omniORB::log << pd_id << " Ripper: scan.\n";
	  omniORB::log.flush();
	}

	ropeFactory_iterator iter(pd_l);
	ropeFactory* rp;

	while ((rp = (ropeFactory*)iter())) {
	  Rope_iterator next_rope(rp->anchor());
	  Rope *r;
	  while ((r = next_rope())) {
	    Strand_iterator next_strand(r);
	    while ((s = next_strand())) {
	      if (!s->_strandIsDying() && Strand::Sync::clicksGet(s) < 0) {

		// Should shutdown this strand. Release all mutexes before
		// doing so. Increment the reference count on the strand
		// to make sure that it does not go away once the
		// mutexes have been released.
		s->incrRefCount(1);
		goto do_shutdown;
	      }
	    }
	  }
	}
	pd_scan = 0;
      }
      goto cleanup;
    }

  do_shutdown:

    // Do shutdown once the all the mutexes has been released.
    // On some platform, e.g. linux and under certain condition,
    // shutdown(2) actually blocks for seconds before returning.
    // If we do shutdown while holding the mutexes, e.g. the
    // rope mutex, we will stop the progress of other threads in
    // the ORB. This is bad!

    s->shutdown();
    s->decrRefCount();

    // XXX Go back to scan the strands all over again. This is rather
    // inefficent especially when we have a long list of strands to
    // shutdown.
  }
 cleanup:

  if (omniORB::traceLevel >= 15) {
    omniORB::log << pd_id << " Ripper: exit.\n";
    omniORB::log.flush();
  }
  return 0;
}

void*
inScavenger_t::run_undetached(void *)
{
  if (omniORB::traceLevel >= 15) {
    omniORB::log << "inScavenger: start.\n";
    omniORB::log.flush();
  }

  Ripper_t* ripper = new Ripper_t("in",omniObjectManager::root()->incomingRopeFactories());
  pd_mutex.lock();
  while (!isDying())
    {
      unsigned long abs_sec,abs_nsec;
      omni_thread::get_time(&abs_sec,&abs_nsec,inScanPeriod);
      if (inScanPeriod) {
	int poke = pd_cond.timedwait(abs_sec,abs_nsec);
	if (omniORB::traceLevel >= 15) {
	  if (poke) {
	    omniORB::log << "inScavenger: woken by poke()\n";
	    omniORB::log.flush();
	  }
	}
      }
      else {
	// inScanPeriod == 0 implies stop the scan. Block here indefinitely.
	pd_cond.wait();
	continue;
      }
      if (isDying())
	continue;
      pd_mutex.unlock();

      if (omniORB::traceLevel >  10) {
	omniORB::log << "inScavenger: scanning incoming connections\n";
	omniORB::log.flush();
      }

      {
	ropeFactory_iterator iter(omniObjectManager::root()->incomingRopeFactories());
	ropeFactory* rp;
	CORBA::Boolean doshutdown = 0;

	while ((rp = (ropeFactory*)iter())) {
	  // Scan all the outgoing rope
	  Rope_iterator next_rope(rp->anchor());
	  Rope *r;
	  while ((r = next_rope())) 
	    {
	      // For each rope, scan all the strands
	      Strand_iterator next_strand(r);
	      Strand *s;
	      while ((s = next_strand())) 
		{
		  if (!s->_strandIsDying() && 
		      Strand::Sync::clicksDecrAndGet(s) < 0) {
		      doshutdown = 1;
		  }
		}
	    }
	}
	if (doshutdown) ripper->triggerScan();
      }
      pd_mutex.lock();
    }
  pd_mutex.unlock();

  ripper->setIsDying();
  ripper->triggerScan();
  ripper->join(0);
  ripper = 0;

  if (omniORB::traceLevel >= 15) {
    omniORB::log << "inScavenger: exit.\n";
    omniORB::log.flush();
  }
  return 0;
}

void*
outScavenger_t::run_undetached(void *)
{
  if (omniORB::traceLevel >= 15) {
    omniORB::log << "outScavenger: start.\n";
    omniORB::log.flush();
  }

  Ripper_t* ripper = new Ripper_t("out",globalOutgoingRopeFactories);

  pd_mutex.lock();
  while (!isDying())
    {
      unsigned long abs_sec,abs_nsec;
      omni_thread::get_time(&abs_sec,&abs_nsec,outScanPeriod);
      if (outScanPeriod) {
	int poke = pd_cond.timedwait(abs_sec,abs_nsec);
	if (omniORB::traceLevel >= 15) {
	  if (poke) {
	    omniORB::log << "outScavenger: woken by poke()\n";
	    omniORB::log.flush();
	  }
	}
      }
      else {
	// outScanPeriod == 0 implies stop the scan. Block here indefinitely.
	pd_cond.wait();
	continue;
      }
      if (isDying())
	break;
      pd_mutex.unlock();

      if (omniORB::traceLevel > 10) {
	omniORB::log << "outScavenger: scanning outgoing connections\n";
	omniORB::log.flush();
      }
      {
	ropeFactory_iterator iter(globalOutgoingRopeFactories);
	ropeFactory* rp;
	CORBA::Boolean doshutdown = 0;

	while ((rp = (ropeFactory*)iter())) {
	  // Scan all the outgoing rope
	  Rope_iterator next_rope(rp->anchor());
	  Rope *r;
	  while ((r = next_rope())) 
	    {
	      // For each rope, scan all the strands
	      Strand_iterator next_strand(r);
	      Strand *s;
	      while ((s = next_strand())) 
		{
		  if (!s->_strandIsDying() && 
		      Strand::Sync::clicksDecrAndGet(s) < 0) {
		      doshutdown = 1;
		  }
		}
	    }
	}
	if (doshutdown) ripper->triggerScan();
      }
      pd_mutex.lock();
    }
  pd_mutex.unlock();

  ripper->setIsDying();
  ripper->triggerScan();
  ripper->join(0);
  ripper = 0;

  if (omniORB::traceLevel >= 15) {
    omniORB::log << "outScavenger: exit.\n";
    omniORB::log.flush();
  }
  return 0;
}

void 
omniORB::idleConnectionScanPeriod(omniORB::idleConnType direction,
				  CORBA::ULong sec)
{
  switch (direction)
    {
    case omniORB::idleIncoming:
      if (sec && inScanPeriod)
	inIdleTimeLimit_ = ((sec >= inScanPeriod) ? sec : inScanPeriod) / 
                           inScanPeriod;
      else
	inIdleTimeLimit_ = INT_MAX;
      break;
    case omniORB::idleOutgoing:
      if (sec && outScanPeriod)
	outIdleTimeLimit_ = ((sec >= outScanPeriod) ? sec : outScanPeriod)
                             / outScanPeriod;
      else
	outIdleTimeLimit_ = INT_MAX;
      break;
    }
}

CORBA::ULong 
omniORB::idleConnectionScanPeriod(omniORB::idleConnType direction)
{
  switch (direction)
    {
    case omniORB::idleIncoming:
      return ((inIdleTimeLimit_ != INT_MAX) ? 
	      (inIdleTimeLimit_ * inScanPeriod) : 0);
    case omniORB::idleOutgoing:
    default:  // stop MSVC complaining
      return ((outIdleTimeLimit_ != INT_MAX) ? 
	      (outIdleTimeLimit_ * outScanPeriod) : 0);
    }
}

void 
omniORB::callTimeOutPeriod(omniORB::callTimeOutType direction,
			   CORBA::ULong sec)
{
  switch (direction)
    {
    case omniORB::serverSide:
      if (sec && inScanPeriod)
	serverCallTimeLimit_ = ((sec >= inScanPeriod) ? sec : inScanPeriod) / 
                               inScanPeriod;
      else
	serverCallTimeLimit_ = INT_MAX;
      break;
    case omniORB::clientSide:
      if (sec && outScanPeriod)
	clientCallTimeLimit_ = ((sec >= outScanPeriod) ? sec : outScanPeriod)
                                / outScanPeriod;
      else
	clientCallTimeLimit_ = INT_MAX;
      break;
    }
}

CORBA::ULong 
omniORB::callTimeOutPeriod(omniORB::callTimeOutType direction)
{
  switch (direction)
    {
    case omniORB::serverSide:
      return ((serverCallTimeLimit_ != INT_MAX) ? 
	      (serverCallTimeLimit_ * inScanPeriod) : 0);
    case omniORB::clientSide:
    default:  // stop MSVC complaining
      return ((clientCallTimeLimit_ != INT_MAX) ? 
	      (clientCallTimeLimit_ * outScanPeriod) : 0);
    }
}

void 
omniORB::scanGranularity(omniORB::scanType direction,CORBA::ULong sec)
{
  switch (direction)
    {
    case omniORB::scanIncoming:
      {
	if (sec) {
	  CORBA::ULong cl = omniORB::callTimeOutPeriod(omniORB::serverSide);
	  CORBA::ULong il = omniORB::idleConnectionScanPeriod(omniORB::idleIncoming);
	  inScanPeriod = sec;
	  omniORB::callTimeOutPeriod(omniORB::serverSide,cl);
	  omniORB::idleConnectionScanPeriod(omniORB::idleIncoming,il);
	}
	else {
	  inScanPeriod = sec;
	}

	if (inScavenger) {
	  inScavenger->poke();
	}
      }
      break;
    case omniORB::scanOutgoing:
      {
	if (sec) {
	  CORBA::ULong cl = omniORB::callTimeOutPeriod(omniORB::clientSide);
	  CORBA::ULong il = omniORB::idleConnectionScanPeriod(omniORB::idleOutgoing);
	  outScanPeriod = sec;
	  omniORB::callTimeOutPeriod(omniORB::clientSide,cl);
	  omniORB::idleConnectionScanPeriod(omniORB::idleOutgoing,il);
	}
	else {
	  outScanPeriod = sec;
	}
	if (outScavenger) {
	  outScavenger->poke();
	}
      }
      break;
    }
}

CORBA::ULong 
omniORB::scanGranularity(omniORB::scanType direction)
{
  switch (direction)
    {
    case omniORB::scanIncoming:
      return inScanPeriod;
    case omniORB::scanOutgoing:
    default:  // stop MSVC complaining
      return outScanPeriod;
    }
}

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_scavenger_initialiser : public omniInitialiser {
public:

  void attach() {
    StrandScavenger::initOutScavenger();
  }

  void detach() {
    StrandScavenger::killOutScavenger();
    StrandScavenger::killInScavenger();
  }
};

static omni_scavenger_initialiser initialiser;

omniInitialiser& omni_scavenger_initialiser_ = initialiser;
