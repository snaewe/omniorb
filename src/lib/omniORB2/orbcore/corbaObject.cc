// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaObject.cc             Created on: 13/5/96
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
//      Implementation of the Object interface
//      
 
/*
  $Log$
  Revision 1.17  1999/05/25 17:12:22  sll
  Added check for invalid parameter in static member functions.

  Revision 1.16  1999/03/11 16:25:52  djr
  Updated copyright notice

  Revision 1.15  1999/01/07 15:23:42  djr
  Moved CORBA::Object::_get_interface() to ir.cc in dynamic library.

  Revision 1.14  1998/08/26 11:05:05  sll
  Test for NEED_DUMMY_RETURN in _get_interface and _get_implementation.

  Revision 1.13  1998/08/14 13:44:30  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.12  1998/04/07 19:50:22  sll
  Replace cerr with omniORB::log.

  Revision 1.11  1998/03/25 14:20:51  sll
  Temporary work-around for egcs compiler.

  Revision 1.10  1998/02/27 13:58:55  sll
  _is_equivalent() now returns the correct answer when a proxy object
  is tested against its colocated object implmentation. This situation will
  only occur if the proxy object is created before the object implementation.

  Revision 1.9  1997/12/09 17:26:32  sll
  Updated _non_existent() to use the system exception handlers.

// Revision 1.8  1997/05/06  15:09:45  sll
// Public release.
//
  */

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <objectManager.h>


CORBA::Object CORBA::Object::CORBA_Object_nil;


CORBA::
Object::Object()
{
  pd_obj = 0;
  pd_magic = CORBA::Object::PR_magic;
  return;
}


CORBA::Object::~Object() {
  pd_magic = 0;
}


CORBA::Object_ptr
CORBA::
Object::_nil()
{
  return (&CORBA_Object_nil);
}


CORBA::Boolean
CORBA::
Object::NP_is_nil() const
{ 
  return (pd_obj)?0:1; 
}


CORBA::Object_ptr 
CORBA::
Object::_duplicate(CORBA::Object_ptr obj)
{
  if (!PR_is_valid(obj))
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
  if (!CORBA::is_nil(obj)) {
    omni::objectDuplicate(obj->pd_obj);
  }
  return obj;
}

void
CORBA::
Object::NP_release()
{ 
  if( pd_obj )  omni::objectRelease(pd_obj);
}

void
CORBA::
Object::PR_setobj(omniObject *obj)
{
  pd_obj = obj;
  return;
}

omniObject *
CORBA::
Object::PR_getobj()
{ 
  return pd_obj;
}

CORBA::Boolean
CORBA::
Object::_is_a(const char *repoId)
{
  if (!PR_is_valid(this))
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if (!repoId)
    return 0;

  if (NP_is_nil()) {
    if (strcmp(repoId,"") == 0)
      return 1;
    else
      return 0;
  }
  else {
    return PR_getobj()->_real_is_a(repoId);
  }
}

CORBA::Boolean
CORBA::
Object::_is_equivalent(CORBA::Object_ptr other_object)
{
  if ( !PR_is_valid(this) || !PR_is_valid(other_object) ) 
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if (CORBA::is_nil(other_object)) {
    if (NP_is_nil())
      return 1;
    else
      return 0;
  }
  else {
    omniObject * objptr = PR_getobj();
    if (!objptr) return 0;
    omniObject * other_objptr = other_object->PR_getobj();
    omniRopeAndKey rak, other_rak;
    objptr->getRopeAndKey(rak);
    other_objptr->getRopeAndKey(other_rak);
    if (rak.keysize() != other_rak.keysize() ||
	memcmp((void*)rak.key(),(void*)other_rak.key(),
	       rak.keysize()) != 0)
      {
	return 0; // object keys do not match
      }
    if (!objptr->is_proxy()) {
      // this is a local object
      if (!other_objptr->is_proxy()) {
	return 1;  // other_object is also local
      }
      else {
	// other_object is a proxy.
	// Have to check if the proxy actually points back to this object
	// via the loop back connection
	if (objptr->_objectManager()->defaultLoopBack() == other_rak.rope())
	  return 1;
	else
	  return 0;
      }
    }
    else {
      // this is a proxy object
      if (!other_objptr->is_proxy()) {
	// other_object is local.
	// Have to check if this proxy actually points back to the local
	// object via the loop back connection
	if (other_objptr->_objectManager()->defaultLoopBack() == rak.rope())
	  return 1;
	else
	  return 0;
      }
      else {
	// both are proxy objects, check whether they go back to the same
	// address space. Note: object keys are not globally unique.
	if (rak.rope() == other_rak.rope())
	  return 1;
	else
	  return 0;
      }
    }
  }
}

