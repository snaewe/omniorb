// -*- Mode: C++; -*-
//                            Package   : omniORB2
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

#include <pseudo.h>


#define INIT_MAX_SEQ_LENGTH  6


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
  if( !s )  throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

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
  if( !s )  throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

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
  return pd_list[index];
}


CORBA::Status
ContextListImpl::remove(CORBA::ULong index)
{
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

class NilCtList : public CORBA::ContextList {
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

static NilCtList _nilContextList;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::ContextList::~ContextList() {}


CORBA::ContextList_ptr
CORBA::
ContextList::_duplicate(ContextList_ptr p)
{
  if( p )  return p->NP_duplicate();
  else     return _nil();
}


CORBA::ContextList_ptr
CORBA::
ContextList::_nil()
{
  return &_nilContextList;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::release(ContextList_ptr p)
{
  if( !p->NP_is_nil() )
    ((ContextListImpl*)p)->decrRefCount();
}


CORBA::Status
CORBA::
ORB::create_context_list(ContextList_out ctxtlist)
{
  ctxtlist = new ContextListImpl();
  RETURN_CORBA_STATUS;
}
