// -*- Mode: C++; -*-
//                            Package   : omniORB
// dynAnyNil.cc               Created on: 11/1998
//                            Author    : David Riddoch (djr)
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
//   Implementation of nil DynAny objects.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/objTracker.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <dynAny.h>

OMNI_USING_NAMESPACE(omni)


OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////// omniNilDynAny ///////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynAny : public virtual CORBA::DynAny,
		      public omniTrackedObject {
public:
  virtual CORBA::TypeCode_ptr type() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void assign(CORBA::DynAny_ptr dyn_any) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void from_any(const CORBA::Any& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Any* to_any() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void destroy() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::DynAny_ptr copy() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void insert_boolean(CORBA::Boolean value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_octet(CORBA::Octet value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_char(CORBA::Char value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_wchar(CORBA::WChar value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_short(CORBA::Short value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_ushort(CORBA::UShort value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_long(CORBA::Long value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_ulong(CORBA::ULong value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
#ifdef HAS_LongLong
  virtual void insert_longlong(CORBA::LongLong value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_ulonglong(CORBA::ULongLong value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
#endif
#ifndef NO_FLOAT
  virtual void insert_float(CORBA::Float value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_double(CORBA::Double value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
#ifdef HAS_LongDouble
  virtual void insert_longdouble(CORBA::LongDouble value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
#endif
#endif
  virtual void insert_string(const char* value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_wstring(const CORBA::WChar* value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_reference(CORBA::Object_ptr value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_typecode(CORBA::TypeCode_ptr value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_any(const CORBA::Any& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Boolean get_boolean() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Octet get_octet() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Char get_char() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::WChar get_wchar() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Short get_short() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::UShort get_ushort() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Long get_long() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::ULong get_ulong() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
#ifdef HAS_LongLong
  virtual CORBA::LongLong get_longlong() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::ULongLong get_ulonglong() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
#endif
#ifndef NO_FLOAT
  virtual CORBA::Float get_float() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0.0;
  }
  virtual CORBA::Double get_double() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0.0;
  }
#ifdef HAS_LongDouble
  virtual CORBA::LongDouble get_longdouble() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0.0;
  }
#endif
#endif
  virtual char* get_string() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::WChar* get_wstring() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Object_ptr get_reference() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::TypeCode_ptr get_typecode() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Any* get_any() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::DynAny_ptr current_component() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Boolean next() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Boolean seek(CORBA::Long index) {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void rewind() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Boolean NP_is_nil() const {
    return 1;
  }
  virtual int NP_nodetype() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void* NP_narrow() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
};

OMNI_NAMESPACE_END(omni)

CORBA::DynAny_ptr
CORBA::DynAny::_nil()
{
  static omniNilDynAny* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilDynAny;
    registerTrackedObject(_the_nil_ptr);
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniNilDynEnum ///////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynEnum : public CORBA::DynEnum, public omniNilDynAny {
public:
  virtual char* value_as_string() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void value_as_string(const char* value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::ULong value_as_ulong() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void value_as_ulong(CORBA::ULong value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
};

OMNI_NAMESPACE_END(omni)

CORBA::DynEnum_ptr
CORBA::DynEnum::_nil()
{
  static omniNilDynEnum* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilDynEnum;
    registerTrackedObject(_the_nil_ptr);
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
////////////////////////// omniNilDynStruct //////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynStruct : public CORBA::DynStruct, public omniNilDynAny {
public:
  virtual char*  current_member_name() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::TCKind current_member_kind() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::tk_null;
  }
  virtual CORBA::NameValuePairSeq* get_members() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_members(const CORBA::NameValuePairSeq& NVSeqVal) {
    _CORBA_invoked_nil_pseudo_ref();
  }
};

OMNI_NAMESPACE_END(omni)

CORBA::DynStruct_ptr
CORBA::DynStruct::_nil()
{
  static omniNilDynStruct* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilDynStruct;
    registerTrackedObject(_the_nil_ptr);
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniNilDynUnion //////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynUnion : public CORBA::DynUnion, public omniNilDynAny {
public:
  virtual CORBA::Boolean set_as_default() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_as_default(CORBA::Boolean value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::DynAny_ptr discriminator() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::TCKind discriminator_kind() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::tk_null;
  }
  virtual CORBA::DynAny_ptr member() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual char*  member_name() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void member_name(const char* value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::TCKind member_kind() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::tk_null;
  }
};

OMNI_NAMESPACE_END(omni)

CORBA::DynUnion_ptr
CORBA::DynUnion::_nil()
{
  static omniNilDynUnion* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilDynUnion;
    registerTrackedObject(_the_nil_ptr);
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
///////////////////////// omniNilDynSequence /////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynSequence : public CORBA::DynSequence, public omniNilDynAny {
public:
  virtual CORBA::ULong length() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void length (CORBA::ULong value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::AnySeq* get_elements() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_elements(const CORBA::AnySeq& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
};

OMNI_NAMESPACE_END(omni)

CORBA::DynSequence_ptr
CORBA::DynSequence::_nil()
{
  static omniNilDynSequence* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilDynSequence;
    registerTrackedObject(_the_nil_ptr);
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniNilDynArray //////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynArray : public CORBA::DynArray, public omniNilDynAny {
public:
  virtual CORBA::AnySeq* get_elements() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_elements(const CORBA::AnySeq& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
};

OMNI_NAMESPACE_END(omni)

CORBA::DynArray_ptr
CORBA::DynArray::_nil()
{
  static omniNilDynArray* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilDynArray;
    registerTrackedObject(_the_nil_ptr);
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}
