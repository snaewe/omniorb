// -*- Mode: C++; -*-
//                            Package   : omniORB
// any.cc                     Created on: 31/07/97
//                            Author1   : Eoin Carroll (ewc)
//                            Author2   : James Weatherall (jnw)
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
//      Implementation of type any

/*
 * $Log$
 * Revision 1.21.2.1  2003/03/23 21:02:51  dgrisby
 * Start of omniORB 4.1.x development branch.
 *
 * Revision 1.19.2.12  2001/10/17 16:44:02  dpg1
 * Update DynAny to CORBA 2.5 spec, const Any exception extraction.
 *
 * Revision 1.19.2.11  2001/09/24 10:41:08  dpg1
 * Minor codes for Dynamic library and omniORBpy.
 *
 * Revision 1.19.2.10  2001/08/22 13:29:45  dpg1
 * Re-entrant Any marshalling.
 *
 * Revision 1.19.2.9  2001/08/17 17:08:05  sll
 * Modularise ORB configuration parameters.
 *
 * Revision 1.19.2.8  2001/08/17 13:45:55  dpg1
 * C++ mapping fixes.
 *
 * Revision 1.19.2.7  2001/04/19 09:14:16  sll
 * Scoped where appropriate with the omni namespace.
 *
 * Revision 1.19.2.6  2001/03/13 10:32:05  dpg1
 * Fixed point support.
 *
 * Revision 1.19.2.5  2000/11/17 19:09:36  dpg1
 * Support codeset conversion in any.
 *
 * Revision 1.19.2.4  2000/11/09 12:27:52  dpg1
 * Huge merge from omni3_develop, plus full long long from omni3_1_develop.
 *
 * Revision 1.19.2.3  2000/10/06 16:40:52  sll
 * Changed to use cdrStream.
 *
 * Revision 1.19.2.2  2000/09/27 17:25:39  sll
 * Changed include/omniORB3 to include/omniORB4.
 *
 * Revision 1.19.2.1  2000/07/17 10:35:40  sll
 * Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
 *
 * Revision 1.20  2000/07/13 15:26:03  dpg1
 * Merge from omni3_develop for 3.0 release.
 *
 * Revision 1.18.6.6  2000/06/27 16:15:09  sll
 * New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
 * _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
 * sequence of string and a sequence of object reference.
 *
 * Revision 1.18.6.5  2000/02/09 12:04:52  djr
 * Fixed memory allocation bug in Any insertion/extraction of strings.
 * Optimisation for insertion/extraction of sequence of simple types.
 *
 * Revision 1.18.6.4  1999/10/14 17:31:30  djr
 * Minor corrections.
 *
 * Revision 1.18.6.3  1999/10/14 16:21:54  djr
 * Implemented logging when system exceptions are thrown.
 *
 * Revision 1.18.6.2  1999/09/27 08:48:31  djr
 * Minor corrections to get rid of warnings.
 *
 * Revision 1.18.6.1  1999/09/22 14:26:27  djr
 * Major rewrite of orbcore to support POA.
 *
 * Revision 1.18  1999/07/02 19:35:16  sll
 * Corrected typo in operator>>= for typecode.
 *
 * Revision 1.16  1999/07/02 19:10:46  sll
 * Typecode extraction is now non-copy as well.
 *
 * Revision 1.15  1999/06/28 17:38:19  sll
 * Fixed bug in Any marshalling when tcAliasExpand is set to 1.
 *
 * Revision 1.14  1999/06/25 13:47:19  sll
 * Rename copyStringInAnyExtraction to omniORB_27_CompatibleAnyExtraction.
 * operator<<=(Object_ptr) now marshal the real repository ID of the object.
 * operator<<=(const char*) changed to use new format of string data in tcDescriptor.
 * operator<<=(from_string)
 * Removed operator>>=(Object_ptr&) const
 * operator>>=(char*&)  Default to non-copy semantics. Override by
 * omniORB_27_CompatibleAnyExtraction.
 * operator>>=(const char*&) const
 * operator>>=(tostring) const Default to non-copy semantics. Override by
 * omniORB_27_CompatibleAnyExtraction.
 * operator>>=(to_object) const Use _0RL_tcParser_objref_setObjectPtr in
 * the redundent setObjectPtr.
 *
 * Revision 1.13  1999/06/18 20:59:48  sll
 * Updated to CORBA 2.3 mapping.
 * Semantics of extraction operator for string has changed.
 *
 * Revision 1.12  1999/05/25 18:07:51  sll
 * In value(), return 0 if typecode is _tc_null.
 *
 * Revision 1.11  1999/03/11 16:25:58  djr
 * Updated copyright notice
 *
 * Revision 1.10  1999/03/01 09:12:18  djr
 * Accept insertion of null strings into Any (with warning message)
 *
 * Revision 1.9  1999/02/18 15:45:39  djr
 * (Re)Fixed broken insertion/extraction of Object_ptr from Any.
 *
 * Revision 1.8  1999/02/10 15:14:50  djr
 * Fixed broken implementation of marshalling object references into Anys.
 *
 * Revision 1.7  1999/01/07 16:47:03  djr
 * New implementation
 *
 * Revision 1.6  1998/08/14 13:43:04  sll
 * Added pragma hdrstop to control pre-compile header if the compiler feature
 * is available.
 *
 * Revision 1.5  1998/08/10 18:08:26  sll
 * Fixed Any ctor and Any::replace() for untyped values. Now accept null
 * pointer for the value parameter for all typecode types.
 *
 * Revision 1.4  1998/08/05 18:03:36  sll
 * Fixed bug in Any::operator>>=(NetBufferedStream|MemBufferedStream).
 * Previously, basic data types other than any, objref and typecode would be
 * marshalled incorrectly.
 *
 * Revision 1.3  1998/04/08 16:07:32  sll
 * Minor change to help some compiler to find the right TypeCode ctor.
 *
 * Revision 1.2  1998/04/07 19:30:45  sll
 * Moved inline functions to this module.
 *
// Revision 1.1  1998/01/27  15:43:47  ewc
// Initial revision
//
// Revision 1.1  1998/01/27  15:43:47  ewc
// Initial revision
//
*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <anyP.h>
#include <typecode.h>
#include <orbParameters.h>
#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(dynamicLib);

OMNI_USING_NAMESPACE(omni)

#define pdAnyP() ((AnyP*) (NP_pd()))
#define pdAnyP2(a) ((AnyP*) ((a)->NP_pd()))

// CONSTRUCTORS / DESTRUCTOR
CORBA::Any::Any()
{
  pd_ptr = new AnyP(CORBA::_tc_null);
}


CORBA::Any::~Any()
{
  delete pdAnyP();
}


CORBA::Any::Any(const Any& a) 
{
  // Replace the internal AnyP with a new one, based on the Any passed to us
  pd_ptr = new AnyP(pdAnyP2(&a));
}


CORBA::
Any::Any(TypeCode_ptr tc, void* value, Boolean release)
{
  if (value == 0)
    {
      // No value, so just create an empty, writable buffer for the
      // specified type.
      pd_ptr = new AnyP(tc);
    }
  else
    {
      // Value specified, so create a read-only parser based on it
      pd_ptr = new AnyP(tc, value, release);
    }
}

// Marshalling operators
void
CORBA::Any::operator>>= (cdrStream& s) const
{
  if( orbParameters::tcAliasExpand ) {
    CORBA::TypeCode_var tc =
      TypeCode_base::aliasExpand(ToTcBase(pdAnyP()->getTC()));
    CORBA::TypeCode::marshalTypeCode(tc, s);
  }
  else
    CORBA::TypeCode::marshalTypeCode(pdAnyP()->getTC(), s);

  pdAnyP()->copyTo(s);
}

void
CORBA::Any::operator<<= (cdrStream& s)
{
  CORBA::TypeCode_member newtc;
  newtc <<= s;
  pdAnyP()->setTC_and_reset(newtc);
  pdAnyP()->copyFrom(s);
}

// omniORB data-only marshalling functions
void
CORBA::Any::NP_marshalDataOnly(cdrStream& s) const
{
  pdAnyP()->copyTo(s);
}

void
CORBA::Any::NP_unmarshalDataOnly(cdrStream& s)
{
  pdAnyP()->copyFrom(s);
}

// omniORB internal data packing functions, for use only by stub code
void
CORBA::Any::PR_packFrom(CORBA::TypeCode_ptr newtc,
			void* tcdesc)
{
  // Pack the tcDescriptor data into this Any
  pdAnyP()->setData(newtc, *((tcDescriptor*)tcdesc));
}

CORBA::Boolean
CORBA::Any::PR_unpackTo(CORBA::TypeCode_ptr tc,
			void* tcdesc) const
{
  // Unpack the Any data out to the descriptor
  return pdAnyP()->getData(tc, *((tcDescriptor*)tcdesc));
}

void*
CORBA::Any::PR_getCachedData() const
{
  // Complex types, such as structures, have to have their storage
  // handled by the Any, so the storage pointer is cached internally
  // and this routine is used to retrieve the cached pointer
  return pdAnyP()->getCachedData();
}

void
CORBA::Any::PR_setCachedData(void* data, void (*destructor)(void*))
{
  // Complex types, such as structures, have to have their storage
  // handled by the Any, so the storage pointer is cached internally
  // and this routine is used to set the cached pointer & its destructor
  pdAnyP()->setCachedData(data, destructor);
}

//////////////////////////////////////////////////////////////////////
///////////////////////// INSERTION OPERATORS ////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Any&
CORBA::Any::operator=(const CORBA::Any& a)
{
  if (&a != this) {
    // Delete the old internal AnyP and create a new one,
    // based on the Any passed to us
    delete pdAnyP();
    pd_ptr = new AnyP(pdAnyP2(&a));
  }
  return *this;
}


void
CORBA::Any::operator<<=(Short s)
{
  tcDescriptor tcd;
  tcd.p_short = &s;
  pdAnyP()->setData(CORBA::_tc_short, tcd);
}


void CORBA::Any::operator<<=(UShort u)
{
  tcDescriptor tcd;
  tcd.p_ushort = &u;
  pdAnyP()->setData(CORBA::_tc_ushort, tcd);
}


void
CORBA::Any::operator<<=(Long l)
{
  tcDescriptor tcd;
  tcd.p_long = &l;
  pdAnyP()->setData(CORBA::_tc_long, tcd);
}


void
CORBA::Any::operator<<=(ULong u)
{
  tcDescriptor tcd;
  tcd.p_ulong = &u;
  pdAnyP()->setData(CORBA::_tc_ulong, tcd);
}

#ifdef HAS_LongLong
void
CORBA::Any::operator<<=(LongLong l)
{
  tcDescriptor tcd;
  tcd.p_longlong = &l;
  pdAnyP()->setData(CORBA::_tc_longlong, tcd);
}

void
CORBA::Any::operator<<=(ULongLong u)
{
  tcDescriptor tcd;
  tcd.p_ulonglong = &u;
  pdAnyP()->setData(CORBA::_tc_ulonglong, tcd);
}
#endif


#if !defined(NO_FLOAT)
void
CORBA::Any::operator<<=(Float f)
{
  tcDescriptor tcd;
  tcd.p_float = &f;
  pdAnyP()->setData(CORBA::_tc_float, tcd);
}

void
CORBA::Any::operator<<=(Double d)
{
  tcDescriptor tcd;
  tcd.p_double = &d;
  pdAnyP()->setData(CORBA::_tc_double, tcd);
}

#ifdef HAS_LongDouble
void
CORBA::Any::operator<<=(LongDouble d)
{
  tcDescriptor tcd;
  tcd.p_longdouble = &d;
  pdAnyP()->setData(CORBA::_tc_longdouble, tcd);
}
#endif

#endif

void
CORBA::Any::operator<<=(const Any& a)
{
  tcDescriptor tcd;
  // *** Should we really subvert the 'const' stuff here?
  // *** Should be safe - marshalling Any is now re-entrant
  tcd.p_any = (CORBA::Any*)&a;
  pdAnyP()->setData(CORBA::_tc_any, tcd);
}

void
CORBA::Any::operator<<=(Any* a)
{
  tcDescriptor tcd;
  tcd.p_any = a;
  pdAnyP()->setData(CORBA::_tc_any, tcd);
  delete a;
}

void
CORBA::Any::operator<<=(TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc)) {
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidTypeCode,CORBA::COMPLETED_NO);
  }
  CORBA::TypeCode_member tcm(tc);
  tcDescriptor tcd;
  tcd.p_TypeCode = &tcm;
  pdAnyP()->setData(CORBA::_tc_TypeCode, tcd);
  tcm._ptr = CORBA::TypeCode::_nil();
}

void
CORBA::Any::operator<<=(Object_ptr obj)
{
  if (!CORBA::Object::_PR_is_valid(obj)) {
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidObjectRef,CORBA::COMPLETED_NO);
  }
  const char* repoid = CORBA::Object::_PD_repoId;
  const char* name   = "";
  if (!CORBA::is_nil(obj))
    repoid = obj->_PR_getobj()->_mostDerivedRepoId();
  CORBA::TypeCode_var tc = CORBA::TypeCode::NP_interface_tc(repoid,name);
  tcDescriptor tcd;
  tcd.p_objref.opq_objref = (void*) &obj;
  tcd.p_objref.opq_release = 0;
  tcd.p_objref.getObjectPtr = _0RL_tcParser_objref_getObjectPtr;
  pdAnyP()->setData(tc, tcd);
}


void
CORBA::Any::operator<<=(from_boolean f)
{
  tcDescriptor tcd;
  tcd.p_boolean = &(f.val);
  pdAnyP()->setData(CORBA::_tc_boolean, tcd);
}


void
CORBA::Any::operator<<=(from_char c)
{
  tcDescriptor tcd;
  tcd.p_char = &(c.val);
  pdAnyP()->setData(CORBA::_tc_char, tcd);
}


void
CORBA::Any::operator<<=(from_wchar c)
{
  tcDescriptor tcd;
  tcd.p_wchar = &(c.val);
  pdAnyP()->setData(CORBA::_tc_wchar, tcd);
}


void 
CORBA::Any::operator<<=(from_octet o)
{
  tcDescriptor tcd;
  tcd.p_octet = &(o.val);
  pdAnyP()->setData(CORBA::_tc_octet, tcd);
}


void
CORBA::Any::operator<<=(const char* s)
{
  tcDescriptor tcd;
  tcd.p_string.ptr = (char**) &s;
  // tcd.p_string.release not needed for insertion
  pdAnyP()->setData(CORBA::_tc_string, tcd);
}


void 
CORBA::Any::operator<<=(from_string s)
{
  tcDescriptor tcd;
  tcd.p_string.ptr = &s.val;
  // tcd.p_string.release not needed for insertion

  if( s.bound ) {
    CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_string_tc(s.bound);
    pdAnyP()->setData(newtc, tcd);
  }else
    pdAnyP()->setData(CORBA::_tc_string, tcd);

  if( s.nc )  CORBA::string_free(s.val);
}


void
CORBA::Any::operator<<=(const CORBA::WChar* s)
{
  tcDescriptor tcd;
  tcd.p_wstring.ptr = (CORBA::WChar**) &s;
  // tcd.p_wstring.release not needed for insertion
  pdAnyP()->setData(CORBA::_tc_wstring, tcd);
}  


void 
CORBA::Any::operator<<=(from_wstring s)
{
  tcDescriptor tcd;
  tcd.p_wstring.ptr = &s.val;
  // tcd.p_wstring.release not needed for insertion

  if( s.bound ) {
    CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_wstring_tc(s.bound);
    pdAnyP()->setData(newtc, tcd);
  }else
    pdAnyP()->setData(CORBA::_tc_wstring, tcd);

  if( s.nc )  CORBA::wstring_free(s.val);
}

void
CORBA::Any::operator<<=(from_fixed f)
{
  tcDescriptor tcd;
  tcd.p_fixed = (CORBA::Fixed*)&f.val;
  // The cast to (CORBA::Fixed*) hides the fact that f.val is
  // const. setData() doesn't modify the value, so it's OK.

  CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_fixed_tc(f.digits,f.scale);
  pdAnyP()->setData(newtc, tcd);
}

// EXTRACTION OPERATORS

CORBA::Boolean 
CORBA::Any::operator>>=(Short& s) const
{
  tcDescriptor tcd;
  tcd.p_short = &s;
  return pdAnyP()->getData(CORBA::_tc_short, tcd);
}

    
CORBA::Boolean
CORBA::Any::operator>>=(UShort& u) const
{
  tcDescriptor tcd;
  tcd.p_ushort = &u;
  return pdAnyP()->getData(CORBA::_tc_ushort, tcd);
}


CORBA::Boolean
CORBA::Any::operator>>=(Long& l) const
{
  tcDescriptor tcd;
  tcd.p_long = &l;
  return pdAnyP()->getData(CORBA::_tc_long, tcd);
}

  
CORBA::Boolean
CORBA::Any::operator>>=(ULong& u) const
{
  tcDescriptor tcd;
  tcd.p_ulong = &u;
  return pdAnyP()->getData(CORBA::_tc_ulong, tcd);
}


#ifdef HAS_LongLong
CORBA::Boolean
CORBA::Any::operator>>=(LongLong& l) const
{
  tcDescriptor tcd;
  tcd.p_longlong = &l;
  return pdAnyP()->getData(CORBA::_tc_longlong, tcd);
}

  
CORBA::Boolean
CORBA::Any::operator>>=(ULongLong& u) const
{
  tcDescriptor tcd;
  tcd.p_ulonglong = &u;
  return pdAnyP()->getData(CORBA::_tc_ulonglong, tcd);
}
#endif


#if !defined(NO_FLOAT)
CORBA::Boolean
CORBA::Any::operator>>=(Float& f) const
{
  tcDescriptor tcd;
  tcd.p_float = &f;
  return pdAnyP()->getData(CORBA::_tc_float, tcd);
}


CORBA::Boolean
CORBA::Any::operator>>=(Double& d) const
{
  tcDescriptor tcd;
  tcd.p_double = &d;
  return pdAnyP()->getData(CORBA::_tc_double, tcd);
}

#ifdef HAS_LongDouble
CORBA::Boolean
CORBA::Any::operator>>=(LongDouble& d) const
{
  tcDescriptor tcd;
  tcd.p_longdouble = &d;
  return pdAnyP()->getData(CORBA::_tc_longdouble, tcd);
}
#endif

#endif

CORBA::Boolean CORBA::Any::operator>>=(CORBA::Any*& a) const
{
  return this->operator>>=((const CORBA::Any*&) a);
}

static 
void delete_any(void* data) {
  CORBA::Any* ap = (CORBA::Any*) data;
  delete ap;
}

CORBA::Boolean CORBA::Any::operator>>=(const CORBA::Any*& a) const
{
  CORBA::Any* ap = (CORBA::Any*) PR_getCachedData();
  if (ap == 0) {
    tcDescriptor tcd;
    ap = new CORBA::Any();
    tcd.p_any = ap;
    if (pdAnyP()->getData(CORBA::_tc_any, tcd)) {
      ((CORBA::Any*) this)->PR_setCachedData(ap,delete_any);
      a = ap; return 1;
    }
    else {
      delete ap; 
      a = 0; return 0;
    }
  }
  else {
    CORBA::TypeCode_var tc = type();
    if (tc->equivalent(CORBA::_tc_any)) {
      a = ap; return 1;
    }
    else {
      a = 0; return 0;
    }
  }
}

// pre- CORBA 2.3 operator. Obsoleted.
CORBA::Boolean CORBA::Any::operator>>=(Any& a) const
{
  tcDescriptor tcd;
  tcd.p_any = &a;
  return pdAnyP()->getData(CORBA::_tc_any, tcd);
}



static
void delete_typecode(void* data) {
  CORBA::TypeCode_ptr sp = (CORBA::TypeCode_ptr) data;
  CORBA::release(sp);
}

CORBA::Boolean
CORBA::Any::operator>>=(CORBA::TypeCode_ptr& tc) const
{
  CORBA::TypeCode_ptr sp = (CORBA::TypeCode_ptr) PR_getCachedData();
  if (sp == 0) {
    CORBA::TypeCode_member tcm;
    tcDescriptor tcd;
    tcd.p_TypeCode = &tcm;
    CORBA::Boolean ret = pdAnyP()->getData(CORBA::_tc_TypeCode, tcd);
    if( ret ) {
      if (!omniORB::omniORB_27_CompatibleAnyExtraction) {
        ((CORBA::Any*)this)->PR_setCachedData((void*)tcm._ptr,delete_typecode);
      }
      tc = tcm._ptr;
      tcm._ptr = CORBA::TypeCode::_nil(); return 1;
    } else {
      tc = CORBA::TypeCode::_nil(); return 0;
    }
  }
  else {
    CORBA::TypeCode_var t = type();
    if (t->equivalent(CORBA::_tc_TypeCode)) {
      tc = sp; return 1;
    }
    else {
      tc = CORBA::TypeCode::_nil(); return 0;
    }
  }
}

CORBA::Boolean
CORBA::Any::operator>>=(to_boolean b) const
{
  tcDescriptor tcd;
  tcd.p_boolean = &b.ref;
  return pdAnyP()->getData(CORBA::_tc_boolean, tcd);
}


CORBA::Boolean
CORBA::Any::operator>>=(to_char c) const
{
  tcDescriptor tcd;
  tcd.p_char = &c.ref;
  return pdAnyP()->getData(CORBA::_tc_char, tcd);
}

    
CORBA::Boolean
CORBA::Any::operator>>=(to_wchar c) const
{
  tcDescriptor tcd;
  tcd.p_wchar = &c.ref;
  return pdAnyP()->getData(CORBA::_tc_wchar, tcd);
}

    
CORBA::Boolean
CORBA::Any::operator>>=(to_octet o) const
{
  tcDescriptor tcd;
  tcd.p_octet = &o.ref;
  return pdAnyP()->getData(CORBA::_tc_octet, tcd);
}

CORBA::Boolean
CORBA::Any::operator>>=(char*& s) const
{
  if (!omniORB::omniORB_27_CompatibleAnyExtraction) {
    return this->operator>>=((const char*&) s);
  }
  else {
    char* p = 0;
    tcDescriptor tcd;
    tcd.p_string.ptr = &p;
    tcd.p_string.release = 0;
    
    if (pdAnyP()->getData(CORBA::_tc_string, tcd))
    {
      s = p;
      return 1;
    }
  }

  return 0;
}

static
void delete_string(void* data) {
  char* sp = (char*) data;
  CORBA::string_free(sp);
}

CORBA::Boolean
CORBA::Any::operator>>=(const char*& s) const
{
  char* sp = (char*) PR_getCachedData();
  if (sp == 0) {
    tcDescriptor tcd;
    tcd.p_string.ptr = &sp;
    tcd.p_string.release = 0;

    if (pdAnyP()->getData(CORBA::_tc_string, tcd))
    {
      ((CORBA::Any*)this)->PR_setCachedData(sp,delete_string);
      s = sp; return 1;
    }
    else {
      s = 0; return 0;
    }
  }
  else {
    CORBA::TypeCode_var tc = type();
    if (tc->equivalent(CORBA::_tc_string)) {
      s = sp; return 1;
    }
    else {
      s = 0; return 0;
    }
  }
}

CORBA::Boolean
CORBA::Any::operator>>=(to_string s) const
{
  CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_string_tc(s.bound);

  char* sp = (char*) PR_getCachedData();
  if (sp == 0) {
    tcDescriptor tcd;
    tcd.p_string.ptr = &sp;
    tcd.p_string.release = 0;

    if (pdAnyP()->getData(newtc, tcd))
    {
      if (!omniORB::omniORB_27_CompatibleAnyExtraction) {
	((CORBA::Any*)this)->PR_setCachedData(sp,delete_string);
      }
      s.val = sp; return 1;
    }
    else {
      s.val = 0; return 0;
    }
  }
  else {
    CORBA::TypeCode_var tc = type();
    if (tc->equivalent(newtc)) {
      s.val = sp; return 1;
    }
    else {
      s.val = 0; return 0;
    }
  }
}


CORBA::Boolean
CORBA::Any::operator>>=(const CORBA::SystemException*& e) const
{
  CORBA::TypeCode_var tc = type();
  CORBA::Boolean r;
#define EXTRACT_IF_MATCH(name) \
  if (tc->equivalent(CORBA::_tc_##name)) { \
    const CORBA::name* ex; \
    r = *this >>= ex; \
    e = ex; \
    return r; \
  }
  OMNIORB_FOR_EACH_SYS_EXCEPTION(EXTRACT_IF_MATCH)
#undef EXTRACT_IF_MATCH

  return 0;
}


static
void delete_wstring(void* data) {
  CORBA::WChar* sp = (CORBA::WChar*) data;
  CORBA::wstring_free(sp);
}

CORBA::Boolean
CORBA::Any::operator>>=(const CORBA::WChar*& s) const
{
  CORBA::WChar* sp = (CORBA::WChar*) PR_getCachedData();
  if (sp == 0) {
    tcDescriptor tcd;
    tcd.p_wstring.ptr = &sp;
    tcd.p_wstring.release = 0;

    if (pdAnyP()->getData(CORBA::_tc_wstring, tcd))
    {
      ((CORBA::Any*)this)->PR_setCachedData(sp,delete_wstring);
      s = sp; return 1;
    }
    else {
      s = 0; return 0;
    }
  }
  else {
    CORBA::TypeCode_var tc = type();
    if (tc->equivalent(CORBA::_tc_wstring)) {
      s = sp; return 1;
    }
    else {
      s = 0; return 0;
    }
  }
}

CORBA::Boolean
CORBA::Any::operator>>=(to_wstring s) const
{
  CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_wstring_tc(s.bound);

  CORBA::WChar* sp = (CORBA::WChar*) PR_getCachedData();
  if (sp == 0) {
    tcDescriptor tcd;
    tcd.p_wstring.ptr = &sp;
    tcd.p_wstring.release = 0;

    if (pdAnyP()->getData(newtc, tcd))
    {
      if (!omniORB::omniORB_27_CompatibleAnyExtraction) {
	((CORBA::Any*)this)->PR_setCachedData(sp,delete_wstring);
      }
      s.val = sp; return 1;
    }
    else {
      s.val = 0; return 0;
    }
  }
  else {
    CORBA::TypeCode_var tc = type();
    if (tc->equivalent(newtc)) {
      s.val = sp; return 1;
    }
    else {
      s.val = 0; return 0;
    }
  }
}

CORBA::Boolean
CORBA::Any::operator>>=(to_fixed f) const
{
  CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_fixed_tc(f.digits, f.scale);
  tcDescriptor tcd;
  tcd.p_fixed = &f.val;
  return pdAnyP()->getData(newtc, tcd);
}


CORBA::Boolean
CORBA::Any::operator>>=(to_object o) const
{
  tcDescriptor tcd;
  tcd.p_objref.opq_objref = (void*) &o.ref;
  tcd.p_objref.opq_release = 0;
  tcd.p_objref.setObjectPtr = _0RL_tcParser_objref_setObjectPtr;
  return pdAnyP()->getObjRef(tcd);
}

static
void delete_object(void* data) {
  CORBA::release((CORBA::Object_ptr)data);
}


CORBA::Boolean
CORBA::Any::operator>>=(CORBA::Object_ptr& obj) const
{
  CORBA::Object_ptr sp = (CORBA::Object_ptr) PR_getCachedData();
  if (sp == 0) {
    tcDescriptor tcd;
    CORBA::Object_var tmp;
    _0RL_buildDesc_cCORBA_mObject(tcd, tmp);
    if( PR_unpackTo(CORBA::_tc_Object, &tcd) ) {
      if (!omniORB::omniORB_27_CompatibleAnyExtraction) {
        ((CORBA::Any*)this)->PR_setCachedData((void*)(CORBA::Object_ptr)tmp,
					      delete_object);
      }
      obj = tmp._retn();
      return 1;
    } else {
      obj = CORBA::Object::_nil(); return 0;
    }
  }
  else {
    CORBA::TypeCode_var tc = type();
    if (tc->equivalent(CORBA::_tc_Object)) {
      obj = sp; return 1;
    }
    else {
      obj = CORBA::Object::_nil(); return 0;
    }
  }
}



void
CORBA::Any::replace(TypeCode_ptr TCp, void* value, Boolean release)
{
  // Get rid of the old implementation object
  delete pdAnyP();

  if (value == 0)
    {
      // Create a writable, empty implementation object for the desired type
      pd_ptr = new AnyP(TCp);
    }
  else
    {
      // Create a read-only implementation for the supplied buffer
      pd_ptr = new AnyP(TCp, value, release);
    }
}


CORBA::TypeCode_ptr
CORBA::Any::type() const
{
  return CORBA::TypeCode::_duplicate(pdAnyP()->getTC());
}

void
CORBA::Any::type(CORBA::TypeCode_ptr tc)
{
  pdAnyP()->replaceTC(tc);
}

const void*
CORBA::Any::value() const
{
  if (pdAnyP()->getTC() == CORBA::_tc_null)
    return 0;
  else
    return pdAnyP()->getBuffer();
}
