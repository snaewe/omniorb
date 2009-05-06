// -*- Mode: C++; -*-
//                            Package   : omniORB
// shutdownIdentity.h         Created on: 2001/09/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2007 Apasphere Ltd
//    Copyright (C) 2001 AT&T Research Cambridge
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
//    Dummy singleton identity placed into object references when the
//    ORB is shutting down.
//

/*
  $Log$
  Revision 1.1.4.3  2009/05/06 16:16:01  dgrisby
  Update lots of copyright notices.

  Revision 1.1.4.2  2007/04/14 17:56:52  dgrisby
  Identity downcasting mechanism was broken by VC++ 8's
  over-enthusiastic optimiser.

  Revision 1.1.4.1  2003/03/23 21:03:42  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.1  2001/09/19 17:26:47  dpg1
  Full clean-up after orb->destroy().

*/

#ifndef __OMNIORB_SHUTDOWNIDENTITY_H__
#define __OMNIORB_SHUTDOWNIDENTITY_H__

#include <omniIdentity.h>

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif


class omniShutdownIdentity : public omniIdentity {
public:
  ~omniShutdownIdentity() { }

  virtual void dispatch(omniCallDescriptor&);
  // Always throws BAD_INV_ORDER.

  virtual void gainRef(omniObjRef* obj = 0);
  virtual void loseRef(omniObjRef* obj = 0);
  virtual omniIdentity::equivalent_fn get_real_is_equivalent() const;
  // Override omniIdentity.

  virtual void locateRequest(omniCallDescriptor&);
  // Always indicates that the object is here.

  virtual _CORBA_Boolean inThisAddressSpace();
  // Returns false.


  virtual void* ptrToClass(int* cptr);
  static inline omniShutdownIdentity* downcast(omniIdentity* i) {
    return (omniShutdownIdentity*)i->ptrToClass(&_classid);
  }
  static _core_attr int _classid;
  // Dynamic casting mechanism.


  static omniShutdownIdentity* singleton();
  // Must hold <omni::internalLock>.

private:
  int pd_refCount;

  static _CORBA_Boolean real_is_equivalent(const omniIdentity*,
					   const omniIdentity*);

  inline omniShutdownIdentity()
    : omniIdentity(),
      pd_refCount(0)
  {}
};

#undef _core_attr

#endif // __OMNIORB_SHUTDOWNIDENTITY_H__
