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
  Revision 1.1.2.2  2000/10/03 17:37:07  sll
  Changed omniIOR synchronisation mutex from omni::internalLock to its own
  mutex.

  Revision 1.1.2.1  2000/09/27 16:54:08  sll
  *** empty log message ***

*/

#ifndef __OMNIIOR_H__
#define __OMNIIOR_H__

class ropeFactoryType;

class omniIOR {
public:

  _CORBA_String_member               repositoryID;
  IOP::TaggedProfileList_var 	     iopProfiles;

  _CORBA_Boolean                     decoded;

  //
  ropeFactoryType*                   selectedRopeFactoryType;

  // The following are decoded from the IIOP Decoded
  IIOP::ProfileBody                  iiop;

  GIOP::AddressingDisposition        addr_mode;
  _CORBA_ULong                       addr_selected_profile_index;
  _CORBA_ULong                       orb_type;




  // Extra info decoded from tag_components_ are storied as
  // opaque data accessible by the relevant modules.
  struct opaque {
    void* data;
    void (*destructor)(void*);
  };

  typedef _CORBA_PseudoValue_Sequence<opaque> opaque_sequence;
  opaque_sequence*                            opaque_data;

  omniIOR(char* repoId, IOP::TaggedProfileList* iop);
  // Both repoId and iop are consumed by the object.

  omniIOR(char* repoId, IOP::TaggedProfile* iop, _CORBA_ULong niops,
	  _CORBA_ULong selected_profile_index);
  // Both repoId and iop are consumed by the object.

  omniIOR(const char* repoId, omniIdentity* id);
  // create an IOR for this local object
  //
  // ** Caller holds lock on internalLock.

  omniIOR(const char* repoId, 
	  const _CORBA_Unbounded_Sequence_Octet& key,
	  const IIOP::Address* addrs, _CORBA_ULong naddrs);

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

private:
  int pd_refCount;
  // Protected by <omni::internalLock>

  omniIOR();
  omniIOR(const omniIOR&);
  omniIOR& operator=(const omniIOR&);


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
