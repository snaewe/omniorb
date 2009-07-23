// -*- Mode: C++; -*-
//                            Package   : omniORB
// localObject.cc             Created on: 2005/10/07
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2005 Apasphere Ltd.
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
//    CORBA::LocalObject
//

/*
  $Log$
*/

#include <omniORB4/CORBA.h>
#include <omniORB4/objTracker.h>


OMNI_USING_NAMESPACE(omni)

//
// LocalObject_Helper
//

CORBA::LocalObject_ptr
CORBA::LocalObject_Helper::_nil()
{
  return CORBA::LocalObject::_nil();
}

CORBA::Boolean
CORBA::LocalObject_Helper::is_nil(CORBA::LocalObject_ptr obj)
{
  return CORBA::is_nil(obj);
}

void
CORBA::LocalObject_Helper::release(CORBA::LocalObject_ptr obj)
{
  CORBA::release(obj);
}

void
CORBA::LocalObject_Helper::duplicate(CORBA::LocalObject_ptr obj) {
  if (obj && !obj->_NP_is_nil())
    obj->_NP_incrRefCount();
}

void
CORBA::LocalObject_Helper::marshalObjRef(CORBA::LocalObject_ptr obj,
					 cdrStream& s) {
  CORBA::LocalObject::_marshalObjRef(obj, s);
}

CORBA::LocalObject_ptr
CORBA::LocalObject_Helper::unmarshalObjRef(cdrStream& s) {
  return CORBA::LocalObject::_unmarshalObjRef(s);
}


//
// LocalObject
//

void
CORBA::LocalObject::_add_ref() {}

void
CORBA::LocalObject::_remove_ref() {}

CORBA::Boolean
CORBA::LocalObject::_non_existent()
{
  return 0;
}

CORBA::LocalObject_ptr
CORBA::LocalObject::_duplicate(CORBA::LocalObject_ptr obj)
{
  if (obj && !obj->_NP_is_nil())
    obj->_NP_incrRefCount();
  return obj;
}

CORBA::LocalObject_ptr
CORBA::LocalObject::_narrow(CORBA::Object_ptr obj)
{
  if (!obj || obj->_NP_is_nil()) return _nil();
  _ptr_type e = (_ptr_type) obj->_ptrToObjRef(_PD_repoId);
  if (e){
    e->_NP_incrRefCount();
    return e;
  }
  return _nil();
}

CORBA::LocalObject_ptr
CORBA::LocalObject::_unchecked_narrow(CORBA::Object_ptr obj)
{
  return _narrow(obj);
}

CORBA::LocalObject_ptr
CORBA::LocalObject::_nil()
{
  static CORBA::LocalObject* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new CORBA::LocalObject;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

CORBA::LocalObject::~LocalObject() {}

void
CORBA::LocalObject::_NP_incrRefCount()
{
  _add_ref();
}

void
CORBA::LocalObject::_NP_decrRefCount()
{
  _remove_ref();
}

void*
CORBA::LocalObject::_ptrToObjRef(const char* id)
{
  if (id == CORBA::LocalObject::_PD_repoId )
    return (CORBA::LocalObject_ptr) this;
  
  if (id == CORBA::Object::_PD_repoId )
    return (CORBA::Object_ptr) this;

  if (omni::strMatch(id, CORBA::LocalObject::_PD_repoId))
    return (CORBA::LocalObject_ptr) this;
  
  if (omni::strMatch(id, CORBA::Object::_PD_repoId))
    return (CORBA::Object_ptr) this;

  return 0;
}

void
CORBA::LocalObject::_marshalObjRef(CORBA::LocalObject_ptr o, cdrStream& s)
{
  OMNIORB_THROW(MARSHAL, MARSHAL_LocalObject,
		(CORBA::CompletionStatus)s.completion());
}

CORBA::LocalObject_ptr
CORBA::LocalObject::_unmarshalObjRef(cdrStream& s)
{
  OMNIORB_THROW(MARSHAL, MARSHAL_LocalObject,
		(CORBA::CompletionStatus)s.completion());
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

const char*
CORBA::LocalObject::_PD_repoId = "IDL:omg.org/CORBA/LocalObject:1.0";
