// -*- Mode: C++; -*-
//                            Package   : omniORB
// typecode.cc                Created on: 03/09/98
//                            Author1   : James Weatherall (jnw)
//                            Author2   : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2004 Apasphere Ltd.
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
//      Implementation of the CORBA::TypeCode psuedo object
//

/*
 * $Log$
 * Revision 1.40.2.2  2004/04/02 13:26:23  dgrisby
 * Start refactoring TypeCode to support value TypeCodes, start of
 * abstract interfaces support.
 *
 * Revision 1.40.2.1  2003/03/23 21:02:44  dgrisby
 * Start of omniORB 4.1.x development branch.
 *
 * Revision 1.38.2.30  2003/03/10 11:13:52  dgrisby
 * BAD_PARAM with invalid fixed limits.
 *
 * Revision 1.38.2.29  2003/03/05 15:26:54  dgrisby
 * Missing Fixed typecode unmarshal. Thanks Renzo Tomaselli.
 *
 * Revision 1.38.2.28  2002/12/18 15:59:15  dgrisby
 * Proper clean-up of recursive TypeCodes.
 *
 * Revision 1.38.2.27  2002/12/10 17:17:07  dgrisby
 * Yet another indirection problem.
 *
 * Revision 1.38.2.26  2002/12/05 12:22:22  dgrisby
 * More indirection problems.
 *
 * Revision 1.38.2.25  2002/09/06 14:35:55  dgrisby
 * Work around long long literal bug in MSVC.
 *
 * Revision 1.38.2.24  2002/02/25 11:17:12  dpg1
 * Use tracedmutexes everywhere.
 *
 * Revision 1.38.2.23  2002/01/16 11:31:57  dpg1
 * Race condition in use of registerNilCorbaObject/registerTrackedObject.
 * (Reported by Teemu Torma).
 *
 * Revision 1.38.2.22  2001/11/01 12:04:56  dpg1
 * Don't return void in void function.
 *
 * Revision 1.38.2.21  2001/10/29 17:42:36  dpg1
 * Support forward-declared structs/unions, ORB::create_recursive_tc().
 *
 * Revision 1.38.2.20  2001/10/17 16:44:03  dpg1
 * Update DynAny to CORBA 2.5 spec, const Any exception extraction.
 *
 * Revision 1.38.2.19  2001/09/24 10:41:09  dpg1
 * Minor codes for Dynamic library and omniORBpy.
 *
 * Revision 1.38.2.18  2001/09/19 17:26:45  dpg1
 * Full clean-up after orb->destroy().
 *
 * Revision 1.38.2.17  2001/08/29 13:41:03  dpg1
 * jnw's fix for compilers with variable sizeof(enum)
 *
 * Revision 1.38.2.16  2001/08/17 17:09:16  sll
 * Modularise ORB configuration parameters.
 *
 * Revision 1.38.2.15  2001/08/17 13:47:30  dpg1
 * Small bug fixes.
 *
 * Revision 1.38.2.14  2001/07/25 13:39:46  dpg1
 * Missing wstring case in TypeCode unmarshalling.
 *
 * Revision 1.38.2.13  2001/06/13 20:10:04  sll
 * Minor update to make the ORB compiles with MSVC++.
 *
 * Revision 1.38.2.12  2001/06/08 17:12:10  dpg1
 * Merge all the bug fixes from omni3_develop.
 *
 * Revision 1.38.2.11  2001/04/19 09:14:12  sll
 * Scoped where appropriate with the omni namespace.
 *
 * Revision 1.38.2.10  2001/03/13 10:32:06  dpg1
 * Fixed point support.
 *
 * Revision 1.38.2.9  2000/12/05 17:41:00  dpg1
 * New cdrStream functions to marshal and unmarshal raw strings.
 *
 * Revision 1.38.2.8  2000/11/22 14:39:01  dpg1
 * Missed out PR_wstring_tc() function.
 *
 * Revision 1.38.2.7  2000/11/20 14:40:04  sll
 * Added TypeCode::PR_wstring_tc(CORBA::ULong bound).
 *
 * Revision 1.38.2.6  2000/11/17 19:09:38  dpg1
 * Support codeset conversion in any.
 *
 * Revision 1.38.2.5  2000/11/09 12:27:54  dpg1
 * Huge merge from omni3_develop, plus full long long from omni3_1_develop.
 *
 * Revision 1.38.2.4  2000/11/03 19:07:32  sll
 * Use new marshalling functions for byte, octet and char. Use get_octet_array
 * instead of get_char_array.
 *
 * Revision 1.38.2.3  2000/10/06 16:40:54  sll
 * Changed to use cdrStream.
 *
 * Revision 1.38.2.2  2000/09/27 17:25:44  sll
 * Changed include/omniORB3 to include/omniORB4.
 *
 * Revision 1.38.2.1  2000/07/17 10:35:42  sll
 * Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
 *
 * Revision 1.39  2000/07/13 15:26:02  dpg1
 * Merge from omni3_develop for 3.0 release.
 *
 * Revision 1.33.6.10  2000/06/27 16:23:25  sll
 * Merged OpenVMS port.
 *
 * Revision 1.33.6.9  2000/03/20 15:09:30  djr
 * Fixed signed/unsigned mismatch.
 *
 * Revision 1.33.6.8  2000/02/21 10:59:13  djr
 * Another TypeCode_union w aliased discriminator fix.
 *
 * Revision 1.33.6.7  2000/02/17 14:43:57  djr
 * Another fix for TypeCode_union when discriminator tc contains an alias.
 *
 * Revision 1.33.6.6  2000/02/15 13:43:42  djr
 * Fixed bug in create_union_tc() -- problem if discriminator was an alias.
 *
 * Revision 1.33.6.5  1999/10/29 13:18:12  djr
 * Changes to ensure mutexes are constructed when accessed.
 *
 * Revision 1.33.6.4  1999/10/26 20:18:20  sll
 * DynAny no longer do alias expansion on the typecode. In other words, all
 * aliases in the typecode are preserved.
 *
 * Revision 1.33.6.3  1999/10/14 17:31:31  djr
 * Minor corrections.
 *
 * Revision 1.33.6.2  1999/10/14 16:22:01  djr
 * Implemented logging when system exceptions are thrown.
 *
 * Revision 1.33.6.1  1999/09/22 14:26:37  djr
 * Major rewrite of orbcore to support POA.
 *
 * Revision 1.32  1999/08/20 11:41:12  djr
 * Yet another TypeCode alias-expand bug.
 *
 * Revision 1.31  1999/08/06 16:55:17  sll
 * Added missing break statement in extractLabel. This bug affects enum
 * discriminator.
 *
 * Revision 1.30  1999/07/05 09:29:34  sll
 * member_label should return an octet if it is the default member.
 *
 * Revision 1.29  1999/07/01 10:27:38  djr
 * Fixed NP_aliasExpand().
 * Added omg.org to a few IR repo IDs.
 *
 * Revision 1.28  1999/06/22 15:02:07  sll
 * Corrected bug in TypeCode_alias::NP_extendedEqual.
 *
 * Revision 1.27  1999/06/18 21:00:22  sll
 * Updated to CORBA 2.3 mapping.
 *
 * Revision 1.26  1999/05/25 17:54:48  sll
 * Added check for invalid arguments using magic numbers.
 * Perform casting of integer label values in union members.
 *
 * Revision 1.25  1999/04/21 13:24:57  djr
 * Fixed bug in generation of typecode alignment tables.
 *
 * Revision 1.24  1999/03/19 15:15:39  djr
 * Now accept indirections to fundamental TypeCodes. Option to accept
 * misaligned indirections.
 *
 * Revision 1.23  1999/03/11 16:25:59  djr
 * Updated copyright notice
 *
 * Revision 1.22  1999/03/04 09:54:24  djr
 * Disabled cached parameter lists - they have a bug.
 *
 * Revision 1.22  1999/02/26 09:57:51  djr
 * Disabled cached parameter lists, as they have a bug.
 *
 * Revision 1.21  1999/02/23 11:46:07  djr
 * Fixed bugs in size calculation for TypeCodes.
 *
 * Revision 1.21  1999/02/22 09:32:34  djr
 * Bug in size calculation for array and sequence TypeCodes.
 *
 * Revision 1.20  1999/02/18 15:51:23  djr
 * Option to not use indirections in on-the-wire TypeCodes.
 *
 * Revision 1.19  1999/02/12 11:52:12  djr
 * Typecodes for arrays were marshalled/unmarshalled incorrectly.
 *
 * Revision 1.18  1999/02/09 17:45:34  djr
 * Fixed bug in TypeCode_alignTable generation for structures and exceptions.
 *
 * Revision 1.17  1999/02/08 18:55:45  djr
 * Fixed bug in marshalling of TypeCodes for sequences. The sequence
 * bound and the content TypeCode were marshalled in the wrong order.
 *
 * Revision 1.16  1999/01/18 13:54:51  djr
 * Fixed bugs in implementation of unions.
 *
 * Revision 1.15  1999/01/11 15:45:23  djr
 * New implementation.
 *
 * Revision 1.14  1998/09/03 17:38:23  sll
 * CORBA::TypeCode::_nil() called the wrong ctor when compiled with DEC cxx
 * 5.5. fixed.
 *
 * Revision 1.13  1998/08/19 15:46:03  sll
 * operator<<= (CORBA::DefinitionKind,NetBufferedStream&) and friends are now
 * defined in the global scope. Previously they are defined in namespace COR
 * if the compiler support for namespace is used.
 *
 * Revision 1.12  1998/08/15 14:33:47  sll
 * Added NEED_DUMMY_RETURN macros to avoid better compiler to complain about
 * unreachable code.
 * Moved from CORBA.h inline member functions and operators.
 *
 * Revision 1.11  1998/08/14 13:55:01  sll
 * Added pragma hdrstop to control pre-compile header if the compiler feature
 * is available.
 *
 * Revision 1.10  1998/08/11 11:48:49  sll
 * Extended workaround in CORBA::TypeCode::_nil() to cover up to DEC Cxx
 * v5.5-015.
 *
 * Revision 1.9  1998/08/05 18:01:12  sll
 * Fixed bugs caused by typos in TypeCode::TypeCode(TCKind,ULong) and
 * TypeCode::_nil().
 *
 * Revision 1.8  1998/04/18 10:11:17  sll
 * Corrected signature of one TypeCode ctor.
 *
 * Revision 1.7  1998/04/08 16:07:50  sll
 * Minor change to help some compiler to find the right TypeCode ctor.
 *
 * Revision 1.6  1998/04/08 14:07:26  sll
 * Added workaround in CORBA::TypeCode::_nil() for a bug in DEC Cxx v5.5.
 *
 * Revision 1.5  1998/04/07 19:40:53  sll
 * Moved inline member functions to this module.
 *
 * Revision 1.4  1998/03/17 12:52:19  sll
 * Corrected typo.
 *
 * Revision 1.3  1998/03/17 12:12:31  ewc
 * Bug fix to NP_aliasExpand()
 *
// Revision 1.2  1998/02/20  14:45:43  ewc
// Changed to compile with aCC on HPUX
//
// Revision 1.1  1998/01/27  15:41:24  ewc
// Initial revision
//
 */

#include <omniORB4/CORBA.h>
#include <omniORB4/objTracker.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <typecode.h>
#include <tcParser.h>
#include <orbParameters.h>
#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(dynamicLib);

OMNI_USING_NAMESPACE(omni)

// CORBA::TypeCode - core class function implementation
//
// NOTE : internally, the TypeCode implementation implicitly assumes that no
// content typecode is _ever_ CORBA::TypeCode::_nil().
// To support this assumption cleanly, the visible CORBA::TypeCode()
// interface must check any typecodes passed to it and raise
// CORBA::BAD_TYPECODE if a _nil() value is encountered.


//////////////////////////////////////////////////////////////////////
/////////////////////////// CORBA::TypeCode //////////////////////////
//////////////////////////////////////////////////////////////////////

#if !defined(OMNIORB_NO_EXCEPTION_LOGGING) && defined(__DECCXX) && __DECCXX_VER > 60000000
//  Compaq C++ 6.x needs dummy return if function is called to throw an
//  exception.
#ifndef NEED_DUMMY_RETURN
#define NEED_DUMMY_RETURN
#endif
#endif


CORBA::TypeCode::~TypeCode() {
  pd_magic = 0;
}

CORBA::TCKind
CORBA::TypeCode::kind() const
{
  return TypeCode_indirect::strip(ToConstTcBase_Checked(this))->NP_kind();
}

CORBA::Boolean
CORBA::TypeCode::equal(CORBA::TypeCode_ptr TCp) const
{
  if (!PR_is_valid(TCp)) OMNIORB_THROW(BAD_PARAM,
				       BAD_PARAM_InvalidTypeCode,
				       CORBA::COMPLETED_NO);
  return ToConstTcBase_Checked(this)
    ->NP_equal(ToTcBase_Checked(TCp),0,0);
}

CORBA::Boolean
CORBA::TypeCode::equivalent(CORBA::TypeCode_ptr TCp) const
{
  if (!PR_is_valid(TCp)) OMNIORB_THROW(BAD_PARAM,
				       BAD_PARAM_InvalidTypeCode,
				       CORBA::COMPLETED_NO);
  return ToConstTcBase_Checked(this)
    ->NP_equal(ToTcBase_Checked(TCp),1,0);
}

CORBA::TypeCode_ptr
CORBA::TypeCode::get_compact_typecode() const
{
  TypeCode_base* tp = (TypeCode_base*)ToConstTcBase_Checked(this);
  return tp->NP_compactTc();
}

const char*
CORBA::TypeCode::id() const
{
  return ToConstTcBase_Checked(this)->NP_id();
}

const char*
CORBA::TypeCode::name() const
{
  return ToConstTcBase_Checked(this)->NP_name();
}

CORBA::ULong
CORBA::TypeCode::member_count() const
{
  return ToConstTcBase_Checked(this)->NP_member_count();
}

const char*
CORBA::TypeCode::member_name(CORBA::ULong index) const
{
  return ToConstTcBase_Checked(this)->NP_member_name(index);
}

CORBA::TypeCode_ptr
CORBA::TypeCode::member_type(CORBA::ULong i) const
{
  return TypeCode_collector::duplicateRef(ToConstTcBase_Checked(this)
					  ->NP_member_type(i));
}

CORBA::Any*
CORBA::TypeCode::member_label(CORBA::ULong i) const
{
  return ToConstTcBase_Checked(this)->NP_member_label(i);
}

CORBA::TypeCode_ptr
CORBA::TypeCode::discriminator_type() const
{
  return TypeCode_collector::duplicateRef(ToConstTcBase_Checked(this)
					  ->NP_discriminator_type());
}

CORBA::Long
CORBA::TypeCode::default_index() const
{
  return ToConstTcBase_Checked(this)->NP_default_index();
}

CORBA::ULong
CORBA::TypeCode::length() const
{
  return ToConstTcBase_Checked(this)->NP_length();
}

CORBA::TypeCode_ptr
CORBA::TypeCode::content_type() const
{
  return TypeCode_collector::duplicateRef(ToConstTcBase_Checked(this)
					  ->NP_content_type());
}

CORBA::UShort
CORBA::TypeCode::fixed_digits() const
{
  return ToConstTcBase_Checked(this)->NP_fixed_digits();
}

CORBA::Short
CORBA::TypeCode::fixed_scale() const
{
  return ToConstTcBase_Checked(this)->NP_fixed_scale();
}

CORBA::Long
CORBA::TypeCode::param_count() const
{
  return ToConstTcBase_Checked(this)->NP_param_count();
}

CORBA::Any*
CORBA::TypeCode::parameter(Long index) const
{
  return ToConstTcBase_Checked(this)->NP_parameter(index);
}

CORBA::Short
CORBA::TypeCode::member_visibility(CORBA::ULong index) const
{
  return ToConstTcBase_Checked(this)->NP_member_visibility(index);
}

CORBA::ValueModifier
CORBA::TypeCode::type_modifier() const
{
  return ToConstTcBase_Checked(this)->NP_type_modifier();
}

CORBA::TypeCode_ptr
CORBA::TypeCode::concrete_base_type() const
{
  return ToConstTcBase_Checked(this)->NP_concrete_base_type();
}


// Static TypeCode member functions

CORBA::TypeCode_ptr
CORBA::TypeCode::_duplicate(CORBA::TypeCode_ptr t)
{
  if (!PR_is_valid(t))
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidTypeCode,CORBA::COMPLETED_NO);
  if (CORBA::is_nil(t))  return t;
  return TypeCode_collector::duplicateRef(ToTcBase(t));
}

OMNI_NAMESPACE_BEGIN(omni)

//
// The nil TypeCode is derived from neither CORBA::Object or
// omniTrackedObject. We use a holder object to ensure it is deleted
// on exit.
//
class omniNilTypeCodeHolder : public omniTrackedObject {
public:
  inline omniNilTypeCodeHolder(CORBA::TypeCode_ptr nil) : pd_nil(nil) {}
  virtual ~omniNilTypeCodeHolder() { delete pd_nil; }
private:
  CORBA::TypeCode_ptr pd_nil;
};

OMNI_NAMESPACE_END(omni)


