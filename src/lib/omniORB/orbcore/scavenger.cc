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

#include <ropeFactory.h>
#include <objectManager.h>
#include <scavenger.h>

class _Scavenger : public omni_thread {
public:
  _Scavenger() : pd_isdying(0), pd_cond(&pd_mutex)  { start_undetached(); }
  virtual void* run_undetached(void *arg) { return 0; }
  void poke() { pd_cond.signal(); }
  void setIsDying() { omni_mutex_lock sync(pd_mutex); pd_isdying = 1; }
  CORBA::Boolean isDying() const { return pd_isdying; }

protected:
  virtual ~_Scavenger() {}
  CORBA::Boolean pd_isdying;
  omni_mutex     pd_mutex;
  omni_condition pd_cond;

};

class inScavenger_t : public _Scavenger {
public:
  virtual void* run_undetached(void *arg);
protected:
  virtual ~inScavenger_t() {}
};

class outScavenger_t : public _Scavenger {
public:
  virtual void* run_undetached(void *arg);
protected:
  virtual ~outScavenger_t() {}
};


static CORBA::ULong inScanPeriod  = 30;		// seconds
static CORBA::ULong outScanPeriod = 30;         // seconds
static inScavenger_t* inScavenger   = 0;
static outScavenger_t* outScavenger = 0;


static void scanForIdle(Anchor* anchor,char* thread_name);

void
StrandScavenger::initInScavenger()
{
  if (!inScavenger && inScanPeriod)
    inScavenger = new inScavenger_t;
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

CORBA::ULong
StrandScavenger::inScavengerScanPeriod()
{
  return inScanPeriod;
}
 
void
StrandScavenger::inScavengerScanPeriod(CORBA::ULong sec)
{
  inScanPeriod = sec;
  if (inScavenger) {
    inScavenger->poke();
  }
  else {
    StrandScavenger::initInScavenger();
  }
}

void
StrandScavenger::initOutScavenger()
{
  if (!outScavenger && outScanPeriod)
    outScavenger = new outScavenger_t;
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

CORBA::ULong
StrandScavenger::outScavengerScanPeriod()
{
  return outScanPeriod;
}
 
void
StrandScavenger::outScavengerScanPeriod(CORBA::ULong sec)
{
  outScanPeriod = sec;
  if (outScavenger) {
    outScavenger->poke();
  }
  else {
    StrandScavenger::initOutScavenger();
  }
}



void*
inScavenger_t::run_undetached(void *arg)
{
  if (omniORB::traceLevel >= 15) {
    omniORB::log << "inScavenger: start.\n";
    omniORB::log.flush();
  }
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
	omniORB::log << "inScavenger: scanning for idle incoming connections\n";
	omniORB::log.flush();
      }

      {
	ropeFactory_iterator iter(*(omniObjectManager::root()->incomingRopeFactories()));
	ropeFactory* rp;
	while ((rp = (ropeFactory*)iter())) {
	  scanForIdle(rp->anchor(),"inScavenger");
	}
      }
      pd_mutex.lock();
    }
  pd_mutex.unlock();
  if (omniORB::traceLevel >= 15) {
    omniORB::log << "inScavenger: exit.\n";
    omniORB::log.flush();
  }
  return 0;
}

void*
outScavenger_t::run_undetached(void *arg)
{
  if (omniORB::traceLevel >= 15) {
    omniORB::log << "outScavenger: start.\n";
    omniORB::log.flush();
  }
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
	omniORB::log << "outScavenger: scanning for idle outgoing connections\n";
	omniORB::log.flush();
      }
      {
	ropeFactory_iterator iter(globalOutgoingRopeFactories);
	ropeFactory* rp;
	while ((rp = (ropeFactory*)iter())) {
	  scanForIdle(rp->anchor(),"outScavenger");
	}
      }
      pd_mutex.lock();
    }
  pd_mutex.unlock();
  if (omniORB::traceLevel >= 15) {
    omniORB::log << "outScavenger: exit.\n";
    omniORB::log.flush();
  }
  return 0;
}

static
void
scanForIdle(Anchor* anchor,char* thread_name)
{
  // Scan all the outgoing rope
  Rope_iterator next_rope(anchor);
  Rope *r;
  while ((r = next_rope())) 
    {
      // For each rope, scan all the strands
      Strand_iterator next_strand(r);
      Strand *s;
      while ((s = next_strand())) 
	{
	  unsigned long abs_sec,abs_nsec;
	  // Set the heartbeat boolean to 1 and get back its value
	  // prior to this update.
	  CORBA::Boolean heartbeat = 1;
	  omni_thread::get_time(&abs_sec,&abs_nsec);
	  if (Strand::Sync::WrTimedLock(s,heartbeat,abs_sec,abs_nsec))
	    {
	      if (heartbeat) 
		{
		  // Since the last time this thread set the heartbeat
		  // value to 1, nobody has reset it to 0 (via
		  // WrLock). Therefore the strand must have been idle
		  // for over one scan period.  Shut it down.
		  if (omniORB::traceLevel > 10) {
		    omniORB::log << thread_name << ": shutting down idle connection\n";
		    omniORB::log.flush();
		  }
		  s->shutdown();
		}
	      Strand::Sync::WrUnlock(s);
	    }
	}
    }
}


void 
omniORB::idleConnectionScanPeriod(omniORB::idleConnType direction,
				  CORBA::ULong sec)
{
  switch (direction)
    {
    case omniORB::idleIncoming:
      StrandScavenger::inScavengerScanPeriod(sec);
      break;
    case omniORB::idleOutgoing:
      StrandScavenger::outScavengerScanPeriod(sec);
      break;
    }
}

CORBA::ULong 
omniORB::idleConnectionScanPeriod(omniORB::idleConnType direction)
{
  switch (direction)
    {
    case omniORB::idleIncoming:
      return StrandScavenger::inScavengerScanPeriod();
    case omniORB::idleOutgoing:
    default:  // stop MSVC complaining
      return StrandScavenger::outScavengerScanPeriod();
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Kill the out scavenger thread.

class OutScavengerThreadKiller {
public:
  ~OutScavengerThreadKiller() {
    StrandScavenger::killOutScavenger();
  }
  static OutScavengerThreadKiller theInstance;
};

OutScavengerThreadKiller OutScavengerThreadKiller::theInstance;
