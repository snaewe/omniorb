// -*- Mode: C++; -*-
//                            Package   : omniORB2
// namedValue.cc              Created on: 9/1998
//                            Author    : David Riddoch (djr)
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
//   Implementation of CORBA::NamedValue.
//

#include <pseudo.h>


NamedValueImpl::NamedValueImpl(CORBA::Flags flags)
{
  pd_flags = flags;
  pd_name = CORBA::string_dup("");
  pd_value = new CORBA::Any;
  if( !pd_value.operator->() )  _CORBA_new_operator_return_null();
}


NamedValueImpl::NamedValueImpl(const char* name, CORBA::Flags flags)
{
  if( !name )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
  pd_flags = flags;
  pd_name = CORBA::string_dup(name);
  pd_value = new CORBA::Any;
  if( !pd_value.operator->() )  _CORBA_new_operator_return_null();
}


NamedValueImpl::NamedValueImpl(const char* name, const CORBA::Any& value,
			       CORBA::Flags flags)
{
  if( !name )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
  pd_flags = flags;
  pd_name = CORBA::string_dup(name);
  pd_value = new CORBA::Any(value);
  if( !pd_value.operator->() )  _CORBA_new_operator_return_null();
}


NamedValueImpl::NamedValueImpl(char* name, CORBA::Flags flags)
{
  if( !name )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
  pd_flags = flags;
  pd_name = name;
  pd_value = new CORBA::Any;
  if( !pd_value.operator->() )  _CORBA_new_operator_return_null();
}


NamedValueImpl::NamedValueImpl(char* name, CORBA::Any* value,
			       CORBA::Flags flags)
{
  if( !name || !value )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);
  pd_flags = flags;
  pd_name = name;
  pd_value = value;
}


NamedValueImpl::~NamedValueImpl() {}


const char*
NamedValueImpl::name() const
{
  return pd_name;
}


CORBA::Any*
NamedValueImpl::value() const
{
  return pd_value.operator->();
}


CORBA::Flags
NamedValueImpl::flags() const
{
  return pd_flags;
}


CORBA::Boolean
NamedValueImpl::NP_is_nil() const
{
  return 0;
}


CORBA::NamedValue_ptr
NamedValueImpl::NP_duplicate()
{
  incrRefCount();
  return this;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class NilNamedValue : public CORBA::NamedValue {
public:
  virtual const char* name() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Any* value() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Flags flags() const {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::Flags(0);
  }
  virtual CORBA::Boolean NP_is_nil() const {
    return 1;
  }
  virtual CORBA::NamedValue_ptr NP_duplicate() {
    return _nil();
  }
};

static NilNamedValue _nilNamedValue;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::NamedValue::~NamedValue() { pd_magic = 0; }


CORBA::NamedValue_ptr
CORBA::
NamedValue::_duplicate(NamedValue_ptr p)
{
  if (!PR_is_valid(p))
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
  if( !CORBA::is_nil(p) )  return p->NP_duplicate();
  else     return _nil();
}


CORBA::NamedValue_ptr
CORBA::
NamedValue::_nil()
{
  return &_nilNamedValue;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::release(NamedValue_ptr p)
{
  if( CORBA::NamedValue::PR_is_valid(p) && !CORBA::is_nil(p) )
    ((NamedValueImpl*)p)->decrRefCount();
}


CORBA::Status
CORBA::
ORB::create_named_value(NamedValue_out nmval)
{
  nmval = new NamedValueImpl((CORBA::Flags)0);
  RETURN_CORBA_STATUS;
}
