// -*- Mode: C++; -*-
//                            Package   : omniORB
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

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <pseudo.h>
#include <exceptiondefs.h>


// The correct way of creating an environment object is to use
// the ORB::create_environment() method. The environment objects
// created via this method is an EnvironmentImpl and is a proper
// CORBA psuedo object.
//
// However, in the case of a C++ environment with no exception handling.
// The CORBA specification allows environment objects to be created on the
// stack or heap as follows:
//
//    static CORBA::Environment global_env;
//    void foo() {
//       CORBA::Environment  auto_env;
//       CORBA::Environment* new_env = new CORBA::Environment;
//    }
//
// Strictly speaking we can choose not to support this dialact. But to
// make it less painful to convert code written prevously in such an
// environment, we also support this variation.
//
// However, for these objects, we do not attempt to do reference counting.
// In other words, calling CORBA::release() does not have any effect.
// Calling Environment::duplicate() just return the argument pointer.
// Therefore it is possible to leak memory if someone do this:
//   void foo() {
//      CORBA::Environment* new_env = new CORBA::Environment;
//      CORBA::release(new_env);     // heap allocated object not deleted.
//   }
//


CORBA::
Environment::Environment() : pd_exception(0), 
                             pd_magic(CORBA::Environment::PR_magic),
                             pd_is_pseudo(0)
{
}

CORBA::
Environment::~Environment()
{
  if( pd_exception )  delete pd_exception;
  pd_magic = 0;
}


void
CORBA::
Environment::exception(CORBA::Exception* e)
{
  if (!CORBA::Exception::PR_is_valid(e))
    OMNIORB_THROW(BAD_PARAM,0,CORBA::COMPLETED_NO);
  if( pd_exception )  delete pd_exception;
  pd_exception = e;
}


CORBA::Exception*
CORBA::
Environment::exception() const
{
  return pd_exception;
}


void
CORBA::
Environment::clear()
{
  if( pd_exception )  delete pd_exception;
  pd_exception = 0;
}

CORBA::Boolean
CORBA::
Environment::NP_is_nil() const
{
  return 0;
}

CORBA::Environment_ptr
CORBA::
Environment::NP_duplicate()
{
  return this;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniNilEnv : public CORBA::Environment {
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

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

EnvironmentImpl::EnvironmentImpl()
{
  pd_is_pseudo = 1;
}

EnvironmentImpl::~EnvironmentImpl()
{
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



CORBA::Environment_ptr
CORBA::
Environment::_duplicate(CORBA::Environment_ptr p)
{
  if (!PR_is_valid(p))
    OMNIORB_THROW(BAD_PARAM,0,CORBA::COMPLETED_NO);
  if( !CORBA::is_nil(p) )  return p->NP_duplicate();
  else     return _nil();
}


CORBA::Environment_ptr
CORBA::
Environment::_nil()
{
  static omniNilEnv* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilEnv;
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

void
CORBA::release(CORBA::Environment_ptr p)
{
  if( CORBA::Environment::PR_is_valid(p) && !CORBA::is_nil(p) && p->pd_is_pseudo)
    ((EnvironmentImpl*)p)->decrRefCount();
}


CORBA::Status
CORBA::ORB::create_environment(Environment_out new_env)
{
  new_env = new EnvironmentImpl();
  RETURN_CORBA_STATUS;
}
