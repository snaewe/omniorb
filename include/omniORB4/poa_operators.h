// -*- Mode: C++; -*-
//                            Package   : omniORB
// poa_operators.h            Created on: 19/7/99
//                            Author    : David Riddoch (djr)
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
//

/*
  $Log$
  Revision 1.2.2.4  2005/01/24 18:29:52  dgrisby
  HPUX 11.23 support. Thanks Matej Kenda.

  Revision 1.2.2.3  2001/07/31 16:04:07  sll
  Added ORB::create_policy() and associated types and operators.

  Revision 1.2.2.2  2000/09/27 16:57:15  sll
  Replaced marshalling operators for MemBufferedStream and NetBufferedStream
  with just one type for cdrStream.

  Revision 1.2.2.1  2000/07/17 10:35:36  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:04  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.1  1999/09/24 09:51:50  djr
  Moved from omniORB2 + some new files.

*/

#ifndef __OMNIPOA_H__
#error poa_operators.h should only be included by poa.h
#endif

#ifndef __OMNI_POA_OPERATORS_H__
#define __OMNI_POA_OPERATORS_H__

inline void
PortableServer::AdapterActivator::_marshalObjRef(PortableServer::AdapterActivator_ptr obj, cdrStream& s) {
  omniObjRef::_marshal(obj->_PR_getobj(),s);
}

inline void
PortableServer::ServantManager::_marshalObjRef(PortableServer::ServantManager_ptr obj, cdrStream& s) {
  omniObjRef::_marshal(obj->_PR_getobj(),s);
}

inline void
PortableServer::ServantActivator::_marshalObjRef(PortableServer::ServantActivator_ptr obj, cdrStream& s) {
  omniObjRef::_marshal(obj->_PR_getobj(),s);
}

inline void
PortableServer::ServantLocator::_marshalObjRef(PortableServer::ServantLocator_ptr obj, cdrStream& s) {
  omniObjRef::_marshal(obj->_PR_getobj(),s);
}

OMNIORB_DECLARE_POLICY_OBJECT_OPERATORS(PortableServer::OMNIORB_POLICY_VALUE(ThreadPolicy))
OMNIORB_DECLARE_POLICY_OBJECT_OPERATORS(PortableServer::OMNIORB_POLICY_VALUE(LifespanPolicy))
OMNIORB_DECLARE_POLICY_OBJECT_OPERATORS(PortableServer::OMNIORB_POLICY_VALUE(IdUniquenessPolicy))
OMNIORB_DECLARE_POLICY_OBJECT_OPERATORS(PortableServer::OMNIORB_POLICY_VALUE(IdAssignmentPolicy))
OMNIORB_DECLARE_POLICY_OBJECT_OPERATORS(PortableServer::OMNIORB_POLICY_VALUE(ImplicitActivationPolicy))
OMNIORB_DECLARE_POLICY_OBJECT_OPERATORS(PortableServer::OMNIORB_POLICY_VALUE(ServantRetentionPolicy))
OMNIORB_DECLARE_POLICY_OBJECT_OPERATORS(PortableServer::OMNIORB_POLICY_VALUE(RequestProcessingPolicy))

#endif  // __OMNI_POA_OPERATORS_H__
