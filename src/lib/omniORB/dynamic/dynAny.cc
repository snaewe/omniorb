// -*- Mode: C++; -*-
//                            Package   : omniORB2
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

/*
  Clarification on the DynAny interface
  -------------------------------------

  DynAny current_component();

  Returns the DynAny object associates with the current logical pointer
  into the DynAny buffer.

  [clarification end]
  If the current logical pointer is invalid, return CORBA::DynAny::_nil().
  [clarification begin]

  Clarification on the DynStruct interface
  ----------------------------------------

  char* current_member_name();

  Returns the name of the member currently being accessed.

  [clarification end]
  The current logical pointer points to the member.
  If the current logical pointer is invalid, return 0.
  [clarification begin]

  TCKind current_member_kind();

  Returns the TCKind associated with the current member being accessed.

  [clarification end]
  The current logical pointer points to the member.
  If the current logical pointer is invalid, return tk_null.
  [clarification begin]


  NameValuePairSeq* get_members();

  Obtain a sequence of name/value pairs describing the name and the value
  of each member in the struct.

  [clarification end]
  If the struct has not bee completely initialised, raise a 
  CORBA::SystemException. 
  [clarification begin]



  Clarification on the DynUnion interface
  ---------------------------------------
  The DynUnion interface as defined in CORBA 2.2 is lacking in detail on the
  behaviour of the member functions. As a result a number of intepretations
  could be applied. A number of these issues are currently opened with the ORB 
  revision task force. Until the issues are resolved, the following 
  interpretation is used in this implementation:

  DynUnion objects are associated with unions.

  interface DynUnion: DynAny {
     attribute boolean set_as_default;
     DynAny discriminator();
     TCKind discriminator_kind();
     DynAny member();
     attribute FieldName member_name;
     TCKind member_kind();
  };

  The DynUnion interface allows for the insertion/extraction of an OMG
  IDL union type into/from a DynUnion object.

  The TCKind associated with the discriminator is returned by 
  discriminator_kind().

  TCKind discriminator_kind();

  The discriminator operation returns a DynAny object reference that must
  be narrowed to the type of the discriminator in order to insert/get
  the discriminator value:

  DynAny discriminator();

  [clarification begin]
  Insert a value into the discriminator select the member. 

  If the discriminator has already been initialised by a previous insert
  operation, the new insert may cause a different member to be selected. In
  this case the previous member would be overridden.

  Overriding an already initialised discriminator, to say the least,
  is confusing. Developers are STRONG DISCOURAGED from writing code that
  relies on this behaviour. It is possible that a future CORBA revision may
  disallow this completely.
  [clarification end] 

  The member operation retuns a DynAny object reference that is used in order
  to insert/get the member of the union:

  DynAny member();

  The TCKind associated with the member of the union is returned by
  member_kind.

  TCKind member_kind();

  [clarification begin]
  If the discriminator of the DynUnion has not been initialised, either through
  the DynAny returned by discriminator() or through the value copied from
  the argument when the DynUnion was constructed, member() returns a
  nil DynAny (CORBA::DynAny::_nil()). member_kind() returns tk_null.
  
  Suppose a member() call returns a DynAny_ptr A. Then a different 
  discriminant value is inserted  causing a different member to be selected. 
  In this case, inserting a value through A would not affect the value of
  the DynUnion. A effectively is detached from the union. If A already contains
  a valid value, this value would be returned by A and would not be the
  current value of the union.

  Again, developers are STRONG DISCOURAGED from writing code that relies
  on this behaviour.
  [clarification end]

  The member_name attribute allows for the inspection/assignment of the
  name of the union member without checking the value of the discriminator.

  [clarification begin]
  Setting the name of the union member automatically causes the value of
  the discriminator to changed to the corresponding value reflecting the
  selection. 

  If the discriminant has already been initialised to a different value-member
  selection, the description above applies.
  
  [clarification end]

  The set_as_default attribute determines whether the discriminator
  associated with the union has been assigned a valid default value.

  [clarification begin]

  If the union does have an explicit default branch, writing a TRUE value
  to the attribute select that member. Writing a FALSE value is quietly
  ignored because the exact meaning of this operation is unclear.

  If the union does not have an explicit default branch but not all permissible
  values of the union discriminant are listed, writing a TRUE value to
  the attribute select the implicit default. In this case, the value of the
  union is composed only of the discriminant value. Writing a FALSE value
  is ignored.

  If all the permissible values of the union discriminant are listed,
  set_as_default always returns FALSE and writing a value to the attribute
  is quietly ignored.


  [clarification end]

  Recommendation to developers on the use of DynUnion
  ---------------------------------------------------
  When writing into a DynUnion, uses the following steps:
  1. Create a DynUnion using CORBA::ORB::create_dyn_union.
  2. Set the member_name attribute to select the member.
  3. Use member() to obtain the DynAny of the member.
  4. Insert value into the member DynAny.

  Never change the discriminator once it has been initialised.  



  Clarification on the DynSequence interface
  ------------------------------------------

  AnySeq* get_elements();

  Return the elements of the sequence.

  [clarification end]
  If any of the elements in the sequence has not be initialised, raise a
  CORBA::SystemException.
  [clarification begin]


  Clarification on the DynArray interface
  ------------------------------------------

  AnySeq* get_elements();

  Return the elements of the array.

  [clarification end]
  If any of the elements in the array has not be initialised, raise a
  CORBA::SystemException.
  [clarification begin]


*/

#include <dynAny.h>
#include <tcParser.h>
#include <anyP.h>
#include <string.h>


// Note:
//  The constructors in this file NEVER throw exceptions. This
// is to avoid a bug in MSVC 4 & 5, which happens when public
// virtual inheritance is used. See relStream.h for more details.


static DynAnyImplBase*
create_dyn_any(TypeCode_base* tc, CORBA::Boolean is_root);

static DynUnionDisc*
create_dyn_any_discriminator(TypeCode_base* tc, DynUnionImpl* du);

CORBA::DynAny::DynAny() { pd_magic = CORBA::DynAny::PR_magic; }
CORBA::DynAny::~DynAny() { pd_magic = 0; }
CORBA::DynEnum::~DynEnum() {}
CORBA::DynStruct::~DynStruct() {}
CORBA::DynUnion::~DynUnion() {}
CORBA::DynSequence::~DynSequence() {}
CORBA::DynArray::~DynArray() {}


//////////////////////////////////////////////////////////////////////
/////////////////////////// DynAnyImplBase ///////////////////////////
//////////////////////////////////////////////////////////////////////


omni_mutex DynAnyImplBase::refCountLock;
omni_mutex DynAnyImplBase::lock;


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
  return CORBA::TypeCode::_duplicate(pd_tc);
}


void
DynAnyImplBase::destroy()
{
  // Do nothing. Let reference counting do garbage collection.
}


void
DynAnyImplBase::from_any(const CORBA::Any& value)
{
  CORBA::TypeCode_var value_tc = value.type();
  if( !value_tc->equal(tc()) )  throw CORBA::DynAny::Invalid();

  MemBufferedStream& buf = ((AnyP*)value.NP_pd())->getMemBufferedStream();
  buf.rewind_in_mkr();

  Lock sync(this);
  if( !copy_from(buf) )  throw CORBA::DynAny::Invalid();
}


CORBA::Any*
DynAnyImplBase::to_any()
{
  CORBA::Any* a = new CORBA::Any(tc(), 0);
  MemBufferedStream& buf = ((AnyP*)a->NP_pd())->getMemBufferedStream();

  // <buf> should already be rewound.

  Lock sync(this);
  if( !copy_to(buf) ) {
    delete a;
    throw CORBA::DynAny::Invalid();
  }
  return a;
}

///////////////////////////////
// exposed private interface //
///////////////////////////////

