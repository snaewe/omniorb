// -*- Mode: C++; -*-
//                            Package   : omniORB
// abstractBase.c             Created on: 2004/03/05
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2004-2005 Apasphere Ltd.
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
//    AbstractBase implementation
//

/*
  $Log$
  Revision 1.1.2.3  2005/01/17 14:44:46  dgrisby
  Surprisingly few changes to compile on Windows.

  Revision 1.1.2.2  2004/10/13 17:58:20  dgrisby
  Abstract interfaces support; values support interfaces; value bug fixes.

  Revision 1.1.2.1  2004/04/02 13:26:24  dgrisby
  Start refactoring TypeCode to support value TypeCodes, start of
  abstract interfaces support.

*/

#include <omniORB4/CORBA.h>


const char*
CORBA::AbstractBase::_PD_repoId = "IDL:omg.org/CORBA/AbstractBase:1.0";


CORBA::AbstractBase_ptr
CORBA::AbstractBase::_duplicate(CORBA::AbstractBase_ptr a)
{
  if (a) {
    if (!a->_to_value())
      a->_to_object();
  }
  return a;
}

CORBA::AbstractBase_ptr
CORBA::AbstractBase::_narrow(CORBA::AbstractBase_ptr a)
{
  if (a) {
    if (!a->_to_value())
      a->_to_object();
  }
  return a;
}

CORBA::AbstractBase_ptr
CORBA::AbstractBase::_nil()
{
  // *** HERE: use a singleton object.
  return 0;
}

CORBA::AbstractBase::AbstractBase() {}
CORBA::AbstractBase::AbstractBase(const CORBA::AbstractBase&) {}
CORBA::AbstractBase::~AbstractBase() {}

CORBA::Boolean
CORBA::AbstractBase::_NP_is_nil() const
{
  return 0;
}

CORBA::Object_ptr
CORBA::AbstractBase::_NP_to_object()
{
  return CORBA::Object::_nil();
}

CORBA::ValueBase*
CORBA::AbstractBase::_NP_to_value()
{
  return 0;
}

void
CORBA::release(CORBA::AbstractBase_ptr a)
{
  if (a) {
    CORBA::ValueBase* v = a->_NP_to_value();
    if (v) {
      v->_remove_ref();
      return;
    }
    CORBA::Object_ptr o = a->_NP_to_object();
    CORBA::release(o);
  }
}

CORBA::Boolean
CORBA::_omni_AbstractBaseObjref::_NP_is_nil() const
{
  return this->OMNIORB_BASE_CTOR(CORBA::)Object::_NP_is_nil();
}

CORBA::Object_ptr
CORBA::_omni_AbstractBaseObjref::_NP_to_object()
{
  return (CORBA::Object_ptr)this;
}
