# -*- python -*-
#                           Package   : omniidl
# template.py               Created on: 2000/01/18
#			    Author    : David Scott (djs)
#
#    Copyright (C) 1999 AT&T Laboratories Cambridge
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
#   
#   C++ templates for the .hh file

# $Id$
# $Log$
# Revision 1.1  2000/01/18 18:05:53  djs
# Extracted most C++ from header/defs and put in a template file.
# General refactoring.
#

"""C++ templates for the .hh file"""

##
## Modules
##
# name => C++ form of the module identifier
module_begin = """\
_CORBA_MODULE @name@

_CORBA_MODULE_BEG
"""
module_end = """\
_CORBA_MODULE_END
"""

##
## Interfaces
##
interface_begin = """\
#ifndef __@guard@__
#define __@guard@__

class @name@;
class _objref_@name@;
class _impl_@name@;
typedef _objref_@name@* @name@_ptr;
typedef @name@_ptr @name@Ref;

class @name@_Helper {
public:
  typedef @name@_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static size_t NP_alignedSize(_ptr_type, size_t);
  static void marshalObjRef(_ptr_type, NetBufferedStream&);
  static _ptr_type unmarshalObjRef(NetBufferedStream&);
  static void marshalObjRef(_ptr_type, MemBufferedStream&);
  static _ptr_type unmarshalObjRef(MemBufferedStream&);
};

typedef _CORBA_ObjRef_Var<_objref_@name@, @name@_Helper> @name@_var;
typedef _CORBA_ObjRef_OUT_arg<_objref_@name@,@name@_Helper > @name@_out;

#endif
"""

interface_type = """\
class @name@ {
public:
  // Declarations for this interface type.
  typedef @name@_ptr _ptr_type;
  typedef @name@_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(CORBA::Object_ptr);
  static _ptr_type _nil();

  static inline size_t _alignedSize(_ptr_type, size_t);
  static inline void _marshalObjRef(_ptr_type, NetBufferedStream&);
  static inline void _marshalObjRef(_ptr_type, MemBufferedStream&);

  static inline _ptr_type _unmarshalObjRef(NetBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static inline _ptr_type _unmarshalObjRef(MemBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static _core_attr const char* _PD_repoId;

  // Other IDL defined within this scope.
  @Other_IDL@
};
"""

interface_objref = """\
class _objref_@name@ :
  @inherits@
{
public:
  @operations@
  @attributes@

  inline _objref_@name@() { _PR_setobj(0); }  // nil
  _objref_@name@(const char*, IOP::TaggedProfileList*, omniIdentity*, omniLocalIdentity*);

protected:
  virtual ~_objref_@name@();

private:
  virtual void* _ptrToObjRef(const char*);

  _objref_@name@(const _objref_@name@&);
  _objref_@name@& operator = (const _objref_@name@&);
  // not implemented
};
"""

interface_pof = """\
class _pof_@name@ : public proxyObjectFactory {
public:
  inline _pof_@name@() : proxyObjectFactory(@name@::_PD_repoId) {}
  virtual ~_pof_@name@();

  virtual omniObjRef* newObjRef(const char*, IOP::TaggedProfileList*,
                                omniIdentity*, omniLocalIdentity*);
  virtual _CORBA_Boolean is_a(const char*) const;
};
"""

interface_impl = """\
class _impl_@name@ :
  @inherits@
{
public:
  virtual ~_impl_@name@();

  @virtual_operations@
  @virtual_attributes@

protected:
  virtual _CORBA_Boolean _dispatch(GIOP_S&);

private:
  virtual void* _ptrToInterface(const char*);
  virtual const char* _mostDerivedRepoId();
};

"""

interface_sk = """\
class _sk_@name@ :
  public virtual _impl_@name@,
  public virtual omniOrbBoaServant
{
public:
  virtual ~_sk_@name@();
  inline @name@::_ptr_type _this() {
    return (@name@::_ptr_type) omniOrbBoaServant::_this(@name@::_PD_repoId);
  }
  inline void _obj_is_ready(CORBA::BOA_ptr) { omniOrbBoaServant::_obj_is_ready(); }
  inline CORBA::BOA_ptr _boa() { return CORBA::BOA::getBOA(); }
};
"""

