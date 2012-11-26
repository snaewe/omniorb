// -*- Mode: C++; -*-
//                            Package   : omniORB
// ziopStubs.cc               Created on: 2012/10/02
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2012 Apasphere Ltd.
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
// Description:
//    ZIOP stubs

#include <omniORB4/omniZIOP.h>

#include <omniORB4/compressionSK.cc>
#include <omniORB4/ziopSK.cc>


// Policies

#define DEFINE_POLICY_OBJECT(name) \
  \
ZIOP::name::~name() {}  \
  \
CORBA::Policy_ptr  \
ZIOP::name::copy()  \
{  \
  if (_NP_is_nil())  _CORBA_invoked_nil_pseudo_ref();  \
  return new name(pd_value);  \
}  \
  \
void*  \
ZIOP::name::_ptrToObjRef(const char* repoId)  \
{  \
  OMNIORB_ASSERT(repoId);  \
  \
  if (omni::ptrStrMatch(repoId, ZIOP::name::_PD_repoId))  \
    return (ZIOP::name##_ptr) this;  \
  if (omni::ptrStrMatch(repoId, CORBA::Policy::_PD_repoId))  \
    return (CORBA::Policy_ptr) this;  \
  if (omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId))  \
    return (CORBA::Object_ptr) this;  \
  \
  return 0;  \
}  \
  \
ZIOP::name##_ptr  \
ZIOP::name::_duplicate(ZIOP::name##_ptr obj)  \
{  \
  if (!CORBA::is_nil(obj))  obj->_NP_incrRefCount();  \
  \
  return obj;  \
}  \
  \
ZIOP::name##_ptr  \
ZIOP::name::_narrow(CORBA::Object_ptr obj)  \
{  \
  if (CORBA::is_nil(obj))  return _nil();  \
  \
  name##_ptr p = (name##_ptr) obj->_ptrToObjRef(name::_PD_repoId);  \
  \
  if (p)  p->_NP_incrRefCount();  \
  \
  return p ? p : _nil();  \
}  \
  \
ZIOP::name##_ptr  \
ZIOP::name::_nil()  \
{  \
  static name* _the_nil_ptr = 0;  \
  if (!_the_nil_ptr) {  \
    omni::nilRefLock().lock();  \
    if (!_the_nil_ptr) { \
      _the_nil_ptr = new name;  \
      registerNilCorbaObject(_the_nil_ptr); \
    } \
    omni::nilRefLock().unlock();  \
  }  \
  return _the_nil_ptr;  \
}  \
  \
const char* ZIOP::name::_PD_repoId = "IDL:omg.org/ZIOP/" #name ":1.0";


#define DEFINE_POLICY_VALMETHOD(name, valmethod) \
 \
ZIOP::name##Value \
ZIOP::name::valmethod() \
{ \
  return pd_value; \
}


DEFINE_POLICY_OBJECT(CompressionEnablingPolicy)
DEFINE_POLICY_OBJECT(CompressionIdLevelListPolicy)
DEFINE_POLICY_OBJECT(CompressionLowValuePolicy)
DEFINE_POLICY_OBJECT(CompressionMinRatioPolicy)

DEFINE_POLICY_VALMETHOD(CompressionEnablingPolicy, compression_enabled)
DEFINE_POLICY_VALMETHOD(CompressionLowValuePolicy, low_value)
DEFINE_POLICY_VALMETHOD(CompressionMinRatioPolicy, ratio)

ZIOP::CompressionIdLevelListPolicyValue*
ZIOP::CompressionIdLevelListPolicy::compressor_ids()
{
  return new ZIOP::CompressionIdLevelListPolicyValue(pd_value);
}
