// -*- Mode: C++; -*-
//                            Package   : omniORB
// unknownUserExn.cc          Created on: 9/1998
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
//   Implementation of CORBA::UnknownUserException.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/tcDescriptor.h>

OMNI_USING_NAMESPACE(omni)

CORBA::UnknownUserException::UnknownUserException(Any* ex)
  : pd_exception(ex)
{
  if( !ex )
    throw omniORB::fatalException(__FILE__,__LINE__,
       "CORBA::UnknownUserException::UnknownUserException(Any*)");
  pd_insertToAnyFn    = insertToAnyFn; 
  pd_insertToAnyFnNCP = insertToAnyFnNCP;
}


CORBA::UnknownUserException::~UnknownUserException()
{
  delete pd_exception;
}


CORBA::Any&
CORBA::UnknownUserException::exception()
{
  return *pd_exception;
}


void
CORBA::UnknownUserException::_raise() const
{
  throw *this;
}


CORBA::UnknownUserException*
CORBA::UnknownUserException::_downcast(Exception* e)
{
  return (UnknownUserException*) _NP_is_a(e,
			 "Exception/UserException/UnknownUserException");
}


const CORBA::UnknownUserException*
CORBA::UnknownUserException::_downcast(const Exception* e)
{
  return (const UnknownUserException*) _NP_is_a(e,
		       "Exception/UserException/UnknownUserException");
}


CORBA::Exception*
CORBA::UnknownUserException::_NP_duplicate() const
{
  // pd_exception is guarenteed not null
  Any* ex = new Any(*pd_exception);
  Exception* e = new UnknownUserException(ex);
  return e;
}


const char*
CORBA::UnknownUserException::_NP_typeId() const
{
  return "Exception/UserException/UnknownUserException";
}


const char*
CORBA::UnknownUserException::_NP_repoId(int* size) const
{
  *size = sizeof("IDL:omg.org/CORBA/UnknownUserException:1.0");
  return "IDL:omg.org/CORBA/UnknownUserException:1.0";
}


void
CORBA::UnknownUserException::_NP_marshal(cdrStream&) const
{
  // I don't think we can be called.
  OMNIORB_ASSERT(0);
}

static CORBA::TypeCode::_Tracker _tcTrack(__FILE__);

static 
CORBA::PR_structMember mUnknownUserException[] = {
  {"exception", CORBA::TypeCode::PR_any_tc()}
};

static
const CORBA::TypeCode_ptr 
_tc_UnknownUserException = CORBA::TypeCode::PR_exception_tc(
	      "IDL:omg.org/CORBA/UnknownUserException:1.0", 
	      "UnknownUserException", 
	      mUnknownUserException, 1, &_tcTrack);


static
CORBA::Boolean
getMemberDesc_UnknownUserException(const tcStructDesc *_desc,
				   CORBA::ULong _index,
				   tcDescriptor &_newdesc)
{
  switch( _index ) {
  case 0:
    _0RL_buildDesc_cany(_newdesc, 
			((CORBA::UnknownUserException*)_desc->opq_struct)->exception());
    return 1;
  default:
    return 0;
  };
}

static
CORBA::ULong
getMemberCount_UnknownUserException(const tcStructDesc *_desc)
{
  return 1;
}

static
void 
buildDesc_mUnknownUserException(tcDescriptor &_desc,
				const CORBA::UnknownUserException& _data)
{
  _desc.p_struct.getMemberDesc = getMemberDesc_UnknownUserException;
  _desc.p_struct.getMemberCount = getMemberCount_UnknownUserException;
  _desc.p_struct.opq_struct = (void *)&_data;
}


static
void
delete_UnknownUserException(void* _data) {
  CORBA::UnknownUserException* _0RL_t = (CORBA::UnknownUserException*) _data;
  delete _0RL_t;
}

static
void
insertUnknownUserExceptionToAny(CORBA::Any& a,
				const CORBA::UnknownUserException&  s)
{
  tcDescriptor _0RL_tcdesc;
  buildDesc_mUnknownUserException(_0RL_tcdesc, s);
  a.PR_packFrom(_tc_UnknownUserException, &_0RL_tcdesc);
}

static
void
insertUnknownUserExceptionToAny(CORBA::Any& a,
				const CORBA::UnknownUserException*  s)
{
  tcDescriptor _0RL_tcdesc;
  buildDesc_mUnknownUserException(_0RL_tcdesc, *s);
  a.PR_packFrom(_tc_UnknownUserException, &_0RL_tcdesc);
  delete (CORBA::UnknownUserException*)s;
}

static
CORBA::Boolean
extractUnknownUserExceptionFromAny(const CORBA::Any& a,
				   CORBA::UnknownUserException*& s,
				   CORBA::Boolean cacheOnly)
{
  if (cacheOnly) {
    s = (CORBA::UnknownUserException *) a.PR_getCachedData();
    if (s) {
      CORBA::TypeCode_var a_tc = a.type();
      if (a_tc->equivalent(_tc_UnknownUserException))
	return 1;
    }
  }
  else {
    tcDescriptor _0RL_tcdesc;
    buildDesc_mUnknownUserException(_0RL_tcdesc,*s);
    if (a.PR_unpackTo(_tc_UnknownUserException, &_0RL_tcdesc)) {
      ((CORBA::Any *)&a)->PR_setCachedData(s,delete_UnknownUserException);
      return 1;
    }
  }
  return 0;
}


void operator<<=(CORBA::Any& _a, const CORBA::UnknownUserException& _s) {
  insertUnknownUserExceptionToAny(_a,_s);
}

void operator<<=(CORBA::Any& _a, const CORBA::UnknownUserException* _sp) {
  insertUnknownUserExceptionToAny(_a,_sp);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, const CORBA::UnknownUserException*& _sp) {
  CORBA::UnknownUserException* s = 0;
  if (extractUnknownUserExceptionFromAny(_a,s,1)) {
    _sp = s;
    return 1;
  }
  else {
    if (s == 0) {
      s = new CORBA::UnknownUserException(new CORBA::Any());
      if (extractUnknownUserExceptionFromAny(_a,s,0)) {
	_sp = s;
	return 1;
      }
      else {
	delete s;
      }
    }
  }
  _sp = 0;
  return 0;
}

static
void
UnknownUserException_insertToAny(CORBA::Any& a,const CORBA::Exception& e)
{
  const CORBA::UnknownUserException & ex = (const CORBA::UnknownUserException &) e;
  insertUnknownUserExceptionToAny(a,ex);
}

static
void
UnknownUserException_insertToAnyNCP(CORBA::Any& a,const CORBA::Exception* e)
{
  const CORBA::UnknownUserException * ex = (const CORBA::UnknownUserException *) e;
  insertUnknownUserExceptionToAny(a,ex);
}

CORBA::Exception::insertExceptionToAny CORBA::UnknownUserException::insertToAnyFn = UnknownUserException_insertToAny;

CORBA::Exception::insertExceptionToAnyNCP CORBA::UnknownUserException::insertToAnyFnNCP = UnknownUserException_insertToAnyNCP;
