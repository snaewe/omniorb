// -*- Mode: C++; -*-
//                            Package   : omniORB2
// unknownUserExn.cc          Created on: 9/1998
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
//   Implementation of CORBA::UnknownUserException.
//

#include <omniORB2/CORBA.h>


CORBA::
UnknownUserException::UnknownUserException(Any* ex)
  : pd_exception(ex)
{
  if( !ex )
    throw omniORB::fatalException(__FILE__,__LINE__,
       "CORBA::UnknownUserException::UnknownUserException(Any*)");
}


CORBA::
UnknownUserException::~UnknownUserException()
{
  delete pd_exception;
}


CORBA::Any&
CORBA::
UnknownUserException::exception()
{
  return *pd_exception;
}


void
CORBA::
UnknownUserException::_raise()
{
  throw *this;
}


CORBA::Exception*
CORBA::
UnknownUserException::_NP_duplicate() const
{
  // pd_exception is guarenteed not null
  Any* ex = new Any(*pd_exception);
  Exception* e = new UnknownUserException(ex);
  return e;
}


const char*
CORBA::
UnknownUserException::_NP_mostDerivedTypeId() const
{
  return "Exception/UnknownUserException";
}


CORBA::UnknownUserException*
CORBA::
UnknownUserException::_narrow(Exception* e)
{
  return (UnknownUserException*)_NP_is_a(e, "Exception/UnknownUserException");
}
