// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_DynAny.h             Created on: 2001/08/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//    CORBA::DynAny
//

/*
  $Log$
  Revision 1.1.2.1  2001/08/17 13:39:45  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
////////////////////////// DynAny Interface //////////////////////////
//////////////////////////////////////////////////////////////////////

class DynAny;
typedef DynAny* DynAny_ptr;
typedef DynAny_ptr DynAnyRef;

class DynEnum;
typedef DynEnum* DynEnum_ptr;
typedef DynEnum_ptr DynEnumRef;

class DynStruct;
typedef DynStruct* DynStruct_ptr;
typedef DynStruct_ptr DynStructRef;

class DynUnion;
typedef DynUnion* DynUnion_ptr;
typedef DynUnion_ptr DynUnionRef;

class DynSequence;
typedef DynSequence* DynSequence_ptr;
typedef DynSequence_ptr DynSequenceRef;

class DynArray;
typedef DynArray* DynArray_ptr;
typedef DynArray_ptr DynArrayRef;


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// DynAny ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

class DynAny {
public:

  OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(Invalid, _dyn_attr)
    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(InvalidValue, _dyn_attr)
    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(TypeMismatch, _dyn_attr)
    OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(InvalidSeq, _dyn_attr)

    typedef _CORBA_Unbounded_Sequence_Octet OctetSeq;

  virtual ~DynAny();

  virtual TypeCode_ptr type() const = 0;
  virtual void assign(DynAny_ptr dyn_any) = 0;
  virtual void from_any(const Any& value) = 0;
  virtual Any* to_any() = 0;
  virtual void destroy() = 0;
  virtual DynAny_ptr copy() = 0;
  virtual void insert_boolean(Boolean value) = 0;
  virtual void insert_octet(Octet value) = 0;
  virtual void insert_char(Char value) = 0;
  virtual void insert_wchar(WChar value) = 0;
  virtual void insert_short(Short value) = 0;
  virtual void insert_ushort(UShort value) = 0;
  virtual void insert_long(Long value) = 0;
  virtual void insert_ulong(ULong value) = 0;
#ifdef HAS_LongLong
  virtual void insert_longlong(LongLong value) = 0;
  virtual void insert_ulonglong(ULongLong value) = 0;
#endif
#ifndef NO_FLOAT
  virtual void insert_float(Float value) = 0;
  virtual void insert_double(Double value) = 0;
#ifdef HAS_LongDouble
  virtual void insert_longdouble(LongDouble value) = 0;
#endif
#endif
  virtual void insert_string(const char* value) = 0;
  virtual void insert_wstring(const WChar* value) = 0;
  virtual void insert_reference(Object_ptr value) = 0;
  virtual void insert_typecode(TypeCode_ptr value) = 0;
  virtual void insert_any(const Any& value) = 0;
  virtual Boolean get_boolean() = 0;
  virtual Octet get_octet() = 0;
  virtual Char get_char() = 0;
  virtual WChar get_wchar() = 0;
  virtual Short get_short() = 0;
  virtual UShort get_ushort() = 0;
  virtual Long get_long() = 0;
  virtual ULong get_ulong() = 0;
#ifdef HAS_LongLong
  virtual LongLong get_longlong() = 0;
  virtual ULongLong get_ulonglong() = 0;
#endif
#ifndef NO_FLOAT
  virtual Float get_float() = 0;
  virtual Double get_double() = 0;
#ifdef HAS_LongDouble
  virtual LongDouble get_longdouble() = 0;
#endif
#endif
  virtual char* get_string() = 0;
  virtual WChar* get_wstring() = 0;
  virtual Object_ptr get_reference() = 0;
  virtual TypeCode_ptr get_typecode() = 0;
  virtual Any* get_any() = 0;
  virtual DynAny_ptr current_component() = 0;
  virtual Boolean next() = 0;
  virtual Boolean seek(Long index) = 0;
  virtual void rewind() = 0;

  static DynAny_ptr _duplicate(DynAny_ptr);
  static DynAny_ptr _narrow(DynAny_ptr);
  static DynAny_ptr _nil();

  virtual Boolean NP_is_nil() const = 0;
  virtual int NP_nodetype() const = 0;
  virtual void* NP_narrow() = 0;

  static inline _CORBA_Boolean PR_is_valid(DynAny_ptr p ) {
    return ((p) ? (p->pd_magic == PR_magic) : 1);
  }

  static _dyn_attr const _CORBA_ULong PR_magic;
protected:
  DynAny();

private:
  _CORBA_ULong pd_magic;

  DynAny(const DynAny&);
  DynAny& operator=(const DynAny&);
};

#if 0
// DynFixed - not yet implemented.
class DynFixed :  public virtual DynAny {
public:

  virtual OctetSeq* get_value() = 0;
  virtual void set_value(const OctetSeq& val) = 0;

  static DynFixed_ptr _duplicate(DynFixed_ptr);
  static DynFixed_ptr _narrow(DynAny_ptr);
  static DynFixed_ptr _nil();

  virtual ~DynFixed();

protected:
  DynFixed() {}

private:
  DynFixed(const DynFixed&);
  DynFixed& operator=(const DynFixed&);
};
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////////////// DynEnum //////////////////////////////
//////////////////////////////////////////////////////////////////////

class DynEnum :  public virtual DynAny {
public:

  virtual char* value_as_string() = 0;
  virtual void value_as_string(const char* value) = 0;
  virtual ULong value_as_ulong() = 0;
  virtual void value_as_ulong(ULong value) = 0;

  static DynEnum_ptr _duplicate(DynEnum_ptr);
  static DynEnum_ptr _narrow(DynAny_ptr);
  static DynEnum_ptr _nil();

  virtual ~DynEnum();

protected:
  DynEnum() {}

private:
  DynEnum(const DynEnum&);
  DynEnum& operator=(const DynEnum&);
};

//////////////////////////////////////////////////////////////////////
////////////////////////////// DynStruct /////////////////////////////
//////////////////////////////////////////////////////////////////////

typedef char* FieldName;
typedef String_var FieldName_var;

struct NameValuePair {
  String_member id;
  Any value;
};

typedef _CORBA_ConstrType_Variable_Var<NameValuePair> NameValuePair_var;
typedef _CORBA_PseudoValue_Sequence<NameValuePair> NameValuePairSeq;

class DynStruct :  public virtual DynAny {
public:

  virtual char*  current_member_name() = 0;
  virtual TCKind current_member_kind() = 0;
  virtual NameValuePairSeq* get_members() = 0;
  virtual void set_members(const NameValuePairSeq& NVSeqVal) = 0;

  static DynStruct_ptr _duplicate(DynStruct_ptr);
  static DynStruct_ptr _narrow(DynAny_ptr);
  static DynStruct_ptr _nil();

  virtual ~DynStruct();

protected:
  DynStruct() {}

private:
  DynStruct(const DynStruct&);
  DynStruct& operator=(const DynStruct&);
};

//////////////////////////////////////////////////////////////////////
////////////////////////////// DynUnion //////////////////////////////
//////////////////////////////////////////////////////////////////////

class DynUnion :  public virtual DynAny {
public:
  virtual Boolean set_as_default() = 0;
  virtual void set_as_default(Boolean value) = 0;
  virtual DynAny_ptr discriminator() = 0;
  virtual TCKind discriminator_kind() = 0;
  virtual DynAny_ptr member() = 0;
  virtual char*  member_name() = 0;
  virtual void member_name(const char* value) = 0;
  virtual TCKind member_kind() = 0;

  static DynUnion_ptr _duplicate(DynUnion_ptr);
  static DynUnion_ptr _narrow(DynAny_ptr);
  static DynUnion_ptr _nil();

  virtual ~DynUnion();

protected:
  DynUnion() {}

private:
  DynUnion(const DynUnion&);
  DynUnion& operator=(const DynUnion&);
};

//////////////////////////////////////////////////////////////////////
///////////////////////////// DynSequence ////////////////////////////
//////////////////////////////////////////////////////////////////////

typedef _CORBA_PseudoValue_Sequence<Any> AnySeq;

class DynSequence :  public virtual DynAny {
public:

  virtual ULong length() = 0;
  virtual void length (ULong value) = 0;
  virtual AnySeq* get_elements() = 0;
  virtual void set_elements(const AnySeq& value) = 0;

  static DynSequence_ptr _duplicate(DynSequence_ptr);
  static DynSequence_ptr _narrow(DynAny_ptr);
  static DynSequence_ptr _nil();

  virtual ~DynSequence();

protected:
  DynSequence() {}

private:
  DynSequence(const DynSequence&);
  DynSequence& operator=(const DynSequence&);
};

//////////////////////////////////////////////////////////////////////
////////////////////////////// DynArray //////////////////////////////
//////////////////////////////////////////////////////////////////////

class DynArray : public virtual DynAny {
public:

  virtual AnySeq* get_elements() = 0;
  virtual void set_elements(const AnySeq& value) = 0;

  static DynArray_ptr _duplicate(DynArray_ptr);
  static DynArray_ptr _narrow(DynAny_ptr);
  static DynArray_ptr _nil();

  virtual ~DynArray();

protected:
  DynArray() {}

private:
  DynArray(const DynArray&);
  DynArray& operator=(const DynArray&);
};
