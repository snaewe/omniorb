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
  Revision 1.1.2.1  1999/09/24 09:51:50  djr
  Moved from omniORB2 + some new files.

*/

#ifndef __OMNIPOA_H__
#error poa_operators.h should only be included by poa.h
#endif

#ifndef __OMNI_POA_OPERATORS_H__
#define __OMNI_POA_OPERATORS_H__


inline size_t
PortableServer::AdapterActivator::_alignedSize(PortableServer::AdapterActivator_ptr obj, size_t offset) {
  return CORBA::AlignedObjRef(obj, _PD_repoId, 48, offset);
}

inline void
PortableServer::AdapterActivator::_marshalObjRef(PortableServer::AdapterActivator_ptr obj, NetBufferedStream& s) {
  CORBA::MarshalObjRef(obj, _PD_repoId, 48, s);
}

inline void
PortableServer::AdapterActivator::_marshalObjRef(PortableServer::AdapterActivator_ptr obj, MemBufferedStream& s) {
  CORBA::MarshalObjRef(obj, _PD_repoId, 48, s);
}


inline size_t
PortableServer::ServantManager::_alignedSize(PortableServer::ServantManager_ptr obj, size_t offset) {
  return CORBA::AlignedObjRef(obj, _PD_repoId, 46, offset);
}

inline void
PortableServer::ServantManager::_marshalObjRef(PortableServer::ServantManager_ptr obj, NetBufferedStream& s) {
  CORBA::MarshalObjRef(obj, _PD_repoId, 46, s);
}

inline void
PortableServer::ServantManager::_marshalObjRef(PortableServer::ServantManager_ptr obj, MemBufferedStream& s) {
  CORBA::MarshalObjRef(obj, _PD_repoId, 46, s);
}


inline size_t
PortableServer::ServantActivator::_alignedSize(PortableServer::ServantActivator_ptr obj, size_t offset) {
  return CORBA::AlignedObjRef(obj, _PD_repoId, 48, offset);
}

inline void
PortableServer::ServantActivator::_marshalObjRef(PortableServer::ServantActivator_ptr obj, NetBufferedStream& s) {
  CORBA::MarshalObjRef(obj, _PD_repoId, 48, s);
}

inline void
PortableServer::ServantActivator::_marshalObjRef(PortableServer::ServantActivator_ptr obj, MemBufferedStream& s) {
  CORBA::MarshalObjRef(obj, _PD_repoId, 48, s);
}


inline size_t
PortableServer::ServantLocator::_alignedSize(PortableServer::ServantLocator_ptr obj, size_t offset) {
  return CORBA::AlignedObjRef(obj, _PD_repoId, 46, offset);
}

inline void
PortableServer::ServantLocator::_marshalObjRef(PortableServer::ServantLocator_ptr obj, NetBufferedStream& s) {
  CORBA::MarshalObjRef(obj, _PD_repoId, 46, s);
}

inline void
PortableServer::ServantLocator::_marshalObjRef(PortableServer::ServantLocator_ptr obj, MemBufferedStream& s) {
  CORBA::MarshalObjRef(obj, _PD_repoId, 46, s);
}


#endif  // __OMNI_POA_OPERATORS_H__
