# -*- python -*-
#                           Package   : omniidl
# value.py                  Created on: 2003/09/29
#			    Author    : Duncan Grisby
#
#    Copyright (C) 2003 Apasphere Ltd.
#
#  This file is part of omniidl.
#
#  omniidl is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA.
#
# Description:
#  Implementation of ValueType / ValueBox mapping
#

import string
from omniidl import idlast, idltype
from omniidl_be.cxx import mapping, call, output, config, skutil
from omniidl_be.cxx import id, types, iface, cxx, ast, util

#
# Templates
#
value_forward = """\
#ifndef __@guard@__
#define __@guard@__

class @name@;
typedef _CORBA_Value_Var<@name@>     @name@_var;
typedef _CORBA_Value_Member<@name@>  @name@_member;
typedef _CORBA_Value_OUT_arg<@name@> @name@_out;

#endif // __@guard@__
"""

value_class = """\
class @name@ : 
  @inherits@
{
public:  
  // Standard mapping
  typedef @name@*    _ptr_type;
  typedef @name@_var _var_type;

  static _ptr_type _downcast (CORBA::ValueBase*);
  @downcast_abstract@

#ifdef OMNI_HAVE_COVARIANT_RETURNS
  virtual @name@* _copy_value();
#else
  virtual CORBA::ValueBase* _copy_value();
#endif

  // Definitions in this scope
  @other_idl@

  // Operations and attributes
  @operations@

  // Accessors for public members
  @public_accessors@

protected:
  // Accessors for private members
  @private_accessors@

public:
  // omniORB internal
  virtual const char* _NP_repositoryId() const;
  virtual const char* _NP_repositoryId(CORBA::ULong& _hashval) const;

  virtual const _omni_ValueIds* _NP_truncatableIds() const;

  virtual CORBA::Boolean _NP_custom() const;

  virtual void* _ptrToValue(const char* id);

  static void _NP_marshal(@name@*, cdrStream&);
  static @name@* _NP_unmarshal(cdrStream&);
  @np_to_value@

  virtual void _PR_marshal_state(cdrStream&) const;
  virtual void _PR_unmarshal_state(cdrStream&);
  virtual void _PR_copy_state(@name@*);

  static _core_attr const char* _PD_repoId;

protected:
  @name@();
  virtual ~@name@();

private:
  // Not implemented
  @name@(const @name@ &);
  void operator=(const @name@ &);
};
"""

value_class_downcast_abstract = """\
static _ptr_type _downcast (CORBA::AbstractBase*);"""

value_class_np_to_value = """\
virtual CORBA::ValueBase* _NP_to_value();"""


valuefactory_class_initialisers = """\
class @name@_init : public CORBA::ValueFactoryBase
{
public:
  virtual ~@name@_init();

  @factory_funcs@

  static @name@_init* _downcast(CORBA::ValueFactory _v);
  virtual void* _ptrToFactory(const char* _id);
protected:
  @name@_init();
};
"""

valuefactory_class_no_operations = """\
class @name@_init : public CORBA::ValueFactoryBase
{
public:
  @name@_init();
  virtual ~@name@_init();

  virtual CORBA::ValueBase* create_for_unmarshal();

  static @name@_init* _downcast(CORBA::ValueFactory _v);
  virtual void* _ptrToFactory(const char* _id);
};
"""


value_poa_class = """\
class @poa_name@ :
  public virtual @intf_poa@,
  public virtual @value_name@
{
public:
  virtual ~@poa_name@();
};
"""


value_obv_class = """\
class @obv_name@ :
  @inherits@
{
@constructor_access@:
  @obv_name@();
  @init_constructor@
  virtual ~@obv_name@();

public:
  @public_accessors@

protected:
  @private_accessors@

private:
  @state_holders@
};
"""

value_functions = """\
// valuetype @name@

@fqname@*
@fqname@::_downcast(CORBA::ValueBase* _b)
{
  return _b ? (@fqname@*)_b->_ptrToValue(_PD_repoId) : 0;
}

const char* @fqname@::_PD_repoId = "@repoId@";

const char*
@fqname@::_NP_repositoryId() const
{
  return _PD_repoId;
}

const char*
@fqname@::_NP_repositoryId(CORBA::ULong& hash) const
{
  hash = @idhash@U;
  return _PD_repoId;
}

CORBA::Boolean
@fqname@::_NP_custom() const
{
  return @custom@;
}

void*
@fqname@::_ptrToValue(const char* _id)
{
  if (_id == ::@fqname@::_PD_repoId)
    return (::@fqname@*) this;
  
  @ptrToValuePtr@

  if (_id == CORBA::ValueBase::_PD_repoId)
    return (CORBA::ValueBase*) this;
  
  if (omni::strMatch(_id, ::@fqname@::_PD_repoId))
    return (::@fqname@*) this;
  
  @ptrToValueStr@

  if (omni::strMatch(_id, CORBA::ValueBase::_PD_repoId))
    return (CORBA::ValueBase*) this;

  return 0;
}

void
@fqname@::_NP_marshal(@fqname@* _v, cdrStream& _0s)
{
  omniValueType::marshal(_v, @fqname@::_PD_repoId, _0s);
}

@fqname@*
@fqname@::_NP_unmarshal(cdrStream& _0s)
{
  CORBA::ValueBase* _b = omniValueType::unmarshal(@fqname@::_PD_repoId,
						  @idhash@U, 0, _0s);
  @fqname@* _d = @fqname@::_downcast(_b);
  if (_b && !_d) {
    _b = omniValueType::handleIncompatibleValue(
           @fqname@::_PD_repoId,
           @idhash@U, _b, (CORBA::CompletionStatus)_0s.completion());
    _d = @fqname@::_downcast(_b);
  }
  return _d;
}

void
@fqname@::_PR_marshal_state(cdrStream& _0s) const
{
  @marshal_members@
}

void
@fqname@::_PR_unmarshal_state(cdrStream& _0s)
{
  @unmarshal_members@
}

#ifdef OMNI_HAVE_COVARIANT_RETURNS
@fqname@*
#else
CORBA::ValueBase*
#endif
@fqname@::_copy_value()
{
  CORBA::ValueBase* _b;
  _b = _omni_ValueFactoryManager::create_for_unmarshal(_PD_repoId, @idhash@U);
  @fqname@* _v = @fqname@::_downcast(_b);
  if (!_v) {
    CORBA::remove_ref(_b);
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_ValueFactoryFailure,
		  CORBA::COMPLETED_NO);
  }
  _v->_PR_copy_state(this);
  return _v;
}

void
@fqname@::_PR_copy_state(@fqname@* _v)
{
  @copy_members@
}


@fqname@::@name@()  {}
@fqname@::~@name@() {}
"""

value_functions_abstract = """\
@fqname@*
@fqname@::_downcast(CORBA::AbstractBase* _a)
{
  return _downcast(_a->_NP_to_value());
}

CORBA::ValueBase*
@fqname@::_NP_to_value()
{
  return (CORBA::ValueBase*)this;
}
"""

value_poa_functions = """\
POA_@fqname@::~@name@() {}
"""

value_obv_functions = """\
OBV_@fqname@::@name@() {}
OBV_@fqname@::~@name@() {}
"""

value_obv_init_function = """\
OBV_@fqname@::@name@(@init_params@)
@base_init@
{
  @member_initialisers@
}
"""

value_ptrToValuePtr = """\
if (_id == @iname@::_PD_repoId)
  return (@iname@*) this;
"""

value_ptrToValueStr = """\
if (omni::strMatch(_id, @iname@::_PD_repoId))
  return (@iname@*) this;
"""


value_no_truncatable_bases = """\
const _omni_ValueIds*
@fqname@::_NP_truncatableIds() const
{
  return 0;
}
"""

value_truncatable_bases = """\

static _omni_ValueId _0RL_@flatname@_baseIds[] = {
  @baseids@
};

static _omni_ValueIds _0RL_@flatname@_bases = {
  @basecount@, @baseshash@U, _0RL_@flatname@_baseIds
};

const _omni_ValueIds*
@fqname@::_NP_truncatableIds() const
{
  return &_0RL_@flatname@_bases;
}
"""

valuefactory_functions = """\
@fqname@_init::@name@_init()  {}
@fqname@_init::~@name@_init() {}

@fqname@_init*
@fqname@_init::_downcast(CORBA::ValueFactory _v)
{
  return _v ? (::@fqname@_init*)_v->_ptrToFactory(::@fqname@::_PD_repoId) : 0;
}

void*
@fqname@_init::_ptrToFactory(const char* _id)
{
  if (_id == ::@fqname@::_PD_repoId)
    return (::@fqname@_init*) this;
  
  if (_id == CORBA::ValueBase::_PD_repoId)
    return (CORBA::ValueFactoryBase*) this;
  
  if (omni::strMatch(_id, ::@fqname@::_PD_repoId))
    return (::@fqname@_init*) this;
  
  if (omni::strMatch(_id, CORBA::ValueBase::_PD_repoId))
    return (CORBA::ValueFactoryBase*) this;

  return 0;
}
"""

valuefactory_create_for_unmarshal = """\
CORBA::ValueBase*
@fqname@_init::create_for_unmarshal()
{
  return new OBV_@fqname@();
}
"""

statemember_copy = """\
@name@(_v->@name@());"""

statemember_copy_value = """\
#ifdef OMNI_HAVE_COVARIANT_RETURNS
@name@(_v->@name@()->_copy_value());
#else
CORBA::ValueBase* _0v_@name@ = _v->@name@()->_copy_value();
@name@(@type@::_downcast(_0v_@name@));
#endif"""

statemember_init = """\
_pd_@name@ = _@name@;"""

statemember_array_declarator = """\
typedef @memtype@ @prefix@_@name@@dims@;
typedef @memtype@ _@name@_slice@tail_dims@;
"""

statemember_array_sig = """\
virtual const @memtype@_slice* @name@ () const @abs@;
virtual @memtype@_slice* @name@ () @abs@;
virtual void @name@ (const @const_type@ _value) @abs@;
"""

statemember_array_impl = """\
const @memtype@_slice*
OBV_@value_name@::@name@() const
{
  return _pd_@name@;
}

@memtype@_slice*
OBV_@value_name@::@name@()
{
  return _pd_@name@;
}

void
OBV_@value_name@::@name@(const @const_type@ _value)
{
  @array_copy@
}
"""

statemember_any_sig = """\
virtual const @type@& @name@ () const @abs@;
virtual @type@& @name@() @abs@;
virtual void @name@(const @type@& _value) @abs@;
"""

statemember_any_impl = """\
const @type@&
OBV_@value_name@::@name@() const
{
  return _pd_@name@;
}

@type@&
OBV_@value_name@::@name@()
{
  return _pd_@name@;
}

void
OBV_@value_name@::@name@(const @type@& _value)
{
  _pd_@name@ = _value;
}
"""

statemember_any_marshal = """\
(const @type@&) @name@() >>= _0s;"""

statemember_any_unmarshal = """\
@type@& _@name@ = @name@();
_@name@ <<= _0s;
"""

statemember_typecode_sig = """\
virtual CORBA::TypeCode_ptr @name@() const @abs@;
virtual void @name@(CORBA::TypeCode_ptr _value) @abs@;
"""