##
## Typedefs
##
typedef_simple_to_array = """\
typedef @base@ @derived@;
typedef @base@_slice @derived@_slice;
typedef @base@_copyHelper @derived@_copyHelper;
typedef @base@_var @derived@_var;
typedef @base@_out @derived@_out;
typedef @base@_forany @derived@_forany;
"""

typedef_simple_to_array_static_fn = """\
static @derived@_slice* @derived@_alloc() { return @base@_alloc(); }
static @derived@_slice* @derived@_dup(const @derived@_slice* p) { return @base@_dup(p); }
static void @derived@_copy( @derived@_slice* _to, const @derived@_slice* _from ) { @base@_copy(_to, _from); }
static void @derived@_free( @derived@_slice* p) { @base@_free(p); }
"""

typedef_simple_to_array_extern = """\
extern @derived@_slice* @derived@_alloc();
extern @derived@_slice* @derived@_dup(const @derived@_slice* p);
extern void @derived@_copy( @derived@_slice* _to, const @derived@_slice* _from );
extern void @derived@_free( @derived@_slice* p);
"""

typedef_simple_string = """\
typedef char* @name@;
typedef CORBA::String_var @name@_var;
"""

typedef_simple_typecode = """\
typedef CORBA::TypeCode_ptr @name@_ptr;
typedef CORBA::TypeCode_var @name@_var;
"""

typedef_simple_any = """\
typedef CORBA::Any @name@;
typedef CORBA::Any_var @name@_var;
"""

typedef_simple_basic = """\
typedef @base@ @derived@;
"""

typedef_simple_constructed = """\
typedef @base@ @name@;
typedef @base@_var @name@_var;
typedef @base@_out @name@_out;
"""

typedef_simple_objref = """\
typedef @base@ @name@;
typedef @base@_ptr @name@_ptr;
typedef @base@Ref @name@Ref;
@impl_base@
typedef @base@_Helper @name@_Helper;
@objref_base@
typedef @base@_var @name@_var;
typedef @base@_out @name@_out;
"""

typedef_enum_oper_friend = """\
// Need to declare <<= for elem type, as GCC expands templates early
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
 @friend@ inline void operator >>= (@element@, NetBufferedStream&);
 @friend@ inline void operator <<= (@element@&, NetBufferedStream&);
 @friend@ inline void operator >>= (@element@, MemBufferedStream&);
 @friend@ inline void operator <<= (@element@&, MemBufferedStream&);
#endif
"""

# Arrays

typedef_array = """\
typedef @type@ @name@@dims@;
typedef @type@ @name@_slice@taildims@;
"""

# this is almost the same as typedef_simple_to_array_extern above
typedef_array_extern = """\
extern @name@_slice* @name@_alloc();
extern @name@_slice* @name@_dup(const @name@_slice* _s);
extern void @name@_free(@name@_slice* _s);
extern void @name@_copy(@name@_slice* _to, const @name@_slice* _from);
"""

typedef_array_static = """\
static inline @name@_slice* @name@_alloc() {
  return new @name@_slice[@firstdim@];
}

static inline @name@_slice* @name@_dup(const @name@_slice* _s) {
  if (!_s) return 0;
  @name@_slice* _data = @name@_alloc();
  if (_data) {
    @dup_loop@
  }
  return _data;
}

static inline void @name@_copy(@name@_slice* _to, const @name@_slice* _from){
  @copy_loop@
}

static inline void @name@_free(@name@_slice* _s) {
    delete [] _s;
}
"""

typedef_array_copyHelper = """\
class @name@_copyHelper {
public:
  static inline @name@_slice* alloc() { return @name@_alloc(); }
  static inline @name@_slice* dup(const @name@_slice* p) { return @name@_dup(p); }
  static inline void free(@name@_slice* p) { @name@_free(p); }
};

typedef _CORBA_Array_Var<@name@_copyHelper,@name@_slice> @name@_var;
typedef _CORBA_Array_OUT_arg<@name@_slice,@name@_var > @name@_out;
typedef _CORBA_Array_Forany<@name@_copyHelper,@name@_slice> @name@_forany;
"""

