// -*- Mode: C++; -*-
//                            Package   : omniORB
// exceptionList.cc           Created on: 9/1998
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
//   Implementation of CORBA::ExceptionList.
//

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <pseudo.h>
#include <exceptiondefs.h>


#define INIT_MAX_SEQ_LENGTH  6


ExceptionListImpl::ExceptionListImpl()
{
  pd_list.length(INIT_MAX_SEQ_LENGTH);
  pd_list.length(0);
}


ExceptionListImpl::~ExceptionListImpl()
{
  free_entries();
}


CORBA::ULong
ExceptionListImpl::count() const
{
  return pd_list.length();
}


void
ExceptionListImpl::add(CORBA::TypeCode_ptr tc)
{
  if( !CORBA::TypeCode::PR_is_valid(tc) || CORBA::is_nil(tc) )
    OMNIORB_THROW(BAD_PARAM,0,CORBA::COMPLETED_NO);

  CORBA::ULong len = pd_list.length();

  if( len == pd_list.maximum() )
    // allocate new space in decent chunks
    pd_list.length(len * 6 / 5 + 1);

  CORBA::TypeCode_ptr ntc = CORBA::TypeCode::_duplicate(tc);
  pd_list.length(len + 1);
  pd_list[len] = ntc;
}


void
ExceptionListImpl::add_consume(CORBA::TypeCode_ptr tc)
{
  if( !CORBA::TypeCode::PR_is_valid(tc) || CORBA::is_nil(tc) )
    OMNIORB_THROW(BAD_PARAM,0,CORBA::COMPLETED_NO);

  CORBA::ULong len = pd_list.length();

  if( len == pd_list.maximum() )
    // allocate new space in decent chunks
    pd_list.length(len * 6 / 5 + 1);

  pd_list.length(len + 1);
  pd_list[len] = tc;
}


CORBA::TypeCode_ptr
ExceptionListImpl::item(CORBA::ULong index)
{
  if (index >= pd_list.length())
    throw CORBA::ExceptionList::Bounds();

  return pd_list[index];
}


CORBA::Status
ExceptionListImpl::remove(CORBA::ULong index)
{
  if (index >= pd_list.length())
    throw CORBA::ExceptionList::Bounds();

  // operator[] on the sequence will do the bounds check for us here
  CORBA::release(pd_list[index]);

  for( CORBA::ULong i = index; i < pd_list.length() - 1; i++ )
    pd_list[i] = pd_list[i + 1];

  pd_list.length(pd_list.length() - 1);
  RETURN_CORBA_STATUS;
}


CORBA::Boolean
ExceptionListImpl::NP_is_nil() const
{
  return 0;
}


CORBA::ExceptionList_ptr
ExceptionListImpl::NP_duplicate()
{
  incrRefCount();
  return this;
}


void
ExceptionListImpl::free_entries()
{
  for( CORBA::ULong i=0; i < pd_list.length(); i++ )
    CORBA::release(pd_list[i]);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilExList : public CORBA::ExceptionList {
public:
  virtual CORBA::ULong count() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void add(CORBA::TypeCode_ptr tc) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void add_consume(CORBA::TypeCode_ptr tc) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::TypeCode_ptr item(CORBA::ULong index) {
    _CORBA_invoked_nil_pseudo_ref();
    return CORBA::TypeCode::_nil();
  }
  virtual CORBA::Status remove(CORBA::ULong index) {
    _CORBA_invoked_nil_pseudo_ref();
    RETURN_CORBA_STATUS;
  }
  virtual CORBA::Boolean NP_is_nil() const {
    return 1;
  }
  virtual CORBA::ExceptionList_ptr NP_duplicate() {
    return _nil();
  }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::ExceptionList::~ExceptionList() { pd_magic = 0; }


CORBA::ExceptionList_ptr
CORBA::
ExceptionList::_duplicate(ExceptionList_ptr p)
{
  if (!PR_is_valid(p))
    OMNIORB_THROW(BAD_PARAM,0,CORBA::COMPLETED_NO);
  if( !CORBA::is_nil(p) )  return p->NP_duplicate();
  else     return _nil();
}


CORBA::ExceptionList_ptr
CORBA::
ExceptionList::_nil()
{
  static omniNilExList* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilExList;
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::release(ExceptionList_ptr p)
{
  if( CORBA::ExceptionList::PR_is_valid(p) && !CORBA::is_nil(p) )
    ((ExceptionListImpl*)p)->decrRefCount();
}


CORBA::Status
CORBA::ORB::create_exception_list(ExceptionList_out exclist)
{
  exclist = new ExceptionListImpl();
  RETURN_CORBA_STATUS;
}
