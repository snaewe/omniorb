// -*- Mode: C++; -*-
//                            Package   : omniORB
// policy.cc                  Created on: 30/7/2001
//                            Author    : Sai-Lai Lo (sll)
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
//
 
/*
  $Log$
  Revision 1.1.4.3  2004/07/23 10:29:58  dgrisby
  Completely new, much simpler Any implementation.

  Revision 1.1.4.2  2004/04/02 13:26:24  dgrisby
  Start refactoring TypeCode to support value TypeCodes, start of
  abstract interfaces support.

  Revision 1.1.4.1  2003/03/23 21:02:46  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.5  2003/02/17 02:03:08  dgrisby
  vxWorks port. (Thanks Michael Sturm / Acterna Eningen GmbH).

  Revision 1.1.2.4  2001/11/08 16:33:50  dpg1
  Local servant POA shortcut policy.

  Revision 1.1.2.3  2001/08/22 13:29:47  dpg1
  Re-entrant Any marshalling.

  Revision 1.1.2.2  2001/08/17 15:00:47  dpg1
  Fixes for pre-historic compilers.

  Revision 1.1.2.1  2001/07/31 16:04:06  sll
  Added ORB::create_policy() and associated types and operators.

*/

#include <omniORB4/CORBA.h>
#include <exceptiondefs.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////// ORB::create_policy //////////////////////
//////////////////////////////////////////////////////////////////////

#define CASE_CPFN_POA(id,policy) \
  case id: \
    { \
      try { \
	PortableServer::policy##Value v; \
	if (!(value >>= v)) throw CORBA::PolicyError(CORBA::BAD_POLICY_TYPE); \
	return new PortableServer::policy(v); \
      } \
      catch(CORBA::PolicyError& ex) { \
	throw; \
      } \
      catch(...) { \
	throw CORBA::PolicyError(CORBA::BAD_POLICY_TYPE); \
      } \
      break; \
    }


#define CASE_CPFN_BIDIR(id,policy) \
  case id: \
    { \
      try { \
	CORBA::UShort v; \
        if (!(value >>= v) || (v != 0 && v != 1)) \
	  throw CORBA::PolicyError(CORBA::BAD_POLICY_TYPE); \
	return new BiDirPolicy::policy(v); \
      } \
      catch(CORBA::PolicyError& ex) { \
	throw; \
      } \
      catch(...) { \
	throw CORBA::PolicyError(CORBA::BAD_POLICY_TYPE); \
      } \
      break; \
    }

#define CASE_CPFN_OMNI(id,policy) \
  case id: \
    { \
      try { \
	omniPolicy::policy##Value v; \
        if (!(value >>= v)) throw CORBA::PolicyError(CORBA::BAD_POLICY_TYPE); \
	return new omniPolicy::policy(v); \
      } \
      catch(CORBA::PolicyError& ex) { \
	throw; \
      } \
      catch(...) { \
	throw CORBA::PolicyError(CORBA::BAD_POLICY_TYPE); \
      } \
      break; \
    }


CORBA::Policy_ptr
CORBA::
ORB::create_policy(CORBA::PolicyType t, const CORBA::Any& value) {

  switch (t) {

  // POA policies
  CASE_CPFN_POA(/*THREAD_POLICY_ID*/              16, ThreadPolicy)
  CASE_CPFN_POA(/*LIFESPAN_POLICY_ID*/            17, LifespanPolicy)
  CASE_CPFN_POA(/*ID_UNIQUENESS_POLICY_ID*/       18, IdUniquenessPolicy)
  CASE_CPFN_POA(/*ID_ASSIGNMENT_POLICY_ID*/       19, IdAssignmentPolicy)
  CASE_CPFN_POA(/*IMPLICIT_ACTIVATION_POLICY_ID*/ 20, ImplicitActivationPolicy)
  CASE_CPFN_POA(/*SERVANT_RETENTION_POLICY_ID*/   21, ServantRetentionPolicy)
  CASE_CPFN_POA(/*REQUEST_PROCESSING_POLICY_ID*/  22, RequestProcessingPolicy)

  // Bidirectional policy
  CASE_CPFN_BIDIR(/*BIDIRECTIONAL_POLICY_TYPE*/   37, BidirectionalPolicy)

  // omniORB specific policies
  CASE_CPFN_OMNI(/*LOCAL_SHORTCUT_POLICY_TYPE*/0x41545401, LocalShortcutPolicy)

  // Anything else we do not know
  default:
    throw CORBA::PolicyError(CORBA::BAD_POLICY);
  }
}