##
## Sequences
##
sequence_type = """\
class @name@_var;

class @name@ : public @derived@ {
public:
  typedef @name@_var _var_type;
  inline @name@() {}
  inline @name@(const @name@& s)
    : @derived@(s) {}

  @bounds@

  inline @name@& operator = (const @name@& s) {
    @derived@::operator=(s);
    return *this;
  }
};
"""

sequence_type_bounds = """\
inline @name@(_CORBA_ULong _max)
  : @derived@(_max) {}
inline @name@(_CORBA_ULong _max, _CORBA_ULong _len, @element@* _val, _CORBA_Boolean _rel=0)
  : @derived@(_max, _len, _val, _rel) {}
"""

sequence_var_array_subscript = """\
inline @element@_slice* operator [] (_CORBA_ULong s) {
  return (@element@_slice*) ((pd_seq->NP_data())[s]);
}
"""

sequence_var_subscript = """\
inline @element@ operator [] (_CORBA_ULong s) {
  return (*pd_seq)[s];
}
"""

sequence_var = """\
class @name@_out;

class @name@_var {
public:
  typedef @name@ T;
  typedef @name@_var T_var;
    
  inline @name@_var() : pd_seq(0) {}
  inline @name@_var(T* s) : pd_seq(s) {}
  inline @name@_var(const T_var& s) {
    if( s.pd_seq )  pd_seq = new T(*s.pd_seq);
    else             pd_seq = 0;
  }
  inline ~@name@_var() { if( pd_seq )  delete pd_seq; }
    
  inline T_var& operator = (T* s) {
    if( pd_seq )  delete pd_seq;
    pd_seq = s;
    return *this;
  }
  inline T_var& operator = (const T_var& s) {
    if( s.pd_seq ) {
      if( !pd_seq )  pd_seq = new T;
      *pd_seq = *s.pd_seq;
    } else if( pd_seq ) {
      delete pd_seq;
      pd_seq = 0;
    }
    return *this;
  }

  @subscript_operator@

  inline T* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
  inline operator T& () const { return *pd_seq; }
#else
  inline operator const T& () const { return *pd_seq; }
  inline operator T& () { return *pd_seq; }
#endif
    
  inline const T& in() const { return *pd_seq; }
  inline T&       inout()    { return *pd_seq; }
  inline T*&      out() {
    if( pd_seq ) { delete pd_seq; pd_seq = 0; }
    return pd_seq;
  }
  inline T* _retn() { T* tmp = pd_seq; pd_seq = 0; return tmp; }
    
  friend class @name@_out;
  
private:
  T* pd_seq;
};
"""

sequence_out_array_subscript = """\
inline @element@_slice* operator [] (_CORBA_ULong i) {
  return (@element@_slice*) ((_data->NP_data())[i]);
}
"""

sequence_out_subscript = """\
inline @element@ operator [] (_CORBA_ULong i) {
  return (*_data)[i];
}
"""

sequence_out = """\
class @name@_out {
public:
  typedef @name@ T;
  typedef @name@_var T_var;

  inline @name@_out(T*& s) : _data(s) { _data = 0; }
  inline @name@_out(T_var& s)
    : _data(s.pd_seq) { s = (T*) 0; }
  inline @name@_out(const @name@_out& s) : _data(s._data) {}
  inline @name@_out& operator = (const @name@_out& s) {
    _data = s._data;
    return *this;
  }  inline @name@_out& operator = (T* s) {
    _data = s;
    return *this;
  }
  inline operator T*&()  { return _data; }
  inline T*& ptr()       { return _data; }
  inline T* operator->() { return _data; }

  @subscript_operator@

  T*& _data;
  
private:
  @name@_out();
  @name@_out& operator=(const T_var&);
};
"""

##
## Structs
##

