// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaObject.cc             Created on: 13/5/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//      Implementation of the Object interface
//      
 
/*
  $Log$
  Revision 1.5  1997/01/30 19:57:48  sll
  Added is_nil(), duplicate(), release() in Object_Helper class.

  Revision 1.4  1997/01/23 15:29:50  sll
  Removed static object CORBA_Object_nil.
  Added copy ctor and operator= for Object_var class because it is not
  instantiated from the sequence template anymore.

// Revision 1.3  1997/01/13  14:57:23  sll
// Added marshalling routines for CORBA::Object.
//
// Revision 1.2  1997/01/08  18:02:32  ewc
// Added unsigned int to remove compiler warning
//
// Revision 1.1  1997/01/08  17:26:01  sll
// Initial revision
//
  */

#include <omniORB2/CORBA.h>


CORBA::
Object::Object()
{
  pd_obj = 0;
  return;
}


CORBA::Object_ptr
CORBA::
Object::_nil()
{
  return (&CORBA_Object_nil);
}


CORBA::Boolean
CORBA::
Object::NP_is_nil()
{ 
  return (pd_obj)?0:1; 
}


CORBA::Object_ptr 
CORBA::
Object::_duplicate(CORBA::Object_ptr obj)
{
  if (!CORBA::is_nil(obj)) {
    omniORB::objectDuplicate(obj->pd_obj);
  }
  return obj;
}

void
CORBA::
Object::NP_release()
{ 
  if (!NP_is_nil()) {
    omniORB::objectRelease(pd_obj); 
  }
  return;
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
  if (!repoId)
    return 0;

  if (NP_is_nil()) {
    if (strcmp(repoId,"") == 0)
      return 1;
    else
      return 0;
  }
  else {
    omniObject * objptr = PR_getobj();
    if (objptr->_widenFromTheMostDerivedIntf(repoId))
      return 1;
    else
      return 0;
  }
}

CORBA::Boolean
CORBA::
Object::_is_equivalent(CORBA::Object_ptr other_object)
{
  if (other_object->NP_is_nil()) {
    if (NP_is_nil())
      return 1;
    else
      return 0;
  }
  else {
    omniObject * objptr = PR_getobj();
    omniObject * other_objptr = other_object->PR_getobj();
    if (objptr->objkeysize() != other_objptr->objkeysize() ||
	memcmp(objptr->objkey(),other_objptr->objkey(),
	       objptr->objkeysize()) != 0)
      {
	return 0; // object keys do not match
      }
    if (!objptr->is_proxy()) {
      // this is a local object
      if (!other_objptr->is_proxy()) {
	return 1;  // other_object is also local
      }
      else {
	return 0;   // other_object is a proxy 
      }
    }
    else {
      // this is a proxy object
      if (!other_objptr->is_proxy()) {
	return 0;  // other_object is local
      }
      else {
	// both are proxy objects, check whether they go back to the same
	// address space. Note: object keys are not globally unique.
	if (objptr->_rope() == other_objptr->_rope())
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
  size_t s = objptr->objkeysize();
  char *k = (char *)objptr->objkey();
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
  // XXX not implemented yet, waiting for support of GIOP Lookup message
  throw omniORB::fatalException(__FILE__,__LINE__,
				"CORBA::Object::_non_existent() has not been implemeted yet.");
  return 0;
}


CORBA::ImplementationDef_ptr
CORBA::
Object::_get_implementation()
{
  // XXX not implemented yet
  throw omniORB::fatalException(__FILE__,__LINE__,
				"CORBA::Object::_get_implementation() has not been implemeted yet.");
  return 0;
}

CORBA::InterfaceDef_ptr
CORBA::
Object::_get_interface()
{
  // XXX not implemented yet
  throw omniORB::fatalException(__FILE__,__LINE__,
				"CORBA::Object::_get_interface() has not been implemeted yet.");
  return 0;
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