CORBA::TypeCode_ptr
CORBA::TypeCode::_nil()
{
  static TypeCode* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new TypeCode;
      registerTrackedObject(new omniNilTypeCodeHolder(_the_nil_ptr));
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

// omniORB marshalling routines
void
CORBA::TypeCode::marshalTypeCode(TypeCode_ptr obj,cdrStream &s)
{
  TypeCode_offsetTable otbl;

  TypeCode_marshaller::marshal(ToTcBase_Checked(obj), s, &otbl);
}


CORBA::TypeCode_ptr
CORBA::TypeCode::unmarshalTypeCode(cdrStream &s)
{
  TypeCode_offsetTable otbl;

  TypeCode_base* tc = TypeCode_marshaller::unmarshal(s, &otbl);
  TypeCode_collector::markLoopMembers(tc);

  return tc;
}

CORBA::Boolean
CORBA::TypeCode::NP_is_nil() const
{
  return 1;
}


// Complex TypeCode constructors
// These routines are the omniORB-specific constructors for complex typecodes.
// CORBA-compliant applications should only ever access these through the ORB
// interface.  These functions are for the use of omniORB stubs & libraries.
//  NB: The reference count of the returned TypeCode_ptr is always 1, unless
// the TypeCode structure contains recursive references.

#if 0
CORBA::TypeCode_ptr
CORBA::TypeCode::NP_struct_tc(const char* id, const char* name,
			      const CORBA::StructMemberSeq& members)
{
  const CORBA::ULong memberCount = members.length();
  for( CORBA::ULong i = 0; i < memberCount; i++ )
    if ( !PR_is_valid(members[i].type) || CORBA::is_nil(members[i].type))
      OMNIORB_THROW(BAD_TYPECODE,
		    BAD_TYPECODE_TypeCodeIsNil,
		    CORBA::COMPLETED_NO);

  return new TypeCode_struct(id, name, members);
}


CORBA::TypeCode_ptr
CORBA::TypeCode::NP_union_tc(const char* id, const char* name,
			    CORBA::TypeCode_ptr discriminator_type,
			    const CORBA::UnionMemberSeq& members)
{
  const CORBA::ULong memberCount = members.length();
  for( CORBA::ULong i = 0; i < memberCount; i++ )
    if( !PR_is_valid(members[i].type) || CORBA::is_nil(members[i].type) )
      OMNIORB_THROW(BAD_TYPECODE,
		    BAD_TYPECODE_TypeCodeIsNil,
		    CORBA::COMPLETED_NO);

  return new TypeCode_union(id, name, ToTcBase_Checked(discriminator_type),
			    members);
}
#endif


CORBA::TypeCode_ptr
CORBA::TypeCode::NP_enum_tc(const char* id, const char* name,
		      const CORBA::EnumMemberSeq& members)
{
  return new TypeCode_enum(id, name, members);
}

CORBA::TypeCode_ptr
CORBA::TypeCode::NP_alias_tc(const char* id, const char* name,
		       CORBA::TypeCode_ptr original_type)
{
  return new TypeCode_alias(id, name, ToTcBase_Checked(original_type));
}

#if 0
CORBA::TypeCode_ptr
CORBA::TypeCode::NP_exception_tc(const char* id, const char* name,
			   const CORBA::StructMemberSeq& members)
{
  const CORBA::ULong memberCount = members.length();
  for( CORBA::ULong i = 0; i < memberCount; i++ )
    if ( !PR_is_valid(members[i].type) || CORBA::is_nil(members[i].type))
      OMNIORB_THROW(BAD_TYPECODE,
		    BAD_TYPECODE_TypeCodeIsNil,
		    CORBA::COMPLETED_NO);

  return new TypeCode_except(id, name, members);
}
#endif

CORBA::TypeCode_ptr
CORBA::TypeCode::NP_interface_tc(const char* id, const char* name)
{
  return new TypeCode_objref(id, name);
}

CORBA::TypeCode_ptr
CORBA::TypeCode::NP_string_tc(CORBA::ULong bound)
{
  return new TypeCode_string(bound);
}

CORBA::TypeCode_ptr
CORBA::TypeCode::NP_wstring_tc(CORBA::ULong bound)
{
  return new TypeCode_wstring(bound);
}

CORBA::TypeCode_ptr
CORBA::TypeCode::NP_fixed_tc(CORBA::UShort digits, CORBA::Short scale)
{
  return new TypeCode_fixed(digits, scale);
}

CORBA::TypeCode_ptr
CORBA::TypeCode::NP_sequence_tc(CORBA::ULong bound,
				CORBA::TypeCode_ptr element_type)
{
  return new TypeCode_sequence(bound, ToTcBase_Checked(element_type));
}

CORBA::TypeCode_ptr
CORBA::TypeCode::NP_array_tc(CORBA::ULong length,
		       CORBA::TypeCode_ptr element_type)
{
  return new TypeCode_array(length, ToTcBase_Checked(element_type));
}

CORBA::TypeCode_ptr
CORBA::TypeCode::NP_recursive_sequence_tc(CORBA::ULong bound,
					  CORBA::ULong offset)
{
  return new TypeCode_sequence(bound, offset);
}

CORBA::TypeCode_ptr
CORBA::TypeCode::NP_recursive_tc(const char* id)
{
  return new TypeCode_indirect(id);
}

//////////////////////////////////////////////////////////////////////
/////////////////////// Stub TypeCode Accessors //////////////////////
//////////////////////////////////////////////////////////////////////

// These are needed by the stubs to generate static typecodes
// for user-defined types. Before constructing or using any
// of the typecode functionnality it is important to ensure
// that any statically initialised data is properly constructed.

OMNI_NAMESPACE_BEGIN(omni)

static void check_static_data_is_initialised();


// The omniTypeCodeList singleton is used to track internal
// TypeCodes created in this file.
class omniTypeCodeList : public omniTrackedObject {
public:
  omniTypeCodeList() {
    pd_tracker = new CORBA::TypeCode::_Tracker(__FILE__);
  }
  void add(CORBA::TypeCode_ptr tc) {
    pd_tracker->add(tc);
  }

  virtual ~omniTypeCodeList();

  CORBA::TypeCode::_Tracker* pd_tracker;
};

static omniTypeCodeList* the_tc_list = 0;

OMNI_NAMESPACE_END(omni)


CORBA::TypeCode_ptr
CORBA::TypeCode::PR_struct_tc(const char* id, const char* name,
			      const PR_structMember* members,
			      ULong memberCount,
			      CORBA::TypeCode::_Tracker* tracker)
{
  check_static_data_is_initialised();

  TypeCode_struct::Member* new_members
    = new TypeCode_struct::Member[memberCount];

  for( ULong i = 0; i < memberCount; i++ ) {
    // We duplicate the name and type.
    new_members[i].name = CORBA::string_dup(members[i].name);
    new_members[i].type = CORBA::TypeCode::_duplicate(members[i].type);
  }

  CORBA::TypeCode_ptr r = new TypeCode_struct(CORBA::string_dup(id),
					      CORBA::string_dup(name),
					      new_members, memberCount);
  tracker->add(r);
  return r;
}


CORBA::TypeCode_ptr
CORBA::TypeCode::PR_union_tc(const char* id, const char* name,
			     TypeCode_ptr discriminator_type,
			     const PR_unionMember* members,
			     ULong memberCount, Long deflt,
			     CORBA::TypeCode::_Tracker* tracker)
{
  check_static_data_is_initialised();

  CORBA::TypeCode_ptr r = new TypeCode_union(id, name,
					     ToTcBase(discriminator_type),
					     members, memberCount, deflt);
  tracker->add(r);
  return r;
}


CORBA::TypeCode_ptr
CORBA::TypeCode::PR_enum_tc(const char* id, const char* name,
			    const char** members, ULong memberCount,
			    CORBA::TypeCode::_Tracker* tracker)
{
  check_static_data_is_initialised();

  CORBA::EnumMemberSeq memberSeq;

  memberSeq.length(memberCount);
  for( ULong i = 0; i < memberCount; i++ )
    memberSeq[i] = members[i];

  CORBA::TypeCode_ptr r = NP_enum_tc(id, name, memberSeq);
  tracker->add(r);
  return r;
}


CORBA::TypeCode_ptr
CORBA::TypeCode::PR_alias_tc(const char* id, const char* name,
			     CORBA::TypeCode_ptr original_type,
			     CORBA::TypeCode::_Tracker* tracker)
{
  check_static_data_is_initialised();
  CORBA::TypeCode_ptr r = new TypeCode_alias(id, name,
					     ToTcBase(original_type));
  tracker->add(r);
  return r;
}


CORBA::TypeCode_ptr
CORBA::TypeCode::PR_exception_tc(const char* id, const char* name,
				 const PR_structMember* members,
				 ULong memberCount,
				 CORBA::TypeCode::_Tracker* tracker)
{
  check_static_data_is_initialised();

  TypeCode_struct::Member* new_members =
    new TypeCode_struct::Member[memberCount];

  for( ULong i = 0; i < memberCount; i++ ) {
    // We duplicate the name and type.
    new_members[i].name = CORBA::string_dup(members[i].name);
    new_members[i].type = CORBA::TypeCode::_duplicate(members[i].type);
  }

  CORBA::TypeCode_ptr r = new TypeCode_except(CORBA::string_dup(id),
					      CORBA::string_dup(name),
					      new_members, memberCount);
  tracker->add(r);
  return r;
}


CORBA::TypeCode_ptr
CORBA::TypeCode::PR_interface_tc(const char* id, const char* name,
				 CORBA::TypeCode::_Tracker* tracker)
{
  check_static_data_is_initialised();
  CORBA::TypeCode_ptr r = new TypeCode_objref(id, name);
  tracker->add(r);
  return r;
}


CORBA::TypeCode_ptr
CORBA::TypeCode::PR_string_tc(CORBA::ULong bound,
			      CORBA::TypeCode::_Tracker* tracker)
{
  if( bound == 0 )  return PR_string_tc();
  check_static_data_is_initialised();
  CORBA::TypeCode_ptr r = new TypeCode_string(bound);
  tracker->add(r);
  return r;
}

CORBA::TypeCode_ptr
CORBA::TypeCode::PR_wstring_tc(CORBA::ULong bound,
			       CORBA::TypeCode::_Tracker* tracker)
{
  if( bound == 0 )  return PR_wstring_tc();
  check_static_data_is_initialised();
  CORBA::TypeCode_ptr r = new TypeCode_wstring(bound);
  tracker->add(r);
  return r;
}

CORBA::TypeCode_ptr
CORBA::TypeCode::PR_fixed_tc(CORBA::UShort digits, CORBA::UShort scale,
			     CORBA::TypeCode::_Tracker* tracker)
{
  check_static_data_is_initialised();
  CORBA::TypeCode_ptr r = new TypeCode_fixed(digits, scale);
  tracker->add(r);
  return r;
}

CORBA::TypeCode_ptr
CORBA::TypeCode::PR_sequence_tc(CORBA::ULong bound,
				CORBA::TypeCode_ptr element_type,
				CORBA::TypeCode::_Tracker* tracker)
{
  check_static_data_is_initialised();
  CORBA::TypeCode_ptr r = new TypeCode_sequence(bound, ToTcBase(element_type));
  tracker->add(r);
  return r;
}


CORBA::TypeCode_ptr
CORBA::TypeCode::PR_array_tc(CORBA::ULong length,
			     CORBA::TypeCode_ptr element_type,
			     CORBA::TypeCode::_Tracker* tracker)
{
  check_static_data_is_initialised();
  CORBA::TypeCode_ptr r = new TypeCode_array(length, ToTcBase(element_type));
  tracker->add(r);
  return r;
}


CORBA::TypeCode_ptr
CORBA::TypeCode::PR_recursive_sequence_tc(CORBA::ULong bound,
					  CORBA::ULong offset,
					  CORBA::TypeCode::_Tracker* tracker)
{
  check_static_data_is_initialised();
  CORBA::TypeCode_ptr r = new TypeCode_sequence(bound, offset);
  tracker->add(r);
  return r;
}



CORBA::TypeCode_ptr CORBA::TypeCode::PR_null_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_null;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_void_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_void;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_short_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_short;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_long_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_long;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_ushort_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_ushort;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_ulong_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_ulong;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_float_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_float;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_double_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_double;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_boolean_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_boolean;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_char_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_char;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_wchar_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_wchar;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_octet_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_octet;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_any_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_any;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_TypeCode_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_TypeCode;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_Principal_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_Principal;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_Object_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_Object;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_string_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_string;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_wstring_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_wstring;
}
#ifdef HAS_LongLong
CORBA::TypeCode_ptr CORBA::TypeCode::PR_longlong_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_longlong;
}
CORBA::TypeCode_ptr CORBA::TypeCode::PR_ulonglong_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_ulonglong;
}
#endif
#ifdef HAS_LongDouble
CORBA::TypeCode_ptr CORBA::TypeCode::PR_longdouble_tc() {
  check_static_data_is_initialised();
  return CORBA::_tc_longdouble;
}
#endif


// OMG TypeCode release function

void
CORBA::release(TypeCode_ptr o)
{
  OMNIORB_ASSERT(CORBA::TypeCode::PR_is_valid(o));

  if( CORBA::TypeCode::PR_is_valid(o) && !CORBA::is_nil(o) )
    TypeCode_collector::releaseRef(ToTcBase(o));
}


OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////// TypeCode_base ///////////////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_base::TypeCode_base(CORBA::TCKind tck)
  : pd_complete(1), pd_mark(0), pd_ref_count(1),
    pd_loop_member(0), pd_internal_ref_count(0),
    pd_cached_paramlist(0),
    pd_aliasExpandedTc(0), pd_compactTc(0), pd_tck(tck),
    pd_next(0)
{
  switch( tck ) {

  case CORBA::tk_null:
  case CORBA::tk_void:
    pd_alignmentTable.setNumEntries(1);
    pd_alignmentTable.addSimple(omni::ALIGN_1, 0);
    pd_aliasExpandedTc = pd_compactTc = this;
    break;

  case CORBA::tk_boolean:
  case CORBA::tk_octet:
    pd_alignmentTable.setNumEntries(1);
    pd_alignmentTable.addSimple(omni::ALIGN_1, 1);
    pd_aliasExpandedTc = pd_compactTc = this;
    break;

  case CORBA::tk_short:
  case CORBA::tk_ushort:
    pd_alignmentTable.setNumEntries(1);
    pd_alignmentTable.addSimple(omni::ALIGN_2, 2);
    pd_aliasExpandedTc = pd_compactTc = this;
    break;

  case CORBA::tk_long:
  case CORBA::tk_ulong:
  case CORBA::tk_float:
    pd_alignmentTable.setNumEntries(1);
    pd_alignmentTable.addSimple(omni::ALIGN_4, 4);
    pd_aliasExpandedTc = pd_compactTc = this;
    break;

  case CORBA::tk_double:
#ifdef HAS_LongLong
  case CORBA::tk_longlong:
  case CORBA::tk_ulonglong:
#endif
    pd_alignmentTable.setNumEntries(1);
    pd_alignmentTable.addSimple(omni::ALIGN_8, 8);
    pd_aliasExpandedTc = pd_compactTc = this;
    break;

#ifdef HAS_LongDouble
  case CORBA::tk_longdouble:
    pd_alignmentTable.setNumEntries(1);
    pd_alignmentTable.addSimple(omni::ALIGN_8, 16);
    pd_aliasExpandedTc = pd_compactTc = this;
    break;
#endif

  case CORBA::tk_char:
  case CORBA::tk_wchar:
    pd_alignmentTable.setNumEntries(1);
    pd_alignmentTable.addNasty(this);
    pd_aliasExpandedTc = pd_compactTc = this;
    break;

  case CORBA::tk_any:
  case CORBA::tk_TypeCode:
  case CORBA::tk_Principal:
    pd_alignmentTable.setNumEntries(1);
    pd_alignmentTable.addNasty(this);
    pd_aliasExpandedTc = pd_compactTc = this;
    break;

  case CORBA::tk_string:
    pd_compactTc = this;
    break;

  case CORBA::tk_wstring:
    pd_compactTc = this;
    break;

  case CORBA::tk_fixed:
    pd_aliasExpandedTc = pd_compactTc = this;
    break;

  default:
    pd_complete = 0;
    break;
  }
}


TypeCode_base::~TypeCode_base()
{
  if( pd_cached_paramlist )  delete pd_cached_paramlist;
  if( pd_aliasExpandedTc && pd_aliasExpandedTc != this )
    TypeCode_collector::releaseRef(pd_aliasExpandedTc);
  if( pd_compactTc && pd_compactTc != this )
    TypeCode_collector::releaseRef(pd_compactTc);
}


void
TypeCode_base::NP_marshalSimpleParams(cdrStream &,
				      TypeCode_offsetTable* ) const
{
  OMNIORB_THROW(BAD_TYPECODE,
		BAD_TYPECODE_InvalidOperation,
		CORBA::COMPLETED_NO);
}

void
TypeCode_base::NP_marshalComplexParams(cdrStream &,
				       TypeCode_offsetTable* ) const
{
  OMNIORB_THROW(BAD_TYPECODE,
		BAD_TYPECODE_InvalidOperation,
		CORBA::COMPLETED_NO);
}

const TypeCode_base*
TypeCode_base::NP_expand(const TypeCode_base* tc)
{
  while (1) {
    if (tc->NP_kind() == CORBA::tk_alias)
      tc = tc->NP_content_type();
    else if (tc->NP_kind() == CORBA::_np_tk_indirect)
      tc = ((TypeCode_indirect*)tc)->NP_resolved();
    else
      break;
  }
  return tc;
}

CORBA::Boolean
TypeCode_base::NP_equal(const TypeCode_base* TCp,
			CORBA::Boolean is_equivalent,
			const TypeCode_pairlist* tcpl) const
{
  if (NP_kind() == CORBA::_np_tk_indirect)
    return TypeCode_indirect::strip(this)->NP_equal(TCp, is_equivalent, tcpl);

  TCp = TypeCode_indirect::strip(TCp);

  // Check for trivial pointer-based equality
  if (this == TCp) return 1;

  // Check the pairlist for a match, for recursive typecodes
  if( TypeCode_pairlist::contains(tcpl, this, TCp) )
    return 1;

  // No match, so create a pair for these two codes, to avoid loops
  TypeCode_pairlist tcpl_tmp(tcpl, this, TCp);

  // Should we expand the aliases?
  if (is_equivalent)
    {
      const TypeCode_base* tc1_tmp = NP_expand(this);
      const TypeCode_base* tc2_tmp = NP_expand(TCp);

      return tc1_tmp->NP_extendedEqual(tc2_tmp, 1, &tcpl_tmp);
    }
  else
    return NP_extendedEqual(TCp, 0, &tcpl_tmp);
}


CORBA::Boolean
TypeCode_base::NP_extendedEqual(const TypeCode_base* TCp,
				CORBA::Boolean,
				const TypeCode_pairlist*) const
{
  // Base types are equivalent if their Kinds match
  return NP_kind() == TCp->NP_kind();
}


const char*
TypeCode_base::NP_id() const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

const char*
TypeCode_base::NP_name() const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

CORBA::ULong
TypeCode_base::NP_member_count() const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

const char*
TypeCode_base::NP_member_name(CORBA::ULong index) const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

TypeCode_base*
TypeCode_base::NP_member_type(CORBA::ULong index) const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

CORBA::Any*
TypeCode_base::NP_member_label(CORBA::ULong index) const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

TypeCode_base*
TypeCode_base::NP_discriminator_type() const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

CORBA::Long
TypeCode_base::NP_default_index() const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

CORBA::ULong
TypeCode_base::NP_length() const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


TypeCode_base*
TypeCode_base::NP_content_type() const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

CORBA::UShort
TypeCode_base::NP_fixed_digits() const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

CORBA::Short
TypeCode_base::NP_fixed_scale() const
{
  throw CORBA::TypeCode::BadKind();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

CORBA::Long
TypeCode_base::NP_param_count() const
{
  return 0;
}


CORBA::Any*
TypeCode_base::NP_parameter(CORBA::Long) const
{
  throw CORBA::TypeCode::Bounds();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

CORBA::Short
TypeCode_base::NP_member_visibility(CORBA::ULong) const
{
  throw CORBA::TypeCode::Bounds();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

CORBA::ValueModifier
TypeCode_base::NP_type_modifier() const
{
  throw CORBA::TypeCode::Bounds();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

TypeCode_base*
TypeCode_base::NP_concrete_base_type() const
{
  throw CORBA::TypeCode::Bounds();
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


CORBA::Boolean
TypeCode_base::NP_is_nil() const
{
  return 0;
}


CORBA::Boolean
TypeCode_base::NP_containsAnAlias()
{
  return 0;
}


TypeCode_base*
TypeCode_base::NP_aliasExpand(TypeCode_pairlist*)
{
  throw omniORB::fatalException(__FILE__,__LINE__,
     "TypeCode_base::NP_aliasExpand() - should not have been called");
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


static omni_tracedmutex* aliasExpandedTc_lock = 0;


TypeCode_base*
TypeCode_base::aliasExpand(TypeCode_base* tc)
{
  if( !tc->pd_aliasExpandedTc ) {
    TypeCode_base* aetc =
      tc->NP_containsAnAlias() ? tc->NP_aliasExpand(0) : tc;

    aliasExpandedTc_lock->lock();
    if( !tc->pd_aliasExpandedTc )  tc->pd_aliasExpandedTc = aetc;
    else                           TypeCode_collector::releaseRef(aetc);
    aliasExpandedTc_lock->unlock();
  }

  return TypeCode_collector::duplicateRef(tc->pd_aliasExpandedTc);
}

TypeCode_base*
TypeCode_base::NP_compactTc()
{
  if ( !pd_compactTc ) {
    // Bounce this typecode off a cdrMemorystream.
    // This ensures that any recursive members are duplicated correctly.
    cdrMemoryStream s;
    CORBA::TypeCode::marshalTypeCode(this,s);
    pd_compactTc = ToTcBase(CORBA::TypeCode::unmarshalTypeCode(s));
    // Now remove all names and member_names from the typecode
    pd_compactTc->removeOptionalNames();
  }
  return TypeCode_collector::duplicateRef(pd_compactTc);
}

void
TypeCode_base::removeOptionalNames()
{
  return;
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_string //////////////////////////
//////////////////////////////////////////////////////////////////////

// Notes:
// String typecodes have SIMPLE parameter lists (i.e. ones NOT enclosed in an
// octet sequence encapsualtion)

TypeCode_string::TypeCode_string(CORBA::ULong maxLen)
  : TypeCode_base(CORBA::tk_string)
{
  pd_length = maxLen;

  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);
}


TypeCode_string::TypeCode_string()
  : TypeCode_base(CORBA::tk_string)
{
  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);
}


TypeCode_string::~TypeCode_string() {}


void
TypeCode_string::NP_marshalSimpleParams(cdrStream &s,
					TypeCode_offsetTable* ) const
{
  pd_length >>= s;
}

TypeCode_base*
TypeCode_string::NP_unmarshalSimpleParams(cdrStream &s,
					  TypeCode_offsetTable* otbl)
{
  TypeCode_string* _ptr = new TypeCode_string;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  _ptr->pd_length <<= s;

  return _ptr;
}


CORBA::Boolean
TypeCode_string::NP_extendedEqual(const TypeCode_base*  TCp,
				  CORBA::Boolean,
				  const TypeCode_pairlist*) const
{
  return ((NP_kind() == TCp->NP_kind()) && (NP_length() == TCp->NP_length()));
}


CORBA::ULong
TypeCode_string::NP_length() const
{
  return pd_length;
}


CORBA::Long
TypeCode_string::NP_param_count() const
{
  return 1;
}


CORBA::Any*
TypeCode_string::NP_parameter(CORBA::Long index) const
{
  CORBA::Any* rv = new CORBA::Any;

  try {
    switch (index) {
    case 0:
      *rv <<= pd_length;
      break;
    default:
      throw CORBA::TypeCode::Bounds();
    };
  } catch (...) {
    delete rv;
    throw;
  }

  return rv;
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_wstring /////////////////////////
//////////////////////////////////////////////////////////////////////

// Notes:
// WString typecodes have SIMPLE parameter lists (i.e. ones NOT enclosed in an
// octet sequence encapsualtion)

TypeCode_wstring::TypeCode_wstring(CORBA::ULong maxLen)
  : TypeCode_base(CORBA::tk_wstring)
{
  pd_length = maxLen;

  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);
}


TypeCode_wstring::TypeCode_wstring()
  : TypeCode_base(CORBA::tk_wstring)
{
  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);
}


TypeCode_wstring::~TypeCode_wstring() {}


void
TypeCode_wstring::NP_marshalSimpleParams(cdrStream &s,
					 TypeCode_offsetTable* ) const
{
  pd_length >>= s;
}

TypeCode_base*
TypeCode_wstring::NP_unmarshalSimpleParams(cdrStream &s,
					   TypeCode_offsetTable* otbl)
{
  TypeCode_wstring* _ptr = new TypeCode_wstring;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  _ptr->pd_length <<= s;

  return _ptr;
}


CORBA::Boolean
TypeCode_wstring::NP_extendedEqual(const TypeCode_base*  TCp,
				   CORBA::Boolean,
				   const TypeCode_pairlist*) const
{
  return ((NP_kind() == TCp->NP_kind()) && (NP_length() == TCp->NP_length()));
}


CORBA::ULong
TypeCode_wstring::NP_length() const
{
  return pd_length;
}


CORBA::Long
TypeCode_wstring::NP_param_count() const
{
  return 1;
}


CORBA::Any*
TypeCode_wstring::NP_parameter(CORBA::Long index) const
{
  CORBA::Any* rv = new CORBA::Any;

  try {
    switch (index) {
    case 0:
      *rv <<= pd_length;
      break;
    default:
      throw CORBA::TypeCode::Bounds();
    };
  } catch (...) {
    delete rv;
    throw;
  }

  return rv;
}


//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_fixed ///////////////////////////
//////////////////////////////////////////////////////////////////////

// Notes:
// Fixed typecodes have SIMPLE parameter lists (i.e. ones NOT enclosed in an
// octet sequence encapsualtion)

TypeCode_fixed::TypeCode_fixed(CORBA::UShort digits, CORBA::Short scale)
  : TypeCode_base(CORBA::tk_fixed)
{
  if (digits < 1 || digits > OMNI_FIXED_DIGITS || scale > digits)
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_InvalidFixedPointLimits,
		  CORBA::COMPLETED_NO);

  pd_digits = digits;
  pd_scale  = scale;

  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);
}

TypeCode_fixed::TypeCode_fixed()
  : TypeCode_base(CORBA::tk_fixed)
{
  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);
}

TypeCode_fixed::~TypeCode_fixed() {}

void
TypeCode_fixed::NP_marshalSimpleParams(cdrStream &s,
				       TypeCode_offsetTable* ) const
{
  pd_digits >>= s;
  pd_scale  >>= s;
}

TypeCode_base*
TypeCode_fixed::NP_unmarshalSimpleParams(cdrStream &s,
					 TypeCode_offsetTable* otbl)
{
  TypeCode_fixed* _ptr = new TypeCode_fixed;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  _ptr->pd_digits <<= s;
  _ptr->pd_scale  <<= s;

  return _ptr;
}


CORBA::Boolean
TypeCode_fixed::NP_extendedEqual(const TypeCode_base*  TCp,
				 CORBA::Boolean,
				 const TypeCode_pairlist*) const
{
  return ((NP_kind() == TCp->NP_kind()) &&
	  (NP_fixed_digits() == TCp->NP_fixed_digits()) &&
	  (NP_fixed_scale()  == TCp->NP_fixed_scale()));
}


CORBA::UShort
TypeCode_fixed::NP_fixed_digits() const
{
  return pd_digits;
}

CORBA::Short
TypeCode_fixed::NP_fixed_scale() const
{
  return pd_scale;
}



//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_objref //////////////////////////
//////////////////////////////////////////////////////////////////////

// Notes:
// The object reference typecode has a COMPLEX parameter list, which must
// be encoded as an encapsulated octet sequence.  This makes things
// horrendously difficult, because of the way Indirection typecodes work.

TypeCode_objref::TypeCode_objref(const char* repositoryId, const char* name,
				 CORBA::TCKind tck)
  : TypeCode_base(tck)
{
  pd_repoId = repositoryId;
  pd_name = name;
  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);
  pd_complete = 1;
}