CORBA::Boolean
DynAnyImplBase::NP_is_nil() const
{
  return 0;
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
DynAnyImplBase::incrRefCount()
{
  omni_mutex_lock sync(DynAnyImplBase::refCountLock);
  pd_refcount++;
}


void
DynAnyImplBase::decrRefCount()
{
  int do_delete = 0;
  {
    omni_mutex_lock sync(DynAnyImplBase::refCountLock);

    if( pd_refcount > 0 )  pd_refcount--;

    if( pd_refcount == 0 && pd_is_root ) {
      do_delete = 1;
      // This hook allows derived types to detach their children
      // so that they will be destoyed when their ref count goes
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
DynAnyImpl::assign(CORBA::DynAny_ptr da)
{
  if ( !CORBA::DynAny::PR_is_valid(da) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(da) )  throw CORBA::DynAny::Invalid();

  DynAnyImplBase* daib = ToDynAnyImplBase(da);
  if( !tc()->equal(daib->tc()) )  throw CORBA::DynAny::Invalid();
  DynAnyImpl* dai = ToDynAnyImpl(daib);

  Lock sync(this);

  if( !dai->isValid() )  throw CORBA::DynAny::Invalid();

  dai->pd_buf.rewind_in_mkr();
  pd_buf.rewind_inout_mkr();
  pd_buf.copy_from(dai->pd_buf, dai->pd_buf.RdMessageUnRead());
  setValid();
}


CORBA::DynAny_ptr
DynAnyImpl::copy()
{
  DynAnyImpl* da = new DynAnyImpl(tc(), dt_any, DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(CORBA::DynAny::Invalid&) {
    da->decrRefCount();
    throw;
  }
  return da;
}


void
DynAnyImpl::insert_boolean(CORBA::Boolean value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_boolean);
}


void
DynAnyImpl::insert_octet(CORBA::Octet value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_octet);
}


void
DynAnyImpl::insert_char(CORBA::Char value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_char);
}


void
DynAnyImpl::insert_short(CORBA::Short value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_short);
}


void
DynAnyImpl::insert_ushort(CORBA::UShort value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_ushort);
}


void
DynAnyImpl::insert_long(CORBA::Long value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_long);
}


void
DynAnyImpl::insert_ulong(CORBA::ULong value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_ulong);
}
#ifndef NO_FLOAT
void
DynAnyImpl::insert_float(CORBA::Float value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_float);
}


void
DynAnyImpl::insert_double(CORBA::Double value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_double);
}
#endif

void
DynAnyImpl::insert_string(const char* value)
{
  if( !value || tckind() != CORBA::tk_string )
    throw CORBA::DynAny::InvalidValue();

  CORBA::ULong length = strlen(value) + 1;
  CORBA::ULong maxlen = tc()->NP_length();
  if( maxlen && length - 1 > maxlen )
    throw CORBA::DynAny::InvalidValue();

  Lock sync(this);
  MemBufferedStream& buf = doWrite(CORBA::tk_string);
  length >>= buf;
  buf.put_char_array((const CORBA::Char*)value, length);
}


void
DynAnyImpl::insert_reference(CORBA::Object_ptr value)
{
  if ( !CORBA::Object::PR_is_valid(value) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(value) )  throw CORBA::DynAny::InvalidValue();

  Lock sync(this);
  CORBA::Object::marshalObjRef(value, doWrite(CORBA::tk_objref));
}


void
DynAnyImpl::insert_typecode(CORBA::TypeCode_ptr value)
{
  if ( !CORBA::TypeCode::PR_is_valid(value) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(value) )  throw CORBA::DynAny::InvalidValue();

  Lock sync(this);
  CORBA::TypeCode::marshalTypeCode(value, doWrite(CORBA::tk_TypeCode));
}


void
DynAnyImpl::insert_any(const CORBA::Any& value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_any);
}


CORBA::Boolean
DynAnyImpl::get_boolean()
{
  Lock sync(this);
  CORBA::Boolean value;
  value <<= doRead(CORBA::tk_boolean);
  return value;
}


CORBA::Octet
DynAnyImpl::get_octet()
{
  Lock sync(this);
  CORBA::Octet value;
  value <<= doRead(CORBA::tk_octet);
  return value;
}


CORBA::Char
DynAnyImpl::get_char()
{
  Lock sync(this);
  CORBA::Char value;
  value <<= doRead(CORBA::tk_char);
  return value;
}


CORBA::Short
DynAnyImpl::get_short()
{
  Lock sync(this);
  CORBA::Short value;
  value <<= doRead(CORBA::tk_short);
  return value;
}


CORBA::UShort
DynAnyImpl::get_ushort()
{
  Lock sync(this);
  CORBA::UShort value;
  value <<= doRead(CORBA::tk_ushort);
  return value;
}


CORBA::Long
DynAnyImpl::get_long()
{
  Lock sync(this);
  CORBA::Long value;
  value <<= doRead(CORBA::tk_long);
  return value;
}


CORBA::ULong
DynAnyImpl::get_ulong()
{
  Lock sync(this);
  CORBA::ULong value;
  value <<= doRead(CORBA::tk_ulong);
  return value;
}

#ifndef NO_FLOAT
CORBA::Float
DynAnyImpl::get_float()
{
  Lock sync(this);
  CORBA::Float value;
  value <<= doRead(CORBA::tk_float);
  return value;
}


CORBA::Double
DynAnyImpl::get_double()
{
  Lock sync(this);
  CORBA::Double value;
  value <<= doRead(CORBA::tk_double);
  return value;
}
#endif

char*
DynAnyImpl::get_string()
{
  Lock sync(this);
  MemBufferedStream& buf = doRead(CORBA::tk_string);

  CORBA::ULong length;
  CORBA::ULong maxlen = tc()->NP_length();
  char* value;

  length <<= buf;
  if( maxlen && length - 1 > maxlen )  throw CORBA::DynAny::TypeMismatch();
  if( length == 0 )
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "DynAnyImpl::get_string() - string has zero length "
				  "(including terminator)");

  value = CORBA::string_alloc(length - 1);
  buf.get_char_array((CORBA::Char*)value, length);

  if( value[length - 1] != '\0' )
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "DynAnyImpl::get_string() - string is not terminated");

  return value;
}


CORBA::Object_ptr
DynAnyImpl::get_reference()
{
  Lock sync(this);
  return CORBA::Object::unmarshalObjRef(doRead(CORBA::tk_objref));
}


CORBA::TypeCode_ptr
DynAnyImpl::get_typecode()
{
  Lock sync(this);
  return CORBA::TypeCode::unmarshalTypeCode(doRead(CORBA::tk_TypeCode));
}


CORBA::Any*
DynAnyImpl::get_any()
{
  Lock sync(this);
  MemBufferedStream& buf = doRead(CORBA::tk_any);

  CORBA::Any* value = new CORBA::Any();
  *value <<= buf;
  return value;
}


CORBA::DynAny_ptr
DynAnyImpl::current_component()
{
  incrRefCount();
  return this;
}


CORBA::Boolean
DynAnyImpl::next()
{
  return 0;
}


CORBA::Boolean
DynAnyImpl::seek(CORBA::Long index)
{
  if( index == 0 )  return 1;
  else              return 0;
}


void
DynAnyImpl::rewind()
{
  // no-op
}

///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynAnyImpl::NP_nodetype() const
{
  return dt_any;
}


void*
DynAnyImpl::NP_narrow()
{
  return this;
}

//////////////
// internal //
//////////////

int
DynAnyImpl::copy_to(MemBufferedStream& mbs)
{
  tcParser tcp(pd_buf, tc());
  try {
    tcp.copyTo(mbs);
  }
  catch(CORBA::MARSHAL&) {
    return 0;
  }
  return 1;
}


