// -*- Mode: C++; -*-
//                            Package   : omniORB
// exception.cc               Created on: 13/5/96
//                            Author    : Sai Lai Lo (sll)
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
//

/*
  $Log$
  Revision 1.9.6.2  1999/09/30 12:25:58  djr
  Minor changes.

  Revision 1.9.6.1  1999/09/22 14:26:48  djr
  Major rewrite of orbcore to support POA.

  Revision 1.9  1999/06/18 20:53:10  sll
  New function _CORBA_bad_param_freebuf().

  Revision 1.8  1999/03/11 16:25:52  djr
  Updated copyright notice

  Revision 1.7  1999/01/07 15:44:03  djr
  Added _CORBA_invoked_nil_pseudo_ref() and
  _CORBA_use_nil_ptr_as_nil_pseudo_objref().

  Revision 1.6  1998/08/14 13:46:37  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.5  1998/04/07 19:33:40  sll
  Replace cerr with omniORB::log.
  Use namespce when available.

  Revision 1.4  1997/12/09 18:06:37  sll
  Added support for system exception handlers.

  Revision 1.3  1997/08/21 22:03:56  sll
  Added system exception TRANSACTION_REQUIRED, TRANSACTION_ROLLEDBACK,
  INVALID_TRANSACTION, WRONG_TRANSACTION.
  INVALID_TRANSACTION, WRONG_TRANSACTION.

// Revision 1.2  1997/05/06  15:12:21  sll
// Public release.
//
  */

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <excepthandler.h>
#include <omniORB3/omniObjRef.h>


#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the variables external linkage otherwise. Its a bug.
namespace omniORB {

CORBA::ULong defaultTransientRetryDelayMaximum = 30;

CORBA::ULong defaultTransientRetryDelayIncrement = 1;
}
#else

CORBA::ULong
omniORB::defaultTransientRetryDelayMaximum = 30;

CORBA::ULong
omniORB::defaultTransientRetryDelayIncrement = 1;

#endif


static CORBA::Boolean
omni_defaultTransientExcHandler(void*, CORBA::ULong n_retries,
				const CORBA::TRANSIENT& ex);
static CORBA::Boolean
omni_defaultCommFailureExcHandler(void*, CORBA::ULong n_retries,
				  const CORBA::COMM_FAILURE& ex);
static CORBA::Boolean
omni_defaultSystemExcHandler(void*, CORBA::ULong n_retries,
			     const CORBA::SystemException& ex);


static omniORB::transientExceptionHandler_t
omni_globalTransientExcHandler = omni_defaultTransientExcHandler;

static omniORB::commFailureExceptionHandler_t
omni_globalCommFailureExcHandler = omni_defaultCommFailureExcHandler;

static omniORB::systemExceptionHandler_t
omni_globalSystemExcHandler = omni_defaultSystemExcHandler;

static void* omni_globalTransientExcHandlerCookie = 0;
static void* omni_globalCommFailureExcHandlerCookie =0;
static void* omni_globalSystemExcHandlerCookie = 0;


omniExHandlers** omniExHandlers::Table = 0;
omni_mutex       omniExHandlers::TableLock;


static CORBA::Boolean
omni_defaultTransientExcHandler(void*,
				CORBA::ULong n_retries,
				const CORBA::TRANSIENT& ex)
{
  if (omniORB::trace(10))
    omniORB::logf("defaultTransientExceptionHandler: retry %dth times.",
		  int(n_retries));

  unsigned long secs = n_retries*omniORB::defaultTransientRetryDelayIncrement;
  if (secs > omniORB::defaultTransientRetryDelayMaximum) {
    secs = omniORB::defaultTransientRetryDelayMaximum;
  }
  if (secs)
    omni_thread::sleep(secs,0);
  return 1;
}


void
omniORB::installTransientExceptionHandler(void* cookie,
			       omniORB::transientExceptionHandler_t fn)
{
  omni_globalTransientExcHandler = fn;
  omni_globalTransientExcHandlerCookie = cookie;
}


void
omniORB::installTransientExceptionHandler(CORBA::Object_ptr obj,
				   void* cookie,
				   omniORB::transientExceptionHandler_t fn)
{
  if (CORBA::is_nil(obj)) 
    return;
  obj->_PR_getobj()->_transientExceptionHandler((void*)fn,cookie);
}


static CORBA::Boolean
omni_defaultCommFailureExcHandler(void*,
				   CORBA::ULong,
				   const CORBA::COMM_FAILURE&)
{
  return 0;
}


void
omniORB::installCommFailureExceptionHandler(void* cookie,
				  omniORB::commFailureExceptionHandler_t fn)
{
  omni_globalCommFailureExcHandler = fn;
  omni_globalCommFailureExcHandlerCookie = cookie;
}


void
omniORB::installCommFailureExceptionHandler(CORBA::Object_ptr obj,
				   void* cookie,
				   omniORB::commFailureExceptionHandler_t fn)
{
  if (CORBA::is_nil(obj)) 
    return;
  obj->_PR_getobj()->_commFailureExceptionHandler((void*)fn,cookie);
}


static CORBA::Boolean
omni_defaultSystemExcHandler(void*,
			     CORBA::ULong,
			     const CORBA::SystemException&)
{	
  return 0;
}


void
omniORB::installSystemExceptionHandler(void* cookie,
				       omniORB::systemExceptionHandler_t fn)
{
 omni_globalSystemExcHandler = fn;
 omni_globalSystemExcHandlerCookie = cookie;
}


void
omniORB::installSystemExceptionHandler(CORBA::Object_ptr obj,
				       void* cookie,
				       omniORB::systemExceptionHandler_t fn)
{
  if (CORBA::is_nil(obj)) 
    return;
  obj->_PR_getobj()->_systemExceptionHandler((void*)fn,cookie);
}


