// -*- Mode: C++; -*-
//                            Package   : omniORB2
// scavenger.cc               Created on: 5/8/97
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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
  Revision 1.1  1997/12/09 18:43:11  sll
  Initial revision

  */


#include <omniORB2/CORBA.h>
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


static CORBA::ULong inScanPeriod  = 15;		// seconds
static CORBA::ULong outScanPeriod = 10;         // seconds
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
    cerr << "inScavenger: start." << endl;
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
	    cerr << "inScavenger: woken by poke()" << endl;
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
	cerr << "inScavenger: scanning for idle incoming connections" << endl;
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
    cerr << "inScavenger: exit." << endl;
  }
  return 0;
}

void*
outScavenger_t::run_undetached(void *arg)
{
  if (omniORB::traceLevel >= 15) {
    cerr << "outScavenger: start." << endl;
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
	    cerr << "outScavenger: woken by poke()" << endl;
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
	cerr << "outScavenger: scanning for idle outgoing connections" << endl;
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
    cerr << "outScavenger: exit." << endl;
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
		    cerr << thread_name << ": shutting down idle connection" << endl;
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