int
DynAnyImpl::copy_from(MemBufferedStream& mbs)
{
  tcParser tcp(pd_buf, tc());
  try {
    setInvalid();
    tcp.copyFrom(mbs);
    setValid();
  }
  catch(CORBA::MARSHAL&) {
    return 0;
  }
  return 1;
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

CORBA::DynAny_ptr
DynEnumImpl::copy()
{
  DynEnumImpl* da = new DynEnumImpl(tc(), DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(...) {
    da->decrRefCount();
    throw;
  }
  return da;
}


char*
DynEnumImpl::value_as_string()
{
  // We must not return a null pointer since CORBA & IDL have
  // no concept of these things.

  CORBA::ULong val;
  {
    Lock sync(this);
    if( !isValid() )  return CORBA::string_dup("");
    pd_buf.rewind_in_mkr();
    val <<= pd_buf;
  }

  if( val >= tc()->NP_member_count() )  return CORBA::string_dup("");

  return CORBA::string_dup(tc()->NP_member_name(val));
}


void
DynEnumImpl::value_as_string(const char* value)
{
  if( !value )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  CORBA::Long index = tc()->NP_member_index(value);
  if( index < 0 )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  Lock sync(this);
  pd_buf.rewind_inout_mkr();
  CORBA::ULong(index) >>= pd_buf;
  setValid();
}


CORBA::ULong
DynEnumImpl::value_as_ulong()
{
  CORBA::ULong val;
  {
    Lock sync(this);
    if( !isValid() )  throw CORBA::SystemException(0, CORBA::COMPLETED_NO);
    pd_buf.rewind_in_mkr();
    val <<= pd_buf;
  }

  if( val >= tc()->NP_member_count() )
    throw CORBA::SystemException(0, CORBA::COMPLETED_NO);

  return val;
}


void
DynEnumImpl::value_as_ulong(CORBA::ULong value)
{
  if( value >= tc()->NP_member_count() )
    throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  Lock sync(this);
  pd_buf.rewind_inout_mkr();
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


void*
DynEnumImpl::NP_narrow()
{
  return this;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// DynFixedImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

#if 0

void*
DynFixedImpl::NP_narrow()
{
  return this;
}

#endif

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
    pd_components[i]->decrRefCount();
}

//////////////////////
// public interface //
//////////////////////

void
DynAnyConstrBase::assign(CORBA::DynAny_ptr da)
{
  if ( !CORBA::DynAny::PR_is_valid(da) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(da) )  throw CORBA::DynAny::Invalid();

  DynAnyImplBase* daib = ToDynAnyImplBase(da);
  if( !tc()->equal(daib->tc()) )  throw CORBA::DynAny::Invalid();

  // We do the copy via an intermediate buffer.

  MemBufferedStream buf;
  Lock sync(this);

  if( !daib->copy_to(buf) )  throw CORBA::DynAny::Invalid();

  if( !copy_from(buf) )
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "DynAnyConstrBase::assign() - copy_from failed unexpectedly");
}


void
DynAnyConstrBase::insert_boolean(CORBA::Boolean value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_boolean);
}


void
DynAnyConstrBase::insert_octet(CORBA::Octet value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_octet);
}


void
DynAnyConstrBase::insert_char(CORBA::Char value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_char);
}


void
DynAnyConstrBase::insert_short(CORBA::Short value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_short);
}


void
DynAnyConstrBase::insert_ushort(CORBA::UShort value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_ushort);
}


void
DynAnyConstrBase::insert_long(CORBA::Long value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_long);
}


void
DynAnyConstrBase::insert_ulong(CORBA::ULong value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_ulong);
}

#ifndef NO_FLOAT
void
DynAnyConstrBase::insert_float(CORBA::Float value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_float);
}


void
DynAnyConstrBase::insert_double(CORBA::Double value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_double);
}
#endif

void
DynAnyConstrBase::insert_string(const char* value)
{
  if( !value )  throw CORBA::DynAny::InvalidValue();
  CORBA::ULong length = strlen(value) + 1;

  Lock sync(this);
  if( pd_curr_index < 0 )  throw CORBA::DynAny::InvalidValue();

  TypeCode_base* tc = nthComponentTC(pd_curr_index);
  if( tc->NP_kind() != CORBA::tk_string )
    throw CORBA::DynAny::InvalidValue();

  CORBA::ULong maxlen = tc->NP_length();
  if( maxlen && length - 1 > maxlen )
    throw CORBA::DynAny::InvalidValue();

  MemBufferedStream& buf = writeCurrent(CORBA::tk_string);
  length >>= buf;
  buf.put_char_array((const CORBA::Char*)value, length);
}


void
DynAnyConstrBase::insert_reference(CORBA::Object_ptr value)
{
  if ( !CORBA::Object::PR_is_valid(value) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(value) )  throw CORBA::DynAny::InvalidValue();

  Lock sync(this);
  CORBA::Object::marshalObjRef(value, writeCurrent(CORBA::tk_objref));
}


void
DynAnyConstrBase::insert_typecode(CORBA::TypeCode_ptr value)
{
  if ( !CORBA::TypeCode::PR_is_valid(value) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(value) )  throw CORBA::DynAny::InvalidValue();

  Lock sync(this);
  CORBA::TypeCode::marshalTypeCode(value, writeCurrent(CORBA::tk_TypeCode));
}


void
DynAnyConstrBase::insert_any(const CORBA::Any& value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_any);
}


CORBA::Boolean
DynAnyConstrBase::get_boolean()
{
  Lock sync(this);
  CORBA::Boolean value;
  value <<= readCurrent(CORBA::tk_boolean);
  return value;
}


CORBA::Octet
DynAnyConstrBase::get_octet()
{
  Lock sync(this);
  CORBA::Octet value;
  value <<= readCurrent(CORBA::tk_octet);
  return value;
}


CORBA::Char
DynAnyConstrBase::get_char()
{
  Lock sync(this);
  CORBA::Char value;
  value <<= readCurrent(CORBA::tk_char);
  return value;
}


CORBA::Short
DynAnyConstrBase::get_short()
{
  Lock sync(this);
  CORBA::Short value;
  value <<= readCurrent(CORBA::tk_short);
  return value;
}


CORBA::UShort
DynAnyConstrBase::get_ushort()
{
  Lock sync(this);
  CORBA::UShort value;
  value <<= readCurrent(CORBA::tk_ushort);
  return value;
}


CORBA::Long
DynAnyConstrBase::get_long()
{
  Lock sync(this);
  CORBA::Long value;
  value <<= readCurrent(CORBA::tk_long);
  return value;
}


CORBA::ULong
DynAnyConstrBase::get_ulong()
{
  Lock sync(this);
  CORBA::ULong value;
  value <<= readCurrent(CORBA::tk_ulong);
  return value;
}

#ifndef NO_FLOAT
CORBA::Float
DynAnyConstrBase::get_float()
{
  Lock sync(this);
  CORBA::Float value;
  value <<= readCurrent(CORBA::tk_float);
  return value;
}


CORBA::Double
DynAnyConstrBase::get_double()
{
  Lock sync(this);
  CORBA::Double value;
  value <<= readCurrent(CORBA::tk_double);
  return value;
}
#endif

char*
DynAnyConstrBase::get_string()
{
  Lock sync(this);
  MemBufferedStream& buf = readCurrent(CORBA::tk_string);

  TypeCode_base* tc = nthComponentTC(pd_curr_index);
  CORBA::ULong maxlen = tc->NP_length();

  CORBA::ULong length;
  length <<= buf;

  if( maxlen && length - 1 > maxlen ) {
    pd_read_index = -1;
    throw CORBA::DynAny::TypeMismatch();
  }
  if( length == 0 )
    throw omniORB::fatalException(__FILE__,__LINE__,
      "DynAnyConstrBase::get_string() - zero length string in buffer");

  char* value = CORBA::string_alloc(length - 1);
  buf.get_char_array((CORBA::Char*)value, length);

  if( value[length - 1] != '\0' )
    throw omniORB::fatalException(__FILE__,__LINE__,
      "DynAnyConstrBase::get_string() - string not terminated");

  return value;
}


CORBA::Object_ptr
DynAnyConstrBase::get_reference()
{
  Lock sync(this);
  return CORBA::Object::unmarshalObjRef(readCurrent(CORBA::tk_objref));
}


CORBA::TypeCode_ptr
DynAnyConstrBase::get_typecode()
{
  Lock sync(this);
  return CORBA::TypeCode::unmarshalTypeCode(readCurrent(CORBA::tk_TypeCode));
}


