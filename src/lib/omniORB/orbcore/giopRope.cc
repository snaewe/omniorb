// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopRope.cc                Created on: 16/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
  Revision 1.1.4.24  2003/06/02 09:28:22  dgrisby
  Segfault in log message when throwing an exception in an interceptor.

  Revision 1.1.4.23  2003/02/17 01:20:00  dgrisby
  Avoid deadlock with bidir connection shutdown.

  Revision 1.1.4.22  2002/09/08 21:12:38  dgrisby
  Properly handle IORs with no usable profiles.

  Revision 1.1.4.21  2002/08/23 14:15:02  dgrisby
  Avoid exception with bidir when no POA.

  Revision 1.1.4.20  2002/03/18 15:13:08  dpg1
  Fix bug with old-style ORBInitRef in config file; look for
  -ORBtraceLevel arg before anything else; update Windows registry
  key. Correct error message.

  Revision 1.1.4.19  2001/09/19 17:26:49  dpg1
  Full clean-up after orb->destroy().

  Revision 1.1.4.18  2001/09/10 17:47:17  sll
  startIdleCounter when the strand is definitely idle.

  Revision 1.1.4.17  2001/09/04 14:40:30  sll
  Added the boolean argument to notifyCommFailure to indicate if
  omniTransportLock is held by the caller.

  Revision 1.1.4.16  2001/09/03 17:31:52  sll
  Make sure that acquireClient honours the deadline set in the calldescriptor.

  Revision 1.1.4.15  2001/09/03 13:31:45  sll
  Removed debug trace.

  Revision 1.1.4.14  2001/09/03 13:26:35  sll
  In filterAndSortAddressList, change to use the lowest value to represent
  the highest priority.

  Revision 1.1.4.13  2001/08/31 11:57:16  sll
  Client side transport selection is now determined by the clientTransportRules.

  Revision 1.1.4.12  2001/08/21 11:02:14  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.1.4.11  2001/08/17 17:12:37  sll
  Modularise ORB configuration parameters.

  Revision 1.1.4.10  2001/08/06 15:50:27  sll
  In filterAndSortAddressList, make unix transport as the first choice if
  available. This is just a temporary solution until we have table driven
  transport selection.

  Revision 1.1.4.9  2001/08/03 17:41:21  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.4.8  2001/08/01 18:12:54  sll
  In filterAndSortAddressList, use_bidir could be left uninitialised.

  Revision 1.1.4.7  2001/07/31 16:24:23  sll
  Moved filtering and sorting of available addresses into a separate
  function. Make acquireClient, decrRefCount and notifyCommFailure virtual.

  Revision 1.1.4.6  2001/07/13 15:26:58  sll
  Use safeDelete to remove a strand.

  Revision 1.1.4.5  2001/06/13 20:13:15  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.1.4.4  2001/06/11 18:01:58  sll
  Temporarily hardwared to choose ssl over tcp transport if the IOR has both.

  Revision 1.1.4.3  2001/05/31 16:18:13  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.1.4.2  2001/05/08 17:06:53  sll
  Client side now closes the connection if it encounters any error in
  processing a call.

  Revision 1.1.4.1  2001/04/18 18:10:49  sll
  Big checkin with the brand new internal APIs.

  */


#include <omniORB4/CORBA.h>
#include <omniORB4/IOP_C.h>
#include <giopRope.h>
#include <giopStream.h>
#include <giopStrand.h>
#include <giopStreamImpl.h>
#include <giopBiDir.h>
#include <GIOP_C.h>
#include <objectAdapter.h>
#include <exceptiondefs.h>
#include <omniORB4/minorCode.h>
#include <initialiser.h>
#include <orbOptions.h>
#include <orbParameters.h>
#include <transportRules.h>
#include <omniORB4/callDescriptor.h>

#include <stdlib.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
CORBA::Boolean orbParameters::oneCallPerConnection = 1;
//  1 means only one call can be in progress at any time per connection.
//
//  Valid values = 0 or 1

CORBA::ULong orbParameters::maxGIOPConnectionPerServer = 5;
//  The ORB could open more than one connections to a server
//  depending on the number of concurrent invocations to the same
//  server. This variable decide what is the maximum number of
//  connections to use per server. This variable is read only once
//  at ORB_init. If the number of concurrent invocations exceed this
//  number, the extra invocations would be blocked until the
//  the outstanding ones return.
//
//  Valid values = (n >= 1) 


