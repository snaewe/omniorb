// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopRope.h               Created on: 05/01/2001
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
//	*** PROPRIETORY INTERFACE ***
//

/*
  $Log$
  Revision 1.1.6.3  2009/05/05 14:44:39  dgrisby
  Ropes rememeber the bidir configuration set at the time of their
  creation, meaning it can safely be changed at run time.

  Revision 1.1.6.2  2006/03/26 20:59:28  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.6.1  2003/03/23 21:03:48  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.7  2002/08/21 06:23:15  dgrisby
  Properly clean up bidir connections and ropes. Other small tweaks.

  Revision 1.1.4.6  2001/09/19 17:26:46  dpg1
  Full clean-up after orb->destroy().

  Revision 1.1.4.5  2001/09/04 14:38:09  sll
  Added the boolean argument to notifyCommFailure to indicate if
  omniTransportLock is held by the caller.

  Revision 1.1.4.4  2001/08/03 17:43:19  sll
  Make sure dll import spec for win32 is properly done.

  Revision 1.1.4.3  2001/07/31 16:24:23  sll
  Moved filtering and sorting of available addresses into a separate
  function. Make acquireClient, decrRefCount and notifyCommFailure virtual.

  Revision 1.1.4.2  2001/06/13 20:11:37  sll
  Minor update to make the ORB compiles with MSVC++.

  Revision 1.1.4.1  2001/04/18 17:19:00  sll
  Big checkin with the brand new internal APIs.

  Revision 1.1.2.1  2001/02/23 16:47:04  sll
  Added new files.

  */


#ifndef __GIOPROPE_H__
#define __GIOPROPE_H__

#include <omniORB4/omniTransport.h>

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

class omniIOR;

OMNI_NAMESPACE_BEGIN(omni)

class giopStream;
class giopStrand;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class giopRope : public Rope, public RopeLink {
 public:

  static int selectRope(const giopAddressList&,
			omniIOR::IORInfo*,
			Rope*&,
			CORBA::Boolean& is_local);
  // Given an address list, return a rope that can be used to talk to
  // remote objects in that address space. If the address list is in fact
  // points to ourselves, set <is_local> to TRUE(1).
  // Returns TRUE(1) if either the addresses are local or the rope is found.
  // Returns FALSE(0) if no suitable rope is find.
  // If a rope is returned, the reference count on the rope is already
  // incremented by 1.
  //
  // This is normally the entry function that causes a rope to be created
  // in the first place.
  //
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.

  giopRope(const giopAddressList& addrlist, 
	   const omnivector<CORBA::ULong>& preferred);
  // <list> & <preferred> are copied.
  // Reference count is initialised to 0.
  // No thread safety precondition

  giopRope(giopAddress* addr, int i = 0);
  // <addr> is consumed by this instance.
  // Reference count is initialised to <i>
  // No thread safety precondition

  virtual ~giopRope();
  // No thread safety precondition


  virtual IOP_C* acquireClient(const omniIOR*,
			       const CORBA::Octet* key,
			       CORBA::ULong keysize,
			       omniCallDescriptor*);
  // Acquire a GIOP_C from this rope.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.

  void releaseClient(IOP_C*);
  // Release the GIOP_C back to this rope. The GIOP_C must have been acquired
  // previously through acquireClient from this rope. Passing in a GIOP_C
  // from a different rope would result in undefined behaviour.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.


  void incrRefCount();
  // Increment the reference count by 1.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.

  virtual void decrRefCount();
  // Decrement the reference count by 1. If the reference count becomes
  // 0, the rope will be deleted at the earliest convenient time.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.

  CORBA::Boolean hasAddress(const giopAddress*);
  // Returns true if the address is in this rope's address list; false
  // otherwise.
  //
  // Thread Safety preconditions:
  //    None: the list of addresses is constant once set.

  virtual const giopAddress* notifyCommFailure(const giopAddress*,
					       CORBA::Boolean heldlock);
  // Caller detects an error in sending or receiving data with this address.
  // It calls this function to indicate to the rope that the address is bad.
  // If the rope has other alternative addresses, it should select another
  // address next time acquireClient is called.
  //
  // Returns the next address the rope will try. In the case when there is
  // only 1 address, the return value will be the same as the argument.
  // When there are more than 1 addresses and the caller decides to retry
  // an invocation on all of these addresses, the caller can use the return
  // value to decide when all the addresses have been tried. This
  // is done by comparing the return value with the address in use when
  // the first call is made.
  //
  // Thread Safety preconditions:
  //    Internally, omniTransportLock is used for synchronisation, if
  //    <heldlock> is TRUE(1), the caller already hold the lock.

  // Access functions to change the rope parameters. Notice that these
  // functions does not perform any mutual exclusion internally. It is
  // however safe to change the parameters while the rope is in use.  The
  // new parameter value will take effect as soon as appropriate.  For
  // instance, decrease the max. number of strand will not have any effect
  // on these strands that have already been created.

  CORBA::Boolean oneCallPerConnection() {
    // No thread safety precondition, use with extreme care
    // return True(1) if only one call can be in progress at any time on
    // each strand.
    // return False(0) if the same strand can be used to carry multiple
    // concurrent calls.
    // The default is True.
    return pd_oneCallPerConnection;
  }

  void oneCallPerConnection(CORBA::Boolean yes) {
  // No thread safety precondition, use with extreme care
    pd_oneCallPerConnection = yes;
  }

  CORBA::ULong maxStrands() {
    // No thread safety precondition, use with extreme care
    // Return the maximum no. of strands that can be opened.
    // The default is omniORB::maxTcpConnectionPerServer.
    return pd_maxStrands;
  }

  void maxStrands(CORBA::ULong max) {
    // No thread safety precondition, use with extreme care
    if (max == 0) max = 1;
    pd_maxStrands = max;
  }

  friend class giopStream;
  friend class giopStrand;
  friend class omni_giopRope_initialiser;
  friend class omni_giopbidir_initialiser;

 protected:
  int                  pd_refcount;  // reference count
  giopAddressList      pd_addresses; // Addresses of the remote address space
  omnivector<CORBA::ULong>              pd_addresses_order;
  omnivector<CORBA::ULong>::size_type   pd_address_in_use;
  CORBA::ULong         pd_maxStrands;
  CORBA::Boolean       pd_oneCallPerConnection;
  int                  pd_nwaiting;
  omni_tracedcondition pd_cond;
  CORBA::Boolean       pd_offerBiDir; // State of orbParameters::offerBiDir...
				      // at time of creation.

  static _core_attr RopeLink ropes;
  // All ropes created by selectRope are linked together by this list.

  virtual void realIncrRefCount();
  // Really increment the reference count.
  //
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock.

  CORBA::Boolean match(const giopAddressList&) const;
  // Return TRUE(1) if the address list matches EXACTLY those of this rope.
  // No thread safety precondition

  static void filterAndSortAddressList(const giopAddressList& list,
				       omnivector<CORBA::ULong>& ordered_list,
				       CORBA::Boolean& use_bidir);
  // Consult the configuration table, filter out the addresses in <list> that
  // should not be used. Sort the remaining ones in the order of preference.
  // Write out the *index* of the sorted addresses into <ordered_list>.
  // Set use_bidir to 1 if bidirection GIOP is to be used for the sorted
  // addresses.

 private:
  giopRope();
  giopRope(const giopRope&);
  giopRope& operator=(const giopRope&);
};

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __GIOPROPE_H__