CORBA::Any*
DynAnyConstrBase::get_any()
{
  Lock sync(this);
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


CORBA::DynAny_ptr
DynAnyConstrBase::current_component()
{
  Lock sync(this);

  if( pd_curr_index < 0 )  return CORBA::DynAny::_nil();
  DynAnyImplBase* da = getCurrent();
  da->incrRefCount();
  return da;
}


CORBA::Boolean
DynAnyConstrBase::next()
{
  Lock sync(this);

  if( pd_curr_index + 1 < (int)pd_n_components )  pd_curr_index++;
  else                                            pd_curr_index = -1;

  return pd_curr_index >= 0;
}


CORBA::Boolean
DynAnyConstrBase::seek(CORBA::Long index)
{
  Lock sync(this);

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
  Lock sync(this);

  pd_curr_index = pd_n_components ? 0 : -1;
}

//////////////
// internal //
//////////////

int
DynAnyConstrBase::copy_to(MemBufferedStream& mbs)
{
  if( pd_n_in_buf != pd_first_in_comp )  return 0;

  pd_buf.rewind_in_mkr();
  pd_read_index = -1;

  unsigned i;
  try {
    // Copy the components in the buffer.
    for( i = 0; i < pd_n_in_buf; i++ ) {
      TypeCode_base* ctc = nthComponentTC(i);
      tcParser tcp(pd_buf, ctc);
      tcp.copyTo(mbs, 0);
    }
  }
  catch(CORBA::MARSHAL&) {
    return 0;
  }

  // The rest are in external components.
  for( ; i < pd_n_components; i++ )
    if( !pd_components[i]->copy_to(mbs) )
      return 0;

  return 1;
}


int
DynAnyConstrBase::copy_from(MemBufferedStream& mbs)
{
  pd_buf.rewind_inout_mkr();
  pd_read_index = 0;

  unsigned i;
  try {
    // Copy components into the buffer.
    for( i = 0; i < pd_first_in_comp; i++ ) {
      TypeCode_base* ctc = nthComponentTC(i);
      tcParser tcp(pd_buf, ctc);
      tcp.copyFrom(mbs, 0);
    }
  }
  catch(CORBA::MARSHAL&) {
    pd_buf.rewind_inout_mkr();
    pd_n_in_buf = 0;
    pd_n_really_in_buf = 0;
    return 0;
  }

  // Copy into external components.
  for( ; i < pd_n_components; i++ )
    if( !pd_components[i]->copy_from(mbs) )
      return 0;

  pd_n_really_in_buf = pd_n_in_buf = pd_first_in_comp;
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
      pd_components[i]->decrRefCount();
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

    if( pd_first_in_comp == old_n_components )
      pd_first_in_comp = pd_n_components;
    else {
      // Add in the new components
      for( unsigned i = old_n_components; i < pd_n_components; i++ ) {
	CORBA::TypeCode_ptr tc =
	  CORBA::TypeCode::_duplicate(nthComponentTC(i));
	pd_components[i] = create_dyn_any(ToTcBase(tc), DYNANY_CHILD);
      }
    }
  }
  if( pd_curr_index == -1 && pd_n_components > 0 )
    pd_curr_index = 0;
  else if( pd_curr_index >= (int)pd_n_components )
    pd_curr_index = -1;
}


