// -*- Mode: C++; -*-
//                            Package   : omniORB2
// remoteGroupIdentity.h      Created on: 18/01/01
//                            Author    : Bob Gruber (reg)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
//    Encapsulation of the identity of a remote object group implementation.
//      
 
/*
  $Log$
  Revision 1.1.2.1  2001/02/23 19:35:22  sll
  Merged interim FT client stuff.

*/

#ifndef __OMNIORB_REMOTEGROUPIDENTITY_H__
#define __OMNIORB_REMOTEGROUPIDENTITY_H__

#include <omniIdentity.h>
#include <remoteIdentity.h>

class omniRemoteGroupIdentity_RefHolder;

class omniRemoteGroupIdentity : public omniIdentity {
public:
  omniRemoteGroupIdentity(omniIOR* ior, CORBA::Boolean locked);
  // Constructor consumes <ior>; initial ref count of 0.

  virtual void dispatch(omniCallDescriptor&);
  virtual void gainObjRef(omniObjRef*);
  virtual void loseObjRef(omniObjRef*);
  // Overrides omniIdentity.

  void locateRequest();
  // If this returns normally, then the object exists.
  // Throws OBJECT_NOT_EXIST, or omniORB::LOCATION_FORWARD
  // otherwise.


private:
  friend class omniRemoteGroupIdentity_RefHolder;

  ~omniRemoteGroupIdentity();

  omniRemoteGroupIdentity(const omniRemoteGroupIdentity&);
  omniRemoteGroupIdentity& operator = (const omniRemoteGroupIdentity&);
  // Not implemented.


  typedef _CORBA_PseudoValue_Sequence<omniRemoteIdentity*> TargetSeq;

  int           pd_refCount;
  omniIOR*      pd_ior;
  TargetSeq     pd_targets;
  int           pd_selected_target;
};


#endif  // __OMNIORB_REMOTEGROUPIDENTITY_H__