TypeCode_objref::TypeCode_objref(CORBA::TCKind tck)
  : TypeCode_base(tck)
{
  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);
}


TypeCode_objref::~TypeCode_objref() {}


void
TypeCode_objref::NP_marshalComplexParams(cdrStream &s,
					 TypeCode_offsetTable* ) const
{
  s.marshalRawString(pd_repoId);
  s.marshalRawString(pd_name);
}

TypeCode_base*
TypeCode_objref::NP_unmarshalComplexParams(cdrStream &s,
					   TypeCode_offsetTable* otbl)
{
  TypeCode_objref*  _ptr = new TypeCode_objref;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  _ptr->pd_repoId = s.unmarshalRawString();
  _ptr->pd_name   = s.unmarshalRawString();
  _ptr->pd_complete = 1;

  return _ptr;
}


TypeCode_paramListType
TypeCode_objref::NP_paramListType() const
{
  return plt_Complex;
}


CORBA::Boolean
TypeCode_objref::NP_extendedEqual(const TypeCode_base*  TCp,
				  CORBA::Boolean is_equivalent,
				  const TypeCode_pairlist* ) const
{
  if (NP_kind() != TCp->NP_kind())
    return 0;

  if (!omni::ptrStrMatch(NP_id(), TCp->NP_id())) {
    if (is_equivalent) {
      // Empty string is equivalent to CORBA::Object repoId

      if (NP_id()[0] == '\0' &&
	  omni::ptrStrMatch(TCp->NP_id(), CORBA::Object::_PD_repoId))
	return 1;

      if (TCp->NP_id()[0] == '\0' &&
	  omni::ptrStrMatch(NP_id(), CORBA::Object::_PD_repoId))
	return 1;
      
      return 0;
    }
  }
  if (!is_equivalent)
    return NP_namesEqual(NP_name(), TCp->NP_name());
  else
    return 1;
}


const char*
TypeCode_objref::NP_id() const
{
  return pd_repoId;
}


const char*
TypeCode_objref::NP_name() const
{
  return pd_name;
}


CORBA::Long
TypeCode_objref::NP_param_count() const
{
  return 1;
}


CORBA::Any*
TypeCode_objref::NP_parameter(CORBA::Long index) const
{
  CORBA::Any* rv = new CORBA::Any;

  try {
    switch (index) {
    case 0:
      *rv <<= (const char* )pd_name;
      break;
    default:
      throw CORBA::TypeCode::Bounds();
    };
  } catch (...) {
    delete rv;
    throw;
  }

  return rv;
}

void
TypeCode_objref::removeOptionalNames()
{
  if (!pd_compactTc) {
    pd_name = (const char*)"";
    pd_compactTc = this;
  }
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_alias ///////////////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_alias::TypeCode_alias(const char* repositoryId,
			       const char* name,
			       TypeCode_base* real_type)
  : TypeCode_base(CORBA::tk_alias)
{
  // Initialise
  pd_repoId = repositoryId;
  pd_name = name;
  pd_content = TypeCode_collector::duplicateRef(real_type);

  pd_alignmentTable.set(real_type->alignmentTable());

  if( real_type->aliasExpandedTc() )
    pd_aliasExpandedTc =
      TypeCode_collector::duplicateRef(real_type->aliasExpandedTc());

  pd_complete = NP_complete_recursive(this, repositoryId);
}


TypeCode_alias::~TypeCode_alias() {}


void
TypeCode_alias::NP_marshalComplexParams(cdrStream &s,
					TypeCode_offsetTable* otbl) const
{
  s.marshalRawString(pd_repoId);
  s.marshalRawString(pd_name);
  TypeCode_marshaller::marshal(ToTcBase(pd_content), s, otbl);
}

TypeCode_base*
TypeCode_alias::NP_unmarshalComplexParams(cdrStream &s,
					  TypeCode_offsetTable* otbl)
{
  TypeCode_alias* _ptr = new TypeCode_alias;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  _ptr->pd_repoId = s.unmarshalRawString();
  _ptr->pd_name   = s.unmarshalRawString();
  _ptr->pd_content = TypeCode_marshaller::unmarshal(s, otbl);
  _ptr->pd_complete = 1;
  _ptr->pd_alignmentTable.set(ToTcBase(_ptr->pd_content)->alignmentTable());

  return _ptr;
}


CORBA::Boolean
TypeCode_alias::NP_complete_recursive_sequences(TypeCode_base*  tc,
						CORBA::ULong offset)
{
  // *** omniORB-specific - I assume here that the 'offset' parameter
  // ignores intermediate tk_alias typecodes between the recursive sequence
  // and it's content type, so I don't increase the offset here:
  if (!pd_complete)
    pd_complete =
      ToTcBase(pd_content)->NP_complete_recursive_sequences(tc, offset);
  return pd_complete;
}

CORBA::Boolean
TypeCode_alias::NP_complete_recursive(TypeCode_base* tc, const char* repoId)
{
  if (!pd_complete)
    pd_complete = ToTcBase(pd_content)->NP_complete_recursive(tc, repoId);
  return pd_complete;
}


CORBA::Boolean
TypeCode_alias::NP_extendedEqual(const TypeCode_base*  TCp,
				 CORBA::Boolean is_equivalent,
				 const TypeCode_pairlist* tcpl) const
{
  if (NP_kind() != TCp->NP_kind())
    return 0;

  if (is_equivalent) {
    if (NP_id() && TCp->NP_id())
      return NP_namesEqual(NP_id(),TCp->NP_id());
  }
  else {
    if (!NP_namesEqual(NP_id(),TCp->NP_id()))
      return 0;
  }

  return (NP_content_type()->NP_equal(TCp->NP_content_type(),is_equivalent,
				      tcpl));
}


const char*
TypeCode_alias::NP_id() const
{
  return pd_repoId;
}


const char*
TypeCode_alias::NP_name() const
{
  return pd_name;
}


TypeCode_base*
TypeCode_alias::NP_content_type() const
{
  return ToTcBase(pd_content);
}


CORBA::Long
TypeCode_alias::NP_param_count() const
{
  return 2;
}


CORBA::Any*
TypeCode_alias::NP_parameter(CORBA::Long index) const
{
  CORBA::Any* rv = new CORBA::Any;

  try {
    switch (index) {
    case 0:
      *rv <<= (const char* )pd_name;
      break;
    case 1:
      *rv <<= pd_content;
      break;
    default:
      throw CORBA::TypeCode::Bounds();
    };
  } catch (...) {
    delete rv;
    throw;
  }

  return rv;
}


CORBA::Boolean
TypeCode_alias::NP_containsAnAlias()
{
  return 1;
}


TypeCode_base*
TypeCode_alias::NP_aliasExpand(TypeCode_pairlist* tcpl)
{
  if( ToTcBase(pd_content)->NP_containsAnAlias() )
    return ToTcBase(pd_content)->NP_aliasExpand(tcpl);
  else
    return TypeCode_collector::duplicateRef(ToTcBase(pd_content));
}


void
TypeCode_alias::removeOptionalNames()
{
  if (!pd_compactTc) {
    pd_compactTc = this;
    pd_name = (const char*)"";
    ToTcBase(pd_content)->removeOptionalNames();
  }
}

//////////////////////////////////////////////////////////////////////
////////////////////////// TypeCode_sequence /////////////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_sequence::TypeCode_sequence(CORBA::ULong maxLen,
				     TypeCode_base* elem_type)
  : TypeCode_base(CORBA::tk_sequence)
{
  // Initialise
  pd_length = maxLen;
  pd_content = TypeCode_collector::duplicateRef(elem_type);
  pd_offset = 0;

  NP_complete_recursive_sequences(this, 0);

  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);
}


TypeCode_sequence::TypeCode_sequence(CORBA::ULong maxLen,
				     CORBA::ULong offset)
  : TypeCode_base(CORBA::tk_sequence)
{
  pd_length = maxLen;
  pd_offset = offset;

  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);
}

TypeCode_sequence::TypeCode_sequence()
  : TypeCode_base(CORBA::tk_sequence)
{
  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);
}


TypeCode_sequence::~TypeCode_sequence() { }


void
TypeCode_sequence::NP_marshalComplexParams(cdrStream &s,
					   TypeCode_offsetTable* otbl) const
{
  if (!pd_complete)
    OMNIORB_THROW(BAD_TYPECODE,BAD_TYPECODE_Incomplete, CORBA::COMPLETED_NO);
  TypeCode_marshaller::marshal(ToTcBase(pd_content), s, otbl);
  pd_length >>= s;
}

TypeCode_base*
TypeCode_sequence::NP_unmarshalComplexParams(cdrStream &s,
					     TypeCode_offsetTable* otbl)
{
  TypeCode_sequence* _ptr = new TypeCode_sequence;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  _ptr->pd_content = TypeCode_marshaller::unmarshal(s, otbl);
  if( !ToTcBase(_ptr->pd_content)->complete() )
    // Not necassarily correct offset -- but I don't
    // think it matters.  We only use pd_offset when
    // creating typecodes.
    _ptr->pd_offset = 1;
  else
    _ptr->pd_offset = 0;

  _ptr->pd_length <<= s;
  _ptr->pd_complete = 1;

  return _ptr;
}

CORBA::Boolean
TypeCode_sequence::NP_complete_recursive_sequences(TypeCode_base*  tc,
						   CORBA::ULong offset)
{
  if (!pd_complete)
    {
      if (pd_offset != 0)
	{
	  if (pd_offset == offset)
	    {
	      // The offset is the one we're looking for, so duplicate the
	      // typecode_ptr and mark ourself as completed
	      pd_complete = 1;
	      pd_content = TypeCode_collector::duplicateRef(tc);

	      // Now would be a good time to mark nodes as containing loops,
	      // since all looped typecodes MUST contain a recursive sequence.
	      // See the TypeCode_collector and TypeCode_marshaller classes
	      // for more detail.
	      TypeCode_collector::markLoopMembers(tc);
	    }
	}
      else
	{
	  // Call the child typecode to complete itself
	  if (!CORBA::is_nil(pd_content))
	      pd_complete = ToTcBase(pd_content)->
	                        NP_complete_recursive_sequences(tc, offset+1);
	}
    }

  return pd_complete;
}

CORBA::Boolean
TypeCode_sequence::NP_complete_recursive(TypeCode_base* tc, const char* repoId)
{
  if (!pd_complete && !CORBA::is_nil(pd_content)) {
    pd_complete = ToTcBase(pd_content)->NP_complete_recursive(tc, repoId);
    TypeCode_collector::markLoopMembers(tc);
  }
  return pd_complete;
}


TypeCode_paramListType
TypeCode_sequence::NP_paramListType() const
{
  return plt_Complex;
}


CORBA::Boolean
TypeCode_sequence::NP_extendedEqual(const TypeCode_base*  TCp,
				    CORBA::Boolean is_equivalent,
				    const TypeCode_pairlist* tcpl) const
{
  if ((NP_kind() != TCp->NP_kind()) || (NP_length() != TCp->NP_length()))
    return 0;
  else
    return (NP_content_type()->NP_equal(TCp->NP_content_type(),
					is_equivalent,tcpl));
}


CORBA::ULong
TypeCode_sequence::NP_length() const
{
  return pd_length;
}


TypeCode_base*
TypeCode_sequence::NP_content_type() const
{
  // Sanity check that recursive sequences have been properly completed
  OMNIORB_ASSERT(!CORBA::is_nil(pd_content));
  return ToTcBase(pd_content);
}


CORBA::Long
TypeCode_sequence::NP_param_count() const
{
  return 2;
}


CORBA::Any*
TypeCode_sequence::NP_parameter(CORBA::Long index) const
{
  CORBA::Any* rv = new CORBA::Any;

  try {
    switch (index) {
    case 0:
      *rv <<= pd_length;
      break;
    case 1:
      *rv <<= pd_content;
      break;
    default:
      throw CORBA::TypeCode::Bounds();
    };
  } catch (...) {
    delete rv;
    throw;
  }

  return rv;
}


CORBA::Boolean
TypeCode_sequence::NP_containsAnAlias()
{
  if( pd_offset ) {
    // If any of the nodes included in the recursive loop need
    // to be replaced with an expanded version, then we need to
    // replace all of them.  So say we are a (sub) member of
    // a structure -- then if any of the members contains an
    // alias, then we need to be replaced.
    //  It would be very difficult to detect this however, so
    // lets play safe and say that this is kind of like an alias,
    // so say yes.  This just means that some recursive sequences
    // get duplicated needlessly.

    return 1;
  }
  else
    return ToTcBase(pd_content)->NP_containsAnAlias();
}


TypeCode_base*
TypeCode_sequence::NP_aliasExpand(TypeCode_pairlist* tcpl)
{
  TypeCode_sequence* tc = new TypeCode_sequence;
  TypeCode_pairlist pl(tcpl, tc, this);

  TypeCode_base* rcontent =
    (TypeCode_base*) TypeCode_pairlist::search(tcpl, ToTcBase(pd_content));

  if( rcontent ) {
    // Recursive sequence ...
    tc->pd_content = TypeCode_collector::duplicateRef(rcontent);
    // Not necassarily correct offset -- but I don't
    // think it matters.  We only use pd_offset when
    // creating typecodes.
    tc->pd_offset = 1;
  }
  else {
    tc->pd_content = ToTcBase(pd_content)->NP_aliasExpand(&pl);
    tc->pd_offset = 0;
  }
  tc->pd_length = pd_length;
  tc->pd_complete = 1;

  return tc;
}

void
TypeCode_sequence::removeOptionalNames()
{
  if (!pd_compactTc) {
    pd_compactTc = this;
    ToTcBase(pd_content)->removeOptionalNames();
  }
}


//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_array ///////////////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_array::TypeCode_array(CORBA::ULong length,
			       TypeCode_base* elem_type)
  : TypeCode_base(CORBA::tk_array)
{
  // Initialise
  pd_length = length;
  pd_content = TypeCode_collector::duplicateRef(elem_type);

  NP_complete_recursive_sequences(this, 0);

  generateAlignmentTable();
}


TypeCode_array::~TypeCode_array() {}


void
TypeCode_array::NP_marshalComplexParams(cdrStream &s,
					TypeCode_offsetTable* otbl) const
{
  TypeCode_marshaller::marshal(ToTcBase(pd_content), s, otbl);
  pd_length >>= s;
}

TypeCode_base*
TypeCode_array::NP_unmarshalComplexParams(cdrStream &s,
					  TypeCode_offsetTable* otbl)
{
  TypeCode_array* _ptr = new TypeCode_array;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  _ptr->pd_content = TypeCode_marshaller::unmarshal(s, otbl);
  _ptr->pd_length <<= s;
  _ptr->pd_complete = 1;
  _ptr->generateAlignmentTable();

  return _ptr;
}

CORBA::Boolean
TypeCode_array::NP_complete_recursive_sequences(TypeCode_base*  tc,
						   CORBA::ULong offset)
{
  if (!pd_complete)
    pd_complete =
      ToTcBase(pd_content)->NP_complete_recursive_sequences(tc, offset+1);

  return pd_complete;
}

CORBA::Boolean
TypeCode_array::NP_complete_recursive(TypeCode_base* tc, const char* repoId)
{
  if (!pd_complete)
    pd_complete = ToTcBase(pd_content)->NP_complete_recursive(tc, repoId);
  return pd_complete;
}

CORBA::Boolean
TypeCode_array::NP_extendedEqual(const TypeCode_base*  TCp,
				 CORBA::Boolean is_equivalent,
				 const TypeCode_pairlist* tcpl) const
{
  if ((NP_kind() != TCp->NP_kind()) || (NP_length() != TCp->NP_length()))
    return 0;
  else
    return (NP_content_type()->NP_equal(TCp->NP_content_type(),is_equivalent,tcpl));
}


CORBA::ULong
TypeCode_array::NP_length() const
{
  return pd_length;
}


TypeCode_base*
TypeCode_array::NP_content_type() const
{
  return ToTcBase(pd_content);
}


CORBA::Long
TypeCode_array::NP_param_count() const
{
  return 2;
}


CORBA::Any*
TypeCode_array::NP_parameter(CORBA::Long index) const
{
  CORBA::Any* rv = new CORBA::Any;

  try {
    switch (index) {
    case 0:
      *rv <<= pd_length;
      break;
    case 1:
      *rv <<= pd_content;
      break;
    default:
      throw CORBA::TypeCode::Bounds();
    };
  } catch (...) {
    delete rv;
    throw;
  }

  return rv;
}


void
TypeCode_array::generateAlignmentTable()
{
  const TypeCode_alignTable& eat = ToTcBase(pd_content)->alignmentTable();

  if( pd_length == 1 ) {
    pd_alignmentTable.set(eat);
  } else if( eat.is_simple() ) {
    // We can copy the whole array as a single block.
    pd_alignmentTable.setNumEntries(1);
    CORBA::ULong size_aligned = omni::align_to(eat[0].simple.size,
					       eat[0].simple.alignment);
    pd_alignmentTable.addSimple(eat[0].simple.alignment,
				eat[0].simple.size +
				(pd_length - 1) * size_aligned);
  } else if( eat.has_only_simple() ) {
    // We need to copy the first element separately, and then the
    // rest as a simple block. This is possible because a struct of
    // simple blocks will always finish with the same alignment
    // (modulo the max alignment requirement of the elements).
    pd_alignmentTable.setNumEntries(eat.entries() + 1);
    CORBA::ULong start = 0;
    for( unsigned i = 0; i < eat.entries(); i++ ) {
      pd_alignmentTable.add(eat, i);
      start = omni::align_to(start, eat[i].simple.alignment);
      start += eat[i].simple.size;
    }
    CORBA::ULong end = start;
    for( unsigned j = 0; j < eat.entries(); j++ ) {
      end = omni::align_to(end, eat[j].simple.alignment);
      end += eat[j].simple.size;
    }
    pd_alignmentTable.addSimple(omni::ALIGN_1,
				(pd_length - 1) * (end - start));
  } else {
    pd_alignmentTable.setNumEntries(1);
    pd_alignmentTable.addNasty(this);
  }
}