statemember_typecode_member = """\
void @name@(const CORBA::TypeCode_member& _value) {
  @name@(CORBA::TypeCode::_duplicate(_value));
}
"""

statemember_typecode_impl = """\
CORBA::TypeCode_ptr
OBV_@value_name@::@name@() const
{
  return _pd_@name@.in();
}

void
OBV_@value_name@::@name@(CORBA::TypeCode_ptr _value)
{
  _pd_@name@ = CORBA::TypeCode::_duplicate(_value);
}
"""

statemember_typecode_marshal = """\
CORBA::TypeCode::marshalTypeCode(@name@(), _0s);
"""

statemember_typecode_unmarshal = """\
CORBA::TypeCode_var _@name@;
_@name@ = CORBA::TypeCode::unmarshalTypeCode(_0s);
@name@(_@name@);
"""

statemember_typecode_init = """\
_pd_@name@ = CORBA::TypeCode::_duplicate(_@name@);"""

statemember_basic_sig = """\
virtual @type@ @name@() const @abs@;
virtual void @name@(@type@ _value) @abs@;
"""

statemember_basic_impl = """\
@type@
OBV_@value_name@::@name@() const
{
  return _pd_@name@;
}

void
OBV_@value_name@::@name@(@type@ _value)
{
  _pd_@name@ = _value;
}
"""

statemember_basic_marshal = """\
@name@() >>= _0s;"""

statemember_basic_unmarshal = """\
@type@ _@name@;
_@name@ <<= _0s;
@name@(_@name@);
"""

statemember_basic_kind_marshal = """\
_0s.marshal@kind@(@name@());"""

statemember_basic_kind_unmarshal = """\
@type@ _@name@;
_@name@ = _0s.unmarshal@kind@();
@name@(_@name@);
"""

statemember_string_sig = """\
virtual const char* @name@() const @abs@;
virtual void @name@(char* _value) @abs@;
virtual void @name@(const char* _value) @abs@;
virtual void @name@(const CORBA::String_var& _value) @abs@;
"""

statemember_string_member = """\
void @name@(const CORBA::String_member& _value) {
  @name@((const char*)_value);
}
"""

statemember_string_impl = """\
const char*
OBV_@value_name@::@name@() const
{
  return (const char*)_pd_@name@;
}

void
OBV_@value_name@::@name@(char* _value)
{
  _pd_@name@ = _value;
}

void
OBV_@value_name@::@name@(const char* _value)
{
  _pd_@name@ = _value;
}

void
OBV_@value_name@::@name@(const CORBA::String_var& _value)
{
  _pd_@name@ = _value;
}
"""

statemember_string_marshal = """\
_0s.marshalString(@name@());"""

statemember_string_unmarshal = """\
char* _@name@;
_@name@ = _0s.unmarshalString();
@name@(_@name@);
"""

statemember_wstring_sig = """\
virtual const CORBA::WChar* @name@() const @abs@;
virtual void @name@(CORBA::WChar* _value) @abs@;
virtual void @name@(const CORBA::WChar* _value) @abs@;
virtual void @name@(const CORBA::WString_var& _value) @abs@;
"""

statemember_wstring_member = """\
void @name@(const CORBA::WString_member& _value) {
  @name@((const CORBA::WChar*)_value);
}
"""

statemember_wstring_impl = """\
const CORBA::WChar*
OBV_@value_name@::@name@() const
{
  return (const CORBA::WChar*)_pd_@name@;
}

void
OBV_@value_name@::@name@(CORBA::WChar* _value)
{
  _pd_@name@ = _value;
}

void
OBV_@value_name@::@name@(const CORBA::WChar* _value)
{
  _pd_@name@ = _value;
}

void
OBV_@value_name@::@name@(const CORBA::WString_var& _value)
{
  _pd_@name@ = _value;
}
"""

statemember_wstring_marshal = """\
_0s.marshalWString(@name@());"""

statemember_wstring_unmarshal = """\
CORBA::WChar* _@name@;
_@name@ = _0s.unmarshalWString();
@name@(_@name@);
"""

statemember_objref_sig = """\
virtual @ptr_type@ @name@() const @abs@;
virtual void @name@(@ptr_type@ _value) @abs@;
virtual void @name@(const @var_type@& _value) @abs@;
"""

statemember_objref_member = """\
void @name@(const @memtype@& _value) {
  @helper@::duplicate(_value);
  @name@((@ptr_type@)_value_);
}
"""

statemember_objref_impl = """\
@ptr_type@
OBV_@value_name@::@name@() const
{
  return _pd_@name@.in();
}

void
OBV_@value_name@::@name@(@ptr_type@ _value)
{
  _pd_@name@ = @type@::_duplicate(_value);
}

void
OBV_@value_name@::@name@(const @var_type@& _value)
{
  _pd_@name@ = _value;
}
"""

statemember_objref_marshal = """\
@type@::_marshalObjRef(@name@(), _0s);"""

statemember_objref_unmarshal = """\
@var_type@ _@name@;
_@name@ = @type@::_unmarshalObjRef(_0s);
@name@(_@name@);
"""

statemember_objref_init = """\
_pd_@name@ = @type@::_duplicate(_@name@);"""

statemember_constructed_sig = """\
virtual const @type@& @name@() const @abs@;
virtual @type@& @name@() @abs@;
virtual void @name@(const @type@& _value) @abs@;
"""

statemember_constructed_impl = """\
const @type@&
OBV_@value_name@::@name@() const
{
  return _pd_@name@;
}

@type@&
OBV_@value_name@::@name@()
{
  return _pd_@name@;
}

void
OBV_@value_name@::@name@(const @type@& _value)
{
  _pd_@name@ = _value;
}
"""

statemember_constructed_marshal = """\
@name@() >>= _0s;"""

statemember_constructed_unmarshal = """\
@type@& _@name@ = @name@();
_@name@ <<= _0s;
"""

statemember_sequence_typedef = """\
typedef @sequence_template@ _@name@_seq;"""

statemember_sequence_sig = """\
virtual const _@name@_seq& @name@() const @abs@;
virtual _@name@_seq& @name@() @abs@;
virtual void @name@(const _@name@_seq& _value) @abs@;
"""

statemember_sequence_impl = """\
const @value_name@::_@name@_seq&
OBV_@value_name@::@name@() const
{
  return _pd_@name@;
}

@value_name@::_@name@_seq&
OBV_@value_name@::@name@()
{
  return _pd_@name@;
}

void
OBV_@value_name@::@name@(const @value_name@::_@name@_seq& _value)
{
  _pd_@name@ = _value;
}
"""

statemember_sequence_marshal = """\
(@sequence_template@&) @name@() >>= _0s;"""

statemember_sequence_unmarshal = """\
@sequence_template@& _@name@ = @name@();
_@name@ <<= _0s;
"""

statemember_value_sig = """\
virtual @type@* @name@() const @abs@;
virtual void @name@(@type@* _value) @abs@;
"""

statemember_value_impl = """\
@type@*
OBV_@value_name@::@name@() const
{
  return _pd_@name@.in();
}

void
OBV_@value_name@::@name@(@type@* _value)
{
  CORBA::add_ref(_value);
  _pd_@name@ = _value;
}
"""

statemember_value_marshal = """\
@type@::_NP_marshal(@name@(), _0s);"""

statemember_value_unmarshal = """\
@type@_var _@name@;
_@name@ = @type@::_NP_unmarshal(_0s);
@name@(_@name@);
"""

valuebox_class = """\
class @name@ :
  public CORBA::DefaultValueRefCountBase
{
public:
  typedef @name@_var _var_type;

  @member_funcs@

  static @name@* _downcast(ValueBase*);

#ifdef OMNI_HAVE_COVARIANT_RETURNS
  virtual @name@* _copy_value();
#else
  virtual CORBA::ValueBase* _copy_value();
#endif

  // omniORB internal
  virtual const char* _NP_repositoryId() const;
  virtual const char* _NP_repositoryId(CORBA::ULong& _hashval) const;

  virtual const _omni_ValueIds* _NP_truncatableIds() const;

  virtual CORBA::Boolean _NP_custom() const;
  virtual CORBA::Boolean _NP_box() const;

  virtual void* _ptrToValue(const char* id);

  static void _NP_marshal(@name@*, cdrStream&);
  static @name@* _NP_unmarshal(cdrStream&);

  virtual void _PR_marshal_state(cdrStream&) const;
  virtual void _PR_unmarshal_state(cdrStream&);
  virtual void _PR_copy_state(@name@*);

  static _core_attr const char* _PD_repoId;

protected:
  ~@name@();
private:
  void operator=(const @name@&);

  @boxed_member@ _pd_boxed;
};
"""

valuebox_member_funcs_basic = """\
inline @name@() {}
inline @name@(@boxedtype@ _v) : _pd_boxed(_v) {}
inline @name@(const @name@& _v) : _pd_boxed(_v._pd_boxed) {}

inline @name@& operator=(@boxedtype@ _v) {
  _pd_boxed = _v;
  return *this;
}

inline @boxedtype@ _value() const {
  return _pd_boxed;
}
inline void _value(@boxedtype@ _v) {
  _pd_boxed = _v;
}

inline @boxedtype@ _boxed_in() const {
  return _pd_boxed;
}
inline @boxedtype@& _boxed_inout() {
  return _pd_boxed;
}
inline @boxedtype@& _boxed_out() {
  return _pd_boxed;
}
"""

valuebox_member_funcs_string = """\
inline @name@()
  : _pd_boxed(OMNI_CONST_CAST(char*,_CORBA_String_helper::empty_string))
{}
inline @name@(const @name@& _v) {
  if (_v._pd_boxed) _pd_boxed = _CORBA_String_helper::dup(_v._pd_boxed);
  else _pd_boxed = 0;
}
inline @name@(char* _v) {
  _pd_boxed = _v;
}
inline @name@(const char* _v) {
  if (_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
}
inline @name@(const CORBA::String_var& _v) {
  if ((const char*)_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
}
inline @name@(const CORBA::String_member& _v) {
  if ((const char*)_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
}
inline @name@(const _CORBA_String_element& _v) {
  if ((const char*)_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
}

inline @name@& operator=(char* _v) {
  _CORBA_String_helper::free(_pd_boxed);
  _pd_boxed = _v;
  return *this;
}
inline @name@& operator=(const char* _v) {
  _CORBA_String_helper::free(_pd_boxed);
  if (_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
  return *this;
}
inline @name@& operator=(const CORBA::String_var& _v) {
  _CORBA_String_helper::free(_pd_boxed);
  if ((const char*)_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
  return *this;
}
inline @name@& operator=(const CORBA::String_member& _v) {
  _CORBA_String_helper::free(_pd_boxed);
  if ((const char*)_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
  return *this;
}
inline @name@& operator=(const _CORBA_String_element& _v) {
  _CORBA_String_helper::free(_pd_boxed);
  if ((const char*)_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
  return *this;
}

inline const char* _value() const {
  return _pd_boxed;
}
inline void _value(char* _v) {
  _CORBA_String_helper::free(_pd_boxed);
  _pd_boxed = _v;
}
inline void _value(const char* _v) {
  _CORBA_String_helper::free(_pd_boxed);
  if (_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
}
inline void _value(const CORBA::String_var& _v) {
  _CORBA_String_helper::free(_pd_boxed);
  if ((const char*)_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
}
inline void _value(const CORBA::String_member& _v) {
  _CORBA_String_helper::free(_pd_boxed);
  if ((const char*)_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
}
inline void _value(const _CORBA_String_element& _v) {
  _CORBA_String_helper::free(_pd_boxed);
  if ((const char*)_v) _pd_boxed = _CORBA_String_helper::dup(_v);
  else _pd_boxed = 0;
}

inline char& operator[] (_CORBA_ULong _i) {
  if (!_pd_boxed) {
    _CORBA_bound_check_error();	// never return
  }
  return _pd_boxed[_i];
}
inline char operator[] (_CORBA_ULong _i) const {
  if (!_pd_boxed) {
    _CORBA_bound_check_error();	// never return
  }
  return _pd_boxed[_i];
}

inline const char* _boxed_in() const {
  return _pd_boxed;
}
inline char*& _boxed_inout() {
  return _pd_boxed;
}
inline char*& _boxed_out() {
  return _pd_boxed;
}
"""