struct = """\
struct @name@ {
  typedef _CORBA_ConstrType_@type@_Var<@name@> _var_type;

  @Other_IDL@

  @members@

  size_t _NP_alignedSize(size_t initialoffset) const;
  void operator>>= (NetBufferedStream &) const;
  void operator<<= (NetBufferedStream &);
  void operator>>= (MemBufferedStream &) const;
  void operator<<= (MemBufferedStream &);
};

typedef @name@::_var_type @name@_var;

typedef _CORBA_ConstrType_@type@_OUT_arg< @name@,@name@_var > @name@_out;
"""

struct_nonarray_sequence = """\
typedef @memtype@ _@cxx_id@_seq;
_@cxx_id@_seq @cxx_id@;
"""

struct_normal_member = """\
@memtype@ @cxx_id@@dims@;
"""


##
## Exceptions
##

exception = """\
class @name@ : public CORBA::UserException {
public:

  @Other_IDL@

  @members@

  inline @name@() {
    pd_insertToAnyFn    = insertToAnyFn;
    pd_insertToAnyFnNCP = insertToAnyFnNCP;
  }
  @name@(const @name@&);
  @constructor@
  @name@& operator=(const @name@&);
  virtual ~@name@();
  virtual void _raise();
  static @name@* _downcast(CORBA::Exception*);
  static const @name@* _downcast(const CORBA::Exception*);
  static inline @name@* _narrow(CORBA::Exception* e) {
    return _downcast(e);
  }
  
  @alignedSize@

  @inline@ void operator>>=(NetBufferedStream&) const @body@
  @inline@ void operator>>=(MemBufferedStream&) const @body@
  @inline@ void operator<<=(NetBufferedStream&) @body@
  @inline@ void operator<<=(MemBufferedStream&) @body@

  static _core_attr insertExceptionToAny    insertToAnyFn;
  static _core_attr insertExceptionToAnyNCP insertToAnyFnNCP;

  static _core_attr const char* _PD_repoId;

private:
  virtual CORBA::Exception* _NP_duplicate() const;
  virtual const char* _NP_typeId() const;
  virtual const char* _NP_repoId(int*) const;
  virtual void _NP_marshal(NetBufferedStream&) const;
  virtual void _NP_marshal(MemBufferedStream&) const;
};
"""  


exception_array_declarator = """\
typedef @memtype@ @private_prefix@_@cxx_id@@dims@;
typedef @memtype@ _@cxx_id@_slice;
"""

exception_member = """\
@memtype@ @cxx_id@@dims@;
"""

##
## Unions
##

union_ctor_nonexhaustive = """\
if ((pd__default = _value.pd__default)) {
  pd__d = _value.pd__d;
  @default@
}
else {
  switch(_value.pd__d) {
    @cases@
  }
}"""

union_ctor_exhaustive = """\
switch(_value.pd__d) {
  @cases@
}"""

union_ctor_case = """\
case @discrimvalue@: @name@(_value.pd_@name@); break;
"""

union_ctor_bool_default = """\
#ifndef HAS_Cplusplus_Bool
  default: break;
#endif
"""

union_ctor_default = """\
  default: break;
"""

union = """\
class @unionname@ {
public:

  typedef _CORBA_ConstrType_@fixed@_Var<@unionname@> _var_type;

  @Other_IDL@

  @unionname@() {
    @default_constructor@
  }
  
  @unionname@(const @unionname@& _value) {
    @copy_constructor@
  }

  ~@unionname@() {}

  @unionname@& operator=(const @unionname@& _value) {
    @copy_constructor@
    return *this;
  }

  @discrimtype@ _d() const { return pd__d;}
  void _d(@discrimtype@ _value) {}

  @implicit_default@

  @members@
  
  size_t _NP_alignedSize(size_t initialoffset) const;
  void operator>>= (NetBufferedStream&) const;
  void operator<<= (NetBufferedStream&);
  void operator>>= (MemBufferedStream&) const;
  void operator<<= (MemBufferedStream&);

  @tcParser_unionHelper@

private:
  @discrimtype@ pd__d;
  CORBA::Boolean pd__default;

  @union@
  @outsideUnion@
};

typedef @unionname@::_var_type @unionname@_var;
typedef _CORBA_ConstrType_@fixed@_OUT_arg< @unionname@,@unionname@_var > @unionname@_out;
"""
union_union = """\
union {
  @members@
};
"""

