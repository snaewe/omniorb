// -*- Mode: C++; -*-
//                            Package   : omniORB
// valueBase.cc               Created on: 2003/08/20
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2003 Apasphere Ltd.
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
//    ValueBase implementation
//

/*
  $Log$
  Revision 1.1.2.6  2005/01/06 16:39:25  dgrisby
  DynValue and DynValueBox implementations; misc small fixes.

  Revision 1.1.2.5  2004/10/13 17:58:21  dgrisby
  Abstract interfaces support; values support interfaces; value bug fixes.

  Revision 1.1.2.4  2004/07/26 22:56:39  dgrisby
  Support valuetypes in Anys.

  Revision 1.1.2.3  2004/07/04 23:53:37  dgrisby
  More ValueType TypeCode and Any support.

  Revision 1.1.2.2  2003/11/06 11:56:56  dgrisby
  Yet more valuetype. Plain valuetype and abstract valuetype are now working.

  Revision 1.1.2.1  2003/09/26 16:12:54  dgrisby
  Start of valuetype support.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/valueType.h>

//////////////////////////////////////////////////////////////////////
//////////////////////////// ValueBase ///////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::ValueBase*
CORBA::ValueBase::_downcast(CORBA::ValueBase* v)
{
  return (CORBA::ValueBase*)v;
}

void*
CORBA::ValueBase::_ptrToValue(const char* repoId)
{
  if (repoId == _PD_repoId)
    return (void*)this;
  if (omni::strMatch(repoId, _PD_repoId))
    return (void*)this;
  return 0;
}

const char* CORBA::ValueBase::_PD_repoId = "IDL:omg.org/CORBA/ValueBase:1.0";

void
CORBA::ValueBase::_NP_marshal(CORBA::ValueBase* v, cdrStream& s)
{
  omniValueType::marshal(v, _PD_repoId, s);
}

CORBA::ValueBase*
CORBA::ValueBase::_NP_unmarshal(cdrStream& s)
{
  return omniValueType::unmarshal(_PD_repoId, 0, 0, s);
}

CORBA::Boolean
CORBA::ValueBase::_NP_box() const
{
  return 0;
}

CORBA::ValueBase::ValueBase() : _pd_magic(_PR_magic) {}
CORBA::ValueBase::ValueBase(const ValueBase&) : _pd_magic(_PR_magic) {}
CORBA::ValueBase::~ValueBase() {}


//////////////////////////////////////////////////////////////////////
///////////////////// DefaultValueRefCountBase ///////////////////////
//////////////////////////////////////////////////////////////////////

static omni_tracedmutex ref_count_lock;

void
CORBA::DefaultValueRefCountBase::_add_ref()
{
  omni_tracedmutex_lock sync(ref_count_lock);
  _pd_refCount++;
}

void
CORBA::DefaultValueRefCountBase::_remove_ref()
{
  {
    omni_tracedmutex_lock sync(ref_count_lock);
    OMNIORB_ASSERT(_pd_refCount > 0);

    if (--_pd_refCount > 0)
      return;
  }
  delete this;
}

CORBA::ULong
CORBA::DefaultValueRefCountBase::_refcount_value()
{
  omni_tracedmutex_lock sync(ref_count_lock);
  return _pd_refCount;
}

CORBA::DefaultValueRefCountBase::~DefaultValueRefCountBase() {
  OMNIORB_ASSERT(_pd_refCount == 0);
}


//////////////////////////////////////////////////////////////////////
///////////////// PortableServer::ValueRefCountBase //////////////////
//////////////////////////////////////////////////////////////////////


void
PortableServer::ValueRefCountBase::_add_ref()
{
  PortableServer::ServantBase::_add_ref();
}

void
PortableServer::ValueRefCountBase::_remove_ref()
{
  PortableServer::ServantBase::_remove_ref();
}

CORBA::ULong
PortableServer::ValueRefCountBase::_refcount_value()
{
  return PortableServer::ServantBase::_refcount_value();
}

PortableServer::ValueRefCountBase::~ValueRefCountBase() {
}


//////////////////////////////////////////////////////////////////////
///////////////////// ValueFactoryBase ///////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::ValueFactoryBase::_add_ref()
{
  omni_tracedmutex_lock sync(ref_count_lock);
  _pd_refCount++;
}

void
CORBA::ValueFactoryBase::_remove_ref()
{
  {
    omni_tracedmutex_lock sync(ref_count_lock);
    OMNIORB_ASSERT(_pd_refCount > 0);

    if (--_pd_refCount > 0)
      return;
  }
  delete this;
}

CORBA::ValueFactoryBase::ValueFactoryBase() : _pd_refCount(1) {}

CORBA::ValueFactoryBase::~ValueFactoryBase() {
  OMNIORB_ASSERT(_pd_refCount == 0);
}

CORBA::ValueFactory
CORBA::ValueFactoryBase::_downcast(CORBA::ValueFactory vf)
{
  return (ValueFactory)vf;
}

void*
CORBA::ValueFactoryBase::_ptrToFactory(const char* repoId)
{
  if (omni::ptrStrMatch(repoId, CORBA::ValueBase::_PD_repoId))
    return (CORBA::ValueBase*)this;

  return 0;
}
