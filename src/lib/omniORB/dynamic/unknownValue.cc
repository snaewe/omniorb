// -*- Mode: C++; -*-
//                            Package   : omniORB
// unknownValue.cc            Created on: 2004/07/26
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2004 Apasphere Ltd.
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
//    Object to hold unknown valuetypes received in Anys.
//

/*
  $Log$
  Revision 1.1.2.2  2004/10/13 17:58:21  dgrisby
  Abstract interfaces support; values support interfaces; value bug fixes.

  Revision 1.1.2.1  2004/07/26 22:56:39  dgrisby
  Support valuetypes in Anys.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/valueType.h>
#include <omniORB4/anyStream.h>
#include <unknownValue.h>
#include <tcParser.h>

OMNI_NAMESPACE_BEGIN(omni)

UnknownValue::UnknownValue(CORBA::TypeCode_ptr tc)
{
  OMNIORB_ASSERT(tc);
  OMNIORB_ASSERT(!CORBA::is_nil(tc));
  OMNIORB_ASSERT(tc->kind() == CORBA::tk_value ||
		 tc->kind() == CORBA::tk_value_box);

  pd_tc = CORBA::TypeCode::_duplicate(tc);
  pd_hash = omniValueType::hash_id(tc->id());

  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "Create UnknownValue instance for repoId '" << tc->id() << "'.\n";
  }
}

UnknownValue::UnknownValue(const UnknownValue& v)
  : pd_mbuf(v.pd_mbuf), pd_tc(v.pd_tc), pd_hash(v.pd_hash)
{
}

UnknownValue::~UnknownValue()
{
}

const char*
UnknownValue::_NP_repositoryId() const
{
  return pd_tc->id();
}

const char*
UnknownValue::_NP_repositoryId(CORBA::ULong& hash) const
{
  hash = pd_hash;
  return pd_tc->id();
}

const _omni_ValueIds*
UnknownValue::_NP_truncatableIds() const
{
  return 0;
}

CORBA::Boolean
UnknownValue::_NP_custom() const
{
  return 0;
}


static void
marshal_value_state(cdrStream& from, cdrStream& to, CORBA::TypeCode_ptr tc)
{
  // Marshal / unmarshal state for value, recursively handling inheritance.

  CORBA::TypeCode_var base = tc->concrete_base_type();
  if (base->kind() == CORBA::tk_value)
    marshal_value_state(from, to, base.in());

  CORBA::ULong nmembers = tc->member_count();

  CORBA::TypeCode_var mtc;

  for (CORBA::ULong i=0; i < nmembers; i++) {
    mtc = tc->member_type(i);
    tcParser::copyStreamToStream(mtc, from, to);
  }
}

void
UnknownValue::_PR_marshal_state(cdrStream& s) const
{
  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "Marshal UnknownValue for '" << pd_tc->id() << "'.\n";
  }

  // Read only stream wrapper for thread safety
  cdrAnyMemoryStream tmps(pd_mbuf, 1);

  if (pd_tc->kind() == CORBA::tk_value) {
    marshal_value_state(tmps, s, pd_tc);
  }
  else {
    // Value Box
    CORBA::TypeCode_var mtc = pd_tc->content_type();
    tcParser::copyStreamToStream(mtc, tmps, s);
  }
}

void
UnknownValue::_PR_unmarshal_state(cdrStream& s)
{
  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "Unmarshal UnknownValue for '" << pd_tc->id() << "'.\n";
  }

  if (pd_tc->kind() == CORBA::tk_value) {
    marshal_value_state(s, pd_mbuf, pd_tc);
  }
  else {
    CORBA::TypeCode_var mtc = pd_tc->content_type();
    tcParser::copyStreamToStream(mtc, s, pd_mbuf);
  }
}


UnknownValue*
UnknownValue::_downcast(CORBA::ValueBase* _b)
{
  return _b ? (UnknownValue*)_b->_ptrToValue(_PD_repoId) : 0;
}

const char* UnknownValue::_PD_repoId = "omni:UnknownValue";

void*
UnknownValue::_ptrToValue(const char* _id)
{
  if (_id == UnknownValue::_PD_repoId)
    return (UnknownValue*) this;

  if (_id == CORBA::ValueBase::_PD_repoId)
    return (CORBA::ValueBase*) this;

  if (omni::strMatch(_id, UnknownValue::_PD_repoId))
    return (UnknownValue*) this;

  if (omni::strMatch(_id, CORBA::ValueBase::_PD_repoId))
    return (CORBA::ValueBase*) this;

  return 0;
}

#ifdef OMNI_HAVE_COVARIANT_RETURNS
UnknownValue*
#else
CORBA::ValueBase*
#endif
UnknownValue::_copy_value()
{
  return new UnknownValue(*this);
}

OMNI_NAMESPACE_END(omni)