CORBA::Boolean
TypeCode_array::NP_containsAnAlias()
{
  return ToTcBase(pd_content)->NP_containsAnAlias();
}


TypeCode_base*
TypeCode_array::NP_aliasExpand(TypeCode_pairlist* tcpl)
{
  TypeCode_array* tc = new TypeCode_array;
  TypeCode_pairlist pl(tcpl, tc, this);

  tc->pd_content = ToTcBase(pd_content)->NP_aliasExpand(&pl);
  tc->pd_length = pd_length;
  tc->pd_complete = 1;
  tc->generateAlignmentTable();

  return tc;
}


void
TypeCode_array::removeOptionalNames()
{
  if (!pd_compactTc) {
    pd_compactTc = this;
    ToTcBase(pd_content)->removeOptionalNames();
  }
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_struct //////////////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_struct::TypeCode_struct(char* repositoryId, char* name,
				 TypeCode_struct::Member* members,
				 CORBA::ULong memberCount)
  : TypeCode_base(CORBA::tk_struct)
{
  pd_repoId = repositoryId;
  pd_name = name;
  pd_nmembers = memberCount;
  pd_members = members;
  NP_complete_recursive_sequences(this, 0);
  NP_complete_recursive(this, repositoryId);

  generateAlignmentTable();
}


TypeCode_struct::~TypeCode_struct()
{
  for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
    CORBA::string_free(pd_members[i].name);
    if( pd_members[i].type )  CORBA::release(pd_members[i].type);
  }
  delete[] pd_members;
}


void
TypeCode_struct::NP_marshalComplexParams(cdrStream &s,
					 TypeCode_offsetTable* otbl) const
{
  s.marshalRawString(pd_repoId);
  s.marshalRawString(pd_name);
  pd_nmembers >>= s;

  for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
    s.marshalRawString(pd_members[i].name);
    TypeCode_marshaller::marshal(ToTcBase(pd_members[i].type), s, otbl);
  }
}


TypeCode_base*
TypeCode_struct::NP_unmarshalComplexParams(cdrStream& s,
					   TypeCode_offsetTable* otbl)
{
  TypeCode_struct* _ptr = new TypeCode_struct;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  try {
    _ptr->pd_repoId = s.unmarshalRawString();
    _ptr->pd_name   = s.unmarshalRawString();
    _ptr->pd_nmembers <<= s;

    // We need to initialised the members of <pd_members> to zero
    // to ensure we can destroy this properly in the case of an
    // exception being thrown.
    _ptr->pd_members = new TypeCode_struct::Member[_ptr->pd_nmembers];
    for( CORBA::ULong j = 0; j < _ptr->pd_nmembers; j++ ) {
      _ptr->pd_members[j].name = 0;
      _ptr->pd_members[j].type = 0;
    }

    for( CORBA::ULong i = 0; i < _ptr->pd_nmembers; i++ ) {
      _ptr->pd_members[i].name = s.unmarshalRawString();
      _ptr->pd_members[i].type = TypeCode_marshaller::unmarshal(s, otbl);
    }
  }
  catch(...) {
    delete _ptr;
    throw;
  }

  _ptr->pd_complete = 1;
  _ptr->generateAlignmentTable();
  return _ptr;
}

CORBA::Boolean
TypeCode_struct::NP_complete_recursive_sequences(TypeCode_base*  tc,
						 CORBA::ULong offset)
{
  if (!pd_complete)
    {
      pd_complete = 1;

      for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
	pd_complete = pd_complete && ToTcBase(pd_members[i].type)
	  ->NP_complete_recursive_sequences(tc, offset + 1);
      }
    }

  return pd_complete;
}

CORBA::Boolean
TypeCode_struct::NP_complete_recursive(TypeCode_base* tc, const char* repoId)
{
  if (!pd_complete) {
    pd_complete = 1;

    for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
      pd_complete = pd_complete &&
	ToTcBase(pd_members[i].type)->NP_complete_recursive(tc, repoId);
    }
  }
  return pd_complete;
}


// OMG Interface:
CORBA::Boolean
TypeCode_struct::NP_extendedEqual(const TypeCode_base*  TCp,
				  CORBA::Boolean is_equivalent,
				  const TypeCode_pairlist* tcpl) const
{
  if (NP_kind() != TCp->NP_kind())
    return 0;

  if (is_equivalent) {
    if (NP_id() && TCp->NP_id())
      return NP_namesEqual(NP_id(),TCp->NP_id());
  }
  else {
    if (!NP_namesEqual(NP_id(),TCp->NP_id()))
      return 0;
  }

  if (pd_nmembers != TCp->NP_member_count())
    return 0;

  if (!is_equivalent && !NP_namesEqual(NP_name(),TCp->NP_name()))
    return 0;

  for( CORBA::ULong i=0; i < pd_nmembers; i++ ) {
    if (!is_equivalent && !NP_namesEqual(pd_members[i].name,
					 TCp->NP_member_name(i)))
      return 0;

    if (!ToTcBase(pd_members[i].type)->NP_equal(TCp->NP_member_type(i),
						is_equivalent, tcpl))
      return 0;
  }
  return 1;
}


const char*
TypeCode_struct::NP_id() const
{
  return pd_repoId;
}


const char*
TypeCode_struct::NP_name() const
{
  return pd_name;
}


CORBA::ULong
TypeCode_struct::NP_member_count() const
{
  return pd_nmembers;
}


const char*
TypeCode_struct::NP_member_name(CORBA::ULong index) const
{
  if( index >= pd_nmembers )  throw CORBA::TypeCode::Bounds();
  return pd_members[index].name;
}


TypeCode_base*
TypeCode_struct::NP_member_type(CORBA::ULong index) const
{
  if( index >= pd_nmembers )  throw CORBA::TypeCode::Bounds();
  return ToTcBase(pd_members[index].type);
}


CORBA::Long
TypeCode_struct::NP_param_count() const
{
  return 1 + pd_nmembers * 2;
}


CORBA::Any*
TypeCode_struct::NP_parameter(CORBA::Long index) const
{
  CORBA::Any* rv = new CORBA::Any;

  try {
    switch (index) {
    case 0:
      *rv <<= (const char*) pd_name;
      break;

    default:
      if( (index >= 1) && ((CORBA::ULong)index < (1 + pd_nmembers * 2)) ) {
	const CORBA::Long tmp = (index-1)/2;

	switch( (index - 1) % 2 ) {
	case 0:
	  *rv <<= (const char*) pd_members[tmp].name;
	  break;
	case 1:
	  *rv <<= pd_members[tmp].type;
	  break;
	}
      }
      else
	throw CORBA::TypeCode::Bounds();
    }
  }
  catch (...) {
    delete rv;
    throw;
  }

  return rv;
}


void
TypeCode_struct::generateAlignmentTable()
{
  unsigned num_entries = 0;
  int simple_size = 0;
  omni::alignment_t simple_alignment = omni::ALIGN_8;

  // Determine how many table entries we will need.
  for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
    TypeCode_base* mtc = ToTcBase(pd_members[i].type);
    const TypeCode_alignTable& mat = mtc->alignmentTable();

    for( unsigned j = 0; j < mat.entries(); j++ ) {
      switch( mat[j].type ) {
      case TypeCode_alignTable::it_simple:
	if( simple_size % mat[j].simple.alignment == 0 &&
	    mat[j].simple.alignment <= simple_alignment ) {
	  // If can, add onto existing simple ...
	  if( simple_size == 0 )  simple_alignment = mat[j].simple.alignment;
	  simple_size += mat[j].simple.size;
	} else {
	  simple_size = mat[j].simple.size;
	  simple_alignment = mat[j].simple.alignment;
	  num_entries++;
	}
	break;

      default:
	if( simple_size > 0 ) {
	  simple_size = 0;
	  simple_alignment = omni::ALIGN_8;
	  num_entries++;
	}
	num_entries++;
	break;
      }
    }
  }
  // And there may be an extra simple one at the end ...
  if( simple_size > 0 )  num_entries++;

  // Generate the entries.
  if( num_entries == 0 ) {
    pd_alignmentTable.setNumEntries(1);
    pd_alignmentTable.addSimple(omni::ALIGN_1, 0);
  } else {
    pd_alignmentTable.setNumEntries(num_entries);
    simple_size = 0;
    simple_alignment = omni::ALIGN_8;

    for( CORBA::ULong ii = 0; ii < pd_nmembers; ii++ ) {
      TypeCode_base* mtc = ToTcBase(pd_members[ii].type);
      const TypeCode_alignTable& mat = mtc->alignmentTable();

      for( unsigned j = 0; j < mat.entries(); j++ ) {
	switch( mat[j].type ) {
	case TypeCode_alignTable::it_simple:
	  if( simple_size % mat[j].simple.alignment == 0 &&
	      mat[j].simple.alignment <= simple_alignment ) {
	    // If can add onto existing simple ...
	    if( simple_size == 0 )  simple_alignment = mat[j].simple.alignment;
	    simple_size += mat[j].simple.size;
	  } else {
	    pd_alignmentTable.addSimple(simple_alignment, simple_size);
	    simple_size = mat[j].simple.size;
	    simple_alignment = mat[j].simple.alignment;
	  }
	  break;

	default:
	  if( simple_size > 0 ) {
	    pd_alignmentTable.addSimple(simple_alignment, simple_size);
	    simple_size = 0;
	    simple_alignment = omni::ALIGN_8;
	  }
	  pd_alignmentTable.add(mat, j);
	  break;
	}
      }
    }
    // And there may be an extra simple one at the end ...
    if( simple_size > 0 ) {
      pd_alignmentTable.addSimple(simple_alignment, simple_size);
    }
  }
}


CORBA::Boolean
TypeCode_struct::NP_containsAnAlias()
{
  for( CORBA::ULong i = 0; i < pd_nmembers; i++ )
    if( ToTcBase(pd_members[i].type)->NP_containsAnAlias() )
      return 1;

  return 0;
}


TypeCode_base*
TypeCode_struct::NP_aliasExpand(TypeCode_pairlist* tcpl)
{
  TypeCode_struct* tc = new TypeCode_struct;
  TypeCode_pairlist pl(tcpl, tc, this);

  tc->pd_repoId = pd_repoId;
  tc->pd_name = pd_name;
  tc->pd_nmembers = pd_nmembers;

  TypeCode_struct::Member* members = new TypeCode_struct::Member[pd_nmembers];
  tc->pd_members = members;

  for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
    if( ToTcBase(pd_members[i].type)->NP_containsAnAlias() )
      members[i].type = ToTcBase(pd_members[i].type)->NP_aliasExpand(&pl);
    else
      members[i].type =
	TypeCode_collector::duplicateRef(ToTcBase(pd_members[i].type));

    members[i].name = CORBA::string_dup(pd_members[i].name);
  }

  tc->pd_complete = 1;
  tc->generateAlignmentTable();

  return tc;
}

void
TypeCode_struct::removeOptionalNames()
{
  if (!pd_compactTc) {
    pd_compactTc = this;
    pd_name = (const char*)"";
    for (CORBA::ULong i=0; i< pd_nmembers; i++) {
      CORBA::string_free(pd_members[i].name);
      pd_members[i].name = CORBA::string_dup("");
      ToTcBase(pd_members[i].type)->removeOptionalNames();
    }
  }
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_except //////////////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_except::TypeCode_except(char* repositoryId, char* name,
				 TypeCode_struct::Member* members,
				 CORBA::ULong memberCount)
  : TypeCode_base(CORBA::tk_except)
{
  pd_repoId = repositoryId;
  pd_name = name;
  pd_nmembers = memberCount;
  pd_members = members;

  NP_complete_recursive_sequences(this, 0);
  NP_complete_recursive(this, repositoryId);

  generateAlignmentTable();
}


TypeCode_except::~TypeCode_except()
{
  for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
    CORBA::string_free(pd_members[i].name);
    if( pd_members[i].type )  CORBA::release(pd_members[i].type);
  }
  delete[] pd_members;
}


void
TypeCode_except::NP_marshalComplexParams(cdrStream &s,
					 TypeCode_offsetTable* otbl) const
{
  s.marshalRawString(pd_repoId);
  s.marshalRawString(pd_name);
  pd_nmembers >>= s;

  for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
    s.marshalRawString(pd_members[i].name);
    TypeCode_marshaller::marshal(ToTcBase(pd_members[i].type), s, otbl);
  }
}

TypeCode_base*
TypeCode_except::NP_unmarshalComplexParams(cdrStream& s,
					   TypeCode_offsetTable* otbl)
{
  TypeCode_except* _ptr = new TypeCode_except;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  try {
    _ptr->pd_repoId = s.unmarshalRawString();
    _ptr->pd_name   = s.unmarshalRawString();
    _ptr->pd_nmembers <<= s;

    // We need to initialised the members of <pd_members> to zero
    // to ensure we can destroy this properly in the case of an
    // exception being thrown.
    _ptr->pd_members = new TypeCode_struct::Member[_ptr->pd_nmembers];
    for( CORBA::ULong j = 0; j < _ptr->pd_nmembers; j++ ) {
      _ptr->pd_members[j].name = 0;
      _ptr->pd_members[j].type = 0;
    }

    for( CORBA::ULong i = 0; i < _ptr->pd_nmembers; i++ ) {
      _ptr->pd_members[i].name = s.unmarshalRawString();
      _ptr->pd_members[i].type = TypeCode_marshaller::unmarshal(s, otbl);
    }
  }
  catch(...) {
    delete _ptr;
    throw;
  }

  _ptr->pd_complete = 1;
  _ptr->generateAlignmentTable();
  return _ptr;
}


CORBA::Boolean
TypeCode_except::NP_complete_recursive_sequences(TypeCode_base*  tc,
						 CORBA::ULong offset)
{
  if (!pd_complete)
    {
      pd_complete = 1;

      for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
	pd_complete = pd_complete && ToTcBase(pd_members[i].type)
	  ->NP_complete_recursive_sequences(tc, offset + 1);
	}
    }

  return pd_complete;
}

CORBA::Boolean
TypeCode_except::NP_complete_recursive(TypeCode_base* tc, const char* repoId)
{
  if (!pd_complete) {
    pd_complete = 1;
    
    for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
      pd_complete = pd_complete &&
	ToTcBase(pd_members[i].type)->NP_complete_recursive(tc, repoId);
    }
  }
  return pd_complete;
}


// OMG Interface:
CORBA::Boolean
TypeCode_except::NP_extendedEqual(const TypeCode_base*  TCp,
				  CORBA::Boolean is_equivalent,
				  const TypeCode_pairlist* tcpl) const
{
  if (NP_kind() != TCp->NP_kind())
    return 0;

  if (strcmp(NP_id(), TCp->NP_id()) != 0)
    return 0;
  else if (is_equivalent)
    return 1;

  if (pd_nmembers != TCp->NP_member_count())
    return 0;

  if (!NP_namesEqual(NP_name(),TCp->NP_name()))
    return 0;

  for( CORBA::ULong i=0; i < pd_nmembers; i++ ) {
    if ((!NP_namesEqual(pd_members[i].name, TCp->NP_member_name(i))) ||
	(!ToTcBase(pd_members[i].type)->NP_equal(TCp->NP_member_type(i),
						 0, tcpl)))
      return 0;
  }

  return 1;
}


const char*
TypeCode_except::NP_id() const
{
  return pd_repoId;
}


const char*
TypeCode_except::NP_name() const
{
  return pd_name;
}


CORBA::ULong
TypeCode_except::NP_member_count() const
{
  return pd_nmembers;
}


const char*
TypeCode_except::NP_member_name(CORBA::ULong index) const
{
  if( index >= pd_nmembers )  throw CORBA::TypeCode::Bounds();
  return pd_members[index].name;
}


TypeCode_base*
TypeCode_except::NP_member_type(CORBA::ULong index) const
{
  if( index >= pd_nmembers )  throw CORBA::TypeCode::Bounds();
  return ToTcBase(pd_members[index].type);
}


CORBA::Long
TypeCode_except::NP_param_count() const
{
  return 1 + pd_nmembers * 2;
}


CORBA::Any*
TypeCode_except::NP_parameter(CORBA::Long index) const
{
  CORBA::Any* rv = new CORBA::Any;

  try {
    switch (index) {
    case 0:
      *rv <<= (const char*) pd_name;
      break;

    default:
      if( (index >= 1) && ((CORBA::ULong)index < (1 + pd_nmembers * 2)) ) {
	const CORBA::Long tmp = (index-1)/2;

	switch( (index - 1) % 2 ) {
	case 0:
	  *rv <<= (const char*) pd_members[tmp].name;
	  break;
	case 1:
	  *rv <<= pd_members[tmp].type;
	  break;
	}
      }
      else
	throw CORBA::TypeCode::Bounds();
    }
  }
  catch (...) {
    delete rv;
    throw;
  }

  return rv;
}


void
TypeCode_except::generateAlignmentTable()
{
  unsigned num_entries = 0;
  int simple_size = 0;
  omni::alignment_t simple_alignment = omni::ALIGN_8;

  // Determine how many table entries we will need.
  for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
    TypeCode_base* mtc = ToTcBase(pd_members[i].type);
    const TypeCode_alignTable& mat = mtc->alignmentTable();

    for( unsigned j = 0; j < mat.entries(); j++ ) {
      switch( mat[j].type ) {
      case TypeCode_alignTable::it_simple:
	if( simple_size % mat[j].simple.alignment == 0 &&
	    mat[j].simple.alignment <= simple_alignment ) {
	  // If can add onto existing simple ...
	  if( simple_size == 0 )  simple_alignment = mat[j].simple.alignment;
	  simple_size += mat[j].simple.size;
	} else {
	  simple_size = mat[j].simple.size;
	  simple_alignment = mat[j].simple.alignment;
	  num_entries++;
	}
	break;

      default:
	if( simple_size > 0 ) {
	  simple_size = 0;
	  simple_alignment = omni::ALIGN_8;
	  num_entries++;
	}
	num_entries++;
	break;
      }
    }
  }
  // And there may be an extra simple one at the end ...
  if( simple_size > 0 )  num_entries++;

  // Generate the entries.
  if( num_entries == 0 ) {
    pd_alignmentTable.setNumEntries(1);
    pd_alignmentTable.addSimple(omni::ALIGN_1, 0);
  } else {
    pd_alignmentTable.setNumEntries(num_entries);
    simple_size = 0;
    simple_alignment = omni::ALIGN_8;

    for( CORBA::ULong ii = 0; ii < pd_nmembers; ii++ ) {
      TypeCode_base* mtc = ToTcBase(pd_members[ii].type);
      const TypeCode_alignTable& mat = mtc->alignmentTable();

      for( unsigned j = 0; j < mat.entries(); j++ ) {
	switch( mat[j].type ) {
	case TypeCode_alignTable::it_simple:
	  if( simple_size % mat[j].simple.alignment == 0 &&
	      mat[j].simple.alignment <= simple_alignment ) {
	    // If can add onto existing simple ...
	    if( simple_size == 0 )  simple_alignment = mat[j].simple.alignment;
	    simple_size += mat[j].simple.size;
	  } else {
	    pd_alignmentTable.addSimple(simple_alignment, simple_size);
	    simple_size = mat[j].simple.size;
	    simple_alignment = mat[j].simple.alignment;
	  }
	  break;

	default:
	  if( simple_size > 0 ) {
	    pd_alignmentTable.addSimple(simple_alignment, simple_size);
	    simple_size = 0;
	    simple_alignment = omni::ALIGN_8;
	  }
	  pd_alignmentTable.add(mat, j);
	  break;
	}
      }
    }
    // And there may be an extra simple one at the end ...
    if( simple_size > 0 ) {
      pd_alignmentTable.addSimple(simple_alignment, simple_size);
    }
  }
}


CORBA::Boolean
TypeCode_except::NP_containsAnAlias()
{
  for( CORBA::ULong i = 0; i < pd_nmembers; i++ )
    if( ToTcBase(pd_members[i].type)->NP_containsAnAlias() )
      return 1;

  return 0;
}


TypeCode_base*
TypeCode_except::NP_aliasExpand(TypeCode_pairlist* tcpl)
{
  TypeCode_except* tc = new TypeCode_except;
  TypeCode_pairlist pl(tcpl, tc, this);

  tc->pd_repoId = pd_repoId;
  tc->pd_name = pd_name;
  tc->pd_nmembers = pd_nmembers;

  TypeCode_struct::Member* members = new TypeCode_struct::Member[pd_nmembers];
  tc->pd_members = members;

  for( CORBA::ULong i = 0; i < pd_nmembers; i++ ) {
    if( ToTcBase(pd_members[i].type)->NP_containsAnAlias() )
      members[i].type = ToTcBase(pd_members[i].type)->NP_aliasExpand(&pl);
    else
      members[i].type =
	TypeCode_collector::duplicateRef(ToTcBase(pd_members[i].type));

    members[i].name = CORBA::string_dup(pd_members[i].name);
  }

  tc->pd_complete = 1;
  tc->generateAlignmentTable();

  return tc;
}

