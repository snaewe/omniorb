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
typedef _CORBA_Value_Var<@name@>    @name@_var;
typedef _CORBA_Value_Member<@name@> @name@_member;

#endif // __@guard@__
"""

value_class = """\
class @name@ : 
  @inherits@
{
public:  
  // Standard mapping
  typedef @name@_var _var_type;

  static @name@* _downcast (CORBA::ValueBase*);

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

  static void _NP_marshal(const @name@*, cdrStream&);
  static @name@* _NP_unmarshal(cdrStream&);

  virtual void _PR_marshal_state(cdrStream&) const;
  virtual void _PR_unmarshal_state(cdrStream&);
  virtual void _PR_copy_state(@name@*);

  static _dyn_attr const char* _PD_repoId;

protected:
  @name@();
  virtual ~@name@();

private:
  // Not implemented
  @name@(const @name@ &);
  void operator=(const @name@ &);
};
"""

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


value_obv_class = """\
class @obv_name@ :
  @inherits@
{
@constructor_access@:
  @obv_name@();
  @obv_name@(@init_params@);
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
@fqname@::_NP_marshal(const @fqname@* _v, cdrStream& _0s)
{
  omniValueType::marshal(_v, @fqname@::_PD_repoId, _0s);
}

@fqname@*
@fqname@::_NP_unmarshal(cdrStream& _0s)
{
  CORBA::ValueBase* _b = omniValueType::unmarshal(@fqname@::_PD_repoId,
						  @idhash@U, _0s);
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

value_obv_functions = """\
OBV_@fqname@::@name@() {}
OBV_@fqname@::~@name@() {}
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
  // return new OBV_@fqname@();

  //?? The standard C++ mapping requires the above code, but
  // it is invalid because the reference counting functions in
  // CORBA::ValueBase are not implemented.

  return 0;
}
"""

statemember_copy = """\
@name@(_v->@name@());"""

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

        mType = types.Type(mtype)
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

        elif mType.typedef() or d_mType.struct() or d_mType.union():
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

        elif d_mType.value():
            otype = mType.base()

            base_s.out(statemember_value_sig,
                       name = member,
                       type = mType.base(environment),
                       abs = "= 0")

            obv_s.out(statemember_value_sig,
                      name = member,
                      type = otype,
                      abs = "")

            copy_s.out(statemember_copy, name=member)
            marshal_s.out(statemember_value_marshal, name=member, type=otype)
            unmarshal_s.out(statemember_value_unmarshal,
                            name=member, type=otype)
            init_s.out(statemember_init, name=member)

            impl_s.out(statemember_value_impl, name=member, type=otype,
                       value_name=value_name)

            decl._cxx_holder   = omemtype + " _pd_" + member + ";"
            decl._cxx_init_arg = omemtype + "* _" + member

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

        has_callables = 0
        has_factories = 0

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

        inherits = string.join(inheritl, ",\n")

        astdecl._cxx_has_callables = has_callables
        astdecl._cxx_has_factories = has_factories

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
        for c in astdecl.callables():
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

        stream.out(value_forward, name=cxx_name, guard=guard)

        stream.out(value_class, name=cxx_name, inherits=inherits,
                   public_accessors=public_accessors,
                   private_accessors=private_accessors,
                   operations=operations, other_idl=gen_other_idl)

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


    def poa_module_decls(self, stream, visitor):
        # *** POA module defs if supports an interface
        pass

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
            isname = v.scopedName()
            irname = id.Name(isname).relName(self._environment)
            if irname is None:
                iname = "::OBV_" + string.join(isname, "::")
            elif irname == isname:
                iname = "OBV_" + string.join(irname, "::")
            else:
                iname = string.join(irname, "::")

            inheritl.append("public virtual " + iname)

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

        stream.out(value_obv_class, obv_name=obv_name,
                   inherits=inherits,
                   init_params=init_params,
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

        if not self._abstract:
            stream.out(value_obv_functions, fqname=value_name, name=cxx_name,
                       init_params=init_params, base_init=base_init,
                       member_initialisers=member_initialisers)

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


class ValueBox (mapping.Decl):

    pass


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

