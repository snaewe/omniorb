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

#include <stdlib.h>

OMNI_NAMESPACE_BEGIN(omni)

///////////////////////////////////////////////////////////////////////
RopeLink giopRope::ropes;

////////////////////////////////////////////////////////////////////////
giopRope::giopRope(const giopAddressList& addrlist,
		   const omnivector<CORBA::ULong>& preferred) :
  pd_refcount(0),
  pd_address_in_use(0),
  pd_maxStrands(omniORB::maxTcpConnectionPerServer),
  pd_oneCallPerConnection(omniORB::oneCallPerConnection),
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
  pd_maxStrands(omniORB::maxTcpConnectionPerServer),
  pd_oneCallPerConnection(omniORB::oneCallPerConnection),
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
    // no valid address to use. This can be the case if we have unmarshalled
    // an IOR in which none of the profiles we can support. In theory it
    // shouldn't happen because all IOR must support IIOP. However, this
    // could be a special ORB version in which the IIOP transport is taken
    // out. Notice that we do not raise an exception at the time when the IOR
    // was unmarshalled because we would like to be able to receive and pass
    // along object references that we ourselves cannot talk to.
    //
    // XXX Actually this is not quite true yet. If the IOR hasn't at
    // least have an IIOP profile, MARSHAL exception is raised! This should
    // be fixed.
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
    pd_cond.wait();   // XXX Should do time want if deadline is set.
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
      l << "Unexpected error encountered in talking to the server "
	<< s->connection->peeraddress()
	<< " , the connection is closed immediately.\n";
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
    if (s->isClient()) s->resetIdleCounter(giopStrand::idleOutgoingBeats);
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
giopRope::notifyCommFailure(const giopAddress* addr) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

  omni_tracedmutex_lock sync(*omniTransportLock);

  const giopAddress* addr_in_use;

  addr_in_use = pd_addresses[pd_addresses_order[pd_address_in_use]];
  if (addr != addr_in_use) return addr_in_use;

  pd_address_in_use++;
  if (pd_address_in_use >= pd_addresses_order.size())
    pd_address_in_use = 0;
  return pd_addresses[pd_addresses_order[pd_address_in_use]];
}

////////////////////////////////////////////////////////////////////////
int
giopRope::selectRope(const giopAddressList& addrlist,
		     omniIOR::IORInfo* info,
		     Rope*& r,CORBA::Boolean& loc) {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);

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
    gr = new BiDirClientRope(addrlist,prefer_list);
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
  // We should consult the configuration table to decide which address is
  // more preferable than the others. Some of the addresses in addrlist
  // may not be usable anyway. We then record the order of addresses to use
  // in pd_addresses_order.

  // XXX Since we haven't got a configuration table yet, we use all the
  //     addresses and use them in the order as supplied.
  CORBA::ULong index;
  CORBA::ULong total = addrlist.size();
  for (index = 0; index < total; index++)
    ordered_list.push_back(index);

  // XXX Make SSL the first one to try if it is available.
  for (index = 0; index < total; index++) {
    if (strcmp(addrlist[index]->type(),"giop:ssl")==0) {
      ordered_list[index] = ordered_list[0];
      ordered_list[0] = index;
      break;
    }
  }

  // XXX Make unix socket the first one to try if it is available.
  for (index = 0; index < total; index++) {
    if (strcmp(addrlist[index]->type(),"giop:unix")==0) {
      ordered_list[index] = ordered_list[0];
      ordered_list[0] = index;
      break;
    }
  }

  if (omniORB::offerBiDirectionalGIOP) {
    // XXX in future, we will be more selective as to which addresses will
    // use bidirectional.
    use_bidir = 1;
    // XXX A temporary, ugly and local hack to make sure that we do not
    //     use bidir to contact our naming service even when 
    //     offerBiDirectionalGIOP is set. This is done so that our testsuite
    //     works when using the naming service to pass the IOR. (Our code
    //     resolves the IOR before initialising a POA.)
    if (!addrlist.empty() && 
	strcmp(addrlist[0]->address(),"giop:tcp:158.124.64.61:5009") == 0) {
      use_bidir = 0;
    }
  }
  else {
    use_bidir = 0;
  }


}

OMNI_NAMESPACE_END(omni)
