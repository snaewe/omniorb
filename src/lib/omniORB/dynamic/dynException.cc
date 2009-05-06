// -*- Mode: C++; -*-
//                            Package   : omniORB
// dynException.cc            Created on: 10/1998
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2004-2005 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//   Exceptions used in the Dynamic library.
//

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <exceptiondefs.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
////////  Implementation of Any Insertion for Exception //////////////
//////////////////////////////////////////////////////////////////////

void operator<<=(CORBA::Any& a, const CORBA::Exception& ex)
{
  if (ex.insertToAnyFn()) {
    ex.insertToAnyFn()(a,ex);
  }
  else {
    omniORB::logs(1, "Error: function to insert the user exception "
		  "into an Any is not available");
    OMNIORB_THROW(INTERNAL,0,CORBA::COMPLETED_NO);
  }
}

void operator<<=(CORBA::Any& a, const CORBA::Exception* ex)
{
  if (ex->insertToAnyFnNCP()) {
    ex->insertToAnyFnNCP()(a,ex);
  }
  else {
    omniORB::logs(1, "Error: function to insert the user exception "
		  "into an Any is not available");
    OMNIORB_THROW(INTERNAL,0,CORBA::COMPLETED_NO);
  }
}

//////////////////////////////////////////////////////////////////////
////////////// Implementation of standard UserException //////////////
//////////////////////////////////////////////////////////////////////

OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA, WrongTransaction,
		       "IDL:omg.org/CORBA/WrongTransaction:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::ContextList, Bounds,
		       "IDL:omg.org/CORBA/ContextList/Bounds:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::ExceptionList, Bounds,
		       "IDL:omg.org/CORBA/ExceptionList/Bounds:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::NVList, Bounds,
		       "IDL:omg.org/CORBA/NVList/Bounds:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::TypeCode, Bounds,
		       "IDL:omg.org/CORBA/TypeCode/Bounds:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::TypeCode, BadKind,
		       "IDL:omg.org/CORBA/TypeCode/BadKind:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::ORB, InconsistentTypeCode,
		       "IDL:omg.org/CORBA/ORB/InconsistentTypeCode:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(DynamicAny::DynAny, InvalidValue,
		       "IDL:omg.org/DynamicAny/DynAny/InvalidValue:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(DynamicAny::DynAny, TypeMismatch,
		       "IDL:omg.org/DynamicAny/DynAny/TypeMismatch:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(DynamicAny::DynAnyFactory,
				       InconsistentTypeCode,
             "IDL:omg.org/DynamicAny/DynAnyFactory/InconsistentTypeCode:1.0")

//////////////////////////////////////////////////////////////////////
////////////// Implementation of User Exception Any operators ////////
//////////////////////////////////////////////////////////////////////

static CORBA::TypeCode::_Tracker _0RL_tcTrack(__FILE__);

