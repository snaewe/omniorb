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

#include <dynAnyImpl.h>

#define NILINHERITANCE(x) public omniNilDynAny, public DynamicAny::x


OMNI_USING_NAMESPACE(omni)


OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////// omniNilDynAny ///////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynAny : public virtual DynamicAny::DynAny {
public:
  omniNilDynAny() : OMNIORB_BASE_CTOR(DynamicAny::)DynAny(1) {}

  virtual CORBA::TypeCode_ptr type() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void assign(DynamicAny::DynAny_ptr dyn_any) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void from_any(const CORBA::Any& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Any* to_any() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Boolean equal(DynamicAny::DynAny_ptr dyn_any) {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void destroy() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual DynamicAny::DynAny_ptr copy() {
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
#ifndef NO_FLOAT
  virtual void insert_float(CORBA::Float value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_double(CORBA::Double value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
#endif
  virtual void insert_string(const char* value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_reference(CORBA::Object_ptr value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_typecode(CORBA::TypeCode_ptr value) {
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
#ifdef HAS_LongDouble
  virtual void insert_longdouble(CORBA::LongDouble value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
#endif
  virtual void insert_wchar(CORBA::WChar value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_wstring(const CORBA::WChar* value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_any(const CORBA::Any& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_dyn_any(DynamicAny::DynAny_ptr value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_val(CORBA::ValueBase* value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_abstract(CORBA::AbstractBase_ptr value) {
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
#ifndef NO_FLOAT
  virtual CORBA::Float get_float() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0.0;
  }
  virtual CORBA::Double get_double() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0.0;
  }
#endif
  virtual char* get_string() {
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
#ifdef HAS_LongDouble
  virtual CORBA::LongDouble get_longdouble() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0.0;
  }
#endif
  virtual CORBA::WChar get_wchar() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::WChar* get_wstring() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Any* get_any() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual DynamicAny::DynAny_ptr get_dyn_any() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::ValueBase* get_val() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::AbstractBase_ptr get_abstract() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }

  virtual void insert_boolean_seq(CORBA::BooleanSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_octet_seq(CORBA::OctetSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_char_seq(CORBA::CharSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_short_seq(CORBA::ShortSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_ushort_seq(CORBA::UShortSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_long_seq(CORBA::LongSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_ulong_seq(CORBA::ULongSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
#ifndef NO_FLOAT
  virtual void insert_float_seq(CORBA::FloatSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_double_seq(CORBA::DoubleSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
#endif
#ifdef HAS_LongLong
  virtual void insert_longlong_seq(CORBA::LongLongSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void insert_ulonglong_seq(CORBA::ULongLongSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
#endif
#ifdef HAS_LongDouble						 
  virtual void insert_longdouble_seq(CORBA::LongDoubleSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
#endif								 
  virtual void insert_wchar_seq(CORBA::WCharSeq&) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::BooleanSeq* get_boolean_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::OctetSeq* get_octet_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::CharSeq* get_char_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::ShortSeq* get_short_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::UShortSeq* get_ushort_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::LongSeq* get_long_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::ULongSeq* get_ulong_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
#ifndef NO_FLOAT
  virtual CORBA::FloatSeq* get_float_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::DoubleSeq* get_double_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
#endif
#ifdef HAS_LongLong
  virtual CORBA::LongLongSeq* get_longlong_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::ULongLongSeq* get_ulonglong_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
#endif
#ifdef HAS_LongDouble
  virtual CORBA::LongDoubleSeq* get_longdouble_seq() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
#endif
  virtual CORBA::WCharSeq* get_wchar_seq() {
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
  virtual CORBA::Boolean next() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::ULong component_count() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual DynamicAny::DynAny_ptr current_component() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual int NP_nodetype() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void* _ptrToObjRef(const char* repoId) {
    if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
      return (DynamicAny::DynAny_ptr) this;
  
    if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
      return (CORBA::Object_ptr) this;

    return 0;
  }
  virtual void _NP_incrRefCount() {}
  virtual void _NP_decrRefCount() {}
};

OMNI_NAMESPACE_END(omni)

DynamicAny::DynAny_ptr
DynamicAny::DynAny::_nil()
{
  static omniNilDynAny* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilDynAny;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniNilDynFixed //////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynFixed : NILINHERITANCE(DynFixed) {
public:
  omniNilDynFixed() : OMNIORB_BASE_CTOR(DynamicAny::)DynAny(1) {}

  virtual char* get_value() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Boolean set_value(const char* val) {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  void* _ptrToObjRef(const char* repoId) {
    if( omni::ptrStrMatch(repoId, DynamicAny::DynFixed::_PD_repoId) )
      return (DynamicAny::DynFixed_ptr) this;
  
    if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
      return (DynamicAny::DynAny_ptr) this;
  
    if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
      return (CORBA::Object_ptr) this;

    return 0;
  }
};

OMNI_NAMESPACE_END(omni)

DynamicAny::DynFixed_ptr
DynamicAny::DynFixed::_nil()
{
  static omniNilDynFixed* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilDynFixed;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniNilDynEnum ///////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynEnum : NILINHERITANCE(DynEnum) {
public:
  omniNilDynEnum() : OMNIORB_BASE_CTOR(DynamicAny::)DynAny(1) {}

  virtual char* get_as_string() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_as_string(const char* value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::ULong get_as_ulong() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_as_ulong(CORBA::ULong value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  void* _ptrToObjRef(const char* repoId) {
    if( omni::ptrStrMatch(repoId, DynamicAny::DynEnum::_PD_repoId) )
      return (DynamicAny::DynEnum_ptr) this;
  
    if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
      return (DynamicAny::DynAny_ptr) this;
  
    if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
      return (CORBA::Object_ptr) this;

    return 0;
  }
};

OMNI_NAMESPACE_END(omni)

DynamicAny::DynEnum_ptr
DynamicAny::DynEnum::_nil()
{
  static omniNilDynEnum* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilDynEnum;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
////////////////////////// omniNilDynStruct //////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynStruct : NILINHERITANCE(DynStruct) {
public:
  omniNilDynStruct() : OMNIORB_BASE_CTOR(DynamicAny::)DynAny(1) {}

  virtual char*  current_member_name() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::TCKind current_member_kind() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::tk_null;
  }
  virtual DynamicAny::NameValuePairSeq* get_members() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_members(const DynamicAny::NameValuePairSeq& NVSeqVal) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual DynamicAny::NameDynAnyPairSeq* get_members_as_dyn_any() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_members_as_dyn_any(const DynamicAny::NameDynAnyPairSeq& NVSeqVal) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  void* _ptrToObjRef(const char* repoId) {
    if( omni::ptrStrMatch(repoId, DynamicAny::DynStruct::_PD_repoId) )
      return (DynamicAny::DynStruct_ptr) this;
  
    if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
      return (DynamicAny::DynAny_ptr) this;
  
    if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
      return (CORBA::Object_ptr) this;

    return 0;
  }
};

OMNI_NAMESPACE_END(omni)

DynamicAny::DynStruct_ptr
DynamicAny::DynStruct::_nil()
{
  static omniNilDynStruct* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilDynStruct;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniNilDynUnion //////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynUnion : NILINHERITANCE(DynUnion) {
public:
  omniNilDynUnion() : OMNIORB_BASE_CTOR(DynamicAny::)DynAny(1) {}

  virtual DynamicAny::DynAny_ptr get_discriminator() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_discriminator(DynamicAny::DynAny_ptr d) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void set_to_default_member() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void set_to_no_active_member() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Boolean has_no_active_member() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::TCKind discriminator_kind() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::tk_null;
  }
  virtual DynamicAny::DynAny_ptr member() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual char* member_name() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::TCKind member_kind() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::tk_null;
  }
  virtual CORBA::Boolean is_set_to_default_member() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  void* _ptrToObjRef(const char* repoId) {
    if( omni::ptrStrMatch(repoId, DynamicAny::DynUnion::_PD_repoId) )
      return (DynamicAny::DynUnion_ptr) this;
  
    if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
      return (DynamicAny::DynAny_ptr) this;
  
    if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
      return (CORBA::Object_ptr) this;

    return 0;
  }
};

OMNI_NAMESPACE_END(omni)

DynamicAny::DynUnion_ptr
DynamicAny::DynUnion::_nil()
{
  static omniNilDynUnion* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilDynUnion;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
///////////////////////// omniNilDynSequence /////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynSequence : NILINHERITANCE(DynSequence) {
public:
  omniNilDynSequence() : OMNIORB_BASE_CTOR(DynamicAny::)DynAny(1) {}

  virtual CORBA::ULong get_length() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_length(CORBA::ULong value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual DynamicAny::AnySeq* get_elements() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_elements(const DynamicAny::AnySeq& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual DynamicAny::DynAnySeq* get_elements_as_dyn_any() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_elements_as_dyn_any(const DynamicAny::DynAnySeq& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  void* _ptrToObjRef(const char* repoId) {
    if( omni::ptrStrMatch(repoId, DynamicAny::DynSequence::_PD_repoId) )
      return (DynamicAny::DynSequence_ptr) this;
  
    if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
      return (DynamicAny::DynAny_ptr) this;
  
    if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
      return (CORBA::Object_ptr) this;

    return 0;
  }
};

OMNI_NAMESPACE_END(omni)

DynamicAny::DynSequence_ptr
DynamicAny::DynSequence::_nil()
{
  static omniNilDynSequence* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilDynSequence;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniNilDynArray //////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynArray : NILINHERITANCE(DynArray) {
public:
  omniNilDynArray() : OMNIORB_BASE_CTOR(DynamicAny::)DynAny(1) {}

  virtual DynamicAny::AnySeq* get_elements() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_elements(const DynamicAny::AnySeq& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual DynamicAny::DynAnySeq* get_elements_as_dyn_any() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_elements_as_dyn_any(const DynamicAny::DynAnySeq& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  void* _ptrToObjRef(const char* repoId) {
    if( omni::ptrStrMatch(repoId, DynamicAny::DynArray::_PD_repoId) )
      return (DynamicAny::DynArray_ptr) this;
  
    if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
      return (DynamicAny::DynAny_ptr) this;
  
    if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
      return (CORBA::Object_ptr) this;

    return 0;
  }
};

OMNI_NAMESPACE_END(omni)

DynamicAny::DynArray_ptr
DynamicAny::DynArray::_nil()
{
  static omniNilDynArray* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilDynArray;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)


//////////////////////////////////////////////////////////////////////
/////////////////////////// omniNilDynValueCommon ////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynValueCommon : NILINHERITANCE(DynValueCommon) {
public:
  omniNilDynValueCommon() : OMNIORB_BASE_CTOR(DynamicAny::)DynAny(1) {}

  virtual CORBA::Boolean is_null() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_to_null() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void set_to_value() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  void* _ptrToObjRef(const char* repoId) {
    if( omni::ptrStrMatch(repoId, DynamicAny::DynValueCommon::_PD_repoId) )
      return (DynamicAny::DynValueCommon_ptr) this;
  
    if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
      return (DynamicAny::DynAny_ptr) this;
  
    if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
      return (CORBA::Object_ptr) this;

    return 0;
  }
};

OMNI_NAMESPACE_END(omni)

DynamicAny::DynValueCommon_ptr
DynamicAny::DynValueCommon::_nil()
{
  static omniNilDynValueCommon* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilDynValueCommon;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)


//////////////////////////////////////////////////////////////////////
/////////////////////////// omniNilDynValue //////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynValue : NILINHERITANCE(DynValue) {
public:
  omniNilDynValue() {}

  virtual CORBA::Boolean is_null() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_to_null() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void set_to_value() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual char* current_member_name() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::TCKind current_member_kind() {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::tk_null;
  }
  virtual DynamicAny::NameValuePairSeq* get_members() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_members(const DynamicAny::NameValuePairSeq& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual DynamicAny::NameDynAnyPairSeq* get_members_as_dyn_any() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_members_as_dyn_any(const DynamicAny::NameDynAnyPairSeq& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  void* _ptrToObjRef(const char* repoId) {
    if( omni::ptrStrMatch(repoId, DynamicAny::DynValue::_PD_repoId) )
      return (DynamicAny::DynValue_ptr) this;
  
    if( omni::ptrStrMatch(repoId, DynamicAny::DynValueCommon::_PD_repoId) )
      return (DynamicAny::DynValueCommon_ptr) this;
  
    if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
      return (DynamicAny::DynAny_ptr) this;
  
    if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
      return (CORBA::Object_ptr) this;

    return 0;
  }
};

OMNI_NAMESPACE_END(omni)

DynamicAny::DynValue_ptr
DynamicAny::DynValue::_nil()
{
  static omniNilDynValue* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilDynValue;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)


//////////////////////////////////////////////////////////////////////
/////////////////////////// omniNilDynValueBox ///////////////////////
//////////////////////////////////////////////////////////////////////

  class omniNilDynValueBox : NILINHERITANCE(DynValueBox) {
public:
  omniNilDynValueBox() {}

  virtual CORBA::Boolean is_null() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_to_null() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void set_to_value() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Any* get_boxed_value() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_boxed_value(const CORBA::Any& value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual DynamicAny::DynAny_ptr get_boxed_value_as_dyn_any() {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void set_boxed_value_as_dyn_any(DynamicAny::DynAny_ptr value) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  void* _ptrToObjRef(const char* repoId) {
    if( omni::ptrStrMatch(repoId, DynamicAny::DynValueBox::_PD_repoId) )
      return (DynamicAny::DynValueBox_ptr) this;
  
    if( omni::ptrStrMatch(repoId, DynamicAny::DynValueCommon::_PD_repoId) )
      return (DynamicAny::DynValueCommon_ptr) this;
  
    if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
      return (DynamicAny::DynAny_ptr) this;
  
    if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
      return (CORBA::Object_ptr) this;

    return 0;
  }
};

OMNI_NAMESPACE_END(omni)

DynamicAny::DynValueBox_ptr
DynamicAny::DynValueBox::_nil()
{
  static omniNilDynValueBox* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilDynValueBox;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

OMNI_NAMESPACE_BEGIN(omni)





//////////////////////////////////////////////////////////////////////
/////////////////////////// omniNilDynAnyFactory /////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilDynAnyFactory : public virtual DynamicAny::DynAnyFactory {
public:
  omniNilDynAnyFactory() : OMNIORB_BASE_CTOR(DynamicAny::)DynAnyFactory(1) {}

  virtual DynamicAny::DynAny_ptr
  create_dyn_any(const CORBA::Any& value) {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual DynamicAny::DynAny_ptr
  create_dyn_any_from_type_code(CORBA::TypeCode_ptr type) {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  void* _ptrToObjRef(const char* repoId) {
    if( omni::ptrStrMatch(repoId, DynamicAny::DynAnyFactory::_PD_repoId) )
      return (DynamicAny::DynAnyFactory_ptr) this;
  
    if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
      return (CORBA::Object_ptr) this;

    return 0;
  }
};

OMNI_NAMESPACE_END(omni)

DynamicAny::DynAnyFactory_ptr
DynamicAny::DynAnyFactory::_nil()
{
  static omniNilDynAnyFactory* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniNilDynAnyFactory;
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}
