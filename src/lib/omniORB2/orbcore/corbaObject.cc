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
  Revision 1.19.4.1  1999/09/15 20:18:33  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

  Revision 1.19  1999/06/22 14:57:20  sll
  _is_equivalent() now throws OBJECT_NOT_EXIST instead of BAD_PARAM if
  the parameter is an invalid object reference.

  Revision 1.18  1999/06/18 20:52:56  sll
  Moved Object_var copy ctor and operator= to header.

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

#include <omniORB2/proxyCall.h>
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
  if ( !PR_is_valid(this) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if ( !PR_is_valid(other_object) ) 
    throw CORBA::OBJECT_NOT_EXIST(0,CORBA::COMPLETED_NO);

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

    CORBA::Boolean original;
    GIOPObjectInfo_var self_info = objptr->getInvokeInfo(original);
    GIOPObjectInfo_var other_info = other_objptr->getInvokeInfo(original);
    
    if (!objptr->is_proxy()) {
      // this is a local object
      if (!other_objptr->is_proxy()) {
	return 1;  // other_object is also local
      }
      else {
	// other_object is a proxy.
	// Have to check if the proxy actually points back to this object
	// via the loop back connection
	if (objptr->_objectManager()->defaultLoopBack() == other_info->rope())
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
	if (other_objptr->_objectManager()->defaultLoopBack() == self_info->rope())
	  return 1;
	else
	  return 0;
      }
      else {
	// both are proxy objects, check whether they go back to the same
	// address space. Note: object keys are not globally unique.
	if (self_info->rope() == other_info->rope())
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

  CORBA::Boolean original;
  GIOPObjectInfo_var self_info = objptr->getInvokeInfo(original);

  size_t s = self_info->keysize();
  char *k = (char*)self_info->key();
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


class omniNonExistentProxyCallDesc : public OmniProxyCallDesc
{
public:
  inline omniNonExistentProxyCallDesc(const char* _op, size_t _op_len) :
    OmniProxyCallDesc(_op, _op_len)  {}

  virtual void unmarshalReturnedValues(cdrStream& s) {
    pd_result <<= s;
  }
  inline CORBA::Boolean result() { return pd_result; }

private:
  CORBA::Boolean pd_result;
};


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

  try {
    omniNonExistentProxyCallDesc _call_desc("_non_existent", 14);

    OmniProxyCallWrapper::invoke(objptr, _call_desc);
    return _call_desc.result();
  }
  catch (const CORBA::OBJECT_NOT_EXIST&) {
    return 1;
  }
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


void
CORBA::
Object::marshalObjRef(CORBA::Object_ptr obj,cdrStream &s)
{
  s.marshalObjRef((CORBA::is_nil(obj) ? 0 : obj->PR_getobj()));
}

CORBA::Object_ptr
CORBA::
Object::unmarshalObjRef(cdrStream &s)
{
  omniObject* obj = s.unMarshalObjRef(0);
  if (obj)
    return (CORBA::Object_ptr)(obj->_widenFromTheMostDerivedIntf(0));
  else
    return CORBA::Object::_nil();
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

void
CORBA::
Object_Helper::marshalObjRef(CORBA::Object_ptr obj,cdrStream &s)
{
  CORBA::Object::marshalObjRef(obj,s);
}

CORBA::Object_ptr
CORBA::
Object_Helper::unmarshalObjRef(cdrStream &s)
{
  return CORBA::Object::unmarshalObjRef(s);
}