valuebox_member_funcs_wstring = """\
inline @name@()
  : _pd_boxed(OMNI_CONST_CAST(CORBA::WChar*,_CORBA_WString_helper::empty_wstring))
{}
inline @name@(const @name@& _v) {
  if (_v._pd_boxed) _pd_boxed = _CORBA_WString_helper::dup(_v._pd_boxed);
  else _pd_boxed = 0;
}
inline @name@(CORBA::WChar* _v) {
  _pd_boxed = _v;
}
inline @name@(const CORBA::WChar* _v) {
  if (_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
}
inline @name@(const CORBA::WString_var& _v) {
  if ((const CORBA::WChar*)_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
}
inline @name@(const CORBA::WString_member& _v) {
  if ((const CORBA::WChar*)_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
}
inline @name@(const _CORBA_WString_element& _v) {
  if ((const CORBA::WChar*)_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
}

inline @name@& operator=(CORBA::WChar* _v) {
  _CORBA_WString_helper::free(_pd_boxed);
  _pd_boxed = _v;
  return *this;
}
inline @name@& operator=(const CORBA::WChar* _v) {
  _CORBA_WString_helper::free(_pd_boxed);
  if (_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
  return *this;
}
inline @name@& operator=(const CORBA::WString_var& _v) {
  _CORBA_WString_helper::free(_pd_boxed);
  if ((const CORBA::WChar*)_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
  return *this;
}
inline @name@& operator=(const CORBA::WString_member& _v) {
  _CORBA_WString_helper::free(_pd_boxed);
  if ((const CORBA::WChar*)_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
  return *this;
}
inline @name@& operator=(const _CORBA_WString_element& _v) {
  _CORBA_WString_helper::free(_pd_boxed);
  if ((const CORBA::WChar*)_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
  return *this;
}

inline const CORBA::WChar* _value() const {
  return _pd_boxed;
}
inline void _value(CORBA::WChar* _v) {
  _CORBA_WString_helper::free(_pd_boxed);
  _pd_boxed = _v;
}
inline void _value(const CORBA::WChar* _v) {
  _CORBA_WString_helper::free(_pd_boxed);
  if (_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
}
inline void _value(const CORBA::WString_var& _v) {
  _CORBA_WString_helper::free(_pd_boxed);
  if ((const CORBA::WChar*)_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
}
inline void _value(const CORBA::WString_member& _v) {
  _CORBA_WString_helper::free(_pd_boxed);
  if ((const CORBA::WChar*)_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
}
inline void _value(const _CORBA_WString_element& _v) {
  _CORBA_WString_helper::free(_pd_boxed);
  if ((const CORBA::WChar*)_v) _pd_boxed = _CORBA_WString_helper::dup(_v);
  else _pd_boxed = 0;
}

inline CORBA::WChar& operator[] (_CORBA_ULong _i) {
  if (!_pd_boxed) {
    _CORBA_bound_check_error();	// never return
  }
  return _pd_boxed[_i];
}
inline CORBA::WChar operator[] (_CORBA_ULong _i) const {
  if (!_pd_boxed) {
    _CORBA_bound_check_error();	// never return
  }
  return _pd_boxed[_i];
}

inline const CORBA::WChar* _boxed_in() const {
  return _pd_boxed;
}
inline CORBA::WChar*& _boxed_inout() {
  return _pd_boxed;
}
inline CORBA::WChar*& _boxed_out() {
  return _pd_boxed;
}
"""

valuebox_member_funcs_objref = """\
inline @name@() {}
inline @name@(@boxedif@_ptr _v) {
  _pd_boxed = @boxedif@::_duplicate(_v);
}
inline @name@(const @name@& _v) {
  _pd_boxed = @boxedif@::_duplicate(_v._value());
}
inline @name@& operator=(@boxedif@_ptr _v) {
  _pd_boxed = @boxedif@::_duplicate(_v);
  return *this;
}

inline @boxedif@_ptr _value() const {
  return _pd_boxed.in();
}
inline void _value(@boxedif@_ptr _v) {
  _pd_boxed = @boxedif@::_duplicate(_v);
}

inline @boxedif@_ptr _boxed_in() const {
  return _pd_boxed.in();
}
inline @boxedif@_ptr& _boxed_inout() {
  return _pd_boxed.inout();
}
inline @boxedif@_ptr& _boxed_out() {
  return _pd_boxed.out();
}
"""

valuebox_member_funcs_any = """\
inline @name@() {}
inline @name@(const @boxedtype@& _v) {
  _pd_boxed = new @boxedtype@(_v);
}
inline @name@(const @name@& _v) {
  _pd_boxed = new @boxedtype@(_v._pd_boxed.in());
}
inline @name@(CORBA::TypeCode_ptr tc, void* value, Boolean release = 0) {
  _pd_boxed = new @boxedtype@(tc, value, release);
}

inline @name@& operator=(const @boxedtype@& _v) {
  _pd_boxed = new @boxedtype@(_v);
  return *this;
}

inline const @boxedtype@& _value() const {
  return _pd_boxed.in();
}
inline @boxedtype@& _value() {
  return _pd_boxed.inout();
}
inline void _value(const @boxedtype@& _v) {
  _pd_boxed = new @boxedtype@(_v);
}

inline const @boxedtype@& _boxed_in() const {
  return _pd_boxed.in();
}
inline @boxedtype@& _boxed_inout() {
  return _pd_boxed.inout();
}
inline @boxedtype@_out _boxed_out() {
  return _pd_boxed.out();
}

CORBA::TypeCode_ptr type() const {
  return _pd_boxed->type();
}
void type(CORBA::TypeCode_ptr _t) {
  _pd_boxed->type(t);
}
"""

valuebox_member_funcs_sequence = """\
inline @name@() {
  _pd_boxed = new @boxedtype@;
}
inline @name@(const @boxedtype@& _v) {
  _pd_boxed = new @boxedtype@(_v);
}
inline @name@(const @name@& _v) {
  _pd_boxed = new @boxedtype@(_v._pd_boxed.in());
}
@sequence_constructors@

inline @name@& operator=(const @boxedtype@& _v) {
  _pd_boxed = new @boxedtype@(_v);
  return *this;
}

inline const @boxedtype@& _value() const {
  return _pd_boxed.in();
}
inline @boxedtype@& _value() {
  return _pd_boxed.inout();
}
inline void _value(const @boxedtype@& _v) {
  _pd_boxed = new @boxedtype@(_v);
}

inline const @boxedtype@& _boxed_in() const {
  return _pd_boxed.in();
}
inline @boxedtype@& _boxed_inout() {
  return _pd_boxed.inout();
}
inline @boxedtype@_out _boxed_out() {
  return _pd_boxed.out();
}

inline CORBA::ULong maximum() const {
  return _pd_boxed->maximum();
}
inline CORBA::ULong length() const {
  return _pd_boxed->length();
}
inline void length(CORBA::ULong _len) {
  _pd_boxed->length(_len);
}

inline @subscript_element@ operator[](CORBA::ULong _i) {
  return _pd_boxed[_i];
}
"""

valuebox_sequence_constructors_unbounded = """\
inline @name@(CORBA::ULong _max) {
  _pd_boxed = new @boxedtype@(_max);
}
inline @name@(CORBA::ULong _max, CORBA::ULong _len, @element@* _val, CORBA::Boolean _rel=0) {
  _pd_boxed = new @boxedtype@(_max, _len, _val, _rel);
}
"""

valuebox_sequence_constructors_bounded = """\
inline @name@(CORBA::ULong _len, @element@* _val, CORBA::Boolean _rel=0) {
  _pd_boxed = new @boxedtype@(_len, _val, _rel);
}
"""

valuebox_member_funcs_fixed = """\
inline @name@(const @boxedtype@& _v) {}
inline @name@(const @name@& _v) {
  _pd_boxed = _v._pd_boxed;
}

inline @name@(int _v = 0)           : _pd_boxed(_v) {}
inline @name@(unsigned _v)          : _pd_boxed(_v) {}

#ifndef OMNI_LONG_IS_INT
inline @name@(CORBA::Long _v)       : _pd_boxed(_v) {}
inline @name@(CORBA::ULong _v)      : _pd_boxed(_v) {}
#endif
#ifdef HAS_LongLong
inline @name@(CORBA::LongLong _v)   : _pd_boxed(_v) {}
inline @name@(CORBA::ULongLong _v)  : _pd_boxed(_v) {}
#endif
#ifndef NO_FLOAT
inline @name@(CORBA::Double _v)     : _pd_boxed(_v) {}
#endif
#ifdef HAS_LongDouble
inline @name@(CORBA::LongDouble _v) : _pd_boxed(_v) {}
#endif
inline @name@(const char* _v)       : _pd_boxed(_v) {}

inline @name@& operator=(const @boxedtype@& _v) {
  _pd_boxed = _v;
  return *this;
}

inline const @boxedtype@& _value() const {
  return _pd_boxed;
}
inline @boxedtype@& _value() {
  return _pd_boxed;
}
inline void _value(const @boxedtype@& _v) {
  _pd_boxed = _v;
}

inline const @boxedtype@& _boxed_in() const {
  return _pd_boxed;
}
inline @boxedtype@& _boxed_inout() {
  return _pd_boxed;
}
inline @boxedtype@_out _boxed_out() {
  return _pd_boxed;
}

CORBA::Fixed round   (CORBA::UShort scale) const {
  return _pd_boxed.round(scale);
}
CORBA::Fixed truncate(CORBA::UShort scale) const {
  return _pd_boxed.truncate(scale);
}
CORBA::UShort fixed_digits() const {
  return pd_boxed.fixed_digits();
}
CORBA::UShort fixed_scale() const {
  return pd_boxed.fixed_scale();
}
char* to_string() const {
  return pd_boxed.to_string();
}
"""


