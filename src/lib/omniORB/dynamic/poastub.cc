// -*- Mode: C++; -*-
//                            Package   : omniORB
// poastubs.cc                Created on: 30/07/2001
//                            Author    : Sai-Lai Lo
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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

/*
  $Log$
  Revision 1.1.2.1  2001/07/31 16:04:06  sll
  Added ORB::create_policy() and associated types and operators.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/tcDescriptor.h>
#include <poaimpl.h>

OMNI_USING_NAMESPACE(omni)

static const char* _0RL_library_version = omniORB_4_0;

static const char* _0RL_enumMember_PortableServer_mThreadPolicyValue[] = { "ORB_CTRL_MODEL", "SINGLE_THREAD_MODEL" };
static CORBA::TypeCode_ptr _0RL_tc_PortableServer_mThreadPolicyValue = CORBA::TypeCode::PR_enum_tc("IDL:PortableServer/ThreadPolicyValue" PS_VERSION, "ThreadPolicyValue", _0RL_enumMember_PortableServer_mThreadPolicyValue, 2);
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.
namespace PortableServer { 
  const CORBA::TypeCode_ptr _tc_ThreadPolicyValue = _0RL_tc_PortableServer_mThreadPolicyValue;
} 
#else
const CORBA::TypeCode_ptr PortableServer::_tc_ThreadPolicyValue = _0RL_tc_PortableServer_mThreadPolicyValue;
#endif

static const char* _0RL_enumMember_PortableServer_mLifespanPolicyValue[] = { "TRANSIENT", "PERSISTENT" };
static CORBA::TypeCode_ptr _0RL_tc_PortableServer_mLifespanPolicyValue = CORBA::TypeCode::PR_enum_tc("IDL:PortableServer/LifespanPolicyValue" PS_VERSION, "LifespanPolicyValue", _0RL_enumMember_PortableServer_mLifespanPolicyValue, 2);
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.
namespace PortableServer { 
  const CORBA::TypeCode_ptr _tc_LifespanPolicyValue = _0RL_tc_PortableServer_mLifespanPolicyValue;
} 
#else
const CORBA::TypeCode_ptr PortableServer::_tc_LifespanPolicyValue = _0RL_tc_PortableServer_mLifespanPolicyValue;
#endif

static const char* _0RL_enumMember_PortableServer_mIdUniquenessPolicyValue[] = { "UNIQUE_ID", "MULTIPLE_ID" };
static CORBA::TypeCode_ptr _0RL_tc_PortableServer_mIdUniquenessPolicyValue = CORBA::TypeCode::PR_enum_tc("IDL:PortableServer/IdUniquenessPolicyValue" PS_VERSION, "IdUniquenessPolicyValue", _0RL_enumMember_PortableServer_mIdUniquenessPolicyValue, 2);
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.
namespace PortableServer { 
  const CORBA::TypeCode_ptr _tc_IdUniquenessPolicyValue = _0RL_tc_PortableServer_mIdUniquenessPolicyValue;
} 
#else
const CORBA::TypeCode_ptr PortableServer::_tc_IdUniquenessPolicyValue = _0RL_tc_PortableServer_mIdUniquenessPolicyValue;
#endif

static const char* _0RL_enumMember_PortableServer_mIdAssignmentPolicyValue[] = { "USER_ID", "SYSTEM_ID" };
static CORBA::TypeCode_ptr _0RL_tc_PortableServer_mIdAssignmentPolicyValue = CORBA::TypeCode::PR_enum_tc("IDL:PortableServer/IdAssignmentPolicyValue" PS_VERSION, "IdAssignmentPolicyValue", _0RL_enumMember_PortableServer_mIdAssignmentPolicyValue, 2);
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.
namespace PortableServer { 
  const CORBA::TypeCode_ptr _tc_IdAssignmentPolicyValue = _0RL_tc_PortableServer_mIdAssignmentPolicyValue;
} 
#else
const CORBA::TypeCode_ptr PortableServer::_tc_IdAssignmentPolicyValue = _0RL_tc_PortableServer_mIdAssignmentPolicyValue;
#endif

static const char* _0RL_enumMember_PortableServer_mImplicitActivationPolicyValue[] = { "IMPLICIT_ACTIVATION", "NO_IMPLICIT_ACTIVATION" };
static CORBA::TypeCode_ptr _0RL_tc_PortableServer_mImplicitActivationPolicyValue = CORBA::TypeCode::PR_enum_tc("IDL:PortableServer/ImplicitActivationPolicyValue" PS_VERSION, "ImplicitActivationPolicyValue", _0RL_enumMember_PortableServer_mImplicitActivationPolicyValue, 2);
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.
namespace PortableServer { 
  const CORBA::TypeCode_ptr _tc_ImplicitActivationPolicyValue = _0RL_tc_PortableServer_mImplicitActivationPolicyValue;
} 
#else
const CORBA::TypeCode_ptr PortableServer::_tc_ImplicitActivationPolicyValue = _0RL_tc_PortableServer_mImplicitActivationPolicyValue;
#endif

static const char* _0RL_enumMember_PortableServer_mServantRetentionPolicyValue[] = { "RETAIN", "NON_RETAIN" };
static CORBA::TypeCode_ptr _0RL_tc_PortableServer_mServantRetentionPolicyValue = CORBA::TypeCode::PR_enum_tc("IDL:PortableServer/ServantRetentionPolicyValue" PS_VERSION, "ServantRetentionPolicyValue", _0RL_enumMember_PortableServer_mServantRetentionPolicyValue, 2);
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.
namespace PortableServer { 
  const CORBA::TypeCode_ptr _tc_ServantRetentionPolicyValue = _0RL_tc_PortableServer_mServantRetentionPolicyValue;
} 
#else
const CORBA::TypeCode_ptr PortableServer::_tc_ServantRetentionPolicyValue = _0RL_tc_PortableServer_mServantRetentionPolicyValue;
#endif

static const char* _0RL_enumMember_PortableServer_mRequestProcessingPolicyValue[] = { "USE_ACTIVE_OBJECT_MAP_ONLY", "USE_DEFAULT_SERVANT", "USE_SERVANT_MANAGER" };
static CORBA::TypeCode_ptr _0RL_tc_PortableServer_mRequestProcessingPolicyValue = CORBA::TypeCode::PR_enum_tc("IDL:PortableServer/RequestProcessingPolicyValue" PS_VERSION, "RequestProcessingPolicyValue", _0RL_enumMember_PortableServer_mRequestProcessingPolicyValue, 3);
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.
namespace PortableServer { 
  const CORBA::TypeCode_ptr _tc_RequestProcessingPolicyValue = _0RL_tc_PortableServer_mRequestProcessingPolicyValue;
} 
#else
const CORBA::TypeCode_ptr PortableServer::_tc_RequestProcessingPolicyValue = _0RL_tc_PortableServer_mRequestProcessingPolicyValue;
#endif

void _0RL_buildDesc_cPortableServer_mThreadPolicyValue(tcDescriptor& _desc, const PortableServer::ThreadPolicyValue& _data)
{
  _desc.p_enum = (CORBA::ULong*)&_data;
}

void operator<<=(CORBA::Any& _a, PortableServer::ThreadPolicyValue _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mThreadPolicyValue(_0RL_tcd, _s);
  _a.PR_packFrom(_0RL_tc_PortableServer_mThreadPolicyValue, &_0RL_tcd);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, PortableServer::ThreadPolicyValue& _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mThreadPolicyValue(_0RL_tcd, _s);
  return _a.PR_unpackTo(_0RL_tc_PortableServer_mThreadPolicyValue, &_0RL_tcd);
}

void _0RL_buildDesc_cPortableServer_mLifespanPolicyValue(tcDescriptor& _desc, const PortableServer::LifespanPolicyValue& _data)
{
  _desc.p_enum = (CORBA::ULong*)&_data;
}

void operator<<=(CORBA::Any& _a, PortableServer::LifespanPolicyValue _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mLifespanPolicyValue(_0RL_tcd, _s);
  _a.PR_packFrom(_0RL_tc_PortableServer_mLifespanPolicyValue, &_0RL_tcd);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, PortableServer::LifespanPolicyValue& _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mLifespanPolicyValue(_0RL_tcd, _s);
  return _a.PR_unpackTo(_0RL_tc_PortableServer_mLifespanPolicyValue, &_0RL_tcd);
}

void _0RL_buildDesc_cPortableServer_mIdUniquenessPolicyValue(tcDescriptor& _desc, const PortableServer::IdUniquenessPolicyValue& _data)
{
  _desc.p_enum = (CORBA::ULong*)&_data;
}

void operator<<=(CORBA::Any& _a, PortableServer::IdUniquenessPolicyValue _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mIdUniquenessPolicyValue(_0RL_tcd, _s);
  _a.PR_packFrom(_0RL_tc_PortableServer_mIdUniquenessPolicyValue, &_0RL_tcd);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, PortableServer::IdUniquenessPolicyValue& _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mIdUniquenessPolicyValue(_0RL_tcd, _s);
  return _a.PR_unpackTo(_0RL_tc_PortableServer_mIdUniquenessPolicyValue, &_0RL_tcd);
}

void _0RL_buildDesc_cPortableServer_mIdAssignmentPolicyValue(tcDescriptor& _desc, const PortableServer::IdAssignmentPolicyValue& _data)
{
  _desc.p_enum = (CORBA::ULong*)&_data;
}

void operator<<=(CORBA::Any& _a, PortableServer::IdAssignmentPolicyValue _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mIdAssignmentPolicyValue(_0RL_tcd, _s);
  _a.PR_packFrom(_0RL_tc_PortableServer_mIdAssignmentPolicyValue, &_0RL_tcd);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, PortableServer::IdAssignmentPolicyValue& _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mIdAssignmentPolicyValue(_0RL_tcd, _s);
  return _a.PR_unpackTo(_0RL_tc_PortableServer_mIdAssignmentPolicyValue, &_0RL_tcd);
}

void _0RL_buildDesc_cPortableServer_mImplicitActivationPolicyValue(tcDescriptor& _desc, const PortableServer::ImplicitActivationPolicyValue& _data)
{
  _desc.p_enum = (CORBA::ULong*)&_data;
}

void operator<<=(CORBA::Any& _a, PortableServer::ImplicitActivationPolicyValue _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mImplicitActivationPolicyValue(_0RL_tcd, _s);
  _a.PR_packFrom(_0RL_tc_PortableServer_mImplicitActivationPolicyValue, &_0RL_tcd);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, PortableServer::ImplicitActivationPolicyValue& _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mImplicitActivationPolicyValue(_0RL_tcd, _s);
  return _a.PR_unpackTo(_0RL_tc_PortableServer_mImplicitActivationPolicyValue, &_0RL_tcd);
}

void _0RL_buildDesc_cPortableServer_mServantRetentionPolicyValue(tcDescriptor& _desc, const PortableServer::ServantRetentionPolicyValue& _data)
{
  _desc.p_enum = (CORBA::ULong*)&_data;
}

void operator<<=(CORBA::Any& _a, PortableServer::ServantRetentionPolicyValue _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mServantRetentionPolicyValue(_0RL_tcd, _s);
  _a.PR_packFrom(_0RL_tc_PortableServer_mServantRetentionPolicyValue, &_0RL_tcd);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, PortableServer::ServantRetentionPolicyValue& _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mServantRetentionPolicyValue(_0RL_tcd, _s);
  return _a.PR_unpackTo(_0RL_tc_PortableServer_mServantRetentionPolicyValue, &_0RL_tcd);
}

void _0RL_buildDesc_cPortableServer_mRequestProcessingPolicyValue(tcDescriptor& _desc, const PortableServer::RequestProcessingPolicyValue& _data)
{
  _desc.p_enum = (CORBA::ULong*)&_data;
}

void operator<<=(CORBA::Any& _a, PortableServer::RequestProcessingPolicyValue _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mRequestProcessingPolicyValue(_0RL_tcd, _s);
  _a.PR_packFrom(_0RL_tc_PortableServer_mRequestProcessingPolicyValue, &_0RL_tcd);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, PortableServer::RequestProcessingPolicyValue& _s)
{
  tcDescriptor _0RL_tcd;
  _0RL_buildDesc_cPortableServer_mRequestProcessingPolicyValue(_0RL_tcd, _s);
  return _a.PR_unpackTo(_0RL_tc_PortableServer_mRequestProcessingPolicyValue, &_0RL_tcd);
}