///////////////////////////////////////////////////////////////////////
RopeLink giopRope::ropes;

////////////////////////////////////////////////////////////////////////
giopRope::giopRope(const giopAddressList& addrlist,
		   const omnivector<CORBA::ULong>& preferred) :
  pd_refcount(0),
  pd_address_in_use(0),
  pd_maxStrands(orbParameters::maxGIOPConnectionPerServer),
  pd_oneCallPerConnection(orbParameters::oneCallPerConnection),
  pd_nwaiting(0),
  pd_cond(omniTransportLock)
{
  {
    giopAddressList::const_iterator i, last;
    i    = addrlist.begin();
    last = addrlist.end();
    for (; i != last; i++) {
      giopAddress* a = (*i)->duplicate();
      pd_addresses.push_back(a);
    }
  }

  {
    omnivector<CORBA::ULong>::const_iterator i, last;
    i    = preferred.begin();
    last = preferred.end();
    for (; i != last; i++) {
      pd_addresses_order.push_back(*i);
    }
  }
}


////////////////////////////////////////////////////////////////////////
giopRope::giopRope(giopAddress* addr,int initialRefCount) :
  pd_refcount(initialRefCount),
  pd_address_in_use(0),
  pd_maxStrands(orbParameters::maxGIOPConnectionPerServer),
  pd_oneCallPerConnection(orbParameters::oneCallPerConnection),
  pd_nwaiting(0),
  pd_cond(omniTransportLock)
{
  pd_addresses.push_back(addr);
  pd_addresses_order.push_back(0);
}

////////////////////////////////////////////////////////////////////////
giopRope::~giopRope() {
  OMNIORB_ASSERT(pd_nwaiting == 0);
  giopAddressList::iterator i, last;
  i    = pd_addresses.begin();
  last = pd_addresses.end();
  for (; i != last; i++) {
    delete (*i);
  }
}

