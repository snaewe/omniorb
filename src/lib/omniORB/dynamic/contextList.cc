// -*- Mode: C++; -*-
//                            Package   : omniORB
// contextList.cc             Created on: 9/1998
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
//   Implementation of CORBA::ContextList.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/objTracker.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <pseudo.h>
#include <exceptiondefs.h>


#define INIT_MAX_SEQ_LENGTH  6

OMNI_NAMESPACE_BEGIN(omni)

ContextListImpl::ContextListImpl()
{
  pd_list.length(INIT_MAX_SEQ_LENGTH);
  pd_list.length(0);
}


ContextListImpl::~ContextListImpl()
{
  free_entries();
}


CORBA::ULong
ContextListImpl::count() const
{
  return pd_list.length();
}


void
ContextListImpl::add(const char* s)
{
  if( !s )  OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_InvalidContextName,
			  CORBA::COMPLETED_NO);

  CORBA::ULong len = pd_list.length();

  if( len == pd_list.maximum() )
    // allocate new space in decent chunks
    pd_list.length(len * 6 / 5 + 1);

  char* ns = CORBA::string_dup(s);
  if( !ns )  _CORBA_new_operator_return_null();
  pd_list.length(len + 1);
  pd_list[len] = ns;
}


void
ContextListImpl::add_consume(char* s)
{
  if( !s )  OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_InvalidContextName,
			  CORBA::COMPLETED_NO);

  CORBA::ULong len = pd_list.length();

  if( len == pd_list.maximum() )
    // allocate new space in decent chunks
    pd_list.length(len * 6 / 5 + 1);

  pd_list.length(len + 1);
  pd_list[len] = s;
}


const char*
ContextListImpl::item(CORBA::ULong index)
{
  if (index >= pd_list.length())
    throw CORBA::ContextList::Bounds();

  return pd_list[index];
}


CORBA::Status
ContextListImpl::remove(CORBA::ULong index)
{
  if (index >= pd_list.length())
    throw CORBA::ContextList::Bounds();


  // operator[] on the sequence will do the bounds check for us here
  CORBA::string_free(pd_list[index]);

  for( CORBA::ULong i = index; i < pd_list.length() - 1; i++ )
    pd_list[i] = pd_list[i + 1];

  pd_list.length(pd_list.length() - 1);
  RETURN_CORBA_STATUS;
}


CORBA::Boolean
ContextListImpl::NP_is_nil() const
{
  return 0;
}


CORBA::ContextList_ptr
ContextListImpl::NP_duplicate()
{
  incrRefCount();
  return this;
}


void
ContextListImpl::free_entries()
{
  for( CORBA::ULong i=0; i < pd_list.length(); i++ )
    CORBA::string_free(pd_list[i]);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilCtList : public CORBA::ContextList, public omniTrackedObject {
public:
  virtual CORBA::ULong count() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void add(const char* ctxt) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual void add_consume(char* ctxt) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual const char* item(CORBA::ULong index) {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual CORBA::Status remove(CORBA::ULong index) {
    _CORBA_invoked_nil_pseudo_ref();
    RETURN_CORBA_STATUS;
  }
  virtual CORBA::Boolean NP_is_nil() const {
    return 1;
  }
  virtual CORBA::ContextList_ptr NP_duplicate() {
    return _nil();
  }
};

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::ContextList::~ContextList() { pd_magic = 0; }


CORBA::ContextList_ptr
CORBA::
ContextList::_duplicate(CORBA::ContextList_ptr p)
{
  if (!PR_is_valid(p))
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidContext,CORBA::COMPLETED_NO);
  if( !CORBA::is_nil(p) )  return p->NP_duplicate();
  else     return _nil();
}


CORBA::ContextList_ptr
CORBA::
ContextList::_nil()
{
  static omniNilCtList* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilCtList();
    registerTrackedObject(_the_nil_ptr);
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::release(CORBA::ContextList_ptr p)
{
  if( CORBA::ContextList::PR_is_valid(p) && !CORBA::is_nil(p) )
    ((ContextListImpl*)p)->decrRefCount();
}


CORBA::Status
CORBA::ORB::create_context_list(ContextList_out ctxtlist)
{
  ctxtlist = new ContextListImpl();
  RETURN_CORBA_STATUS;
}
