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

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB3/omniObjRef.h>
#include <objectAdapter.h>
#include <ropeFactory.h>
#include <anonObject.h>
#include <exception.h>

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
    if( strcmp(repoId, "") == 0 )  return 1;
    else                           return 0;
  }
  else {
    return pd_obj->_real_is_a(repoId);
  }
}


CORBA::Boolean
CORBA::Object::_non_existent()
{
  if ( !_PR_is_valid(this) )  OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

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
  if ( !_PR_is_valid(this) )  OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

  if ( !_PR_is_valid(other_object) ) 
    OMNIORB_THROW(OBJECT_NOT_EXIST,0,CORBA::COMPLETED_NO);

  if( other_object == this )  return 1;

  // Pseudo objects are equivalent only if pointers are equal.
  // (So the above test should have gotten it).
  if( _NP_is_pseudo() )
    return other_object->_NP_is_pseudo();

  if( other_object->_NP_is_nil() ) {
    return _NP_is_nil();
  }
  else {
    if( _NP_is_nil() )  return 0;

    omniObjRef* objptr = _PR_getobj();
    omniObjRef* other_objptr = other_object->_PR_getobj();

    omniRopeAndKey rak, other_rak;
    CORBA::Boolean is_local, other_is_local;
    objptr->_getRopeAndKey(rak, &is_local);
    other_objptr->_getRopeAndKey(other_rak, &other_is_local);

    if( rak.keysize() != other_rak.keysize() ||
	memcmp((void*) rak.key(),(void*) other_rak.key(),
	       rak.keysize()) != 0 )
      // Object keys do not match.
      return 0;

    return is_local && other_is_local
      || !is_local && !other_is_local && rak.rope() == other_rak.rope();

  }
}


