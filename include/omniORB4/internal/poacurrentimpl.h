// -*- Mode: C++; -*-
//                            Package   : omniORB
// poacurrentimpl.h           Created on: 2001/06/01
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
//    Internal implementation of PortableServer::Current
//

/*
  $Log$
  Revision 1.1.2.4  2001/08/17 17:12:34  sll
  Modularise ORB configuration parameters.

  Revision 1.1.2.3  2001/08/01 10:08:20  dpg1
  Main thread policy.

  Revision 1.1.2.2  2001/06/11 11:38:17  dpg1
  Failed to initialise refcount in Current object.

  Revision 1.1.2.1  2001/06/07 16:24:09  dpg1
  PortableServer::Current support.

*/

#include <omnithread.h>
#include <omniORB4/callDescriptor.h>
#include <omniCurrent.h>


OMNI_NAMESPACE_BEGIN(omni)


class omniOrbPOACurrent : public PortableServer::Current
{
public:
  omniOrbPOACurrent(CORBA::Boolean is_nil = 0)
    : OMNIORB_BASE_CTOR(PortableServer::)Current(is_nil), pd_refCount(1)
  {}
  virtual ~omniOrbPOACurrent();

  PortableServer::POA_ptr    get_POA();
  PortableServer::ObjectId*  get_object_id();
  CORBA::Object_ptr          get_reference();
  PortableServer::Servant    get_servant();

  static PortableServer::Current_ptr theCurrent();
  // Returns a reference to the POA Current, initialising it if
  // necessary.
  //  This function is thread-safe.
  //  Does not throw any exceptions.

  static omniObjRef*             real_get_reference(omniCallDescriptor*);
  static PortableServer::Servant real_get_servant  (omniCallDescriptor*);
  // Static functions to do the work of get_reference() and
  // get_servant() given a call descriptor. Used by POA functions
  // which don't have access to an omniOrbPOACurrent instance.

  ////////////////////////////
  // Override CORBA::Object //
  ////////////////////////////
  virtual void* _ptrToObjRef(const char* repoId);
  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();

private:
  int pd_refCount;
};


class poaCurrentStackInsert {
public:
  inline poaCurrentStackInsert(omniCallDescriptor* desc)
    : pd_current(0)
  {
    if (desc && _OMNI_NS(orbParameters)::supportCurrent) {
      omni_thread* self = omni_thread::self();
      if (self)
	pd_dummy = 0;
      else {
	pd_dummy = 1;
	self     = omni_thread::create_dummy();
      }
      pd_current = omniCurrent::get(self);
      pd_current->pushCallDescriptor(desc);
    }
  }
  inline ~poaCurrentStackInsert()
  {
    if (pd_current) {
      pd_current->popCallDescriptor();
      if (pd_dummy)
	omni_thread::release_dummy();
    }
  }
private:
  omniCurrent* pd_current;
  int          pd_dummy;
};

OMNI_NAMESPACE_END(omni)
