// -*- Mode: C++; -*-
//                            Package   : omniORB3
// dynException.cc            Created on: 10/1998
//                            Author    : David Riddoch (djr)
//
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

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB3/tcDescriptor.h>
#include <dynException.h>
#include <exception.h>

//////////////////////////////////////////////////////////////////////
////////  Implementation of Any Insertion for Exception //////////////
//////////////////////////////////////////////////////////////////////

void operator<<=(CORBA::Any& a, const CORBA::Exception& ex)
{
  if (ex.insertToAnyFn()) {
    ex.insertToAnyFn()(a,ex);
  }
  else {
    omniORB::log << "Error: function to insert the user exception into an Any is not available\n";
    omniORB::log.flush();
    OMNIORB_THROW(INTERNAL,0,CORBA::COMPLETED_NO);
  }
}

void operator<<=(CORBA::Any& a, const CORBA::Exception* ex)
{
  if (ex->insertToAnyFnNCP()) {
    ex->insertToAnyFnNCP()(a,ex);
  }
  else {
    omniORB::log << "Error: function to insert the user exception into an Any is not available\n";
    omniORB::log.flush();
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
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::DynAny, Invalid,
		       "IDL:omg.org/CORBA/DynAny/Invalid:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::DynAny, InvalidValue,
		       "IDL:omg.org/CORBA/DynAny/InvalidValue:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::DynAny, TypeMismatch,
		       "IDL:omg.org/CORBA/DynAny/TypeMismatch:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::DynAny, InvalidSeq,
		       "IDL:omg.org/CORBA/DynAny/InvalidSeq:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::ORB, InconsistentTypeCode,
		       "IDL:omg.org/CORBA/ORB/InconsistentTypeCode:1.0")

//////////////////////////////////////////////////////////////////////
////////////// Implementation of User   Exception Any operators //////
//////////////////////////////////////////////////////////////////////

static
CORBA::Boolean 
getMemberDesc_UserException(tcStructDesc *,CORBA::ULong,tcDescriptor &)
{
  return 0;
}

static
CORBA::ULong
getMemberCount_UserException(tcStructDesc *)
{
  return 0;
}

static
void
buildDesc_cUserException(tcDescriptor &_desc, 
			 const CORBA::UserException& _data)
{
  _desc.p_struct.getMemberDesc = getMemberDesc_UserException;
  _desc.p_struct.getMemberCount = getMemberCount_UserException;
  _desc.p_struct.opq_struct = (void *)&_data;
}

static
void
delete_UserException(void* _data) {
  CORBA::UserException* _0RL_t = (CORBA::UserException*) _data;
  delete _0RL_t;
}

static
void
insertUserExceptionToAny(CORBA::Any& a, CORBA::TypeCode_ptr tc,
			 const CORBA::UserException&  s)
{
  tcDescriptor _0RL_tcdesc;
  buildDesc_cUserException(_0RL_tcdesc, s);
  a.PR_packFrom(tc, &_0RL_tcdesc);
}

static
void
insertUserExceptionToAny(CORBA::Any& a, CORBA::TypeCode_ptr tc,
			   const CORBA::UserException*  s)
{
  tcDescriptor _0RL_tcdesc;
  buildDesc_cUserException(_0RL_tcdesc, *s);
  a.PR_packFrom(tc, &_0RL_tcdesc);
  delete (CORBA::UserException*)s;
}

static
CORBA::Boolean
extractUserExceptionFromAny(const CORBA::Any& a, CORBA::TypeCode_ptr tc,
			    CORBA::UserException*& s,
			    CORBA::Boolean cacheOnly)
{
  if (cacheOnly) {
    s = (CORBA::UserException *) a.PR_getCachedData();
    if (s) {
      CORBA::TypeCode_var a_tc = a.type();
      if (a_tc->equivalent(tc))
	return 1;
    }
  }
  else {
    tcDescriptor _0RL_tcdesc;
    buildDesc_cUserException(_0RL_tcdesc,*s);
    if (a.PR_unpackTo(tc, &_0RL_tcdesc)) {
      ((CORBA::Any *)&a)->PR_setCachedData(s,delete_UserException);
      return 1;
    }
  }
  return 0;
}

#define USER_EXCEPTION_1(name) \
      USER_EXCEPTION(name,name,name,#name)

#define USER_EXCEPTION_2(scope,name) \
      USER_EXCEPTION(scope::name,scope##_##name,name,#scope "/" #name)

#define USER_EXCEPTION(fqname,_fqname,uqname,repostr) \
static const CORBA::TypeCode_ptr _tc_##_fqname = CORBA::TypeCode::PR_exception_tc("IDL:omg.org/CORBA/" repostr ":1.0", #uqname, (CORBA::PR_structMember*) 0, 0); \
 \
void operator<<=(CORBA::Any& _a, const CORBA::fqname& _s) { \
  insertUserExceptionToAny(_a,_tc_##_fqname,_s); \
} \
 \
void operator<<=(CORBA::Any& _a, const CORBA::fqname* _sp) { \
  insertUserExceptionToAny(_a,_tc_##_fqname,_sp); \
} \
 \
CORBA::Boolean operator>>=(const CORBA::Any& _a, CORBA::fqname*& _sp) { \
  CORBA::UserException* s = 0; \
  if (extractUserExceptionFromAny(_a,_tc_##_fqname,s,1)) { \
    _sp = (CORBA::fqname *)s; \
    return 1; \
  } \
  else { \
    if (s == 0) { \
      s = new CORBA::fqname; \
      if (extractUserExceptionFromAny(_a,_tc_##_fqname,s,0)) { \
	_sp = (CORBA::fqname *)s; \
	return 1; \
      } \
      else { \
	delete s; \
      } \
    } \
  } \
  _sp = 0; \
  return 0; \
} \
 \
static \
void \
_fqname##_insertToAny(CORBA::Any& a,const CORBA::Exception& e) \
{ \
  const CORBA::fqname & ex = (const CORBA::fqname &) e; \
  insertUserExceptionToAny(a,_tc_##_fqname,ex); \
} \
 \
static \
void \
_fqname##_insertToAnyNCP(CORBA::Any& a,const CORBA::Exception* e) \
{ \
  const CORBA::fqname * ex = (const CORBA::fqname *) e; \
  insertUserExceptionToAny(a,_tc_##_fqname,ex); \
}


USER_EXCEPTION_1 (WrongTransaction)
USER_EXCEPTION_2 (ContextList,Bounds)
USER_EXCEPTION_2 (ExceptionList,Bounds)
USER_EXCEPTION_2 (NVList,Bounds)
USER_EXCEPTION_2 (TypeCode,Bounds)
USER_EXCEPTION_2 (TypeCode,BadKind)
USER_EXCEPTION_2 (DynAny,Invalid)
USER_EXCEPTION_2 (DynAny,InvalidValue)
USER_EXCEPTION_2 (DynAny,TypeMismatch)
USER_EXCEPTION_2 (DynAny,InvalidSeq)
USER_EXCEPTION_2 (ORB,InconsistentTypeCode)
USER_EXCEPTION_2 (ORB,InvalidName)


#undef USER_EXCEPTION
#undef USER_EXCEPTION_1
#undef USER_EXCEPTION_2

//////////////////////////////////////////////////////////////////////
////////////// Implementation of System Exception Any operators //////
//////////////////////////////////////////////////////////////////////

static const char* mCompletionStatus[] = { "COMPLETED_YES", 
					   "COMPLETED_NO", 
					   "COMPLETED_MAYBE"};

static CORBA::TypeCode_ptr _tc_CompletionStatus =
CORBA::TypeCode::PR_enum_tc("IDL:omg.org/CORBA/CompletionStatus:1.0",
			    "CompletionStatus", mCompletionStatus, 3);


static CORBA::PR_structMember mSystemException[] = {
  {"minor", CORBA::TypeCode::PR_ulong_tc()},
  {"completed", _tc_CompletionStatus}
};


struct _SystemException {
  CORBA::ULong            minor;
  CORBA::CompletionStatus completed;
};

static 
void 
buildDesc_mCompletionStatus(tcDescriptor& _desc, 
			    const CORBA::CompletionStatus& _data)
{
  _desc.p_enum = (CORBA::ULong*)&_data;
}

static
CORBA::Boolean 
getMemberDesc_mSystemException(tcStructDesc *_desc,
			       CORBA::ULong _index,
			       tcDescriptor &_newdesc)
{
  switch( _index ) {
  case 0:
    _0RL_buildDesc_cunsigned_plong(_newdesc,
				   ((_SystemException*)
				    _desc->opq_struct)->minor);
    return 1;
  case 1:
    buildDesc_mCompletionStatus(_newdesc,
				((_SystemException*)
				 _desc->opq_struct)->completed);
    return 1;
  default:
    return 0;
  };
}

static
CORBA::ULong
getMemberCount_mSystemException(tcStructDesc *_desc)
{
  return 2;
}

static
void
buildDesc_mSystemException(tcDescriptor &_desc,
			   const _SystemException& _data)
{
  _desc.p_struct.getMemberDesc = getMemberDesc_mSystemException;
  _desc.p_struct.getMemberCount = getMemberCount_mSystemException;
  _desc.p_struct.opq_struct = (void *)&_data;
}

static
void 
delete_mSystemException(void* _data) {
  CORBA::SystemException* _0RL_t = (CORBA::SystemException*) _data;
  delete _0RL_t;
}

static
void
insertSystemExceptionToAny(CORBA::Any& a, CORBA::TypeCode_ptr tc,
			   const CORBA::SystemException&  s)
{
  tcDescriptor _0RL_tcdesc;
  _SystemException ex;
  ex.minor = s.minor();
  ex.completed = s.completed();
  buildDesc_mSystemException(_0RL_tcdesc, ex);
  a.PR_packFrom(tc, &_0RL_tcdesc);
}

static
void
insertSystemExceptionToAny(CORBA::Any& a, CORBA::TypeCode_ptr tc,
			   const CORBA::SystemException*  s)
{
  tcDescriptor _0RL_tcdesc;
  _SystemException ex;
  ex.minor = s->minor();
  ex.completed = s->completed();
  buildDesc_mSystemException(_0RL_tcdesc, ex);
  a.PR_packFrom(tc, &_0RL_tcdesc);
  delete (CORBA::SystemException*)s;
}

static
CORBA::Boolean
extractSystemExceptionFromAny(const CORBA::Any& a, CORBA::TypeCode_ptr tc,
			      CORBA::SystemException*& s,
			      CORBA::Boolean cacheOnly)
{
  if (cacheOnly) {
    s = (CORBA::SystemException *) a.PR_getCachedData();
    if (s) {
      CORBA::TypeCode_var a_tc = a.type();
      if (a_tc->equivalent(tc))
	return 1;
    }
  }
  else {
    tcDescriptor _0RL_tcdesc;
    _SystemException ex;
    buildDesc_mSystemException(_0RL_tcdesc,ex);
    if (a.PR_unpackTo(tc, &_0RL_tcdesc)) {
      s->minor(ex.minor);
      s->completed(ex.completed);
      ((CORBA::Any *)&a)->PR_setCachedData(s,
					   delete_mSystemException);
      return 1;
    }
  }
  return 0;
}


#define STD_EXCEPTION(name) \
static const CORBA::TypeCode_ptr _tc_##name = CORBA::TypeCode::PR_exception_tc("IDL:omg.org/CORBA/" #name ":1.0", #name, mSystemException, 2); \
 \
void operator<<=(CORBA::Any& _a, const CORBA::name & _s) { \
  insertSystemExceptionToAny(_a,_tc_##name,_s); \
}  \
 \
void operator<<=(CORBA::Any& _a, const CORBA::name * _sp) { \
  insertSystemExceptionToAny(_a,_tc_##name,_sp); \
}\
 \
CORBA::Boolean \
operator>>=(const CORBA::Any& _a, CORBA::name *& _sp) \
{ \
  CORBA::SystemException* s = 0; \
  if (extractSystemExceptionFromAny(_a,_tc_##name,s,1)) { \
    _sp = (CORBA::name *)s; \
    return 1; \
  } \
  else { \
    if (s == 0) { \
      s = new CORBA::name; \
      if (extractSystemExceptionFromAny(_a,_tc_##name,s,0)) { \
	_sp = (CORBA::name *)s; \
	return 1; \
      } \
      else { \
	delete s; \
      } \
    } \
  } \
  _sp = 0; \
  return 0; \
} \
 \
static \
void \
name##_insertToAny(CORBA::Any& a,const CORBA::Exception& e) \
{ \
  const CORBA::name & ex = (const CORBA::name &) e; \
  insertSystemExceptionToAny(a,_tc_##name,ex); \
} \
 \
static \
void \
name##_insertToAnyNCP(CORBA::Any& a,const CORBA::Exception* e) \
{ \
  const CORBA::name * ex = (const CORBA::name *) e; \
  insertSystemExceptionToAny(a,_tc_##name,ex); \
}


OMNIORB_FOR_EACH_SYS_EXCEPTION(STD_EXCEPTION)
#undef STD_EXCEPTION


//////////////////////////////////////////////////////////////////////
////////////// isaSystemException                                /////
//////////////////////////////////////////////////////////////////////

CORBA::Boolean
isaSystemException(const CORBA::Any* a)
{
  CORBA::TypeCode_var tc = a->type();

  if (tc->equivalent(_tc_UNKNOWN) ||
      tc->equivalent(_tc_UNKNOWN) ||
      tc->equivalent(_tc_BAD_PARAM) ||
      tc->equivalent(_tc_NO_MEMORY) ||
      tc->equivalent(_tc_IMP_LIMIT) ||
      tc->equivalent(_tc_COMM_FAILURE) ||
      tc->equivalent(_tc_INV_OBJREF) ||
      tc->equivalent(_tc_OBJECT_NOT_EXIST) ||
      tc->equivalent(_tc_NO_PERMISSION) ||
      tc->equivalent(_tc_INTERNAL) ||
      tc->equivalent(_tc_MARSHAL) ||
      tc->equivalent(_tc_INITIALIZE) ||
      tc->equivalent(_tc_NO_IMPLEMENT) ||
      tc->equivalent(_tc_BAD_TYPECODE) ||
      tc->equivalent(_tc_BAD_OPERATION) ||
      tc->equivalent(_tc_NO_RESOURCES) ||
      tc->equivalent(_tc_NO_RESPONSE) ||
      tc->equivalent(_tc_PERSIST_STORE) ||
      tc->equivalent(_tc_BAD_INV_ORDER) ||
      tc->equivalent(_tc_TRANSIENT) ||
      tc->equivalent(_tc_FREE_MEM) ||
      tc->equivalent(_tc_INV_IDENT) ||
      tc->equivalent(_tc_INV_FLAG) ||
      tc->equivalent(_tc_INTF_REPOS) ||
      tc->equivalent(_tc_BAD_CONTEXT) ||
      tc->equivalent(_tc_OBJ_ADAPTER) ||
      tc->equivalent(_tc_DATA_CONVERSION) ||
      tc->equivalent(_tc_TRANSACTION_REQUIRED) ||
      tc->equivalent(_tc_TRANSACTION_ROLLEDBACK) ||
      tc->equivalent(_tc_INVALID_TRANSACTION) ||
      tc->equivalent(_tc_WRONG_TRANSACTION)) {
    return 1;
  }
  else
    return 0;
}

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


#define USER_EXCEPTION_1(name) USER_EXCEPTION(name,name)

#define USER_EXCEPTION_2(scope,name) USER_EXCEPTION(scope::name,scope##_##name)

#define USER_EXCEPTION(fqname,_fqname) \
CORBA::fqname::insertToAnyFn    = _fqname##_insertToAny; \
CORBA::fqname::insertToAnyFnNCP = _fqname##_insertToAnyNCP

    USER_EXCEPTION_1 (WrongTransaction);
    USER_EXCEPTION_2 (ContextList,Bounds);
    USER_EXCEPTION_2 (ExceptionList,Bounds);
    USER_EXCEPTION_2 (NVList,Bounds);
    USER_EXCEPTION_2 (TypeCode,Bounds);
    USER_EXCEPTION_2 (TypeCode,BadKind);
    USER_EXCEPTION_2 (DynAny,Invalid);
    USER_EXCEPTION_2 (DynAny,InvalidValue);
    USER_EXCEPTION_2 (DynAny,TypeMismatch);
    USER_EXCEPTION_2 (DynAny,InvalidSeq);
    USER_EXCEPTION_2 (ORB,InconsistentTypeCode);
    USER_EXCEPTION_2 (ORB,InvalidName);

#undef USER_EXCEPTION_1
#undef USER_EXCEPTION_2
#undef USER_EXCEPTION

      }

  dynException_Singleton() {
    attach();
  }
  ~dynException_Singleton() {}
};

static dynException_Singleton dynException_Singleton_;

