// -*- Mode: C++; -*-
//                            Package   : omniORB
// shutdownIdentity.h         Created on: 2001/09/17
//                            Author    : Duncan Grisby (dpg1)
//
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
  Revision 1.1.2.1  2001/09/19 17:26:47  dpg1
  Full clean-up after orb->destroy().

*/

#ifndef __OMNIORB_SHUTDOWNIDENTITY_H__
#define __OMNIORB_SHUTDOWNIDENTITY_H__

#include <omniIdentity.h>


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

  static void* thisClassCompare(omniIdentity*, void*);

  static inline omniShutdownIdentity* downcast(omniIdentity* id)
  {
    return (omniShutdownIdentity*)(id->classCompare()
				   (id, (void*)thisClassCompare));
  }

  static omniShutdownIdentity* singleton();
  // Must hold <omni::internalLock>.

private:
  int pd_refCount;

  static _CORBA_Boolean real_is_equivalent(const omniIdentity*,
					   const omniIdentity*);

  inline omniShutdownIdentity(classCompare_fn compare = thisClassCompare)
    : omniIdentity(compare),
      pd_refCount(0)
  {}
};

#endif // __OMNIORB_SHUTDOWNIDENTITY_H__