CORBA::ULong
CORBA::
Object::_hash(CORBA::ULong maximum)
{
  if (NP_is_nil() || maximum == 0) {
    return 0;
  }
  omniObject * objptr = PR_getobj();
  omniRopeAndKey rak;
  objptr->getRopeAndKey(rak);

  size_t s = rak.keysize();
  char *k = (char *)rak.key();
  CORBA::ULong v = 0;

  unsigned int i;
  for (i = 0; i+4 < s; i+=4) {
    v += (((CORBA::ULong)k[i] << 24) +
	 ((CORBA::ULong)k[i+1] << 16) +
	 ((CORBA::ULong)k[i+2] << 8) +
	 ((CORBA::ULong)k[i+3]));
  }
  CORBA::ULong v2 = 0;
  while (i < s) {
    v2 += (v2 << 8) + (CORBA::ULong)k[i];
    i++;
  }
  v += v2;
  return (v % maximum);
}

CORBA::Boolean
CORBA::
Object::_non_existent()
{
  if ( !PR_is_valid(this) ) 
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if (NP_is_nil()) {
    return 1;
  }
  
  omniObject * objptr = PR_getobj();

  if (!objptr->is_proxy()) {
    return 0;
  }

  CORBA::ULong   _retries = 0;
#ifndef EGCS_WORKAROUND
NONEXIST_again:
#else
while(1) {
#endif
  omniRopeAndKey _r;
  CORBA::Boolean _fwd = objptr->getRopeAndKey(_r);
  CORBA::Boolean _reuse = 0;
  CORBA::Boolean _result;
  try {
    GIOP_C _c(_r.rope());
    CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(_r.keysize(),14);
    _c.InitialiseRequest(_r.key(),
			 _r.keysize(),
			 (char *)"_non_existent",14,_msgsize,0);
    switch (_c.ReceiveReply())
      {
      case GIOP::NO_EXCEPTION:
	{
	  _result <<= _c;
	  _c.RequestCompleted();
	  return _result;
	  break;
	}
      case GIOP::USER_EXCEPTION:
	{
	  _c.RequestCompleted(1);
	  throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
	  break;
	}
      case GIOP::SYSTEM_EXCEPTION:
	{
	  _c.RequestCompleted(1);
	  throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
	}
      case GIOP::LOCATION_FORWARD:
	{
	  {
	    CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_c);
	    _c.RequestCompleted();
	    if (CORBA::is_nil(obj)) {
	      if (omniORB::traceLevel > 10) {
		omniORB::log << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference.\n";
		omniORB::log.flush();
	      }
	      throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
	    }
	    omniRopeAndKey _0RL_r;
	    obj->PR_getobj()->getRopeAndKey(_0RL_r);
	    objptr->setRopeAndKey(_0RL_r);
	  }
	  if (omniORB::traceLevel > 10) {
	    omniORB::log << "GIOP::LOCATION_FORWARD: retry request.\n";
	    omniORB::log.flush();
	  }
	  break;
	}
      }
  }
  catch (const CORBA::OBJECT_NOT_EXIST&) {
    return 1;
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_reuse || _fwd) {
      if (_fwd)
        objptr->resetRopeAndKey();
      CORBA::TRANSIENT ex2(ex.minor(),ex.completed());
      if (!_omni_callTransientExceptionHandler(objptr,_retries++,ex2))
	throw ex2;
    }
    else {
      if (!_omni_callCommFailureExceptionHandler(objptr,_retries++,ex))
	throw;
    }
  }
  catch (const CORBA::TRANSIENT& ex) {
    if (!_omni_callTransientExceptionHandler(objptr,_retries++,ex))
      throw;
  }
  catch (const CORBA::SystemException& ex) {
    if (!_omni_callSystemExceptionHandler(objptr,_retries++,ex))
      throw;
  }
#ifndef EGCS_WORKAROUND
  goto NONEXIST_again;
