// -*- Mode: C++; -*-
//                            Package   : omniORB2
// any.cc                     Created on: 31/07/97
//                            Author1   : Eoin Carroll (ewc)
//                            Author2   : James Weatherall (jnw)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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


/* $Log$
/* Revision 1.10  1999/03/01 09:12:18  djr
/* Accept insertion of null strings into Any (with warning message)
/*
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

#include <anyP.h>
#include <typecode.h>


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
CORBA::Any::operator>>= (NetBufferedStream& s) const
{
  if( omniORB::tcAliasExpand ) {
    CORBA::TypeCode_var tc =
      TypeCode_base::aliasExpand(ToTcBase(pdAnyP()->getTC_parser()->getTC()));
    CORBA::TypeCode::marshalTypeCode(tc, s);
  }
  else
    CORBA::TypeCode::marshalTypeCode(pdAnyP()->getTC_parser()->getTC(), s);

  pdAnyP()->getTC_parser()->copyTo(s);
}

void
CORBA::Any::operator<<= (NetBufferedStream& s)
{
  CORBA::TypeCode_member newtc;
  newtc <<= s;
  pdAnyP()->setTC_and_reset(newtc);
  pdAnyP()->getTC_parser()->copyFrom(s);
}

void
CORBA::Any::operator>>= (MemBufferedStream& s) const
{
  if( omniORB::tcAliasExpand ) {
    CORBA::TypeCode_var tc =
      TypeCode_base::aliasExpand(ToTcBase(pdAnyP()->getTC_parser()->getTC()));
    CORBA::TypeCode::marshalTypeCode(tc, s);
  }
  else
    CORBA::TypeCode::marshalTypeCode(pdAnyP()->getTC_parser()->getTC(), s);

  pdAnyP()->getTC_parser()->copyTo(s);
}

void
CORBA::Any::operator<<= (MemBufferedStream& s)
{
  CORBA::TypeCode_member newtc;
  newtc <<= s;
  pdAnyP()->setTC_and_reset(newtc);
  pdAnyP()->getTC_parser()->copyFrom(s);
}

size_t
CORBA::Any::NP_alignedSize(size_t initialoffset) const
{
  size_t _msgsize = initialoffset;
  _msgsize = pdAnyP()->getTC_parser()->getTC()->NP_alignedSize(_msgsize);
  return NP_alignedDataOnlySize(_msgsize);
}

// omniORB2 data-only marshalling functions
void
CORBA::Any::NP_marshalDataOnly(NetBufferedStream& s) const
{
  pdAnyP()->getTC_parser()->copyTo(s);
}

void
CORBA::Any::NP_unmarshalDataOnly(NetBufferedStream& s)
{
  pdAnyP()->getTC_parser()->copyFrom(s);
}

void
CORBA::Any::NP_marshalDataOnly(MemBufferedStream& s) const
{
  pdAnyP()->getTC_parser()->copyTo(s);
}

void
CORBA::Any::NP_unmarshalDataOnly(MemBufferedStream& s)
{
  pdAnyP()->getTC_parser()->copyFrom(s);
}

size_t
CORBA::Any::NP_alignedDataOnlySize(size_t initialoffset) const
{
  return pdAnyP()->alignedSize(initialoffset);
}

// omniORB2 internal data packing functions, for use only by stub code
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
  // Delete the old internal AnyP and create a new one,
  // based on the Any passed to us
  delete pdAnyP();
  pd_ptr = new AnyP(pdAnyP2(&a));
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
#endif


void
CORBA::Any::operator<<=(const Any& a)
{
  tcDescriptor tcd;
  // *** Should we really subvert the 'const' stuff here?
  tcd.p_any = (CORBA::Any*)&a;
  pdAnyP()->setData(CORBA::_tc_any, tcd);
}


void
CORBA::Any::operator<<=(TypeCode_ptr tc)
{
  CORBA::TypeCode_member tcm(tc);
  tcDescriptor tcd;
  tcd.p_TypeCode = &tcm;
  pdAnyP()->setData(CORBA::_tc_TypeCode, tcd);
  tcm._ptr = CORBA::TypeCode::_nil();
}


static void
setObjectPtr(tcObjrefDesc* desc, CORBA::Object_ptr ptr)
{
  *((CORBA::Object_ptr*)desc->opq_objref) = ptr;
}


static CORBA::Object_ptr
getObjectPtr(tcObjrefDesc* desc)
{
  return * (CORBA::Object_ptr*) desc->opq_objref;
}


void
CORBA::Any::operator<<=(Object_ptr obj)
{
  tcDescriptor tcd;
  tcd.p_objref.opq_objref = (void*) &obj;
  tcd.p_objref.getObjectPtr = getObjectPtr;
  pdAnyP()->setData(CORBA::_tc_Object, tcd);
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
CORBA::Any::operator<<=(from_octet o)
{
  tcDescriptor tcd;
  tcd.p_octet = &(o.val);
  pdAnyP()->setData(CORBA::_tc_octet, tcd);
}


// Internal functions used when inserting raw string data
static CORBA::ULong 
tcParser_fromstring_getLength(tcStringDesc* tcsd)
{
  return tcsd->opq_len;
}


static char*
tcParser_fromstring_getBuffer(tcStringDesc* tcsd)
{
  return (char*) (tcsd->opq_string);
}


void
CORBA::Any::operator<<=(const char* s)
{
  tcDescriptor tcd;
  tcd.p_string.getLength = tcParser_fromstring_getLength;
  tcd.p_string.getBuffer = tcParser_fromstring_getBuffer;
  tcd.p_string.opq_string = (void*) s;
  tcd.p_string.opq_len = s ? strlen(s) : 0;

  pdAnyP()->setData(CORBA::_tc_string, tcd);
}  


void 
CORBA::Any::operator<<=(from_string s)
{
  tcDescriptor tcd;
  tcd.p_string.getLength = tcParser_fromstring_getLength;
  tcd.p_string.getBuffer = tcParser_fromstring_getBuffer;
  tcd.p_string.opq_string = s.val;
  tcd.p_string.opq_len = s.val ? strlen(s.val) : 0;

  if( s.bound ) {
    CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_string_tc(s.bound);
    pdAnyP()->setData(newtc, tcd);
  }else
    pdAnyP()->setData(CORBA::_tc_string, tcd);

  if( s.nc )  CORBA::string_free(s.val);
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
#endif


CORBA::Boolean CORBA::Any::operator>>=(Any& a) const
{
  tcDescriptor tcd;
  tcd.p_any = &a;
  return pdAnyP()->getData(CORBA::_tc_any, tcd);
}


CORBA::Boolean
CORBA::Any::operator>>=(TypeCode_ptr& tc) const
{
  CORBA::TypeCode_member tcm;
  tcDescriptor tcd;
  tcd.p_TypeCode = &tcm;
  CORBA::Boolean ret = pdAnyP()->getData(CORBA::_tc_TypeCode, tcd);
  if( ret )
    tc = tcm._ptr;
  else
    tc = CORBA::TypeCode::_nil();
  tcm._ptr = CORBA::TypeCode::_nil();
  return ret;
}


CORBA::Boolean
CORBA::Any::operator>>=(Object_ptr& obj) const
{
  tcDescriptor tcd;
  tcd.p_objref.opq_objref = (void*) &obj;
  tcd.p_objref.setObjectPtr = setObjectPtr;
  return pdAnyP()->getData(CORBA::_tc_Object, tcd);
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
CORBA::Any::operator>>=(to_octet o) const
{
  tcDescriptor tcd;
  tcd.p_octet = &o.ref;
  return pdAnyP()->getData(CORBA::_tc_octet, tcd);
}


// Internal functions used when extracting string data
static void
tcParser_tostring_setLength(tcStringDesc* tcsd, CORBA::ULong len)
{
  if (tcsd->opq_string != NULL)
    CORBA::string_free((char*) tcsd->opq_string);
  tcsd->opq_string = CORBA::string_alloc(len);
  if (tcsd->opq_string != NULL)
    tcsd->opq_len = len;
}


static CORBA::ULong 
tcParser_tostring_getLength(tcStringDesc* tcsd)
{
  return tcsd->opq_len;
}


static char*
tcParser_tostring_getBuffer(tcStringDesc* tcsd)
{
  return (char*) (tcsd->opq_string);
}


CORBA::Boolean
CORBA::Any::operator>>=(char*& s) const
{
  tcDescriptor tcd;
  tcd.p_string.setLength = tcParser_tostring_setLength;
  tcd.p_string.getLength = tcParser_tostring_getLength;
  tcd.p_string.getBuffer = tcParser_tostring_getBuffer;
  tcd.p_string.opq_string = 0;
  tcd.p_string.opq_len = 0;

  if (pdAnyP()->getData(CORBA::_tc_string, tcd))
    {
      s = (char*) tcd.p_string.opq_string;
      return 1;
    }

  return 0;
}


CORBA::Boolean
CORBA::Any::operator>>=(to_string s) const
{
  CORBA::TypeCode_var newtc = CORBA::TypeCode::NP_string_tc(s.bound);

  tcDescriptor tcd;
  tcd.p_string.setLength = tcParser_tostring_setLength;
  tcd.p_string.getLength = tcParser_tostring_getLength;
  tcd.p_string.getBuffer = tcParser_tostring_getBuffer;
  tcd.p_string.opq_string = 0;
  tcd.p_string.opq_len = 0;

  if (pdAnyP()->getData(newtc, tcd))
    {
      s.val = (char*) tcd.p_string.opq_string;
      return 1;
    }

  return 0;
}


CORBA::Boolean
CORBA::Any::operator>>=(to_object o) const
{
  tcDescriptor tcd;
  tcd.p_objref.opq_objref = (void*) &o.ref;
  tcd.p_objref.setObjectPtr = setObjectPtr;
  return pdAnyP()->getObjRef(tcd);
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
  return CORBA::TypeCode::_duplicate(pdAnyP()->getTC_parser()->getTC());
}


const void*
CORBA::Any::value() const
{
  return pdAnyP()->getBuffer();
}