CORBA::Boolean
_omni_callTransientExceptionHandler(omniObjRef* obj,
				    CORBA::ULong nretries,
				    const CORBA::TRANSIENT& ex)
{
  void* cookie = 0;
  void* handler = obj->_transientExceptionHandler(cookie);
  if (handler) {
    return (*(omniORB::transientExceptionHandler_t)handler)(cookie,
							    nretries,
							    ex);
  }
  else {
    return (*omni_globalTransientExcHandler)(cookie,
					     nretries,
					     ex);
  }
}


CORBA::Boolean
_omni_callCommFailureExceptionHandler(omniObjRef* obj,
				      CORBA::ULong nretries,
				      const CORBA::COMM_FAILURE& ex)
{
  void* cookie = 0;
  void* handler = obj->_commFailureExceptionHandler(cookie);
  if (handler) {
    return (*(omniORB::commFailureExceptionHandler_t)handler)(cookie,
							      nretries,
							      ex);
  }
  else {
    return (*omni_globalCommFailureExcHandler)(cookie,
					       nretries,
					       ex);
  }
}


CORBA::Boolean
_omni_callSystemExceptionHandler(omniObjRef* obj,
				 CORBA::ULong nretries,
				 const CORBA::SystemException& ex)
{
  void* cookie = 0;
  void* handler = obj->_systemExceptionHandler(cookie);
  if (handler) {
    return (*(omniORB::systemExceptionHandler_t)handler)(cookie,
							 nretries,
							 ex);
  }
  else {
    return (*omni_globalSystemExcHandler)(cookie,
					  nretries,
					  ex);
  }
}


// This should really be extensible ...
static int exHandlersTableSize = 103;


#ifdef EXHANDLER_HASH
#undef EXHANDLER_HASH
#endif

#define EXHANDLER_HASH(p) ((omni::ptr_arith_t) p % exHandlersTableSize)


omniExHandlers::omniExHandlers() :
  transient_hdr(0), transient_cookie(0),
  commfail_hdr(0), commfail_cookie(0), 
  sysexcpt_hdr(0), sysexcpt_cookie(0),
  objptr(0), next(0)
{
}


omniExHandlers_iterator::omniExHandlers_iterator()
{
  omniExHandlers::TableLock.lock();
}


omniExHandlers_iterator::~omniExHandlers_iterator()
{
  omniExHandlers::TableLock.unlock();
}


omniExHandlers*
omniExHandlers_iterator::find_or_create(omniObjRef* p)
{
  if (omniExHandlers::Table == 0) {
    omniExHandlers::Table = new omniExHandlers* [exHandlersTableSize];
    for( int i = 0; i < exHandlersTableSize; i++ )
      omniExHandlers::Table[i] = 0;
  }
  omniExHandlers* exlist = omniExHandlers::Table[EXHANDLER_HASH(p)];
  while (exlist) {
    if (exlist->objptr == p)
      break;
    exlist = exlist->next;
  }
  if (exlist) {
    return exlist;
  }
  else {
    omniExHandlers* np = new omniExHandlers;
    np->objptr = p;
    omniExHandlers** exlistp = &(omniExHandlers::Table[EXHANDLER_HASH(p)]);
    np->next = *exlistp;
    *exlistp = np;
    return np;
  }
}


omniExHandlers*
omniExHandlers_iterator::find(omniObjRef* p)
{
  if (omniExHandlers::Table == 0) return 0;
  omniExHandlers* exlist = omniExHandlers::Table[EXHANDLER_HASH(p)];
  while (exlist) {
    if (exlist->objptr == p)
      break;
    exlist = exlist->next;
  }
  return exlist;
}


void
omniExHandlers_iterator::remove(omniObjRef* p)
{
  if (omniExHandlers::Table == 0) return;
  omniExHandlers** exlistp = &(omniExHandlers::Table[EXHANDLER_HASH(p)]);
  while (*exlistp) {
    if ((*exlistp)->objptr == p) {
      omniExHandlers* exp = *exlistp;
      *exlistp = exp->next;
      delete exp;
      break;
    }
    else {
      exlistp = &((*exlistp)->next);
    }
  }
}

#undef EXHANDLER_HASH

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Boolean 
_CORBA_use_nil_ptr_as_nil_objref()
{
  omniORB::logs(10, "WARNING -- a nil (0) pointer is wrongly used as a\n"
		" nil object reference.");

  return 1;
}


void
_CORBA_new_operator_return_null()
{
  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
}


void
_CORBA_bound_check_error()
{
  throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
}


void
_CORBA_marshal_error()
{
  throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
}


void
_CORBA_invoked_nil_pseudo_ref()
{
  omniORB::logs(1, "ERROR -- the application attempted to invoke an"
		" operation\n"
		" on a nil pseudo-object reference.");

  throw CORBA::INV_OBJREF(0, CORBA::COMPLETED_NO);
}


CORBA::Boolean
_CORBA_use_nil_ptr_as_nil_pseudo_objref(const char* objType)
{
  if( omniORB::trace(10) )
    omniORB::logf("WARNING -- a nil (0) pointer is wrongly used as a\n"
		  " nil CORBA::%s object reference.\n"
		  " Use CORBA::%s::_nil()", objType, objType);

  return 1;
}


void
_CORBA_invoked_nil_objref()
{
  omniORB::logs(1, "ERROR -- the application attempted to invoke an"
		" operation\n"
		" on a nil reference.");

  throw CORBA::INV_OBJREF(0, CORBA::COMPLETED_NO);
}


void
_CORBA_bad_param_freebuf()
{
  omniORB::logs(1, "ERROR -- an invalid buffer pointer is passed to freebuf\n"
		" of string or object sequence");
}
