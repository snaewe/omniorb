// -*- Mode: C++; -*-
//                            Package   : omniORB
// callHandle.h               Created on: 16/05/2001
//                            Author    : Duncan Grisby (dpg1)
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
//   Call handle used during remote or in-process operation dispatch.

/*
 $Log$
 Revision 1.1.2.2  2001/06/07 16:24:08  dpg1
 PortableServer::Current support.

 Revision 1.1.2.1  2001/05/29 17:03:48  dpg1
 In process identity.

*/

#ifndef __OMNIORB_CALLHANDLE_H__
#define __OMNIORB_CALLHANDLE_H__

#include <omniORB4/callDescriptor.h>
#include <omniORB4/IOP_S.h>

OMNI_NAMESPACE_BEGIN(omni)
class omniOrbPOA;
OMNI_NAMESPACE_END(omni)

class omniLocalIdentity;

class omniCallHandle {
public:

  inline omniCallHandle(_OMNI_NS(IOP_S)* iop_s)
    : pd_iop_s(iop_s),
      pd_call_desc(0),
      pd_op(iop_s->operation_name()),
      pd_upcall_hook(0),
      pd_poa(0),
      pd_localId(0)
  {}

  inline omniCallHandle(omniCallDescriptor* call_desc)
    : pd_iop_s(0),
      pd_call_desc(call_desc),
      pd_op(call_desc->op()),
      pd_upcall_hook(0),
      pd_poa(0),
      pd_localId(0)
  {}

  inline const char*         operation_name() const { return pd_op; }
  inline _OMNI_NS(IOP_S*)    iop_s()          const { return pd_iop_s; }
  inline omniCallDescriptor* call_desc()      const { return pd_call_desc; }

  void upcall(omniServant* servant, omniCallDescriptor& desc);

  // Class UpcallHook is used to insert extra processing before and
  // after an upcall is made. It is used in the ServantLocator case to
  // call postinvoke() before the results of an upcall are marshalled.
  class UpcallHook {
  public:
    virtual void upcall(omniServant*, omniCallDescriptor&) = 0;
  };

  inline void upcall_hook(UpcallHook* hook) { pd_upcall_hook = hook; }
  inline UpcallHook* upcall_hook() { return pd_upcall_hook; }

  void SkipRequestBody();
  // SkipRequestBody is called if an exception occurs while
  // unmarshalling arguments. It skips any remaining input data so the
  // exception may be returned as soon as possible.

  inline void poa(_OMNI_NS(omniOrbPOA)* poa) { pd_poa = poa; }
  inline void localId(omniLocalIdentity* id) { pd_localId = id; }

private:
  _OMNI_NS(IOP_S)*      pd_iop_s;
  omniCallDescriptor*   pd_call_desc;
  const char*           pd_op;
  UpcallHook*           pd_upcall_hook;
  _OMNI_NS(omniOrbPOA)* pd_poa;
  omniLocalIdentity*    pd_localId;

private:
  // Not implemented
  omniCallHandle(const omniCallHandle&);
  omniCallHandle& operator=(const omniCallHandle&);
};



#endif // __OMNIORB_CALLHANDLE_H__