#else
}
#endif
#ifdef NEED_DUMMY_RETURN
  {
    // never reach here! Dummy return to keep some compilers happy.
    CORBA::Boolean _result = 0;
    return _result;
  }
#endif
}


CORBA::ImplementationDef_ptr
CORBA::
Object::_get_implementation()
{
  // XXX not implemented yet
  throw NO_IMPLEMENT(0, COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


size_t
CORBA::
Object::NP_alignedSize(CORBA::Object_ptr obj,size_t initialoffset)
{
  if (CORBA::is_nil(obj)) {
    return CORBA::AlignedObjRef(obj,0,0,initialoffset);
  }
  else {
    const char *repoId = obj->PR_getobj()->NP_IRRepositoryId();
    return CORBA::AlignedObjRef(obj,repoId,strlen(repoId)+1,initialoffset);
  }
}

void
CORBA::
Object::marshalObjRef(CORBA::Object_ptr obj,NetBufferedStream &s)
{
  if (CORBA::is_nil(obj)) {
    CORBA::MarshalObjRef(obj,0,0,s);
  }
  else {
    const char *repoId = obj->PR_getobj()->NP_IRRepositoryId();
    CORBA::MarshalObjRef(obj,repoId,strlen(repoId)+1,s);
  }
}

CORBA::Object_ptr
CORBA::
Object::unmarshalObjRef(NetBufferedStream &s)
{
  CORBA::Object_ptr _obj = CORBA::UnMarshalObjRef(0,s);
  return _obj;
}

void
CORBA::
Object::marshalObjRef(CORBA::Object_ptr obj,MemBufferedStream &s)
{
  if (CORBA::is_nil(obj)) {
    CORBA::MarshalObjRef(obj,0,0,s);
  }
  else {
    const char *repoId = obj->PR_getobj()->NP_IRRepositoryId();
    CORBA::MarshalObjRef(obj,repoId,strlen(repoId)+1,s);
  }
}

CORBA::Object_ptr
CORBA::
Object::unmarshalObjRef(MemBufferedStream &s)
{
  CORBA::Object_ptr _obj = CORBA::UnMarshalObjRef(0,s);
  return _obj;
}

CORBA::Object_ptr
CORBA::
Object_Helper::_nil() 
{
  return CORBA::Object::_nil();
}

CORBA::Boolean
CORBA::
Object_Helper::is_nil(CORBA::Object_ptr obj)
{
  return CORBA::is_nil(obj);
}

void
CORBA::
Object_Helper::release(CORBA::Object_ptr obj)
{
  CORBA::release(obj);
}

void
CORBA::
Object_Helper::duplicate(CORBA::Object_ptr obj)
{
  CORBA::Object::_duplicate(obj);
}

size_t
CORBA::
Object_Helper::NP_alignedSize(CORBA::Object_ptr obj,size_t initialoffset)
{
  return CORBA::Object::NP_alignedSize(obj,initialoffset);
}

void
CORBA::
Object_Helper::marshalObjRef(CORBA::Object_ptr obj,NetBufferedStream &s)
{
  CORBA::Object::marshalObjRef(obj,s);
}

CORBA::Object_ptr
CORBA::
Object_Helper::unmarshalObjRef(NetBufferedStream &s)
{
  return CORBA::Object::unmarshalObjRef(s);
}

void
CORBA::
Object_Helper::marshalObjRef(CORBA::Object_ptr obj,MemBufferedStream &s)
{
  CORBA::Object::marshalObjRef(obj,s);
}

CORBA::Object_ptr
CORBA::
Object_Helper::unmarshalObjRef(MemBufferedStream &s)
{
  return CORBA::Object::unmarshalObjRef(s);
}

CORBA::
Object_var::Object_var(const CORBA::Object_member& p) 
{
  if (!CORBA::is_nil(p._ptr)) {
    pd_objref = CORBA::Object::_duplicate(p._ptr);
  }
  else
    pd_objref = CORBA::Object::_nil();
}

CORBA::Object_var&
CORBA::
Object_var::operator= (const CORBA::Object_member& p) 
{
  if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
  if (!CORBA::is_nil(p._ptr)) {
    pd_objref = CORBA::Object::_duplicate(p._ptr);
  }
  else
    pd_objref = CORBA::Object::_nil();
  return *this;
}
