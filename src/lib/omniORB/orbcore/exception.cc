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
  Revision 1.10.2.10  2002/08/16 17:47:39  dgrisby
  Documentation, message updates. ORB tweaks to match docs.

  Revision 1.10.2.9  2002/02/25 11:17:12  dpg1
  Use tracedmutexes everywhere.

  Revision 1.10.2.8  2002/01/15 16:38:13  dpg1
  On the road to autoconf. Dependencies refactored, configure.ac
  written. No makefiles yet.

  Revision 1.10.2.7  2001/09/20 15:10:47  sll
  Default Transient Handler now checks for the minor code
  TRANSIENT_FailedOnForwarded and do a retry.

  Revision 1.10.2.6  2001/08/03 17:41:21  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.10.2.5  2001/07/31 16:38:05  sll
  Remove dead code.

  Revision 1.10.2.4  2001/05/11 14:29:23  sll
  Default tranisent handler now do not retry at all.

  Revision 1.10.2.3  2001/04/18 18:18:08  sll
  Big checkin with the brand new internal APIs.

  Revision 1.10.2.2  2000/09/27 17:35:49  sll
  Updated include/omniORB3 to include/omniORB4

  Revision 1.10.2.1  2000/07/17 10:35:53  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.11  2000/07/13 15:25:58  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.9.6.5  2000/06/22 10:40:14  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.9.6.4  2000/06/12 11:16:23  dpg1
  Global exception handlers were returning a zero cookie.

  Revision 1.9.6.3  1999/10/14 16:22:08  djr
  Implemented logging when system exceptions are thrown.

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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <excepthandler.h>
#include <omniORB4/omniObjRef.h>
#include <exceptiondefs.h>
#include <omniORB4/minorCode.h>

OMNI_USING_NAMESPACE(omni)

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
omni_tracedmutex omniExHandlers::TableLock;


static CORBA::Boolean
omni_defaultTransientExcHandler(void*,
				CORBA::ULong n_retries,
				const CORBA::TRANSIENT& ex)
{
  if (ex.minor() == TRANSIENT_FailedOnForwarded) {
    if (omniORB::trace(10)) {
      omniORB::logger log;
      log << "Invocation on a location forwarded object has failed. "
	  << n_retries << " retries.\n";
    }
    unsigned long secs;
    secs = ((n_retries < 30) ? n_retries : 30);
    if (secs) omni_thread::sleep(secs,0);
    return 1;
  }
  return 0;
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
    cookie = omni_globalTransientExcHandlerCookie;
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
    cookie = omni_globalCommFailureExcHandlerCookie;
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
    cookie = omni_globalSystemExcHandlerCookie;
    return (*omni_globalSystemExcHandler)(cookie,
					  nretries,
					  ex);
  }
}


// This should really be extensible ...
static int exHandlersTableSize = 103;


#ifdef EXHANDLER_HASH
#  undef EXHANDLER_HASH
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
  OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
}


void
_CORBA_bound_check_error()
{
  OMNIORB_THROW(BAD_PARAM,BAD_PARAM_IndexOutOfRange,CORBA::COMPLETED_NO);
}


void
_CORBA_marshal_sequence_range_check_error(cdrStream& s)
{
  OMNIORB_THROW(MARSHAL,MARSHAL_SequenceIsTooLong,
		(CORBA::CompletionStatus)s.completion());
}


void
_CORBA_invoked_nil_pseudo_ref()
{
  omniORB::logs(1, "ERROR -- the application attempted to invoke an"
		" operation\n"
		" on a nil pseudo-object reference.");
  OMNIORB_THROW(INV_OBJREF,INV_OBJREF_InvokeOnNilObjRef, CORBA::COMPLETED_NO);
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

  OMNIORB_THROW(INV_OBJREF,INV_OBJREF_InvokeOnNilObjRef, CORBA::COMPLETED_NO);
}


void
_CORBA_bad_param_freebuf()
{
  omniORB::logs(1, "ERROR -- an invalid buffer pointer is passed to freebuf\n"
		" of string or object sequence");
}
