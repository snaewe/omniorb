// -*- Mode: C++; -*-
//                            Package   : omniORB
// DynAny.cc                  Created on: 12/02/98
//                            Author    : Sai-Lai Lo (sll)
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
//      Implementation of type DynAny


/*
   $Log$
   Revision 1.11.2.14  2001/10/19 11:04:02  dpg1
   Avoid confusing (to gcc 2.95) inheritance of refcount functions.

   Revision 1.11.2.13  2001/10/17 18:51:50  dpg1
   Fix inevitable Windows problems.

   Revision 1.11.2.12  2001/10/17 16:44:02  dpg1
   Update DynAny to CORBA 2.5 spec, const Any exception extraction.

   Revision 1.11.2.11  2001/09/24 10:41:08  dpg1
   Minor codes for Dynamic library and omniORBpy.

   Revision 1.11.2.10  2001/08/22 13:29:46  dpg1
   Re-entrant Any marshalling.

   Revision 1.11.2.9  2001/06/15 10:23:21  sll
   Changed the name of the internal create_dyn_any function to
   internal_create_dyn_any. Compilers which do not support namespace are
   confused by the original name.

   Revision 1.11.2.8  2001/06/13 20:10:04  sll
   Minor update to make the ORB compiles with MSVC++.

   Revision 1.11.2.7  2001/04/19 09:14:15  sll
   Scoped where appropriate with the omni namespace.

   Revision 1.11.2.6  2000/11/17 19:09:37  dpg1
   Support codeset conversion in any.

   Revision 1.11.2.5  2000/11/09 12:27:53  dpg1
   Huge merge from omni3_develop, plus full long long from omni3_1_develop.

   Revision 1.11.2.4  2000/11/03 19:07:32  sll
   Use new marshalling functions for byte, octet and char. Use get_octet_array
   instead of get_char_array.

   Revision 1.11.2.3  2000/10/06 16:40:53  sll
   Changed to use cdrStream.

   Revision 1.11.2.2  2000/09/27 17:25:41  sll
   Changed include/omniORB3 to include/omniORB4.

   Revision 1.11.2.1  2000/07/17 10:35:41  sll
   Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

   Revision 1.12  2000/07/13 15:26:02  dpg1
   Merge from omni3_develop for 3.0 release.

   Revision 1.8.6.5  2000/06/22 10:40:12  dpg1
   exception.h renamed to exceptiondefs.h to avoid name clash on some
   platforms.

   Revision 1.8.6.4  1999/10/26 20:18:18  sll
   DynAny no longer do alias expansion on the typecode. In other words, all
   aliases in the typecode are preserved.

   Revision 1.8.6.3  1999/10/14 16:21:56  djr
   Implemented logging when system exceptions are thrown.

   Revision 1.8.6.2  1999/09/22 16:15:58  djr
   Removed MT locking.

   Revision 1.8.6.1  1999/09/22 14:26:30  djr
   Major rewrite of orbcore to support POA.

   Revision 1.8  1999/07/20 14:22:58  djr
   Accept nil ref in insert_reference().
   Allow DynAny with type tk_void.

   Revision 1.7  1999/06/18 21:01:11  sll
   Use TypeCode equivalent() instead of equal().

   Revision 1.6  1999/05/25 18:05:00  sll
   Added check for invalid arguments using magic numbers.

   Revision 1.5  1999/03/11 16:25:58  djr
   Updated copyright notice

   Revision 1.4  1999/01/07 16:58:16  djr
   New implementation using new version of TypeCode and Any.

   Revision 1.3  1998/08/25 18:52:59  sll
   Added signed-unsigned cast to keep egcs and gcc-2.7.2 happy.

   Revision 1.2  1998/08/14 13:45:31  sll
   Added pragma hdrstop to control pre-compile header if the compiler feature
   is available.

   Revision 1.1  1998/08/05 18:03:49  sll
   Initial revision

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <dynAnyImpl.h>
#include <tcParser.h>
#include <anyP.h>
#include <exceptiondefs.h>
#include <initialiser.h>
#include <initRefs.h>

DynamicAny::DynAny::~DynAny() { pd_dynmagic = 0; }
DynamicAny::DynFixed::~DynFixed() {}
DynamicAny::DynEnum::~DynEnum() {}
DynamicAny::DynStruct::~DynStruct() {}
DynamicAny::DynUnion::~DynUnion() {}
DynamicAny::DynSequence::~DynSequence() {}
DynamicAny::DynArray::~DynArray() {}
DynamicAny::DynAnyFactory::~DynAnyFactory() {}

const char* DynamicAny::DynAny::
_PD_repoId = "IDL:omg.org/DynamicAny/DynAny:1.0";

const char* DynamicAny::DynFixed::
_PD_repoId = "IDL:omg.org/DynamicAny/DynFixed:1.0";

const char* DynamicAny::DynEnum::
_PD_repoId = "IDL:omg.org/DynamicAny/DynEnum:1.0";

const char* DynamicAny::DynStruct::
_PD_repoId = "IDL:omg.org/DynamicAny/DynStruct:1.0";

const char* DynamicAny::DynUnion::
_PD_repoId = "IDL:omg.org/DynamicAny/DynUnion:1.0";

const char* DynamicAny::DynSequence::
_PD_repoId = "IDL:omg.org/DynamicAny/DynSequence:1.0";

const char* DynamicAny::DynArray::
_PD_repoId = "IDL:omg.org/DynamicAny/DynArray:1.0";

const char* DynamicAny::DynAnyFactory::
_PD_repoId = "IDL:omg.org/DynamicAny/DynAnyFactory:1.0";

OMNI_NAMESPACE_BEGIN(omni)

// Fake repository ids for implementation classes
const char* DynAnyImplBase::
_PD_repoId = "omniorb:DynAnyImplBase";

const char* DynAnyImpl::
_PD_repoId = "omniorb:DynAnyImpl";

const char* DynAnyConstrBase::
_PD_repoId = "omniorb:DynAnyConstrBase";

const char* DynUnionImpl::
_PD_repoId = "omniorb:DynUnionImpl";


// Note:
//  The constructors in this file NEVER throw exceptions. This
// is to avoid a bug in MSVC 4 & 5, which happens when public
// virtual inheritance is used.


static DynAnyImplBase*
internal_create_dyn_any(TypeCode_base* tc, CORBA::Boolean is_root);

static DynUnionDisc*
internal_create_dyn_any_discriminator(TypeCode_base* tc, DynUnionImpl* du);

static DynamicAny::DynAny_ptr
factory_create_dyn_any(const CORBA::Any& value);

static DynamicAny::DynAny_ptr
factory_create_dyn_any_from_type_code(CORBA::TypeCode_ptr type);



//////////////////////////////////////////////////////////////////////
/////////////////////////// DynAnyImplBase ///////////////////////////
//////////////////////////////////////////////////////////////////////

#define CHECK_NOT_DESTROYED \
do { \
  if (!DynamicAny::DynAny::PR_is_valid(this)) \
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO); \
  if (destroyed()) \
    OMNIORB_THROW(OBJECT_NOT_EXIST, OBJECT_NOT_EXIST_DynAnyDestroyed, \
                  CORBA::COMPLETED_NO); \
} while(0)


omni_tracedmutex DynAnyImplBase::refCountLock;


DynAnyImplBase::~DynAnyImplBase()
{
  CORBA::release(pd_tc);
}

//////////////////////
// public interface //
//////////////////////

CORBA::TypeCode_ptr
DynAnyImplBase::type() const
{
  CHECK_NOT_DESTROYED;
  return CORBA::TypeCode::_duplicate(pd_tc);
}

void
DynAnyImplBase::from_any(const CORBA::Any& value)
{
  CHECK_NOT_DESTROYED;
  CORBA::TypeCode_var value_tc = value.type();
  if( !value_tc->equivalent(tc()) )  throw DynamicAny::DynAny::TypeMismatch();

  cdrMemoryStream buf(((AnyP*)value.NP_pd())->theMemoryStream(), 1);

  if( !copy_from(buf) )  throw DynamicAny::DynAny::InvalidValue();
}

CORBA::Any*
DynAnyImplBase::to_any()
{
  CHECK_NOT_DESTROYED;
  CORBA::Any* a = new CORBA::Any(tc(), 0);
  cdrMemoryStream& buf = ((AnyP*)a->NP_pd())->getWRableMemoryStream();

  // <buf> should already be rewound.

  if( !copy_to(buf) ) {
    delete a;
    throw DynamicAny::DynAny::InvalidValue();
  }
  return a;
}

void
DynAnyImplBase::destroy()
{
  if (is_root()) {
    omni_tracedmutex_lock sync(DynAnyImplBase::refCountLock);
    CHECK_NOT_DESTROYED;
    pd_destroyed = 1;
    // We don't actually do anything to destroy the data.
  }
  else
    CHECK_NOT_DESTROYED;
}


//////////////
// internal //
//////////////

void
DynAnyImplBase::onDispose()
{
  // no-op
}


void
DynAnyImplBase::_NP_incrRefCount()
{
  omni_tracedmutex_lock sync(DynAnyImplBase::refCountLock);
  pd_refcount++;
}


void
DynAnyImplBase::_NP_decrRefCount()
{
  int do_delete = 0;
  {
    omni_tracedmutex_lock sync(DynAnyImplBase::refCountLock);

    if( pd_refcount > 0 )  pd_refcount--;

    if( pd_refcount == 0 && pd_is_root ) {
      if ( !pd_destroyed && omniORB::trace(25)) {
	omniORB::logger l;
	l << "Warning: Last reference to DynAny dropped without having "
	     "called destroy()\n";
      }
      do_delete = 1;
      // This hook allows derived types to detach their children
      // so that they will be destroyed when their ref count goes
      // to zero.
      onDispose();
    }
  }

  if( do_delete )  delete this;
}

//////////////////////////////////////////////////////////////////////
///////////////////////////// DynAnyImpl /////////////////////////////
//////////////////////////////////////////////////////////////////////


DynAnyImpl::DynAnyImpl(TypeCode_base* tc, int nodetype, CORBA::Boolean is_root)
  : DynAnyImplBase(tc, nodetype, is_root), pd_isValid(0)
{
}


DynAnyImpl::~DynAnyImpl()
{
}

//////////////////////
// public interface //
//////////////////////

void
DynAnyImpl::assign(DynamicAny::DynAny_ptr da)
{
  CHECK_NOT_DESTROYED;

  if ( !DynamicAny::DynAny::PR_is_valid(da) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);

  if( CORBA::is_nil(da) )
    throw DynamicAny::DynAny::InvalidValue();

  DynAnyImplBase* daib = ToDynAnyImplBase(da);

  if( !tc()->equivalent(daib->tc()) )
    throw DynamicAny::DynAny::TypeMismatch();

  DynAnyImpl* dai = ToDynAnyImpl(daib);

  if( !dai->isValid() )  throw DynamicAny::DynAny::InvalidValue();

  dai->pd_buf.rewindInputPtr();
  pd_buf.rewindPtrs();

  dai->pd_buf.copy_to(pd_buf,dai->pd_buf.bufSize());
  setValid();
}


DynamicAny::DynAny_ptr
DynAnyImpl::copy()
{
  CHECK_NOT_DESTROYED;

  DynAnyImpl* da = new DynAnyImpl(TypeCode_collector::duplicateRef(tc()),
				  dt_any, DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(DynamicAny::DynAny::TypeMismatch&) {
    OMNIORB_ASSERT(0);
    // This should never happen since the types always match -- they
    // are the same TypeCode.
  }
  catch(...) {
    da->_NP_decrRefCount();
    throw;
  }
  return da;
}


CORBA::Boolean
DynAnyImpl::equal(DynamicAny::DynAny_ptr dyn_any)
{
  CHECK_NOT_DESTROYED;

  if ( !DynamicAny::DynAny::PR_is_valid(dyn_any) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);

  if( CORBA::is_nil(dyn_any) )
    return 0;

  DynAnyImplBase* ib = ToDynAnyImplBase(dyn_any);
  if (!tc()->equivalent(ib->tc()))
    return 0;

  switch (actualTc()->NP_kind()) {
  case CORBA::tk_any:
    {
      DynamicAny::DynAny_var a = get_dyn_any();
      DynamicAny::DynAny_var b = dyn_any->get_dyn_any();
      return a->equal(b);
    }

  case CORBA::tk_TypeCode:
    {
      // TypeCodes must be equal, not just equivalent.
      // We can't just compare the marshalled form, since they may
      // have different indirections.
      CORBA::TypeCode_var a = get_typecode();
      CORBA::TypeCode_var b = dyn_any->get_typecode();
      return a->equal(b);
    }
  case CORBA::tk_objref:
    {
      // Compare objrefs for equivalence.
      CORBA::Object_var a = get_reference();
      CORBA::Object_var b = dyn_any->get_reference();
      return a->_is_equivalent(b);
    }
  default:
    // With all other types supported by this class, it's sufficient
    // to see if the data in the memory buffers is identical.
    if (pd_buf.bufSize() != ib->pd_buf.bufSize())
      return 0;
    return (!memcmp(pd_buf.bufPtr(), ib->pd_buf.bufPtr(), pd_buf.bufSize()));
  }
}


void
DynAnyImpl::insert_boolean(CORBA::Boolean value)
{
  CHECK_NOT_DESTROYED;
  doWrite(CORBA::tk_boolean).marshalBoolean(value);
}


void
DynAnyImpl::insert_octet(CORBA::Octet value)
{
  CHECK_NOT_DESTROYED;
  doWrite(CORBA::tk_octet).marshalOctet(value);
}


void
DynAnyImpl::insert_char(CORBA::Char value)
{
  CHECK_NOT_DESTROYED;
  doWrite(CORBA::tk_char).marshalChar(value);
}


void
DynAnyImpl::insert_short(CORBA::Short value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_short);
}


void
DynAnyImpl::insert_ushort(CORBA::UShort value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_ushort);
}


void
DynAnyImpl::insert_long(CORBA::Long value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_long);
}


void
DynAnyImpl::insert_ulong(CORBA::ULong value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_ulong);
}

#ifndef NO_FLOAT
void
DynAnyImpl::insert_float(CORBA::Float value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_float);
}

void
DynAnyImpl::insert_double(CORBA::Double value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_double);
}

#endif

void
DynAnyImpl::insert_string(const char* value)
{
  CHECK_NOT_DESTROYED;
  if( !value )
    throw DynamicAny::DynAny::InvalidValue();

  if ( tckind() != CORBA::tk_string )
    throw DynamicAny::DynAny::TypeMismatch();

  CORBA::ULong maxlen = actualTc()->NP_length();
  cdrMemoryStream& buf = doWrite(CORBA::tk_string);
  try {
    buf.marshalString(value,maxlen);
  }
  catch (CORBA::MARSHAL& ex) {
    if (ex.minor() == MARSHAL_StringIsTooLong)
      throw DynamicAny::DynAny::InvalidValue();
    else
      throw;
  }
}


void
DynAnyImpl::insert_reference(CORBA::Object_ptr value)
{
  CHECK_NOT_DESTROYED;

  if ( !CORBA::Object::_PR_is_valid(value) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidObjectRef, CORBA::COMPLETED_NO);

  CORBA::Object::_marshalObjRef(value, doWrite(CORBA::tk_objref));
}


void
DynAnyImpl::insert_typecode(CORBA::TypeCode_ptr value)
{
  CHECK_NOT_DESTROYED;

  if ( !CORBA::TypeCode::PR_is_valid(value) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);

  if( CORBA::is_nil(value) )  throw DynamicAny::DynAny::InvalidValue();

  CORBA::TypeCode::marshalTypeCode(value, doWrite(CORBA::tk_TypeCode));
}


#ifdef HAS_LongLong
void
DynAnyImpl::insert_longlong(CORBA::LongLong value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_longlong);
}

void
DynAnyImpl::insert_ulonglong(CORBA::ULongLong value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_ulonglong);
}
#endif

#ifdef HAS_LongDouble
void
DynAnyImpl::insert_longdouble(CORBA::LongDouble value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_longdouble);
}
#endif

void
DynAnyImpl::insert_wchar(CORBA::WChar value)
{
  CHECK_NOT_DESTROYED;
  doWrite(CORBA::tk_wchar).marshalWChar(value);
}

void
DynAnyImpl::insert_wstring(const CORBA::WChar* value)
{
  if( !value )
    throw DynamicAny::DynAny::InvalidValue();

  if ( tckind() != CORBA::tk_wstring )
    throw DynamicAny::DynAny::TypeMismatch();

  CORBA::ULong maxlen = actualTc()->NP_length();
  cdrMemoryStream& buf = doWrite(CORBA::tk_wstring);
  try {
    buf.marshalWString(value,maxlen);
  }
  catch (CORBA::MARSHAL& ex) {
    if (ex.minor() == MARSHAL_WStringIsTooLong)
      throw DynamicAny::DynAny::InvalidValue();
    else
      throw;
  }
}


void
DynAnyImpl::insert_any(const CORBA::Any& value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_any);
}

void
DynAnyImpl::insert_dyn_any(DynamicAny::DynAny_ptr value)
{
  CHECK_NOT_DESTROYED;
  CORBA::Any_var a(to_any());
  a.in() >>= doWrite(CORBA::tk_any);
}


CORBA::Boolean
DynAnyImpl::get_boolean()
{
  CHECK_NOT_DESTROYED;
  return doRead(CORBA::tk_boolean).unmarshalBoolean();
}


CORBA::Octet
DynAnyImpl::get_octet()
{
  CHECK_NOT_DESTROYED;
  return doRead(CORBA::tk_octet).unmarshalOctet();
}


CORBA::Char
DynAnyImpl::get_char()
{
  CHECK_NOT_DESTROYED;
  return doRead(CORBA::tk_char).unmarshalChar();
}


CORBA::Short
DynAnyImpl::get_short()
{
  CHECK_NOT_DESTROYED;
  CORBA::Short value;
  value <<= doRead(CORBA::tk_short);
  return value;
}


CORBA::UShort
DynAnyImpl::get_ushort()
{
  CHECK_NOT_DESTROYED;
  CORBA::UShort value;
  value <<= doRead(CORBA::tk_ushort);
  return value;
}


CORBA::Long
DynAnyImpl::get_long()
{
  CHECK_NOT_DESTROYED;
  CORBA::Long value;
  value <<= doRead(CORBA::tk_long);
  return value;
}


CORBA::ULong
DynAnyImpl::get_ulong()
{
  CHECK_NOT_DESTROYED;
  CORBA::ULong value;
  value <<= doRead(CORBA::tk_ulong);
  return value;
}

#ifndef NO_FLOAT
CORBA::Float
DynAnyImpl::get_float()
{
  CHECK_NOT_DESTROYED;
  CORBA::Float value;
  value <<= doRead(CORBA::tk_float);
  return value;
}

CORBA::Double
DynAnyImpl::get_double()
{
  CHECK_NOT_DESTROYED;
  CORBA::Double value;
  value <<= doRead(CORBA::tk_double);
  return value;
}
#endif

char*
DynAnyImpl::get_string()
{
  CHECK_NOT_DESTROYED;
  cdrMemoryStream& buf = doRead(CORBA::tk_string);

  CORBA::ULong length;
  CORBA::ULong maxlen = actualTc()->NP_length();
  try {
    char* value = buf.unmarshalString(maxlen);
    return value;
  }
  catch (CORBA::MARSHAL& ex) {
    if (ex.minor() == MARSHAL_StringIsTooLong)
      throw DynamicAny::DynAny::InvalidValue();
    else
      throw;
  }
  return 0;
}


CORBA::Object_ptr
DynAnyImpl::get_reference()
{
  CHECK_NOT_DESTROYED;
  return CORBA::Object::_unmarshalObjRef(doRead(CORBA::tk_objref));
}


CORBA::TypeCode_ptr
DynAnyImpl::get_typecode()
{
  CHECK_NOT_DESTROYED;
  return CORBA::TypeCode::unmarshalTypeCode(doRead(CORBA::tk_TypeCode));
}


#ifdef HAS_LongLong
CORBA::LongLong
DynAnyImpl::get_longlong()
{
  CHECK_NOT_DESTROYED;
  CORBA::LongLong value;
  value <<= doRead(CORBA::tk_longlong);
  return value;
}

CORBA::ULongLong
DynAnyImpl::get_ulonglong()
{
  CHECK_NOT_DESTROYED;
  CORBA::ULongLong value;
  value <<= doRead(CORBA::tk_ulonglong);
  return value;
}
#endif

#ifdef HAS_LongDouble
CORBA::LongDouble
DynAnyImpl::get_longdouble()
{
  CHECK_NOT_DESTROYED;
  CORBA::LongDouble value;
  value <<= doRead(CORBA::tk_longdouble);
  return value;
}
#endif

CORBA::WChar
DynAnyImpl::get_wchar()
{
  CHECK_NOT_DESTROYED;
  return doRead(CORBA::tk_wchar).unmarshalWChar();
}

CORBA::WChar*
DynAnyImpl::get_wstring()
{
  CHECK_NOT_DESTROYED;
  cdrMemoryStream& buf = doRead(CORBA::tk_wstring);

  CORBA::ULong length;
  CORBA::ULong maxlen = actualTc()->NP_length();
  CORBA::WChar* value = buf.unmarshalWString(maxlen);
  return value;
}

CORBA::Any*
DynAnyImpl::get_any()
{
  cdrMemoryStream& buf = doRead(CORBA::tk_any);

  CORBA::Any* value = new CORBA::Any();
  *value <<= buf;
  return value;
}

DynamicAny::DynAny_ptr
DynAnyImpl::get_dyn_any()
{
  // This could be made faster by short-cutting the Any step, but it's
  // probably not worth the effort.
  cdrMemoryStream& buf = doRead(CORBA::tk_any);
  CORBA::Any a;
  a <<= buf;
  return factory_create_dyn_any(a);
}


// CORBA 2.5 sequence operations

#define SEQUENCE_OPS(lcname, ucname) \
void \
DynAnyImpl::insert_##lcname##_seq(CORBA::##ucname##Seq& value) \
{ \
  CHECK_NOT_DESTROYED; \
  throw DynamicAny::DynAny::TypeMismatch(); \
} \
\
CORBA::##ucname##Seq* \
DynAnyImpl::get_##lcname##_seq() \
{ \
  CHECK_NOT_DESTROYED; \
  throw DynamicAny::DynAny::TypeMismatch(); \
  return 0; \
}

SEQUENCE_OPS(boolean, Boolean)
SEQUENCE_OPS(octet, Octet)
SEQUENCE_OPS(char, Char)
SEQUENCE_OPS(short, Short)
SEQUENCE_OPS(ushort, UShort)
SEQUENCE_OPS(long, Long)
SEQUENCE_OPS(ulong, ULong)
#ifndef NO_FLOAT
SEQUENCE_OPS(float, Float)
SEQUENCE_OPS(double, Double)
#endif
#ifdef HAS_LongLong
SEQUENCE_OPS(longlong, LongLong)
SEQUENCE_OPS(ulonglong, ULongLong)
#endif
#ifdef HAS_LongDouble
SEQUENCE_OPS(longdouble, LongDouble)
#endif
SEQUENCE_OPS(wchar, WChar)

#undef SEQUENCE_OPS


CORBA::Boolean
DynAnyImpl::seek(CORBA::Long index)
{
  return 0;
}


void
DynAnyImpl::rewind()
{
  // no-op
}


CORBA::Boolean
DynAnyImpl::next()
{
  return 0;
}

CORBA::ULong
DynAnyImpl::component_count()
{
  return 0;
}


DynamicAny::DynAny_ptr
DynAnyImpl::current_component()
{
  // The spec is slightly unclear about whether current_component on
  // base type DynAny should return nil or raise TypeMismatch. We
  // choose to raise the exception.
  throw DynamicAny::DynAny::TypeMismatch();
  return 0;
}




///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynAnyImpl::NP_nodetype() const
{
  return dt_any;
}

//////////////
// internal //
//////////////

void
DynAnyImpl::set_to_initial_value()
{
  switch (actualTc()->NP_kind()) {
  case CORBA::tk_null:
  case CORBA::tk_void:
    ToDynAnyImpl(this)->setValid();
    break;

  case CORBA::tk_short:      insert_short(0);     break;
  case CORBA::tk_long:       insert_long(0);      break;
  case CORBA::tk_ushort:     insert_ushort(0);    break;
  case CORBA::tk_ulong:      insert_ulong(0);     break;
#ifndef NO_FLOAT
  case CORBA::tk_float:      insert_float(0.0);   break;
  case CORBA::tk_double:     insert_double(0.0);  break;
#endif
  case CORBA::tk_boolean:    insert_boolean(0);   break;
  case CORBA::tk_char:       insert_char(0);      break;
  case CORBA::tk_octet:      insert_octet(0);     break;
  case CORBA::tk_any:
    {
      CORBA::Any a;
      insert_any(a);
      break;
    }
  case CORBA::tk_TypeCode:   insert_typecode(CORBA::_tc_null);        break;
  case CORBA::tk_objref:     insert_reference(CORBA::Object::_nil()); break;
  case CORBA::tk_string:
    insert_string(_CORBA_String_helper::empty_string);
    break;
#ifdef HAS_LongLong
  case CORBA::tk_longlong:   insert_longlong(0);     break;
  case CORBA::tk_ulonglong:  insert_ulonglong(0);    break;
#endif
#ifdef HAS_LongDouble
  case CORBA::tk_longdouble: insert_longdouble(0.0); break;
#endif
  case CORBA::tk_wchar:      insert_wchar(0);        break;
  case CORBA::tk_wstring:
    insert_wstring(_CORBA_WString_helper::empty_wstring);
    break;
  default:
    // Should never get here.
    OMNIORB_ASSERT(0);
  }
}


int
DynAnyImpl::copy_to(cdrMemoryStream& mbs)
{
  cdrMemoryStream src(pd_buf, 1);
  try {
    tcParser::copyStreamToStream(tc(), src, mbs);
  }
  catch(CORBA::MARSHAL&) {
    return 0;
  }
  return 1;
}


int
DynAnyImpl::copy_from(cdrMemoryStream& mbs)
{
  try {
    setInvalid();
    tcParser::copyStreamToMemStream_flush(tc(), mbs, pd_buf);
    setValid();
  }
  catch(CORBA::MARSHAL&) {
    return 0;
  }
  return 1;
}

void*
DynAnyImpl::_ptrToObjRef(const char* repoId)
{
  if( omni::ptrStrMatch(repoId, DynAnyImpl::_PD_repoId) )
    return (DynAnyImpl*) this;
  
  if( omni::ptrStrMatch(repoId, DynAnyImplBase::_PD_repoId) )
    return (DynAnyImplBase*) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
    return (DynamicAny::DynAny_ptr) this;
  
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}



//////////////////////////////////////////////////////////////////////
//////////////////////////// DynFixedImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

DynFixedImpl::DynFixedImpl(TypeCode_base* tc, CORBA::Boolean is_root)
  : DynAnyImpl(tc, dt_fixed, is_root)
{
}

DynFixedImpl::~DynFixedImpl() {}

//////////////////////
// public interface //
//////////////////////

DynamicAny::DynAny_ptr
DynFixedImpl::copy()
{
  CHECK_NOT_DESTROYED;

  DynFixedImpl* da = new DynFixedImpl(TypeCode_collector::duplicateRef(tc()),
				      DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(DynamicAny::DynAny::TypeMismatch&) {
    OMNIORB_ASSERT(0);
    // This should never happen since the types always match -- they
    // are the same TypeCode.
  }
  catch(...) {
    da->_NP_decrRefCount();
    throw;
  }
  return da;
}

char*
DynFixedImpl::get_value()
{
  CHECK_NOT_DESTROYED;

  CORBA::Fixed f;
  f.PR_setLimits(actualTc()->NP_fixed_digits(), actualTc()->NP_fixed_scale());
  pd_buf.rewindInputPtr();
  f <<= pd_buf;

  return f.NP_asString();
}

CORBA::Boolean
DynFixedImpl::set_value(const char* val)
{
  CHECK_NOT_DESTROYED;

  CORBA::Fixed f;
  f.PR_setLimits(actualTc()->NP_fixed_digits(), actualTc()->NP_fixed_scale());

  try {
    CORBA::Boolean precise = f.NP_fromString(val);
    pd_buf.rewindPtrs();
    f >>= pd_buf;
    setValid();
    return precise;
  }
  catch (CORBA::DATA_CONVERSION& ex) {
    if (ex.minor() == DATA_CONVERSION_RangeError) {
      // Too many digits
      throw DynamicAny::DynAny::InvalidValue();
    }
    else
      throw DynamicAny::DynAny::TypeMismatch();
  }
  return 0; // Never reach here
}

///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynFixedImpl::NP_nodetype() const
{
  return dt_fixed;
}

void
DynFixedImpl::set_to_initial_value()
{
  set_value("0");
}

void
DynFixedImpl::_NP_incrRefCount()
{
  DynAnyImplBase::_NP_incrRefCount();
}

void
DynFixedImpl::_NP_decrRefCount()
{
  DynAnyImplBase::_NP_decrRefCount();
}


void*
DynFixedImpl::_ptrToObjRef(const char* repoId)
{
  if( omni::ptrStrMatch(repoId, DynAnyImpl::_PD_repoId) )
    return (DynAnyImpl*) this;
  
  if( omni::ptrStrMatch(repoId, DynAnyImplBase::_PD_repoId) )
    return (DynAnyImplBase*) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynFixed::_PD_repoId) )
    return (DynamicAny::DynFixed_ptr) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
    return (DynamicAny::DynAny_ptr) this;
  
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


//////////////////////////////////////////////////////////////////////
///////////////////////////// DynEnumImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

DynEnumImpl::DynEnumImpl(TypeCode_base* tc, CORBA::Boolean is_root)
  : DynAnyImpl(tc, dt_enum, is_root)
{
}

DynEnumImpl::~DynEnumImpl() {}

//////////////////////
// public interface //
//////////////////////

DynamicAny::DynAny_ptr
DynEnumImpl::copy()
{
  CHECK_NOT_DESTROYED;

  DynEnumImpl* da = new DynEnumImpl(TypeCode_collector::duplicateRef(tc()),
				    DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(DynamicAny::DynAny::TypeMismatch&) {
    OMNIORB_ASSERT(0);
    // This should never happen since the types always match -- they
    // are the same TypeCode.
  }
  catch(...) {
    da->_NP_decrRefCount();
    throw;
  }
  return da;
}


char*
DynEnumImpl::get_as_string()
{
  CHECK_NOT_DESTROYED;

  CORBA::ULong val;
  {
    if( !isValid() )
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);
				    
    pd_buf.rewindInputPtr();
    val <<= pd_buf;
  }

  if( val >= actualTc()->NP_member_count() )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);

  return CORBA::string_dup(actualTc()->NP_member_name(val));
}


void
DynEnumImpl::set_as_string(const char* value)
{
  CHECK_NOT_DESTROYED;

  if( !value )  OMNIORB_THROW(BAD_PARAM,
			      BAD_PARAM_NullStringUnexpected,
			      CORBA::COMPLETED_NO);

  CORBA::Long index = actualTc()->NP_member_index(value);
  if( index < 0 )  throw DynamicAny::DynAny::InvalidValue();

  pd_buf.rewindPtrs();
  CORBA::ULong(index) >>= pd_buf;
  setValid();
}


CORBA::ULong
DynEnumImpl::get_as_ulong()
{
  CHECK_NOT_DESTROYED;

  CORBA::ULong val;
  {
    if( !isValid() )
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);
    pd_buf.rewindInputPtr();
    val <<= pd_buf;
  }

  if( val >= actualTc()->NP_member_count() )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);

  return val;
}


void
DynEnumImpl::set_as_ulong(CORBA::ULong value)
{
  CHECK_NOT_DESTROYED;

  if( value >= actualTc()->NP_member_count() )
    throw DynamicAny::DynAny::InvalidValue();

  pd_buf.rewindPtrs();
  value >>= pd_buf;
  setValid();
}

///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynEnumImpl::NP_nodetype() const
{
  return dt_enum;
}

void
DynEnumImpl::set_to_initial_value()
{
  set_as_ulong(0);
}

void
DynEnumImpl::_NP_incrRefCount()
{
  DynAnyImplBase::_NP_incrRefCount();
}

void
DynEnumImpl::_NP_decrRefCount()
{
  DynAnyImplBase::_NP_decrRefCount();
}

void*
DynEnumImpl::_ptrToObjRef(const char* repoId)
{
  if( omni::ptrStrMatch(repoId, DynAnyImpl::_PD_repoId) )
    return (DynAnyImpl*) this;
  
  if( omni::ptrStrMatch(repoId, DynAnyImplBase::_PD_repoId) )
    return (DynAnyImplBase*) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynEnum::_PD_repoId) )
    return (DynamicAny::DynEnum_ptr) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
    return (DynamicAny::DynAny_ptr) this;
  
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}



//////////////////////////////////////////////////////////////////////
////////////////////////// DynAnyConstrBase //////////////////////////
//////////////////////////////////////////////////////////////////////

DynAnyConstrBase::DynAnyConstrBase(TypeCode_base* tc, int nodetype,
				   CORBA::Boolean is_root)
  : DynAnyImplBase(tc, nodetype, is_root)
{
  pd_n_components = 0;
  pd_n_in_buf = 0;
  pd_n_really_in_buf = 0;
  pd_first_in_comp = 0;
  pd_curr_index = -1;
  pd_read_index = 0;
}


DynAnyConstrBase::~DynAnyConstrBase()
{
  for( unsigned i = pd_first_in_comp; i < pd_n_components; i++ )
    pd_components[i]->_NP_decrRefCount();
}

//////////////////////
// public interface //
//////////////////////

void
DynAnyConstrBase::assign(DynamicAny::DynAny_ptr da)
{
  CHECK_NOT_DESTROYED;

  if ( !DynamicAny::DynAny::PR_is_valid(da) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);

  if( CORBA::is_nil(da) )
    throw DynamicAny::DynAny::InvalidValue();

  DynAnyImplBase* daib = ToDynAnyImplBase(da);
  if( !tc()->equivalent(daib->tc()) )
    throw DynamicAny::DynAny::TypeMismatch();

  // We do the copy via an intermediate buffer.

  cdrMemoryStream buf;

  if( !daib->copy_to(buf) )
    throw DynamicAny::DynAny::InvalidValue();

  if( !copy_from(buf) )
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "DynAnyConstrBase::assign() - copy_from failed unexpectedly");
}

CORBA::Boolean
DynAnyConstrBase::equal(DynamicAny::DynAny_ptr da)
{
  CHECK_NOT_DESTROYED;

  if ( !DynamicAny::DynAny::PR_is_valid(da) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);

  if( CORBA::is_nil(da) )
    return 0;

  DynAnyImplBase* daib = ToDynAnyImplBase(da);
  if( !tc()->equivalent(daib->tc()) )
    return 0;

  DynAnyConstrBase* dacb = ToDynAnyConstrBase(da);

  createComponent(0);
  dacb->createComponent(0);

  for (unsigned i=0; i < pd_n_components; i++) {
    if (!pd_components[i]->equal(dacb->pd_components[i]))
      return 0;
  }
  return 1;
}


void
DynAnyConstrBase::insert_boolean(CORBA::Boolean value)
{
  CHECK_NOT_DESTROYED;
  writeCurrent(CORBA::tk_boolean).marshalBoolean(value);
}


void
DynAnyConstrBase::insert_octet(CORBA::Octet value)
{
  CHECK_NOT_DESTROYED;
  writeCurrent(CORBA::tk_octet).marshalOctet(value);
}


void
DynAnyConstrBase::insert_char(CORBA::Char value)
{
  CHECK_NOT_DESTROYED;
  writeCurrent(CORBA::tk_char).marshalChar(value);
}


void
DynAnyConstrBase::insert_short(CORBA::Short value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_short);
}


void
DynAnyConstrBase::insert_ushort(CORBA::UShort value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_ushort);
}


void
DynAnyConstrBase::insert_long(CORBA::Long value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_long);
}


void
DynAnyConstrBase::insert_ulong(CORBA::ULong value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_ulong);
}

#ifndef NO_FLOAT
void
DynAnyConstrBase::insert_float(CORBA::Float value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_float);
}


void
DynAnyConstrBase::insert_double(CORBA::Double value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_double);
}

#endif

void
DynAnyConstrBase::insert_string(const char* value)
{
  CHECK_NOT_DESTROYED;
  if( !value || pd_curr_index < 0 )
    throw DynamicAny::DynAny::InvalidValue();

  TypeCode_base* tc =
    (TypeCode_base*) TypeCode_base::NP_expand(nthComponentTC(pd_curr_index));

  if( tc->NP_kind() != CORBA::tk_string )
    throw DynamicAny::DynAny::TypeMismatch();

  CORBA::ULong maxlen = tc->NP_length();
  cdrMemoryStream& buf = writeCurrent(CORBA::tk_string);
  try {
    buf.marshalString(value,maxlen);
  }
  catch (CORBA::MARSHAL& ex) {
    if (ex.minor() == MARSHAL_StringIsTooLong)
      throw DynamicAny::DynAny::InvalidValue();
    else
      throw;
  }
}


void
DynAnyConstrBase::insert_reference(CORBA::Object_ptr value)
{
  CHECK_NOT_DESTROYED;
  if ( !CORBA::Object::_PR_is_valid(value) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidObjectRef, CORBA::COMPLETED_NO);

  CORBA::Object::_marshalObjRef(value, writeCurrent(CORBA::tk_objref));
}


void
DynAnyConstrBase::insert_typecode(CORBA::TypeCode_ptr value)
{
  CHECK_NOT_DESTROYED;

  if ( !CORBA::TypeCode::PR_is_valid(value) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);

  if( CORBA::is_nil(value) )  throw DynamicAny::DynAny::InvalidValue();

  CORBA::TypeCode::marshalTypeCode(value, writeCurrent(CORBA::tk_TypeCode));
}


#ifdef HAS_LongLong
void
DynAnyConstrBase::insert_longlong(CORBA::LongLong value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_longlong);
}

void
DynAnyConstrBase::insert_ulonglong(CORBA::ULongLong value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_ulonglong);
}
#endif

#ifdef HAS_LongDouble
void
DynAnyConstrBase::insert_longdouble(CORBA::LongDouble value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_longdouble);
}
#endif


void
DynAnyConstrBase::insert_wchar(CORBA::WChar value)
{
  CHECK_NOT_DESTROYED;
  writeCurrent(CORBA::tk_wchar).marshalWChar(value);
}

void
DynAnyConstrBase::insert_wstring(const CORBA::WChar* value)
{
  CHECK_NOT_DESTROYED;
  if( !value || pd_curr_index < 0 )
    throw DynamicAny::DynAny::InvalidValue();

  TypeCode_base* tc =
    (TypeCode_base*) TypeCode_base::NP_expand(nthComponentTC(pd_curr_index));

  if( tc->NP_kind() != CORBA::tk_wstring )
    throw DynamicAny::DynAny::TypeMismatch();

  CORBA::ULong maxlen = tc->NP_length();
  cdrMemoryStream& buf = writeCurrent(CORBA::tk_wstring);
  try {
    buf.marshalWString(value,maxlen);
  }
  catch (CORBA::MARSHAL& ex) {
    if (ex.minor() == MARSHAL_WStringIsTooLong)
      throw DynamicAny::DynAny::InvalidValue();
    else
      throw;
  }
}


void
DynAnyConstrBase::insert_any(const CORBA::Any& value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_any);
}

void
DynAnyConstrBase::insert_dyn_any(DynamicAny::DynAny_ptr value)
{
  CHECK_NOT_DESTROYED;
  CORBA::Any_var a(to_any());
  a.in() >>= writeCurrent(CORBA::tk_any);
}




CORBA::Boolean
DynAnyConstrBase::get_boolean()
{
  CHECK_NOT_DESTROYED;
  return readCurrent(CORBA::tk_boolean).unmarshalBoolean();
}


CORBA::Octet
DynAnyConstrBase::get_octet()
{
  CHECK_NOT_DESTROYED;
  return readCurrent(CORBA::tk_octet).unmarshalOctet();
}


CORBA::Char
DynAnyConstrBase::get_char()
{
  CHECK_NOT_DESTROYED;
  return readCurrent(CORBA::tk_char).unmarshalChar();
}


CORBA::Short
DynAnyConstrBase::get_short()
{
  CHECK_NOT_DESTROYED;
  CORBA::Short value;
  value <<= readCurrent(CORBA::tk_short);
  return value;
}


CORBA::UShort
DynAnyConstrBase::get_ushort()
{
  CHECK_NOT_DESTROYED;
  CORBA::UShort value;
  value <<= readCurrent(CORBA::tk_ushort);
  return value;
}


CORBA::Long
DynAnyConstrBase::get_long()
{
  CHECK_NOT_DESTROYED;
  CORBA::Long value;
  value <<= readCurrent(CORBA::tk_long);
  return value;
}


CORBA::ULong
DynAnyConstrBase::get_ulong()
{
  CHECK_NOT_DESTROYED;
  CORBA::ULong value;
  value <<= readCurrent(CORBA::tk_ulong);
  return value;
}

#ifndef NO_FLOAT
CORBA::Float
DynAnyConstrBase::get_float()
{
  CHECK_NOT_DESTROYED;
  CORBA::Float value;
  value <<= readCurrent(CORBA::tk_float);
  return value;
}


CORBA::Double
DynAnyConstrBase::get_double()
{
  CHECK_NOT_DESTROYED;
  CORBA::Double value;
  value <<= readCurrent(CORBA::tk_double);
  return value;
}

#endif

char*
DynAnyConstrBase::get_string()
{
  CHECK_NOT_DESTROYED;

  cdrMemoryStream& buf = readCurrent(CORBA::tk_string);

  TypeCode_base* tc = (TypeCode_base*)TypeCode_base::NP_expand(nthComponentTC(pd_curr_index));
  CORBA::ULong maxlen = tc->NP_length();
  try {
    char* value = buf.unmarshalString(maxlen);
    return value;
  }
  catch (CORBA::MARSHAL& ex) {
    if (ex.minor() == MARSHAL_StringIsTooLong)
      throw DynamicAny::DynAny::InvalidValue();
    else
      throw;
  }
  return 0;
}


CORBA::Object_ptr
DynAnyConstrBase::get_reference()
{
  CHECK_NOT_DESTROYED;
  return CORBA::Object::_unmarshalObjRef(readCurrent(CORBA::tk_objref));
}


CORBA::TypeCode_ptr
DynAnyConstrBase::get_typecode()
{
  CHECK_NOT_DESTROYED;
  return CORBA::TypeCode::unmarshalTypeCode(readCurrent(CORBA::tk_TypeCode));
}

#ifdef HAS_LongLong
CORBA::LongLong
DynAnyConstrBase::get_longlong()
{
  CHECK_NOT_DESTROYED;
  CORBA::LongLong value;
  value <<= readCurrent(CORBA::tk_longlong);
  return value;
}


CORBA::ULongLong
DynAnyConstrBase::get_ulonglong()
{
  CHECK_NOT_DESTROYED;
  CORBA::ULongLong value;
  value <<= readCurrent(CORBA::tk_ulonglong);
  return value;
}
#endif


#ifdef HAS_LongDouble
CORBA::Double
DynAnyConstrBase::get_longdouble()
{
  CHECK_NOT_DESTROYED;
  CORBA::LongDouble value;
  value <<= readCurrent(CORBA::tk_longdouble);
  return value;
}
#endif

CORBA::WChar
DynAnyConstrBase::get_wchar()
{
  CHECK_NOT_DESTROYED;
  return readCurrent(CORBA::tk_wchar).unmarshalWChar();
}

CORBA::WChar*
DynAnyConstrBase::get_wstring()
{
  CHECK_NOT_DESTROYED;
  cdrMemoryStream& buf = readCurrent(CORBA::tk_wstring);

  TypeCode_base* tc = (TypeCode_base*)TypeCode_base::NP_expand(nthComponentTC(pd_curr_index));
  CORBA::ULong maxlen = tc->NP_length();
  CORBA::WChar* value = buf.unmarshalWString(maxlen);
  return value;
}


CORBA::Any*
DynAnyConstrBase::get_any()
{
  CHECK_NOT_DESTROYED;
  CORBA::Any* value = new CORBA::Any();
  try {
    *value <<= readCurrent(CORBA::tk_any);
  }
  catch(...) {
    delete value;
    throw;
  }
  return value;
}

DynamicAny::DynAny_ptr
DynAnyConstrBase::get_dyn_any()
{
  CHECK_NOT_DESTROYED;
  CORBA::Any value;
  value <<= readCurrent(CORBA::tk_any);
  return factory_create_dyn_any(value);
}

//
// CORBA 2.5 sequence operations
//

void
DynAnyConstrBase::insert_boolean_seq(CORBA::BooleanSeq& value)
{
  CHECK_NOT_DESTROYED;

  if (prepareSequenceWrite(CORBA::tk_boolean, value.length()) == SEQ_HERE)
    pd_buf.put_octet_array((_CORBA_Octet*)value.NP_data(), value.length());
  else
    getCurrent()->insert_boolean_seq(value);
}

void
DynAnyConstrBase::insert_octet_seq(CORBA::OctetSeq& value)
{
  CHECK_NOT_DESTROYED;

  if (prepareSequenceWrite(CORBA::tk_octet, value.length()) == SEQ_HERE)
    pd_buf.put_octet_array((_CORBA_Octet*)value.NP_data(), value.length());
  else
    getCurrent()->insert_octet_seq(value);
}

void
DynAnyConstrBase::insert_char_seq(CORBA::CharSeq& value)
{
  CHECK_NOT_DESTROYED;

  if (prepareSequenceWrite(CORBA::tk_char, value.length()) == SEQ_HERE) {
    CORBA::Char* data = value.NP_data();
    for (CORBA::ULong i=0; i < value.length(); i++)
      pd_buf.marshalChar(data[i]);
  }
  else
    getCurrent()->insert_char_seq(value);
}

void
DynAnyConstrBase::insert_wchar_seq(CORBA::WCharSeq& value)
{
  CHECK_NOT_DESTROYED;

  if (prepareSequenceWrite(CORBA::tk_wchar, value.length()) == SEQ_HERE) {
    CORBA::WChar* data = value.NP_data();
    for (CORBA::ULong i=0; i < value.length(); i++)
      pd_buf.marshalWChar(data[i]);
  }
  else
    getCurrent()->insert_wchar_seq(value);
}


// Too lazy to type all these out...

#define INSERT_SEQ_OP(lcname, ucname, align, size) \
void \
DynAnyConstrBase::insert_##lcname##_seq(CORBA::##ucname##Seq& value) \
{ \
  CHECK_NOT_DESTROYED; \
\
  if (prepareSequenceWrite(CORBA::tk_##lcname, value.length()) == SEQ_HERE) { \
    if (!pd_buf.marshal_byte_swap()) { \
      pd_buf.put_octet_array((_CORBA_Octet*)value.NP_data(), \
			     value.length() * size, omni::ALIGN_##align); \
    } \
    else { \
      _CORBA_##ucname* data = value.NP_data(); \
      for (CORBA::ULong i=0; i < value.length(); i++) \
	data[i] >>= pd_buf; \
    } \
  } \
  else \
    getCurrent()->insert_##lcname##_seq(value); \
}

INSERT_SEQ_OP(short,      Short,      2, 2)
INSERT_SEQ_OP(ushort,     UShort,     2, 2)
INSERT_SEQ_OP(long,       Long,       4, 4)
INSERT_SEQ_OP(ulong,      ULong,      4, 4)
#ifndef NO_FLOAT
INSERT_SEQ_OP(float,      Float,      4, 4)
INSERT_SEQ_OP(double,     Double,     8, 8)
#endif
#ifdef HAS_LongLong
INSERT_SEQ_OP(longlong,   LongLong,   8, 8)
INSERT_SEQ_OP(ulonglong,  ULongLong,  8, 8)
#endif
#ifdef HAS_LongDouble
INSERT_SEQ_OP(longdouble, LongDouble, 8, 16)
#endif

#undef INSERT_SEQ_OP



CORBA::BooleanSeq*
DynAnyConstrBase::get_boolean_seq()
{
  CHECK_NOT_DESTROYED;

  if (prepareSequenceRead(CORBA::tk_boolean) == SEQ_HERE) {
    if (pd_n_in_buf < pd_first_in_comp)
      throw DynamicAny::DynAny::InvalidValue();

    CORBA::BooleanSeq_var seq = new CORBA::BooleanSeq(pd_n_components);
    seq->length(pd_n_components);
    CORBA::Boolean* data = seq->NP_data();

    if (pd_first_in_comp > 0) {
      pd_buf.rewindInputPtr();
      pd_buf.get_octet_array((_CORBA_Octet*)data, pd_first_in_comp);
    }
    for (unsigned i = pd_first_in_comp; i < pd_n_components; i++)
      data[i] = pd_components[i]->get_boolean();
    
    return seq._retn();
  }
  else {
    return getCurrent()->get_boolean_seq();
  }
}

CORBA::OctetSeq*
DynAnyConstrBase::get_octet_seq()
{
  CHECK_NOT_DESTROYED;

  if (prepareSequenceRead(CORBA::tk_octet) == SEQ_HERE) {
    if (pd_n_in_buf < pd_first_in_comp)
      throw DynamicAny::DynAny::InvalidValue();

    CORBA::OctetSeq_var seq = new CORBA::OctetSeq(pd_n_components);
    seq->length(pd_n_components);
    CORBA::Octet* data = seq->NP_data();

    if (pd_first_in_comp > 0) {
      pd_buf.rewindInputPtr();
      pd_buf.get_octet_array((_CORBA_Octet*)data, pd_first_in_comp);
    }
    for (unsigned i = pd_first_in_comp; i < pd_n_components; i++)
      data[i] = pd_components[i]->get_octet();
    
    return seq._retn();
  }
  else {
    return getCurrent()->get_octet_seq();
  }
}

CORBA::CharSeq*
DynAnyConstrBase::get_char_seq()
{
  CHECK_NOT_DESTROYED;

  if (prepareSequenceRead(CORBA::tk_char) == SEQ_HERE) {
    if (pd_n_in_buf < pd_first_in_comp)
      throw DynamicAny::DynAny::InvalidValue();

    CORBA::CharSeq_var seq = new CORBA::CharSeq(pd_n_components);
    seq->length(pd_n_components);
    CORBA::Char* data = seq->NP_data();

    if (pd_first_in_comp > 0) {
      pd_buf.rewindInputPtr();

      for (unsigned i=0; i < pd_first_in_comp; i++)
	data[i] = pd_buf.unmarshalChar();
    }
    for (unsigned i = pd_first_in_comp; i < pd_n_components; i++)
      data[i] = pd_components[i]->get_char();
    
    return seq._retn();
  }
  else {
    return getCurrent()->get_char_seq();
  }
}

CORBA::WCharSeq*
DynAnyConstrBase::get_wchar_seq()
{
  CHECK_NOT_DESTROYED;

  if (prepareSequenceRead(CORBA::tk_wchar) == SEQ_HERE) {
    if (pd_n_in_buf < pd_first_in_comp)
      throw DynamicAny::DynAny::InvalidValue();

    CORBA::WCharSeq_var seq = new CORBA::WCharSeq(pd_n_components);
    seq->length(pd_n_components);
    CORBA::WChar* data = seq->NP_data();

    if (pd_first_in_comp > 0) {
      pd_buf.rewindInputPtr();

      for (unsigned i=0; i < pd_first_in_comp; i++)
	data[i] = pd_buf.unmarshalWChar();
    }
    for (unsigned i = pd_first_in_comp; i < pd_n_components; i++)
      data[i] = pd_components[i]->get_wchar();
    
    return seq._retn();
  }
  else {
    return getCurrent()->get_wchar_seq();
  }
}

#define GET_SEQ_OP(lcname, ucname, align, size) \
CORBA::##ucname##Seq* \
DynAnyConstrBase::get_##lcname##_seq() \
{ \
  CHECK_NOT_DESTROYED; \
\
  if (prepareSequenceRead(CORBA::tk_##lcname) == SEQ_HERE) { \
    if (pd_n_in_buf < pd_first_in_comp) \
      throw DynamicAny::DynAny::InvalidValue(); \
\
    CORBA::##ucname##Seq_var seq = new CORBA::##ucname##Seq(pd_n_components); \
    seq->length(pd_n_components); \
    _CORBA_##ucname##* data = seq->NP_data(); \
\
    if (pd_first_in_comp > 0) { \
      pd_buf.rewindInputPtr(); \
      if (!pd_buf.unmarshal_byte_swap()) { \
	pd_buf.get_octet_array((_CORBA_Octet*)data, pd_first_in_comp * size, \
			       omni::ALIGN_##align); \
      } \
      else { \
	for (unsigned i=0; i < pd_first_in_comp; i++) \
	  data[i] <<= pd_buf; \
      } \
    } \
    for (unsigned i = pd_first_in_comp; i < pd_n_components; i++) \
      data[i] = pd_components[i]->get_octet(); \
\
    return seq._retn(); \
  } \
  else { \
    return getCurrent()->get_##lcname##_seq(); \
  } \
}

GET_SEQ_OP(short,      Short,      2, 2)
GET_SEQ_OP(ushort,     UShort,     2, 2)
GET_SEQ_OP(long,       Long,       4, 4)
GET_SEQ_OP(ulong,      ULong,      4, 4)
#ifndef NO_FLOAT
GET_SEQ_OP(float,      Float,      4, 4)
GET_SEQ_OP(double,     Double,     8, 8)
#endif
#ifdef HAS_LongLong
GET_SEQ_OP(longlong,   LongLong,   8, 8)
GET_SEQ_OP(ulonglong,  ULongLong,  8, 8)
#endif
#ifdef HAS_LongDouble
GET_SEQ_OP(longdouble, LongDouble, 8, 16)
#endif

#undef GET_SEQ_OP




CORBA::Boolean
DynAnyConstrBase::seek(CORBA::Long index)
{
  CHECK_NOT_DESTROYED;
  if( index < 0 || index >= (int)pd_n_components ) {
    pd_curr_index = -1;
    return 0;
  }
  else {
    pd_curr_index = index;
    return 1;
  }
}


void
DynAnyConstrBase::rewind()
{
  CHECK_NOT_DESTROYED;
  pd_curr_index = pd_n_components ? 0 : -1;
}


CORBA::Boolean
DynAnyConstrBase::next()
{
  CHECK_NOT_DESTROYED;
  if( pd_curr_index + 1 < (int)pd_n_components )  pd_curr_index++;
  else                                            pd_curr_index = -1;

  return pd_curr_index >= 0;
}


CORBA::ULong
DynAnyConstrBase::component_count()
{
  CHECK_NOT_DESTROYED;
  return pd_n_components;
}


DynamicAny::DynAny_ptr
DynAnyConstrBase::current_component()
{
  CHECK_NOT_DESTROYED;
  if( pd_n_components == 0 && actualTc()->NP_kind() != CORBA::tk_sequence )
    throw DynamicAny::DynAny::TypeMismatch();

  if( pd_curr_index < 0 )
    return DynamicAny::DynAny::_nil();

  DynAnyImplBase* da = getCurrent();
  da->_NP_incrRefCount();
  return da;
}




//////////////
// internal //
//////////////

void
DynAnyConstrBase::set_to_initial_value()
{
  if (pd_n_components > 0) {
    createComponent(0);
    for (unsigned i=0; i < pd_n_components; i++)
      pd_components[i]->set_to_initial_value();
  }
  pd_curr_index = (pd_n_components == 0) ? -1 : 0;
}

int
DynAnyConstrBase::copy_to(cdrMemoryStream& mbs)
{
  if( pd_n_in_buf != pd_first_in_comp )  return 0;

  cdrMemoryStream src(pd_buf, 1);
  pd_read_index = -1;

  unsigned i;
  try {
    // Copy the components in the buffer.
    for( i = 0; i < pd_n_in_buf; i++ ) {
      TypeCode_base* ctc = nthComponentTC(i);
      tcParser::copyStreamToStream(ctc, src, mbs);
    }
  }
  catch(CORBA::MARSHAL&) {
    return 0;
  }

  // The rest are in external components.
  for( ; i < pd_n_components; i++ ) {
    if( !pd_components[i]->copy_to(mbs) )
      return 0;
  }
  return 1;
}


int
DynAnyConstrBase::copy_from(cdrMemoryStream& mbs)
{
  pd_buf.rewindPtrs();
  pd_read_index = 0;

  unsigned i;
  try {
    // Copy components into the buffer.
    for( i = 0; i < pd_first_in_comp; i++ ) {
      TypeCode_base* ctc = nthComponentTC(i);
      tcParser::copyStreamToStream(ctc, mbs, pd_buf);
    }
  }
  catch(CORBA::MARSHAL&) {
    pd_buf.rewindPtrs();
    pd_n_in_buf = 0;
    pd_n_really_in_buf = 0;
    return 0;
  }

  // Copy into external components.
  for( ; i < pd_n_components; i++ ) {
    if( !pd_components[i]->copy_from(mbs) )
      return 0;
  }

  pd_n_really_in_buf = pd_n_in_buf = pd_first_in_comp;
  pd_curr_index = (pd_n_components == 0) ? -1 : 0;
  return 1;
}


void
DynAnyConstrBase::onDispose()
{
  for( unsigned i = pd_first_in_comp; i < pd_n_components; i++ )
    pd_components[i]->detach();
}


void
DynAnyConstrBase::setNumComponents(unsigned n)
{
  if( n == pd_n_components )  return;

  if( n < pd_n_components ) {
    // Detach any orphaned components stored in <pd_components>.
    unsigned start = pd_first_in_comp > n ? pd_first_in_comp : n;
    for( unsigned i = start; i < pd_n_components; i++ ) {
      pd_components[i]->detach();
      pd_components[i]->_NP_decrRefCount();
    }
    pd_components.reserve(n);
    DynAnyImplBase* tmp = 0;
    while( pd_components.size() < n )  pd_components.push_back(tmp);
    pd_n_components = n;
    if( n < pd_n_in_buf   )       pd_n_in_buf = n;
    if( (int)n < pd_read_index )  pd_read_index = -1;
    if( pd_first_in_comp > pd_n_components )
      pd_first_in_comp = pd_n_components;
  }
  else {
    // Add new components.
    pd_components.reserve(n);
    DynAnyImplBase* tmp = 0;
    while( pd_components.size() < n )  pd_components.push_back(tmp);
    unsigned old_n_components = pd_n_components;
    pd_n_components = n;

    OMNIORB_ASSERT(pd_first_in_comp <= old_n_components);

    // Add in the new components
    for( unsigned i = old_n_components; i < pd_n_components; i++ ) {
      CORBA::TypeCode_ptr tc =
	CORBA::TypeCode::_duplicate(nthComponentTC(i));
      pd_components[i] = internal_create_dyn_any(ToTcBase(tc), DYNANY_CHILD);
      pd_components[i]->set_to_initial_value();
    }
  }
}


void
DynAnyConstrBase::createComponent(unsigned n)
{
  if( n >= pd_first_in_comp )  return;

  // Copy components out of the buffer.
  unsigned i;
  for( i = n; i < pd_n_in_buf; i++ ) {
    CORBA::TypeCode_ptr tc = CORBA::TypeCode::_duplicate(nthComponentTC(i));
    DynAnyImplBase* da = internal_create_dyn_any(ToTcBase(tc), DYNANY_CHILD);
    if( pd_read_index != (int)i )  seekTo(i);
    if( !da->copy_from(pd_buf) ) {
      throw omniORB::fatalException(__FILE__,__LINE__,
	 "DynAnyConstrBase::createComponent() - copy_from() failed");
    }
    pd_read_index++;
    pd_components[i] = da;
  }

  // Create uninitialised components for those not yet inserted.
  for( ; i < pd_first_in_comp; i++ ) {
    CORBA::TypeCode_ptr tc = CORBA::TypeCode::_duplicate(nthComponentTC(i));
    pd_components[i] = internal_create_dyn_any(ToTcBase(tc), DYNANY_CHILD);
  }

  pd_first_in_comp = n;
  if( n < pd_n_in_buf )  pd_n_in_buf = n;
}

void
DynAnyConstrBase::seekTo(unsigned n)
{
  // For safety we assume that this would not have been called
  // unless a seek really is needed - so we do not just return
  // if( pd_read_index == i ).

  if( n >= pd_n_in_buf ) {
    throw omniORB::fatalException(__FILE__,__LINE__,
	"DynAnyConstrBase::seekTo() - <n> out of bounds");
  }

  pd_buf.rewindInputPtr();

  for( unsigned i = 0; i < n; i++ ) {
    TypeCode_base* ctc = nthComponentTC(i);
    try {
      tcParser::skip(ctc, pd_buf);
    }
    catch(CORBA::MARSHAL&) {
      throw omniORB::fatalException(__FILE__,__LINE__,
	"DynAnyConstrBase::seekTo() - unexpected exception");
    }
  }
  pd_read_index = n;
}


int
DynAnyConstrBase::component_to_any(unsigned i, CORBA::Any& a)
{
  a.replace(nthComponentTC(i), 0);

  if( i < pd_n_in_buf ) {
    AnyP* anyp = (AnyP*)a.NP_pd();
    if( pd_read_index != (int)i )  seekTo(i);
    try {
      tcParser::copyStreamToStream(anyp->getTC(), pd_buf,
				   anyp->getWRableMemoryStream());
    }
    catch(CORBA::MARSHAL&) {
      throw omniORB::fatalException(__FILE__,__LINE__,
	 "DynAnyConstrBase::component_to_any() - unexpected exception");
    }
    pd_read_index++;
    return 1;
  }
  else if( i >= pd_first_in_comp ) {
    cdrMemoryStream& buf = ((AnyP*)a.NP_pd())->getWRableMemoryStream();
    return pd_components[i]->copy_to(buf);
  }
  else
    return 0;
}


int
DynAnyConstrBase::component_from_any(unsigned i, const CORBA::Any& a)
{
  CORBA::TypeCode_var tc = a.type();
  if( !tc->equivalent(nthComponentTC(i)) )  return 0;

  if( canAppendComponent(i) ) {
    AnyP* anyp = (AnyP*)a.NP_pd();
    try {
      cdrMemoryStream src(anyp->theMemoryStream(), 1);
      tcParser::copyStreamToMemStream_flush(anyp->getTC(), src, pd_buf);
    }
    catch(CORBA::MARSHAL&) {
      // <pd_buf> may have been partly written to, so we can't
      // append to it any more. Appends are only allowed if
      // pd_n_in_buf == pd_n_really_in_buf so ...
      pd_n_really_in_buf++;
      return 0;
    }
    pd_n_in_buf++;
    pd_n_really_in_buf++;
    return 1;
  }

  if( i < pd_first_in_comp )  createComponent(i);

  cdrMemoryStream buf(((AnyP*)a.NP_pd())->theMemoryStream(), 1);
  return pd_components[i]->copy_from(buf);
}





//////////////////////////////////////////////////////////////////////
//////////////////////////// DynStructImpl ///////////////////////////
//////////////////////////////////////////////////////////////////////

DynStructImpl::DynStructImpl(TypeCode_base* tc, CORBA::Boolean is_root)
  : DynAnyConstrBase(tc, dt_struct, is_root)
{
  setNumComponents(actualTc()->NP_member_count());
}


DynStructImpl::~DynStructImpl()
{
}

//////////////////////
// public interface //
//////////////////////

DynamicAny::DynAny_ptr
DynStructImpl::copy()
{
  CHECK_NOT_DESTROYED;
  DynStructImpl* da = new DynStructImpl(TypeCode_collector::duplicateRef(tc()),
					DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(...) {
    da->_NP_decrRefCount();
    throw;
  }
  return da;
}

char*
DynStructImpl::current_member_name()
{
  CHECK_NOT_DESTROYED;
  if( pd_n_components == 0 ) throw DynamicAny::DynAny::TypeMismatch();    
  if( pd_curr_index < 0 )    throw DynamicAny::DynAny::InvalidValue();

  return CORBA::string_dup(actualTc()->NP_member_name(pd_curr_index));
}


CORBA::TCKind
DynStructImpl::current_member_kind()
{
  CHECK_NOT_DESTROYED;
  if( pd_n_components == 0 ) throw DynamicAny::DynAny::TypeMismatch();    
  if( pd_curr_index < 0 )    throw DynamicAny::DynAny::InvalidValue();

  return actualTc()->NP_member_type(pd_curr_index)->NP_kind();
}


DynamicAny::NameValuePairSeq*
DynStructImpl::get_members()
{
  CHECK_NOT_DESTROYED;
  DynamicAny::NameValuePairSeq* nvps = new DynamicAny::NameValuePairSeq();

  nvps->length(pd_n_components);

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    (*nvps)[i].id = CORBA::string_dup(actualTc()->NP_member_name(i));
    if( !component_to_any(i, (*nvps)[i].value) ) {
      delete nvps;
      OMNIORB_THROW(BAD_INV_ORDER, BAD_INV_ORDER_DynAnyNotInitialised,
		    CORBA::COMPLETED_NO);
    }
  }
  return nvps;
}


void
DynStructImpl::set_members(const DynamicAny::NameValuePairSeq& nvps)
{
  CHECK_NOT_DESTROYED;
  if( nvps.length() != pd_n_components )
    throw DynamicAny::DynAny::InvalidValue();

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    if( ((const char*)(nvps[i].id))[0] != '\0' &&
	strcmp((const char*)(nvps[i].id), actualTc()->NP_member_name(i)) )
      throw DynamicAny::DynAny::TypeMismatch();

    if( !component_from_any(i, nvps[i].value) )
      throw DynamicAny::DynAny::TypeMismatch();
  }
  pd_curr_index = (pd_n_components == 0) ? -1 : 0;
}


DynamicAny::NameDynAnyPairSeq*
DynStructImpl::get_members_as_dyn_any()
{
  CHECK_NOT_DESTROYED;

  if (pd_n_in_buf != pd_first_in_comp)
    OMNIORB_THROW(BAD_INV_ORDER, BAD_INV_ORDER_DynAnyNotInitialised,
		  CORBA::COMPLETED_NO);

  DynamicAny::NameDynAnyPairSeq* nvps = new DynamicAny::NameDynAnyPairSeq();

  nvps->length(pd_n_components);

  createComponent(0);
  // All components are now in the buffer

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    (*nvps)[i].id = CORBA::string_dup(actualTc()->NP_member_name(i));
    pd_components[i]->_NP_incrRefCount();
    (*nvps)[i].value = pd_components[i];
  }
  return nvps;
}

void
DynStructImpl::set_members_as_dyn_any(const DynamicAny::NameDynAnyPairSeq& nvps)
{
  CHECK_NOT_DESTROYED;

  if( nvps.length() != pd_n_components )
    throw DynamicAny::DynAny::InvalidValue();

  pd_n_in_buf = 0;
  pd_first_in_comp = 0;

  CORBA::TypeCode_var tc;

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    if( ((const char*)(nvps[i].id))[0] != '\0' &&
	strcmp((const char*)(nvps[i].id), actualTc()->NP_member_name(i)) )
      throw DynamicAny::DynAny::TypeMismatch();

    tc = nvps[i].value->type();
    if( !tc->equivalent(nthComponentTC(i)) ) {
      pd_first_in_comp = pd_n_components;
      throw DynamicAny::DynAny::TypeMismatch();
    }
    pd_components[i] = ToDynAnyImplBase(nvps[i].value);
    pd_components[i]->_NP_incrRefCount();
  }
  pd_curr_index = (pd_n_components == 0) ? -1 : 0;
}




///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynStructImpl::NP_nodetype() const
{
  return dt_struct;
}


//////////////
// internal //
//////////////

TypeCode_base*
DynStructImpl::nthComponentTC(unsigned n)
{
  if( n >= pd_n_components )
    throw omniORB::fatalException(__FILE__,__LINE__,
		    "DynStructImpl::nthComponentTC() - n out of bounds");

  return actualTc()->NP_member_type(n);
}


DynAnyConstrBase::SeqLocation
DynStructImpl::prepareSequenceWrite(CORBA::TCKind kind, CORBA::ULong len)
{
  // Note that we ignore then length here. When the insert function
  // tries to call insert on the sub-component, that call will check
  // the length.

  const TypeCode_base* tc = actualTc();

  if (pd_curr_index < 0)
    throw DynamicAny::DynAny::InvalidValue();

  const TypeCode_base* ctc;
  ctc = TypeCode_base::NP_expand(nthComponentTC(pd_curr_index));
  CORBA::TCKind k = ctc->NP_kind();

  if ((k == CORBA::tk_sequence || k == CORBA::tk_array) &&
      TypeCode_base::NP_expand(ctc->NP_content_type())->NP_kind() == kind) {
    return SEQ_COMPONENT;
  }
  throw DynamicAny::DynAny::TypeMismatch();
#ifdef NEED_DUMMY_RETURN
  return SEQ_COMPONENT;
#endif
}

DynAnyConstrBase::SeqLocation
DynStructImpl::prepareSequenceRead(CORBA::TCKind kind)
{
  const TypeCode_base* tc = actualTc();

  if (pd_curr_index < 0)
    throw DynamicAny::DynAny::InvalidValue();

  const TypeCode_base* ctc;
  ctc = TypeCode_base::NP_expand(nthComponentTC(pd_curr_index));
  CORBA::TCKind k = ctc->NP_kind();

  if ((k == CORBA::tk_sequence || k == CORBA::tk_array) &&
      TypeCode_base::NP_expand(ctc->NP_content_type())->NP_kind() == kind) {
    return SEQ_COMPONENT;
  }
  throw DynamicAny::DynAny::TypeMismatch();
#ifdef NEED_DUMMY_RETURN
  return SEQ_COMPONENT;
#endif
}


void
DynStructImpl::_NP_incrRefCount()
{
  DynAnyImplBase::_NP_incrRefCount();
}

void
DynStructImpl::_NP_decrRefCount()
{
  DynAnyImplBase::_NP_decrRefCount();
}

void*
DynStructImpl::_ptrToObjRef(const char* repoId)
{
  if( omni::ptrStrMatch(repoId, DynAnyConstrBase::_PD_repoId) )
    return (DynAnyConstrBase*) this;
  
  if( omni::ptrStrMatch(repoId, DynAnyImplBase::_PD_repoId) )
    return (DynAnyImplBase*) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynStruct::_PD_repoId) )
    return (DynamicAny::DynStruct_ptr) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
    return (DynamicAny::DynAny_ptr) this;
  
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}



//////////////////////////////////////////////////////////////////////
/////////////////////////// DynUnionDisc /////////////////////////////
//////////////////////////////////////////////////////////////////////

DynUnionDisc::~DynUnionDisc()
{
}


void
DynUnionDisc::assign(DynamicAny::DynAny_ptr da)
{
  DynAnyImpl::assign(da);
  if( pd_union )  pd_union->discriminatorHasChanged();
}

void
DynUnionDisc::insert_boolean(CORBA::Boolean value)
{
  CHECK_NOT_DESTROYED;
  doWrite(CORBA::tk_boolean).marshalBoolean(value);
  if( pd_union )  pd_union->discriminatorHasChanged();
}


void
DynUnionDisc::insert_octet(CORBA::Octet value)
{
  CHECK_NOT_DESTROYED;
  // Not a legal discriminator type.
  throw DynamicAny::DynAny::InvalidValue();
}


void
DynUnionDisc::insert_char(CORBA::Char value)
{
  CHECK_NOT_DESTROYED;
  doWrite(CORBA::tk_char).marshalChar(value);
  if( pd_union )  pd_union->discriminatorHasChanged();
}


void
DynUnionDisc::insert_short(CORBA::Short value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_short);
  if( pd_union )  pd_union->discriminatorHasChanged();
}


void
DynUnionDisc::insert_ushort(CORBA::UShort value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_ushort);
  if( pd_union )  pd_union->discriminatorHasChanged();
}


void
DynUnionDisc::insert_long(CORBA::Long value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_long);
  if( pd_union )  pd_union->discriminatorHasChanged();
}


void
DynUnionDisc::insert_ulong(CORBA::ULong value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_ulong);
  if( pd_union )  pd_union->discriminatorHasChanged();
}

#ifndef NO_FLOAT
void
DynUnionDisc::insert_float(CORBA::Float value)
{
  CHECK_NOT_DESTROYED;
  // Not a legal discriminator type.
  throw DynamicAny::DynAny::InvalidValue();
}


void
DynUnionDisc::insert_double(CORBA::Double value)
{
  CHECK_NOT_DESTROYED;
  // Not a legal discriminator type.
  throw DynamicAny::DynAny::InvalidValue();
}

#endif

void
DynUnionDisc::insert_string(const char* value)
{
  CHECK_NOT_DESTROYED;
  // Not a legal discriminator type.
  throw DynamicAny::DynAny::InvalidValue();
}


void
DynUnionDisc::insert_reference(CORBA::Object_ptr value)
{
  CHECK_NOT_DESTROYED;
  // Not a legal discriminator type.
  throw DynamicAny::DynAny::InvalidValue();
}


void
DynUnionDisc::insert_typecode(CORBA::TypeCode_ptr value)
{
  CHECK_NOT_DESTROYED;
  // Not a legal discriminator type.
  throw DynamicAny::DynAny::InvalidValue();
}


#ifdef HAS_LongLong
void
DynUnionDisc::insert_longlong(CORBA::LongLong value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_longlong);
  if( pd_union )  pd_union->discriminatorHasChanged();
}


void
DynUnionDisc::insert_ulonglong(CORBA::ULongLong value)
{
  CHECK_NOT_DESTROYED;
  value >>= doWrite(CORBA::tk_ulonglong);
  if( pd_union )  pd_union->discriminatorHasChanged();
}
#endif

#ifdef HAS_LongDouble
void
DynUnionDisc::insert_longdouble(CORBA::LongDouble value)
{
  CHECK_NOT_DESTROYED;
  // Not a legal discriminator type.
  throw DynamicAny::DynAny::InvalidValue();
}
#endif

void
DynUnionDisc::insert_wchar(CORBA::WChar value)
{
  CHECK_NOT_DESTROYED;
  // Not a legal discriminator type.
  throw DynamicAny::DynAny::InvalidValue();
}

void
DynUnionDisc::insert_wstring(const CORBA::WChar* value)
{
  CHECK_NOT_DESTROYED;
  // Not a legal discriminator type.
  throw DynamicAny::DynAny::InvalidValue();
}

void
DynUnionDisc::insert_any(const CORBA::Any& value)
{
  CHECK_NOT_DESTROYED;
  // Not a legal discriminator type.
  throw DynamicAny::DynAny::InvalidValue();
}

void
DynUnionDisc::insert_dyn_any(DynamicAny::DynAny_ptr value)
{
  CHECK_NOT_DESTROYED;
  // Not a legal discriminator type.
  throw DynamicAny::DynAny::InvalidValue();
}

///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynUnionDisc::NP_nodetype() const
{
  return dt_disc;
}

void
DynUnionDisc::set_to_initial_value()
{
  // Choose a value corresponding to the first case
  OMNIORB_ASSERT(pd_union);
  set_value(pd_union->actualTc()->NP_member_label_val(0));
}


void
DynUnionDisc::set_value(TypeCode_union::Discriminator v)
{
  switch( tckind() ) {
  case CORBA::tk_char:
    insert_char((CORBA::Char)v);
    break;
  case CORBA::tk_boolean:
    insert_boolean((CORBA::Boolean)v);
    break;
  case CORBA::tk_short:
    insert_short((CORBA::Short)v);
    break;
  case CORBA::tk_ushort:
    insert_ushort((CORBA::UShort)v);
    break;
  case CORBA::tk_long:
    insert_long((CORBA::Long)v);
    break;
  case CORBA::tk_ulong:
    insert_ulong((CORBA::ULong)v);
    break;
#ifdef HAS_LongLong
  case CORBA::tk_longlong:
    insert_longlong((CORBA::LongLong)v);
    break;
  case CORBA::tk_ulonglong:
    insert_ulonglong((CORBA::ULongLong)v);
    break;
#endif
  default:
    throw omniORB::fatalException(__FILE__,__LINE__,
       "DynUnionDisc::set_value() - illegal disciminator type");
  }
}

//////////////////////////////////////////////////////////////////////
///////////////////////// DynUnionEnumDisc ///////////////////////////
//////////////////////////////////////////////////////////////////////

DynUnionEnumDisc::DynUnionEnumDisc(TypeCode_base* tc, DynUnionImpl* un)
  : DynUnionDisc(tc, dt_enumdisc, un)
{
}


DynUnionEnumDisc::~DynUnionEnumDisc() {}

//////////////////////
// public interface //
//////////////////////

DynamicAny::DynAny_ptr
DynUnionEnumDisc::copy()
{
  CHECK_NOT_DESTROYED;

  DynEnumImpl* da = new DynEnumImpl(TypeCode_collector::duplicateRef(tc()),
				    DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(DynamicAny::DynAny::TypeMismatch&) {
    OMNIORB_ASSERT(0);
    // This should never happen since the types always match -- they
    // are the same TypeCode.
  }
  catch(...) {
    da->_NP_decrRefCount();
    throw;
  }
  return da;
}


char*
DynUnionEnumDisc::get_as_string()
{
  CHECK_NOT_DESTROYED;

  CORBA::ULong val;
  {
    if( !isValid() )
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);
				    
    pd_buf.rewindInputPtr();
    val <<= pd_buf;
  }

  if( val >= actualTc()->NP_member_count() )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);

  return CORBA::string_dup(actualTc()->NP_member_name(val));
}

void
DynUnionEnumDisc::set_as_string(const char* value)
{
  CHECK_NOT_DESTROYED;

  if( !value )  OMNIORB_THROW(BAD_PARAM,
			      BAD_PARAM_NullStringUnexpected,
			      CORBA::COMPLETED_NO);

  CORBA::Long index = actualTc()->NP_member_index(value);
  if( index < 0 )  throw DynamicAny::DynAny::InvalidValue();

  pd_buf.rewindPtrs();
  CORBA::ULong(index) >>= pd_buf;
  setValid();
  if( pd_union )  pd_union->discriminatorHasChanged();
}


CORBA::ULong
DynUnionEnumDisc::get_as_ulong()
{
  CHECK_NOT_DESTROYED;

  CORBA::ULong val;
  {
    if( !isValid() )
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);
    pd_buf.rewindInputPtr();
    val <<= pd_buf;
  }

  if( val >= actualTc()->NP_member_count() )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);

  return val;
}


void
DynUnionEnumDisc::set_as_ulong(CORBA::ULong value)
{
  CHECK_NOT_DESTROYED;

  if( value >= actualTc()->NP_member_count() )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_IndexOutOfRange, CORBA::COMPLETED_NO);

  pd_buf.rewindPtrs();
  value >>= pd_buf;
  setValid();
  if( pd_union )  pd_union->discriminatorHasChanged();
}

///////////////////////////////
// exposed private interface //
///////////////////////////////

void
DynUnionEnumDisc::set_to_initial_value()
{
  // Choose a value corresponding to the first case
  OMNIORB_ASSERT(pd_union);
  set_value(pd_union->actualTc()->NP_member_label_val(0));
}

int
DynUnionEnumDisc::NP_nodetype() const
{
  return dt_enumdisc;
}


void
DynUnionEnumDisc::set_value(TypeCode_union::Discriminator v)
{
  pd_buf.rewindPtrs();
  CORBA::ULong(v) >>= pd_buf;
  setValid();
  if( pd_union )  pd_union->discriminatorHasChanged();
}

void
DynUnionEnumDisc::_NP_incrRefCount()
{
  DynAnyImplBase::_NP_incrRefCount();
}

void
DynUnionEnumDisc::_NP_decrRefCount()
{
  DynAnyImplBase::_NP_decrRefCount();
}

void*
DynUnionEnumDisc::_ptrToObjRef(const char* repoId)
{
  if( omni::ptrStrMatch(repoId, DynAnyImplBase::_PD_repoId) )
    return (DynAnyImplBase*) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynEnum::_PD_repoId) )
    return (DynamicAny::DynEnum_ptr) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
    return (DynamicAny::DynAny_ptr) this;
  
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// DynUnionImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

DynUnionImpl::DynUnionImpl(TypeCode_base* tc, CORBA::Boolean is_root)
  : DynAnyImplBase(tc, dt_union, is_root)
{
  CORBA::TypeCode_ptr tcdup =
    CORBA::TypeCode::_duplicate(actualTc()->NP_discriminator_type());
  pd_disc = internal_create_dyn_any_discriminator(ToTcBase(tcdup), this);
  pd_disc_type = ToTcBase(tcdup);
  pd_disc_kind = pd_disc_type->NP_kind();
  pd_disc_index = -1;

  pd_member = 0;
  pd_member_kind = CORBA::tk_null;

  pd_curr_index = 0;
}


DynUnionImpl::~DynUnionImpl()
{
  pd_disc->_NP_decrRefCount();
  if( pd_member )  pd_member->_NP_decrRefCount();
}

//////////////////////
// public interface //
//////////////////////

void
DynUnionImpl::assign(DynamicAny::DynAny_ptr da)
{
  CHECK_NOT_DESTROYED;

  if ( !DynamicAny::DynAny::PR_is_valid(da) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);

  if( CORBA::is_nil(da) )
    throw DynamicAny::DynAny::InvalidValue();

  DynAnyImplBase* daib = ToDynAnyImplBase(da);

  if( !tc()->equivalent(daib->tc()) )
    throw DynamicAny::DynAny::TypeMismatch();

  // We do the copy via an intermediate buffer.

  cdrMemoryStream buf;

  if( !daib->copy_to(buf) )
    throw DynamicAny::DynAny::InvalidValue();

  if( !copy_from(buf) )
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "DynUnionImpl::assign() - copy_from failed unexpectedly");
}


DynamicAny::DynAny_ptr
DynUnionImpl::copy()
{
  CHECK_NOT_DESTROYED;

  DynUnionImpl* da = new DynUnionImpl(TypeCode_collector::duplicateRef(tc()),
				      DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(DynamicAny::DynAny::TypeMismatch&) {
    OMNIORB_ASSERT(0);
    // This should never happen since the types always match -- they
    // are the same TypeCode.
  }
  catch(...) {
    da->_NP_decrRefCount();
    throw;
  }
  return da;
}


CORBA::Boolean
DynUnionImpl::equal(DynamicAny::DynAny_ptr da)
{
  CHECK_NOT_DESTROYED;

  if ( !DynamicAny::DynAny::PR_is_valid(da) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO);

  if( CORBA::is_nil(da) )
    return 0;

  DynAnyImplBase* daib = ToDynAnyImplBase(da);
  if( !tc()->equivalent(daib->tc()) )
    return 0;

  DynUnionImpl* daui = ToDynUnionImpl(da);

  if (!pd_disc->equal(daui->pd_disc)) return 0;

  if (pd_member) return pd_member->equal(daui->pd_member);
  return 1;
}

void
DynUnionImpl::insert_boolean(CORBA::Boolean value)
{
  CHECK_NOT_DESTROYED;
  writeCurrent(CORBA::tk_boolean).marshalBoolean(value);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_octet(CORBA::Octet value)
{
  CHECK_NOT_DESTROYED;
  writeCurrent(CORBA::tk_octet).marshalOctet(value);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_char(CORBA::Char value)
{
  CHECK_NOT_DESTROYED;
  writeCurrent(CORBA::tk_char).marshalChar(value);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_short(CORBA::Short value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_short);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_ushort(CORBA::UShort value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_ushort);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_long(CORBA::Long value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_long);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_ulong(CORBA::ULong value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_ulong);
  discriminatorHasChanged();
}

#ifndef NO_FLOAT
void
DynUnionImpl::insert_float(CORBA::Float value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_float);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_double(CORBA::Double value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_double);
  discriminatorHasChanged();
}

#endif

void
DynUnionImpl::insert_string(const char* value)
{
  CHECK_NOT_DESTROYED;
  if( !value || pd_curr_index != 1 )
    throw DynamicAny::DynAny::InvalidValue();

  if( pd_member_kind != CORBA::tk_string )
    throw DynamicAny::DynAny::TypeMismatch();

  CORBA::ULong maxlen = pd_member->actualTc()->NP_length();

  cdrMemoryStream& buf = writeCurrent(CORBA::tk_string);
  try {
    buf.marshalString(value,maxlen);
  }
  catch (CORBA::MARSHAL& ex) {
    if (ex.minor() == MARSHAL_StringIsTooLong)
      throw DynamicAny::DynAny::InvalidValue();
    else
      throw;
  }
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_reference(CORBA::Object_ptr value)
{
  CHECK_NOT_DESTROYED;

  if ( !CORBA::Object::_PR_is_valid(value) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidObjectRef, CORBA::COMPLETED_NO);

  CORBA::Object::_marshalObjRef(value, writeCurrent(CORBA::tk_objref));
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_typecode(CORBA::TypeCode_ptr value)
{
  CHECK_NOT_DESTROYED;

  if ( !CORBA::TypeCode::PR_is_valid(value) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);

  if( CORBA::is_nil(value) )  throw DynamicAny::DynAny::InvalidValue();

  CORBA::TypeCode::marshalTypeCode(value, writeCurrent(CORBA::tk_TypeCode));
  discriminatorHasChanged();
}


#ifdef HAS_LongLong
void
DynUnionImpl::insert_longlong(CORBA::LongLong value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_longlong);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_ulonglong(CORBA::ULongLong value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_ulonglong);
  discriminatorHasChanged();
}
#endif

#ifdef HAS_LongDouble
void
DynUnionImpl::insert_double(CORBA::LongDouble value)
{
  CHECK_NOT_DESTROYED;
  value >>= writeCurrent(CORBA::tk_longdouble);
  discriminatorHasChanged();
}
#endif

void
DynUnionImpl::insert_wchar(CORBA::WChar value)
{
  CHECK_NOT_DESTROYED;
  writeCurrent(CORBA::tk_wchar).marshalWChar(value);
  discriminatorHasChanged();
}

void
DynUnionImpl::insert_wstring(const CORBA::WChar* value)
{
  CHECK_NOT_DESTROYED;

  if( !value || pd_curr_index != 1 )
    throw DynamicAny::DynAny::InvalidValue();

  if( pd_member_kind != CORBA::tk_wstring )
    throw DynamicAny::DynAny::TypeMismatch();

  CORBA::ULong maxlen = pd_member->actualTc()->NP_length();

  cdrMemoryStream& buf = writeCurrent(CORBA::tk_wstring);
  try {
    buf.marshalWString(value,maxlen);
  }
  catch (CORBA::MARSHAL& ex) {
    if (ex.minor() == MARSHAL_WStringIsTooLong)
      throw DynamicAny::DynAny::InvalidValue();
    else
      throw;
  }
  discriminatorHasChanged();
}

void
DynUnionImpl::insert_any(const CORBA::Any& value)
{
  value >>= writeCurrent(CORBA::tk_any);
  discriminatorHasChanged();
}

void
DynUnionImpl::insert_dyn_any(DynamicAny::DynAny_ptr value)
{
  CHECK_NOT_DESTROYED;
  CORBA::Any_var a(to_any());
  a.in() >>= writeCurrent(CORBA::tk_any);
}


CORBA::Boolean
DynUnionImpl::get_boolean()
{
  CHECK_NOT_DESTROYED;
  return readCurrent(CORBA::tk_boolean).unmarshalBoolean();
}


CORBA::Octet
DynUnionImpl::get_octet()
{
  CHECK_NOT_DESTROYED;
  return readCurrent(CORBA::tk_octet).unmarshalOctet();
}


CORBA::Char
DynUnionImpl::get_char()
{
  CHECK_NOT_DESTROYED;
  return readCurrent(CORBA::tk_char).unmarshalChar();
}


CORBA::Short
DynUnionImpl::get_short()
{
  CHECK_NOT_DESTROYED;
  CORBA::Short value;
  value <<= readCurrent(CORBA::tk_short);
  return value;
}


CORBA::UShort
DynUnionImpl::get_ushort()
{
  CHECK_NOT_DESTROYED;
  CORBA::UShort value;
  value <<= readCurrent(CORBA::tk_ushort);
  return value;
}


CORBA::Long
DynUnionImpl::get_long()
{
  CHECK_NOT_DESTROYED;
  CORBA::Long value;
  value <<= readCurrent(CORBA::tk_long);
  return value;
}


CORBA::ULong
DynUnionImpl::get_ulong()
{
  CHECK_NOT_DESTROYED;
  CORBA::ULong value;
  value <<= readCurrent(CORBA::tk_ulong);
  return value;
}

#ifndef NO_FLOAT
CORBA::Float
DynUnionImpl::get_float()
{
  CHECK_NOT_DESTROYED;
  CORBA::Float value;
  value <<= readCurrent(CORBA::tk_float);
  return value;
}


CORBA::Double
DynUnionImpl::get_double()
{
  CHECK_NOT_DESTROYED;
  CORBA::Double value;
  value <<= readCurrent(CORBA::tk_double);
  return value;
}

#endif

char*
DynUnionImpl::get_string()
{
  CHECK_NOT_DESTROYED;
  cdrMemoryStream& buf = readCurrent(CORBA::tk_string);

  CORBA::ULong maxlen = pd_member->actualTc()->NP_length();
  try {
    char* value = buf.unmarshalString(maxlen);
    return value;
  }
  catch (CORBA::MARSHAL& ex) {
    if (ex.minor() == MARSHAL_StringIsTooLong)
      throw DynamicAny::DynAny::InvalidValue();
    else
      throw;
  }
  return 0;
}


CORBA::Object_ptr
DynUnionImpl::get_reference()
{
  CHECK_NOT_DESTROYED;
  return CORBA::Object::_unmarshalObjRef(readCurrent(CORBA::tk_objref));
}


CORBA::TypeCode_ptr
DynUnionImpl::get_typecode()
{
  CHECK_NOT_DESTROYED;
  return CORBA::TypeCode::unmarshalTypeCode(readCurrent(CORBA::tk_TypeCode));
}



#ifdef HAS_LongLong
CORBA::LongLong
DynUnionImpl::get_longlong()
{
  CHECK_NOT_DESTROYED;
  CORBA::LongLong value;
  value <<= readCurrent(CORBA::tk_longlong);
  return value;
}


CORBA::ULongLong
DynUnionImpl::get_ulonglong()
{
  CHECK_NOT_DESTROYED;
  CORBA::ULongLong value;
  value <<= readCurrent(CORBA::tk_ulonglong);
  return value;
}
#endif

#ifdef HAS_LongDouble
CORBA::LongDouble
DynUnionImpl::get_longdouble()
{
  CHECK_NOT_DESTROYED;
  CORBA::LongDouble value;
  value <<= readCurrent(CORBA::tk_longdouble);
  return value;
}
#endif

CORBA::WChar
DynUnionImpl::get_wchar()
{
  CHECK_NOT_DESTROYED;
  return readCurrent(CORBA::tk_wchar).unmarshalWChar();
}

CORBA::WChar*
DynUnionImpl::get_wstring()
{
  CHECK_NOT_DESTROYED;
  cdrMemoryStream& buf = readCurrent(CORBA::tk_wstring);

  CORBA::ULong maxlen = pd_member->actualTc()->NP_length();
  CORBA::WChar* value = buf.unmarshalWString(maxlen);
  return value;
}

CORBA::Any*
DynUnionImpl::get_any()
{
  CHECK_NOT_DESTROYED;
  CORBA::Any* value = new CORBA::Any();
  try {
    *value <<= readCurrent(CORBA::tk_any);
  }
  catch(...) {
    delete value;
    throw;
  }
  return value;
}

DynamicAny::DynAny_ptr
DynUnionImpl::get_dyn_any()
{
  CHECK_NOT_DESTROYED; 
  CORBA::Any value;
  value <<= readCurrent(CORBA::tk_any);
  return factory_create_dyn_any(value);
}


#define UNION_SEQUENCE_OPS(lcname, ucname) \
void \
DynUnionImpl::insert_##lcname##_seq(CORBA::##ucname##Seq& value) \
{ \
  CHECK_NOT_DESTROYED; \
  if (pd_curr_index != 1 || !pd_member) \
    throw DynamicAny::DynAny::InvalidValue(); \
\
  if (pd_member_kind != CORBA::tk_sequence && \
      pd_member_kind != CORBA::tk_array) \
    throw DynamicAny::DynAny::TypeMismatch(); \
\
  pd_member->insert_##lcname##_seq(value); \
}\
\
CORBA::##ucname##Seq* \
DynUnionImpl::get_##lcname##_seq() \
{ \
  CHECK_NOT_DESTROYED; \
  if (pd_curr_index != 1 || !pd_member) \
    throw DynamicAny::DynAny::InvalidValue(); \
\
  if (pd_member_kind != CORBA::tk_sequence && \
      pd_member_kind != CORBA::tk_array) \
    throw DynamicAny::DynAny::TypeMismatch(); \
\
  return pd_member->get_##lcname##_seq(); \
}

UNION_SEQUENCE_OPS(boolean, Boolean)
UNION_SEQUENCE_OPS(octet, Octet)
UNION_SEQUENCE_OPS(char, Char)
UNION_SEQUENCE_OPS(short, Short)
UNION_SEQUENCE_OPS(ushort, UShort)
UNION_SEQUENCE_OPS(long, Long)
UNION_SEQUENCE_OPS(ulong, ULong)
#ifndef NO_FLOAT
UNION_SEQUENCE_OPS(float, Float)
UNION_SEQUENCE_OPS(double, Double)
#endif
#ifdef HAS_LongLong
UNION_SEQUENCE_OPS(longlong, LongLong)
UNION_SEQUENCE_OPS(ulonglong, ULongLong)
#endif
#ifdef HAS_LongDouble
UNION_SEQUENCE_OPS(longdouble, LongDouble)
#endif
UNION_SEQUENCE_OPS(wchar, WChar)

#undef UNION_SEQUENCE_OPS






CORBA::Boolean
DynUnionImpl::seek(CORBA::Long index)
{
  CHECK_NOT_DESTROYED;

  switch( index ) {
  case 0:
    pd_curr_index = 0;
    return 1;
  case 1:
    if (pd_member) {
      pd_curr_index = 1;
      return 1;
    }
    // drop through
  default:
    pd_curr_index = -1;
    return 0;
  }
}


void
DynUnionImpl::rewind()
{
  CHECK_NOT_DESTROYED;
  pd_curr_index = 0;
}


CORBA::Boolean
DynUnionImpl::next()
{
  CHECK_NOT_DESTROYED;

  switch (pd_curr_index) {
  case -1:
    pd_curr_index = 0;
    return 1;
  case 0:
    if (pd_member) {
      pd_curr_index = 1;
      return 1;
    }
    // drop through
  default:
    pd_curr_index = -1;
    return 0;
  }
}

CORBA::ULong
DynUnionImpl::component_count()
{
  CHECK_NOT_DESTROYED;
  if (pd_member)
    return 2;
  else
    return 1;
}


DynamicAny::DynAny_ptr
DynUnionImpl::current_component()
{
  CHECK_NOT_DESTROYED;

  switch( pd_curr_index ) {
  case 0:
    pd_disc->_NP_incrRefCount();
    return pd_disc;

  case 1:
    if( pd_member ) {
      pd_member->_NP_incrRefCount();
      return pd_member;
    } else
      return DynamicAny::DynAny::_nil();

  default:
    return DynamicAny::DynAny::_nil();
  }
}


DynamicAny::DynAny_ptr
DynUnionImpl::get_discriminator()
{
  CHECK_NOT_DESTROYED;
  pd_disc->_NP_incrRefCount();
  return pd_disc;
}

void
DynUnionImpl::set_discriminator(DynamicAny::DynAny_ptr d)
{
  CHECK_NOT_DESTROYED;
  pd_disc->assign(d);
  pd_curr_index = pd_member ? 1 : 0;
}

void
DynUnionImpl::set_to_default_member()
{
  CHECK_NOT_DESTROYED;

  CORBA::Long defaulti = actualTc()->NP_default_index();

  if (defaulti < 0)
    throw DynamicAny::DynAny::TypeMismatch();

  pd_disc->set_value(actualTc()->NP_default_value());
  pd_curr_index = 0;
}

void
DynUnionImpl::set_to_no_active_member()
{
  CHECK_NOT_DESTROYED;

  CORBA::Long defaulti = actualTc()->NP_default_index();

  if (defaulti != TYPECODE_UNION_IMPLICIT_DEFAULT)
    throw DynamicAny::DynAny::TypeMismatch();

  pd_disc->set_value(actualTc()->NP_default_value());
  pd_curr_index = 0;
}

CORBA::Boolean
DynUnionImpl::has_no_active_member()
{
  CHECK_NOT_DESTROYED;

  return pd_member == 0;
}

CORBA::TCKind
DynUnionImpl::discriminator_kind()
{
  CHECK_NOT_DESTROYED;
  return pd_disc_kind;
}


DynamicAny::DynAny_ptr
DynUnionImpl::member()
{
  CHECK_NOT_DESTROYED;
  if( !pd_member )
    throw DynamicAny::DynAny::InvalidValue();

  pd_member->_NP_incrRefCount();
  return pd_member;
}


char*
DynUnionImpl::member_name()
{
  CHECK_NOT_DESTROYED;
  if( !pd_member )
    throw DynamicAny::DynAny::InvalidValue();

  return CORBA::string_dup(actualTc()->NP_member_name(pd_disc_index));
}


CORBA::TCKind
DynUnionImpl::member_kind()
{
  CHECK_NOT_DESTROYED;
  if( !pd_member )
    throw DynamicAny::DynAny::InvalidValue();

  return pd_member_kind;
}

CORBA::Boolean
DynUnionImpl::is_set_to_default_member()
{
  CHECK_NOT_DESTROYED;
  return (pd_disc_index >= 0 &&
	  pd_disc_index == actualTc()->NP_default_index());
}


///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynUnionImpl::NP_nodetype() const
{
  return dt_union;
}

//////////////
// internal //
//////////////

void
DynUnionImpl::set_to_initial_value()
{
  pd_disc->set_to_initial_value();
  if (pd_member)
    pd_member->set_to_initial_value();
}


int
DynUnionImpl::copy_to(cdrMemoryStream& mbs)
{
  if( !pd_disc->copy_to(mbs) )  return 0;

  return !pd_member || pd_member->copy_to(mbs);
}


int
DynUnionImpl::copy_from(cdrMemoryStream& mbs)
{
  if( !pd_disc->copy_from(mbs) )  return 0;

  discriminatorHasChanged();

  pd_curr_index = 0;
  return !pd_member || pd_member->copy_from(mbs);
}


void
DynUnionImpl::onDispose()
{
  pd_disc->detach();
  if( pd_member )  pd_member->detach();
}


void
DynUnionImpl::discriminatorHasChanged()
{
  // Retrieve the integer value of the discriminator from
  // the buffer of the DynAny which it is stored in.
  TypeCode_union::Discriminator newdisc;
  try {
    pd_disc->pd_buf.rewindInputPtr();
    newdisc =
      TypeCode_union_helper::unmarshalLabel(pd_disc_type, pd_disc->pd_buf);
  }
  catch(CORBA::MARSHAL&) {
    // Not properly initialised.
    if( pd_member )  detachMember();
    return;
  }

  if( pd_member && newdisc == pd_disc_value )  return;

  pd_disc_value = newdisc;

  CORBA::Long newindex = actualTc()->NP_index_from_discriminator(newdisc);

  if( pd_member ) {
    if (pd_disc_index >= 0 && newindex >= 0) {
      // If the discriminator is set to a value corresponding to the
      // same member, we should leave the member value unchanged.
      // Unfortunately, there's no foolproof way of checking if the
      // member is the same. If we have member names, we compare them;
      // if not, we see if the member TypeCodes are equal. Equal
      // TypeCodes doesn't necessarily mean the same member, but it's
      // the best we can do.

      const char* old_member = actualTc()->NP_member_name(pd_disc_index);
      const char* new_member = actualTc()->NP_member_name(newindex);

      if (*old_member == '\0' || *new_member == '\0') {
	CORBA::TypeCode_ptr old_tc = actualTc()->NP_member_type(pd_disc_index);
	CORBA::TypeCode_ptr new_tc = actualTc()->NP_member_type(newindex);

	if (old_tc->equal(new_tc)) {
	  pd_disc_index = newindex;
	  return;
	}
      }
      else if (omni::ptrStrMatch(old_member, new_member)) {
	pd_disc_index = newindex;
	return;
      }
    }
    detachMember();
  }
  pd_disc_index = newindex;
  if( pd_disc_index < 0 ) {
    // Invalid label - implicit default.
    return;
  }

  // Create new member of the appropriate type.
  CORBA::TypeCode_ptr mtc = actualTc()->member_type(pd_disc_index);
  pd_member = internal_create_dyn_any(ToTcBase(mtc), DYNANY_CHILD);
  pd_member->set_to_initial_value();
  pd_member_kind = TypeCode_base::NP_expand(ToTcBase(mtc))->NP_kind();
}

void
DynUnionImpl::_NP_incrRefCount()
{
  DynAnyImplBase::_NP_incrRefCount();
}

void
DynUnionImpl::_NP_decrRefCount()
{
  DynAnyImplBase::_NP_decrRefCount();
}

void*
DynUnionImpl::_ptrToObjRef(const char* repoId)
{
  if( omni::ptrStrMatch(repoId, DynUnionImpl::_PD_repoId) )
    return (DynUnionImpl*) this;
  
  if( omni::ptrStrMatch(repoId, DynAnyImplBase::_PD_repoId) )
    return (DynAnyImplBase*) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynUnion::_PD_repoId) )
    return (DynamicAny::DynUnion_ptr) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
    return (DynamicAny::DynAny_ptr) this;
  
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


//////////////////////////////////////////////////////////////////////
/////////////////////////// DynSequenceImpl //////////////////////////
//////////////////////////////////////////////////////////////////////

DynSequenceImpl::DynSequenceImpl(TypeCode_base* tc, CORBA::Boolean is_root)
  : DynAnyConstrBase(tc, dt_seq, is_root)
{
  pd_bound = actualTc()->NP_length();
}


DynSequenceImpl::~DynSequenceImpl()
{
}

//////////////////////
// public interface //
//////////////////////

DynamicAny::DynAny_ptr
DynSequenceImpl::copy()
{
  CHECK_NOT_DESTROYED;
  DynSequenceImpl* da =
    new DynSequenceImpl(TypeCode_collector::duplicateRef(tc()), DYNANY_ROOT);

  try {
    da->assign(this);
  }
  catch(...) {
    da->_NP_decrRefCount();
    throw;
  }
  return da;
}


CORBA::ULong
DynSequenceImpl::get_length()
{
  CHECK_NOT_DESTROYED;
  return pd_n_components;
}


void
DynSequenceImpl::set_length(CORBA::ULong value)
{
  CHECK_NOT_DESTROYED;
  if( pd_bound && value > pd_bound )
    throw DynamicAny::DynAny::InvalidValue();

  if (pd_n_components == value) return;

  unsigned old_n_components = pd_n_components;

  setNumComponents(value);

  if (value > old_n_components) {
    if (pd_curr_index == -1)
      pd_curr_index = old_n_components;
  }
  else {
    if (pd_curr_index >= (int)pd_n_components)
      pd_curr_index = -1;
  }
}


DynamicAny::AnySeq*
DynSequenceImpl::get_elements()
{
  CHECK_NOT_DESTROYED;
  DynamicAny::AnySeq* as = new DynamicAny::AnySeq();
  as->length(pd_n_components);

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    if( !component_to_any(i, (*as)[i]) ) {
      delete as;
      OMNIORB_THROW(BAD_INV_ORDER, BAD_INV_ORDER_DynAnyNotInitialised,
		    CORBA::COMPLETED_NO);
    }
  }
  return as;
}


void
DynSequenceImpl::set_elements(const DynamicAny::AnySeq& as)
{
  CHECK_NOT_DESTROYED;
  if( pd_bound && as.length() > pd_bound )
    throw DynamicAny::DynAny::InvalidValue();

  if( as.length() != pd_n_components )
    setNumComponents(as.length());

  if( as.length() == 0 )
    pd_curr_index = -1;
  else
    pd_curr_index = 0;

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    if( !component_from_any(i, as[i]) )
      throw DynamicAny::DynAny::TypeMismatch();
  }
}


DynamicAny::DynAnySeq*
DynSequenceImpl::get_elements_as_dyn_any()
{
  CHECK_NOT_DESTROYED;
  DynamicAny::DynAnySeq* as = new DynamicAny::DynAnySeq();
  as->length(pd_n_components);

  createComponent(0);
  // All components are now in the buffer

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    pd_components[i]->_NP_incrRefCount();
    (*as)[i] = pd_components[i];
  }
  return as;
}


void
DynSequenceImpl::set_elements_as_dyn_any(const DynamicAny::DynAnySeq& as)
{
  CHECK_NOT_DESTROYED;
  if( pd_bound && as.length() > pd_bound )
    throw DynamicAny::DynAny::InvalidValue();

  if( as.length() != pd_n_components )
    setNumComponents(as.length());

  if( as.length() == 0 )
    pd_curr_index = -1;
  else
    pd_curr_index = 0;

  pd_n_in_buf = 0;
  pd_first_in_comp = 0;

  CORBA::TypeCode_var tc;

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    tc = as[i]->type();
    if( !tc->equivalent(nthComponentTC(i)) ) {
      pd_first_in_comp = pd_n_components;
      throw DynamicAny::DynAny::TypeMismatch();
    }
    DynAnyImplBase* daib = ToDynAnyImplBase(as[i]);
    if (daib->is_root()) {
      // Take ownership of the DynAny
      daib->_NP_incrRefCount();
      daib->attach();
    }
    else {
      DynamicAny::DynAny_ptr newda = daib->copy();
      daib = ToDynAnyImplBase(newda);
    }
    pd_components[i] = daib;
  }
}

///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynSequenceImpl::NP_nodetype() const
{
  return dt_seq;
}



//////////////
// internal //
//////////////

int
DynSequenceImpl::copy_to(cdrMemoryStream& mbs)
{
  // Write the length of the sequence.
  CORBA::ULong(pd_n_components) >>= mbs;

  // Copy the elements.
  return DynAnyConstrBase::copy_to(mbs);
}


int
DynSequenceImpl::copy_from(cdrMemoryStream& mbs)
{
  CORBA::ULong len;
  try {
    // Read the length of the sequence.
    len <<= mbs;
  }
  catch(CORBA::MARSHAL&) {
    return 0;
  }

  if (len > pd_n_components && pd_first_in_comp == pd_n_components) {
    // Extend sequence without initialising components
    pd_components.reserve(len);
    DynAnyImplBase* tmp = 0;
    while (pd_components.size() < len) pd_components.push_back(tmp);
    pd_n_components = pd_first_in_comp = len;
  }
  else
    setNumComponents(len);

  return DynAnyConstrBase::copy_from(mbs);
}


TypeCode_base*
DynSequenceImpl::nthComponentTC(unsigned n)
{
  return actualTc()->NP_content_type();
}


DynAnyConstrBase::SeqLocation
DynSequenceImpl::prepareSequenceWrite(CORBA::TCKind kind, CORBA::ULong len)
{
  const TypeCode_base* tc = actualTc();
  const TypeCode_base* ctc;

  ctc = TypeCode_base::NP_expand(tc->NP_content_type());
  CORBA::TCKind k = ctc->NP_kind();

  if (ctc->NP_kind() == kind) {
    if (pd_bound && len > pd_bound)
      throw DynamicAny::DynAny::InvalidValue();

    // Clear old components
    setNumComponents(0);

    // Create new components in <pd_buf>
    pd_components.reserve(len);
    DynAnyImplBase* tmp = 0;
    while( pd_components.size() < len )  pd_components.push_back(tmp);
    pd_n_components = pd_first_in_comp = len;
    pd_n_in_buf = pd_n_really_in_buf = len;
    pd_buf.rewindPtrs();
    return SEQ_HERE;
  }

  if (pd_curr_index < 0)
    throw DynamicAny::DynAny::InvalidValue();

  if ((k == CORBA::tk_sequence || k == CORBA::tk_array) &&
      TypeCode_base::NP_expand(ctc->NP_content_type())->NP_kind() == kind) {
    return SEQ_COMPONENT;
  }
  throw DynamicAny::DynAny::TypeMismatch();
#ifdef NEED_DUMMY_RETURN
  return SEQ_COMPONENT;
#endif
}

DynAnyConstrBase::SeqLocation
DynSequenceImpl::prepareSequenceRead(CORBA::TCKind kind)
{
  const TypeCode_base* tc = actualTc();
  const TypeCode_base* ctc;

  ctc = TypeCode_base::NP_expand(tc->NP_content_type());
  CORBA::TCKind k = ctc->NP_kind();

  if (ctc->NP_kind() == kind)
    return SEQ_HERE;

  if (pd_curr_index < 0)
    throw DynamicAny::DynAny::InvalidValue();

  if ((k == CORBA::tk_sequence || k == CORBA::tk_array) &&
      TypeCode_base::NP_expand(ctc->NP_content_type())->NP_kind() == kind) {
    return SEQ_COMPONENT;
  }
  throw DynamicAny::DynAny::TypeMismatch();
#ifdef NEED_DUMMY_RETURN
  return SEQ_COMPONENT;
#endif
}


void
DynSequenceImpl::_NP_incrRefCount()
{
  DynAnyImplBase::_NP_incrRefCount();
}

void
DynSequenceImpl::_NP_decrRefCount()
{
  DynAnyImplBase::_NP_decrRefCount();
}

void*
DynSequenceImpl::_ptrToObjRef(const char* repoId)
{
  if( omni::ptrStrMatch(repoId, DynAnyConstrBase::_PD_repoId) )
    return (DynAnyConstrBase*) this;
  
  if( omni::ptrStrMatch(repoId, DynAnyImplBase::_PD_repoId) )
    return (DynAnyImplBase*) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynSequence::_PD_repoId) )
    return (DynamicAny::DynSequence_ptr) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
    return (DynamicAny::DynAny_ptr) this;
  
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////// DynArrayImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

DynArrayImpl::DynArrayImpl(TypeCode_base* tc, CORBA::Boolean is_root)
  : DynAnyConstrBase(tc, dt_array, is_root)
{
  // Set things up without initialising components
  pd_n_components = pd_first_in_comp = actualTc()->NP_length();
  DynAnyImplBase* tmp = 0;
  while (pd_components.size() < pd_n_components)
    pd_components.push_back(tmp);
}


DynArrayImpl::~DynArrayImpl()
{
}

//////////////////////
// public interface //
//////////////////////

DynamicAny::DynAny_ptr
DynArrayImpl::copy()
{
  DynArrayImpl* da = new DynArrayImpl(TypeCode_collector::duplicateRef(tc()),
				      DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(...) {
    da->_NP_decrRefCount();
    throw;
  }
  return da;
}


DynamicAny::AnySeq*
DynArrayImpl::get_elements()
{
  DynamicAny::AnySeq* as = new DynamicAny::AnySeq();
  as->length(pd_n_components);

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    if( !component_to_any(i, (*as)[i]) ) {
      delete as;
      OMNIORB_THROW(BAD_INV_ORDER, BAD_INV_ORDER_DynAnyNotInitialised,
		    CORBA::COMPLETED_NO);
    }
  }
  return as;
}


void
DynArrayImpl::set_elements(const DynamicAny::AnySeq& as)
{
  if( as.length() != pd_n_components )
    throw DynamicAny::DynAny::InvalidValue();

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    if( !component_from_any(i, as[i]) )
      throw DynamicAny::DynAny::TypeMismatch();
  }
}

DynamicAny::DynAnySeq*
DynArrayImpl::get_elements_as_dyn_any()
{
  DynamicAny::DynAnySeq* as = new DynamicAny::DynAnySeq();
  as->length(pd_n_components);

  createComponent(0);
  // All components are now in the buffer

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    pd_components[i]->_NP_incrRefCount();
    (*as)[i] = pd_components[i];
  }
  return as;
}


void
DynArrayImpl::set_elements_as_dyn_any(const DynamicAny::DynAnySeq& as)
{
  pd_n_in_buf = 0;
  pd_first_in_comp = 0;

  CORBA::TypeCode_var tc;

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    tc = as[i]->type();
    if( !tc->equivalent(nthComponentTC(i)) ) {
      pd_first_in_comp = pd_n_components;
      throw DynamicAny::DynAny::TypeMismatch();
    }
    DynAnyImplBase* daib = ToDynAnyImplBase(as[i]);
    if (daib->is_root()) {
      // Take ownership of the DynAny
      daib->_NP_incrRefCount();
      daib->attach();
    }
    else {
      DynamicAny::DynAny_ptr newda = daib->copy();
      daib = ToDynAnyImplBase(newda);
    }
    pd_components[i] = daib;
  }
}


///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynArrayImpl::NP_nodetype() const
{
  return dt_array;
}


//////////////
// internal //
//////////////

TypeCode_base*
DynArrayImpl::nthComponentTC(unsigned n)
{
  return actualTc()->NP_content_type();
}


DynAnyConstrBase::SeqLocation
DynArrayImpl::prepareSequenceWrite(CORBA::TCKind kind, CORBA::ULong len)
{
  const TypeCode_base* tc = actualTc();
  const TypeCode_base* ctc;

  ctc = TypeCode_base::NP_expand(tc->NP_content_type());
  CORBA::TCKind k = ctc->NP_kind();

  if (ctc->NP_kind() == kind) {
    if (len != pd_n_components)
      throw DynamicAny::DynAny::InvalidValue();

    // Clear old components
    for (unsigned i = pd_first_in_comp; i < pd_n_components; i++) {
      pd_components[i]->detach();
      pd_components[i]->_NP_decrRefCount();
    }

    // Write will fill <pd_buf>
    pd_first_in_comp = pd_n_in_buf = pd_n_really_in_buf = len;
    pd_buf.rewindPtrs();
    return SEQ_HERE;
  }

  if (pd_curr_index < 0)
    throw DynamicAny::DynAny::InvalidValue();

  if ((k == CORBA::tk_sequence || k == CORBA::tk_array) &&
      TypeCode_base::NP_expand(ctc->NP_content_type())->NP_kind() == kind) {
    return SEQ_COMPONENT;
  }
  throw DynamicAny::DynAny::TypeMismatch();
#ifdef NEED_DUMMY_RETURN
  return SEQ_COMPONENT;
#endif
}


DynAnyConstrBase::SeqLocation
DynArrayImpl::prepareSequenceRead(CORBA::TCKind kind)
{
  const TypeCode_base* tc = actualTc();
  const TypeCode_base* ctc;

  ctc = TypeCode_base::NP_expand(tc->NP_content_type());
  CORBA::TCKind k = ctc->NP_kind();

  if (ctc->NP_kind() == kind)
    return SEQ_HERE;

  if (pd_curr_index < 0)
    throw DynamicAny::DynAny::InvalidValue();

  if ((k == CORBA::tk_sequence || k == CORBA::tk_array) &&
      TypeCode_base::NP_expand(ctc->NP_content_type())->NP_kind() == kind) {
    return SEQ_COMPONENT;
  }
  throw DynamicAny::DynAny::TypeMismatch();
#ifdef NEED_DUMMY_RETURN
  return SEQ_COMPONENT;
#endif
}


void
DynArrayImpl::_NP_incrRefCount()
{
  DynAnyImplBase::_NP_incrRefCount();
}

void
DynArrayImpl::_NP_decrRefCount()
{
  DynAnyImplBase::_NP_decrRefCount();
}

void*
DynArrayImpl::_ptrToObjRef(const char* repoId)
{
  if( omni::ptrStrMatch(repoId, DynAnyConstrBase::_PD_repoId) )
    return (DynAnyConstrBase*) this;
  
  if( omni::ptrStrMatch(repoId, DynAnyImplBase::_PD_repoId) )
    return (DynAnyImplBase*) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynArray::_PD_repoId) )
    return (DynamicAny::DynArray_ptr) this;
  
  if( omni::ptrStrMatch(repoId, DynamicAny::DynAny::_PD_repoId) )
    return (DynamicAny::DynAny_ptr) this;
  
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}



OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)



//////////////////////////////////////////////////////////////////////
/////////////////////////////// _narrow //////////////////////////////
//////////////////////////////////////////////////////////////////////

#define DECLARE_NARROW_FN(dynkind) \
DynamicAny::dynkind::_ptr_type \
DynamicAny::dynkind::_narrow(CORBA::Object_ptr o) \
{ \
  if (CORBA::is_nil(o)) return DynamicAny::dynkind::_nil(); \
\
  DynamicAny::dynkind::_ptr_type r = \
    (DynamicAny::dynkind::_ptr_type)o-> \
                     _ptrToObjRef(DynamicAny::dynkind::_PD_repoId); \
\
  if (r) return DynamicAny::dynkind::_duplicate(r); \
\
  return DynamicAny::dynkind::_nil(); \
}

DECLARE_NARROW_FN(DynAny)
DECLARE_NARROW_FN(DynEnum)
DECLARE_NARROW_FN(DynFixed)
DECLARE_NARROW_FN(DynStruct)
DECLARE_NARROW_FN(DynUnion)
DECLARE_NARROW_FN(DynSequence)
DECLARE_NARROW_FN(DynArray)

#undef DECLARE_NARROW_FN

//////////////////////////////////////////////////////////////////////
///////////////////////////// _duplicate /////////////////////////////
//////////////////////////////////////////////////////////////////////

#define DECLARE_DUPLICATE_FN(dynkind) \
DynamicAny::dynkind::_ptr_type \
DynamicAny::dynkind::_duplicate(DynamicAny::dynkind::_ptr_type p) \
{ \
  if ( !DynamicAny::DynAny::PR_is_valid(p) ) \
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidDynAny, CORBA::COMPLETED_NO); \
\
  if( !CORBA::is_nil(p) ) { \
    ToDynAnyImplBase(p)->_NP_incrRefCount(); \
    return p; \
  } \
  else \
    return DynamicAny::dynkind::_nil(); \
}

DECLARE_DUPLICATE_FN(DynAny)
DECLARE_DUPLICATE_FN(DynEnum)
DECLARE_DUPLICATE_FN(DynFixed)
DECLARE_DUPLICATE_FN(DynStruct)
DECLARE_DUPLICATE_FN(DynUnion)
DECLARE_DUPLICATE_FN(DynSequence)
DECLARE_DUPLICATE_FN(DynArray)

#undef DECLARE_DUPLICATE_FN


//////////////////////////////////////////////////////////////////////
/////////////////////////// DynAny_helper ////////////////////////////
//////////////////////////////////////////////////////////////////////

DynamicAny::DynAny_ptr
DynamicAny::DynAny_Helper::_nil()
{
  return DynamicAny::DynAny::_nil();
}

CORBA::Boolean
DynamicAny::DynAny_Helper::is_nil(DynamicAny::DynAny_ptr p)
{
  return CORBA::is_nil(p);
}

void
DynamicAny::DynAny_Helper::release(DynamicAny::DynAny_ptr p)
{
  CORBA::release(p);
}

void
DynamicAny::DynAny_Helper::duplicate(DynamicAny::DynAny_ptr p)
{
  DynamicAny::DynAny::_duplicate(p);
}


//////////////////////////////////////////////////////////////////////
///////////////////////////// DynAnyFactory //////////////////////////
//////////////////////////////////////////////////////////////////////

OMNI_NAMESPACE_BEGIN(omni)

// <tc> is consumed. Since 2.8.0 it no longer have to alias-expand
static DynAnyImplBase*
internal_create_dyn_any(TypeCode_base* tc, CORBA::Boolean is_root)
{
  if ( !CORBA::TypeCode::PR_is_valid(tc) )
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidTypeCode,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(tc) )
    OMNIORB_THROW(BAD_TYPECODE,BAD_TYPECODE_TypeCodeIsNil,CORBA::COMPLETED_NO);

  DynAnyImplBase* da = 0;

  try {
    switch( TypeCode_base::NP_expand(tc)->NP_kind() ) {
    case CORBA::tk_void:
    case CORBA::tk_short:
    case CORBA::tk_long:
    case CORBA::tk_ushort:
    case CORBA::tk_ulong:
#ifdef HAS_LongLong
    case CORBA::tk_longlong:
    case CORBA::tk_ulonglong:
#endif
#ifndef NO_FLOAT
    case CORBA::tk_float:
    case CORBA::tk_double:
#ifdef HAS_LongDouble
    case CORBA::tk_longdouble:
#endif
#endif
    case CORBA::tk_boolean:
    case CORBA::tk_char:
    case CORBA::tk_wchar:
    case CORBA::tk_octet:
    case CORBA::tk_any:
    case CORBA::tk_TypeCode:
    case CORBA::tk_objref:
    case CORBA::tk_string:
    case CORBA::tk_wstring:
      da = new DynAnyImpl(tc, dt_any, is_root);
      break;
    case CORBA::tk_fixed:
      da = new DynFixedImpl(tc, is_root);
      break;
    case CORBA::tk_enum:
      da = new DynEnumImpl(tc, is_root);
      break;
    case CORBA::tk_struct:
    case CORBA::tk_except:
      da = new DynStructImpl(tc, is_root);
      break;
    case CORBA::tk_union:
      da = new DynUnionImpl(tc, is_root);
      break;
    case CORBA::tk_sequence:
      da = new DynSequenceImpl(tc, is_root);
      break;
    case CORBA::tk_array:
      da = new DynArrayImpl(tc, is_root);
      break;
    default:
      throw DynamicAny::DynAny::TypeMismatch();
    }
  }
  catch(...) {
    CORBA::release(tc);
    throw;
  }
  return da;
}


// <tc> is consumed. Since 2.8.0 we no longer have to alias-expand
static DynUnionDisc*
internal_create_dyn_any_discriminator(TypeCode_base* tc, DynUnionImpl* du)
{
  if ( !CORBA::TypeCode::PR_is_valid(tc) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);

  if( CORBA::is_nil(tc) )
    OMNIORB_THROW(BAD_TYPECODE, BAD_TYPECODE_TypeCodeIsNil,
		  CORBA::COMPLETED_NO);

  DynUnionDisc* da = 0;

  try {
    switch( TypeCode_base::NP_expand(tc)->NP_kind() ) {
    case CORBA::tk_enum:
      da = new DynUnionEnumDisc(tc, du);
      break;
    default:
      da = new DynUnionDisc(tc, dt_disc, du);
      break;
    }
  }
  catch(...) {
    CORBA::release(tc);
    throw;
  }
  return da;
}

DynamicAny::DynAny_ptr
factory_create_dyn_any(const CORBA::Any& value)
{
  CORBA::TypeCode_var tc = value.type();
  if( CORBA::is_nil(tc) )
    OMNIORB_THROW(BAD_TYPECODE, BAD_TYPECODE_TypeCodeIsNil,
		  CORBA::COMPLETED_NO);

  DynAnyImplBase* da;
  try {
    da = internal_create_dyn_any(ToTcBase_Checked(CORBA::
						  TypeCode::_duplicate(tc)),
				 DYNANY_ROOT);
  }
  catch (DynamicAny::DynAny::TypeMismatch&) {
    CORBA::TCKind tck = TypeCode_base::NP_expand(ToTcBase(tc))->NP_kind();
    if (tck == CORBA::tk_Principal || tck == CORBA::tk_native)
      throw DynamicAny::DynAnyFactory::InconsistentTypeCode();
    else
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);
  }
  da->from_any(value);
  return da;
}


DynamicAny::DynAny_ptr
factory_create_dyn_any_from_type_code(CORBA::TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);

  if( CORBA::is_nil(tc) )
    OMNIORB_THROW(BAD_TYPECODE, BAD_TYPECODE_TypeCodeIsNil,
		  CORBA::COMPLETED_NO);

  const TypeCode_base* aetc = TypeCode_base::NP_expand(ToTcBase_Checked(tc));

  DynAnyImplBase* r;

  switch( aetc->kind() ) {
  case CORBA::tk_null:
  case CORBA::tk_void:
  case CORBA::tk_short:
  case CORBA::tk_long:
  case CORBA::tk_ushort:
  case CORBA::tk_ulong:
#ifdef HAS_LongLong
  case CORBA::tk_longlong:
  case CORBA::tk_ulonglong:
#endif
#ifndef NO_FLOAT
  case CORBA::tk_float:
  case CORBA::tk_double:
#ifdef HAS_LongDouble
  case CORBA::tk_longdouble:
#endif
#endif
  case CORBA::tk_boolean:
  case CORBA::tk_char:
  case CORBA::tk_wchar:
  case CORBA::tk_octet:
  case CORBA::tk_any:
  case CORBA::tk_TypeCode:
  case CORBA::tk_objref:
  case CORBA::tk_string:
  case CORBA::tk_wstring:
    r = new DynAnyImpl(ToTcBase_Checked(CORBA::TypeCode::_duplicate(tc)),
		       dt_any, DYNANY_ROOT);
    r->set_to_initial_value();
    return r;

  case CORBA::tk_fixed:
    r = new DynFixedImpl(ToTcBase_Checked(CORBA::TypeCode::_duplicate(tc)),
			 DYNANY_ROOT);
    r->set_to_initial_value();
    return r;

  case CORBA::tk_enum:
    r = new DynEnumImpl(ToTcBase_Checked(CORBA::TypeCode::_duplicate(tc)),
			DYNANY_ROOT);
    r->set_to_initial_value();
    return r;

  case CORBA::tk_struct:
  case CORBA::tk_except:
    r = new DynStructImpl(ToTcBase_Checked(CORBA::TypeCode::_duplicate(tc)),
			  DYNANY_ROOT);
    r->set_to_initial_value();
    return r;

  case CORBA::tk_union:
    r = new DynUnionImpl(ToTcBase_Checked(CORBA::TypeCode::_duplicate(tc)),
			 DYNANY_ROOT);
    r->set_to_initial_value();
    return r;

  case CORBA::tk_sequence:
    r = new DynSequenceImpl(ToTcBase_Checked(CORBA::TypeCode::_duplicate(tc)),
			    DYNANY_ROOT);
    r->set_to_initial_value();
    return r;
    
  case CORBA::tk_array:
    r = new DynArrayImpl(ToTcBase_Checked(CORBA::TypeCode::_duplicate(tc)),
		     DYNANY_ROOT);
    r->set_to_initial_value();
    return r;

  case CORBA::tk_Principal:
  case CORBA::tk_native:
    throw DynamicAny::DynAnyFactory::InconsistentTypeCode();

  default:
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidTypeCode, CORBA::COMPLETED_NO);
  }
  return 0;
}


DynAnyFactoryImpl::~DynAnyFactoryImpl()
{
}

DynamicAny::DynAny_ptr
DynAnyFactoryImpl::create_dyn_any(const CORBA::Any& value)
{
  return factory_create_dyn_any(value);
}

DynamicAny::DynAny_ptr
DynAnyFactoryImpl::create_dyn_any_from_type_code(const CORBA::TypeCode_ptr t)
{
  return factory_create_dyn_any_from_type_code(t);
}

static DynAnyFactoryImpl* theDynAnyFactory;


void
DynAnyFactoryImpl::_NP_incrRefCount()
{
  omni::poRcLock->lock();
  pd_refCount++;
  omni::poRcLock->unlock();
}

void
DynAnyFactoryImpl::_NP_decrRefCount()
{
  omni::poRcLock->lock();
  int dead = --pd_refCount == 0;
  if (dead) theDynAnyFactory = 0;
  omni::poRcLock->unlock();

  if( !dead )  return;

  delete this;
}

DynamicAny::DynAnyFactory_ptr
DynAnyFactoryImpl::theFactory()
{
  omni_tracedmutex_lock l(*omni::poRcLock);

  if (!theDynAnyFactory) {
    theDynAnyFactory = new DynAnyFactoryImpl();
    return theDynAnyFactory;
  }

  theDynAnyFactory->pd_refCount++;
  return theDynAnyFactory;
}


void*
DynAnyFactoryImpl::_ptrToObjRef(const char* repoId)
{
  if( omni::ptrStrMatch(repoId, DynamicAny::DynAnyFactory::_PD_repoId) )
    return (DynamicAny::DynAnyFactory_ptr) this;
  
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}

OMNI_NAMESPACE_END(omni)

DynamicAny::DynAnyFactory_ptr
DynamicAny::DynAnyFactory::_narrow(CORBA::Object_ptr o)
{
  if (CORBA::is_nil(o)) return DynamicAny::DynAnyFactory::_nil();

  DynamicAny::DynAnyFactory_ptr r;
  r = (DynamicAny::DynAnyFactory_ptr)o->
                          _ptrToObjRef(DynamicAny::DynAnyFactory::_PD_repoId);

  if (r) return DynamicAny::DynAnyFactory::_duplicate(r);
  return DynamicAny::DynAnyFactory::_nil();
}

DynamicAny::DynAnyFactory_ptr
DynamicAny::DynAnyFactory::_duplicate(DynamicAny::DynAnyFactory_ptr p)
{
  if (!CORBA::is_nil(p)) {
    p->_NP_incrRefCount();
    return p;
  }
  else
    return DynamicAny::DynAnyFactory::_nil();
}

OMNI_NAMESPACE_BEGIN(omni)

static CORBA::Object_ptr resolveDynAnyFactoryFn() {
  return DynAnyFactoryImpl::theFactory();
}

class omni_dynAny_initialiser : public omniInitialiser {
public:
  omni_dynAny_initialiser() {
    omniInitialReferences::registerPseudoObjFn("DynAnyFactory",
					       resolveDynAnyFactoryFn);
  }

  void attach() { }
  void detach() { }
};

static omni_dynAny_initialiser initialiser;

omniInitialiser& omni_dynAny_initialiser_ = initialiser;


OMNI_NAMESPACE_END(omni)