//////////////////////////////////////////////////////////////////////
///////////////// CORBA::PolicyError user exception //////////////////
//////////////////////////////////////////////////////////////////////

#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the variables external linkage otherwise. Its a bug.
namespace CORBA {

_init_in_def_( const PolicyErrorCode BAD_POLICY = 0; )
_init_in_def_( const PolicyErrorCode UNSUPPORTED_POLICY = 1; )
_init_in_def_( const PolicyErrorCode BAD_POLICY_TYPE = 2; )
_init_in_def_( const PolicyErrorCode BAD_POLICY_VALUE = 3; )
_init_in_def_( const PolicyErrorCode UNSUPPORTED_POLICY_VALUE = 4; )

}
#elif defined(__vxWorks__)
//?? Is this really necessary?
_init_in_decl_( const PolicyErrorCode CORBA::BAD_POLICY = 0; )
_init_in_decl_( const PolicyErrorCode CORBA::UNSUPPORTED_POLICY = 1; )
_init_in_decl_( const PolicyErrorCode CORBA::BAD_POLICY_TYPE = 2; )
_init_in_decl_( const PolicyErrorCode CORBA::BAD_POLICY_VALUE = 3; )
_init_in_decl_( const PolicyErrorCode CORBA::UNSUPPORTED_POLICY_VALUE = 4; )
#else
_init_in_def_( const PolicyErrorCode CORBA::BAD_POLICY = 0; )
_init_in_def_( const PolicyErrorCode CORBA::UNSUPPORTED_POLICY = 1; )
_init_in_def_( const PolicyErrorCode CORBA::BAD_POLICY_TYPE = 2; )
_init_in_def_( const PolicyErrorCode CORBA::BAD_POLICY_VALUE = 3; )
_init_in_def_( const PolicyErrorCode CORBA::UNSUPPORTED_POLICY_VALUE = 4; )
#endif


OMNIORB_DEFINE_USER_EX_COMMON_FNS(CORBA, PolicyError,
				  "IDL:omg.org/CORBA/PolicyError:1.0");


CORBA::PolicyError::PolicyError(const CORBA::PolicyError& _s) : CORBA::UserException(_s)
{
  reason = _s.reason;

}

CORBA::PolicyError::PolicyError(PolicyErrorCode _reason)
{
  pd_insertToAnyFn    = CORBA::PolicyError::insertToAnyFn;
  pd_insertToAnyFnNCP = CORBA::PolicyError::insertToAnyFnNCP;
  reason = _reason;

}

CORBA::PolicyError& CORBA::PolicyError::operator=(const CORBA::PolicyError& _s)
{
  ((CORBA::UserException*) this)->operator=(_s);
  reason = _s.reason;

  return *this;
}

void
CORBA::
PolicyError::operator>>=(cdrStream& _n) const {
  reason >>= _n;
}

void
CORBA::
PolicyError::operator<<=(cdrStream& _n) {
  (PolicyErrorCode&)reason <<= _n;
}


// The following code are generated by omniidl.


static CORBA::TypeCode::_Tracker _0RL_tcTrack(__FILE__);

static CORBA::TypeCode_ptr _0RL_tc_CORBA_mPolicyErrorCode = CORBA::TypeCode::PR_alias_tc("IDL:omg.org/CORBA/PolicyErrorCode:1.0", "PolicyErrorCode", CORBA::TypeCode::PR_short_tc(), &_0RL_tcTrack);


