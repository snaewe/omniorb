// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniIOR.h                  Created on: 11/8/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1999-2000 AT&T Laboratories, Cambridge
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
  Revision 1.1.2.3.2.1  2001/02/23 16:50:43  sll
  SLL work in progress.

  Revision 1.1.2.3  2000/11/15 17:06:49  sll
  Added tcs_c and tcs_w in omniIOR to record what code set convertor to use
  for this IOR.
  Added declarations for handlers of the tagged components used by the ORB.

  Revision 1.1.2.2  2000/10/03 17:37:07  sll
  Changed omniIOR synchronisation mutex from omni::internalLock to its own
  mutex.

  Revision 1.1.2.1  2000/09/27 16:54:08  sll
  *** empty log message ***

*/

#ifndef __OMNIIOR_H__
#define __OMNIIOR_H__

#include <omniORB4/giopEndpoint.h>

class _OMNI_NS(Rope);

class omniIOR {
public:

  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  // Accessor functions for the 2 components of the IOR
  const char* repositoryID() const {
    return pd_repositoryID;
  }

  const IOP::TaggedProfileList& iopProfiles() const {
    return pd_iopProfiles;
  }


  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  // set and get functions for addr_mode.
  // The value of addr_mode determines what AddressingDisposition mode
  // the ORB will use to invoke on the object. The mode is only relevant
  // for GIOP 1.2 upwards.
  GIOP::AddressingDisposition addr_mode() {
    return pd_addr_mode;
  }
  
  void addr_mode(GIOP::AddressingDisposition m) {
    pd_addr_mode = m;
  }
  
  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  // The following accessor functions *must* only be called when
  // the ior is in decoded state.

  _CORBA_Boolean decoded() const {
    return pd_decoded;
  }

  const IIOP::ProfileBody&  getIIOPprofile() const {
    OMNIORB_ASSERT(pd_decoded);
    return pd_iiop;
  }

  omniCodeSet::TCS_C* TCS_C() const {
    OMNIORB_ASSERT(pd_decoded);
    return pd_tcs_c;
  }
  omniCodeSet::TCS_W* TCS_W() const {
    OMNIORB_ASSERT(pd_decoded);
    return pd_tcs_w;
  }    

  ///////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  int selectRope(_OMNI_NS(Rope)*& rope, _CORBA_Boolean& is_local);
  // Look at the IOR, returns the
  // most suitable Rope to talk to the object.
  // The reference count of the rope returned is first incremented.
  //  If the object is in fact a local object, set <is_local> to TRUE.
  // In this case <rope> will be 0 on return.
  //  Returns 0 on failure.
  //  This function does not throw any exceptions.
  // The side-effect of this function is to decode the IOP profiles.
  // After this function returns, decoded() returns true.
  
public:

  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  omniIOR(char* repoId, IOP::TaggedProfileList* iop);
  // Both repoId and iop are consumed by the object.

  omniIOR(char* repoId, IOP::TaggedProfile* iop, _CORBA_ULong niops,
	  _CORBA_ULong selected_profile_index);
  // Both repoId and iop are consumed by the object.

  omniIOR(const char* repoId, omniIdentity* id);
  // create an IOR for this local object
  //
  // ** Caller holds lock on internalLock.

  ///////////////////////////////////////////////////////////////////
  // Create a new taggedcomponent slot in the IIOP profile.
  // The caller can write to the tagged component immediately after this
  // call.
  // Use this call only in the encodeIOR interceptors.
  IOP::TaggedComponent& newIIOPtaggedComponent();

  omniIOR(const char* repoId, 
	  const _CORBA_Unbounded_Sequence_Octet& key,
	  const IIOP::Address* addrs, _CORBA_ULong naddrs,
	  GIOP::Version ver, _CORBA_Boolean call_interceptors);

  ~omniIOR();

  void marshalIORAddressingInfo(cdrStream& s);

  // Synchronisation and reference counting:
  //
  // The object is reference counted. Call duplcate() to increment the
  // reference count. Call release() to decrement the reference count.
  //
  // The reference count is protected by the mutex omniIOR::lock.
  //
  omniIOR* duplicate();
  // return a pointer to this object.
  // Atomic and thread safe. Caller must not hold omniIOR::lock.

  void release();
  // If the reference count is 0, delete is called on the object.
  // Atomic and thread safe. Caller must not hold omniIOR::lock

  static _core_attr omni_tracedmutex* lock;


public:
  // ORB internal functions.

  omniIOR* duplicateNoLock();
  // Must hold <omniIOR::lock>. Otherwise same semantics as duplicate().

  void releaseNoLock();
  // Must hold <omniIOR::lock>. Otherwise same semantics as release().

  void clearDecodedMembers();
  // Reduce the memory foot print by clearing out all the decoded members.
  // On return, the only valid fields are:
  //    repositoryID, iopProfiles and decoded.
  // 

  void encodeIOPprofile(_CORBA_Boolean call_interceptors = 1);
  // Encode the information in pd_iiop into pd_iopProfiles.
  // If call_interceptors,
  //    calls the encodeIOR interceptors before the encoding.
  //
  // <call_interceptors> should only be set to 0 if the instance has already
  // got the address and object key filled in.
  //
  // If this is a local object, the caller may leave pd_iiop.address unset.
  // In that case, the real address of the ORB will be filled in by the
  // default encodeIOR interceptor.

  // Handlers for each of the tagged component used by the ORB
  static void  add_TAG_ORB_TYPE(IOP::TaggedComponent&, const omniIOR*);
  static void  unmarshal_TAG_ORB_TYPE(const IOP::TaggedComponent&, omniIOR*);
  static char* dump_TAG_ORB_TYPE(const IOP::TaggedComponent&);

  static void  add_TAG_CODE_SETS(IOP::TaggedComponent&, const omniIOR*);
  static void  unmarshal_TAG_CODE_SETS(const IOP::TaggedComponent&, omniIOR*);
  static char* dump_TAG_CODE_SETS(const IOP::TaggedComponent&);

private:

  _CORBA_String_member               pd_repositoryID;
  IOP::TaggedProfileList_var 	     pd_iopProfiles;

  _CORBA_Boolean                     pd_decoded;

  _OMNI_NS(giopAddressList)          pd_addresses;

  // The following are decoded from the IIOP Decoded
  IIOP::ProfileBody                  pd_iiop;

  GIOP::AddressingDisposition        pd_addr_mode;
  _CORBA_Long                        pd_addr_selected_profile_index;
  _CORBA_ULong                       pd_orb_type;
  omniCodeSet::TCS_C*                pd_tcs_c;
  omniCodeSet::TCS_W*                pd_tcs_w;

  int                                pd_refCount;
  // Protected by <omni::internalLock>

  omniIOR();
  omniIOR(const omniIOR&);
  omniIOR& operator=(const omniIOR&);

};

class omniIOR_var {
public:
  inline omniIOR_var() : pd_ior(0) {}
  inline omniIOR_var(omniIOR* ior) : pd_ior(ior) {}
  inline omniIOR* _retn() { omniIOR* p = pd_ior; pd_ior = 0; return p; }
  inline operator omniIOR* () const { return pd_ior; }
  inline omniIOR* operator->() const { return pd_ior; }
  inline omniIOR_var& operator=(omniIOR* p) { 
    if (pd_ior) pd_ior->release();
    pd_ior = p;
    return *this;
  }
  inline ~omniIOR_var() {
    if (pd_ior) pd_ior->release();
  }
private:
  omniIOR* pd_ior;
};


#endif // __OMNIIOR_H__