valuebox_member_funcs_array = """\
inline @name@() {
  _pd_boxed = 0;
}
inline @name@(const @boxedtype@ _v) {
  _pd_boxed = @helper@::dup(_v);
  if (!_pd_boxed) _CORBA_new_operator_return_null();
}
inline @name@(const @name@& _v) {
  if (!_v._pd_boxed) {
    _pd_boxed = 0;
  }
  else {
    _pd_boxed = @helper@::dup(_v._pd_boxed);
    if (!_pd_boxed) _CORBA_new_operator_return_null();
  }
}

inline @name@& operator=(const @boxedtype@ _v) {
  if (_pd_boxed)
    @helper@::free(_pd_boxed);
  _pd_boxed = @helper@::dup(_v);
  if (!_pd_boxed) _CORBA_new_operator_return_null();
  return *this;
}

inline const @slice@* _value() const {
  return (const @slice@*)_pd_boxed;
}
inline @slice@* _value() {
  return _pd_boxed;
}

inline void _value(const @boxedtype@ _v) {
  if (_pd_boxed)
    @helper@::free(_pd_boxed);
  _pd_boxed = @helper@::dup(_v);
  if (!_pd_boxed) _CORBA_new_operator_return_null();
}

inline @slice@& operator[] (_CORBA_ULong index_) {
  return *(_pd_boxed + index_);
}
inline const @slice@& operator[] (_CORBA_ULong index_) const {
  return *((const @slice@*) (_pd_boxed + index_));
}

const @slice@* _boxed_in() const {
  return (const @slice@*)_pd_boxed;
}
@slice@* _boxed_inout() {
  return _pd_boxed;
}
"""

valuebox_member_funcs_array_fixed_out = """\
@slice@* _boxed_out() {
  return _pd_boxed;
}
"""

valuebox_member_funcs_array_variable_out = """\
@slice@*& _boxed_out() {
  if (_pd_boxed) { 
    @helper@::free(_pd_boxed); 
    _pd_boxed = 0; 
  } 
  return _pd_boxed; 
}
"""

valuebox_member_funcs_struct_union = """\
inline @name@() {
  _pd_boxed = new @boxedtype@();
};
inline @name@(const @boxedtype@& _v) {
  _pd_boxed = new @boxedtype@(_v);
};
inline @name@(const @name@& _v) {
  _pd_boxed = new @boxedtype@(_v._pd_boxed.in());
};

inline @name@& operator=(const @boxedtype@& _v) {
  _pd_boxed = new @boxedtype@(_v);
  return *this;
}

inline const @boxedtype@& _value() const {
  return _pd_boxed.in();
}
inline @boxedtype@& _value() {
  return _pd_boxed.inout();
}
inline void _value(const @boxedtype@& _v) {
  _pd_boxed = new @boxedtype@(_v);
}

inline const @boxedtype@& _boxed_in() const {
  return _pd_boxed.in();
}
inline @boxedtype@& _boxed_inout() {
  return _pd_boxed.inout();
}
inline @boxedtype@_out _boxed_out() {
  return _pd_boxed.out();
}

// Member accessors
@accessor_funcs@
"""

# Member accessors for boxed structs

valuebox_structmember_array = """\
inline const @memtype@_slice* @name@ () const
{
  return _pd_boxed->@name@;
}
inline @memtype@_slice* @name@ ()
{
  return _pd_boxed->@name@;
}
inline void @name@ (const @const_type@ _value)
{
  @array_copy@
}
"""

valuebox_structmember_any = """\
inline const @type@& @name@ () const
{
  return _pd_boxed->@name@;
}
inline @type@& @name@()
{
  return _pd_boxed->@name@;
}
inline void @name@(const @type@& _value)
{
  _pd_boxed->@name@ = _value;
}
"""

valuebox_structmember_typecode = """\
inline CORBA::TypeCode_ptr @name@() const
{
  return CORBA::TypeCode::_duplicate(_pd_boxed->@name@);
}
inline void @name@(CORBA::TypeCode_ptr _value)
{
  _pd_boxed->@name@ = CORBA::TypeCode::_duplicate(_value);
}
inline void @name@(CORBA::TypeCode_var& _value)
{
  _pd_boxed->@name@ = _value;
}
inline void @name@(CORBA::TypeCode_member& _value)
{
  _pd_boxed->@name@ = _value;
}
"""

valuebox_structmember_basic = """\
inline @type@ @name@() const
{
  return _pd_boxed->@name@;
}
inline void @name@(@type@ _value)
{
  _pd_boxed->@name@ = _value;
}
"""

valuebox_structmember_string = """\
inline const char* @name@() const
{
  return _pd_boxed->@name@;
}
inline void @name@(char* _value)
{
  _pd_boxed->@name@ = _value;
}
inline void @name@(const char* _value)
{
  _pd_boxed->@name@ = _value;
}
inline void @name@(const CORBA::String_var& _value)
{
  _pd_boxed->@name@ = _value;
}
"""

valuebox_structmember_wstring = """\
inline const CORBA::WChar* @name@() const
{
  return _pd_boxed->@name@;
}
inline void @name@(CORBA::WChar* _value)
{
  _pd_boxed->@name@ = _value;
}
inline void @name@(const CORBA::WChar* _value)
{
  _pd_boxed->@name@ = _value;
}
inline void @name@(const CORBA::WString_var& _value)
{
  _pd_boxed->@name@ = _value;
}
"""

valuebox_structmember_objref = """\
inline @ptr_type@ @name@() const
{
  return _pd_boxed->@name@.in();
}
inline void @name@(@ptr_type@ _value)
{
  _pd_boxed->@name@ = @type@::_duplicate(_value);
}
inline void @name@(const @var_type@& _value)
{
  _pd_boxed->@name@ = _value;
}
inline void @name@(const @memtype@& _value)
{
  _pd_boxed->@name@ = _value;
}
"""

valuebox_structmember_constructed = """\
inline const @type@& @name@() const
{
  return _pd_boxed->@name@;
}
inline @type@& @name@()
{
  return _pd_boxed->@name@;
}
inline void @name@(const @type@& _value)
{
  _pd_boxed->@name@ = _value;
}
"""

valuebox_structmember_sequence = """\
inline const _@name@_seq& @name@() const
{
  return _pd_boxed->@name@;
}
inline _@name@_seq& @name@()
{
  return _pd_boxed->@name@;
}
inline void @name@(const _@name@_seq& _value)
{
  _pd_boxed->@name@ = _value;
}
"""

valuebox_structmember_value = """\
virtual @type@* @name@() const
{
  return _pd_boxed->@name@.in();
}
virtual void @name@(@type@* _value)
{
  CORBA::add_ref(_value);
  _pd_boxed->@name@ = _value;
}
"""

# Member accessors for boxed unions

valuebox_union_discriminant = """\
inline @switchtype@ _d() const
{
  return _pd_boxed->_d();
}
inline void _d(@switchtype@ _value)
{
  _pd_boxed->_d(_value);
}
"""

valuebox_unionmember_array = """\
inline const @memtype@_slice* @name@ () const
{
  return _pd_boxed->@name@();
}
inline @memtype@_slice* @name@ ()
{
  return _pd_boxed->@name@();
}
inline void @name@ (const @const_type@ _value)
{
  _pd_boxed->@name@(_value);
}
"""

valuebox_unionmember_any = """\
inline const @type@& @name@ () const
{
  return _pd_boxed->@name@();
}
inline @type@& @name@()
{
  return _pd_boxed->@name@();
}
inline void @name@(const @type@& _value)
{
  _pd_boxed->@name@(_value);
}
"""

valuebox_unionmember_typecode = """\
inline CORBA::TypeCode_ptr @name@() const
{
  return _pd_boxed->@name@();
}
inline void @name@(CORBA::TypeCode_ptr _value)
{
  _pd_boxed->@name@(_value);
}
inline void @name@(CORBA::TypeCode_member& _value)
{
  _pd_boxed->@name@(_value);
}
inline void @name@(CORBA::TypeCode_var& _value)
{
  _pd_boxed->@name@(_value);
}
"""

valuebox_unionmember_basic = """\
inline @type@ @name@() const
{
  return _pd_boxed->@name@();
}
inline void @name@(@type@ _value)
{
  _pd_boxed->@name@(_value);
}
"""

valuebox_unionmember_string = """\
inline const char* @name@() const
{
  return _pd_boxed->@name@();
}
inline void @name@(char* _value)
{
  _pd_boxed->@name@(_value);
}
inline void @name@(const char* _value)
{
  _pd_boxed->@name@(_value);
}
inline void @name@(const CORBA::String_var& _value)
{
  _pd_boxed->@name@(_value);
}
"""

valuebox_unionmember_wstring = """\
inline const CORBA::WChar* @name@() const
{
  return _pd_boxed->@name@();
}
inline void @name@(CORBA::WChar* _value)
{
  _pd_boxed->@name@(_value);
}
inline void @name@(const CORBA::WChar* _value)
{
  _pd_boxed->@name@(_value);
}
inline void @name@(const CORBA::WString_var& _value)
{
  _pd_boxed->@name@(_value);
}
"""

valuebox_unionmember_objref = """\
inline @ptr_type@ @name@() const
{
  return _pd_boxed->@name@();
}
inline void @name@(@ptr_type@ _value)
{
  _pd_boxed->@name@(_value);
}
inline void @name@(const @var_type@& _value)
{
  _pd_boxed->@name@(_value);
}
inline void @name@(const @memtype@& _value)
{
  _pd_boxed->@name@(_value);
}
"""

valuebox_unionmember_constructed = """\
inline const @type@& @name@() const
{
  return _pd_boxed->@name@();
}
inline @type@& @name@()
{
  return _pd_boxed->@name@();
}
inline void @name@(const @type@& _value)
{
  _pd_boxed->@name@(_value);
}
"""

valuebox_unionmember_sequence = """\
inline const _@name@_seq& @name@() const
{
  return _pd_boxed->@name@();
}
inline _@name@_seq& @name@()
{
  return _pd_boxed->@name@();
}
inline void @name@(const _@name@_seq& _value)
{
  _pd_boxed->@name@(_value);
}
"""

valuebox_unionmember_value = """\
virtual @type@* @name@() const
{
  return _pd_boxed->@name@();
}
virtual void @name@(@type@* _value)
{
  _pd_boxed->@name@(_value);
}
"""


