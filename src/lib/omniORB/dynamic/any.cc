// -*- Mode: C++; -*-
//                            Package   : omniORB
// any.cc                     Created on: 31/07/97
//                            Author1   : Eoin Carroll (ewc)
//                            Author2   : James Weatherall (jnw)
//                            Author3   : Duncan Grisby (dgrisby)
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
//      Implementation of type any

/*
 * $Log$
 * Revision 1.21.2.2  2004/07/23 10:29:58  dgrisby
 * Completely new, much simpler Any implementation.
 *
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

#include <typecode.h>
#include <tcParser.h>
#include <orbParameters.h>
#include <omniORB4/linkHacks.h>
#include <omniORB4/anyStream.h>

OMNI_FORCE_LINK(dynamicLib);

OMNI_USING_NAMESPACE(omni)

// Mutex to protect Any pointers against modification by multiple threads.
static omni_tracedmutex anyLock;


//////////////////////////////////////////////////////////////////////
////////////////// Constructors / destructor /////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Any::Any()
  : pd_mbuf(0), pd_data(0), pd_marshal(0), pd_destructor(0)
{
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_null);
}


CORBA::Any::~Any()
{
  if (pd_mbuf)
    delete pd_mbuf;

  if (pd_data) {
    OMNIORB_ASSERT(pd_destructor);
    pd_destructor(pd_data);
  }
}

void
CORBA::Any::PR_clearData()
{
  if (pd_mbuf)
    delete pd_mbuf;

  if (pd_data) {
    OMNIORB_ASSERT(pd_destructor);
    pd_destructor(pd_data);
  }
  pd_mbuf = 0;
  pd_data = 0;
  pd_marshal = 0;
  pd_destructor = 0;
}


CORBA::Any::Any(const Any& a) 
  : pd_data(0), pd_marshal(0), pd_destructor(0)
{
  pd_tc = CORBA::TypeCode::_duplicate(a.pd_tc);

  if (a.pd_mbuf) {
    pd_mbuf = new cdrAnyMemoryStream(*a.pd_mbuf);
  }
  else if (a.pd_data) {
    // Existing Any has data in its void* pointer. Rather than trying
    // to copy that (which would require a copy function to be
    // registered along with the marshal and destructor functions), we
    // marshal the data into a memory buffer.
    OMNIORB_ASSERT(a.pd_marshal);

    pd_mbuf = new cdrAnyMemoryStream;
    a.pd_marshal(*pd_mbuf, a.pd_data);
  }
  else {
    // The Any has just a TypeCode and no data yet.
    pd_mbuf = 0;
  }
}


CORBA::Any&
CORBA::Any::operator=(const CORBA::Any& a)
{
  if (&a != this) {
    PR_clearData();
    pd_tc = CORBA::TypeCode::_duplicate(a.pd_tc);

    if (a.pd_mbuf) {
      pd_mbuf = new cdrAnyMemoryStream(*a.pd_mbuf);
    }
    else if (a.pd_data) {
      OMNIORB_ASSERT(a.pd_marshal);
      pd_mbuf = new cdrAnyMemoryStream;
      a.pd_marshal(*pd_mbuf, a.pd_data);
    }
    else {
      pd_mbuf = 0;
    }
  }
  return *this;
}


//
// Nasty deprecated constructor and replace() taking a void* buffer
//

static void voidDestructor_fn(void* ptr) {
  delete [] (char*) ptr;
}
static void voidInvalidMarshal_fn(cdrStream&, void*) {
  OMNIORB_ASSERT(0);
}

CORBA::
Any::Any(TypeCode_ptr tc, void* value, Boolean release)
{
  pd_tc = CORBA::TypeCode::_duplicate(tc);

  if (value == 0) {
    // No value yet.
    pd_mbuf = 0;
    pd_data = 0;
    pd_marshal = 0;
    pd_destructor = 0;
  }
  else {
    // Create a cdrAnyMemoryStream referencing the data.
    pd_mbuf = new cdrAnyMemoryStream(value, release);
    pd_data = 0;
    pd_marshal = 0;
    pd_destructor = 0;
  }
}

void
CORBA::Any::replace(TypeCode_ptr tc, void* value, Boolean release)
{
  pd_tc = CORBA::TypeCode::_duplicate(tc);

  PR_clearData();

  if (value == 0) {
    // No value yet.
    pd_mbuf = 0;
    pd_data = 0;
    pd_marshal = 0;
    pd_destructor = 0;
  }
  else {
    // Create a cdrAnyMemoryStream referencing the data.
    pd_mbuf = new cdrAnyMemoryStream(value, release);
    pd_data = 0;
    pd_marshal = 0;
    pd_destructor = 0;
  }
}

//////////////////////////////////////////////////////////////////////
////////////////// Marshalling operators /////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::Any::operator>>= (cdrStream& s) const
{
  if (orbParameters::tcAliasExpand) {
    CORBA::TypeCode_var tc = TypeCode_base::aliasExpand(ToTcBase(pd_tc));
    CORBA::TypeCode::marshalTypeCode(tc, s);
  }
  else
    CORBA::TypeCode::marshalTypeCode(pd_tc, s);

  if (pd_data) {
    OMNIORB_ASSERT(pd_marshal);
    pd_marshal(s, pd_data);
  }
  else {
    OMNIORB_ASSERT(pd_mbuf);
    tcParser::copyMemStreamToStream_rdonly(pd_tc, *pd_mbuf, s);
  }
}

void
CORBA::Any::operator<<= (cdrStream& s)
{
  PR_clearData();

  pd_tc   = CORBA::TypeCode::unmarshalTypeCode(s);
  pd_mbuf = new cdrAnyMemoryStream;
  tcParser::copyStreamToStream(pd_tc, s, *pd_mbuf);
}

// omniORB data-only marshalling functions
void
CORBA::Any::NP_marshalDataOnly(cdrStream& s) const
{
  if (pd_data) {
    OMNIORB_ASSERT(pd_marshal);
    pd_marshal(s, pd_data);
  }
  else {
    OMNIORB_ASSERT(pd_mbuf);
    tcParser::copyMemStreamToStream_rdonly(pd_tc, *pd_mbuf, s);
  }
}

void
CORBA::Any::NP_unmarshalDataOnly(cdrStream& s)
{
  PR_clearData();
  pd_mbuf = new cdrAnyMemoryStream;
  tcParser::copyStreamToMemStream_flush(pd_tc, s, *pd_mbuf);
}


//////////////////////////////////////////////////////////////////////
////////////////// Insertion / extraction functions //////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::Any::
PR_insert(CORBA::TypeCode_ptr newtc, pr_marshal_fn marshal, void* data)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(newtc);
  pd_mbuf = new cdrAnyMemoryStream();
  marshal(*pd_mbuf, data);
}

void
CORBA::Any::
PR_insert(CORBA::TypeCode_ptr newtc, pr_marshal_fn marshal,
	  pr_destructor_fn destructor, void* data)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(newtc);
  pd_data = data;
  pd_marshal = marshal;
  pd_destructor = destructor;
}


CORBA::Boolean
CORBA::Any::
PR_extract(CORBA::TypeCode_ptr tc,
	   pr_unmarshal_fn     unmarshal,
	   void*               data) const
{
  if (!tc->equivalent(pd_tc))
    return 0;

  if (pd_mbuf) {
    // Make a temporary stream wrapper around memory buffer.
    cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
    
    // Extract the data
    unmarshal(tbuf, data);
    return 1;
  }
  else {
    OMNIORB_ASSERT(!pd_data);
    return 0;
  }
}


CORBA::Boolean
CORBA::Any::
PR_extract(CORBA::TypeCode_ptr     tc,
	   pr_unmarshal_fn  	   unmarshal,
	   pr_marshal_fn    	   marshal,
	   pr_destructor_fn 	   destructor,
	   void*&                  data) const
{
  if (!tc->equivalent(pd_tc))
    return 0;

  if (pd_data) {
    data = pd_data;
    return 1;
  }
  else if (pd_mbuf) {
    {
      // Make a temporary stream wrapper around memory buffer.
      cdrAnyMemoryStream tbuf(*pd_mbuf, 1);

      // Extract the data
      data = 0;
      unmarshal(tbuf, data);
      OMNIORB_ASSERT(data);
    }

    // Now set the data pointer
    CORBA::Boolean race = 0;
    {
      omni_tracedmutex_lock l(anyLock);

      if (pd_mbuf) {
	CORBA::Any* me = OMNI_CONST_CAST(CORBA::Any*, this);

	me->pd_data = data;
	me->pd_marshal = marshal;
	me->pd_destructor = destructor;
      }
      else {
	// Another thread got there first. We destroy the data we just
	// extracted, and return what the other thread made.
	race = 1;
      }
    }
    if (race) {
      destructor(data);
      data = pd_data;
    }
    return 1;
  }
  else {
    return 0;
  }
}

cdrAnyMemoryStream&
CORBA::Any::PR_streamToRead() const
{
  if (!pd_mbuf) {
    OMNIORB_ASSERT(pd_marshal);

    cdrAnyMemoryStream* mbuf = new cdrAnyMemoryStream;

    pd_marshal(*mbuf, pd_data);

    {
      omni_tracedmutex_lock l(anyLock);

      if (pd_mbuf) {
	// Another thread beat us to it
	delete mbuf;
      }
      else {
	CORBA::Any* me = OMNI_CONST_CAST(CORBA::Any*, this);
	me->pd_mbuf = mbuf;
      }
    }
  }
  return *pd_mbuf;
}

cdrAnyMemoryStream&
CORBA::Any::PR_streamToWrite()
{
  PR_clearData();
  pd_mbuf = new cdrAnyMemoryStream;
  return *pd_mbuf;
}


//////////////////////////////////////////////////////////////////////
////////////////// Simple insertion operators ////////////////////////
//////////////////////////////////////////////////////////////////////

// Simple types always go in the memory buffer.

void
CORBA::Any::operator<<=(Short s)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_short);
  pd_mbuf = new cdrAnyMemoryStream();
  s >>= *pd_mbuf;
}


void CORBA::Any::operator<<=(UShort u)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_ushort);
  pd_mbuf = new cdrAnyMemoryStream();
  u >>= *pd_mbuf;
}


void
CORBA::Any::operator<<=(Long l)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_long);
  pd_mbuf = new cdrAnyMemoryStream();
  l >>= *pd_mbuf;
}


void
CORBA::Any::operator<<=(ULong u)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_ulong);
  pd_mbuf = new cdrAnyMemoryStream();
  u >>= *pd_mbuf;
}

#ifdef HAS_LongLong
void
CORBA::Any::operator<<=(LongLong l)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_longlong);
  pd_mbuf = new cdrAnyMemoryStream();
  l >>= *pd_mbuf;
}

void
CORBA::Any::operator<<=(ULongLong u)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_ulonglong);
  pd_mbuf = new cdrAnyMemoryStream();
  u >>= *pd_mbuf;
}
#endif


#if !defined(NO_FLOAT)
void
CORBA::Any::operator<<=(Float f)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_float);
  pd_mbuf = new cdrAnyMemoryStream();
  f >>= *pd_mbuf;
}

void
CORBA::Any::operator<<=(Double d)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_double);
  pd_mbuf = new cdrAnyMemoryStream();
  d >>= *pd_mbuf;
}

#ifdef HAS_LongDouble
void
CORBA::Any::operator<<=(LongDouble d)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_longdouble);
  pd_mbuf = new cdrAnyMemoryStream();
  d >>= *pd_mbuf;
}
#endif

#endif


void
CORBA::Any::operator<<=(from_boolean b)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_boolean);
  pd_mbuf = new cdrAnyMemoryStream();
  pd_mbuf->marshalBoolean(b.val);
}

void
CORBA::Any::operator<<=(from_char c)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_char);
  pd_mbuf = new cdrAnyMemoryStream();
  pd_mbuf->marshalChar(c.val);
}

void
CORBA::Any::operator<<=(from_wchar c)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_wchar);
  pd_mbuf = new cdrAnyMemoryStream();
  pd_mbuf->marshalWChar(c.val);
}

void 
CORBA::Any::operator<<=(from_octet o)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_octet);
  pd_mbuf = new cdrAnyMemoryStream();
  pd_mbuf->marshalOctet(o.val);
}

void
CORBA::Any::operator<<=(from_fixed f)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::NP_fixed_tc(f.digits,f.scale);
  pd_mbuf = new cdrAnyMemoryStream();
  f.val >>= *pd_mbuf;
}


//////////////////////////////////////////////////////////////////////
////////////////// Simple extraction operators ///////////////////////
//////////////////////////////////////////////////////////////////////


CORBA::Boolean 
CORBA::Any::operator>>=(Short& s) const
{
  if (!pd_tc->equivalent(CORBA::_tc_short)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  s <<= tbuf;
  return 1;
}

CORBA::Boolean
CORBA::Any::operator>>=(UShort& u) const
{
  if (!pd_tc->equivalent(CORBA::_tc_ushort)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  u <<= tbuf;
  return 1;
}

CORBA::Boolean
CORBA::Any::operator>>=(Long& l) const
{
  if (!pd_tc->equivalent(CORBA::_tc_long)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  l <<= tbuf;
  return 1;
}


CORBA::Boolean
CORBA::Any::operator>>=(ULong& u) const
{
  if (!pd_tc->equivalent(CORBA::_tc_ulong)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  u <<= tbuf;
  return 1;
}


#ifdef HAS_LongLong
CORBA::Boolean
CORBA::Any::operator>>=(LongLong& l) const
{
  if (!pd_tc->equivalent(CORBA::_tc_longlong)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  l <<= tbuf;
  return 1;
}

  
CORBA::Boolean
CORBA::Any::operator>>=(ULongLong& u) const
{
  if (!pd_tc->equivalent(CORBA::_tc_ulonglong)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  u <<= tbuf;
  return 1;
}
#endif


#if !defined(NO_FLOAT)
CORBA::Boolean
CORBA::Any::operator>>=(Float& f) const
{
  if (!pd_tc->equivalent(CORBA::_tc_float)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  f <<= tbuf;
  return 1;
}


CORBA::Boolean
CORBA::Any::operator>>=(Double& d) const
{
  if (!pd_tc->equivalent(CORBA::_tc_double)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  d <<= tbuf;
  return 1;
}

#ifdef HAS_LongDouble
CORBA::Boolean
CORBA::Any::operator>>=(LongDouble& d) const
{
  if (!pd_tc->equivalent(CORBA::_tc_longdouble)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  d <<= tbuf;
  return 1;
}
#endif

#endif


CORBA::Boolean
CORBA::Any::operator>>=(to_boolean b) const
{
  if (!pd_tc->equivalent(CORBA::_tc_boolean)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  b.ref = tbuf.unmarshalBoolean();
  return 1;
}


CORBA::Boolean
CORBA::Any::operator>>=(to_char c) const
{
  if (!pd_tc->equivalent(CORBA::_tc_char)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  c.ref = tbuf.unmarshalChar();
  return 1;
}

    
CORBA::Boolean
CORBA::Any::operator>>=(to_wchar c) const
{
  if (!pd_tc->equivalent(CORBA::_tc_wchar)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  c.ref = tbuf.unmarshalWChar();
  return 1;
}

    
CORBA::Boolean
CORBA::Any::operator>>=(to_octet o) const
{
  if (!pd_tc->equivalent(CORBA::_tc_octet)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  o.ref = tbuf.unmarshalOctet();
  return 1;
}


CORBA::Boolean
CORBA::Any::operator>>=(to_fixed f) const
{
  CORBA::TypeCode_var tc = CORBA::TypeCode::NP_fixed_tc(f.digits,f.scale);

  if (!pd_tc->equivalent(tc)) return 0;
  OMNIORB_ASSERT(pd_mbuf);
  cdrAnyMemoryStream tbuf(*pd_mbuf, 1);
  f.val <<= tbuf;
  return 1;
}



//////////////////////////////////////////////////////////////////////
/////////////// Complex insertion/extraction operators ///////////////
//////////////////////////////////////////////////////////////////////

// Any

static void marshalAny_fn(cdrStream& s, void* d)
{
  CORBA::Any* a = (CORBA::Any*)d;
  *a >>= s;
}
static void unmarshalAny_fn(cdrStream& s, void*& d)
{
  CORBA::Any* a = new CORBA::Any;
  *a <<= s;
  d = a;
}
static void deleteAny_fn(void* d)
{
  CORBA::Any* a = (CORBA::Any*)d;
  delete a;
}

void
CORBA::Any::operator<<=(const Any& a)
{
  CORBA::Any* na = new CORBA::Any(a);
  PR_insert(CORBA::_tc_any, marshalAny_fn, deleteAny_fn, na);
}

void
CORBA::Any::operator<<=(Any* a)
{
  PR_insert(CORBA::_tc_any, marshalAny_fn, deleteAny_fn, a);
}

CORBA::Boolean CORBA::Any::operator>>=(const CORBA::Any*& a) const
{
  void* v;

  if (PR_extract(CORBA::_tc_any,
		 unmarshalAny_fn, marshalAny_fn, deleteAny_fn, v)) {

    a = (const CORBA::Any*)v;
    return 1;
  }
  return 0;
}

// Deprecated non-const version.
CORBA::Boolean CORBA::Any::operator>>=(CORBA::Any*& a) const
{
  return this->operator>>=((const CORBA::Any*&) a);
}

// Obsolete pre-CORBA 2.3 operator.
CORBA::Boolean CORBA::Any::operator>>=(Any& a) const
{
  const CORBA::Any* ap;
  if (*this >>= ap) {
    a = *ap;
    return 1;
  }
  return 0;
}


// TypeCode

static void marshalTypeCode_fn(cdrStream& s, void* d)
{
  CORBA::TypeCode_ptr t = (CORBA::TypeCode_ptr)d;
  CORBA::TypeCode::marshalTypeCode(t, s);
}
static void unmarshalTypeCode_fn(cdrStream& s, void*& d)
{
  CORBA::TypeCode_ptr t = CORBA::TypeCode::unmarshalTypeCode(s);
  d = t;
}
static void deleteTypeCode_fn(void* d)
{
  CORBA::TypeCode_ptr t = (CORBA::TypeCode_ptr)d;
  CORBA::release(t);
}

void
CORBA::Any::operator<<=(TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc)) {
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidTypeCode,CORBA::COMPLETED_NO);
  }
  CORBA::TypeCode_ptr ntc = CORBA::TypeCode::_duplicate(tc);
  PR_insert(CORBA::_tc_TypeCode, marshalTypeCode_fn, deleteTypeCode_fn, ntc);
}
void
CORBA::Any::operator<<=(TypeCode_ptr* tcp)
{
  if (!CORBA::TypeCode::PR_is_valid(*tcp)) {
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidTypeCode,CORBA::COMPLETED_NO);
  }
  PR_insert(CORBA::_tc_TypeCode, marshalTypeCode_fn, deleteTypeCode_fn, *tcp);
  *tcp = CORBA::TypeCode::_nil();
}
CORBA::Boolean
CORBA::Any::operator>>=(CORBA::TypeCode_ptr& tc) const
{
  void* v;
  if (PR_extract(CORBA::_tc_TypeCode,
		 unmarshalTypeCode_fn, marshalTypeCode_fn, deleteTypeCode_fn,
		 v)) {
    tc = (CORBA::TypeCode_ptr)v;
    return 1;
  }
  return 0;
}


// Object

static void marshalObject_fn(cdrStream& s, void* d)
{
  CORBA::Object_ptr o = (CORBA::Object_ptr)d;
  CORBA::Object::_marshalObjRef(o, s);
}
static void unmarshalObject_fn(cdrStream& s, void*& d)
{
  CORBA::Object_ptr o = CORBA::Object::_unmarshalObjRef(s);
  d = o;
}
static void deleteObject_fn(void* d)
{
  CORBA::Object_ptr o = (CORBA::Object_ptr)d;
  CORBA::release(o);
}

void
CORBA::Any::operator<<=(Object_ptr obj)
{
  if (!CORBA::Object::_PR_is_valid(obj)) {
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidObjectRef,CORBA::COMPLETED_NO);
  }
#if 0
  const char* repoid = CORBA::Object::_PD_repoId;
  const char* name   = "";
  if (!CORBA::is_nil(obj))
    repoid = obj->_PR_getobj()->_mostDerivedRepoId();
  CORBA::TypeCode_var tc = CORBA::TypeCode::NP_interface_tc(repoid,name);
#else
  CORBA::TypeCode_ptr tc = CORBA::_tc_Object;
#endif

  CORBA::Object_ptr no = CORBA::Object::_duplicate(obj);
  PR_insert(tc, marshalObject_fn, deleteObject_fn, no);
}

void
CORBA::Any::operator<<=(Object_ptr* objp)
{
  if (!CORBA::Object::_PR_is_valid(*objp)) {
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidObjectRef,CORBA::COMPLETED_NO);
  }
#if 0
  const char* repoid = CORBA::Object::_PD_repoId;
  const char* name   = "";
  if (!CORBA::is_nil(*objp))
    repoid = (*objp)->_PR_getobj()->_mostDerivedRepoId();
  CORBA::TypeCode_var tc = CORBA::TypeCode::NP_interface_tc(repoid,name);
#else
  CORBA::TypeCode_ptr tc = CORBA::_tc_Object;
#endif

  PR_insert(tc, marshalObject_fn, deleteObject_fn, *objp);
  *objp = CORBA::Object::_nil();
}

CORBA::Boolean
CORBA::Any::operator>>=(CORBA::Object_ptr& obj) const
{
  void* v;
  if (PR_extract(CORBA::_tc_Object,
		 unmarshalObject_fn, marshalObject_fn, deleteObject_fn,
		 v)) {
    obj = (CORBA::Object_ptr)v;
    return 1;
  }
  return 0;
}

CORBA::Boolean
CORBA::Any::operator>>=(to_object o) const
{
  void* v;

  if (pd_tc->kind() != CORBA::tk_objref)
    return 0;

  // We call PR_extract giving it our own TypeCode, so its type check
  // always succeeds, whatever specific object reference type we
  // contain.
  if (PR_extract(pd_tc,
		 unmarshalObject_fn, marshalObject_fn, deleteObject_fn,
		 v)) {
    o.ref = (CORBA::Object_ptr)v;
    return 1;
  }
  return 0;
}


// String

static void marshalString_fn(cdrStream& s, void* d)
{
  s.marshalString((const char*)d);
}
static void unmarshalString_fn(cdrStream& s, void*& d)
{
  char* c = s.unmarshalString();
  d = c;
}
static void deleteString_fn(void* d)
{
  CORBA::string_free((char*)d);
}

void
CORBA::Any::operator<<=(const char* s)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_string);

  char* ns = CORBA::string_dup(s);
  PR_insert(CORBA::_tc_string, marshalString_fn, deleteString_fn, ns);
}

void 
CORBA::Any::operator<<=(from_string s)
{
  CORBA::TypeCode_ptr tc;

  if (s.bound)
    tc = CORBA::TypeCode::NP_string_tc(s.bound);
  else
    tc = CORBA::TypeCode::_duplicate(CORBA::_tc_string);

  PR_clearData();
  pd_tc = tc;

  if (s.nc) {
    PR_insert(CORBA::_tc_string, marshalString_fn, deleteString_fn, s.val);
  }
  else {
    char* ns = CORBA::string_dup(s.val);
    PR_insert(CORBA::_tc_string, marshalString_fn, deleteString_fn, ns);
  }
}

CORBA::Boolean
CORBA::Any::operator>>=(const char*& s) const
{
  void* v;
  if (PR_extract(CORBA::_tc_string,
		 unmarshalString_fn, marshalString_fn, deleteString_fn,
		 v)) {
    s = (const char*)v;
    return 1;
  }
  return 0;
}

CORBA::Boolean
CORBA::Any::operator>>=(to_string s) const
{
  CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_string_tc(s.bound);

  void* v;
  if (PR_extract(newtc,
		 unmarshalString_fn, marshalString_fn, deleteString_fn,
		 v)) {
    s.val = (char*)v;
    return 1;
  }
  return 0;
}


// Wstring

static void marshalWString_fn(cdrStream& s, void* d)
{
  s.marshalWString((const CORBA::WChar*)d);
}
static void unmarshalWString_fn(cdrStream& s, void*& d)
{
  CORBA::WChar* c = s.unmarshalWString();
  d = c;
}
static void deleteWString_fn(void* d)
{
  CORBA::wstring_free((CORBA::WChar*)d);
}

void
CORBA::Any::operator<<=(const CORBA::WChar* s)
{
  PR_clearData();
  pd_tc = CORBA::TypeCode::_duplicate(CORBA::_tc_string);

  CORBA::WChar* ns = CORBA::wstring_dup(s);
  PR_insert(CORBA::_tc_wstring, marshalWString_fn, deleteWString_fn, ns);
}  

void 
CORBA::Any::operator<<=(from_wstring s)
{
  CORBA::TypeCode_ptr tc;

  if (s.bound)
    tc = CORBA::TypeCode::NP_wstring_tc(s.bound);
  else
    tc = CORBA::TypeCode::_duplicate(CORBA::_tc_wstring);

  PR_clearData();
  pd_tc = tc;

  if (s.nc) {
    PR_insert(CORBA::_tc_wstring, marshalWString_fn, deleteWString_fn, s.val);
  }
  else {
    CORBA::WChar* ns = CORBA::wstring_dup(s.val);
    PR_insert(CORBA::_tc_wstring, marshalWString_fn, deleteWString_fn, ns);
  }
}

CORBA::Boolean
CORBA::Any::operator>>=(const CORBA::WChar*& s) const
{
  void* v;
  if (PR_extract(CORBA::_tc_wstring,
		 unmarshalWString_fn, marshalWString_fn, deleteWString_fn,
		 v)) {
    s = (const CORBA::WChar*)v;
    return 1;
  }
  return 0;
}

CORBA::Boolean
CORBA::Any::operator>>=(to_wstring s) const
{
  CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_wstring_tc(s.bound);

  void* v;
  if (PR_extract(newtc,
		 unmarshalWString_fn, marshalWString_fn, deleteWString_fn,
		 v)) {
    s.val = (CORBA::WChar*)v;
    return 1;
  }
  return 0;
}


CORBA::Boolean
CORBA::Any::operator>>=(const CORBA::SystemException*& e) const
{
  CORBA::Boolean r;
#define EXTRACT_IF_MATCH(name) \
  if (pd_tc->equivalent(CORBA::_tc_##name)) { \
    const CORBA::name* ex; \
    r = *this >>= ex; \
    e = ex; \
    return r; \
  }
  OMNIORB_FOR_EACH_SYS_EXCEPTION(EXTRACT_IF_MATCH)
#undef EXTRACT_IF_MATCH

  return 0;
}


CORBA::TypeCode_ptr
CORBA::Any::type() const
{
  return CORBA::TypeCode::_duplicate(pd_tc);
}

void
CORBA::Any::type(CORBA::TypeCode_ptr tc)
{
  if (!pd_tc->equivalent(tc))
    OMNIORB_THROW(BAD_TYPECODE,
		  BAD_TYPECODE_NotEquivalent,
		  CORBA::COMPLETED_NO);

  pd_tc = CORBA::TypeCode::_duplicate(tc);
}

const void*
CORBA::Any::value() const
{
  if (pd_tc->kind() == CORBA::tk_null ||
      pd_tc->kind() == CORBA::tk_void)
    return 0;

  if (!pd_mbuf) {
    OMNIORB_ASSERT(pd_marshal);

    // We create a memory buffer and marshal our value into it. Note
    // that this will result in invalid data if we contain a
    // valuetype, since valuetypes do not get marshalled into the
    // memory buffer like other types. This value() method was
    // deprecated before valuetypes were specifified, so we consider
    // this an acceptable limitation.
    cdrAnyMemoryStream* mbuf = new cdrAnyMemoryStream;

    pd_marshal(*mbuf, pd_data);

    {
      omni_tracedmutex_lock l(anyLock);

      if (pd_mbuf) {
	// Another thread beat us to it
	delete mbuf;
      }
      else {
	CORBA::Any* me = OMNI_CONST_CAST(CORBA::Any*, this);
	me->pd_mbuf = mbuf;
      }
    }
  }
  return pd_mbuf->bufPtr();
}