void
TypeCode_except::removeOptionalNames()
{
  if (!pd_compactTc) {
    pd_compactTc = this;
    pd_name = (const char*)"";
    for (CORBA::ULong i=0; i< pd_nmembers; i++) {
      CORBA::string_free(pd_members[i].name);
      pd_members[i].name = CORBA::string_dup("");
      ToTcBase(pd_members[i].type)->removeOptionalNames();
    }
  }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// TypeCode_enum ///////////////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_enum::TypeCode_enum(const char* repositoryId,
			     const char* name,
			     const CORBA::EnumMemberSeq &members)
  : TypeCode_base(CORBA::tk_enum)
{
  pd_repoId = repositoryId;
  pd_name = name;
  pd_members = members;
  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addSimple(omni::ALIGN_4, 4);
}


TypeCode_enum::TypeCode_enum()
  : TypeCode_base(CORBA::tk_enum)
{
  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addSimple(omni::ALIGN_4, 4);
}


TypeCode_enum::~TypeCode_enum() {}


void
TypeCode_enum::NP_marshalComplexParams(cdrStream& s,
				       TypeCode_offsetTable* otbl) const
{
  s.marshalRawString(pd_repoId);
  s.marshalRawString(pd_name);

  // Can't use EnumMemberSeq's insertion operator since that would do
  // code set conversion.
  CORBA::ULong len = pd_members.length();
  len >>= s;

  const char* const* buffer = pd_members.get_buffer();
  for (CORBA::ULong i=0; i < len; i++)
    s.marshalRawString(buffer[i]);
}


TypeCode_base*
TypeCode_enum::NP_unmarshalComplexParams(cdrStream &s,
					 TypeCode_offsetTable* otbl)
{
  TypeCode_enum* _ptr = new TypeCode_enum;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  _ptr->pd_repoId = s.unmarshalRawString();
  _ptr->pd_name   = s.unmarshalRawString();

  CORBA::ULong len;
  len <<= s;

  _ptr->pd_members.length(len);
  char** buffer = _ptr->pd_members.get_buffer(0);

  for (CORBA::ULong i=0; i < len; i++)
    buffer[i] = s.unmarshalRawString();

  return _ptr;
}


CORBA::Boolean
TypeCode_enum::NP_extendedEqual(const TypeCode_base* TCp,
				CORBA::Boolean is_equivalent,
				const TypeCode_pairlist*) const
{
  if (NP_kind() != TCp->NP_kind())
    return 0;

  if (is_equivalent) {
    if (NP_id() && TCp->NP_id())
      return NP_namesEqual(NP_id(),TCp->NP_id());
  }
  else {
    if (!NP_namesEqual(NP_id(),TCp->NP_id()))
      return 0;
  }

  if (pd_members.length() != TCp->NP_member_count())
    return 0;

  if (!is_equivalent) {
    if (!NP_namesEqual(NP_name(),TCp->NP_name()))
      return 0;

    CORBA::ULong memberCount = pd_members.length();
    TypeCode_enum* TCe = (TypeCode_enum*) TCp;

    for( CORBA::ULong i=0; i < memberCount; i++ )
      if( !NP_namesEqual(pd_members[i], TCe->pd_members[i]) )
	return 0;
  }

  return 1;
}


const char*
TypeCode_enum::NP_id() const
{
  return pd_repoId;
}


const char*
TypeCode_enum::NP_name() const
{
  return pd_name;
}


CORBA::ULong
TypeCode_enum::NP_member_count() const
{
  return pd_members.length();
}


const char*
TypeCode_enum::NP_member_name(CORBA::ULong index) const
{
  if (pd_members.length() <= index)
    throw CORBA::TypeCode::Bounds();

  return pd_members[index];
}


CORBA::Long
TypeCode_enum::NP_param_count() const
{
  return 1 + member_count();
}


CORBA::Any*
TypeCode_enum::NP_parameter(CORBA::Long index) const
{
  CORBA::Any* rv = new CORBA::Any;

  try {
    switch (index) {
    case 0:
      *rv <<= (const char* ) pd_name;
      break;
    default:
      if( (index >= 1) && ((CORBA::ULong)index < (1+NP_member_count())) )
	*rv <<= (const char* )pd_members[index-1];
      else
	throw CORBA::TypeCode::Bounds();
    };
  } catch (...) {
    delete rv;
    throw;
  }

  return rv;
}


CORBA::Long
TypeCode_enum::NP_member_index(const char* name) const
{
  CORBA::ULong mc = pd_members.length();

  for( CORBA::ULong i = 0; i < mc; i++ )
    if( !strcmp(name, pd_members[i]) )
      return i;

  return -1;
}

void
TypeCode_enum::removeOptionalNames()
{
  if (!pd_compactTc) {
    pd_compactTc = this;
    pd_name = (const char*)"";
    for (CORBA::ULong i=0; i < pd_members.length(); i++) {
      pd_members[i] = (const char*)"";
    }
  }
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_union ///////////////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_union::TypeCode_union(const char* repositoryId,
			       const char* name,
			       TypeCode_base* discriminatorTC,
			       const CORBA::UnionMemberSeq& members)
  : TypeCode_base(CORBA::tk_union)
{
  const CORBA::ULong memberCount = members.length();

  if (memberCount == 0)
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidInitialSize,CORBA::COMPLETED_NO);

  pd_repoId = CORBA::string_dup(repositoryId);
  pd_name = CORBA::string_dup(name);
  pd_discrim_tc = TypeCode_collector::duplicateRef(discriminatorTC);
  pd_members.length(memberCount);

  pd_default = TYPECODE_UNION_IMPLICIT_DEFAULT;

  // Copy <members> into pd_members, checking the members are valid,
  // and extracting the label values.
  for( CORBA::ULong i = 0; i < memberCount; i++ ) {

    pd_members[i].aname = CORBA::string_dup(members[i].name);
    pd_members[i].atype =
      TypeCode_collector::duplicateRef(ToTcBase(members[i].type));

    CORBA::TypeCode_var lbl_tc = members[i].label.type();

    if( CORBA::_tc_octet->equivalent(lbl_tc) )
      {
	if( pd_default >= 0 )
	  OMNIORB_THROW(BAD_PARAM,
			BAD_PARAM_IllegitimateDiscriminatorType,
			CORBA::COMPLETED_NO);
	CORBA::Octet v;
	members[i].label >>= CORBA::Any::to_octet(v);
	if( v != CORBA::Octet(0) )
	  OMNIORB_THROW(BAD_PARAM,
			BAD_PARAM_IllegitimateDiscriminatorType,
			CORBA::COMPLETED_NO);
	pd_default = i;
	pd_members[i].alabel = Discriminator(0);
      }
    else
      {
	// This checks that:
	//  (a) the label type is the same as pd_discrim_tc
	//  (b) pd_discrim_tc is a legal union discriminator typecode
	// and throws an appropriate exception if either fails.
	pd_members[i].alabel =
	  TypeCode_union_helper::extractLabel(members[i].label, pd_discrim_tc);
      }

    // Check the discriminator isn't the same as one we've already
    // had. This makes the loop O(n^2). Oh well, n is probably
    // small...
    if ((int)i != pd_default) {
      for (CORBA::ULong j=0; j < i; j++) {
	if ((int)j == pd_default) continue;
	if (pd_members[j].alabel == pd_members[i].alabel)
	  OMNIORB_THROW(BAD_PARAM,
			BAD_PARAM_DuplicateLabelValue,
			CORBA::COMPLETED_NO);
      }
    }
  }

  if( pd_default == TYPECODE_UNION_IMPLICIT_DEFAULT &&
      !TypeCode_union_helper::has_implicit_default(this) )
    pd_default = TYPECODE_UNION_NO_DEFAULT;

  NP_complete_recursive_sequences(this, 0);
  NP_complete_recursive(this, repositoryId);

  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);

  pd_have_calculated_default_value = 0;
}


// This constructor is used for PR_union_tc()
TypeCode_union::TypeCode_union(const char* repositoryId,
			       const char* name,
			       TypeCode_base* discriminatorTC,
			       const CORBA::PR_unionMember* members,
			       CORBA::ULong memberCount, CORBA::Long deflt)
  : TypeCode_base(CORBA::tk_union)
{
  // Fill out the union TypeCode
  pd_repoId = repositoryId;
  pd_name = name;
  pd_discrim_tc = TypeCode_collector::duplicateRef(discriminatorTC);
  pd_default = deflt < 0 ? TYPECODE_UNION_IMPLICIT_DEFAULT : deflt;
  pd_members.length(memberCount);

  // Copy <members> into pd_members, checking the members are valid,
  // and extracting the label values.
  for( CORBA::ULong i = 0; i < memberCount; i++ ) {

    pd_members[i].aname = CORBA::string_dup(members[i].name);
    pd_members[i].atype =
      TypeCode_collector::duplicateRef(ToTcBase(members[i].type));
    pd_members[i].alabel = members[i].label;
  }

  if( pd_default == TYPECODE_UNION_IMPLICIT_DEFAULT &&
      !TypeCode_union_helper::has_implicit_default(this) )
    pd_default = TYPECODE_UNION_NO_DEFAULT;

  NP_complete_recursive_sequences(this, 0);
  NP_complete_recursive(this, repositoryId);

  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);

  pd_have_calculated_default_value = 0;
}


TypeCode_union::TypeCode_union()
  : TypeCode_base(CORBA::tk_union)
{
  pd_alignmentTable.setNumEntries(1);
  pd_alignmentTable.addNasty(this);

  pd_have_calculated_default_value = 0;
}


TypeCode_union::~TypeCode_union() {}


void
TypeCode_union::NP_marshalComplexParams(cdrStream &s,
					TypeCode_offsetTable* otbl) const
{
  s.marshalRawString(pd_repoId);
  s.marshalRawString(pd_name);
  TypeCode_marshaller::marshal(ToTcBase(pd_discrim_tc), s, otbl);
  pd_default >>= s;

  const CORBA::ULong memberCount = pd_members.length();
  memberCount >>= s;
  for( CORBA::ULong i = 0; i < memberCount; i++ )
    {
      TypeCode_union_helper::marshalLabel(pd_members[i].alabel,
					  pd_discrim_tc, s);
      s.marshalRawString(pd_members[i].aname);
      TypeCode_marshaller::marshal(ToTcBase(pd_members[i].atype), s, otbl);
    }
}


TypeCode_base*
TypeCode_union::NP_unmarshalComplexParams(cdrStream &s,
					  TypeCode_offsetTable* otbl)
{
  TypeCode_union* _ptr = new TypeCode_union;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  _ptr->pd_repoId = s.unmarshalRawString();
  _ptr->pd_name   = s.unmarshalRawString();
  _ptr->pd_discrim_tc = TypeCode_marshaller::unmarshal(s, otbl);
  _ptr->pd_default <<= s;

  if( _ptr->pd_default < 0 ) {
    if( TypeCode_union_helper::has_implicit_default(_ptr) )
      _ptr->pd_default = TYPECODE_UNION_IMPLICIT_DEFAULT;
    else
      _ptr->pd_default = TYPECODE_UNION_NO_DEFAULT;
  }

  CORBA::ULong memberCount;
  memberCount <<= s;

  _ptr->pd_members.length(memberCount);

  // Read in the different labels, names and types
  for( CORBA::ULong i = 0; i < memberCount; i++ )
    {
      _ptr->pd_members[i].alabel =
	TypeCode_union_helper::unmarshalLabel(_ptr->pd_discrim_tc, s);
      _ptr->pd_members[i].aname = s.unmarshalRawString();
      _ptr->pd_members[i].atype = TypeCode_marshaller::unmarshal(s, otbl);
    }

  _ptr->pd_complete = 1;

  return _ptr;
}


CORBA::Boolean
TypeCode_union::NP_complete_recursive_sequences(TypeCode_base*  tc,
						CORBA::ULong offset)
{
  if (!pd_complete)
    {
      pd_complete = 1;

      const CORBA::ULong memberCount = pd_members.length();
      for( CORBA::ULong i = 0; i < memberCount; i++ )
	pd_complete = pd_complete &&
	  ToTcBase(pd_members[i].atype)->
	    NP_complete_recursive_sequences(tc, offset+1);
    }

  return pd_complete;
}

CORBA::Boolean
TypeCode_union::NP_complete_recursive(TypeCode_base* tc, const char* repoId)
{
  if (!pd_complete) {
    pd_complete = 1;

    const CORBA::ULong memberCount = pd_members.length();
    for( CORBA::ULong i = 0; i < memberCount; i++ )
      pd_complete = pd_complete &&
	ToTcBase(pd_members[i].atype)->NP_complete_recursive(tc, repoId);
  }
  return pd_complete;
}


CORBA::Boolean
TypeCode_union::NP_extendedEqual(const TypeCode_base*  TCp,
				  CORBA::Boolean is_equivalent,
				  const TypeCode_pairlist* tcpl) const
{
  if (NP_kind() != TCp->NP_kind())
    return 0;

  if (is_equivalent) {
    if (NP_id() && TCp->NP_id())
      return NP_namesEqual(NP_id(),TCp->NP_id());
  }
  else {
    if (!NP_namesEqual(NP_id(),TCp->NP_id()))
      return 0;
  }

  if (!is_equivalent && !NP_namesEqual(NP_name(),TCp->NP_name()))
    return 0;

  if (NP_member_count() != TCp->NP_member_count())
    return 0;

  if (NP_default_index() != TCp->NP_default_index() &&
      (NP_default_index() >= 0 || TCp->NP_default_index() >= 0)) {
    return 0;
  }

  if (!(NP_discriminator_type()->NP_equal(TCp->NP_discriminator_type(),
					  is_equivalent, tcpl))) {
    return 0;
  }

  const CORBA::ULong memberCount = pd_members.length();

  TypeCode_union* uTCp = (TypeCode_union*)TCp;

  for( CORBA::ULong i = 0; i < memberCount; i++ ) {
    if ((CORBA::Long(i) != NP_default_index() &&
	 pd_members[i].alabel != uTCp->pd_members[i].alabel) ||
	!ToTcBase(pd_members[i].atype)->
	 NP_equal(ToTcBase(uTCp->pd_members[i].atype), is_equivalent, tcpl) ) {
      return 0;
    }

    if (!is_equivalent && !NP_namesEqual(pd_members[i].aname,
				      uTCp->pd_members[i].aname)) {
      return 0;
    }
  }

  return 1;
}


const char*
TypeCode_union::NP_id() const
{
  return pd_repoId;
}


const char*
TypeCode_union::NP_name() const
{
  return pd_name;
}


CORBA::ULong
TypeCode_union::NP_member_count() const
{
  return pd_members.length();
}


const char*
TypeCode_union::NP_member_name(CORBA::ULong index) const
{
  if (pd_members.length() <= index)
    throw CORBA::TypeCode::Bounds();

  return pd_members[index].aname;
}


TypeCode_base*
TypeCode_union::NP_member_type(CORBA::ULong index) const
{
  if (pd_members.length() <= index)
    throw CORBA::TypeCode::Bounds();

  return ToTcBase(pd_members[index].atype);
}


CORBA::Any*
TypeCode_union::NP_member_label(CORBA::ULong i) const
{
  if (pd_members.length() <= i)
    throw CORBA::TypeCode::Bounds();

  CORBA::Any* a = new CORBA::Any;
  if( !a )  _CORBA_new_operator_return_null();

  if (CORBA::Long(i) != pd_default) {
    TypeCode_union_helper::insertLabel(*a, pd_members[i].alabel, pd_discrim_tc);
  } else {
    (*a) <<= CORBA::Any::from_octet((CORBA::Octet)0);
  }
  return a;
}


TypeCode_base*
TypeCode_union::NP_discriminator_type() const
{
  return ToTcBase(pd_discrim_tc);
}


CORBA::Long
TypeCode_union::NP_default_index() const
{
  return pd_default;
}


CORBA::Long
TypeCode_union::NP_param_count() const
{
  return 2 + (member_count() * 3);
}


CORBA::Any*
TypeCode_union::NP_parameter(CORBA::Long index) const
{
  CORBA::Any* rv = new CORBA::Any;
  if( !rv )  _CORBA_new_operator_return_null();

  try {
    switch (index) {
    case 0:
      *rv <<= (const char* ) pd_name;
      break;
    case 1:
      *rv <<= pd_discrim_tc;
      break;
    default:
      if( (index >= 2) && ((CORBA::ULong)index < (2+NP_member_count()*3)) )
	{
	  const CORBA::Long tmp = (index-2)/3;

	  switch ((index-2) % 3) {
	  case 0:
	    {
	      CORBA::Any* a = NP_member_label(tmp);
	      delete rv;
	      rv = a;
	    }
	    break;
	  case 1:
	    *rv <<= (const char* )pd_members[tmp].aname;
	    break;
	  case 2:
	    *rv <<= pd_members[tmp].atype;
	    break;
	  };
	}
      else
	throw CORBA::TypeCode::Bounds();
    };
  } catch (...) {
    delete rv;
    throw;
  }

  return rv;
}


CORBA::Long
TypeCode_union::NP_index_from_discriminator(Discriminator d) const
{
  //?? We could consider having a sorted version of pd_members. This
  // could be an array of indicies into pd_members (as we need to keep
  // pd_members itself in the order defined in the IDL). We could then
  // implement this lookup very much more efficiently (binary search).

  CORBA::Long n = pd_members.length();

  for( CORBA::Long i = 0; i < n; i++ )
    if( pd_members[i].alabel == d && i != pd_default )
      return i;

  if( pd_default >= 0 )  return pd_default;
  else                   return -1;
}


TypeCode_union::Discriminator
TypeCode_union::NP_default_value()
{
  if( pd_default == TYPECODE_UNION_NO_DEFAULT )
    throw omniORB::fatalException(__FILE__,__LINE__,
       "TypeCode_union::NP_default_value() - union has no default"
				  " (not even implicit)");

  if( !pd_have_calculated_default_value ) {

    // We need to search for a suitable value.
    // Some of the following loops look like they might never terminate -
    // but in fact they will because we do know that there must be some
    // value of the given type which is not in the union.

    switch( ToTcBase(pd_discrim_tc)->NP_kind() ) {
    case CORBA::tk_char:
      {
	CORBA::UShort c = 0x0;
	while( 1 ) {
	  CORBA::Long i = NP_index_from_discriminator(c++);
	  if( i < 0 || i == pd_default ) {
	    pd_default_value = c - 1;
	    break;
	  }
	}
	break;
      }
    case CORBA::tk_boolean:
      {
	CORBA::Long i = NP_index_from_discriminator(0);
	if( i < 0 || i == pd_default )
	  pd_default_value = 0;
	else
	  pd_default_value = 1;
	break;
      }
    case CORBA::tk_short:
      {
	CORBA::Long c = -0x7fff;
	while( 1 ) {
	  CORBA::Long i = NP_index_from_discriminator(c++);
	  if( i < 0 || i == pd_default ) {
	    pd_default_value = c - 1;
	    break;
	  }
	}
	break;
      }
    case CORBA::tk_long:
      {
	CORBA::Long c = -0x7fffffff;
	while( 1 ) {
	  CORBA::Long i = NP_index_from_discriminator(c++);
	  if( i < 0 || i == pd_default ) {
	    pd_default_value = c - 1;
	    break;
	  }
	}
	break;
      }
    case CORBA::tk_ushort:
    case CORBA::tk_ulong:
    case CORBA::tk_enum:
      {
	CORBA::ULong c = 0x0;
	while( 1 ) {
	  CORBA::Long i = NP_index_from_discriminator(c++);
	  if( i < 0 || i == pd_default ) {
	    pd_default_value = c - 1;
	    break;
	  }
	}
	break;
      }
    default:
      // Just to stop compiler warnings ...
      break;
    }

    pd_have_calculated_default_value = 1;
  }

  return pd_default_value;
}


CORBA::Boolean
TypeCode_union::NP_containsAnAlias()
{
  if( ToTcBase(pd_discrim_tc)->NP_containsAnAlias() )
    return 1;

  for( CORBA::ULong i = 0; i < pd_members.length(); i++ )
    if( ToTcBase(pd_members[i].atype)->NP_containsAnAlias() )
      return 1;

  return 0;
}

