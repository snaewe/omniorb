// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_TypeCode.h           Created on: 2001/08/17
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
//    CORBA::TypeCode
//

/*
  $Log$
  Revision 1.1.4.3  2004/04/02 13:26:25  dgrisby
  Start refactoring TypeCode to support value TypeCodes, start of
  abstract interfaces support.

  Revision 1.1.4.2  2003/05/20 16:53:12  dgrisby
  Valuetype marshalling support.

  Revision 1.1.4.1  2003/03/23 21:04:23  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.4  2001/10/29 17:42:35  dpg1
  Support forward-declared structs/unions, ORB::create_recursive_tc().

  Revision 1.1.2.3  2001/10/17 16:44:00  dpg1
  Update DynAny to CORBA 2.5 spec, const Any exception extraction.

  Revision 1.1.2.2  2001/09/19 17:29:04  dpg1
  Cosmetic changes.

  Revision 1.1.2.1  2001/08/17 13:39:48  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
////////////////////////////// TypeCode //////////////////////////////
//////////////////////////////////////////////////////////////////////

// omniORB specific TypeCode support structures, used in stubs.
struct PR_structMember {
  const char*  name;
  TypeCode_ptr type;
};

#ifndef HAS_LongLong
typedef ULong PR_unionDiscriminator;
typedef Long  PR_unionDiscriminatorSigned;
#else
typedef ULongLong PR_unionDiscriminator;
typedef LongLong  PR_unionDiscriminatorSigned;
#endif

struct PR_unionMember {
  const char*           name;
  TypeCode_ptr          type;
  PR_unionDiscriminator label;
};

struct PR_valueMember {
  const char*  name;
  const char*  id;
  TypeCode_ptr type;
  _CORBA_Short access;
};

enum TCKind {
  tk_null                 = 0,
  tk_void                 = 1,
  tk_short                = 2,
  tk_long                 = 3,
  tk_ushort               = 4,
  tk_ulong                = 5,
  tk_float                = 6,
  tk_double               = 7,
  tk_boolean              = 8,
  tk_char                 = 9,
  tk_octet                = 10,
  tk_any                  = 11,
  tk_TypeCode             = 12,
  tk_Principal            = 13,
  tk_objref               = 14,
  tk_struct               = 15,
  tk_union                = 16,
  tk_enum                 = 17,
  tk_string               = 18,
  tk_sequence             = 19,
  tk_array                = 20,
  tk_alias                = 21,
  tk_except               = 22,
  tk_longlong             = 23,
  tk_ulonglong         	  = 24,
  tk_longdouble        	  = 25,
  tk_wchar                = 26,
  tk_wstring              = 27,
  tk_fixed                = 28,
  tk_value                = 29,
  tk_value_box            = 30,
  tk_native               = 31,
  tk_abstract_interface   = 32,
  tk_local_interface      = 33,

  _np_tk_indirect         = 0xffffffff
  // Indirection value used in TypeCode marshalling. Non-portable.
};

typedef short ValueModifier;

_CORBA_MODULE_VARINT
const ValueModifier VM_NONE        _init_in_decl_( = 0 );

_CORBA_MODULE_VARINT
const ValueModifier VM_CUSTOM      _init_in_decl_( = 1 );

_CORBA_MODULE_VARINT
const ValueModifier VM_ABSTRACT    _init_in_decl_( = 2 );

_CORBA_MODULE_VARINT
const ValueModifier VM_TRUNCATABLE _init_in_decl_( = 3 );

class TypeCode {
public:
  virtual ~TypeCode();

  TCKind kind() const;

  Boolean equal(TypeCode_ptr TCp) const;

  Boolean equivalent(TypeCode_ptr TCp) const;
  // CORBA 2.3 addition

  TypeCode_ptr get_compact_typecode() const;
  // CORBA 2.3 addition

  const char* id() const;
  const char* name() const;

  ULong member_count() const;
  const char* member_name(ULong index) const;

  TypeCode_ptr member_type(ULong i) const;

  Any* member_label(ULong i) const;
  TypeCode_ptr discriminator_type() const;
  Long default_index() const;

  ULong length() const;

  TypeCode_ptr content_type() const;

  UShort fixed_digits() const;
  Short fixed_scale() const;
    
  Long param_count() const;             // obsolete
  Any* parameter(Long index) const;     // obsolete

  Short member_visibility(ULong index) const;
  // Return type is really Visibility

  ValueModifier type_modifier() const;
  TypeCode_ptr concrete_base_type() const;

  static TypeCode_ptr _duplicate(TypeCode_ptr t);
  static TypeCode_ptr _nil();

  // OMG Interface:

  OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(Bounds, _dyn_attr)
  OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(BadKind, _dyn_attr)

  static void marshalTypeCode(TypeCode_ptr obj,cdrStream& s);
  static TypeCode_ptr unmarshalTypeCode(cdrStream& s);

  //
  // omniORB specific parts
  //

  class _Tracker {
  public:
    _Tracker(const char* file) : pd_file(file), pd_head(0) {}
    ~_Tracker();
    void add(TypeCode_ptr tc);
  private:
    const char*  pd_file;
    TypeCode_ptr pd_head;
  };

  // omniORB only static constructors
  // 1) These constructors are used by omniORB stubs & libraries to produce
  //    typecodes for complex types.  They should not be used in CORBA
  //    application code.
  // 2) Any typecode pointers passed into these functions are _duplicated
  //    before being saved into the relevant structures.
  // 3) The returned typecode pointers have reference counts of 1.

#if 0
  static TypeCode_ptr NP_struct_tc(const char* id, const char* name,
				   const StructMemberSeq& members);
  static TypeCode_ptr NP_exception_tc(const char* id, const char* name,
				      const StructMemberSeq& members);
  static TypeCode_ptr NP_union_tc(const char* id, const char* name,
				  TypeCode_ptr discriminator_type,
				  const UnionMemberSeq& members);
#endif
  static TypeCode_ptr NP_enum_tc(const char* id, const char* name,
				 const EnumMemberSeq& members);
  static TypeCode_ptr NP_alias_tc(const char* id, const char* name,
				  TypeCode_ptr original_type);
  static TypeCode_ptr NP_interface_tc(const char* id, const char* name);
  static TypeCode_ptr NP_string_tc(ULong bound);
  static TypeCode_ptr NP_wstring_tc(ULong bound);
  static TypeCode_ptr NP_fixed_tc(UShort digits, Short scale);
  static TypeCode_ptr NP_sequence_tc(ULong bound, TypeCode_ptr element_type);
  static TypeCode_ptr NP_array_tc(ULong length, TypeCode_ptr element_type);
  static TypeCode_ptr NP_recursive_sequence_tc(ULong bound, ULong offset);
  static TypeCode_ptr NP_recursive_tc(const char* id);
  static TypeCode_ptr NP_value_tc(const char* id, const char* name,
				  ValueModifier type_modifier,
				  TypeCode_ptr concrete_base,
				  const ValueMemberSeq& members);
  static TypeCode_ptr NP_value_box_tc(const char* id, const char* name,
				      TypeCode_ptr boxed_type);
  static TypeCode_ptr NP_native_tc(const char* id, const char* name);
  static TypeCode_ptr NP_abstract_interface_tc(const char* id,
					       const char* name);
  static TypeCode_ptr NP_local_interface_tc(const char* id, const char* name);


  // omniORB only static constructors for stubs. Either allocate a new
  // TypeCode or return an existing one. Calling module releases via
  // the tracker.

  // TypeCodes for named types. Calling module must release.
  static TypeCode_ptr PR_struct_tc(const char* id, const char* name,
				   const PR_structMember* members,
				   ULong memberCount, _Tracker* tracker);
  static TypeCode_ptr PR_exception_tc(const char* id, const char* name,
				      const PR_structMember* members,
				      ULong memberCount, _Tracker* tracker);
  static TypeCode_ptr PR_union_tc(const char* id, const char* name,
				  TypeCode_ptr discriminator_type,
				  const PR_unionMember* members,
				  ULong memberCount, Long deflt,
				  _Tracker* tracker);
  static TypeCode_ptr PR_enum_tc(const char* id, const char* name,
				 const char** members, ULong memberCount,
				 _Tracker* tracker);
  static TypeCode_ptr PR_alias_tc(const char* id, const char* name,
				  TypeCode_ptr original_type,
				  _Tracker* tracker);
  static TypeCode_ptr PR_interface_tc(const char* id, const char* name,
				      _Tracker* tracker);

  static TypeCode_ptr PR_value_tc(const char* id, const char* name,
				  ValueModifier type_modifier,
				  TypeCode_ptr concrete_base,
				  const PR_valueMember* members,
				  _Tracker* tracker);

  static TypeCode_ptr PR_value_box_tc(const char* id, const char* name,
				      TypeCode_ptr boxed_type,
				      _Tracker* tracker);
  static TypeCode_ptr PR_abstract_interface_tc(const char* id,
					       const char* name,
					       _Tracker* tracker);
  static TypeCode_ptr PR_local_interface_tc(const char* id, const char* name,
					    _Tracker* tracker);

  static TypeCode_ptr PR_forward_tc(const char* id, _Tracker* tracker);

  // TypeCodes for anonymous types.
  static TypeCode_ptr PR_string_tc(ULong bound, _Tracker* tracker);
  static TypeCode_ptr PR_wstring_tc(ULong bound, _Tracker* tracker);
  static TypeCode_ptr PR_fixed_tc(UShort digits, UShort scale,
				  _Tracker* tracker);
  static TypeCode_ptr PR_sequence_tc(ULong bound, TypeCode_ptr element_type,
				     _Tracker* tracker);
  static TypeCode_ptr PR_array_tc(ULong length, TypeCode_ptr element_type,
				  _Tracker* tracker);
  static TypeCode_ptr PR_recursive_sequence_tc(ULong bound, ULong offset,
					       _Tracker* tracker);

  // Static base TypeCodes. Caller does not release.
  static TypeCode_ptr PR_null_tc();
  static TypeCode_ptr PR_void_tc();
  static TypeCode_ptr PR_short_tc();
  static TypeCode_ptr PR_long_tc();
  static TypeCode_ptr PR_ushort_tc();
  static TypeCode_ptr PR_ulong_tc();
  static TypeCode_ptr PR_float_tc();
  static TypeCode_ptr PR_double_tc();
  static TypeCode_ptr PR_boolean_tc();
  static TypeCode_ptr PR_char_tc();
  static TypeCode_ptr PR_wchar_tc();
  static TypeCode_ptr PR_octet_tc();
  static TypeCode_ptr PR_any_tc();
  static TypeCode_ptr PR_TypeCode_tc();
  static TypeCode_ptr PR_Principal_tc();
  static TypeCode_ptr PR_Object_tc();
  static TypeCode_ptr PR_string_tc();
  static TypeCode_ptr PR_wstring_tc();
#ifdef HAS_LongLong
  static TypeCode_ptr PR_longlong_tc();
  static TypeCode_ptr PR_ulonglong_tc();
#endif
#ifdef HAS_LongDouble
  static TypeCode_ptr PR_longdouble_tc();
#endif

  // omniORB internal functions
  virtual CORBA::Boolean NP_is_nil() const;

  static inline _CORBA_Boolean PR_is_valid(TypeCode_ptr p ) {
    return ((p) ? (p->pd_magic == PR_magic) : 1);
  }
  static _dyn_attr const CORBA::ULong PR_magic;

protected:
  // These operators are placed here to avoid them being used externally
  TypeCode(const TypeCode& tc);
  TypeCode& operator=(const TypeCode& tc);
  TypeCode() { pd_magic = PR_magic; };

  _CORBA_ULong pd_magic;
};


//////////////////////////////////////////////////////////////////////
/////////////////////// TypeCodes of Primitives //////////////////////
//////////////////////////////////////////////////////////////////////

_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_null;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_void;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_short;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_long;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_ushort;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_ulong;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_float;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_double;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_boolean;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_char;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_wchar;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_octet;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_any;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_TypeCode;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_Principal;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_Object;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_string;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_wstring;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_NamedValue;
#ifdef HAS_LongLong
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_longlong;
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_ulonglong;
#endif
#ifdef HAS_LongDouble
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_longdouble;
#endif


//////////////////////////////////////////////////////////////////////
/////////////////////// TypeCodes of System Exceptions ///////////////
//////////////////////////////////////////////////////////////////////

#define DECLARE_SYS_EXC_TYPECODE(name) \
_CORBA_MODULE_VAR _dyn_attr TypeCode_ptr _tc_##name;

OMNIORB_FOR_EACH_SYS_EXCEPTION(DECLARE_SYS_EXC_TYPECODE)
#undef DECLARE_SYS_EXC_TYPECODE