CORBA::ULong
CORBA::Object::_hash(CORBA::ULong maximum)
{
  if( _NP_is_nil() || maximum == 0 )  return 0;
  if( _NP_is_pseudo() )  return CORBA::ULong((unsigned long) this) % maximum;

  omniObjKey key;
  pd_obj->_getTheKey(key);

  return CORBA::ULong(omni::hash(key.key(), key.size()) % maximum);
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

  if( !strcmp(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


size_t
CORBA::
Object::_NP_alignedSize(CORBA::Object_ptr obj, size_t initialoffset)
{
  if (CORBA::is_nil(obj)) {
    return CORBA::AlignedObjRef(obj, 0, 0, initialoffset);
  }
  else {
    const char* irid = obj->pd_obj->_mostDerivedRepoId();
    return CORBA::AlignedObjRef(obj, irid, strlen(irid) + 1, initialoffset);
  }
}


void
CORBA::
Object::_marshalObjRef(CORBA::Object_ptr obj, NetBufferedStream& s)
{
  OMNIORB_ASSERT(!obj->_NP_is_pseudo());

  if (CORBA::is_nil(obj)) {
    CORBA::MarshalObjRef(obj,0,0,s);
  }
  else {
    const char *repoId = obj->pd_obj->_mostDerivedRepoId();
    CORBA::MarshalObjRef(obj, repoId, strlen(repoId) + 1, s);
  }
}


CORBA::Object_ptr
CORBA::Object::_unmarshalObjRef(NetBufferedStream& s)
{
  return CORBA::UnMarshalObjRef(_PD_repoId, s);
}


void
CORBA::
Object::_marshalObjRef(CORBA::Object_ptr obj, MemBufferedStream& s)
{
  OMNIORB_ASSERT(!obj->_NP_is_pseudo());

  if (CORBA::is_nil(obj)) {
    CORBA::MarshalObjRef(obj,0,0,s);
  }
  else {
    const char *repoId = obj->pd_obj->_mostDerivedRepoId();
    CORBA::MarshalObjRef(obj, repoId, strlen(repoId) + 1, s);
  }
}


CORBA::Object_ptr
CORBA::Object::_unmarshalObjRef(MemBufferedStream& s)
{
  return CORBA::UnMarshalObjRef(_PD_repoId, s);
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


size_t
CORBA::
Object_Helper::NP_alignedSize(CORBA::Object_ptr obj,size_t initialoffset)
{
  return CORBA::Object::_NP_alignedSize(obj,initialoffset);
}


void
CORBA::
Object_Helper::marshalObjRef(CORBA::Object_ptr obj, NetBufferedStream& s)
{
  CORBA::Object::_marshalObjRef(obj,s);
}


CORBA::Object_ptr
CORBA::
Object_Helper::unmarshalObjRef(NetBufferedStream& s)
{
  return CORBA::Object::_unmarshalObjRef(s);
}


void
CORBA::
Object_Helper::marshalObjRef(CORBA::Object_ptr obj, MemBufferedStream& s)
{
  CORBA::Object::_marshalObjRef(obj,s);
}


CORBA::Object_ptr
CORBA::
Object_Helper::unmarshalObjRef(MemBufferedStream& s)
{
  return CORBA::Object::_unmarshalObjRef(s);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Object_ptr
CORBA::UnMarshalObjRef(const char* repoId, NetBufferedStream& s)
{
  OMNIORB_ASSERT(repoId);

  CORBA::ULong idlen;
  CORBA::Char* id = 0;
  IOP::TaggedProfileList* profiles = 0;

  try {
    idlen <<= s;

    switch (idlen) {

    case 0:
      // According to the CORBA specification 2.0 section 10.6.2:
      //   Null object references are indicated by an empty set of
      //   profiles, and by a NULL type ID (a string which contain
      //   only *** a single terminating character ***).
      //
      // Therefore the idlen should be 1.
      // Visibroker for C++ (Orbeline) 2.0 Release 1.51 gets it wrong
      // and sends out a 0 len string.
      id = new CORBA::Char[1];
      id[0] = (CORBA::Char)'\0';
      break;

    case 1:
      id = new CORBA::Char[1];
      id[0] <<= s;
      if (id[0] != (CORBA::Char)'\0')
	OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_MAYBE);
      idlen = 0;
      break;

    default:
      if (idlen > s.RdMessageUnRead())
	OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_MAYBE);
      id = new CORBA::Char[idlen];
      if( !id )  OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_MAYBE);
      s.get_char_array(id, idlen);
      if( id[idlen - 1] != '\0' )
	OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_MAYBE);
      break;
    }

    profiles = new IOP::TaggedProfileList();
    if( !profiles )  OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_MAYBE);
    *profiles <<= s;

    if (profiles->length() == 0 && idlen == 0) {
      // This is a nil object reference
      delete profiles;
      delete[] id;
      return CORBA::Object::_nil();
    }
    else {
      // It is possible that we reach here with the id string = '\0'.
      // That is alright because the actual type of the object will be
      // verified using _is_a() at the first invocation on the object.
      //
      // Apparently, some ORBs such as ExperSoft's do that. Furthermore,
      // this has been accepted as a valid behaviour in GIOP 1.1/IIOP 1.1.
      //
      omniObjRef* objref = omni::createObjRef((const char*) id, repoId,
					      profiles, 1, 0);

      profiles = 0;
      delete[] id;
      id = 0;

      if( !objref )  OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
      return (CORBA::Object_ptr)  objref->_ptrToObjRef(Object::_PD_repoId);
    }
  }
  catch (...) {
    if( id )        delete[] id;
    if( profiles )  delete profiles;
    throw;
  }
}


void
CORBA::MarshalObjRef(CORBA::Object_ptr obj, const char* repoId,
		     size_t repoIdSize, NetBufferedStream& s)
{
  if (CORBA::is_nil(obj)) {
    // nil object reference
    ::operator>>= ((CORBA::ULong)1,s);
    ::operator>>= ((CORBA::Char) '\0',s);
    ::operator>>= ((CORBA::ULong) 0,s);
    return;
  }

  // non-nil object reference
  repoId = obj->_PR_getobj()->_mostDerivedRepoId();
  repoIdSize = strlen(repoId) + 1;
  CORBA::ULong(repoIdSize) >>= s;
  s.put_char_array((CORBA::Char*) repoId, repoIdSize);
  IOP::TaggedProfileList* pl = obj->_PR_getobj()->_iopProfiles();
  *pl >>= s;
}