TypeCode_base*
TypeCode_union::NP_aliasExpand(TypeCode_pairlist* tcpl)
{
  TypeCode_union* tc = new TypeCode_union();
  TypeCode_pairlist pl(tcpl, tc, this);

  tc->pd_repoId = pd_repoId;
  tc->pd_name = pd_name;
  if( ToTcBase(pd_discrim_tc)->NP_containsAnAlias() )
    tc->pd_discrim_tc = ToTcBase(pd_discrim_tc)->NP_aliasExpand(&pl);
  else
    tc->pd_discrim_tc =
      TypeCode_collector::duplicateRef(ToTcBase(pd_discrim_tc));
  tc->pd_default = pd_default;
  tc->pd_have_calculated_default_value = pd_have_calculated_default_value;
  tc->pd_default_value = pd_default_value;
  tc->pd_members.length(pd_members.length());

  for( CORBA::ULong i = 0; i < pd_members.length(); i++ ) {
    if( ToTcBase(pd_members[i].atype)->NP_containsAnAlias() )
      tc->pd_members[i].atype =
	ToTcBase(pd_members[i].atype)->NP_aliasExpand(&pl);
    else
      tc->pd_members[i].atype =
	TypeCode_collector::duplicateRef(ToTcBase(pd_members[i].atype));
    tc->pd_members[i].aname = pd_members[i].aname;
    tc->pd_members[i].alabel = pd_members[i].alabel;
  }

  tc->pd_complete = 1;

  return tc;
}


void
TypeCode_union::removeOptionalNames()
{
  if (!pd_compactTc) {
    pd_compactTc = this;
    pd_name = (const char*)"";
    ToTcBase(pd_discrim_tc)->removeOptionalNames();

    for (CORBA::ULong i=0; i< pd_members.length(); i++) {
      pd_members[i].aname = CORBA::string_dup("");
      ToTcBase(pd_members[i].atype)->removeOptionalNames();
    }
  }
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_abstract_interface //////////////
//////////////////////////////////////////////////////////////////////

TypeCode_abstract_interface::
TypeCode_abstract_interface(const char* repositoryId, const char* name)
  : TypeCode_objref(repositoryId, name, CORBA::tk_abstract_interface)
{
}


TypeCode_abstract_interface::TypeCode_abstract_interface()
  : TypeCode_objref(CORBA::tk_abstract_interface)
{
}


TypeCode_abstract_interface::~TypeCode_abstract_interface() {}


TypeCode_base*
TypeCode_abstract_interface::
NP_unmarshalComplexParams(cdrStream &s,
			  TypeCode_offsetTable* otbl)
{
  TypeCode_abstract_interface* _ptr = new TypeCode_abstract_interface;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  _ptr->pd_repoId = s.unmarshalRawString();
  _ptr->pd_name   = s.unmarshalRawString();
  _ptr->pd_complete = 1;

  return _ptr;
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_local_interface /////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_local_interface::
TypeCode_local_interface(const char* repositoryId, const char* name)
  : TypeCode_objref(repositoryId, name, CORBA::tk_local_interface)
{
}


TypeCode_local_interface::TypeCode_local_interface()
  : TypeCode_objref(CORBA::tk_local_interface)
{
}


TypeCode_local_interface::~TypeCode_local_interface() {}


TypeCode_base*
TypeCode_local_interface::
NP_unmarshalComplexParams(cdrStream &s,
			  TypeCode_offsetTable* otbl)
{
  TypeCode_local_interface* _ptr = new TypeCode_local_interface;

  otbl->addEntry(otbl->currentOffset(), _ptr);

  _ptr->pd_repoId = s.unmarshalRawString();
  _ptr->pd_name   = s.unmarshalRawString();
  _ptr->pd_complete = 1;

  return _ptr;
}




//////////////////////////////////////////////////////////////////////
//////////////////////// TypeCode_indirect ///////////////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_indirect::TypeCode_indirect(const char* repoId)
  : TypeCode_base(CORBA::_np_tk_indirect), pd_repoId(repoId), pd_resolved(0)
{ }

TypeCode_indirect::~TypeCode_indirect()
{
  if (pd_resolved) TypeCode_collector::releaseRef(pd_resolved);
}

void
TypeCode_indirect::NP_marshalSimpleParams(cdrStream& s,
					  TypeCode_offsetTable* tbl) const
{
  OMNIORB_ASSERT(pd_resolved);
  pd_resolved->NP_marshalSimpleParams(s, tbl);
}

void
TypeCode_indirect::NP_marshalComplexParams(cdrStream& s,
					   TypeCode_offsetTable* tbl) const
{
  OMNIORB_ASSERT(pd_resolved);
  pd_resolved->NP_marshalComplexParams(s, tbl);
}

CORBA::Boolean
TypeCode_indirect::NP_complete_recursive_sequences(TypeCode_base* tc,
						   CORBA::ULong offset)
{
  return 0;
}

CORBA::Boolean
TypeCode_indirect::NP_complete_recursive(TypeCode_base* tc,
					 const char* repoId)
{
  if (!pd_complete) {
    if (omni::strMatch(repoId, pd_repoId)) {
      OMNIORB_ASSERT(!pd_resolved);
      pd_complete = 1;
      pd_resolved = TypeCode_collector::duplicateRef(tc);
      return 1;
    }
  }
  return 0;
}

#define CHECK_RESOLVED do { \
  if (!pd_resolved) \
    OMNIORB_THROW(BAD_TYPECODE, \
                  BAD_TYPECODE_UnresolvedRecursiveTC, \
		  CORBA::COMPLETED_NO); \
} while(0)

CORBA::Boolean
TypeCode_indirect::NP_extendedEqual(const TypeCode_base* TCp,
				    CORBA::Boolean equivalent,
				    const TypeCode_pairlist* pl) const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_extendedEqual(TCp, equivalent, pl);
}

const char*
TypeCode_indirect::NP_id() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_id();
}

const char*
TypeCode_indirect::NP_name() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_name();
}

CORBA::ULong
TypeCode_indirect::NP_member_count() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_member_count();
}

const char*
TypeCode_indirect::NP_member_name(CORBA::ULong index) const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_member_name(index);
}

TypeCode_base*
TypeCode_indirect::NP_member_type(CORBA::ULong index) const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_member_type(index);
}

CORBA::Any*
TypeCode_indirect::NP_member_label(CORBA::ULong index) const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_member_label(index);
}

TypeCode_base*
TypeCode_indirect::NP_discriminator_type() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_discriminator_type();
}

CORBA::Long
TypeCode_indirect::NP_default_index() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_default_index();
}

CORBA::ULong
TypeCode_indirect::NP_length() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_length();
}

TypeCode_base*
TypeCode_indirect::NP_content_type() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_content_type();
}

CORBA::UShort
TypeCode_indirect::NP_fixed_digits() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_fixed_digits();
}

CORBA::Short
TypeCode_indirect::NP_fixed_scale() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_fixed_scale();
}

CORBA::Long
TypeCode_indirect::NP_param_count() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_param_count();
}

CORBA::Any*
TypeCode_indirect::NP_parameter(CORBA::Long p) const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_parameter(p);
}

CORBA::Short
TypeCode_indirect::NP_member_visibility(CORBA::ULong m) const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_member_visibility(m);
}

CORBA::ValueModifier
TypeCode_indirect::NP_type_modifier() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_type_modifier();
}

TypeCode_base*
TypeCode_indirect::NP_concrete_base_type() const
{
  CHECK_RESOLVED;
  return pd_resolved->NP_concrete_base_type();
}

CORBA::Boolean
TypeCode_indirect::NP_containsAnAlias()
{
  CHECK_RESOLVED;
  return pd_resolved->NP_containsAnAlias();
}

TypeCode_base*
TypeCode_indirect::NP_aliasExpand(TypeCode_pairlist* pl)
{
  CHECK_RESOLVED;
  return pd_resolved->NP_aliasExpand(pl);
}

void
TypeCode_indirect::removeOptionalNames()
{
  CHECK_RESOLVED;
  pd_resolved->removeOptionalNames();
}


#undef CHECK_RESOLVED


//////////////////////////////////////////////////////////////////////
//////////////////////// TypeCode_offsetTable ////////////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_offsetTable::TypeCode_offsetTable()
  : pd_table(0), pd_curr_offset(0), pd_parent_table(0),
    pd_parent_base_offset(0)
{
}


TypeCode_offsetTable::~TypeCode_offsetTable()
{
  TypeCode_offsetEntry* entry = pd_table;

  while (entry != 0)
    {
      TypeCode_offsetEntry* next_entry = entry->pd_next;

      delete entry;
      entry = next_entry;
    }
}


// Routine to create a child, encapsulating offsetTable
TypeCode_offsetTable::TypeCode_offsetTable(TypeCode_offsetTable* parent,
					   CORBA::Long base_offset)
  : pd_table(0), pd_curr_offset(base_offset),
    pd_parent_table(parent),
    pd_parent_base_offset(parent->currentOffset() - base_offset)
{
}


// Routine to add an offset->typecode mapping
void
TypeCode_offsetTable::addEntry(CORBA::Long offset, TypeCode_base* typecode)
{
  // If this table is a wrapper round another then correct the offset and
  // pass on the request
  if (pd_parent_table != 0)
    pd_parent_table->addEntry(offset + pd_parent_base_offset, typecode);
  else
    {
      // Otherwise, just look in this table directly
      TypeCode_offsetEntry* new_entry = new TypeCode_offsetEntry;

      new_entry->pd_next = pd_table;
      new_entry->pd_offset = offset;
      new_entry->pd_typecode = typecode;

      pd_table = new_entry;
    }
}


// Routines to retrieve typecode by offset or vica versa
TypeCode_base*
TypeCode_offsetTable::lookupOffset(CORBA::Long offset)
{
  // If this table is a wrapper round another then correct
  // the offset and pass on the request
  if (pd_parent_table != 0)
    return pd_parent_table->lookupOffset(offset + pd_parent_base_offset);

  // Visibroker's Java ORB gives out TypeCode indirections which are not
  // a multiple of 4. Rounding them up seems to solve the problem ...

  if( orbParameters::acceptMisalignedTcIndirections && (offset & 0x3) ) {
    if( omniORB::traceLevel > 1 ) {
      omniORB::logger log;
      log << "omniORB: WARNING - received TypeCode with mis-aligned indirection.\n";
    }
    offset = (offset + 3) & 0xfffffffc;
  }

  // Otherwise, just look in this table directly
  TypeCode_offsetEntry* entry = pd_table;

  while (entry != 0)
    {
      if( (CORBA::Long)entry->pd_offset == offset )
	return entry->pd_typecode;
      entry = entry->pd_next;
    }

  return 0;
}


CORBA::Boolean
TypeCode_offsetTable::lookupTypeCode(const TypeCode_base*  tc,
				     CORBA::Long &offset)
{
  // If this table is a wrapper round another then correct
  // the offset and pass on the request
  if (pd_parent_table != 0)
    {
      if (pd_parent_table->lookupTypeCode(tc, offset))
	{
	  offset = offset - pd_parent_base_offset;
	  return 1;
	}
      return 0;
    }

  // Otherwise, just look in this table directly
  TypeCode_offsetEntry* entry = pd_table;

  while (entry != 0)
    {
      if (entry->pd_typecode == tc)
	{
	  offset = entry->pd_offset;
	  return 1;
	}
      entry = entry->pd_next;
    }

  return 0;
}

//////////////////////////////////////////////////////////////////////
////////////////////////// TypeCode_pairlist /////////////////////////
//////////////////////////////////////////////////////////////////////

int
TypeCode_pairlist::contains(const TypeCode_pairlist* pl,
			    const TypeCode_base* t1, const TypeCode_base* t2)
{
  while( pl ) {

    if( t1 == pl->d_tc1 && t2 == pl->d_tc2 )
      return 1;

    pl = pl->d_next;
  }
  return 0;
}


const TypeCode_base*
TypeCode_pairlist::search(const TypeCode_pairlist* pl, const TypeCode_base* tc)
{
  while( pl ) {

    if( tc == pl->d_tc2 )
      return pl->d_tc1;

    pl = pl->d_next;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////
///////////////////////// TypeCode_marshaller ////////////////////////
//////////////////////////////////////////////////////////////////////

// Global typecode lock. This lock is used to protect the cached parameter
// list data, which is stored in typecodes when they are first marshalled.

// Initialised in check_static_data_is_initialised().
static omni_tracedmutex* pd_cached_paramlist_lock = 0;


void
TypeCode_marshaller::marshal(TypeCode_base* tc,
			     cdrStream& s,
			     TypeCode_offsetTable* otbl)
{
  CORBA::ULong tck = tc->NP_kind();

  // Follow any TypeCode_indirect objects created by
  // ORB::create_recursive_tc().
  while (tck == 0xffffffff)
    {
      tc = ((TypeCode_indirect*)tc)->NP_resolved();
      tck = tc->NP_kind();
    }

  // If this _exact_ typecode has already been marshalled into the stream
  // then just put in an indirection
  CORBA::Long tc_offset;
  if( orbParameters::useTypeCodeIndirections &&
      otbl->lookupTypeCode(tc, tc_offset) )
    {
      // The desired typecode was found, so write out an indirection
      CORBA::ULong tck_indirect = 0xffffffff;
      tck_indirect >>= s;

      // Now write out the offset
      CORBA::Long offset = tc_offset - (s.currentOutputPtr());
      offset >>= s;
    }
  else
    {
      // Write out the Kind
      tck >>= s;

      // Set the current offset of the offset table
      otbl->setOffset(s.currentOutputPtr() - 4);

      // Establish whether there are parameters to marshal
      switch( paramListType(tck) ) {

      case plt_None:
	// No parameters
	break;

      case plt_Simple:
	// Simple parameter list
	tc->NP_marshalSimpleParams(s, otbl);
	break;

      case plt_Complex:
	// Complex parameter list
	{
	  CORBA::Boolean has_cached_paramlist = 0;
	  cdrMemoryStream* paramlist = 0;

	  // The typecode is complex and wasn't found, so add it to the table
	  otbl->addEntry(otbl->currentOffset(), tc);

#if 0
	  // This is broken!!!!!

	  // Is there already a cached form of the parameter list?
	  if( !tc->pd_loop_member ) {
	    omni_tracedmutex_lock l(*pd_cached_paramlist_lock);

	    has_cached_paramlist = tc->pd_cached_paramlist != 0;
	  }
#endif

	  if( has_cached_paramlist ) {
	    paramlist = tc->pd_cached_paramlist;
	  } else {
	    // Create a MemBufferedStream to marshal the data into
	    paramlist = new cdrMemoryStream();

	    try {
	      // Create a child TypeCode_offsetTable with the correct base
	      // offset.
	      //  NB: When the offsetTable is passed to us, the currentOffset
	      // value will indicate the START of the typecode we're
	      // marshalling.  Relative to the start of the encapsulated
	      // data, this location has offset -8, allowing four bytes for
	      // the TypeCode Kind and four for the encapsulation size.
	      TypeCode_offsetTable offsetTbl(otbl, -8);

	      // Write out the byteorder
	      (*paramlist).marshalOctet(omni::myByteOrder);

	      // Call the supplied typecode object to marshal its complex
	      // parameter data into the temporary stream.
	      tc->NP_marshalComplexParams(*paramlist, &offsetTbl);

	      // And we're done!
	    }
	    catch (...) {
	      if( paramlist != 0 )  delete paramlist;
	      throw;
	    }
	  }

	  // Now write the size of the encapsulation out to the main stream
	  ::operator>>= ((CORBA::ULong)paramlist->bufSize(), s);

	  // And copy the data out to the main stream
	  s.put_octet_array((CORBA::Char*) paramlist->bufPtr(),
			    paramlist->bufSize());

#if 0
	  // Ensure that the paramlist is freed, or saved as a cached
	  // param list if not a part of a loop.
	  if( !has_cached_paramlist ){
	    if( tc->pd_loop_member ){
	      delete paramlist;
	    }else{
	      omni_tracedmutex_lock l(*pd_cached_paramlist_lock);

	      // Check some other thread hasn't made the parameter list ...
	      if( tc->pd_cached_paramlist == 0 )
		tc->pd_cached_paramlist = paramlist;
	      else
		delete paramlist;
	    }
	  }
#else
	  delete paramlist;
#endif
	  break;
	}
      } // switch( paramListType(tck) ) {
    }
}


TypeCode_base*
TypeCode_marshaller::unmarshal(cdrStream& s,
			       TypeCode_offsetTable* otbl)
{
  // Read the kind from the stream
  CORBA::ULong tck;
  tck <<= s;

  // Set the current position value in the offsetTable
  otbl->setOffset(s.currentInputPtr()-4);

  // Depending on the kind, create the correct type of TypeCode class

  // Each simple typecode class provides a NP_unmarshalSimpleParams
  // static function which reads the parameters in from a stream &
  // constructs a typecode of that type.  NP_unmarshalSimpleParams is
  // passed the current TypeCode_offsetTable, in order that indirection
  // typecodes can be correctly interpreted.

  // Each complex typecode class provides a NP_unmarshalComplexParams
  // static function which reads the parameters in from a stream &
  // constructs a typecode of that type.  NP_unmarshalComplexParams is
  // passed a child TypeCode_offsetTable and MemBufferedStream, in order
  // that indirection typecodes can be correctly interpreted.

  switch( paramListType(tck) ) {

    // TypeCode Kinds with no parameters
  case plt_None:
    switch (tck) {

      // Indirection typecode
    case 0xffffffff:
      {
	CORBA::Long currpos = s.currentInputPtr();
	CORBA::Long offset;

	// Read in the offset within the GIOP message
	offset <<= s;

	// Now look it up in the table
	TypeCode_base* tc = otbl->lookupOffset(offset+currpos);
	if (tc == 0)
	  OMNIORB_THROW(MARSHAL,
			MARSHAL_InvalidIndirection,
			CORBA::COMPLETED_NO);

	return TypeCode_collector::duplicateRef(tc);
      }

    // Simple types with no parameters
    case CORBA::tk_null:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_null));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_null));
    case CORBA::tk_void:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_void));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_void));
    case CORBA::tk_short:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_short));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_short));
    case CORBA::tk_ushort:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_ushort));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_ushort));
    case CORBA::tk_long:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_long));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_long));
    case CORBA::tk_ulong:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_ulong));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_ulong));
    case CORBA::tk_float:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_float));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_float));
    case CORBA::tk_double:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_double));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_double));
    case CORBA::tk_boolean:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_boolean));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_boolean));
    case CORBA::tk_char:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_char));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_char));
    case CORBA::tk_octet:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_octet));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_octet));
    case CORBA::tk_any:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_any));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_any));
    case CORBA::tk_TypeCode:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_TypeCode));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_TypeCode));
    case CORBA::tk_Principal:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_Principal));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_Principal));
#ifdef HAS_LongLong
    case CORBA::tk_longlong:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_longlong));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_longlong));
    case CORBA::tk_ulonglong:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_ulonglong));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_ulonglong));
#endif
#ifdef HAS_LongDouble
    case CORBA::tk_longdouble:
      otbl->addEntry(otbl->currentOffset(), ToTcBase(CORBA::_tc_longdouble));
      return TypeCode_collector::duplicateRef(ToTcBase(CORBA::_tc_longdouble));
#endif

    default:
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidTypeCodeKind, CORBA::COMPLETED_NO);
    };
    // Never reach here

    // TypeCode Kinds with simple parameter lists
  case plt_Simple:

    switch (tck) {

    case CORBA::tk_string:
      return TypeCode_string::NP_unmarshalSimpleParams(s, otbl);

    case CORBA::tk_wstring:
      return TypeCode_wstring::NP_unmarshalSimpleParams(s, otbl);

    case CORBA::tk_fixed:
      return TypeCode_fixed::NP_unmarshalSimpleParams(s, otbl);

    default:
      OMNIORB_THROW(MARSHAL,MARSHAL_InvalidTypeCodeKind, CORBA::COMPLETED_NO);
    };
    // Never reach here

    // TypeCode Kinds with complex parameter lists
  case plt_Complex:
    {
      // Create a child TypeCode_offsetTable with the correct base offset.
      //  NB: When the offsetTable is passed to us, the currentOffset value
      // will indicate the START of the typecode we're unmarshalling.
      // Relative to the start of the encapsulated data, this location has
      // offset -8, allowing four bytes for the TypeCode Kind and four for
      // the encapsulation size.
      TypeCode_offsetTable tbl(otbl, -8);

      // Read the size of the encapsulation
      CORBA::ULong size;
      size <<= s;

      // Create a buffered stream to handle the encapsulation
      // and read the data in.
      //?? Can we do this without lots of copying? Is it worth it?
      cdrMemoryStream mbs(size);
      s.copy_to(mbs, size);

      // Get the byte order
      CORBA::Boolean byteorder = mbs.unmarshalBoolean();
      mbs.setByteSwapFlag(byteorder);

      // Now switch on the Kind to call the appropriate
      // unmarshalComplexParams fn.
      switch (tck) {

      case CORBA::tk_objref:
	return TypeCode_objref::NP_unmarshalComplexParams(mbs, &tbl);

      case CORBA::tk_alias:
	return TypeCode_alias::NP_unmarshalComplexParams(mbs, &tbl);

      case CORBA::tk_sequence:
	return TypeCode_sequence::NP_unmarshalComplexParams(mbs, &tbl);

      case CORBA::tk_array:
	return TypeCode_array::NP_unmarshalComplexParams(mbs, &tbl);

      case CORBA::tk_struct:
	return TypeCode_struct::NP_unmarshalComplexParams(mbs, &tbl);

      case CORBA::tk_except:
	return TypeCode_except::NP_unmarshalComplexParams(mbs, &tbl);

      case CORBA::tk_enum:
	return TypeCode_enum::NP_unmarshalComplexParams(mbs, &tbl);

      case CORBA::tk_union:
	return TypeCode_union::NP_unmarshalComplexParams(mbs, &tbl);

      default:
	OMNIORB_THROW(MARSHAL,
		      MARSHAL_InvalidTypeCodeKind,
		      CORBA::COMPLETED_NO);
      };
      // Never reach here
    }

  default:
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidTypeCodeKind, CORBA::COMPLETED_NO);
    // Never reach here
  };