union_constructor_implicit = """\
_default();
"""
union_constructor_default = """\
pd__default = 1;
pd__d = @default@;
"""
union_implicit_default = """\
void _default()
{
  pd__d = @arbitraryDefault@;
  pd__default = 1;
}
"""

union_tcParser_friend = """\
#if defined(__GNUG__) || defined(__DECCXX) && (__DECCXX_VER < 60000000)
    friend class @private_prefix@_tcParser_unionhelper_@name@;
#else
    friend class ::@private_prefix@_tcParser_unionhelper_@name@;
#endif
"""

union_proxy_float = """\
#ifdef USING_PROXY_FLOAT
  @type@ pd_@name@@dims@;
#endif
"""

union_noproxy_float = """\
#ifndef USING_PROXY_FLOAT
  @type@ pd_@name@@dims@;
#endif
"""


union_array_declarator = """\
typedef @memtype@ @prefix@_@name@@dims@;
typedef @memtype@ _@name@_slice@tail_dims@;
"""

union_array = """\
const @memtype@_slice *@name@ () const { return pd_@name@; }
void @name@ (const @const_type@ _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  @loop@
}"""

union_any = """\
const @type@ &@name@ () const { return pd_@name@; }
@type@ &@name@ () { return pd_@name@; }
void @name@ (const @type@& _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@name@ = _value;
}"""

union_typecode = """\
CORBA::TypeCode_ptr @name@ () const { return pd_@name@._ptr; }
void @name@(CORBA::TypeCode_ptr _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@name@ = CORBA::TypeCode::_duplicate(_value);
}
void @name@(const CORBA::TypeCode_member& _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@name@ = _value;
}
void @name@(const CORBA::TypeCode_var& _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@name@ = _value;
}"""

union_basic = """\
@type@ @name@ () const { return pd_@name@; }
void @name@ (@type@  _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@name@ = _value;
}"""

union_string = """\
const char * @name@ () const { return (const char*) pd_@name@; }
void @name@(char* _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@name@ = _value;
}
void @name@(const char*  _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@name@ = _value;
}
void @name@(const CORBA::String_var& _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@name@ = _value;
}
void @name@(const CORBA::String_member& _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@name@ = _value;
}"""

union_objref = """\
@ptr_name@ @member@ () const { return pd_@member@._ptr; }
void @member@(@ptr_name@ _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  @Helper_name@::duplicate(_value);
  pd_@member@ = _value;
}
void @member@(const @memtype@& _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@member@ = _value;
}
void @member@(const @var_name@&  _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@member@ = _value;
}
"""

union_constructed = """\
const @type@ &@name@ () const { return pd_@name@; }
@type@ &@name@ () { return pd_@name@; }
void @name@ (const @type@& _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@name@ = _value;
}
"""

union_sequence = """\
typedef @sequence_template@ _@member@_seq;
const _@member@_seq& @member@ () const { return pd_@member@; }
_@member@_seq& @member@ () { return pd_@member@; }
void @member@ (const _@member@_seq& _value) {
  pd__d = @discrimvalue@;
  pd__default = @isDefault@;
  pd_@member@ = _value;
}
"""

union_member = """\
@type@ pd_@name@@dims@;
"""

##
## Enum
##
enum = """\
enum @name@ { @memberlist@ };
typedef @name@& @name@_out;
"""

##
## Const
##
const_inclass_isinteger = """\
static _core_attr const @type@ @name@ _init_in_cldecl_( = @val@ );
"""
const_inclass_notinteger = """\
static _core_attr const @type@ @name@;
"""
const_outsideclass_isinteger = """\
_CORBA_@where@_VARINT const @type@ @name@ _init_in_decl_( = @val@ );
"""
const_outsideclass_notinteger = """\
_CORBA_@where@_VAR _core_attr const @type@ @name@;
"""

##
## Typecode_ptr
##
typecode = """\
@qualifier@ _dyn_attr const CORBA::TypeCode_ptr _tc_@name@;
"""