valuebox_functions = """\
// value box @name@

@fqname@*
@fqname@::_downcast(CORBA::ValueBase* _b)
{
  return _b ? (@fqname@*)_b->_ptrToValue(_PD_repoId) : 0;
}

const char* @fqname@::_PD_repoId = "@repoId@";

const char*
@fqname@::_NP_repositoryId() const
{
  return _PD_repoId;
}

const char*
@fqname@::_NP_repositoryId(CORBA::ULong& hash) const
{
  hash = @idhash@U;
  return _PD_repoId;
}

const _omni_ValueIds*
@fqname@::_NP_truncatableIds() const
{
  return 0;
}

CORBA::Boolean
@fqname@::_NP_custom() const
{
  return 0;
}

CORBA::Boolean
@fqname@::_NP_box() const
{
  return 1;
}

void*
@fqname@::_ptrToValue(const char* _id)
{
  if (_id == ::@fqname@::_PD_repoId)
    return (::@fqname@*) this;
  
  if (_id == CORBA::ValueBase::_PD_repoId)
    return (CORBA::ValueBase*) this;
  
  if (omni::strMatch(_id, ::@fqname@::_PD_repoId))
    return (::@fqname@*) this;
  
  if (omni::strMatch(_id, CORBA::ValueBase::_PD_repoId))
    return (CORBA::ValueBase*) this;

  return 0;
}

void
@fqname@::_NP_marshal(@fqname@* _v, cdrStream& _0s)
{
  omniValueType::marshal(_v, @fqname@::_PD_repoId, _0s);
}

@fqname@*
@fqname@::_NP_unmarshal(cdrStream& _0s)
{
  CORBA::ValueBase* _b = omniValueType::unmarshal(@fqname@::_PD_repoId,
						  @idhash@U, 0, _0s);
  @fqname@* _d = @fqname@::_downcast(_b);
  if (_b && !_d) {
    CORBA::remove_ref(_b);
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_ValueFactoryFailure,
		  (CORBA::CompletionStatus)_0s.completion());
  }
  return _d;
}

void
@fqname@::_PR_marshal_state(cdrStream& _0s) const
{
  @marshal_content@
}

void
@fqname@::_PR_unmarshal_state(cdrStream& _0s)
{
  @unmarshal_content@
}

#ifdef OMNI_HAVE_COVARIANT_RETURNS
@fqname@*
#else
CORBA::ValueBase*
#endif
@fqname@::_copy_value()
{
  CORBA::ValueBase* _b;
  _b = _omni_ValueFactoryManager::create_for_unmarshal(_PD_repoId, @idhash@U);
  @fqname@* _v = @fqname@::_downcast(_b);
  if (!_v) {
    CORBA::remove_ref(_b);
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_ValueFactoryFailure,
		  CORBA::COMPLETED_NO);
  }
  _v->_PR_copy_state(this);
  return _v;
}

void
@fqname@::_PR_copy_state(@fqname@* _v)
{
  @copy_content@
}

@fqname@::~@name@()
{
  @destructor@
}

// ValueFactory for value box
class _0RL_@flatname@_init : public CORBA::ValueFactoryBase
{
public:
  _0RL_@flatname@_init() {}
  virtual ~_0RL_@flatname@_init() {}

  virtual CORBA::ValueBase* create_for_unmarshal();
};

CORBA::ValueBase*
_0RL_@flatname@_init::create_for_unmarshal()
{
  return new @fqname@();
}

class _0RL_@flatname@_install {
public:
  _0RL_@flatname@_install() {
    CORBA::ValueFactoryBase* vf = new _0RL_@flatname@_init();
    _omni_ValueFactoryManager::register_factory(@fqname@::_PD_repoId, @idhash@, vf, 1);
    vf->_remove_ref();
  }
  ~_0RL_@flatname@_install() {
    try {
      _omni_ValueFactoryManager::unregister_factory(@fqname@::_PD_repoId, @idhash@);
    }
    catch (CORBA::BAD_PARAM&) {
    }
  }
};

static _0RL_@flatname@_install _0RL_@flatname@_install_;
"""




def getValueType(node):
    try:
        return node.__valueType
    except AttributeError:
        pass

    if isinstance(node, idlast.Value) or isinstance(node, idlast.ValueAbs):
        v = ValueType(node)

    elif isinstance(node, idlast.ValueForward):
        v = ValueForward(node)

    elif isinstance(node, idlast.ValueBox):
        v = ValueBox(node)

    else:
        util.fatalError("Unexpected node in getValueType: %s" % node)
    
    node.__valueType = v
    return v


def hashval(id):
    h = 0L
    for c in id:
        h = (((h << 5) & 0xffffffffL) ^ ((h >> 27))) ^ ord(c)
    hs = str(h)
    if hs[-1] == "L":
        hs = hs[:-1]
    return hs

def hashlist(ids):
    h = 0L
    for id in ids:
        for c in id:
            h = (((h << 5) & 0xffffffffL) ^ ((h >> 27))) ^ ord(c)
    hs = str(h)
    if hs[-1] == "L":
        hs = hs[:-1]
    return hs



class ValueForward (mapping.Decl):

    def __init__(self, astdecl):
        mapping.Decl.__init__(self, astdecl)

    def module_decls(self, stream, visitor):
        astdecl  = self._astdecl
        name     = astdecl.identifier()
        cxx_name = id.mapID(name)
        fullname = id.Name(astdecl.scopedName())
        guard    = fullname.guard()

        stream.out(value_forward, name=cxx_name, guard=guard)