#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

// Function used to establish whether a particular typecode Kind
// has no parameters, simple parameters, or complex parameters
TypeCode_paramListType
TypeCode_marshaller::paramListType(CORBA::ULong kind)
{
  static TypeCode_paramListType plt[] = {
    plt_None,    // null
    plt_None,    // void
    plt_None,    // short
    plt_None,    // long
    plt_None,    // ushort
    plt_None,    // ulong
    plt_None,    // float
    plt_None,    // double
    plt_None,    // boolean
    plt_None,    // char
    plt_None,    // octet
    plt_None,    // any
    plt_None,    // typecode
    plt_None,    // principal
    plt_Complex, // objref
    plt_Complex, // struct
    plt_Complex, // union
    plt_Complex, // enum
    plt_Simple,  // string
    plt_Complex, // sequence
    plt_Complex, // array
    plt_Complex, // alias
    plt_Complex, // except
    plt_None,    // longlong
    plt_None,    // ulonglong
    plt_None,    // long double
    plt_None,    // wchar
    plt_Simple,  // wstring
    plt_Simple,  // fixed
    plt_Complex, // value
    plt_Complex, // value_box
    plt_Complex, // native
    plt_Complex, // abstract_interface
    plt_Complex  // local_interface
  };

  if( kind == 0xffffffff )  return plt_None;

  if( kind < sizeof(plt) / sizeof(TypeCode_paramListType) )
    return plt[kind];

  OMNIORB_THROW(MARSHAL, MARSHAL_InvalidTypeCodeKind, CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
  return plt_None;
#endif
}

//////////////////////////////////////////////////////////////////////
///////////////////////// TypeCode_collector /////////////////////////
//////////////////////////////////////////////////////////////////////

// Global typecode lock.  This lock guarantees only one duplicate/release is
// in progress at any one time.

// Initialised in check_static_data_is_initialised().
static omni_tracedmutex* pd_refcount_lock = 0;


// Duplicating typecodes

TypeCode_base*
TypeCode_collector::duplicateRef(TypeCode_base* tc)
{
  omni_tracedmutex_lock l(*pd_refcount_lock);

  //  if (tc->pd_tck == CORBA::tk_null) abort();

  tc->pd_ref_count++;

  return tc;
}

// Releasing typecodes

void
TypeCode_collector::releaseRef(TypeCode_base* tc)
{
  CORBA::Boolean node_can_be_freed = 0;

  {
    omni_tracedmutex_lock l(*pd_refcount_lock);

    // If the reference count is already zero then this node is already
    // being deleted
    if (tc->pd_ref_count == 0) {
      return;
    }
    // If the reference count has hit 1 then we can delete the node
    if (tc->pd_ref_count == 1)
      {
	node_can_be_freed = 1;
      }
    else
      {
	// Is this typecode part of a loop?
	if (tc->pd_loop_member)
	  {
	    // Yes, so are the references to it real or because of the loop?
	    countInternalRefs(tc);

	    if (checkInternalRefs(tc, 0))
	      {
		node_can_be_freed = 1;
		tc->pd_ref_count = 0;
	      }
	    else
	      tc->pd_ref_count--;
	  }
	else
	  tc->pd_ref_count--;
      }
  }

  // Now delete the node's storage, if possible
  if (node_can_be_freed)
    {
      delete tc;
    }
}

// Marking typecodes that are part of one or more loops

void
TypeCode_collector::markLoopMembers(TypeCode_base* tc)
{
  // Lock the typecode space before doing this
  omni_tracedmutex_lock l(*pd_refcount_lock);

  markLoops(tc, 0);
}

// -- - Internal reference counting fns

// These functions could be split up and distributed amongs the typecode
// classes they affect but instead I've used switch statements to keep
// the functions in one piece, for clarity.

// NB : It is required that pd_refcount_lock be held for these to operate
// as desired.

// -- markLoops
// Given a typecode and an initial depth count (usually zero), markloops will
// follow the TypeCode tree to establish where it contains loops.

static inline CORBA::ULong _minimum(CORBA::ULong a, CORBA::ULong b)
{
  return a < b ? a : b;
}

CORBA::ULong
TypeCode_collector::markLoops(TypeCode_base* tc, CORBA::ULong depth)
{
  // Have we visited this node before?
  if (tc->pd_mark)
    {
      return tc->pd_internal_depth - 1;
    }
  else
    {
      // No, so mark that we have and set an initial value for the depth count
      tc->pd_mark = 1;
      tc->pd_internal_depth = depth+1;

      // Now enumerate the child nodes, if any, to find out what the
      // lowest-depth node they can access is.
      switch (tc->NP_kind()) {

      case CORBA::tk_alias:
      case CORBA::tk_array:
	tc->pd_internal_depth = markLoops(tc->NP_content_type(), depth+1);
	break;

      case CORBA::tk_sequence:
	if (((TypeCode_sequence*)tc)->PR_content_is_assigned())
	  tc->pd_internal_depth = markLoops(tc->NP_content_type(), depth+1);
	break;

      case CORBA::tk_struct:
      case CORBA::tk_except:
      case CORBA::tk_union:
	{
	  CORBA::ULong memberCount = tc->NP_member_count();

	  // Enumerate the member typecodes
	  for( CORBA::ULong i = 0; i < memberCount; i++ )
	    {
	      tc->pd_internal_depth = _minimum(tc->pd_internal_depth,
					       markLoops(tc->NP_member_type(i),
							 depth+1));
	    }

	  break;
	}

      case CORBA::_np_tk_indirect:
	{
	  tc->pd_internal_depth =
	    markLoops(((TypeCode_indirect*)tc)->NP_resolved(), depth+1);
	  break;
	}

      default:
	break;
      };

      // Now check whether or not we're part of a loop
      if (tc->pd_internal_depth <= depth) {
	tc->pd_loop_member = 1;
      }
      else
	tc->pd_loop_member = 0;

      // Clear the mark
      tc->pd_mark = 0;
    }

  // Return the least-deep accessible node
  return tc->pd_internal_depth;
}

// -- countInternalRefs
// Follows the typecode tree round, marking the nodes with the number of
// references to them that are actually just internal ones.
// If all the nodes in a loop have the same number of references as internal
// references then this indicates that the loop is, in fact, 'floating'.

void
TypeCode_collector::countInternalRefs(TypeCode_base* tc)
{
  // Increase the internal reference count to this node
  tc->pd_internal_ref_count++;

  // If this node is already marked then don't check its children
  if (!tc->pd_mark)
    {
      tc->pd_mark = 1;

      // Now *think* of the children!
      switch (tc->NP_kind()) {

      case CORBA::tk_alias:
      case CORBA::tk_array:
      case CORBA::tk_sequence:
	countInternalRefs(tc->NP_content_type());
	break;

      case CORBA::tk_struct:
      case CORBA::tk_except:
      case CORBA::tk_union:
	{
	  CORBA::ULong memberCount = tc->NP_member_count();

	  // Enumerate the member typecodes
	  for( CORBA::ULong i = 0; i < memberCount; i++ )
	    countInternalRefs(tc->NP_member_type(i));

	  break;
	}

      case CORBA::_np_tk_indirect:
	{
	  countInternalRefs(((TypeCode_indirect*)tc)->NP_resolved());
	  break;
	}

      default:
	break;
      }
    }
}

// -- checkInternalRefs
// Follows the typecode tree, reading back the internal reference count
// values and checking them against the external reference counts if
// necessary.
// The depth counting system used to mark looped typecodes is used here
// to avoid checking the internal reference counts of nodes that aren't
// in the loop(s) we're interested in.
// If the depth count returned by a child node exceeds the parent's depth
// then we know that that particular child node is not in a loop with
// the parent node and so isn't relevant to the test.
// This function returns FALSE if the node is part of a loop in which another
// member is referenced, or returns TRUE if the node is not part of a loop
// or the loop is not externally referenced at all.

CORBA::Boolean
TypeCode_collector::checkInternalRefs(TypeCode_base* tc, CORBA::ULong depth)
{
  CORBA::Boolean loop_can_be_freed = 1;

  // If this node's mark has already been cleared then don't check its children
  if (tc->pd_mark)
    {
      CORBA::ULong internal_ref_count = tc->pd_internal_ref_count;

      // Clear the mark & internal ref count & set the depth value
      tc->pd_mark = 0;
      tc->pd_internal_depth = depth;
      tc->pd_internal_ref_count = 0;

      // Now *think* of the children!
      switch (tc->NP_kind()) {

      case CORBA::tk_alias:
      case CORBA::tk_array:
      case CORBA::tk_sequence:
	{
	  TypeCode_base* child = tc->NP_content_type();
	  CORBA::Boolean child_can_be_freed =
	    checkInternalRefs(child, depth+1);

	  tc->pd_internal_depth = child->pd_internal_depth;

	  // Is this node part of a loop involving the child node?
	  if (child->pd_internal_depth <= depth)
	    {
	      // Yes, so if the child node can't be freed then neither can we
	      if (!child_can_be_freed)
		loop_can_be_freed = 0;
	      else
		{
		  // Child can be freed, so we should check our internal
		  // references to see if we can be, too.
		  if (internal_ref_count < tc->pd_ref_count)
		    loop_can_be_freed = 0;
		}
	    }

	  break;
	}

      case CORBA::tk_struct:
      case CORBA::tk_except:
      case CORBA::tk_union:
	{
	  CORBA::Boolean can_be_freed = 1;
	  CORBA::ULong memberCount = tc->NP_member_count();

	  // Enumerate the member typecodes
	  for( CORBA::ULong i = 0; i < memberCount; i++ )
	    {
	      TypeCode_base* child = tc->NP_member_type(i);
	      CORBA::Boolean child_can_be_freed =
		checkInternalRefs(child, depth+1);

	      tc->pd_internal_depth = _minimum(tc->pd_internal_depth,
					       child->pd_internal_depth);

	      // Is this node part of a loop involving the child node?
	      if (child->pd_internal_depth <= depth)
		{
		  // Yes, so if the child node can't be freed then
		  // neither can we.
		  if (!child_can_be_freed)
		    loop_can_be_freed = 0;
		  else
		    {
		      // Child can be freed, so we should check our internal
		      // references to see if we can be, too.
		      if (internal_ref_count < tc->pd_ref_count)
			loop_can_be_freed = 0;
		    }
		}
	    }

	  break;
	}

      case CORBA::_np_tk_indirect:
	{
	  TypeCode_base* child = ((TypeCode_indirect*)tc)->NP_resolved();
	  CORBA::Boolean child_can_be_freed =
	    checkInternalRefs(child, depth+1);

	  tc->pd_internal_depth = child->pd_internal_depth;

	  // Is this node part of a loop involving the child node?
	  if (child->pd_internal_depth <= depth)
	    {
	      // Yes, so if the child node can't be freed then neither can we
	      if (!child_can_be_freed)
		loop_can_be_freed = 0;
	      else
		{
		  // Child can be freed, so we should check our internal
		  // references to see if we can be, too.
		  if (internal_ref_count < tc->pd_ref_count)
		    loop_can_be_freed = 0;
		}
	    }

	  break;
	}

      default:
	break;
      }
    }

  // We ONLY return true if this node is part of a loop AND can be freed
  if (tc->pd_internal_depth <= depth)
    return loop_can_be_freed;
  else
    return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////// TypeCode_union_helper ///////////////////////
//////////////////////////////////////////////////////////////////////

TypeCode_union::Discriminator
TypeCode_union_helper::extractLabel(const CORBA::Any& label,
				    CORBA::TypeCode_ptr dtc)
{
  // When the discriminator is a long, short, unsigned short or unsigned long,
  // we have to cast the label value from any of these kinds and check
  // if it is within the integer range of the discriminator.
  CORBA::TCKind lbl_kind;
  TypeCode_union::Discriminator lbl_value;
  CORBA::Boolean sign = 0;    // 1 == signed.
  CORBA::TypeCode_var aetc = TypeCode_base::aliasExpand(ToTcBase(dtc));

  {
    CORBA::TypeCode_var lbl_tc = label.type();
    CORBA::TypeCode_var ae_lbl_tc=TypeCode_base::aliasExpand(ToTcBase(lbl_tc));
    lbl_kind = ToTcBase(ae_lbl_tc)->NP_kind();

    switch (lbl_kind) {
    case CORBA::tk_char:
      {
	CORBA::Char c;
	label >>= CORBA::Any::to_char(c);
	lbl_value = c;
	break;
      }
    case CORBA::tk_boolean:
      {
	CORBA::Boolean c;
	label >>= CORBA::Any::to_boolean(c);
	lbl_value = ((c)? 1 : 0);
	break;
      }
    case CORBA::tk_octet:
      {
	CORBA::Octet c;
	label >>= CORBA::Any::to_octet(c);
	lbl_value = c;
	break;
      }
    case CORBA::tk_short:
      {
	CORBA::Short c;
	label >>= c;
	lbl_value = c;
	sign = 1;
	break;
      }
    case CORBA::tk_ushort:
      {
	CORBA::UShort c;
	label >>= c;
	lbl_value = c;
	break;
      }
    case CORBA::tk_long:
      {
	CORBA::Long c;
	label >>= c;
	lbl_value = c;
	sign = 1;
	break;
      }
    case CORBA::tk_ulong:
      {
	CORBA::ULong c;
	label >>= c;
	lbl_value = c;
	break;
      }
#ifdef HAS_LongLong
    case CORBA::tk_longlong:
      {
	CORBA::LongLong c;
	label >>= c;
	lbl_value = c;
	sign = 1;
	break;
      }
    case CORBA::tk_ulonglong:
      {
	CORBA::ULongLong c;
	label >>= c;
	lbl_value = c;
	break;
      }
#endif
    case CORBA::tk_enum:
      {
	// check that <label> is of the correct type
	if( !dtc->equivalent(lbl_tc) )
	  OMNIORB_THROW(BAD_PARAM,
			BAD_PARAM_IncompatibleDiscriminatorType,
			CORBA::COMPLETED_NO);
	break;
      }
    default:
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_IllegitimateDiscriminatorType,
		    CORBA::COMPLETED_NO);
    }
  }

  switch( aetc->kind() ) {
  case CORBA::tk_char:
    if (lbl_kind != CORBA::tk_char)
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_IncompatibleDiscriminatorType,
		    CORBA::COMPLETED_NO);
    break;
  case CORBA::tk_boolean:
    if (lbl_kind != CORBA::tk_boolean)
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_IncompatibleDiscriminatorType,
		    CORBA::COMPLETED_NO);
    break;
  case CORBA::tk_short:
    if ((sign && ((TypeCode_union::DiscriminatorSigned) lbl_value < -32768) )
	|| (lbl_value > 32767) )
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_IllegitimateDiscriminatorType,
		    CORBA::COMPLETED_NO);
    break;
  case CORBA::tk_ushort:
    if ((sign && ((TypeCode_union::DiscriminatorSigned) lbl_value < 0) ) ||
	(lbl_value > 65536) )
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_IllegitimateDiscriminatorType,
		    CORBA::COMPLETED_NO);
    break;
  case CORBA::tk_long:
    // XXX if TypeCode_union::Discriminator is bigger than
    //     CORBA::Long, we test for the negative limit as well.
   if (!sign && (lbl_value > 2147483647) )
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_IllegitimateDiscriminatorType,
		    CORBA::COMPLETED_NO);
#ifdef HAS_LongLong
   // The unlikely looking constant -1 is to work around a bug in MSVC
   // that incorrectly deals with large negative literals :-( .
   if (sign &&
       ((TypeCode_union::DiscriminatorSigned)lbl_value <
	                              _CORBA_LONGLONG_CONST(-2147483647) - 1))
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_IllegitimateDiscriminatorType,
		    CORBA::COMPLETED_NO);
#endif
    break;
  case CORBA::tk_ulong:
    // XXX if TypeCode_union::Discriminator is bigger than
    //     CORBA::ULong, we test for the positive limit as well.
    if (sign && ((TypeCode_union::DiscriminatorSigned) lbl_value < 0))
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_IllegitimateDiscriminatorType,
		    CORBA::COMPLETED_NO);
#ifdef HAS_LongLong
    if (lbl_value > 0xffffffff)
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_IllegitimateDiscriminatorType,
		    CORBA::COMPLETED_NO);
#endif
    break;

#ifdef HAS_LongLong
  case CORBA::tk_longlong:
    if (!sign && (lbl_value > _CORBA_LONGLONG_CONST(0x7fffffffffffffff)))
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_IllegitimateDiscriminatorType,
		    CORBA::COMPLETED_NO);
    break;
  case CORBA::tk_ulonglong:
    if (sign && ((TypeCode_union::DiscriminatorSigned)lbl_value < 0))
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_IllegitimateDiscriminatorType,
		    CORBA::COMPLETED_NO);
    break;
#endif

  case CORBA::tk_enum:
    {
      CORBA::ULong c;
      tcDescriptor enumdesc;
      enumdesc.p_enum.data = &c;
      enumdesc.p_enum.size = sizeof(c);
      label.PR_unpackTo(dtc, &enumdesc);
      lbl_value = c;
      break;
    }
  // case CORBA::tk_wchar:
  case CORBA::tk_octet:
  default:
    OMNIORB_THROW(BAD_TYPECODE,
		  BAD_PARAM_IllegitimateDiscriminatorType,
		  CORBA::COMPLETED_NO);
  }

  return lbl_value;
}


void
TypeCode_union_helper::insertLabel(CORBA::Any& label,
				   TypeCode_union::Discriminator c,
				   CORBA::TypeCode_ptr tc)
{
  const TypeCode_base* aetc = TypeCode_base::NP_expand(ToTcBase(tc));

  switch( aetc->NP_kind() ) {
  case CORBA::tk_char:
    label <<= CORBA::Any::from_char((CORBA::Char)c);
    break;
  case CORBA::tk_boolean:
    label <<= CORBA::Any::from_boolean((CORBA::Boolean)c);
    break;
  case CORBA::tk_octet:
    label <<= CORBA::Any::from_octet((CORBA::Octet)c);
    break;
  case CORBA::tk_short:
    label <<= CORBA::Short(c);
    break;
  case CORBA::tk_ushort:
    label <<= CORBA::UShort(c);
    break;
  case CORBA::tk_long:
    label <<= CORBA::Long(c);
    break;
  case CORBA::tk_ulong:
    label <<= CORBA::ULong(c);
    break;
#ifdef HAS_LongLong
  case CORBA::tk_longlong:
    label <<= CORBA::LongLong(c);
    break;
  case CORBA::tk_ulonglong:
    label <<= CORBA::ULongLong(c);
    break;
#endif

  case CORBA::tk_enum:
    {
      CORBA::ULong val = c;
      tcDescriptor enumdesc;
      enumdesc.p_enum.data = (void*)&val;
      enumdesc.p_enum.size = sizeof(val);
      label.PR_packFrom((TypeCode_base*) aetc, &enumdesc);
      break;
    }
  // case CORBA::tk_wchar:
  default:
    throw omniORB::fatalException(__FILE__,__LINE__,
       "TypeCode_union_helper::insertLabel() - illegal disciminator type");
  }
}


void
TypeCode_union_helper::marshalLabel(TypeCode_union::Discriminator l,
				    CORBA::TypeCode_ptr tc,
				    cdrStream &s)
{
  const TypeCode_base* aetc = TypeCode_base::NP_expand(ToTcBase(tc));

  switch( aetc->NP_kind() ) {
  case CORBA::tk_char:
    {
      CORBA::Char c = CORBA::Char(l);
      s.marshalChar(c);
      break;
    }
  case CORBA::tk_boolean:
    {
      CORBA::Boolean c = CORBA::Boolean(l);
      s.marshalBoolean(c);
      break;
    }
  case CORBA::tk_octet:
    {
      CORBA::Octet c = CORBA::Octet(l);
      s.marshalOctet(c);
      break;
    }
  case CORBA::tk_short:
    {
      CORBA::Short c = CORBA::Short(l);
      c >>= s;
      break;
    }
  case CORBA::tk_ushort:
    {
      CORBA::UShort c = CORBA::UShort(l);
      c >>= s;
      break;
    }
  case CORBA::tk_long:
    {
      CORBA::Long c = CORBA::Long(l);
      c >>= s;
      break;
    }
  case CORBA::tk_ulong:
  case CORBA::tk_enum:
    {
      CORBA::ULong c = CORBA::ULong(l);
      c >>= s;
      break;
    }
#ifdef HAS_LongLong
  case CORBA::tk_longlong:
    {
      CORBA::LongLong c = CORBA::LongLong(l);
      c >>= s;
      break;
    }
  case CORBA::tk_ulonglong:
    {
      CORBA::ULongLong c = CORBA::ULongLong(l);
      c >>= s;
      break;
    }
#endif

  // case CORBA::tk_wchar:
  default:
    throw omniORB::fatalException(__FILE__,__LINE__,
       "TypeCode_union_helper::marshalLabel() - illegal disciminator type");
  }
}