#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.
namespace CORBA { 
  const CORBA::TypeCode_ptr _tc_PolicyErrorCode = _0RL_tc_CORBA_mPolicyErrorCode;
} 
#else
const CORBA::TypeCode_ptr CORBA::_tc_PolicyErrorCode = _0RL_tc_CORBA_mPolicyErrorCode;
#endif

static CORBA::PR_structMember _0RL_structmember_CORBA_mPolicyError[] = {
  {"reason", _0RL_tc_CORBA_mPolicyErrorCode}
};

static CORBA::TypeCode_ptr _0RL_tc_CORBA_mPolicyError = CORBA::TypeCode::PR_exception_tc("IDL:omg.org/CORBA/PolicyError:1.0", "PolicyError", _0RL_structmember_CORBA_mPolicyError, 1, &_0RL_tcTrack);
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.
namespace CORBA { 
  const CORBA::TypeCode_ptr _tc_PolicyError = _0RL_tc_CORBA_mPolicyError;
} 
#else
const CORBA::TypeCode_ptr CORBA::_tc_PolicyError = _0RL_tc_CORBA_mPolicyError;
#endif



static void _0RL_CORBA_mPolicyError_marshal_fn(cdrStream& _s, void* _v)
{
  const CORBA::PolicyError* _p = (const CORBA::PolicyError*)_v;
  *_p >>= _s;
}
static void _0RL_CORBA_mPolicyError_unmarshal_fn(cdrStream& _s, void*& _v)
{
  CORBA::PolicyError* _p = new CORBA::PolicyError;
  *_p <<= _s;
  _v = _p;
}
static void _0RL_CORBA_mPolicyError_destructor_fn(void* _v)
{
  CORBA::PolicyError* _p = (CORBA::PolicyError*)_v;
  delete _p;
}

void operator<<=(CORBA::Any& _a, const CORBA::PolicyError& _s)
{
  CORBA::PolicyError* _p = new CORBA::PolicyError(_s);
  _a.PR_insert(_0RL_tc_CORBA_mPolicyError,
               _0RL_CORBA_mPolicyError_marshal_fn,
               _0RL_CORBA_mPolicyError_destructor_fn,
               _p);
}
void operator<<=(CORBA::Any& _a, const CORBA::PolicyError* _sp)
{
  _a.PR_insert(_0RL_tc_CORBA_mPolicyError,
               _0RL_CORBA_mPolicyError_marshal_fn,
               _0RL_CORBA_mPolicyError_destructor_fn,
               (CORBA::PolicyError*)_sp);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, const CORBA::PolicyError*& _sp)
{
  void* _v;
  if (_a.PR_extract(_0RL_tc_CORBA_mPolicyError,
                    _0RL_CORBA_mPolicyError_unmarshal_fn,
                    _0RL_CORBA_mPolicyError_marshal_fn,
                    _0RL_CORBA_mPolicyError_destructor_fn,
                    _v)) {
    _sp = (const CORBA::PolicyError*)_v;
    return 1;
  }
  return 0;
}

static void _0RL_insertToAny__cCORBA_mPolicyError(CORBA::Any& _a, const CORBA::Exception& _e) {
  const CORBA::PolicyError & _ex = (const CORBA::PolicyError &) _e;
  operator<<=(_a,_ex);
}

static void _0RL_insertToAnyNCP__cCORBA_mPolicyError (CORBA::Any& _a, const CORBA::Exception* _e) {
  const CORBA::PolicyError* _ex = (const CORBA::PolicyError*) _e;
  operator<<=(_a,_ex);
}

class _0RL_insertToAny_Singleton__cCORBA_mPolicyError {
public:
  _0RL_insertToAny_Singleton__cCORBA_mPolicyError() {
    CORBA::PolicyError::insertToAnyFn = _0RL_insertToAny__cCORBA_mPolicyError;
    CORBA::PolicyError::insertToAnyFnNCP = _0RL_insertToAnyNCP__cCORBA_mPolicyError;
  }
};
static _0RL_insertToAny_Singleton__cCORBA_mPolicyError _0RL_insertToAny_Singleton__cCORBA_mPolicyError_;

