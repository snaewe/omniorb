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
  Revision 1.2  1997/01/08 18:02:32  ewc
  Added unsigned int to remove compiler warning

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

static CORBA::Object CORBA_Object_nil;

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

