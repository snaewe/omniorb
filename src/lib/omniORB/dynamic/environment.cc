// -*- Mode: C++; -*-
//                            Package   : omniORB2
// environment.cc             Created on: 9/1998
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
//   Implementation of CORBA::Environment.
//

#include <pseudo.h>


EnvironmentImpl::~EnvironmentImpl()
{
  if( pd_exception )  delete pd_exception;
}


void
EnvironmentImpl::exception(CORBA::Exception* e)
{
  if( pd_exception )  delete pd_exception;
  pd_exception = e;
}


CORBA::Exception*
EnvironmentImpl::exception() const
{
  return pd_exception;
}


void
EnvironmentImpl::clear()
{
  if( pd_exception )  delete pd_exception;
  pd_exception = 0;
}


CORBA::Boolean
EnvironmentImpl::NP_is_nil() const
{
  return 0;
}


CORBA::Environment_ptr
EnvironmentImpl::NP_duplicate()
{
  incrRefCount();
  return this;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class NilEnv : public CORBA::Environment {
public:
  virtual void exception(CORBA::Exception*) {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Exception* exception() const {
    _CORBA_invoked_nil_pseudo_ref();
    return 0;
  }
  virtual void clear() {
    _CORBA_invoked_nil_pseudo_ref();
  }
  virtual CORBA::Boolean NP_is_nil() const {
    return 1;
  }
  virtual CORBA::Environment_ptr NP_duplicate() {
    return _nil();
  }
};

static NilEnv _nilEnvironment;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Environment::~Environment() {}


CORBA::Environment_ptr
CORBA::
Environment::_duplicate(Environment_ptr p)
{
  if( p )  return p->NP_duplicate();
  else     return _nil();
}


CORBA::Environment_ptr
CORBA::
Environment::_nil()
{
  return &_nilEnvironment;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
CORBA::release(Environment_ptr p)
{
  if( !p->NP_is_nil() )
    ((EnvironmentImpl*)p)->decrRefCount();
}


CORBA::Status
CORBA::ORB::create_environment(Environment_out new_env)
{
  new_env = new EnvironmentImpl();
  RETURN_CORBA_STATUS;
}
