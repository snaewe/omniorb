// -*- Mode: C++; -*-
//                            Package   : omniORB2
// DynAny.cc                  Created on: 12/02/98
//                            Author    : Sai-Lai Lo (sll)
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
//      Implementation of type DynAny


/* 
   $Log$
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


#include <omniORB2/CORBA.h>
#include <tcParseEngine.h>
#include <dynAnyP.h>

#define CvtDynAnyP()     ((dynAnyP*)(NP_pd()))
#define CvtDynAnyP2(p) ((dynAnyP*)((p)->NP_pd()))

CORBA::DynAny_ptr
CORBA::
ORB::create_dyn_any(const CORBA::Any& value)
{
  CORBA::TypeCode_var t = value.type();
  if (CORBA::is_nil(t)) 
    throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_MAYBE);
  
  t = t->NP_aliasExpand();

  CORBA::DynAny_ptr retval;
  dynAnyP* n;

  switch (t->kind())
    {
    case CORBA::tk_boolean:
    case CORBA::tk_char:
    case CORBA::tk_octet:
    case CORBA::tk_short:
    case CORBA::tk_ushort:
    case CORBA::tk_long:
    case CORBA::tk_ulong:
#ifndef NO_FLOAT
    case CORBA::tk_float:
    case CORBA::tk_double:
#endif
    case CORBA::tk_string:
    case CORBA::tk_objref:
    case CORBA::tk_TypeCode:
    case CORBA::tk_any:
      n = new dynAnyP(t);
      retval = new DynAny(n,CORBA::DynAny::dt_any);
      break;
    case CORBA::tk_enum:
      n = new dynAnyP(t);
      retval = new DynEnum(n);
      break;
#if 0
    case CORBA::tk_fixed:
      n = new dynAnyP(t);
      retval = new DynFixed(n);
      break;
#endif
    case CORBA::tk_struct:
    case CORBA::tk_except:
      n = new dynAnyP(t);
      retval = new DynStruct(n);
      break;
    case CORBA::tk_union:
      n = new dynAnyP(t);
      retval = new DynUnion(n);
      break;
    case CORBA::tk_sequence:
      n = new dynAnyP(t);
      retval = new DynSequence(n);
      break;
    case CORBA::tk_array:
      n = new dynAnyP(t);
      retval = new DynArray(n);
      break;
    default:
      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_MAYBE);
    }

  try {
    retval->from_any(value);
  }
  catch (...) {
    CORBA::release(retval);
    throw;
  }
  return retval;
}

////////////////////////////////////////////////////////////////////////////
//     DynAny                                                             //
////////////////////////////////////////////////////////////////////////////


CORBA::
DynAny::~DynAny()
{
}

CORBA::DynAny_ptr
CORBA::
ORB::create_basic_dyn_any(CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var t = tc->NP_aliasExpand();
  switch (t->kind()) {
  case CORBA::tk_boolean:
  case CORBA::tk_char:
  case CORBA::tk_octet:
  case CORBA::tk_short:
  case CORBA::tk_ushort:
  case CORBA::tk_long:
  case CORBA::tk_ulong:
#ifndef NO_FLOAT
  case CORBA::tk_float:
  case CORBA::tk_double:
#endif
  case CORBA::tk_string:
  case CORBA::tk_objref:
  case CORBA::tk_TypeCode:
  case CORBA::tk_any:
      break;
  default:
    throw CORBA::ORB::InconsistentTypeCode();    
  }
  dynAnyP* n = new dynAnyP(t);
  return new DynAny(n,CORBA::DynAny::dt_any);
}


CORBA::TypeCode_ptr 
CORBA::DynAny::type() const
{
  return CORBA::TypeCode::_duplicate(CvtDynAnyP()->tc());
}

void 
CORBA::DynAny::assign(CORBA::DynAny_ptr dyn_any)
{
  if (NP_nodetype() != dyn_any->NP_nodetype())
    throw CORBA::DynAny::Invalid();

  omni_mutex_lock sync(dynAnyP::lock);
  CORBA::Any v;
  CvtDynAnyP2(dyn_any)->toAny(v);
  NP_real_from_any(v);
}


void
CORBA::DynAny::from_any(const CORBA::Any& value) 
{
  omni_mutex_lock sync(dynAnyP::lock);
  NP_real_from_any(value);
}


// NP_real_from_any is used in internal recursions. Caller to this function
// already hold mutex on dynAnyP::lock.
void
CORBA::DynAny::NP_real_from_any(const CORBA::Any& value) 
{
  CvtDynAnyP()->fromAny(value);
}


CORBA::Any* 
CORBA::DynAny::to_any()
{
  omni_mutex_lock sync(dynAnyP::lock);
  CORBA::Any* retval = new CORBA::Any;
  try {
    CvtDynAnyP()->toAny(*retval);
  }
  catch (...) {
    delete retval;
    throw;
  }
  return retval;
}


void 
CORBA::DynAny::destroy() 
{
  // Do nothing. Let reference counting to do garbage collection.
}


CORBA::DynAny_ptr
CORBA::DynAny::copy()
{
  CORBA::DynAny_ptr retval;
  omni_mutex_lock sync(dynAnyP::lock);

  dynAnyP* n = new dynAnyP(CvtDynAnyP()->tc());

  switch (NP_nodetype()) {
  case dt_any:
    retval = new DynAny(n,CORBA::DynAny::dt_any);
    break;
  case dt_enum:
    retval = new DynEnum(n);
    break;
  case dt_fixed:
#if 0
    retval = new DynFixed(n);
#else
    // XXX- leaking n.
    return CORBA::DynAny::_nil();
#endif
    break;
  case dt_struct:
    retval = new DynStruct(n);
    break;
  case dt_union:
    retval = new DynUnion(n);
    break;
  case dt_seq:
    retval = new DynSequence(n);
    break;
  case dt_array:
    retval = new DynArray(n);
    break;
  }
  try {
    CORBA::Any v;
    CvtDynAnyP()->toAny(v);
    retval->NP_real_from_any(v);
  }
  catch(...) {
    CORBA::release(retval);
    throw;
  }
  return retval;
}

void
CORBA::DynAny::insert_boolean(CORBA::Boolean value) 
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.bv = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_boolean,v);
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

void 
CORBA::DynAny::insert_octet(CORBA::Octet value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.ov = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_octet,v);
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

void 
CORBA::DynAny::insert_char(CORBA::Char value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)

    {
      dynAnyP::Bval v;
      v.cv = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_char,v); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

void 
CORBA::DynAny::insert_short(CORBA::Short value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.sv = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_short,v); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

void 
CORBA::DynAny::insert_ushort(CORBA::UShort value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.usv = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_ushort,v); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

void 
CORBA::DynAny::insert_long(CORBA::Long value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.lv = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_long,v); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

void 
CORBA::DynAny::insert_ulong(CORBA::ULong value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.ulv = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_ulong,v); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

#if !defined(NO_FLOAT)
void 
CORBA::DynAny::insert_float(CORBA::Float value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.fv = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_float,v); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

void 
CORBA::DynAny::insert_double(CORBA::Double value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.dv = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_double,v); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}
#endif

void 
CORBA::DynAny::insert_string(const char* value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.stiv = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_string,v); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

void 
CORBA::DynAny::insert_reference(CORBA::Object_ptr value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.objv = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_objref,v); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

void 
CORBA::DynAny::insert_typecode(CORBA::TypeCode_ptr value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.tcv = value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_TypeCode,v); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

void 
CORBA::DynAny::insert_any(const CORBA::Any& value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      dynAnyP::Bval v;
      v.anyv = (CORBA::Any*) &value;
      CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_any,v); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::InvalidValue();
}

CORBA::Boolean 
CORBA::DynAny::get_boolean()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_boolean,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.bv;
}


CORBA::Octet 
CORBA::DynAny::get_octet()
{  
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_octet,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.ov;
}

CORBA::Char 
CORBA::DynAny::get_char()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_char,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.cv;
}

CORBA::Short 
CORBA::DynAny::get_short()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_short,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.sv;
}

CORBA::UShort 
CORBA::DynAny::get_ushort()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_ushort,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.usv;
}

CORBA::Long 
CORBA::DynAny::get_long()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_long,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.lv;
}

CORBA::ULong 
CORBA::DynAny::get_ulong()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_ulong,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.ulv;
}

#if !defined(NO_FLOAT)
CORBA::Float 
CORBA::DynAny::get_float()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_float,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.fv;
}

CORBA::Double 
CORBA::DynAny::get_double()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_double,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.dv;
}
#endif

char* 
CORBA::DynAny::get_string()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_string,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.stov;
}

CORBA::Object_ptr 
CORBA::DynAny::get_reference()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_objref,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.objv;
}

CORBA::TypeCode_ptr 
CORBA::DynAny::get_typecode()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_TypeCode,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.tcv;
}

CORBA::Any* 
CORBA::DynAny::get_any()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() >= 0)
    {
      CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_any,retval); 
      if (NP_nodetype() != dt_any)
	(void) CvtDynAnyP()->nextComponent();
    }
  else
    throw CORBA::DynAny::TypeMismatch();
  return retval.anyv;
}


CORBA::DynAny_ptr 
CORBA::DynAny::current_component() 
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (CvtDynAnyP()->currentComponentIndex() < 0)
    return CORBA::DynAny::_nil();

  return CvtDynAnyP()->currentComponent();
}
    

CORBA::Boolean 
CORBA::DynAny::next()
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (NP_nodetype() == dt_any || 
      NP_nodetype() == dt_enum ||
      CvtDynAnyP()->currentComponentIndex() < 0)
    return 0;

  return CvtDynAnyP()->nextComponent();
}


CORBA::Boolean 
CORBA::DynAny::seek(CORBA::Long index)
{
  omni_mutex_lock sync(dynAnyP::lock);
  if (index >= CvtDynAnyP()->totalComponents())
    return 0;
  CvtDynAnyP()->nthComponent(index);
  return 1;
}


void 
CORBA::DynAny::rewind()
{
  omni_mutex_lock sync(dynAnyP::lock);
  CvtDynAnyP()->nthComponent(0);
}


////////////////////////////////////////////////////////////////////////////
//     DynEnum                                                            //
////////////////////////////////////////////////////////////////////////////

CORBA::DynEnum_ptr
CORBA::
ORB::create_dyn_enum(CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var t = tc->NP_aliasExpand();
  if (t->kind() != CORBA::tk_enum)
    throw CORBA::ORB::InconsistentTypeCode();

  dynAnyP* n = new dynAnyP(t);
  return new DynEnum(n);
}

char*
CORBA::
DynEnum::value_as_string()
{
  dynAnyP::Bval val;
  omni_mutex_lock sync(dynAnyP::lock);
  CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_enum,val); 

  CORBA::TypeCode_ptr tc = CvtDynAnyP()->tc();
  CORBA::ULong numMems = tc->member_count();
  CORBA::ULong retval = 0;
  const char* ts = tc->member_name(val.ulv);
  return CORBA::string_dup(ts);
}

void
CORBA::
DynEnum::value_as_string(const char* value)
{
  CORBA::TypeCode_ptr tc = CvtDynAnyP()->tc();
  CORBA::ULong numMems = tc->member_count();
  for(CORBA::ULong count = 0; count < numMems; count++)
    {
      if (strcmp(value,tc->member_name(count)) == 0)
	{
	  value_as_ulong(count);
	  return;
	}
    }
  throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
}

CORBA::ULong
CORBA::
DynEnum::value_as_ulong()
{
  dynAnyP::Bval retval;
  omni_mutex_lock sync(dynAnyP::lock);
  CvtDynAnyP()->currentComponentToBasetype(CORBA::tk_enum,retval); 
  return retval.ulv;
}

void
CORBA::
DynEnum::value_as_ulong(CORBA::ULong value)
{
  if (value >= CvtDynAnyP()->tc()->member_count()) {
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
  }
  omni_mutex_lock sync(dynAnyP::lock);
  dynAnyP::Bval v;
  v.ulv = value;
  CvtDynAnyP()->currentComponentFromBasetype(CORBA::tk_enum,v); 
}

////////////////////////////////////////////////////////////////////////////
//     DynStruct                                                          //
////////////////////////////////////////////////////////////////////////////

CORBA::DynStruct_ptr
CORBA::
ORB::create_dyn_struct(CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var t = tc->NP_aliasExpand();
  if (t->kind() != CORBA::tk_struct &&
      t->kind() != CORBA::tk_except)
    throw CORBA::ORB::InconsistentTypeCode();

  dynAnyP* n = new dynAnyP(t);
  return new DynStruct(n);
}

char*
CORBA::
DynStruct::current_member_name()
{
  omni_mutex_lock sync(dynAnyP::lock);

  int index = CvtDynAnyP()->currentComponentIndex();
  if (index < 0)
    return 0;

  CORBA::TypeCode_ptr t = CvtDynAnyP()->tc();
  const char* ts = t->member_name(index);
  return CORBA::string_dup(ts);
}

CORBA::TCKind
CORBA::
DynStruct::current_member_kind()
{
  omni_mutex_lock sync(dynAnyP::lock);

  int index = CvtDynAnyP()->currentComponentIndex();
  if (index < 0)
    return CORBA::tk_null;

  CORBA::TypeCode_ptr t = CvtDynAnyP()->tc();
  CORBA::TypeCode_var tv = t->member_type(index);
  if (tv->kind() == CORBA::tk_alias) {
    tv = tv->NP_aliasExpand();
  }
  return tv->kind();
}

CORBA::NameValuePairSeq*
CORBA::
DynStruct::get_members()
{
  omni_mutex_lock sync(dynAnyP::lock);
  CORBA::NameValuePairSeq* retval = new CORBA::NameValuePairSeq;
  try {
    CORBA::ULong total = CvtDynAnyP()->totalComponents();
    retval->length(total);
    CvtDynAnyP()->nthComponent(0);
    for (CORBA::ULong index = 0; index < total; index++) {
      CvtDynAnyP()->currentComponentToAny((*retval)[index].value);
      (*retval)[index].id = CORBA::string_dup(CvtDynAnyP()->tc()
					      ->member_name(index));
      (void) CvtDynAnyP()->nextComponent();
    }
  }
  catch (...) {
    delete retval;
    throw;
  }
  return retval;
}

void
CORBA::
DynStruct::set_members(const CORBA::NameValuePairSeq& v)
{
  omni_mutex_lock sync(dynAnyP::lock);
  CORBA::ULong total = v.length();
  if (CvtDynAnyP()->MaxComponents() && total > CvtDynAnyP()->MaxComponents())
    throw CORBA::DynAny::InvalidSeq();
  try {
    CvtDynAnyP()->nthComponent(0);
    for (CORBA::ULong index = 0; index < total; index++) {
      if (strcmp(CvtDynAnyP()->tc()->member_name(index),v[index].id) != 0)
	throw CORBA::DynAny::InvalidSeq();
      CvtDynAnyP()->currentComponentFromAny((CORBA::Any&)v[index].value);
      (void) CvtDynAnyP()->nextComponent();
    }
  }
  catch(...) {
    throw CORBA::DynAny::InvalidSeq();
  }
}


////////////////////////////////////////////////////////////////////////////
//     DynUnion                                                           //
////////////////////////////////////////////////////////////////////////////

CORBA::DynUnion_ptr
CORBA::
ORB::create_dyn_union(CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var t = tc->NP_aliasExpand();
  if (t->kind() != CORBA::tk_union)
    throw CORBA::ORB::InconsistentTypeCode();

  dynAnyP* n = new dynAnyP(t);
  return new DynUnion(n);
}


// CORBA::Boolean CORBA::DynUnion::set_as_default();
// void CORBA::DynUnion::set_as_default(CORBA::Boolean value);
//    - implementation is in dynAnyP.cc

CORBA::DynAny_ptr 
CORBA::
DynUnion::discriminator()
{
  omni_mutex_lock sync(dynAnyP::lock);
  CvtDynAnyP()->nthComponent(0);
  return CvtDynAnyP()->currentComponent();
}

CORBA::TCKind
CORBA::
DynUnion::discriminator_kind()
{
  omni_mutex_lock sync(dynAnyP::lock);
  CORBA::TypeCode_ptr t = CvtDynAnyP()->tc();
  CORBA::TypeCode_var d = t->discriminator_type();
  if (d->kind() == CORBA::tk_alias) {
    d = d->NP_aliasExpand();
  }
  return d->kind();
}

CORBA::DynAny_ptr
CORBA::
DynUnion::member()
{
  omni_mutex_lock sync(dynAnyP::lock);
  CvtDynAnyP()->nthComponent(1);
  return CvtDynAnyP()->currentComponent();
}


CORBA::TCKind
CORBA::
DynUnion::member_kind()
{
  omni_mutex_lock sync(dynAnyP::lock);
  CvtDynAnyP()->nthComponent(1);
  DynAny_var m = CvtDynAnyP()->currentComponent();
  if (CORBA::is_nil(m))
    return CORBA::tk_null;
  CORBA::TypeCode_ptr t = CvtDynAnyP2(m)->tc();
  if (t->kind() == CORBA::tk_alias) {
    CORBA::TypeCode_var tv = t->NP_aliasExpand();
    return tv->kind();
  }
  else {
    return t->kind();
  }
}

char* 
CORBA::
DynUnion::member_name()
{
  omni_mutex_lock sync(dynAnyP::lock);
  CvtDynAnyP()->nthComponent(0);
  DynAny_var v = CvtDynAnyP()->currentComponent();
  int mpos;
  if (v->NP_nodetype() == dt_any) {
    // Discriminator is a scaler.
    // The dynAny *MUST BE* dynAnyUD1.
    // Make sure that this is in sync with the dynAnyP implementation for
    // union.
    dynAnyUD1* p = (dynAnyUD1*) ((CORBA::DynAny_ptr)v);
    mpos = p->get_member();
  }
  else {
    // Discriminator is a enum.
    // The dynAny *MUST BE* dynAnyUD2.
    // Make sure that this is in sync with the dynAnyP implementation for
    // union.
    dynAnyUD2* p = (dynAnyUD2*) ((CORBA::DynAny_ptr)v);
    mpos = p->get_member();
  }
  if (mpos < 0)
    return 0;
  return CORBA::string_dup(CvtDynAnyP()->tc()->member_name(mpos));
}

void
CORBA::
DynUnion::member_name(const char* value)
{
  if (!value) return;
  omni_mutex_lock sync(dynAnyP::lock);
  CORBA::TypeCode_ptr t = CvtDynAnyP()->tc();
  CORBA::ULong total = t->member_count();
  CORBA::ULong pos;
  for (pos=0; pos < total; pos++)
    {
      const char* n = t->member_name(pos);
      if (strcmp(value,n) == 0)
	break;
    }
  if (pos >= total) {
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
  }
  CvtDynAnyP()->nthComponent(0);
  DynAny_var v = CvtDynAnyP()->currentComponent();
  if (v->NP_nodetype() == dt_any) {
    // Discriminator is a scaler.
    // The dynAny *MUST BE* dynAnyUD1.
    // Make sure that this is in sync with the dynAnyP implementation for
    // union.
    dynAnyUD1* p = (dynAnyUD1*) ((CORBA::DynAny_ptr)v);
    p->set_member(pos);
  }
  else {
    // Discriminator is a enum.
    // The dynAny *MUST BE* dynAnyUD2.
    // Make sure that this is in sync with the dynAnyP implementation for
    // union.
    dynAnyUD2* p = (dynAnyUD2*) ((CORBA::DynAny_ptr)v);
    p->set_member(pos);
  }
}

////////////////////////////////////////////////////////////////////////////
//     DynSequence                                                        //
////////////////////////////////////////////////////////////////////////////

CORBA::DynSequence_ptr 
CORBA::
ORB::create_dyn_sequence(CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var t = tc->NP_aliasExpand();
  if (t->kind() != CORBA::tk_sequence)
    throw CORBA::ORB::InconsistentTypeCode();

  dynAnyP* n = new dynAnyP(t);
  return new DynSequence(n);
}

CORBA::ULong
CORBA::
DynSequence::length()
{
  omni_mutex_lock sync(dynAnyP::lock);
  return CvtDynAnyP()->totalComponents();
}

void
CORBA::
DynSequence::length (CORBA::ULong value)
{
  omni_mutex_lock sync(dynAnyP::lock);

  if (CvtDynAnyP()->MaxComponents() && value > CvtDynAnyP()->MaxComponents())
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  CvtDynAnyP()->totalComponents(value);
}

CORBA::AnySeq*
CORBA::
DynSequence::get_elements()
{
  omni_mutex_lock sync(dynAnyP::lock);
  CORBA::AnySeq* retval = new CORBA::AnySeq;
  int savepos = CvtDynAnyP()->currentComponentIndex();
  try {
    CORBA::ULong total = CvtDynAnyP()->totalComponents();
    retval->length(total);
    CvtDynAnyP()->nthComponent(0);
    for (CORBA::ULong index = 0; index < total; index++) {
      CvtDynAnyP()->currentComponentToAny((*retval)[index]);
      (void) CvtDynAnyP()->nextComponent();
    }
  }
  catch (...) {
    if (savepos >= 0) CvtDynAnyP()->nthComponent((CORBA::ULong)savepos);
    delete retval;
    throw;
  }
  if (savepos >= 0) CvtDynAnyP()->nthComponent((CORBA::ULong)savepos);
  return retval;
}

void
CORBA::
DynSequence::set_elements(const CORBA::AnySeq& value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  int savepos = CvtDynAnyP()->currentComponentIndex();
  CORBA::ULong total = value.length();
  if (CvtDynAnyP()->MaxComponents() && total > CvtDynAnyP()->MaxComponents())
    throw CORBA::DynAny::InvalidSeq();
  try {
    CvtDynAnyP()->totalComponents(total);
    CvtDynAnyP()->nthComponent(0);
    for (CORBA::ULong index = 0; index < total; index++) {
      CvtDynAnyP()->currentComponentFromAny((CORBA::Any&)value[index]);
      (void) CvtDynAnyP()->nextComponent();
    }
  }
  catch(...) {
    if (savepos >= 0) CvtDynAnyP()->nthComponent((CORBA::ULong)savepos);
    throw CORBA::DynAny::InvalidSeq();
  }
  if (savepos >= 0) CvtDynAnyP()->nthComponent((CORBA::ULong)savepos);
}


////////////////////////////////////////////////////////////////////////////
//     DynArray                                                           //
////////////////////////////////////////////////////////////////////////////

CORBA::DynArray_ptr
CORBA::
ORB::create_dyn_array(CORBA::TypeCode_ptr tc)
{
  CORBA::TypeCode_var t = tc->NP_aliasExpand();
  if (t->kind() != CORBA::tk_array)
    throw CORBA::ORB::InconsistentTypeCode();

  dynAnyP* n = new dynAnyP(t);
  return new DynArray(n);
}

CORBA::AnySeq* 
CORBA::
DynArray::get_elements()
{
  omni_mutex_lock sync(dynAnyP::lock);
  CORBA::AnySeq* retval = new CORBA::AnySeq;
  int savepos = CvtDynAnyP()->currentComponentIndex();
  try {
    CORBA::ULong total = CvtDynAnyP()->totalComponents();
    retval->length(total);
    CvtDynAnyP()->nthComponent(0);
    for (CORBA::ULong index = 0; index < total; index++) {
      CvtDynAnyP()->currentComponentToAny((*retval)[index]);
      (void) CvtDynAnyP()->nextComponent();
    }
  }
  catch (...) {
    if (savepos >= 0) CvtDynAnyP()->nthComponent((CORBA::ULong)savepos);
    delete retval;
    throw;
  }
  if (savepos >= 0) CvtDynAnyP()->nthComponent((CORBA::ULong)savepos);
  return retval;
}

void
CORBA::
DynArray::set_elements(const CORBA::AnySeq& value)
{
  omni_mutex_lock sync(dynAnyP::lock);
  int savepos = CvtDynAnyP()->currentComponentIndex();
  CORBA::ULong total = value.length();
  if (total != CvtDynAnyP()->MaxComponents())
    throw CORBA::DynAny::InvalidSeq();
  try {
    CvtDynAnyP()->nthComponent(0);
    for (CORBA::ULong index = 0; index < total; index++) {
      CvtDynAnyP()->currentComponentFromAny((CORBA::Any&)value[index]);
      (void) CvtDynAnyP()->nextComponent();
    }
  }
  catch(...) {
    if (savepos >= 0) CvtDynAnyP()->nthComponent((CORBA::ULong)savepos);
    throw CORBA::DynAny::InvalidSeq();
  }
  if (savepos >= 0) CvtDynAnyP()->nthComponent((CORBA::ULong)savepos);
}

#if 0
////////////////////////////////////////////////////////////////////////////
//     DynFixed                                                           //
////////////////////////////////////////////////////////////////////////////


CORBA::DynFixed_ptr
CORBA::
ORB::create_dyn_fixed(CORBA::TypeCode_ptr type)
{
  // XXX not done yet
  return CORBA::DynFixed::_nil();
}

CORBA::OctetSeq*
CORBA::
DynFixed::get_value()
{
  // XXX not done yet
  return 0;
}

void
CORBA::
DynFixed::set_value(const CORBA::OctetSeq& val)
{
  // XXX not done yet
  return;
}

#endif


////////////////////////////////////////////////////////////////////////////
//     _narrow()                                                          //
//     _duplicate()                                                       //
//     release()                                                         //
//     is_nil()                                                          //
////////////////////////////////////////////////////////////////////////////

CORBA::DynAny_ptr
CORBA::DynAny::_narrow(CORBA::DynAny_ptr p)
{
  if (!CORBA::is_nil(p)) {
    return CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynAny::_nil();
}

CORBA::DynEnum_ptr
CORBA::DynEnum::_narrow(CORBA::DynAny_ptr p)
{
  if (!CORBA::is_nil(p) && p->NP_nodetype() == CORBA::DynAny::dt_enum) {
    return (CORBA::DynEnum_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynEnum::_nil();
}

#if 0
CORBA::DynFixed_ptr
CORBA::DynFixed::_narrow(CORBA::DynAny_ptr p)
{
  if (!CORBA::is_nil(p) && p->NP_nodetype() == CORBA::DynAny::dt_fixed) {
    return (CORBA::DynFixed_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynFixed::_nil();
}
#endif

CORBA::DynStruct_ptr
CORBA::DynStruct::_narrow(CORBA::DynAny_ptr p)
{
  if (!CORBA::is_nil(p) && p->NP_nodetype() == CORBA::DynAny::dt_struct) {
    return (CORBA::DynStruct_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynStruct::_nil();
}

CORBA::DynUnion_ptr
CORBA::DynUnion::_narrow(CORBA::DynAny_ptr p)
{
  if (!CORBA::is_nil(p) && p->NP_nodetype() == CORBA::DynAny::dt_union) {
    return (CORBA::DynUnion_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynUnion::_nil();
}

CORBA::DynSequence_ptr
CORBA::DynSequence::_narrow(CORBA::DynAny_ptr p)
{
  if (!CORBA::is_nil(p) && p->NP_nodetype() == CORBA::DynAny::dt_seq) {
    return (CORBA::DynSequence_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynSequence::_nil();

}

CORBA::DynArray_ptr
CORBA::DynArray::_narrow(CORBA::DynAny_ptr p)
{
  if (!CORBA::is_nil(p) && p->NP_nodetype() == CORBA::DynAny::dt_array) {
    return (CORBA::DynArray_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynArray::_nil();
}

CORBA::DynAny_ptr
CORBA::DynAny::_duplicate(CORBA::DynAny_ptr p)
{
  if (!CORBA::is_nil(p)) {
    CvtDynAnyP2(p)->incrRefCount();
    return p;
  }
  else
    return CORBA::DynAny::_nil();
}

CORBA::DynEnum_ptr
CORBA::DynEnum::_duplicate(CORBA::DynEnum_ptr p)
{
  if (!CORBA::is_nil(p)) {
    return (CORBA::DynEnum_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynEnum::_nil();
}

#if 0
CORBA::DynFixed_ptr
CORBA::DynFixed::_duplicate(CORBA::DynFixed_ptr p)
{
  if (!CORBA::is_nil(p)) {
    return (CORBA::DynFixed_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynFixed::_nil();
}
#endif

CORBA::DynStruct_ptr
CORBA::DynStruct::_duplicate(CORBA::DynStruct_ptr p)
{
  if (!CORBA::is_nil(p)) {
    return (CORBA::DynStruct_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynStruct::_nil();
}

CORBA::DynUnion_ptr
CORBA::DynUnion::_duplicate(CORBA::DynUnion_ptr p)
{
  if (!CORBA::is_nil(p)) {
    return (CORBA::DynUnion_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynUnion::_nil();
}

CORBA::DynSequence_ptr
CORBA::DynSequence::_duplicate(CORBA::DynSequence_ptr p)
{
  if (!CORBA::is_nil(p)) {
    return (CORBA::DynSequence_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynSequence::_nil();

}

CORBA::DynArray_ptr
CORBA::DynArray::_duplicate(CORBA::DynArray_ptr p)
{
  if (!CORBA::is_nil(p)) {
    return (CORBA::DynArray_ptr) CORBA::DynAny::_duplicate(p);
  }
  else
    return CORBA::DynArray::_nil();
}


void
CORBA::
DynAny::NP_release()
{
  ((dynAnyP*)pd)->decrRefCount(this);
}

void
CORBA::release(CORBA::DynAny_ptr p)
{
  if (!CORBA::is_nil(p)) {
    p->NP_release();
  }
}

CORBA::Boolean
CORBA::is_nil(CORBA::DynAny_ptr p)
{
  if (p) {
    return p->NP_is_nil();
  }
  else {
    // omniORB2 does not use a nil pointer to represent a nil DynAny_ptr.
    // The program has passed in a pointer which has not
    // been initialised by CORBA::DynAny::_nil() or similar functions.
    // Some ORBs seems to be quite lax about this. We don't want to
    // break the applications that make this assumption. Just call
    // _CORBA_use_nil_ptr_as_nil_objref() to take note of this.
    return _CORBA_use_nil_ptr_as_nil_objref();
  }
}

////////////////////////////////////////////////////////////////////////////
//     DynAny_var                                                         //
////////////////////////////////////////////////////////////////////////////

#define DYNANY_VAR_IMPL(dynvar,dyntype,dynptr,dynmember) \
CORBA:: \
dynvar::dynvar() \
{ \
  pd_d = CORBA::dyntype::_nil(); \
} \
 \
CORBA:: \
dynvar::dynvar(dynptr p) \
{ \
  pd_d = p; \
} \
 \
CORBA:: \
dynvar::~dynvar() \
{ \
  if (!CORBA::is_nil(pd_d)) CORBA::release(pd_d); \
} \
 \
CORBA:: \
dynvar::dynvar(const CORBA::dynvar& p) \
{ \
  if (!CORBA::is_nil(p.pd_d)) \
    pd_d = CORBA::dyntype::_duplicate(p.pd_d); \
  else \
    pd_d = CORBA::dyntype::_nil(); \
} \
 \
CORBA:: \
dynvar::dynvar(const CORBA::dynmember& p) \
{ \
  if (!CORBA::is_nil(p._ptr)) { \
    pd_d = CORBA::dyntype::_duplicate(p._ptr); \
  } \
  else \
    pd_d = CORBA::dyntype::_nil(); \
} \
 \
CORBA::dynvar&  \
CORBA:: \
dynvar::operator= (CORBA::dynptr p) \
{ \
  if (!CORBA::is_nil(pd_d)) \
    CORBA::release(pd_d); \
  pd_d = p; \
  return *this; \
} \
 \
CORBA::dynvar& \
CORBA:: \
dynvar::operator= (const CORBA::dynvar& p) \
{ \
  if (!CORBA::is_nil(pd_d)) \
    CORBA::release(pd_d); \
  if (!CORBA::is_nil(p.pd_d)) { \
    pd_d = CORBA::dyntype::_duplicate(p.pd_d); \
  } \
  else \
    pd_d = CORBA::dyntype::_nil(); \
  return *this; \
} \
 \
CORBA::dynvar&  \
CORBA:: \
dynvar::operator= (const CORBA::dynmember& p) \
{ \
  if (!CORBA::is_nil(pd_d)) \
    CORBA::release(pd_d); \
  if (!CORBA::is_nil(p._ptr)) { \
    pd_d = CORBA::dyntype::_duplicate(p._ptr); \
  } \
  else \
    pd_d = CORBA::dyntype::_nil(); \
  return *this; \
} \
 \
CORBA::dynptr  \
CORBA:: \
dynvar::operator->() const \
{ \
  return pd_d; \
} \
 \
CORBA:: \
dynvar::operator CORBA::dynptr() const \
{ \
  return pd_d; \
}

DYNANY_VAR_IMPL(DynAny_var,DynAny,DynAny_ptr,DynAny_member)
#if 0
DYNANY_VAR_IMPL(DynFixed_var,DynFixed,DynFixed_ptr,DynFixed_member)
#endif
DYNANY_VAR_IMPL(DynEnum_var,DynEnum,DynEnum_ptr,DynEnum_member)
DYNANY_VAR_IMPL(DynStruct_var,DynStruct,DynStruct_ptr,DynStruct_member)
DYNANY_VAR_IMPL(DynUnion_var,DynUnion,DynUnion_ptr,DynUnion_member)
DYNANY_VAR_IMPL(DynSequence_var,DynSequence,DynSequence_ptr,DynSequence_member)
DYNANY_VAR_IMPL(DynArray_var,DynArray,DynArray_ptr,DynArray_member)
#undef DYNANY_VAR_IMPL


////////////////////////////////////////////////////////////////////////////
//     DynAny_member                                                      //
////////////////////////////////////////////////////////////////////////////

#define DYNANY_MEMBER_IMPL(dynmember,dyntype,dynptr,dynvar) \
CORBA:: \
dynmember::dynmember() \
{ \
  _ptr = CORBA::dyntype::_nil(); \
} \
 \
CORBA:: \
dynmember::dynmember(CORBA::dynptr p) \
{ \
  _ptr = p; \
} \
 \
CORBA:: \
dynmember::dynmember(const CORBA::dynmember& p) \
{ \
  if (!CORBA::is_nil(p._ptr)) { \
    _ptr = CORBA::dyntype::_duplicate(p._ptr); \
  } \
  else \
    _ptr = CORBA::dyntype::_nil(); \
} \
 \
CORBA:: \
dynmember::~dynmember() \
{ \
  if (!CORBA::is_nil(_ptr)) \
    CORBA::release(_ptr); \
} \
 \
CORBA::dynmember&  \
CORBA:: \
dynmember::operator= (CORBA::dynptr p) \
{ \
  if (!CORBA::is_nil(_ptr)) \
    CORBA::release(_ptr); \
  _ptr = p; \
  return *this; \
} \
 \
CORBA::dynmember& \
CORBA:: \
dynmember::operator= (const CORBA::dynmember& p) \
{ \
  if (!CORBA::is_nil(_ptr)) \
    CORBA::release(_ptr); \
  if (!CORBA::is_nil(p._ptr)) { \
    _ptr = CORBA::dyntype::_duplicate(p._ptr); \
  } \
  else \
    _ptr = CORBA::dyntype::_nil(); \
  return *this; \
} \
 \
CORBA::dynmember& \
CORBA:: \
dynmember::operator= (const CORBA::dynvar& p) \
{ \
  if (!CORBA::is_nil(_ptr)) CORBA::release(_ptr); \
  if (!CORBA::is_nil(p.pd_d)) { \
    _ptr = CORBA::dyntype::_duplicate(p.pd_d); \
  } \
  else \
    _ptr = CORBA::dyntype::_nil(); \
  return *this; \
} \
 \
CORBA::dynptr \
CORBA:: \
dynmember::operator->() const \
{ \
  return _ptr; \
} \
 \
CORBA:: \
dynmember::operator CORBA::dynptr () const \
{ \
  return _ptr; \
}

DYNANY_MEMBER_IMPL(DynAny_member,DynAny,DynAny_ptr,DynAny_var)
#if 0
DYNANY_MEMBER_IMPL(DynFixed_member,DynFixed,DynFixed_ptr,DynFixed_var)
#endif
DYNANY_MEMBER_IMPL(DynEnum_member,DynEnum,DynEnum_ptr,DynEnum_var)
DYNANY_MEMBER_IMPL(DynStruct_member,DynStruct,DynStruct_ptr,DynStruct_var)
DYNANY_MEMBER_IMPL(DynUnion_member,DynUnion,DynUnion_ptr,DynUnion_var)
DYNANY_MEMBER_IMPL(DynSequence_member,DynSequence,DynSequence_ptr,DynSequence_var)
DYNANY_MEMBER_IMPL(DynArray_member,DynArray,DynArray_ptr,DynArray_var)
#undef DYNANY_MEMBER_IMPL


////////////////////////////////////////////////////////////////////////////
//     nil DynAny objects and _nil()                                      //
////////////////////////////////////////////////////////////////////////////

#ifdef RETURN_NIL
#error "Internal macro RETURN_NIL has already been defined in header files."
#endif

#ifdef NIL_DYNANY_BODY
#error "Internal macro NIL_DYNANY_BODY has already been defined in header files."
#endif

#ifdef NEED_DUMMY_RETURN
#define RETURN_NIL(x)  return x;
#else
#define RETURN_NIL(x)
#endif

#ifdef NIL_DYNANY_FLOAT_BODY
#error "Internal macro NIL_DYNANY_FLOAT_BODY has already been defined in header files."
#else
#ifndef NO_FLOAT
#define NIL_DYNANY_FLOAT_BODY  void insert_float(CORBA::Float) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void insert_double(CORBA::Double) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  CORBA::Float get_float() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::Double get_double() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  }
#else
#define NIL_DYNANY_FLOAT_BODY 
#endif
#endif

#define NIL_DYNANY_BODY \
  CORBA::TypeCode_ptr type() const { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  void assign(CORBA::DynAny_ptr) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void from_any(const CORBA::Any&) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  CORBA::Any* to_any() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  void destroy() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  CORBA::DynAny_ptr copy() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  void insert_boolean(CORBA::Boolean) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void insert_octet(CORBA::Octet) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void insert_char(CORBA::Char) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void insert_short(CORBA::Short) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void insert_ushort(CORBA::UShort) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void insert_long(CORBA::Long) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void insert_ulong(CORBA::ULong) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void insert_string(const char*) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void insert_reference(CORBA::Object_ptr) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void insert_typecode(CORBA::TypeCode_ptr) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void insert_any(const CORBA::Any&) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  CORBA::Boolean get_boolean() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::Octet get_octet() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::Char get_char() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::Short get_short() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::UShort get_ushort() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::Long get_long() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::ULong get_ulong() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  char* get_string() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::Object_ptr get_reference() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::TypeCode_ptr get_typecode() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::Any* get_any() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  NIL_DYNANY_FLOAT_BODY \
  CORBA::DynAny_ptr current_component() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::Boolean next() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  CORBA::Boolean seek(CORBA::Long index) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
    RETURN_NIL(0); \
  } \
  void rewind() { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  } \
  void NP_real_from_any(const CORBA::Any&) { \
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO); \
  }

class _0RL_nil_DynAny : public CORBA::DynAny {
public:    
  _0RL_nil_DynAny() : CORBA::DynAny(0,CORBA::DynAny::dt_any) {}
  ~_0RL_nil_DynAny() {}

  NIL_DYNANY_BODY

};

#if 0
class _0RL_nil_DynFixed :  public CORBA::DynFixed
{
public:
  _0RL_nil_DynFixed() : CORBA::DynAny(0,CORBA::DynAny::dt_fixed) {}
  ~_0RL_nil_DynFixed() { }

  NIL_DYNANY_BODY

  DynAny::OctetSeq* get_value() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  void set_value(const DynAny::OctetSeq&) {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
  }
};
#endif

class _0RL_nil_DynEnum :  public CORBA::DynEnum
{
public:
  _0RL_nil_DynEnum() : CORBA::DynEnum(0) { }
  ~_0RL_nil_DynEnum() { }

  NIL_DYNANY_BODY

  char* value_as_string() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  void value_as_string(const char*) {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
  }
  CORBA::ULong value_as_ulong() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  void value_as_ulong(CORBA::ULong) {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
  }
};
  
class _0RL_nil_DynStruct :  public CORBA::DynStruct
{
public:
  _0RL_nil_DynStruct() : CORBA::DynStruct(0) {}
  ~_0RL_nil_DynStruct() {}

  NIL_DYNANY_BODY

  char*  current_member_name() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  CORBA::TCKind current_member_kind() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return CORBA::tk_null;
#endif
  }
  CORBA::NameValuePairSeq* get_members() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  void set_members(const CORBA::NameValuePairSeq&) {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
  }
};


class _0RL_nil_DynUnion :  public CORBA::DynUnion
{
public:
  _0RL_nil_DynUnion() : CORBA::DynUnion(0) {}
  ~_0RL_nil_DynUnion() {}

  NIL_DYNANY_BODY

  CORBA::Boolean set_as_default() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  void set_as_default(CORBA::Boolean) {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
  }
  CORBA::DynAny_ptr discriminator() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  CORBA::TCKind discriminator_kind() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return CORBA::tk_null;
#endif
  }
  CORBA::DynAny_ptr member() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  char*  member_name() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  void member_name(const char*) {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
  }
  CORBA::TCKind member_kind() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return CORBA::tk_null;
#endif
  }
};

class _0RL_nil_DynSequence :  public CORBA::DynSequence
{
public:
  _0RL_nil_DynSequence() : CORBA::DynSequence(0) {}
  ~_0RL_nil_DynSequence() {}

  NIL_DYNANY_BODY

  CORBA::ULong length() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  void length (CORBA::ULong){
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
  }
  CORBA::AnySeq* get_elements() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  void set_elements(const CORBA::AnySeq&) {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
  }
};

class _0RL_nil_DynArray : public CORBA::DynArray
{
public:
  _0RL_nil_DynArray() : CORBA::DynArray(0) {}
  ~_0RL_nil_DynArray() {}

  NIL_DYNANY_BODY

  CORBA::AnySeq* get_elements() {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
    // never reach here! Dummy return to keep some compilers happy.
    return 0;
#endif
  }
  void set_elements(const CORBA::AnySeq&) {
    throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
  }
};

#undef RETURN_NIL
#undef NIL_DYNANY_BODY
#undef NIL_DYNANY_FLOAT_BODY

static _0RL_nil_DynAny      _nil_DynAny;
static _0RL_nil_DynEnum     _nil_DynEnum;
#if 0
static _0RL_nil_DynFixed    _nil_DynFixed;
#endif
static _0RL_nil_DynStruct   _nil_DynStruct;
static _0RL_nil_DynUnion    _nil_DynUnion;
static _0RL_nil_DynSequence _nil_DynSequence;
static _0RL_nil_DynArray    _nil_DynArray;

CORBA::DynAny_ptr
CORBA::DynAny::_nil()
{
  return &_nil_DynAny;
}

CORBA::DynEnum_ptr
CORBA::DynEnum::_nil()
{
  return &_nil_DynEnum;
}

#if 0
CORBA::DynFixed_ptr
CORBA::DynFixed::_nil()
{
  return &_nil_DynFixed;
}
#endif

CORBA::DynStruct_ptr
CORBA::DynStruct::_nil()
{
  return &_nil_DynStruct;
}

CORBA::DynUnion_ptr
CORBA::DynUnion::_nil()
{
  return &_nil_DynUnion;
}

CORBA::DynSequence_ptr
CORBA::DynSequence::_nil()
{
  return &_nil_DynSequence;
}

CORBA::DynArray_ptr
CORBA::DynArray::_nil()
{
  return &_nil_DynArray;
}