////////////////////////////////////////////////////////////////////////
IOP_C*
giopRope::acquireClient(const omniIOR* ior,
			const CORBA::Octet* key,
			CORBA::ULong keysize,
			omniCallDescriptor* calldesc) {

  GIOP::Version v = ior->getIORInfo()->version();
  giopStreamImpl* impl = giopStreamImpl::matchVersion(v);
  if (!impl) {
    impl = giopStreamImpl::maxVersion();
    v = impl->version();
  }

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);

 again:

  unsigned int nbusy = 0;
  unsigned int ndying = 0;
  unsigned int nwrongver = 0;
  CORBA::ULong max = pd_maxStrands;  // snap the value now as it may
                                     // change by the application anytime.
  RopeLink* p = pd_strands.next;
  for (; p != &pd_strands; p = p->next) {
    giopStrand* s = (giopStrand*)p;
    switch (s->state()) {
    case giopStrand::DYING:
      {
	// Bidirectional strands do not count towards the total of
	// dying strands. This is because with a bidirectional rope,
	// the max number of strands is one. Below, if the number of
	// dying strands is > the max, we wait for the strands to die.
	// However, it is possible that we are the client keeping the
	// strand alive, leading to a deadlock. To avoid the
	// situation, we do not count dying bidir strands, allowing us
	// to create a new one, and release the one that is dying.
	if (!s->biDir)
	  ndying++;
	break;
      }
    case giopStrand::TIMEDOUT:
      {
	s->StrandList::remove();
	s->state(giopStrand::ACTIVE);
	s->StrandList::insert(giopStrand::active);
	// falls through
      }
    case giopStrand::ACTIVE:
      {
	if (s->version.major != v.major || s->version.minor != v.minor) {
	  // Wrong GIOP version. Each strand can only be used
	  // for one GIOP version.
	  // If ever we allow more than one GIOP version
	  // to use one strand, make sure the client side interceptor
	  // for codeset is updated to reflect this.
	  nwrongver++;
	}
	else {
	  GIOP_C* g;
	  if (!giopStreamList::is_empty(s->clients)) {
	    giopStreamList* gp = s->clients.next;
	    for (; gp != &s->clients; gp = gp->next) {
	      g = (GIOP_C*)gp;
	      if (g->state() == IOP_C::UnUsed) {
		g->initialise(ior,key,keysize,calldesc);
		return g;
	      }
	    }
	    nbusy++;
	  }
	  else {
	    g = new GIOP_C(this,s);
	    g->impl(s->giopImpl);
	    g->initialise(ior,key,keysize,calldesc);
	    g->giopStreamList::insert(s->clients);
	    return g;
	  }
	}
      }
    }
  }

  // Reach here if we haven't got a strand to grab a GIOP_C.
  if ((nbusy + ndying) < max) {
    // Create a new strand.
    // Notice that we can have up to
    //  pd_maxStrands * <no. of supported GIOP versions> strands created.
    //
    
    // Do a sanity check here. It could be the case that this rope has
    // no valid address to use. This can be the case if we have
    // unmarshalled an IOR which has no profiles we can support. In
    // theory it shouldn't happen because all IORs must support IIOP.
    // However, this could be a special ORB version in which the IIOP
    // transport is taken out. Notice that we do not raise an
    // exception at the time when the IOR was unmarshalled because we
    // would like to be able to receive and pass along object
    // references that we ourselves cannot talk to.
    if (pd_addresses_order.empty()) {
      OMNIORB_THROW(TRANSIENT,TRANSIENT_NoUsableProfile,CORBA::COMPLETED_NO);
    }

    giopStrand* s = new giopStrand(pd_addresses[pd_addresses_order[pd_address_in_use]]);
    s->state(giopStrand::ACTIVE);
    s->RopeLink::insert(pd_strands);
    s->StrandList::insert(giopStrand::active);
    s->version = v;
    s->giopImpl = impl;
  }
  else if (pd_oneCallPerConnection || ndying >= max) {
    // Wait for a strand to be unused.
    pd_nwaiting++;
    unsigned long deadline_secs,deadline_nanosecs;
    calldesc->getDeadline(deadline_secs,deadline_nanosecs);
    if (deadline_secs || deadline_nanosecs) {
      if (pd_cond.timedwait(deadline_secs,deadline_nanosecs) == 0) {
	pd_nwaiting--;
	OMNIORB_THROW(TRANSIENT,TRANSIENT_CallTimedout,CORBA::COMPLETED_NO);
      }
    }
    else {
      pd_cond.wait();
    }
    pd_nwaiting--;
  }
  else {
    // Pick a random non-dying strand.
    OMNIORB_ASSERT(nbusy);  // There must be a non-dying strand that can
                            // serve this GIOP version
    int n = rand() % max;
    // Pick a random and non-dying strand
    RopeLink* p = pd_strands.next;
    giopStrand* q = 0;
    giopStrand* s = 0;
    while (n >=0 && p != &pd_strands) {
      s = (giopStrand*)p;
      if (s->state() == giopStrand::ACTIVE &&
	  s->version.major == v.major &&
	  s->version.minor == v.minor)
	{
	  n--;
	  if (!q) q = s;
	}
      else {
	s = 0;
      }
      p = p->next;
    }
    s = (s) ? s : q;
    OMNIORB_ASSERT(s);
    GIOP_C* g = new GIOP_C(this,s);
    g->impl(s->giopImpl);
    g->initialise(ior,key,keysize,calldesc);
    g->giopStreamList::insert(s->clients);
    return g;
  }
  goto again;
}