class ValueType (mapping.Decl):

    """Deals with valuetype and abstract valuetype"""

    def __init__(self, astdecl):
        mapping.Decl.__init__(self, astdecl)

        outer_environment = id.lookup(astdecl)
        self._environment = outer_environment.enter(astdecl.identifier())
        self._fullname    = id.Name(astdecl.scopedName())
        value_name        = self._fullname.fullyQualify()

        if isinstance(astdecl, idlast.ValueAbs):
            self._abstract = 1
        else:
            self._abstract = 0

        if not self._abstract:
            for s in astdecl.statemembers():
                mtype = s.memberType()
                for d in s.declarators():
                    self._gen_statemember_code(value_name, mtype,
                                               d, self._environment)

    # Methods to make this look enough like a cxx.Class to make
    # cxx.Method happy.
    
    def environment(self):
        return self._environment

    def name(self):
        return self._fullname


    def _gen_statemember_code(self, value_name, mtype, decl, environment):
        """Generate code fragments for a statemember"""
        
        base_s      = output.StringStream()  # Base class decl
        obv_s       = output.StringStream()  # OBV class decl
        copy_s      = output.StringStream()  # Copy member
        marshal_s   = output.StringStream()  # Marshal member
        unmarshal_s = output.StringStream()  # Unmarshal member
        init_s      = output.StringStream()  # Initialise member
        impl_s      = output.StringStream()  # Member implementations

        mType   = types.Type(mtype)
        d_mType = mType.deref()

        decl_dims = decl.sizes()
        full_dims = decl_dims + mType.dims()

        is_array            = full_dims    != []
        is_array_declarator = decl_dims    != []
        alias_array         = mType.dims() != []

        member   = id.mapID(decl.identifier())
        bmemtype = mType.member(environment)
        omemtype = mType.member(gscope=1)

        bconst_type_str = bmemtype
        oconst_type_str = omemtype

        # anonymous arrays are handled slightly differently
        if is_array_declarator:
            prefix = config.state['Private Prefix']
            base_s.out(statemember_array_declarator,
                       prefix = prefix,
                       memtype = bmemtype,
                       name = member,
                       dims = cxx.dimsToString(decl.sizes()),
                       tail_dims = cxx.dimsToString(decl.sizes()[1:]))
            bconst_type_str = oconst_type_str = prefix + "_" + member
            bmemtype = omemtype = "_" + member

        if is_array:
            base_s.out(statemember_array_sig,
                       memtype = bmemtype,
                       const_type = bconst_type_str,
                       name = member,
                       abs = "= 0")

            obv_s.out(statemember_array_sig,
                      memtype = omemtype,
                      const_type = oconst_type_str,
                      name = member,
                      abs = "")

            copy_s.out(statemember_copy, name=member)

            marshal_s.out("@memtype@_slice* _@name@ = @name@();",
                          memtype=omemtype, name=member)
            skutil.marshall(marshal_s, None, mType, decl, "_" + member, "_0s")
            
            unmarshal_s.out("@memtype@_slice* _@name@ = @name@();",
                            memtype=omemtype, name=member)
            skutil.unmarshall(unmarshal_s, None, mType,
                              decl, "_" + member, "_0s")

            loop = cxx.For(init_s, full_dims)
            init_s.out("_pd_@name@@index@ = _@name@@index@;",
                       name=member, index=loop.index())
            loop.end()

            array_copy = output.StringStream()
            loop = cxx.For(array_copy, full_dims)
            array_copy.out("_pd_@name@@index@ = _value@index@;",
                           name=member, index=loop.index())
            loop.end()
            
            impl_s.out(statemember_array_impl, name=member,
                       memtype=omemtype, const_type=oconst_type_str,
                       value_name=value_name, array_copy=array_copy)

            decl._cxx_holder   = oconst_type_str + " _pd_" + member + ";"
            decl._cxx_init_arg = "const " + oconst_type_str + " _" + member

        elif d_mType.any():
            # note type != CORBA::Any when its an alias...
            base_s.out(statemember_any_sig,
                       type = bmemtype,
                       name = member,
                       abs = "= 0")

            obv_s.out(statemember_any_sig,
                      type = omemtype,
                      name = member,
                      abs = "")

            copy_s.out(statemember_copy, name=member)

            marshal_s.out(statemember_any_marshal,
                          type=bmemtype, name=member)

            unmarshal_s.out(statemember_any_unmarshal,
                            type=bmemtype, name=member)

            init_s.out(statemember_init, name=member)

            impl_s.out(statemember_any_impl, type=omemtype,
                       name=member, value_name=value_name)

            decl._cxx_holder   = omemtype + " _pd_" + member + ";"
            decl._cxx_init_arg = "const " + omemtype + "& _" + member

        elif d_mType.typecode():
            base_s.out(statemember_typecode_sig,
                       name = member,
                       abs = "= 0")

            base_s.out(statemember_typecode_member,
                       name = member)

            obv_s.out(statemember_typecode_sig,
                      name = member,
                      abs = "")

            copy_s.out(statemember_copy, name=member)
            marshal_s.out(statemember_typecode_marshal, name=member)
            unmarshal_s.out(statemember_typecode_unmarshal, name=member)
            init_s.out(statemember_typecode_init, name=member)
            impl_s.out(statemember_typecode_impl, name=member,
                       value_name=value_name)

            decl._cxx_holder = "CORBA::TypeCode_member _pd_" + member + ";"
            decl._cxx_init_arg = "CORBA::TypeCode_ptr _" + member

        elif isinstance(d_mType.type(), idltype.Base) or d_mType.enum():
            # basic type
            base_s.out(statemember_basic_sig,
                       type = bmemtype,
                       name = member,
                       abs = "= 0")

            obv_s.out(statemember_basic_sig,
                      type = omemtype,
                      name = member,
                      abs = "")

            copy_s.out(statemember_copy, name=member)

            if d_mType.char():
                marshal_s.out(statemember_basic_kind_marshal, name=member,
                              kind="Char")
                unmarshal_s.out(statemember_basic_kind_unmarshal,
                                name=member, type=bmemtype, kind="Char")
            elif d_mType.octet():
                marshal_s.out(statemember_basic_kind_marshal, name=member,
                              kind="Octet")
                unmarshal_s.out(statemember_basic_kind_unmarshal,
                                name=member, type=bmemtype, kind="Octet")
            elif d_mType.boolean():
                marshal_s.out(statemember_basic_kind_marshal, name=member,
                              kind="Boolean")
                unmarshal_s.out(statemember_basic_kind_unmarshal,
                                name=member, type=bmemtype, kind="Boolean")
            elif d_mType.wchar():
                marshal_s.out(statemember_basic_kind_marshal, name=member,
                              kind="WChar")
                unmarshal_s.out(statemember_basic_kind_unmarshal,
                                name=member, type=bmemtype, kind="WChar")

                
            else:
                marshal_s.out(statemember_basic_marshal, name=member)
                unmarshal_s.out(statemember_basic_unmarshal,
                                name=member, type=bmemtype)

            init_s.out(statemember_init, name=member)
            impl_s.out(statemember_basic_impl, name=member, type=omemtype,
                       value_name=value_name)

            decl._cxx_holder   = omemtype + " _pd_" + member + ";"
            decl._cxx_init_arg = omemtype + " _" + member

        elif d_mType.string():
            base_s.out(statemember_string_sig,
                       name = member,
                       abs = "= 0")
            
            base_s.out(statemember_string_member,
                       name = member)

            obv_s.out(statemember_string_sig,
                      name = member,
                      abs = "")

            copy_s.out(statemember_copy, name=member)
            marshal_s.out(statemember_string_marshal, name=member)
            unmarshal_s.out(statemember_string_unmarshal, name=member)
            init_s.out(statemember_init, name=member)
            impl_s.out(statemember_string_impl, name=member,
                       value_name=value_name)

            decl._cxx_holder   = omemtype + " _pd_" + member + ";"
            decl._cxx_init_arg = "const char* _" + member

        elif d_mType.wstring():
            base_s.out(statemember_wstring_sig,
                       name = member,
                       abs = "= 0")

            base_s.out(statemember_wstring_member,
                       name = member)

            obv_s.out(statemember_wstring_sig,
                      name = member,
                      abs = "")

            copy_s.out(statemember_copy, name=member)
            marshal_s.out(statemember_wstring_marshal, name=member)
            unmarshal_s.out(statemember_wstring_unmarshal, name=member)
            init_s.out(statemember_init, name=member)
            impl_s.out(statemember_wstring_impl, name=member,
                       value_name=value_name)

            decl._cxx_holder = omemtype + " _pd_" + member + ";"
            decl._cxx_init_arg = "const CORBA::WChar* _" + member

        elif d_mType.objref():
            scopedName = d_mType.type().decl().scopedName()

            name     = id.Name(scopedName)
            type     = name.unambiguous(environment)
            gtype    = name.unambiguous(None)
            ptr_type = name.suffix("_ptr").unambiguous(environment)
            helper   = name.suffix("_Helper").unambiguous(environment)
            var_type = name.suffix("_var").unambiguous(environment)

            base_s.out(statemember_objref_sig,
                       name = member,
                       ptr_type = ptr_type,
                       var_type = var_type,
                       abs = "= 0")

            base_s.out(statemember_objref_member,
                       name = member,
                       memtype = bmemtype,
                       ptr_type = ptr_type,
                       helper = helper)

            # *** HERE: is this right?  What if ptr_type is relative?
            if ptr_type[:2] != "::":
                ptr_type = "::" + ptr_type
            if var_type[:2] != "::":
                var_type = "::" + var_type
            if gtype[:2] != "::":
                gtype = "::" + gtype

            obv_s.out(statemember_objref_sig,
                      name = member,
                      ptr_type = ptr_type,
                      var_type = var_type,
                      abs = "")

            copy_s.out(statemember_copy, name=member)
            marshal_s.out(statemember_objref_marshal, name=member, type=type)
            unmarshal_s.out(statemember_objref_unmarshal, name=member,
                            type=type, var_type=var_type)
            init_s.out(statemember_objref_init, name=member, type=gtype)
            impl_s.out(statemember_objref_impl, name=member, type=gtype,
                       ptr_type=ptr_type, var_type=var_type,
                       value_name=value_name)

            decl._cxx_holder   = omemtype + " _pd_" + member + ";"
            decl._cxx_init_arg = ptr_type + " _" + member

        elif (mType.typedef() or d_mType.struct()
              or d_mType.union() or d_mType.fixed()):

            base_s.out(statemember_constructed_sig,
                       type = bmemtype,
                       name = member,
                       abs = "= 0")

            obv_s.out(statemember_constructed_sig,
                      type = omemtype,
                      name = member,
                      abs = "")

            copy_s.out(statemember_copy, name=member)
            marshal_s.out(statemember_constructed_marshal, name=member)
            unmarshal_s.out(statemember_constructed_unmarshal,
                            name=member, type=bmemtype)
            init_s.out(statemember_init, name=member)

            impl_s.out(statemember_constructed_impl, name=member,
                       type=omemtype, value_name=value_name)

            decl._cxx_holder   = omemtype + " _pd_" + member + ";"
            decl._cxx_init_arg = omemtype + "& _" + member

        elif d_mType.sequence():
            sequence_template = d_mType.sequenceTemplate(environment)

            base_s.out(statemember_sequence_typedef,
                       sequence_template = sequence_template,
                       name = member)
            
            base_s.out(statemember_sequence_sig,
                       name = member,
                       abs = "= 0")

            obv_s.out(statemember_sequence_sig,
                      name = member,
                      abs = "")

            copy_s.out(statemember_copy, name=member)
            marshal_s.out(statemember_sequence_marshal,
                          sequence_template=sequence_template, name=member)
            unmarshal_s.out(statemember_sequence_unmarshal,
                            sequence_template=sequence_template, name=member)
            init_s.out(statemember_init, name=member)

            impl_s.out(statemember_sequence_impl, name=member,
                       value_name=value_name)

            decl._cxx_holder   = omemtype + " _pd_" + member + ";"
            decl._cxx_init_arg = member + "_seq& _" + member

        elif d_mType.value() or d_mType.valuebox():
            otype = mType.base()

            base_s.out(statemember_value_sig,
                       name = member,
                       type = mType.base(environment),
                       abs = "= 0")

            obv_s.out(statemember_value_sig,
                      name = member,
                      type = otype,
                      abs = "")

            copy_s.out(statemember_copy_value, name=member,type=otype)
            marshal_s.out(statemember_value_marshal, name=member, type=otype)
            unmarshal_s.out(statemember_value_unmarshal,
                            name=member, type=otype)
            init_s.out(statemember_init, name=member)

            impl_s.out(statemember_value_impl, name=member, type=otype,
                       value_name=value_name)

            decl._cxx_holder   = omemtype + " _pd_" + member + ";"
            decl._cxx_init_arg = otype + "* _" + member

        else:
            util.fatalError("Unknown statemember type encountered")

        decl._cxx_name           = member
        decl._cxx_base_sig       = str(base_s)
        decl._cxx_obv_sig        = str(obv_s)
        decl._cxx_copy_impl      = str(copy_s)
        decl._cxx_marshal_impl   = str(marshal_s)
        decl._cxx_unmarshal_impl = str(unmarshal_s)
        decl._cxx_init_impl      = str(init_s)
        decl._cxx_impl           = str(impl_s)
        

    def module_decls(self, stream, visitor):
        astdecl  = self._astdecl
        name     = astdecl.identifier()
        cxx_name = id.mapID(name)
        fullname = self._fullname
        guard    = fullname.guard()

        environment = self._environment

        def gen_other_idl(node=astdecl, visitor=visitor):
            for n in node.declarations():
                n.accept(visitor)

        has_callables     = 0
        has_factories     = 0
        supports_abstract = 0

        if astdecl.callables():
            has_callables = 1

        if astdecl.factories():
            has_factories = 1

        inheritl = []
        for v in astdecl.inherits():
            iname = id.Name(v.scopedName())
            uname = iname.unambiguous(environment)
            inheritl.append("public virtual " + uname)
            if v._cxx_has_callables:
                has_callables = 1
            if v._cxx_has_factories:
                has_factories = 1

        if not inheritl:
            inheritl.append("public virtual CORBA::ValueBase")

        if astdecl.supports():
            has_callables = 1

            for intf in astdecl.supports():
                if intf.abstract():
                    supports_abstract = 1
                    iname = id.Name(intf.scopedName())
                    uname = iname.unambiguous(environment)
                    inheritl.append("public virtual " + uname)

        inherits = string.join(inheritl, ",\n")

        astdecl._cxx_has_callables     = has_callables
        astdecl._cxx_has_factories     = has_factories
        astdecl._cxx_supports_abstract = supports_abstract

        public_accessors  = output.StringStream()
        private_accessors = output.StringStream()

        if not self._abstract:
            for s in astdecl.statemembers():
                if s.memberAccess() == 0:
                    # Public
                    for d in s.declarators():
                        public_accessors.out(d._cxx_base_sig)
                else:
                    # Private
                    for d in s.declarators():
                        private_accessors.out(d._cxx_base_sig)

        operationl = []
        callables  = []

        for intf in astdecl.supports():
            callables.extend(intf.all_callables())

        callables.extend(astdecl.callables())

        for c in callables:
            if isinstance(c, idlast.Operation):
                op = call.operation(self, c)
                method = iface._impl_Method(op, self)
                operationl.append(method.hh(1,1))
            else:
                ops = call.read_attributes(self, c)
                if not c.readonly():
                    ops.extend(call.write_attributes(self, c))

                for op in ops:
                    method = iface._impl_Method(op, self)
                    operationl.append(method.hh(1,1))
                
        operations = string.join(operationl, "\n")

        if supports_abstract:
            downcast_abstract = value_class_downcast_abstract
            np_to_value       = value_class_np_to_value
        else:
            downcast_abstract = ""
            np_to_value       = ""

        stream.out(value_forward, name=cxx_name, guard=guard)

        stream.out(value_class,
                   name              = cxx_name,
                   inherits          = inherits,
                   public_accessors  = public_accessors,
                   private_accessors = private_accessors,
                   operations        = operations,
                   downcast_abstract = downcast_abstract,
                   np_to_value       = np_to_value,
                   other_idl         = gen_other_idl)

        if astdecl.factories():
            funcs = []

            valtype = idltype.Declared(astdecl, astdecl.scopedName(),
                                       idltype.tk_value, 0)

            for f in astdecl.factories():
                wrapper = FactoryWrapper(f, valtype)
                op = call.operation(self, wrapper)
                method = iface._impl_Method(op, self)
                funcs.append(method.hh(1,1))
                
            factory_funcs = string.join(funcs, "\n")

            stream.out(valuefactory_class_initialisers,
                       name=cxx_name, factory_funcs=factory_funcs)

        if not (has_callables or has_factories):
            stream.out(valuefactory_class_no_operations, name=cxx_name)

        if config.state['Typecode']:
            from omniidl_be.cxx.header import defs, template
            qualifier = defs.const_qualifier()
            stream.out(template.typecode,
                       qualifier = qualifier, name = cxx_name)



    def poa_module_decls(self, stream, visitor):
        astdecl = self._astdecl

        if astdecl.supports() and not astdecl.supports()[0].abstract():
            name       = astdecl.identifier()
            poa_name   = visitor.POA_prefix() + name
            value_name = self._fullname.fullyQualify()

            intf     = astdecl.supports()[0]
            isname   = intf.scopedName()
            irname   = id.Name(isname).relName(self._environment)

            if irname is None:
                iname = "::POA_" + string.join(isname, "::")
            elif irname == isname:
                iname = "POA_" + string.join(irname, "::")
            else:
                iname = string.join(irname, "::")
            
            stream.out(value_poa_class,
                       poa_name=poa_name, intf_poa=iname,
                       value_name=value_name);


    def obv_module_decls(self, stream, visitor):
        if self._abstract:
            return

        astdecl    = self._astdecl
        name       = astdecl.identifier()
        cxx_name   = id.mapID(name)
        obv_name   = visitor.OBV_prefix() + name
        value_name = self._fullname.fullyQualify()

        public_accessors  = output.StringStream()
        private_accessors = output.StringStream()
        state_holders     = output.StringStream()

        inheritl = ["public virtual " + value_name]

        for v in astdecl.inherits():
            if isinstance(v, idlast.Value):
                isname = v.scopedName()
                irname = id.Name(isname).relName(self._environment)
                if irname is None:
                    iname = "::OBV_" + string.join(isname, "::")
                elif irname == isname:
                    iname = "OBV_" + string.join(irname, "::")
                else:
                    iname = string.join(irname, "::")

                inheritl.append("public virtual " + iname)

        if not (astdecl.inherits() or
                astdecl._cxx_has_callables or
                astdecl._cxx_has_factories):

            # *** Section 1.17.10.3 of the C++ mapping, version 1.1 says
            #
            #   "For valuetypes that have no operations or
            #   initializers, a concrete type-specific factory class
            #   is generated whose implementation of the
            #   create_for_unmarshal function simply constructs an
            #   instance of the OBV_ class for the valuetype using new
            #   and the default constructor.
            #
            # Unfortunately, that requires the generation of invalid
            # C++, since the OBV_ class is abstract. To avoid that, we
            # inherit from DefaultValueRefCountBase.

            inheritl.append("public virtual CORBA::DefaultValueRefCountBase")

        inherits = string.join(inheritl, ",\n")

        if (astdecl.inherits() and
            isinstance(astdecl.inherits()[0], idlast.Value)):

            baseval = astdecl.inherits()[0]
            statememberl = baseval._cxx_statememberl[:]
            init_paraml  = baseval._cxx_init_paraml[:]
        else:
            statememberl = []
            init_paraml  = []

        for s in astdecl.statemembers():
            for d in s.declarators():
                if s.memberAccess() == 0:
                    # Public
                    public_accessors.out(d._cxx_obv_sig)
                else:
                    # Private
                    private_accessors.out(d._cxx_obv_sig)

                state_holders.out(d._cxx_holder)
                statememberl.append(d._cxx_name)
                init_paraml.append(d._cxx_init_arg)

        astdecl._cxx_init_paraml  = init_paraml
        astdecl._cxx_statememberl = statememberl

        init_params = string.join(init_paraml, ", ")

        if astdecl._cxx_has_callables:
            constructor_access = "protected"
        else:
            constructor_access = "public"

        if init_params:
            init_constructor = obv_name + "(" + init_params + ");"
        else:
            init_constructor = ""

        stream.out(value_obv_class, obv_name=obv_name,
                   inherits=inherits,
                   init_constructor = init_constructor,
                   constructor_access=constructor_access,
                   public_accessors=public_accessors,
                   private_accessors=private_accessors,
                   state_holders=state_holders)


    def skel_defs(self, stream, visitor):
        astdecl  = self._astdecl
        name     = astdecl.identifier()
        cxx_name = id.mapID(name)

        value_name  = self._fullname.fullyQualify()

        repoId = astdecl.repoId()
        idhash = hashval(repoId)

        if not self._abstract and astdecl.custom():
            custom = "1"
        else:
            custom = "0"

        ptrToValuePtr       = output.StringStream()
        ptrToValueStr       = output.StringStream()
        copy_members        = output.StringStream()
        marshal_members     = output.StringStream()
        unmarshal_members   = output.StringStream()
        member_initialisers = output.StringStream()

        for v in astdecl.inherits():
            iname  = "::" + string.join(v.scopedName(), "::")
            ptrToValuePtr.out(value_ptrToValuePtr, iname=iname)
            ptrToValueStr.out(value_ptrToValueStr, iname=iname)

        for i in astdecl.supports():
            if i.abstract():
                iname  = "::" + string.join(i.scopedName(), "::")
                ptrToValuePtr.out(value_ptrToValuePtr, iname=iname)
                ptrToValueStr.out(value_ptrToValueStr, iname=iname)

        if astdecl._cxx_supports_abstract:
            ptrToValuePtr.out(value_ptrToValuePtr, iname="CORBA::AbstractBase")
            ptrToValueStr.out(value_ptrToValueStr, iname="CORBA::AbstractBase")

        if (astdecl.inherits() and
            isinstance(astdecl.inherits()[0], idlast.Value)):

            v = astdecl.inherits()[0]
            iname  = "::" + string.join(v.scopedName(), "::")
            ioname = "::OBV_" + string.join(v.scopedName(), "::")
            sms = map(lambda s: "_" + s, v._cxx_statememberl)
            marshal_members.out("@iname@::_PR_marshal_state(_0s);",
                                iname=iname)
            unmarshal_members.out("@iname@::_PR_unmarshal_state(_0s);",
                                  iname=iname)
            copy_members.out("@iname@::_PR_copy_state(_v);", iname=iname)
            base_init = "  : " + ioname + "(" + string.join(sms, ", ") + ")"
        else:
            base_init = ""

        if not self._abstract:
            for s in astdecl.statemembers():
                for d in s.declarators():
                    copy_members.out(d._cxx_copy_impl)
                    marshal_members.out(d._cxx_marshal_impl)
                    unmarshal_members.out(d._cxx_unmarshal_impl)
                    member_initialisers.out(d._cxx_init_impl)

            init_params = string.join(astdecl._cxx_init_paraml, ", ")

        stream.out(value_functions, fqname=value_name, name=cxx_name,
                   idhash=idhash, copy_members=copy_members,
                   repoId=repoId, custom=custom,
                   ptrToValuePtr=ptrToValuePtr, ptrToValueStr=ptrToValueStr,
                   marshal_members=marshal_members,
                   unmarshal_members=unmarshal_members)

        if astdecl._cxx_supports_abstract:
            stream.out(value_functions_abstract, fqname=value_name)

        if not self._abstract:
            vname = cxx_name
            if vname == value_name:
                vname = "OBV_" + vname
                
            stream.out(value_obv_functions, fqname=value_name, name=vname)

            if init_params:
                stream.out(value_obv_init_function, fqname=value_name,
                           name=vname, init_params=init_params,
                           base_init=base_init,
                           member_initialisers=member_initialisers)

        if astdecl.supports() and not astdecl.supports()[0].abstract():
            # POA functions
            vname = cxx_name
            if vname == value_name:
                vname = "POA_" + vname
                
            stream.out(value_poa_functions, fqname=value_name, name=vname)

        if not self._abstract and astdecl.truncatable():
            flatname  = string.join(astdecl.scopedName(), "_")
            baseidl   = []

            v = astdecl
            idlist = []
            while v:
                iname = "::" + string.join(v.scopedName(), "::")
                hash = hashval(v.repoId())
                idlist.append(v.repoId())
                baseidl.append("{ %s::_PD_repoId, %sU }" % (iname, hash))
                if v.truncatable():
                    v = v.inherits()[0]
                else:
                    v = None

            baseids = string.join(baseidl, ",\n")
            baseshash = hashlist(idlist)

            stream.out(value_truncatable_bases, fqname=value_name,
                       flatname=flatname, baseids=baseids,
                       basecount=len(baseidl), baseshash=baseshash)

        else:
            stream.out(value_no_truncatable_bases, fqname=value_name)

        if not self._abstract:
            for s in astdecl.statemembers():
                for d in s.declarators():
                    stream.out(d._cxx_impl)

        if astdecl.factories():
            stream.out(valuefactory_functions, fqname=value_name,
                       name=cxx_name)

        if not (astdecl._cxx_has_callables or astdecl._cxx_has_factories):
            stream.out(valuefactory_functions, fqname=value_name,
                       name=cxx_name)
            stream.out(valuefactory_create_for_unmarshal, fqname=value_name)


