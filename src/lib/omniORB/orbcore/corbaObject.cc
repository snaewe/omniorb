// -*- Mode: C++; -*-
//                            Package   : omniORB
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
// Description:
//    Implementation of the CORBA::Object interface.
//      
 
/*
  $Log$
  Revision 1.20.2.6  2001/08/03 17:41:19  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.20.2.5  2001/05/31 16:18:12  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.20.2.4  2001/04/18 18:18:10  sll
  Big checkin with the brand new internal APIs.

  Revision 1.20.2.3  2000/11/07 18:44:03  sll
  Renamed omniObjRef::_hash and _is_equivalent to __hash and __is_equivalent
  to avoid name clash with the member functions of CORBA::Object.

  Revision 1.20.2.2  2000/09/27 18:13:26  sll
  Use the new cdrStream abstraction.
  Removed obsoluted code CORBA::UnMarshalObjRef() and CORBA::MarshalObjRef().
  Let the omniObjRef handles is_equivalent() and hash().

  Revision 1.20.2.1  2000/07/17 10:35:52  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.21  2000/07/13 15:25:58  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.19.6.9  2000/06/22 10:40:14  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.19.6.8  2000/02/07 15:27:36  dpg1
  Silly mistake in _is_equivalent() for pseudo object references.

  Revision 1.19.6.7  2000/02/04 18:11:01  djr
  Minor mods for IRIX (casting pointers to ulong instead of int).

  Revision 1.19.6.6  1999/10/29 13:18:16  djr
  Changes to ensure mutexes are constructed when accessed.

  Revision 1.19.6.5  1999/10/16 13:22:53  djr
  Changes to support compiling on MSVC.

  Revision 1.19.6.4  1999/10/14 16:22:06  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.19.6.3  1999/09/27 11:01:10  djr
  Modifications to logging.

  Revision 1.19.6.2  1999/09/24 10:29:33  djr
  CORBA::Object::Object now requires an argument.

  Revision 1.19.6.1  1999/09/22 14:26:45  djr
  Major rewrite of orbcore to support POA.

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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/minorCode.h>
#include <omniORB4/omniObjRef.h>
#include <objectAdapter.h>
#include <anonObject.h>
#include <exceptiondefs.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////// CORBA::Object ///////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Object::~Object()
{
  pd_magic = 0;
}


CORBA::Boolean
CORBA::Object::_is_a(const char* repoId)
{
  if( !repoId )  return 0;

  if( _NP_is_pseudo() )  return _ptrToObjRef(repoId) ? 1 : 0;

  if( _NP_is_nil() ) {
    if( omni::strMatch(repoId, "") )  return 1;
    else                              return 0;
  }
  else {
    return pd_obj->_real_is_a(repoId);
  }
}


CORBA::Boolean
CORBA::Object::_non_existent()
{
  if ( !_PR_is_valid(this) )  OMNIORB_THROW(BAD_PARAM,
					    BAD_PARAM_InvalidObjectRef,
					    CORBA::COMPLETED_NO);

  if( _NP_is_nil()    )  return 1;
  if( _NP_is_pseudo() )  return 0;

  try {
    return pd_obj->_remote_non_existent();
  }
  catch(CORBA::OBJECT_NOT_EXIST&) {
    return 1;
  }
}


CORBA::Boolean
CORBA::Object::_is_equivalent(CORBA::Object_ptr other_object)
{
  if ( !_PR_is_valid(this) )  OMNIORB_THROW(BAD_PARAM,
					    BAD_PARAM_InvalidObjectRef,
					    CORBA::COMPLETED_NO);


  if ( !_PR_is_valid(other_object) ) 
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidObjectRef,
		  CORBA::COMPLETED_NO);


  if( other_object == this )  return 1;

  // Pseudo objects are equivalent only if pointers are equal.
  // (So the above test should have gotten it).
  if( _NP_is_pseudo() )
    return 0;

  if( other_object->_NP_is_nil() ) {
    return _NP_is_nil();
  }
  else {
    if( _NP_is_nil() )  return 0;

    omniObjRef* objptr = _PR_getobj();
    omniObjRef* other_objptr = other_object->_PR_getobj();

    return objptr->__is_equivalent(other_objptr);
  }
}


CORBA::ULong
CORBA::Object::_hash(CORBA::ULong maximum)
{
  if( _NP_is_nil() || maximum == 0 )  return 0;
  if( _NP_is_pseudo() )  return CORBA::ULong((unsigned long) this) % maximum;

  return _PR_getobj()->__hash(maximum);
}


CORBA::Object_ptr 
CORBA::Object::_duplicate(CORBA::Object_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}


CORBA::Object_ptr
CORBA::Object::_nil()
{
  static CORBA::Object* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new CORBA::Object;
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


void
CORBA::Object::_NP_incrRefCount()
{
  OMNIORB_ASSERT(pd_obj);  OMNIORB_ASSERT(!_NP_is_pseudo());

  omni::duplicateObjRef(pd_obj);
}


void
CORBA::Object::_NP_decrRefCount()
{
  OMNIORB_ASSERT(pd_obj);  OMNIORB_ASSERT(!_NP_is_pseudo());

  omni::releaseObjRef(pd_obj);
}


void*
CORBA::Object::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


void
CORBA::
Object::_marshalObjRef(CORBA::Object_ptr obj, cdrStream& s)
{
  if (obj->_NP_is_pseudo()) OMNIORB_THROW(MARSHAL,MARSHAL_LocalObject,
					  (CORBA::CompletionStatus)s.completion());
  omniObjRef::_marshal(obj->_PR_getobj(),s);
}


CORBA::Object_ptr
CORBA::Object::_unmarshalObjRef(cdrStream& s)
{
  omniObjRef* o = omniObjRef::_unMarshal(_PD_repoId,s);
  if (o)
    return (CORBA::Object_ptr)o->_ptrToObjRef(_PD_repoId);
  else
    return _nil();
}



const char*
CORBA::Object::_PD_repoId = "IDL:omg.org/CORBA/Object:1.0";


// We put this here rather than in anonObject.cc to ensure that
// it is always linked into the application.
static const omniAnonObjRef_pof _theomniAnonObjRef_pof;

//////////////////////////////////////////////////////////////////////
//////////////////////// CORBA::Object_Helper ////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Object_ptr
CORBA::
Object_Helper::_nil() 
{
  return Object::_nil();
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
Object_Helper::marshalObjRef(CORBA::Object_ptr obj, cdrStream& s)
{
  CORBA::Object::_marshalObjRef(obj,s);
}


CORBA::Object_ptr
CORBA::
Object_Helper::unmarshalObjRef(cdrStream& s)
{
  return CORBA::Object::_unmarshalObjRef(s);
}