////////////////////////////////////////////////////////////////////////
void
giopRope::releaseClient(IOP_C* iop_c) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);

  GIOP_C* giop_c = (GIOP_C*) iop_c;

  giop_c->rdUnLock();
  giop_c->wrUnLock();

  // We decide in the following what to do with this GIOP_C and the strand
  // it is attached:
  //
  // 1. If the strand is used simultaneously for multiple calls, it will have
  //    multiple GIOP_Cs attached. We only want to keep at most 1 idle GIOP_C.
  //    In other words, if this is not the last GIOP_C attached to the strand
  //    we delete it. (Actually it does no harm to delete all GIOP_C
  //    irrespectively. It will be slower to do an invocation because a
  //    new GIOP_C has to be instantiated in every call.
  //
  // 2. If the strand is in the DYING state, we obviously should delete the
  //    GIOP_C. If this is also the last GIOP_C, we delete the strand as
  //    well. If the strand is used to support bidirectional GIOP, we
  //    also check to ensure that the GIOP_S list is empty.
  //

  giopStrand* s = &((giopStrand&)(*(giopStream*)giop_c));
  giop_c->giopStreamList::remove();

  CORBA::Boolean remove = 0;
  CORBA::Boolean avail = 1;

  if (giop_c->state() != IOP_C::Idle && s->state() != giopStrand::DYING ) {
    s->state(giopStrand::DYING);
    if (omniORB::trace(30)) {
      omniORB::logger l;

      if (s->connection) {
	l << "Unexpected error encountered in talking to the server "
	  << s->connection->peeraddress()
	  << " . The connection is closed immediately.\n";
      }
      else {
	OMNIORB_ASSERT(s->address);
	l << "Unexpected error encountered before talking to the server "
	  << s->address->address()
	  << " . No connection was opened.\n";
      }
    }
  }

  if ( s->state()== giopStrand::DYING ) {
    remove = 1;
    avail = s->safeDelete(); // If safeDelete() returns 1, this strand
                             // can be regarded as deleted. Therefore, we
                             // flag avail to 1 to wake up any threads waiting
                             // on the rope to have a chance to create
                             // another strand.
  }
  else if ( (s->biDir && !s->isClient()) || 
	    !giopStreamList::is_empty(s->clients) ) {
    // We do not cache the GIOP_C if this is server side bidirectional or
    // we already have other GIOP_Cs active or available.
    remove = 1;
    avail = 0;
  }
  else {
    OMNIORB_ASSERT(giop_c->state() == IOP_C::Idle);
    giop_c->giopStreamList::insert(s->clients);
    // The strand is definitely idle from this point onwards, we
    // reset the idle counter so that it will be retired at the right time.
    if ( s->isClient() && !s->biDir ) 
      s->startIdleCounter();
  }

  if (remove) {
    delete giop_c;
  }
  else {
    giop_c->cleanup();
  }

  // If any thread is waiting for a strand to become available, we signal
  // it here.
  if (avail && pd_nwaiting) pd_cond.signal();
}

////////////////////////////////////////////////////////////////////////
void
giopRope::realIncrRefCount() {
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);

  OMNIORB_ASSERT(pd_refcount >= 0);

  if (pd_refcount == 0 && !RopeLink::is_empty(pd_strands)) {
    // This Rope still has some strands in the giopStrand::active_timedout list
    // put there by decrRefCount() when the reference count goes to 0
    // previously. We move these stands back to the giopStrand::active list so
    // that they can be used straight away.
    RopeLink* p = pd_strands.next;
    for (; p != &pd_strands; p = p->next) {
      giopStrand* g = (giopStrand*)p;
      if (g->state() != giopStrand::DYING) {
	g->StrandList::remove();
	g->state(giopStrand::ACTIVE);
	g->StrandList::insert(giopStrand::active);
      }
    }
  }

  pd_refcount++;
}

////////////////////////////////////////////////////////////////////////
void
giopRope::incrRefCount() {
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);
  realIncrRefCount();
}