TypeCode_union::Discriminator
TypeCode_union_helper::unmarshalLabel(CORBA::TypeCode_ptr tc,
				      cdrStream& s)
{
  const TypeCode_base* aetc = TypeCode_base::NP_expand(ToTcBase(tc));

  switch( aetc->NP_kind() ) {
  case CORBA::tk_char:
    {
      CORBA::Char c = s.unmarshalChar();
      return TypeCode_union::Discriminator(c);
    }
  case CORBA::tk_boolean:
    {
      CORBA::Boolean c = s.unmarshalBoolean();
      return TypeCode_union::Discriminator(c);
    }
  case CORBA::tk_octet:
    {
      CORBA::Octet c = s.unmarshalOctet();
      return TypeCode_union::Discriminator(c);
    }
  case CORBA::tk_short:
    {
      CORBA::Short c;
      c <<= s;
      return TypeCode_union::Discriminator(c);
    }
  case CORBA::tk_ushort:
    {
      CORBA::UShort c;
      c <<= s;
      return TypeCode_union::Discriminator(c);
    }
  case CORBA::tk_long:
    {
      CORBA::Long c;
      c <<= s;
      return TypeCode_union::Discriminator(c);
    }
  case CORBA::tk_ulong:
  case CORBA::tk_enum:
    {
      CORBA::ULong c;
      c <<= s;
      return TypeCode_union::Discriminator(c);
    }
#ifdef HAS_LongLong
  case CORBA::tk_longlong:
    {
      CORBA::LongLong c;
      c <<= s;
      return TypeCode_union::Discriminator(c);
    }
  case CORBA::tk_ulonglong:
    {
      CORBA::ULongLong c;
      c <<= s;
      return TypeCode_union::Discriminator(c);
    }
#endif

  // case CORBA::tk_wchar:
  default:
    throw omniORB::fatalException(__FILE__,__LINE__,
       "TypeCode_union_helper::unmarshalLabel() - illegal disciminator type");
  }

#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


CORBA::Boolean
TypeCode_union_helper::has_implicit_default(TypeCode_base* tc)
{
  if( tc->NP_default_index() >= 0 )  return 0;

  TypeCode_base* dtc = tc->NP_discriminator_type();
  CORBA::TypeCode_var aedtc = TypeCode_base::aliasExpand(dtc);
  TypeCode_union::Discriminator npossible = 0;

  switch( ToTcBase(aedtc)->NP_kind() ) {

  case CORBA::tk_short:
  case CORBA::tk_ushort:
    npossible = 65536;
    break;
  case CORBA::tk_long:
  case CORBA::tk_ulong:
    npossible = 0xffffffff;
    break;
  case CORBA::tk_boolean:
    npossible = 2;
    break;
  case CORBA::tk_char:
  case CORBA::tk_octet:
    npossible = 256;
    break;
#ifdef HAS_LongLong
  case CORBA::tk_longlong:
  case CORBA::tk_ulonglong:
    // Not likely to have this many cases!
    npossible = _CORBA_LONGLONG_CONST(0xffffffffffffffff);
    break;
#endif
  case CORBA::tk_enum:
    npossible = ToTcBase(aedtc)->NP_member_count();
    break;
  default:
    OMNIORB_THROW(BAD_TYPECODE, BAD_TYPECODE_UnknownKind, CORBA::COMPLETED_NO);
  }

  return npossible > tc->NP_member_count();
}

OMNI_NAMESPACE_END(omni)

//////////////////////////////////////////////////////////////////////
/////////////////////// CORBA::TypeCode_member ///////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::TypeCode_member::TypeCode_member()
{
  _ptr = CORBA::TypeCode::_nil();
}

CORBA::TypeCode_member::TypeCode_member(const CORBA::TypeCode_member& p)
{
  _ptr = CORBA::TypeCode::_duplicate(p._ptr);
}

CORBA::TypeCode_member::~TypeCode_member()
{
  CORBA::release(_ptr);
}


CORBA::TypeCode_member&
CORBA::TypeCode_member::operator=(CORBA::TypeCode_ptr p)
{
  CORBA::release(_ptr);
  _ptr = p;
  return *this;
}


CORBA::TypeCode_member&
CORBA::TypeCode_member::operator=(const CORBA::TypeCode_member& p)
{
  if (this != &p) {
    CORBA::release(_ptr);
    _ptr = CORBA::TypeCode::_duplicate(p._ptr);
  }
  return *this;
}

CORBA::TypeCode_member&
CORBA::TypeCode_member::operator=(const CORBA::TypeCode_var& p)
{
  CORBA::release(_ptr);
  _ptr = CORBA::TypeCode::_duplicate(p.pd_ref);
  return *this;
}

void
CORBA::TypeCode_member::operator>>=(cdrStream& s) const
{
  CORBA::TypeCode::marshalTypeCode(_ptr, s);
}

void
CORBA::TypeCode_member::operator<<=(cdrStream& s)
{
  CORBA::TypeCode_ptr _result = CORBA::TypeCode::unmarshalTypeCode(s);
  CORBA::release(_ptr);
  _ptr = _result;
}

//////////////////////////////////////////////////////////////////////
///////////////////////////// CORBA::ORB /////////////////////////////
//////////////////////////////////////////////////////////////////////

static inline int validFirstChar(char c)
{
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static inline int validOtherChar(char c)
{
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
	  (c >= '0' && c <= '9') || (c == '_'));
}

static void
checkValidName(const char* name)
{
  int ok = 1;

  if (*name) {
    if (!validFirstChar(*name++)) ok = 0;

    for(; ok && *name; name++) {
      if (!validOtherChar(*name)) ok = 0;
    }
  }
  if (!ok)
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidName,CORBA::COMPLETED_NO);
}

static void
checkValidMemberName(const char* name)
{
  int ok = 1;

  if (*name) {
    if (!validFirstChar(*name++)) ok = 0;

    for(; ok && *name; name++) {
      if (!validOtherChar(*name)) ok = 0;
    }
  }
  if (!ok)
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidMemberName,CORBA::COMPLETED_NO);
}

static void
checkValidRepoId(const char* id)
{
  if (*id) {
    for (; *id && *id != ':'; id++);
    if (!*id)
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_InvalidRepositoryId,
		    CORBA::COMPLETED_NO);
  }
}

static void
checkValidTypeCode(const CORBA::TypeCode_ptr tc)
{
  CORBA::TCKind k = ToConstTcBase_Checked(tc)->NP_kind();
  if (k == CORBA::tk_null || k == CORBA::tk_void || k == CORBA::tk_except)
    OMNIORB_THROW(BAD_TYPECODE,
		  BAD_TYPECODE_IllegitimateMember,
		  CORBA::COMPLETED_NO);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_struct_tc(const char* id, const char* name,
			     const CORBA::StructMemberSeq& members)
{
  CORBA::ULong memberCount = members.length();
  CORBA::ULong i;

  checkValidName(name);
  checkValidRepoId(id);

  for( i = 0; i < memberCount; i++ ) {
    if (!CORBA::TypeCode::PR_is_valid(members[i].type))
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);

    checkValidMemberName(members[i].name);
    checkValidTypeCode(members[i].type);
  }

  TypeCode_struct::Member* new_members =
    new TypeCode_struct::Member[memberCount];

  for( i = 0; i < memberCount; i++ ) {
    // We duplicate the name and the type.
    new_members[i].name = CORBA::string_dup(members[i].name);
    new_members[i].type =
       TypeCode_collector::duplicateRef(ToTcBase(members[i].type));
  }

  return new TypeCode_struct(CORBA::string_dup(id), CORBA::string_dup(name),
			     new_members, memberCount);
}


CORBA::TypeCode_ptr
CORBA::ORB::create_union_tc(const char* id, const char* name,
			    CORBA::TypeCode_ptr discriminator_type,
			    const CORBA::UnionMemberSeq& members)
{
  const CORBA::ULong memberCount = members.length();
  CORBA::ULong i;

  checkValidName(name);
  checkValidRepoId(id);

  for( i = 0; i < memberCount; i++ ) {
    if( !CORBA::TypeCode::PR_is_valid(members[i].type) ||
	CORBA::is_nil(members[i].type) )
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);

    checkValidMemberName(members[i].name);
    checkValidTypeCode(members[i].type);
  }

  return new TypeCode_union(id, name, ToTcBase_Checked(discriminator_type),
			    members);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_enum_tc(const char* id, const char* name,
			   const CORBA::EnumMemberSeq& members)
{
  checkValidName(name);
  checkValidRepoId(id);

  for (CORBA::ULong i=0; i < members.length(); i++)
    checkValidMemberName(members[i]);

  return CORBA::TypeCode::NP_enum_tc(id, name, members);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_alias_tc(const char* id, const char* name,
			    CORBA::TypeCode_ptr original_type)
{
  checkValidName(name);
  checkValidRepoId(id);
  return CORBA::TypeCode::NP_alias_tc(id, name, original_type);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_exception_tc(const char* id, const char* name,
				const CORBA::StructMemberSeq& members)
{
  CORBA::ULong memberCount = members.length();
  CORBA::ULong i;

  checkValidName(name);
  checkValidRepoId(id);

  for( i = 0; i < memberCount; i++ ) {
    if (!CORBA::TypeCode::PR_is_valid(members[i].type))
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);

    checkValidMemberName(members[i].name);
    checkValidTypeCode(members[i].type);
  }

  TypeCode_struct::Member* new_members =
    new TypeCode_struct::Member[memberCount];

  for( i = 0; i < memberCount; i++ ) {
    // We duplicate the name and the type.
    new_members[i].name = CORBA::string_dup(members[i].name);
    new_members[i].type =
      TypeCode_collector::duplicateRef(ToTcBase(members[i].type));
  }

  return new TypeCode_except(CORBA::string_dup(id), CORBA::string_dup(name),
			     new_members, memberCount);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_interface_tc(const char* id, const char* name)
{
  checkValidName(name);
  checkValidRepoId(id);
  return CORBA::TypeCode::NP_interface_tc(id, name);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_string_tc(CORBA::ULong bound)
{
  return CORBA::TypeCode::NP_string_tc(bound);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_wstring_tc(CORBA::ULong bound)
{
  return CORBA::TypeCode::NP_wstring_tc(bound);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_fixed_tc(CORBA::UShort digits, CORBA::Short scale)
{
  return CORBA::TypeCode::NP_fixed_tc(digits, scale);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_sequence_tc(CORBA::ULong bound,
			       CORBA::TypeCode_ptr element_type)
{
  if (!CORBA::TypeCode::PR_is_valid(element_type))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);

  checkValidTypeCode(element_type);

  return CORBA::TypeCode::NP_sequence_tc(bound, element_type);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_array_tc(CORBA::ULong length,
			    CORBA::TypeCode_ptr element_type)
{
  if (!CORBA::TypeCode::PR_is_valid(element_type))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);

  checkValidTypeCode(element_type);

  return CORBA::TypeCode::NP_array_tc(length, element_type);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_recursive_sequence_tc(CORBA::ULong bound,
					 CORBA::ULong offset)
{
  return CORBA::TypeCode::NP_recursive_sequence_tc(bound, offset);
}

CORBA::TypeCode_ptr
CORBA::ORB::create_recursive_tc(const char* id)
{
  checkValidRepoId(id);
  return CORBA::TypeCode::NP_recursive_tc(id);
}


//////////////////////////////////////////////////////////////////////
///////////////////////// TypeCode constants /////////////////////////
//////////////////////////////////////////////////////////////////////

#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constants external linkage otherwise. Its a bug.
namespace CORBA {
TypeCode_ptr         _tc_null;
TypeCode_ptr         _tc_void;
TypeCode_ptr         _tc_short;
TypeCode_ptr         _tc_long;
TypeCode_ptr         _tc_ushort;
TypeCode_ptr         _tc_ulong;
TypeCode_ptr         _tc_float;
TypeCode_ptr         _tc_double;
TypeCode_ptr         _tc_boolean;
TypeCode_ptr         _tc_char;
TypeCode_ptr         _tc_wchar;
TypeCode_ptr         _tc_octet;
TypeCode_ptr         _tc_any;
TypeCode_ptr         _tc_TypeCode;
TypeCode_ptr         _tc_Principal;
TypeCode_ptr         _tc_Object;
TypeCode_ptr         _tc_string;
TypeCode_ptr         _tc_wstring;
TypeCode_ptr         _tc_NamedValue;
#ifdef HAS_LongLong
TypeCode_ptr         _tc_longlong;
TypeCode_ptr         _tc_ulonglong;
#endif
#ifdef HAS_LongDouble
TypeCode_ptr         _tc_longdouble;
#endif
}
#else
CORBA::TypeCode_ptr         CORBA::_tc_null;
CORBA::TypeCode_ptr         CORBA::_tc_void;
CORBA::TypeCode_ptr         CORBA::_tc_short;
CORBA::TypeCode_ptr         CORBA::_tc_long;
CORBA::TypeCode_ptr         CORBA::_tc_ushort;
CORBA::TypeCode_ptr         CORBA::_tc_ulong;
CORBA::TypeCode_ptr         CORBA::_tc_float;
CORBA::TypeCode_ptr         CORBA::_tc_double;
CORBA::TypeCode_ptr         CORBA::_tc_boolean;
CORBA::TypeCode_ptr         CORBA::_tc_char;
CORBA::TypeCode_ptr         CORBA::_tc_wchar;
CORBA::TypeCode_ptr         CORBA::_tc_octet;
CORBA::TypeCode_ptr         CORBA::_tc_any;
CORBA::TypeCode_ptr         CORBA::_tc_TypeCode;
CORBA::TypeCode_ptr         CORBA::_tc_Principal;
CORBA::TypeCode_ptr         CORBA::_tc_Object;
CORBA::TypeCode_ptr         CORBA::_tc_string;
CORBA::TypeCode_ptr         CORBA::_tc_wstring;
CORBA::TypeCode_ptr         CORBA::_tc_NamedValue;
#ifdef HAS_LongLong
CORBA::TypeCode_ptr         CORBA::_tc_longlong;
CORBA::TypeCode_ptr         CORBA::_tc_ulonglong;
#endif
#ifdef HAS_LongDouble
CORBA::TypeCode_ptr         CORBA::_tc_longdouble;
#endif

#endif

// Stub TypeCode tracker

CORBA::TypeCode::_Tracker::~_Tracker()
{
  TypeCode_base *current, *next;
  int count = 0;

  current = ToTcBase(pd_head);
  while (current) {
    next = current->pd_next;
    CORBA::release(current);
    current = next;
    count++;
  }
  if (count && omniORB::trace(20)) {
    const char *i, *f;

    for (i=pd_file, f=pd_file; *i; i++)
      if (*i == '/' || *i == '\\')
	f = i+1;

    omniORB::logger l;
    l << "Released " << count
      << " stub TypeCode" << (count == 1 ? "" : "s")
      << " from '" << f << "'.\n";
  }
  pd_head = 0;
}

void
CORBA::TypeCode::_Tracker::add(CORBA::TypeCode_ptr tc)
{
  ToTcBase(tc)->pd_next = ToTcBase(pd_head);
  pd_head = tc;
}


OMNI_NAMESPACE_BEGIN(omni)

// This is needed to ensure that access to statically initialised
// objects is safe during the static initialisation process. The
// stubs need to access the typecodes of pre-defined types, and
// also generate new typecodes. This is used to ensure that all
// necassary infrastructure is properly constructed before it is
// accessed by the stubs. This is necassary because the compiler
// does not specify the order of initialisation.
//
static void check_static_data_is_initialised()
{
  static int is_initialised = 0;

  if( is_initialised )  return;
  is_initialised = 1;

  the_tc_list = new omniTypeCodeList;
  registerTrackedObject(the_tc_list);

  // Mutexes
  aliasExpandedTc_lock = new omni_tracedmutex();
  pd_cached_paramlist_lock = new omni_tracedmutex();
  pd_refcount_lock = new omni_tracedmutex();

  // Primitive TypeCodes
  CORBA::_tc_null = new TypeCode_base(CORBA::tk_null);
  CORBA::_tc_void = new TypeCode_base(CORBA::tk_void);
  CORBA::_tc_short = new TypeCode_base(CORBA::tk_short);
  CORBA::_tc_long = new TypeCode_base(CORBA::tk_long);
  CORBA::_tc_ushort = new TypeCode_base(CORBA::tk_ushort);
  CORBA::_tc_ulong = new TypeCode_base(CORBA::tk_ulong);
  CORBA::_tc_float = new TypeCode_base(CORBA::tk_float);
  CORBA::_tc_double = new TypeCode_base(CORBA::tk_double);
  CORBA::_tc_boolean = new TypeCode_base(CORBA::tk_boolean);
  CORBA::_tc_char = new TypeCode_base(CORBA::tk_char);
  CORBA::_tc_wchar = new TypeCode_base(CORBA::tk_wchar);
  CORBA::_tc_octet = new TypeCode_base(CORBA::tk_octet);
  CORBA::_tc_any = new TypeCode_base(CORBA::tk_any);
  CORBA::_tc_TypeCode = new TypeCode_base(CORBA::tk_TypeCode);
  CORBA::_tc_Principal = new TypeCode_base(CORBA::tk_Principal);
  CORBA::_tc_Object = new TypeCode_objref("IDL:omg.org/CORBA/Object:1.0","Object");
  CORBA::_tc_string = new TypeCode_string(0);
  CORBA::_tc_wstring = new TypeCode_wstring(0);

  the_tc_list->add(CORBA::_tc_null);
  the_tc_list->add(CORBA::_tc_void);
  the_tc_list->add(CORBA::_tc_short);
  the_tc_list->add(CORBA::_tc_long);
  the_tc_list->add(CORBA::_tc_ushort);
  the_tc_list->add(CORBA::_tc_ulong);
  the_tc_list->add(CORBA::_tc_float);
  the_tc_list->add(CORBA::_tc_double);
  the_tc_list->add(CORBA::_tc_boolean);
  the_tc_list->add(CORBA::_tc_char);
  the_tc_list->add(CORBA::_tc_wchar);
  the_tc_list->add(CORBA::_tc_octet);
  the_tc_list->add(CORBA::_tc_any);
  the_tc_list->add(CORBA::_tc_TypeCode);
  the_tc_list->add(CORBA::_tc_Principal);
  the_tc_list->add(CORBA::_tc_Object);
  the_tc_list->add(CORBA::_tc_string);
  the_tc_list->add(CORBA::_tc_wstring);

#ifdef HAS_LongLong
  CORBA::_tc_longlong   = new TypeCode_base(CORBA::tk_longlong);
  CORBA::_tc_ulonglong  = new TypeCode_base(CORBA::tk_ulonglong);

  the_tc_list->add(CORBA::_tc_longlong);
  the_tc_list->add(CORBA::_tc_ulonglong);
#endif
#ifdef HAS_LongDouble
  CORBA::_tc_longdouble = new TypeCode_base(CORBA::tk_longdouble);
  the_tc_list->add(CORBA::_tc_longdouble);
#endif
  {
    CORBA::TypeCode_var tc_Flags = new TypeCode_alias("IDL:omg.org/CORBA/Flags:1.0", "Flags",ToTcBase(CORBA::_tc_ulong));
    CORBA::TypeCode_var tc_Identifier = new TypeCode_alias("IDL:omg.org/CORBA/Identifier:1.0", "Identifier", ToTcBase(CORBA::_tc_string));

    CORBA::PR_structMember nvMembers[4];

    nvMembers[0].name = "name";
    nvMembers[0].type = tc_Identifier;
    nvMembers[1].name = "argument";
    nvMembers[1].type = CORBA::_tc_any;
    nvMembers[2].name = "len";
    nvMembers[2].type = CORBA::_tc_long;
    nvMembers[3].name = "arg_modes";
    nvMembers[3].type = tc_Flags;

    CORBA::_tc_NamedValue = CORBA::TypeCode::PR_struct_tc("IDL:omg.org/CORBA/NamedValue:1.0", "NamedValue",nvMembers, 4, the_tc_list->pd_tracker);
  }
}

//
// Clean-up
//
omniTypeCodeList::~omniTypeCodeList()
{
  CORBA::TypeCode_ptr t = CORBA::_tc_short;

  delete pd_tracker;

  // Delete mutexes
  delete aliasExpandedTc_lock;
  delete pd_cached_paramlist_lock;
  delete pd_refcount_lock;
}


// We need a singleton here as a final check, so that if no
// stub code calls into check_static_data_is_initialised()
// it will still be called before main().

class TypeCodeInitialiser {
  // public just to stop brain-dead compilers complaining
public:
  TypeCodeInitialiser() { check_static_data_is_initialised(); }
  static TypeCodeInitialiser typecode_initialiser;
};
TypeCodeInitialiser TypeCodeInitialiser::typecode_initialiser;

OMNI_NAMESPACE_END(omni)