size_t
CORBA::AlignedObjRef(CORBA::Object_ptr obj, const char* repoId,
		     size_t repoIdSize, size_t initialoffset)
{
  omni::ptr_arith_t msgsize = omni::align_to((omni::ptr_arith_t)
                                                   initialoffset,
						   omni::ALIGN_4);
  if (CORBA::is_nil(obj)) {
    return (size_t) (msgsize + 3 * sizeof(CORBA::ULong));
  }
  else {
    repoId = obj->_PR_getobj()->_mostDerivedRepoId();
    repoIdSize = strlen(repoId)+1;
    msgsize += (omni::ptr_arith_t)(sizeof(CORBA::ULong)+repoIdSize);
    IOP::TaggedProfileList *pl = obj->_PR_getobj()->_iopProfiles();
    return pl->_NP_alignedSize((size_t)msgsize);
  }
}


CORBA::Object_ptr
CORBA::UnMarshalObjRef(const char* repoId, MemBufferedStream& s)
{
  OMNIORB_ASSERT(repoId);

  CORBA::ULong idlen;
  CORBA::Char* id = 0;
  IOP::TaggedProfileList* profiles = 0;

  try {
    idlen <<= s;

    switch (idlen) {

    case 0:
      // According to the CORBA specification 2.0 section 10.6.2:
      //   Null object references are indicated by an empty set of
      //   profiles, and by a NULL type ID (a string which contain
      //   only *** a single terminating character ***).
      //
      // Therefore the idlen should be 1.
      // Visibroker for C++ (Orbeline) 2.0 Release 1.51 gets it wrong
      // and sends out a 0 len string.
      id = new CORBA::Char[1];
      id[0] = (CORBA::Char)'\0';
      break;

    case 1:
      id = new CORBA::Char[1];
      id[0] <<= s;
      if (id[0] != (CORBA::Char)'\0')
	OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_MAYBE);
      idlen = 0;
      break;

    default:
      if (idlen > s.RdMessageUnRead())
	OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_MAYBE);
      id = new CORBA::Char[idlen];
      if( !id )  OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_MAYBE);
      s.get_char_array(id, idlen);
      if( id[idlen - 1] != '\0' )
	OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_MAYBE);
      break;
    }

    profiles = new IOP::TaggedProfileList();
    if( !profiles )  OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_MAYBE);
    *profiles <<= s;

    if (profiles->length() == 0 && idlen == 0) {
      // This is a nil object reference
      delete profiles;
      delete[] id;
      return CORBA::Object::_nil();
    }
    else {
      // It is possible that we reach here with the id string = '\0'.
      // That is alright because the actual type of the object will be
      // verified using _is_a() at the first invocation on the object.
      //
      // Apparently, some ORBs such as ExperSoft's do that. Furthermore,
      // this has been accepted as a valid behaviour in GIOP 1.1/IIOP 1.1.
      // 
      omniObjRef* objref = omni::createObjRef((const char*) id, repoId,
					      profiles, 1, 0);

      profiles = 0;
      delete[] id;
      id = 0;

      if( !objref )  OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
      return (CORBA::Object_ptr)  objref->_ptrToObjRef(Object::_PD_repoId);
    }
  }
  catch (...) {
    if( id )        delete[] id;
    if( profiles )  delete profiles;
    throw;
  }
}


void 
CORBA::MarshalObjRef(CORBA::Object_ptr obj, const char* repoId,
		     size_t repoIdSize, MemBufferedStream& s)
{
  if (CORBA::is_nil(obj)) {
    // nil object reference
    ::operator>>= ((CORBA::ULong)1,s);
    ::operator>>= ((CORBA::Char) '\0',s);
    ::operator>>= ((CORBA::ULong) 0,s);
    return;
  }

  // non-nil object reference
  repoId = obj->_PR_getobj()->_mostDerivedRepoId();
  repoIdSize = strlen(repoId)+1;
  ::operator>>= ((CORBA::ULong) repoIdSize,s);
  s.put_char_array((CORBA::Char*) repoId, repoIdSize);
  IOP::TaggedProfileList * pl = obj->_PR_getobj()->_iopProfiles();
  *pl >>= s;
}