////////////////////////////////////////////////////////////////////////
void
giopRope::decrRefCount() {
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);
  pd_refcount--;
  OMNIORB_ASSERT(pd_refcount >=0);

  if (pd_refcount) return;

  // This Rope is not used by any object reference.
  // If this rope has no strand, we can remove this instance straight away.
  // Otherwise, we move all the strands from the giopStrand::active list to
  // the giopStrand::active_timedout list. Eventually when all the strands are
  // retired by time out, this instance will also be deleted.

  if (RopeLink::is_empty(pd_strands) && !pd_nwaiting) {
    RopeLink::remove();
    delete this;
  }
  else {
    RopeLink* p = pd_strands.next;
    for (; p != &pd_strands; p = p->next) {
      giopStrand* g = (giopStrand*)p;
      if (g->state() != giopStrand::DYING) {
	g->state(giopStrand::TIMEDOUT);
	// The strand may already be on the active_timedout list. However
	// it is OK to remove and reinsert again.
	g->StrandList::remove();
	g->StrandList::insert(giopStrand::active_timedout);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
const giopAddress*
giopRope::notifyCommFailure(const giopAddress* addr,
			    CORBA::Boolean heldlock) {

  if (heldlock) {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);
  }
  else {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);
    omniTransportLock->lock();
  }

  const giopAddress* addr_in_use;

  addr_in_use = pd_addresses[pd_addresses_order[pd_address_in_use]];
  if (addr == addr_in_use) {
    pd_address_in_use++;
    if (pd_address_in_use >= pd_addresses_order.size())
      pd_address_in_use = 0;
    addr_in_use = pd_addresses[pd_addresses_order[pd_address_in_use]];
  }

  if (!heldlock) {
    omniTransportLock->unlock();
  }
  return addr_in_use;
}

////////////////////////////////////////////////////////////////////////
int
giopRope::selectRope(const giopAddressList& addrlist,
		     omniIOR::IORInfo* info,
		     Rope*& r,CORBA::Boolean& loc) {

  omni_tracedmutex_lock sync(*omniTransportLock);

  // Check if we have to use a bidirectional connection.
  if (BiDirServerRope::selectRope(addrlist,info,r)) {
    loc = 0;
    return 1;
  }

  // Check if these are our addresses
  giopAddressList::const_iterator i, last;
  i    = addrlist.begin();
  last = addrlist.end();
  for (; i != last; i++) {
    if (omniObjAdapter::matchMyEndpoints((*i)->address())) {
      r = 0; loc = 1;
      return 1;
    }
  }

  giopRope* gr;

  // Check if there already exists a rope that goes to the same addresses
  RopeLink* p = giopRope::ropes.next;
  while ( p != &giopRope::ropes ) {
    gr = (giopRope*)p;
    if (gr->match(addrlist)) {
      gr->realIncrRefCount();
      r = (Rope*)gr; loc = 0;
      return 1;
    }
    else if (gr->pd_refcount == 0 &&
	     RopeLink::is_empty(gr->pd_strands) &&
	     !gr->pd_nwaiting) {
      // garbage rope, remove it
      p = p->next;
      gr->RopeLink::remove();
      delete gr;
    }
    else {
      p = p->next;
    }
  }

  // Reach here because we cannot find an existing rope that matches,
  // must create a new one.

  omnivector<CORBA::ULong> prefer_list;
  CORBA::Boolean use_bidir;

  filterAndSortAddressList(addrlist,prefer_list,use_bidir);

  if (!use_bidir) {
    gr = new giopRope(addrlist,prefer_list);
  }
  else {
    if (omniObjAdapter::isInitialised()) {
      gr = new BiDirClientRope(addrlist,prefer_list);
    }
    else {
      omniORB::logs(10, "Client policies specify a bidirectional connection, "
		    "but no object adapters have been initialised. Using a "
		    "non-bidirectional connection.");
      gr = new giopRope(addrlist,prefer_list);
    }
  }
  gr->RopeLink::insert(giopRope::ropes);
  gr->realIncrRefCount();
  r = (Rope*)gr; loc = 0;
  return 1;
}


////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopRope::match(const giopAddressList& addrlist) const
{
  if (addrlist.size() != pd_addresses.size()) return 0;

  giopAddressList::const_iterator i, last, j;
  i    = addrlist.begin();
  j    = pd_addresses.begin();
  last = addrlist.end();
  for (; i != last; i++, j++) {
    if (!omni::ptrStrMatch((*i)->address(),(*j)->address())) return 0;
  }
  return 1;
}

////////////////////////////////////////////////////////////////////////
void
giopRope::filterAndSortAddressList(const giopAddressList& addrlist,
				   omnivector<CORBA::ULong>& ordered_list,
				   CORBA::Boolean& use_bidir)
{
  // We consult the clientTransportRules to decide which address is more
  // preferable than others. The rules may forbid the use of some of the
  // addresses and these will be filtered out. We then record the order
  // of the remaining addresses in order_list.
  // If any of the non-exlusion clientTransportRules have the "bidir"
  // attribute, use_bidir will be set to 1, otherwise it is set to 0.

  use_bidir = 0;

  // For each address, find the rule that is applicable. Record the
  // rules priority in the priority list.
  omnivector<CORBA::ULong> prioritylist;

  CORBA::ULong index;
  CORBA::ULong total = addrlist.size();
  for (index = 0; index < total; index++) {
    transportRules::sequenceString actions;
    CORBA::ULong matchedRule;

    if ( transportRules::clientRules().match(addrlist[index]->address(),
					     actions,matchedRule)        ) {

      const char* transport = strchr(addrlist[index]->type(),':');
      OMNIORB_ASSERT(transport);
      transport++;
      
      CORBA::ULong i;
      CORBA::Boolean matched = 0;
      CORBA::Boolean usebidir = 0;
      CORBA::ULong priority;
      for (i = 0; i < actions.length(); i++ ) {
	size_t len = strlen(actions[i]);
	if (strncmp(actions[i],transport,len) == 0 ) {
	  priority = (matchedRule << 16) + i;
	  matched = 1;
	}
	else if ( strcmp(actions[i],"none") == 0 ) {
	  break;
	}
	else if ( strcmp(actions[i],"bidir") == 0 ) {
	  usebidir = 1;
	}
      }
      if (matched) {
	ordered_list.push_back(index);
	prioritylist.push_back(priority);
	if (usebidir && orbParameters::offerBiDirectionalGIOP) {
	  use_bidir = 1;
	}
      }
    }
  }

  // If we have more than 1 addresses to use, sort them according to
  // their value in prioritylist.
  if ( ordered_list.size() > 1 ) {
    // Won't it be nice to just use stl qsort? It is tempting to just
    // forget about old C++ compiler and use stl. Until the time has come
    // use shell sort to sort the addresses in order.
    int n = ordered_list.size();
    for (int gap=n/2; gap > 0; gap=gap/2 ) {
      for (int i=gap; i < n ; i++)
	for (int j =i-gap; j>=0; j=j-gap) {
	  if ( prioritylist[j] > prioritylist[j+gap] ) {
	    CORBA::ULong temp = ordered_list[j];
	    ordered_list[j] = ordered_list[j+gap];
	    ordered_list[j+gap] = temp;
	    temp = prioritylist[j];
	    prioritylist[j] = prioritylist[j+gap];
	    prioritylist[j+gap] = temp;
	  }
	}
    }
  }
#if 0
  {
    omniORB::logger log;
    log << "Sorted addresses are: \n";
    for (int i=0; i<ordered_list.size(); i++) {
      log << addrlist[ordered_list[i]]->address() << "\n";
    }
  }
#endif
}

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class oneCallPerConnectionHandler : public orbOptions::Handler {
public:

  oneCallPerConnectionHandler() : 
    orbOptions::Handler("oneCallPerConnection",
			"oneCallPerConnection = 0 or 1",
			1,
			"-ORBoneCallPerConnection < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::oneCallPerConnection = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::oneCallPerConnection,
			     result);
  }
};