void
DynAnyConstrBase::createComponent(unsigned n)
{
  if( n >= pd_first_in_comp )  return;

  // Copy components out of the buffer.
  unsigned i;
  for( i = n; i < pd_n_in_buf; i++ ) {
    CORBA::TypeCode_ptr tc = CORBA::TypeCode::_duplicate(nthComponentTC(i));
    DynAnyImplBase* da = create_dyn_any(ToTcBase(tc), DYNANY_CHILD);
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
    pd_components[i] = create_dyn_any(ToTcBase(tc), DYNANY_CHILD);
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

  pd_buf.rewind_in_mkr();

  for( unsigned i = 0; i < n; i++ ) {
    TypeCode_base* ctc = nthComponentTC(i);
    try {
      tcParser::skip(pd_buf, ctc);
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
    tcParser* tcp = ((AnyP*)a.NP_pd())->getTC_parser();
    if( pd_read_index != (int)i )  seekTo(i);
    try {
      tcp->copyFrom(pd_buf);
    }
    catch(CORBA::MARSHAL&) {
      throw omniORB::fatalException(__FILE__,__LINE__,
	 "DynAnyConstrBase::component_to_any() - unexpected exception");
    }
    pd_read_index++;
    return 1;
  }
  else if( i >= pd_first_in_comp ) {
    MemBufferedStream& buf = ((AnyP*)a.NP_pd())->getMemBufferedStream();
    buf.rewind_inout_mkr();
    return pd_components[i]->copy_to(buf);
  }
  else
    return 0;
}


int
DynAnyConstrBase::component_from_any(unsigned i, const CORBA::Any& a)
{
  CORBA::TypeCode_var tc = a.type();
  if( !tc->equal(nthComponentTC(i)) )  return 0;

  if( canAppendComponent(i) ) {
    tcParser* tcp = ((AnyP*)a.NP_pd())->getTC_parser();
    try {
      tcp->copyTo(pd_buf);
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

  MemBufferedStream& buf = ((AnyP*)a.NP_pd())->getMemBufferedStream();
  buf.rewind_in_mkr();
  return pd_components[i]->copy_from(buf);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// DynStructImpl ///////////////////////////
//////////////////////////////////////////////////////////////////////

DynStructImpl::DynStructImpl(TypeCode_base* tc, CORBA::Boolean is_root)
  : DynAnyConstrBase(tc, dt_struct, is_root)
{
  setNumComponents(tc->NP_member_count());
}


DynStructImpl::~DynStructImpl()
{
}

//////////////////////
// public interface //
//////////////////////

CORBA::DynAny_ptr
DynStructImpl::copy()
{
  DynStructImpl* da = new DynStructImpl(tc(), DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(...) {
    da->decrRefCount();
    throw;
  }
  return da;
}


char*
DynStructImpl::current_member_name()
{
  Lock sync(this);

  if( pd_curr_index < 0 )
    throw CORBA::SystemException(0, CORBA::COMPLETED_NO);

  return CORBA::string_dup(tc()->NP_member_name(pd_curr_index));
}


CORBA::TCKind
DynStructImpl::current_member_kind()
{
  Lock sync(this);

  if( pd_curr_index < 0 )
    throw CORBA::SystemException(0, CORBA::COMPLETED_NO);

  return tc()->NP_member_type(pd_curr_index)->NP_kind();
}


CORBA::NameValuePairSeq*
DynStructImpl::get_members()
{
  CORBA::NameValuePairSeq* nvps = new CORBA::NameValuePairSeq();

  Lock sync(this);
  nvps->length(pd_n_components);

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    (*nvps)[i].id = CORBA::string_dup(tc()->NP_member_name(i));
    if( !component_to_any(i, (*nvps)[i].value) ) {
      delete nvps;
      throw CORBA::SystemException(0, CORBA::COMPLETED_NO);
    }
  }
  return nvps;
}


void
DynStructImpl::set_members(const CORBA::NameValuePairSeq& nvps)
{
  Lock sync(this);

  if( nvps.length() != pd_n_components )
    throw CORBA::DynAny::InvalidSeq();

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    // We don't bother to check the <id>
    if( !component_from_any(i, nvps[i].value) )
      throw CORBA::DynAny::InvalidSeq();
  }
}

///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynStructImpl::NP_nodetype() const
{
  return dt_struct;
}


void*
DynStructImpl::NP_narrow()
{
  return this;
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

  return tc()->NP_member_type(n);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// DynUnionImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

DynUnionImpl::DynUnionImpl(TypeCode_base* tc, CORBA::Boolean is_root)
  : DynAnyImplBase(tc, dt_union, is_root)
{
  CORBA::TypeCode_ptr tcdup =
    CORBA::TypeCode::_duplicate(tc->NP_discriminator_type());
  pd_disc = create_dyn_any_discriminator(ToTcBase(tcdup), this);
  pd_disc_type = ToTcBase(tcdup);
  pd_disc_kind = pd_disc_type->NP_kind();
  pd_disc_index = -1;

  pd_member = 0;
  pd_member_kind = CORBA::tk_null;

  pd_curr_index = 0;
}


DynUnionImpl::~DynUnionImpl()
{
  pd_disc->decrRefCount();
  if( pd_member )  pd_member->decrRefCount();
}

//////////////////////
// public interface //
//////////////////////

void
DynUnionImpl::assign(CORBA::DynAny_ptr da)
{
  if ( !CORBA::DynAny::PR_is_valid(da) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(da) )  throw CORBA::DynAny::Invalid();

  DynAnyImplBase* daib = ToDynAnyImplBase(da);
  if( !tc()->equal(daib->tc()) )  throw CORBA::DynAny::Invalid();

  // We do the copy via an intermediate buffer.

  MemBufferedStream buf;
  Lock sync(this);

  if( !daib->copy_to(buf) )  throw CORBA::DynAny::Invalid();

  if( !copy_from(buf) )
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "DynUnionImpl::assign() - copy_from failed unexpectedly");
}


CORBA::DynAny_ptr
DynUnionImpl::copy()
{
  DynUnionImpl* da = new DynUnionImpl(tc(), DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(...) {
    da->decrRefCount();
    throw;
  }
  return da;
}


void
DynUnionImpl::insert_boolean(CORBA::Boolean value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_boolean);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_octet(CORBA::Octet value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_octet);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_char(CORBA::Char value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_char);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_short(CORBA::Short value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_short);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_ushort(CORBA::UShort value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_ushort);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_long(CORBA::Long value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_long);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_ulong(CORBA::ULong value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_ulong);
  discriminatorHasChanged();
}

#ifndef NO_FLOAT
void
DynUnionImpl::insert_float(CORBA::Float value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_float);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_double(CORBA::Double value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_double);
  discriminatorHasChanged();
}
#endif

void
DynUnionImpl::insert_string(const char* value)
{
  if( !value )  throw CORBA::DynAny::InvalidValue();
  CORBA::ULong length = strlen(value) + 1;

  Lock sync(this);

  if( pd_curr_index != 1 || pd_member_kind != CORBA::tk_string )
    throw CORBA::DynAny::InvalidValue();
  CORBA::ULong maxlen = pd_member->tc()->NP_length();
  if( maxlen && length - 1 > maxlen )
    throw CORBA::DynAny::InvalidValue();

  MemBufferedStream& buf = writeCurrent(CORBA::tk_string);
  length >>= buf;
  buf.put_char_array((const CORBA::Char*)value, length);
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_reference(CORBA::Object_ptr value)
{
  if ( !CORBA::Object::PR_is_valid(value) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(value) )  throw CORBA::DynAny::InvalidValue();

  Lock sync(this);
  CORBA::Object::marshalObjRef(value, writeCurrent(CORBA::tk_objref));
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_typecode(CORBA::TypeCode_ptr value)
{
  if ( !CORBA::TypeCode::PR_is_valid(value) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(value) )  throw CORBA::DynAny::InvalidValue();

  Lock sync(this);
  CORBA::TypeCode::marshalTypeCode(value, writeCurrent(CORBA::tk_TypeCode));
  discriminatorHasChanged();
}


void
DynUnionImpl::insert_any(const CORBA::Any& value)
{
  Lock sync(this);
  value >>= writeCurrent(CORBA::tk_any);
  discriminatorHasChanged();
}


CORBA::Boolean
DynUnionImpl::get_boolean()
{
  Lock sync(this);
  CORBA::Boolean value;
  value <<= readCurrent(CORBA::tk_boolean);
  return value;
}


CORBA::Octet
DynUnionImpl::get_octet()
{
  Lock sync(this);
  CORBA::Octet value;
  value <<= readCurrent(CORBA::tk_octet);
  return value;
}


CORBA::Char
DynUnionImpl::get_char()
{
  Lock sync(this);
  CORBA::Char value;
  value <<= readCurrent(CORBA::tk_char);
  return value;
}


CORBA::Short
DynUnionImpl::get_short()
{
  Lock sync(this);
  CORBA::Short value;
  value <<= readCurrent(CORBA::tk_short);
  return value;
}


CORBA::UShort
DynUnionImpl::get_ushort()
{
  Lock sync(this);
  CORBA::UShort value;
  value <<= readCurrent(CORBA::tk_ushort);
  return value;
}


CORBA::Long
DynUnionImpl::get_long()
{
  Lock sync(this);
  CORBA::Long value;
  value <<= readCurrent(CORBA::tk_long);
  return value;
}


CORBA::ULong
DynUnionImpl::get_ulong()
{
  Lock sync(this);
  CORBA::ULong value;
  value <<= readCurrent(CORBA::tk_ulong);
  return value;
}

#ifndef NO_FLOAT
CORBA::Float
DynUnionImpl::get_float()
{
  Lock sync(this);
  CORBA::Float value;
  value <<= readCurrent(CORBA::tk_float);
  return value;
}


CORBA::Double
DynUnionImpl::get_double()
{
  Lock sync(this);
  CORBA::Double value;
  value <<= readCurrent(CORBA::tk_double);
  return value;
}
#endif

char*
DynUnionImpl::get_string()
{
  Lock sync(this);
  MemBufferedStream& buf = readCurrent(CORBA::tk_string);

  CORBA::ULong maxlen = pd_member->tc()->NP_length();

  CORBA::ULong length;
  length <<= buf;

  if( maxlen && length - 1 > maxlen )
    throw CORBA::DynAny::TypeMismatch();

  if( length == 0 )
    throw omniORB::fatalException(__FILE__,__LINE__,
      "DynUnionImpl::get_string() - zero length string in buffer");

  char* value = CORBA::string_alloc(length - 1);
  buf.get_char_array((CORBA::Char*)value, length);

  if( value[length - 1] != '\0' )
    throw omniORB::fatalException(__FILE__,__LINE__,
      "DynAnyConstrBase::get_string() - string not terminated");

  return value;
}


CORBA::Object_ptr
DynUnionImpl::get_reference()
{
  Lock sync(this);
  return CORBA::Object::unmarshalObjRef(readCurrent(CORBA::tk_objref));
}


CORBA::TypeCode_ptr
DynUnionImpl::get_typecode()
{
  Lock sync(this);
  return CORBA::TypeCode::unmarshalTypeCode(readCurrent(CORBA::tk_TypeCode));
}


CORBA::Any*
DynUnionImpl::get_any()
{
  Lock sync(this);
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

CORBA::DynAny_ptr
DynUnionImpl::current_component()
{
  Lock sync(this);

  switch( pd_curr_index ) {
  case 0:
    pd_disc->incrRefCount();
    return pd_disc;

  case 1:
    if( pd_member ) {
      pd_member->incrRefCount();
      return pd_member;
    } else
      return CORBA::DynAny::_nil();

  default:
    return CORBA::DynUnion::_nil();
  }
}


CORBA::Boolean
DynUnionImpl::next()
{
  Lock sync(this);

  if( pd_curr_index == 0 ) {
    pd_curr_index = 1;
    return 1;
  }
  else {
    pd_curr_index = -1;
    return 0;
  }
}


CORBA::Boolean
DynUnionImpl::seek(CORBA::Long index)
{
  Lock sync(this);

  switch( index ) {
  case 0:
    pd_curr_index = 0;
    return 1;
  case 1:
    pd_curr_index = 1;
    return 1;
  default:
    pd_curr_index = -1;
    return 0;
  }
}


void
DynUnionImpl::rewind()
{
  Lock sync(this);
  pd_curr_index = 0;
}


CORBA::Boolean
DynUnionImpl::set_as_default()
{
  CORBA::Long defaulti = tc()->NP_default_index();

  try {
    switch( defaulti ) {
    case TYPECODE_UNION_NO_DEFAULT:
      return 0;
    case TYPECODE_UNION_IMPLICIT_DEFAULT:
      {
	Lock sync(this);
	pd_disc->pd_buf.rewind_in_mkr();
	TypeCode_union::Discriminator disc_value =
	  TypeCode_union_helper::unmarshalLabel(pd_disc_type, pd_disc->pd_buf);
	return tc()->NP_index_from_discriminator(disc_value) < 0;
      }
    default:
      {
	Lock sync(this);
	pd_disc->pd_buf.rewind_in_mkr();
	TypeCode_union::Discriminator disc_value =
	  TypeCode_union_helper::unmarshalLabel(pd_disc_type, pd_disc->pd_buf);
	return tc()->NP_index_from_discriminator(disc_value) == defaulti;
      }
    }
  }
  catch(CORBA::MARSHAL&) {
    // Discriminator not initialised.
    return 0;
  }
}


void
DynUnionImpl::set_as_default(CORBA::Boolean set)
{
  if( !set )  return;

  CORBA::Long defaulti = tc()->NP_default_index();

  switch( defaulti ) {

  case TYPECODE_UNION_NO_DEFAULT:
    // Silently ignore it.
    return;

  case TYPECODE_UNION_IMPLICIT_DEFAULT:
  default:
    {
      // Set implicit default.
      pd_disc->set_value(tc()->NP_default_value());
      break;
    }
  }
}


CORBA::DynAny_ptr
DynUnionImpl::discriminator()
{
  // <pd_disc> never changes, so no need to lock.
  pd_disc->incrRefCount();
  return pd_disc;
}


CORBA::TCKind
DynUnionImpl::discriminator_kind()
{
  // <pd_disc> never changes, so no need to lock.
  return pd_disc_kind;
}


CORBA::DynAny_ptr
DynUnionImpl::member()
{
  Lock sync(this);

  if( pd_member ) {
    pd_member->incrRefCount();
    return pd_member;
  }
  else
    return CORBA::DynAny::_nil();
}


char*
DynUnionImpl::member_name()
{
  Lock sync(this);

  if( pd_member )
    return CORBA::string_dup(tc()->NP_member_name(pd_disc_index));
  else
    return CORBA::string_dup("");
}


void
DynUnionImpl::member_name(const char* name)
{
  if( !name )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  CORBA::ULong mcount = tc()->NP_member_count();

  for( CORBA::ULong i = 0; i < mcount; i++ )
    if( !strcmp(name, tc()->NP_member_name(i)) ) {
      // Set discriminator value ...
      pd_disc->set_value(tc()->NP_member_label_val(i));
      return;
    }

  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
}


CORBA::TCKind
DynUnionImpl::member_kind()
{
  Lock sync(this);
  return pd_member_kind;
}

///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynUnionImpl::NP_nodetype() const
{
  return dt_union;
}


void*
DynUnionImpl::NP_narrow()
{
  return this;
}

//////////////
// internal //
//////////////

int
DynUnionImpl::copy_to(MemBufferedStream& mbs)
{
  if( !pd_disc->copy_to(mbs) )  return 0;

  return !pd_member || pd_member->copy_to(mbs);
}


int
DynUnionImpl::copy_from(MemBufferedStream& mbs)
{
  if( !pd_disc->copy_from(mbs) )  return 0;

  discriminatorHasChanged();

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
    pd_disc->pd_buf.rewind_in_mkr();
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
  if( pd_member )  detachMember();

  pd_disc_index = tc()->NP_index_from_discriminator(newdisc);
  if( pd_disc_index < 0 ) {
    // Invalid label - implicit default.
    return;
  }

  // Create new member of the appropriate type.
  CORBA::TypeCode_ptr mtc = tc()->member_type(pd_disc_index);
  pd_member = create_dyn_any(ToTcBase(mtc), DYNANY_CHILD);
  pd_member_kind = mtc->kind();
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// DynUnionDisc /////////////////////////////
//////////////////////////////////////////////////////////////////////

DynUnionDisc::~DynUnionDisc()
{
}


void
DynUnionDisc::assign(CORBA::DynAny_ptr da)
{
  if ( !CORBA::DynAny::PR_is_valid(da) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(da) )  throw CORBA::DynAny::Invalid();

  DynAnyImplBase* daib = ToDynAnyImplBase(da);
  if( !tc()->equal(daib->tc()) )  throw CORBA::DynAny::Invalid();
  DynAnyImpl* dai = ToDynAnyImpl(daib);

  Lock sync(this);

  if( !dai->isValid() )  throw CORBA::DynAny::Invalid();

  dai->pd_buf.rewind_in_mkr();
  pd_buf.rewind_inout_mkr();
  pd_buf.copy_from(dai->pd_buf, dai->pd_buf.RdMessageUnRead());
  setValid();

  if( pd_union )  pd_union->discriminatorHasChanged();
}


CORBA::DynAny_ptr
DynUnionDisc::copy()
{
  // The copy does not have a link to the parent union.
  DynAnyImpl* da = new DynAnyImpl(tc(), dt_any, DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(...) {
    da->decrRefCount();
    throw;
  }
  return da;
}


void
DynUnionDisc::insert_boolean(CORBA::Boolean value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_boolean);
  if( pd_union )  pd_union->discriminatorHasChanged();
}


void
DynUnionDisc::insert_octet(CORBA::Octet value)
{
  // Not a legal discriminator type.
  throw CORBA::DynAny::InvalidValue();
}


void
DynUnionDisc::insert_char(CORBA::Char value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_char);
  if( pd_union )  pd_union->discriminatorHasChanged();
}


void
DynUnionDisc::insert_short(CORBA::Short value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_short);
  if( pd_union )  pd_union->discriminatorHasChanged();
}


void
DynUnionDisc::insert_ushort(CORBA::UShort value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_ushort);
  if( pd_union )  pd_union->discriminatorHasChanged();
}


void
DynUnionDisc::insert_long(CORBA::Long value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_long);
  if( pd_union )  pd_union->discriminatorHasChanged();
}


void
DynUnionDisc::insert_ulong(CORBA::ULong value)
{
  Lock sync(this);
  value >>= doWrite(CORBA::tk_ulong);
  if( pd_union )  pd_union->discriminatorHasChanged();
}

#ifndef NO_FLOAT
void
DynUnionDisc::insert_float(CORBA::Float value)
{
  // Not a legal discriminator type.
  throw CORBA::DynAny::InvalidValue();
}


void
DynUnionDisc::insert_double(CORBA::Double value)
{
  // Not a legal discriminator type.
  throw CORBA::DynAny::InvalidValue();
}
#endif

void
DynUnionDisc::insert_string(const char* value)
{
  // Not a legal discriminator type.
  throw CORBA::DynAny::InvalidValue();
}


void
DynUnionDisc::insert_reference(CORBA::Object_ptr value)
{
  // Not a legal discriminator type.
  throw CORBA::DynAny::InvalidValue();
}


void
DynUnionDisc::insert_typecode(CORBA::TypeCode_ptr value)
{
  // Not a legal discriminator type.
  throw CORBA::DynAny::InvalidValue();
}


void
DynUnionDisc::insert_any(const CORBA::Any& value)
{
  // Not a legal discriminator type.
  throw CORBA::DynAny::InvalidValue();
}

///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynUnionDisc::NP_nodetype() const
{
  return dt_disc;
}


void*
DynUnionDisc::NP_narrow()
{
  return this;
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
  // case CORBA::tk_wchar:
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

CORBA::DynAny_ptr
DynUnionEnumDisc::copy()
{
  // The copy does not have a link to the parent union.
  DynAnyImpl* da = new DynEnumImpl(tc(), DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(...) {
    da->decrRefCount();
    throw;
  }
  return da;
}


char*
DynUnionEnumDisc::value_as_string()
{
  // We must not return a null pointer since CORBA & IDL have
  // no concept of these things.

  CORBA::ULong val;
  {
    Lock sync(this);
    if( !isValid() )  return CORBA::string_dup("");
    pd_buf.rewind_in_mkr();
    val <<= pd_buf;
  }

  if( val >= tc()->NP_member_count() )  return CORBA::string_dup("");

  return CORBA::string_dup(tc()->NP_member_name(val));
}


void
DynUnionEnumDisc::value_as_string(const char* value)
{
  if( !value )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  CORBA::Long index = tc()->NP_member_index(value);
  if( index < 0 )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  Lock sync(this);
  pd_buf.rewind_inout_mkr();
  CORBA::ULong(index) >>= pd_buf;
  setValid();
  if( pd_union )  pd_union->discriminatorHasChanged();
}


CORBA::ULong
DynUnionEnumDisc::value_as_ulong()
{
  CORBA::ULong val;
  {
    Lock sync(this);
    if( !isValid() )  throw CORBA::SystemException(0, CORBA::COMPLETED_NO);
    pd_buf.rewind_in_mkr();
    val <<= pd_buf;
  }

  if( val >= tc()->NP_member_count() )
    throw CORBA::SystemException(0, CORBA::COMPLETED_NO);

  return val;
}


void
DynUnionEnumDisc::value_as_ulong(CORBA::ULong value)
{
  if( value >= tc()->NP_member_count() )
    throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  Lock sync(this);
  pd_buf.rewind_inout_mkr();
  value >>= pd_buf;
  setValid();
  if( pd_union )  pd_union->discriminatorHasChanged();
}

///////////////////////////////
// exposed private interface //
///////////////////////////////

int
DynUnionEnumDisc::NP_nodetype() const
{
  return dt_enumdisc;
}


void*
DynUnionEnumDisc::NP_narrow()
{
  return this;
}


void
DynUnionEnumDisc::set_value(TypeCode_union::Discriminator v)
{
  Lock sync(this);
  pd_buf.rewind_inout_mkr();
  CORBA::ULong(v) >>= pd_buf;
  setValid();
  if( pd_union )  pd_union->discriminatorHasChanged();
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// DynSequenceImpl //////////////////////////
//////////////////////////////////////////////////////////////////////

DynSequenceImpl::DynSequenceImpl(TypeCode_base* tc, CORBA::Boolean is_root)
  : DynAnyConstrBase(tc, dt_seq, is_root)
{
  pd_bound = tc->NP_length();
}


DynSequenceImpl::~DynSequenceImpl()
{
}

//////////////////////
// public interface //
//////////////////////

CORBA::DynAny_ptr
DynSequenceImpl::copy()
{
  DynSequenceImpl* da = new DynSequenceImpl(tc(), DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(...) {
    da->decrRefCount();
    throw;
  }
  return da;
}


CORBA::ULong
DynSequenceImpl::length()
{
  return pd_n_components;
}


void
DynSequenceImpl::length (CORBA::ULong value)
{
  if( pd_bound && value > pd_bound )
    throw CORBA::SystemException(0, CORBA::COMPLETED_NO);

  Lock sync(this);
  setNumComponents(value);
}


CORBA::AnySeq*
DynSequenceImpl::get_elements()
{
  Lock sync(this);

  CORBA::AnySeq* as = new CORBA::AnySeq();
  as->length(pd_n_components);

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    if( !component_to_any(i, (*as)[i]) ) {
      delete as;
      throw CORBA::SystemException(0, CORBA::COMPLETED_NO);
    }
  }
  return as;
}


void
DynSequenceImpl::set_elements(const CORBA::AnySeq& as)
{
  if( pd_bound && as.length() > pd_bound )
    throw CORBA::DynAny::InvalidSeq();

  Lock sync(this);

  if( as.length() != pd_n_components )
    setNumComponents(as.length());

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    if( !component_from_any(i, as[i]) )
      throw CORBA::DynAny::InvalidSeq();
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


void*
DynSequenceImpl::NP_narrow()
{
  return this;
}

//////////////
// internal //
//////////////

int
DynSequenceImpl::copy_to(MemBufferedStream& mbs)
{
  // Write the length of the sequence. This can't fail.
  CORBA::ULong(pd_n_components) >>= mbs;

  // Copy the elements.
  return DynAnyConstrBase::copy_to(mbs);
}


int
DynSequenceImpl::copy_from(MemBufferedStream& mbs)
{
  CORBA::ULong len;
  try {
    // Read the length of the sequence.
    len <<= mbs;
  }
  catch(CORBA::MARSHAL&) {
    return 0;
  }

  setNumComponents(len);

  return DynAnyConstrBase::copy_from(mbs);
}


TypeCode_base*
DynSequenceImpl::nthComponentTC(unsigned n)
{
  return tc()->NP_content_type();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// DynArrayImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

DynArrayImpl::DynArrayImpl(TypeCode_base* tc, CORBA::Boolean is_root)
  : DynAnyConstrBase(tc, dt_array, is_root)
{
  setNumComponents(tc->NP_length());
  
}


DynArrayImpl::~DynArrayImpl()
{
}

//////////////////////
// public interface //
//////////////////////

CORBA::DynAny_ptr
DynArrayImpl::copy()
{
  DynArrayImpl* da = new DynArrayImpl(tc(), DYNANY_ROOT);
  try {
    da->assign(this);
  }
  catch(...) {
    da->decrRefCount();
    throw;
  }
  return da;
}


CORBA::AnySeq*
DynArrayImpl::get_elements()
{
  CORBA::AnySeq* as = new CORBA::AnySeq();
  as->length(pd_n_components);

  Lock sync(this);

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    if( !component_to_any(i, (*as)[i]) ) {
      delete as;
      throw CORBA::SystemException(0, CORBA::COMPLETED_NO);
    }
  }
  return as;
}


void
DynArrayImpl::set_elements(const CORBA::AnySeq& as)
{
  if( as.length() != pd_n_components )
    throw CORBA::DynAny::InvalidSeq();

  Lock sync(this);

  for( unsigned i = 0; i < pd_n_components; i++ ) {
    if( !component_from_any(i, as[i]) )
      throw CORBA::DynAny::InvalidSeq();
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


void*
DynArrayImpl::NP_narrow()
{
  return this;
}

//////////////
// internal //
//////////////

TypeCode_base*
DynArrayImpl::nthComponentTC(unsigned n)
{
  return tc()->NP_content_type();
}

//////////////////////////////////////////////////////////////////////
/////////////////////////////// _narrow //////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::DynAny_ptr
CORBA::DynAny::_narrow(CORBA::DynAny_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if (!CORBA::is_nil(p))  return CORBA::DynAny::_duplicate(p);
  else                    return CORBA::DynAny::_nil();
}


CORBA::DynEnum_ptr
CORBA::DynEnum::_narrow(CORBA::DynAny_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( !CORBA::is_nil(p) ) {
    switch( p->NP_nodetype() ) {
    case dt_enum:
      CORBA::DynAny::_duplicate(p);
      return (DynEnumImpl*) p->NP_narrow();
    case dt_enumdisc:
      CORBA::DynAny::_duplicate(p);
      return (DynUnionEnumDisc*) p->NP_narrow();
    default:
      return CORBA::DynEnum::_nil();
    }
  }
  else
    return CORBA::DynEnum::_nil();
}


#if 0
CORBA::DynFixed_ptr
CORBA::DynFixed::_narrow(CORBA::DynAny_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( !CORBA::is_nil(p) && p->NP_nodetype() == dt_fixed ) {
    CORBA::DynAny::_duplicate(p);
    return (DynFixedImpl*) p->NP_narrow();
  }
  else
    return CORBA::DynFixed::_nil();
}
#endif


CORBA::DynStruct_ptr
CORBA::DynStruct::_narrow(CORBA::DynAny_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( !CORBA::is_nil(p) && p->NP_nodetype() == dt_struct ) {
    CORBA::DynAny::_duplicate(p);
    return (DynStructImpl*) p->NP_narrow();
  }
  else
    return CORBA::DynStruct::_nil();
}


CORBA::DynUnion_ptr
CORBA::DynUnion::_narrow(CORBA::DynAny_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( !CORBA::is_nil(p) && p->NP_nodetype() == dt_union ) {
    CORBA::DynAny::_duplicate(p);
    return (DynUnionImpl*) p->NP_narrow();
  }
  else
    return CORBA::DynUnion::_nil();
}


CORBA::DynSequence_ptr
CORBA::DynSequence::_narrow(CORBA::DynAny_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( !CORBA::is_nil(p) && p->NP_nodetype() == dt_seq ) {
    CORBA::DynAny::_duplicate(p);
    return (DynSequenceImpl*) p->NP_narrow();
  }
  else
    return CORBA::DynSequence::_nil();
}


CORBA::DynArray_ptr
CORBA::DynArray::_narrow(CORBA::DynAny_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( !CORBA::is_nil(p) && p->NP_nodetype() == dt_array ) {
    CORBA::DynAny::_duplicate(p);
    return (DynArrayImpl*) p->NP_narrow();
  }
  else
    return CORBA::DynArray::_nil();
}

//////////////////////////////////////////////////////////////////////
///////////////////////////// _duplicate /////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::DynAny_ptr
CORBA::DynAny::_duplicate(CORBA::DynAny_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( !CORBA::is_nil(p) ) {
    ToDynAnyImplBase(p)->incrRefCount();
    return p;
  }
  else
    return CORBA::DynAny::_nil();
}


CORBA::DynEnum_ptr
CORBA::DynEnum::_duplicate(CORBA::DynEnum_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if (!CORBA::is_nil(p)) {
    CORBA::DynAny::_duplicate(p);
    return p;
  }
  else
    return CORBA::DynEnum::_nil();
}


#if 0
CORBA::DynFixed_ptr
CORBA::DynFixed::_duplicate(CORBA::DynFixed_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if (!CORBA::is_nil(p)) {
    CORBA::DynAny::_duplicate(p);
    return p;
  }
  else
    return CORBA::DynFixed::_nil();
}
#endif


CORBA::DynStruct_ptr
CORBA::DynStruct::_duplicate(CORBA::DynStruct_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if (!CORBA::is_nil(p)) {
    CORBA::DynAny::_duplicate(p);
    return p;
  }
  else
    return CORBA::DynStruct::_nil();
}


CORBA::DynUnion_ptr
CORBA::DynUnion::_duplicate(CORBA::DynUnion_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if (!CORBA::is_nil(p)) {
    CORBA::DynAny::_duplicate(p);
    return p;
  }
  else
    return CORBA::DynUnion::_nil();
}


CORBA::DynSequence_ptr
CORBA::DynSequence::_duplicate(CORBA::DynSequence_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if (!CORBA::is_nil(p)) {
    CORBA::DynAny::_duplicate(p);
    return p;
  }
  else
    return CORBA::DynSequence::_nil();

}


CORBA::DynArray_ptr
CORBA::DynArray::_duplicate(CORBA::DynArray_ptr p)
{
  if ( !CORBA::DynAny::PR_is_valid(p) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if (!CORBA::is_nil(p)) {
    CORBA::DynAny::_duplicate(p);
    return p;
  }
  else
    return CORBA::DynArray::_nil();
}


void
CORBA::release(CORBA::DynAny_ptr p)
{
  if( CORBA::DynAny::PR_is_valid(p) && !CORBA::is_nil(p) )  
    ToDynAnyImplBase(p)->decrRefCount();
}

//////////////////////////////////////////////////////////////////////
///////////////////////////// CORBA::ORB /////////////////////////////
//////////////////////////////////////////////////////////////////////

// <tc> must be the alias-expanded TypeCode, and is consumed.
static DynAnyImplBase*
create_dyn_any(TypeCode_base* tc, CORBA::Boolean is_root)
{
  if ( !CORBA::TypeCode::PR_is_valid(tc) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(tc) )
    throw CORBA::BAD_TYPECODE(0, CORBA::COMPLETED_NO);

  DynAnyImplBase* da = 0;

  try {
    switch( tc->NP_kind() ) {
    case CORBA::tk_short:
    case CORBA::tk_long:
    case CORBA::tk_ushort:
    case CORBA::tk_ulong:
#ifndef NO_FLOAT
    case CORBA::tk_float:
    case CORBA::tk_double:
#endif
    case CORBA::tk_boolean:
    case CORBA::tk_char:
    // case CORBA::tk_wchar:
    case CORBA::tk_octet:
    case CORBA::tk_any:
    case CORBA::tk_TypeCode:
    case CORBA::tk_objref:
    case CORBA::tk_string:
      da = new DynAnyImpl(tc, dt_any, is_root);
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
      throw CORBA::DynAny::TypeMismatch();
    }
  }
  catch(...) {
    CORBA::release(tc);
    throw;
  }
  return da;
}


// <tc> must be the alias-expanded TypeCode, and is consumed.
static DynUnionDisc*
create_dyn_any_discriminator(TypeCode_base* tc, DynUnionImpl* du)
{
  if ( !CORBA::TypeCode::PR_is_valid(tc) )
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  if( CORBA::is_nil(tc) )
    throw CORBA::BAD_TYPECODE(0, CORBA::COMPLETED_NO);

  DynUnionDisc* da = 0;

  try {
    switch( tc->NP_kind() ) {
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


CORBA::DynAny_ptr
CORBA::ORB::create_dyn_any(const Any& value)
{
  CORBA::TypeCode_var tc = value.type();
  if( CORBA::is_nil(tc) )
    throw CORBA::BAD_TYPECODE(0, CORBA::COMPLETED_NO);

  TypeCode_base* aetc = TypeCode_base::aliasExpand(ToTcBase(tc));
  DynAnyImplBase* da = ::create_dyn_any(aetc, DYNANY_ROOT);
  da->from_any(value);
  return da;
}


CORBA::DynAny_ptr
CORBA::ORB::create_basic_dyn_any(TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc))
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  TypeCode_base* aetc = TypeCode_base::aliasExpand(ToTcBase_Checked(tc));

  switch( aetc->kind() ) {
  case CORBA::tk_short:
  case CORBA::tk_long:
  case CORBA::tk_ushort:
  case CORBA::tk_ulong:
#ifndef NO_FLOAT
  case CORBA::tk_float:
  case CORBA::tk_double:
#endif
  case CORBA::tk_boolean:
  case CORBA::tk_char:
    // case CORBA::tk_wchar:
  case CORBA::tk_octet:
  case CORBA::tk_any:
  case CORBA::tk_TypeCode:
  case CORBA::tk_objref:
  case CORBA::tk_string:
  case CORBA::tk_enum:
    break;
  default:
    throw CORBA::DynAny::TypeMismatch();
  }

  return new DynAnyImpl(aetc, dt_any, DYNANY_ROOT);
}


CORBA::DynStruct_ptr
CORBA::ORB::create_dyn_struct(TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc))
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  TypeCode_base* aetc = TypeCode_base::aliasExpand(ToTcBase_Checked(tc));

  if( aetc->kind() != CORBA::tk_struct && aetc->kind() != CORBA::tk_except ) {
    CORBA::release(aetc);
    throw CORBA::DynAny::TypeMismatch();
  }

  return new DynStructImpl(aetc, DYNANY_ROOT);
}


CORBA::DynSequence_ptr
CORBA::ORB::create_dyn_sequence(TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc))
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  TypeCode_base* aetc = TypeCode_base::aliasExpand(ToTcBase_Checked(tc));

  if( aetc->kind() != CORBA::tk_sequence ) {
    CORBA::release(aetc);
    throw CORBA::DynAny::TypeMismatch();
  }

  return new DynSequenceImpl(aetc, DYNANY_ROOT);
}


CORBA::DynArray_ptr
CORBA::ORB::create_dyn_array(TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc))
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  TypeCode_base* aetc = TypeCode_base::aliasExpand(ToTcBase_Checked(tc));

  if( aetc->kind() != CORBA::tk_array ) {
    CORBA::release(aetc);
    throw CORBA::DynAny::TypeMismatch();
  }

  return new DynArrayImpl(aetc, DYNANY_ROOT);
}


CORBA::DynUnion_ptr
CORBA::ORB::create_dyn_union(TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc))
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  TypeCode_base* aetc = TypeCode_base::aliasExpand(ToTcBase_Checked(tc));

  if( aetc->kind() != CORBA::tk_union ) {
    CORBA::release(aetc);
    throw CORBA::DynAny::TypeMismatch();
  }

  return new DynUnionImpl(aetc, DYNANY_ROOT);
}


CORBA::DynEnum_ptr
CORBA::ORB::create_dyn_enum(TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc))
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  TypeCode_base* aetc = TypeCode_base::aliasExpand(ToTcBase_Checked(tc));

  if( aetc->kind() != CORBA::tk_enum ) {
    CORBA::release(aetc);
    throw CORBA::DynAny::TypeMismatch();
  }

  return new DynEnumImpl(aetc, DYNANY_ROOT);
}


#if 0
CORBA::DynFixed_ptr
CORBA::ORB::create_dyn_fixed(TypeCode_ptr tc)
{
  if (!CORBA::TypeCode::PR_is_valid(tc))
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  TypeCode_base* aetc = TypeCode_base::aliasExpand(ToTcBase_Checked(tc));

  if( aetc->kind() != CORBA::tk_fixed )
    CORBA::release(aetc);
    throw CORBA::DynAny::TypeMismatch();

  return new DynImpl(aetc, DYNANY_ROOT);
}
#endif