class FactoryWrapper (idlast.Operation):
    """
    Wrapper around an idlast.Factory object that makes it look like an
    Operation object.
    """
    def __init__(self, factory, valtype):
        self._factory = factory
        self._valtype = valtype

    def oneway(self):
        return 0

    def identifier(self):
        return self._factory.identifier()

    def returnType(self):
        return self._valtype

    def parameters(self):
        return self._factory.parameters()

    def raises(self):
        return self._factory.raises()

    def contexts(self):
        return []



class ValueBox (mapping.Decl):

    """Deals with boxed valuetypes"""

    def __init__(self, astdecl):
        mapping.Decl.__init__(self, astdecl)

        outer_environment = id.lookup(astdecl)
        self._environment = outer_environment
        self._fullname    = id.Name(astdecl.scopedName())

    # Methods to make this look enough like a cxx.Class to make
    # cxx.Method happy.
    
    def environment(self):
        return self._environment

    def name(self):
        return self._fullname

    def module_decls(self, stream, visitor):
        astdecl  = self._astdecl
        name     = astdecl.identifier()
        cxx_name = id.mapID(name)
        fullname = self._fullname
        guard    = fullname.guard()

        environment = self._environment

        if astdecl.constrType():
            astdecl.boxedType().decl().accept(visitor)

        boxedType = types.Type(astdecl.boxedType())

        anonSeq = 0
        if astdecl.boxedType().kind() == idltype.tk_sequence:
            # Boxed anonymous sequence. We create a pretend typedef to
            # the sequence, mangling the name of the valuebox.
            anonSeq = 1
            dname = "_0RL_boxed_" + astdecl.identifier()
            
            declarator = idlast.Declarator(astdecl.file(), astdecl.line(),
                                           astdecl.mainFile(), [], [],
                                           dname, [dname], "omni:internal",
                                           [])

            typedef = idlast.Typedef(astdecl.file(), astdecl.line(),
                                     astdecl.mainFile(), [], [],
                                     astdecl.boxedType(), 0, [declarator])

            declarator._setAlias(typedef)

            env = id.lookup(astdecl)
            id.addNode(typedef, env)

            newtype = idltype.Declared(declarator, [dname],
                                       idltype.tk_alias, 0)
            boxedType = types.Type(newtype)
            typedef.accept(visitor)

        member_funcs = output.StringStream()

        d_boxedType = boxedType.deref()

        if boxedType.dims():
            btype = boxedType.member(environment)
            slice = btype + "_slice"
            helper = btype + "_copyHelper"
            boxed_member = slice + "*"
            member_funcs.out(valuebox_member_funcs_array,
                             name=cxx_name, boxedtype=btype,
                             slice=slice, helper=helper)

            if boxedType.variable():
                member_funcs.out(valuebox_member_funcs_array_variable_out,
                                 slice=slice, helper=helper)
            else:
                member_funcs.out(valuebox_member_funcs_array_fixed_out,
                                 slice=slice)

        elif d_boxedType.any():
            btype = boxedType.member(environment)
            boxed_member = boxedType._var(environment)
            member_funcs.out(valuebox_member_funcs_any,
                             name=cxx_name, boxedtype=btype)

        elif d_boxedType.typecode():
            boxed_member = "CORBA::TypeCode_var"
            member_funcs.out(valuebox_member_funcs_objref, name=cxx_name,
                             boxedif="CORBA::TypeCode")

        elif (isinstance(d_boxedType.type(), idltype.Base) or
              d_boxedType.enum()):

            btype = boxedType.member(environment)
            boxed_member = btype
            member_funcs.out(valuebox_member_funcs_basic,
                             name=cxx_name, boxedtype=btype)

        elif d_boxedType.string():
            boxed_member = "char*"
            member_funcs.out(valuebox_member_funcs_string, name=cxx_name)
        
        elif d_boxedType.wstring():
            boxed_member = "CORBA::WChar*"
            member_funcs.out(valuebox_member_funcs_wstring, name=cxx_name)

        elif d_boxedType.objref():
            scopedName   = d_boxedType.type().decl().scopedName()
            name         = id.Name(scopedName)
            boxedif      = name.unambiguous(environment)
            boxed_member = d_boxedType.member(environment)

            member_funcs.out(valuebox_member_funcs_objref, name=cxx_name,
                             boxedif=boxedif)

        elif d_boxedType.struct():
            btype = boxedType.member(environment)
            boxed_member = boxedType._var(environment)
            members = d_boxedType.type().decl().members()
            accessor_funcs = self._struct_accessor_funcs(members)
            member_funcs.out(valuebox_member_funcs_struct_union,
                             name=cxx_name, boxedtype=btype,
                             accessor_funcs = accessor_funcs)

        elif d_boxedType.union():
            btype = boxedType.member(environment)
            boxed_member = boxedType._var(environment)
            udecl = d_boxedType.type().decl()
            cases = udecl.cases()
            switchtype = types.Type(udecl.switchType())
            accessor_funcs = self._union_accessor_funcs(cases, switchtype)
            member_funcs.out(valuebox_member_funcs_struct_union,
                             name=cxx_name, boxedtype=btype,
                             accessor_funcs = accessor_funcs)

        elif d_boxedType.sequence():
            btype        = boxedType.member(environment)
            boxed_member = boxedType._var(environment)
            seqType      = types.Type(d_boxedType.type().seqType())
            d_seqType    = seqType.deref()
            bounded      = d_boxedType.type().bound()

            if anonSeq:
                assert btype[:2] == "::"
                assert boxed_member[:2] == "::"
                btype = btype[2:]
                boxed_member = boxed_member[2:]

            if seqType.array():
                element = "INVALID"
                element_ptr = seqType.base(environment)
            else:
                if d_seqType.string():
                    element = "_CORBA_String_element"
                    element_ptr = "char*"
                elif d_seqType.wstring():
                    element = "_CORBA_WString_element"
                    element_ptr = "CORBA::WChar*"
                elif d_seqType.objref():
                    element = seqType.base(environment)
                    element_ptr = element
                elif seqType.sequence():
                    element = d_seqType.sequenceTemplate(environment)
                    element_ptr = element
                elif d_seqType.typecode():
                    element = "CORBA::TypeCode_member"
                    element_ptr = element
                else:
                    element = seqType.base(environment)
                    element_ptr = element

            element_reference = "INVALID"
            if not boxedType.array():
                if d_seqType.string():
                    # special case alert
                    element_reference = element
                elif d_seqType.wstring():
                    # special case alert
                    element_reference = element
                elif d_seqType.objref():
                    element_reference = d_seqType.objRefTemplate("Element",
                                                                 environment)
                # only if an anonymous sequence
                elif seqType.sequence():
                    element_reference = d_seqType.\
                                        sequenceTemplate(environment) + "&"
                else:
                    element_reference = element + "&"

            constructors = output.StringStream()
            if bounded:
                constructor_template = valuebox_sequence_constructors_bounded
            else:
                constructor_template = valuebox_sequence_constructors_unbounded

            constructors.out(constructor_template,
                             name=cxx_name, boxedtype=btype,
                             element=element_ptr)
                
            if seqType.array():
                subscript_element = element_ptr + "_slice*"
            else:
                subscript_element = element_reference

            member_funcs.out(valuebox_member_funcs_sequence,
                             name=cxx_name, boxedtype=btype,
                             subscript_element=subscript_element,
                             sequence_constructors=constructors)


        elif d_boxedType.fixed():
            btype = boxedType.member(environment)
            boxed_member = btype
            member_funcs.out(valuebox_member_funcs_fixed,
                             name=cxx_name, boxedtype=btype)

        else:
            util.fatalError("Unknown boxed type encountered")

        stream.out(value_forward, name=cxx_name, guard=guard)
        stream.out(valuebox_class, name=cxx_name,
                   member_funcs=member_funcs,
                   boxed_member=boxed_member)

        if config.state['Typecode']:
            from omniidl_be.cxx.header import defs, template
            qualifier = defs.const_qualifier()
            stream.out(template.typecode,
                       qualifier = qualifier, name = cxx_name)


    def _struct_accessor_funcs(self, members):
        stream = output.StringStream()

        for m in members:
            mType = types.Type(m.memberType())
            d_mType = mType.deref()

            bmemtype = mType.member(self._environment)

            for decl in m.declarators():
                decl_dims = decl.sizes()
                full_dims = decl_dims + mType.dims()
                member = id.mapID(decl.identifier())

                if full_dims:
                    # Array
                    if decl_dims:
                        # Anonymous array
                        prefix = config.state['Private Prefix']
                        bconst_type_str = prefix + "_" + member
                        bmemtype = "_" + member
                    else:
                        bconst_type_str = bmemtype

                    array_copy = output.StringStream()
                    loop = cxx.For(array_copy, full_dims)
                    array_copy.out("_pd_boxed->@name@@index@ = _value@index@;",
                                   name=member, index=loop.index())
                    loop.end()

                    stream.out(valuebox_structmember_array,
                               name=member, memtype=bmemtype,
                               const_type = bconst_type_str,
                               array_copy = array_copy)

                elif d_mType.any():
                    stream.out(valuebox_structmember_any,
                               name=member, type=bmemtype)

                elif d_mType.typecode():
                    stream.out(valuebox_structmember_typecode, name=member)

                elif (isinstance(d_mType.type(), idltype.Base) or
                      d_mType.enum()):

                    # Basic type
                    stream.out(valuebox_structmember_basic,
                               name=member, type=bmemtype)

                elif d_mType.string():
                    stream.out(valuebox_structmember_string, name=member)

                elif d_mType.wstring():
                    stream.out(valuebox_structmember_wstring, name=member)

                elif d_mType.objref():
                    scopedName = d_mType.type().decl().scopedName()

                    env      = self._environment
                    name     = id.Name(scopedName)
                    type     = name.unambiguous(env)
                    ptr_type = name.suffix("_ptr").unambiguous(env)
                    var_type = name.suffix("_var").unambiguous(env)
                    
                    stream.out(valuebox_structmember_objref,
                               name=member, type=type, ptr_type=ptr_type,
                               var_type=var_type, memtype=bmemtype)

                elif (mType.typedef() or d_mType.struct()
                      or d_mType.union() or d_mType.fixed()):

                    stream.out(valuebox_structmember_constructed,
                               name=member, type=bmemtype)

                elif d_mType.sequence():
                    stream.out(valuebox_structmember_sequence, name=member)

                elif d_mType.value():
                    stream.out(valuebox_structmember_value,
                               name=member, type=bmemtype)

                else:
                    util.fatalError("Unknown struct member type encountered")

        return stream

    def _union_accessor_funcs(self, cases, switchtype):
        stream = output.StringStream()

        stream.out(valuebox_union_discriminant,
                   switchtype = switchtype.member(self._environment))

        for c in cases:
            mType = types.Type(c.caseType())
            d_mType = mType.deref()

            bmemtype = mType.member(self._environment)

            decl = c.declarator()

            decl_dims = decl.sizes()
            full_dims = decl_dims + mType.dims()
            member = id.mapID(decl.identifier())

            if full_dims:
                # Array
                if decl_dims:
                    # Anonymous array
                    prefix = config.state['Private Prefix']
                    bconst_type_str = prefix + "_" + member
                    bmemtype = "_" + member
                else:
                    bconst_type_str = bmemtype

                stream.out(valuebox_unionmember_array,
                           name=member, memtype=bmemtype,
                           const_type = bconst_type_str)

            elif d_mType.any():
                stream.out(valuebox_unionmember_any,
                           name=member, type=bmemtype)

            elif d_mType.typecode():
                stream.out(valuebox_unionmember_typecode, name=member)

            elif (isinstance(d_mType.type(), idltype.Base) or
                  d_mType.enum()):

                # Basic type
                stream.out(valuebox_unionmember_basic,
                           name=member, type=bmemtype)

            elif d_mType.string():
                stream.out(valuebox_unionmember_string, name=member)

            elif d_mType.wstring():
                stream.out(valuebox_unionmember_wstring, name=member)

            elif d_mType.objref():
                scopedName = d_mType.type().decl().scopedName()

                env      = self._environment
                name     = id.Name(scopedName)
                type     = name.unambiguous(env)
                ptr_type = name.suffix("_ptr").unambiguous(env)
                var_type = name.suffix("_var").unambiguous(env)

                stream.out(valuebox_unionmember_objref,
                           name=member, type=type, ptr_type=ptr_type,
                           var_type=var_type, memtype=bmemtype)

            elif (mType.typedef() or d_mType.struct()
                  or d_mType.union() or d_mType.fixed()):

                stream.out(valuebox_unionmember_constructed,
                           name=member, type=bmemtype)

            elif d_mType.sequence():
                stream.out(valuebox_unionmember_sequence, name=member)

            elif d_mType.value():
                stream.out(valuebox_unionmember_value,
                           name=member, type=bmemtype)

            else:
                util.fatalError("Unknown struct member type encountered")

        return stream

    def skel_defs(self, stream, visitor):
        astdecl  = self._astdecl
        name     = astdecl.identifier()
        cxx_name = id.mapID(name)
        flatname = string.join(astdecl.scopedName(), "_")

        value_name = self._fullname.fullyQualify()

        repoId = astdecl.repoId()
        idhash = hashval(repoId)

        boxedType   = types.Type(astdecl.boxedType())
        d_boxedType = boxedType.deref()

        marshal_content = output.StringStream()
        skutil.marshall(marshal_content, None, boxedType,
                        None, "_pd_boxed", "_0s")

        unmarshal_content = output.StringStream()
        skutil.unmarshall(unmarshal_content, None, boxedType,
                          None, "_pd_boxed", "_0s")

        copy_content = "_pd_boxed = _v->_pd_boxed;"
        destructor = ""

        if boxedType.dims():
            btype = boxedType.member(self._environment)
            copy_content = ("_pd_boxed = "
                            + btype + "_copyHelper::dup(_v._pd_boxed);")
            destructor = btype + "_copyHelper::free(_pd_boxed);"

        elif d_boxedType.string():
            copy_content = "_pd_boxed = _CORBA_String_helper::dup(_v->_pd_boxed);"
            destructor = "_CORBA_String_helper::free(_pd_boxed);"

        elif d_boxedType.wstring():
            copy_content = "_pd_boxed = _CORBA_WString_helper::dup(_v->_pd_boxed);"
            destructor = "_CORBA_WString_helper::free(_pd_boxed);"

        stream.out(valuebox_functions,
                   fqname=value_name, name=cxx_name, repoId=repoId,
                   idhash=idhash, marshal_content=marshal_content,
                   unmarshal_content=unmarshal_content,
                   copy_content=copy_content, destructor=destructor,
                   flatname=flatname)