static oneCallPerConnectionHandler oneCallPerConnectionHandler_;

/////////////////////////////////////////////////////////////////////////////
class maxGIOPConnectionPerServerHandler : public orbOptions::Handler {
public:

  maxGIOPConnectionPerServerHandler() : 
    orbOptions::Handler("maxGIOPConnectionPerServer",
			"maxGIOPConnectionPerServer = n > 0",
			1,
			"-ORBmaxGIOPConnectionPerServer < n > 0 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
			 orbOptions::expect_greater_than_zero_ulong_msg);
    }
    orbParameters::maxGIOPConnectionPerServer = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::maxGIOPConnectionPerServer,
			   result);
  }

};

static maxGIOPConnectionPerServerHandler maxGIOPConnectionPerServerHandler_;


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_giopRope_initialiser : public omniInitialiser {
public:

  omni_giopRope_initialiser() {
    orbOptions::singleton().registerHandler(oneCallPerConnectionHandler_);
    orbOptions::singleton().registerHandler(maxGIOPConnectionPerServerHandler_);
  }

  void attach() {
  }
  void detach() {
    // Get rid of any remaining ropes. By now they should all be strand-less.
    omni_tracedmutex_lock sync(*omniTransportLock);

    RopeLink* p = giopRope::ropes.next;
    giopRope* gr;
    int i=0;

    while (p != &giopRope::ropes) {
      gr = (giopRope*)p;
      OMNIORB_ASSERT(gr->pd_refcount == 0 &&
		     RopeLink::is_empty(gr->pd_strands) &&
		     !gr->pd_nwaiting);
      p = p->next;
      gr->RopeLink::remove();
      delete gr;
      ++i;
    }
    if (omniORB::trace(15)) {
      omniORB::logger l;
      l << i << " remaining rope" << (i == 1 ? "" : "s") << " deleted.\n";
    }
  }
};


static omni_giopRope_initialiser initialiser;

omniInitialiser& omni_giopRope_initialiser_ = initialiser;


OMNI_NAMESPACE_END(omni)