#define USER_EXCEPTION_1(name) \
      USER_EXCEPTION(CORBA,name,name,name,#name)

#define USER_EXCEPTION_2(scope,name) \
      USER_EXCEPTION(CORBA,scope::name,scope##_##name,name,#scope "/" #name)

#define USER_EXCEPTION_MOD_2(module,scope,name) \
      USER_EXCEPTION(module,scope::name,scope##_##name,name,#scope "/" #name)

#define USER_EXCEPTION(module,fqname,_fqname,uqname,repostr) \
static const CORBA::TypeCode_ptr _tc_##module##_##_fqname = CORBA::TypeCode::PR_exception_tc("IDL:omg.org/" #module "/" repostr ":1.0", #uqname, (CORBA::PR_structMember*) 0, 0, &_0RL_tcTrack); \
 \
static void _0RL_##module##_##_fqname##_marshal_fn(cdrStream& _s, void* _v) \
{ \
} \
static void _0RL_##module##_##_fqname##_unmarshal_fn(cdrStream& _s, void*& _v) \
{ \
  module::fqname* _p = new module::fqname; \
  *_p <<= _s; \
  _v = _p; \
} \
static void _0RL_##module##_##_fqname##_destructor_fn(void* _v) \
{ \
  module::fqname* _p = (module::fqname*)_v; \
  delete _p; \
} \
 \
void operator<<=(CORBA::Any& _a, const module::fqname& _s) \
{ \
  module::fqname* _p = new module::fqname(_s); \
  _a.PR_insert(_tc_##module##_##_fqname, \
               _0RL_##module##_##_fqname##_marshal_fn, \
               _0RL_##module##_##_fqname##_destructor_fn, \
               _p); \
} \
void operator<<=(CORBA::Any& _a, const module::fqname* _sp) \
{ \
  _a.PR_insert(_tc_##module##_##_fqname, \
               _0RL_##module##_##_fqname##_marshal_fn, \
               _0RL_##module##_##_fqname##_destructor_fn, \
               (module::fqname*)_sp); \
} \
 \
CORBA::Boolean operator>>=(const CORBA::Any& _a, const module::fqname*& _sp) \
{ \
  void* _v; \
  if (_a.PR_extract(_tc_##module##_##_fqname, \
                    _0RL_##module##_##_fqname##_unmarshal_fn, \
                    _0RL_##module##_##_fqname##_marshal_fn, \
                    _0RL_##module##_##_fqname##_destructor_fn, \
                    _v)) { \
    _sp = (const module::fqname*)_v; \
    return 1; \
  } \
  return 0; \
} \
 \
static \
void \
module##_##_fqname##_insertToAny(CORBA::Any& _a,const CORBA::Exception& _e) \
{ \
  const module::fqname& _ex = (const module::fqname &)_e; \
  operator<<=(_a,_ex); \
} \
 \
static \
void \
module##_##_fqname##_insertToAnyNCP(CORBA::Any& _a,const CORBA::Exception* _e) \
{ \
  const module::fqname* _ex = (const module::fqname *)_e; \
  operator<<=(_a,_ex); \
}


USER_EXCEPTION_1 (WrongTransaction)
USER_EXCEPTION_2 (ContextList,Bounds)
USER_EXCEPTION_2 (ExceptionList,Bounds)
USER_EXCEPTION_2 (NVList,Bounds)
USER_EXCEPTION_2 (TypeCode,Bounds)
USER_EXCEPTION_2 (TypeCode,BadKind)
USER_EXCEPTION_2 (ORB,InconsistentTypeCode)
USER_EXCEPTION_2 (ORB,InvalidName)
USER_EXCEPTION_MOD_2 (PortableServer,POAManager,AdapterInactive)
USER_EXCEPTION_MOD_2 (PortableServer,POA,AdapterAlreadyExists)
USER_EXCEPTION_MOD_2 (PortableServer,POA,AdapterNonExistent)
USER_EXCEPTION_MOD_2 (PortableServer,POA,NoServant)
USER_EXCEPTION_MOD_2 (PortableServer,POA,ObjectAlreadyActive)
USER_EXCEPTION_MOD_2 (PortableServer,POA,ObjectNotActive)
USER_EXCEPTION_MOD_2 (PortableServer,POA,ServantAlreadyActive)
USER_EXCEPTION_MOD_2 (PortableServer,POA,ServantNotActive)
USER_EXCEPTION_MOD_2 (PortableServer,POA,WrongAdapter)
USER_EXCEPTION_MOD_2 (PortableServer,POA,WrongPolicy)
USER_EXCEPTION_MOD_2 (DynamicAny,DynAny,InvalidValue)
USER_EXCEPTION_MOD_2 (DynamicAny,DynAny,TypeMismatch)
USER_EXCEPTION_MOD_2 (DynamicAny,DynAnyFactory,InconsistentTypeCode)

#undef USER_EXCEPTION
#undef USER_EXCEPTION_1
#undef USER_EXCEPTION_2
#undef USER_EXCEPTION_MOD_2


//////////////////////////////////////////////////////////////////////
////////////// Exceptions with members from PortableServer ///////////
//////////////////////////////////////////////////////////////////////

// This ugly code is generated by omniidl then hand edited. Oh dear...

static CORBA::PR_structMember _0RL_structmember_PortableServer_mForwardRequest[] = {
  {"forward_reference", CORBA::TypeCode::PR_Object_tc()}
};

static CORBA::TypeCode_ptr _0RL_tc_PortableServer_mForwardRequest = CORBA::TypeCode::PR_exception_tc("IDL:omg.org/PortableServer/ForwardRequest:1.0", "ForwardRequest", _0RL_structmember_PortableServer_mForwardRequest, 1, &_0RL_tcTrack);
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.
namespace PortableServer { 
  const CORBA::TypeCode_ptr _tc_ForwardRequest = _0RL_tc_PortableServer_mForwardRequest;
} 
#else
const CORBA::TypeCode_ptr PortableServer::_tc_ForwardRequest = _0RL_tc_PortableServer_mForwardRequest;
#endif


static CORBA::PR_structMember _0RL_structmember_PortableServer_mPOA_mInvalidPolicy[] = {
  {"index", CORBA::TypeCode::PR_ushort_tc()}
};

static CORBA::TypeCode_ptr _0RL_tc_PortableServer_mPOA_mInvalidPolicy = CORBA::TypeCode::PR_exception_tc("IDL:omg.org/PortableServer/POA/InvalidPolicy:1.0", "InvalidPolicy", _0RL_structmember_PortableServer_mPOA_mInvalidPolicy, 1, &_0RL_tcTrack);
const CORBA::TypeCode_ptr PortableServer::POA::_tc_InvalidPolicy = _0RL_tc_PortableServer_mPOA_mInvalidPolicy;

static void _0RL_PortableServer_mForwardRequest_marshal_fn(cdrStream& _s, void* _v)
{
  const PortableServer::ForwardRequest* _p = (const PortableServer::ForwardRequest*)_v;
  *_p >>= _s;
}
static void _0RL_PortableServer_mForwardRequest_unmarshal_fn(cdrStream& _s, void*& _v)
{
  PortableServer::ForwardRequest* _p = new PortableServer::ForwardRequest;
  *_p <<= _s;
  _v = _p;
}
static void _0RL_PortableServer_mForwardRequest_destructor_fn(void* _v)
{
  PortableServer::ForwardRequest* _p = (PortableServer::ForwardRequest*)_v;
  delete _p;
}

void operator<<=(CORBA::Any& _a, const PortableServer::ForwardRequest& _s)
{
  PortableServer::ForwardRequest* _p = new PortableServer::ForwardRequest(_s);
  _a.PR_insert(_0RL_tc_PortableServer_mForwardRequest,
               _0RL_PortableServer_mForwardRequest_marshal_fn,
               _0RL_PortableServer_mForwardRequest_destructor_fn,
               _p);
}
void operator<<=(CORBA::Any& _a, const PortableServer::ForwardRequest* _sp)
{
  _a.PR_insert(_0RL_tc_PortableServer_mForwardRequest,
               _0RL_PortableServer_mForwardRequest_marshal_fn,
               _0RL_PortableServer_mForwardRequest_destructor_fn,
               (PortableServer::ForwardRequest*)_sp);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, const PortableServer::ForwardRequest*& _sp)
{
  void* _v;
  if (_a.PR_extract(_0RL_tc_PortableServer_mForwardRequest,
                    _0RL_PortableServer_mForwardRequest_unmarshal_fn,
                    _0RL_PortableServer_mForwardRequest_marshal_fn,
                    _0RL_PortableServer_mForwardRequest_destructor_fn,
                    _v)) {
    _sp = (const PortableServer::ForwardRequest*)_v;
    return 1;
  }
  return 0;
}

static void _0RL_insertToAny__cPortableServer_mForwardRequest(CORBA::Any& _a,const CORBA::Exception& _e) {
  const PortableServer::ForwardRequest & _ex = (const PortableServer::ForwardRequest &) _e;
  operator<<=(_a,_ex);
}

static void _0RL_insertToAnyNCP__cPortableServer_mForwardRequest (CORBA::Any& _a,const CORBA::Exception* _e) {
  const PortableServer::ForwardRequest * _ex = (const PortableServer::ForwardRequest *) _e;
  operator<<=(_a,_ex);
}

class _0RL_insertToAny_Singleton__cPortableServer_mForwardRequest {
public:
  _0RL_insertToAny_Singleton__cPortableServer_mForwardRequest() {
    PortableServer::ForwardRequest::insertToAnyFn = _0RL_insertToAny__cPortableServer_mForwardRequest;
    PortableServer::ForwardRequest::insertToAnyFnNCP = _0RL_insertToAnyNCP__cPortableServer_mForwardRequest;
  }
};
static _0RL_insertToAny_Singleton__cPortableServer_mForwardRequest _0RL_insertToAny_Singleton__cPortableServer_mForwardRequest_;

static void _0RL_PortableServer_mPOA_mInvalidPolicy_marshal_fn(cdrStream& _s, void* _v)
{
  const PortableServer::POA::InvalidPolicy* _p = (const PortableServer::POA::InvalidPolicy*)_v;
  *_p >>= _s;
}
static void _0RL_PortableServer_mPOA_mInvalidPolicy_unmarshal_fn(cdrStream& _s, void*& _v)
{
  PortableServer::POA::InvalidPolicy* _p = new PortableServer::POA::InvalidPolicy;
  *_p <<= _s;
  _v = _p;
}
static void _0RL_PortableServer_mPOA_mInvalidPolicy_destructor_fn(void* _v)
{
  PortableServer::POA::InvalidPolicy* _p = (PortableServer::POA::InvalidPolicy*)_v;
  delete _p;
}

void operator<<=(CORBA::Any& _a, const PortableServer::POA::InvalidPolicy& _s)
{
  PortableServer::POA::InvalidPolicy* _p = new PortableServer::POA::InvalidPolicy(_s);
  _a.PR_insert(_0RL_tc_PortableServer_mPOA_mInvalidPolicy,
               _0RL_PortableServer_mPOA_mInvalidPolicy_marshal_fn,
               _0RL_PortableServer_mPOA_mInvalidPolicy_destructor_fn,
               _p);
}
void operator<<=(CORBA::Any& _a, const PortableServer::POA::InvalidPolicy* _sp)
{
  _a.PR_insert(_0RL_tc_PortableServer_mPOA_mInvalidPolicy,
               _0RL_PortableServer_mPOA_mInvalidPolicy_marshal_fn,
               _0RL_PortableServer_mPOA_mInvalidPolicy_destructor_fn,
               (PortableServer::POA::InvalidPolicy*)_sp);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, const PortableServer::POA::InvalidPolicy*& _sp)
{
  void* _v;
  if (_a.PR_extract(_0RL_tc_PortableServer_mPOA_mInvalidPolicy,
                    _0RL_PortableServer_mPOA_mInvalidPolicy_unmarshal_fn,
                    _0RL_PortableServer_mPOA_mInvalidPolicy_marshal_fn,
                    _0RL_PortableServer_mPOA_mInvalidPolicy_destructor_fn,
                    _v)) {
    _sp = (const PortableServer::POA::InvalidPolicy*)_v;
    return 1;
  }
  return 0;
}

static void _0RL_insertToAny__cPortableServer_mPOA_mInvalidPolicy(CORBA::Any& _a,const CORBA::Exception& _e) {
  const PortableServer::POA::InvalidPolicy & _ex = (const PortableServer::POA::InvalidPolicy &) _e;
  operator<<=(_a,_ex);
}

static void _0RL_insertToAnyNCP__cPortableServer_mPOA_mInvalidPolicy (CORBA::Any& _a,const CORBA::Exception* _e) {
  const PortableServer::POA::InvalidPolicy * _ex = (const PortableServer::POA::InvalidPolicy *) _e;
  operator<<=(_a,_ex);
}

class _0RL_insertToAny_Singleton__cPortableServer_mPOA_mInvalidPolicy {
public:
  _0RL_insertToAny_Singleton__cPortableServer_mPOA_mInvalidPolicy() {
    PortableServer::POA::InvalidPolicy::insertToAnyFn = _0RL_insertToAny__cPortableServer_mPOA_mInvalidPolicy;
    PortableServer::POA::InvalidPolicy::insertToAnyFnNCP = _0RL_insertToAnyNCP__cPortableServer_mPOA_mInvalidPolicy;
  }
};
static _0RL_insertToAny_Singleton__cPortableServer_mPOA_mInvalidPolicy _0RL_insertToAny_Singleton__cPortableServer_mPOA_mInvalidPolicy_;




//////////////////////////////////////////////////////////////////////
////////////// Implementation of System Exception Any operators //////
//////////////////////////////////////////////////////////////////////

static const char* mCompletionStatus[] = { "COMPLETED_YES", 
					   "COMPLETED_NO", 
					   "COMPLETED_MAYBE"};

static CORBA::TypeCode_ptr _tc_CompletionStatus =
CORBA::TypeCode::PR_enum_tc("IDL:omg.org/CORBA/completion_status:1.0",
			    "completion_status", mCompletionStatus, 3,
			    &_0RL_tcTrack);


static CORBA::PR_structMember mSystemException[] = {
  {"minor", CORBA::TypeCode::PR_ulong_tc()},
  {"completed", _tc_CompletionStatus}
};


#define STD_EXCEPTION(name) \
CORBA::TypeCode_ptr CORBA::_tc_##name = CORBA::TypeCode::PR_exception_tc("IDL:omg.org/CORBA/" #name ":1.0", #name, mSystemException, 2, &_0RL_tcTrack); \
 \
static void _0RL_CORBA_##name##_marshal_fn(cdrStream& _s, void* _v) \
{ \
  const CORBA::name* e = (const CORBA::name*)_v; \
  CORBA::ULong m = e->minor(); \
  CORBA::ULong c = (CORBA::ULong)e->completed(); \
  m >>= _s; \
  c >>= _s; \
} \
static void _0RL_CORBA_##name##_unmarshal_fn(cdrStream& _s, void*& _v) \
{ \
  CORBA::ULong m; \
  CORBA::ULong c; \
  m <<= _s; \
  c <<= _s; \
  CORBA::name* e = new CORBA::name(m,(CORBA::CompletionStatus)c); \
  _v = e; \
} \
static void _0RL_CORBA_##name##_destructor_fn(void* _v) \
{ \
  CORBA::name* _p = (CORBA::name*)_v; \
  delete _p; \
} \
 \
void operator<<=(CORBA::Any& _a, const CORBA::name& _s) \
{ \
  CORBA::name* _p = new CORBA::name(_s); \
  _a.PR_insert(CORBA::_tc_##name, \
               _0RL_CORBA_##name##_marshal_fn, \
               _0RL_CORBA_##name##_destructor_fn, \
               _p); \
} \
void operator<<=(CORBA::Any& _a, const CORBA::name* _sp) \
{ \
  _a.PR_insert(CORBA::_tc_##name, \
               _0RL_CORBA_##name##_marshal_fn, \
               _0RL_CORBA_##name##_destructor_fn, \
               (CORBA::name*)_sp); \
} \
 \
CORBA::Boolean operator>>=(const CORBA::Any& _a, CORBA::name*& _sp) \
{ \
  return _a >>= (const CORBA::name*&) _sp; \
} \
CORBA::Boolean operator>>=(const CORBA::Any& _a, const CORBA::name*& _sp) \
{ \
  void* _v; \
  if (_a.PR_extract(CORBA::_tc_##name, \
                    _0RL_CORBA_##name##_unmarshal_fn, \
                    _0RL_CORBA_##name##_marshal_fn, \
                    _0RL_CORBA_##name##_destructor_fn, \
                    _v)) { \
    _sp = (const CORBA::name*)_v; \
    return 1; \
  } \
  return 0; \
} \
 \
static \
void \
name##_insertToAny(CORBA::Any& _a,const CORBA::Exception& _e) \
{ \
  const CORBA::name& _ex = (const CORBA::name&)_e; \
  operator<<=(_a,_ex); \
} \
 \
static \
void \
name##_insertToAnyNCP(CORBA::Any& _a,const CORBA::Exception* _e) \
{ \
  const CORBA::name* _ex = (const CORBA::name*)_e; \
  operator<<=(_a,_ex); \
}

OMNIORB_FOR_EACH_SYS_EXCEPTION(STD_EXCEPTION)
#undef STD_EXCEPTION


//////////////////////////////////////////////////////////////////////
////////////// Once only initializer to set up all insertToAnyFn /////
////////////// and insertToAnyFnNCP static variables             /////
//////////////////////////////////////////////////////////////////////


// XXX This should really be done using the initializer mechanism as in
//     in orbcore.
class dynException_Singleton {
public:

  void attach() {

#define STD_EXCEPTION(name) \
CORBA::name::insertToAnyFn    = name##_insertToAny; \
CORBA::name::insertToAnyFnNCP = name##_insertToAnyNCP;

    OMNIORB_FOR_EACH_SYS_EXCEPTION(STD_EXCEPTION)
#undef STD_EXCEPTION


#define USER_EXCEPTION_1(name) \
    USER_EXCEPTION(CORBA,name,name)

#define USER_EXCEPTION_2(scope,name) \
    USER_EXCEPTION(CORBA,scope::name,scope##_##name)

#define USER_EXCEPTION_MOD_1(module,name) \
    USER_EXCEPTION(module,name,name)

#define USER_EXCEPTION_MOD_2(module,scope,name) \
    USER_EXCEPTION(module,scope::name,scope##_##name)

#define USER_EXCEPTION(module,fqname,_fqname) \
module::fqname::insertToAnyFn    = module##_##_fqname##_insertToAny; \
module::fqname::insertToAnyFnNCP = module##_##_fqname##_insertToAnyNCP

    USER_EXCEPTION_1 (WrongTransaction);
    USER_EXCEPTION_2 (ContextList,Bounds);
    USER_EXCEPTION_2 (ExceptionList,Bounds);
    USER_EXCEPTION_2 (NVList,Bounds);
    USER_EXCEPTION_2 (TypeCode,Bounds);
    USER_EXCEPTION_2 (TypeCode,BadKind);
    USER_EXCEPTION_2 (ORB,InconsistentTypeCode);
    USER_EXCEPTION_2 (ORB,InvalidName);
    USER_EXCEPTION_MOD_2 (PortableServer,POAManager,AdapterInactive);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,AdapterAlreadyExists);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,AdapterNonExistent);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,NoServant);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,ObjectAlreadyActive);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,ObjectNotActive);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,ServantAlreadyActive);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,ServantNotActive);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,WrongAdapter);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,WrongPolicy);
    USER_EXCEPTION_MOD_2 (DynamicAny,DynAny,InvalidValue);
    USER_EXCEPTION_MOD_2 (DynamicAny,DynAny,TypeMismatch);
    USER_EXCEPTION_MOD_2 (DynamicAny,DynAnyFactory,InconsistentTypeCode);

#undef USER_EXCEPTION_1
#undef USER_EXCEPTION_2
#undef USER_EXCEPTION_MOD_1
#undef USER_EXCEPTION_MOD_2
#undef USER_EXCEPTION

      }

  dynException_Singleton() {
    attach();
  }
  ~dynException_Singleton() {}
};

static dynException_Singleton dynException_Singleton_;

