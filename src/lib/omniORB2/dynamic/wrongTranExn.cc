// -*- Mode: C++; -*-
//                            Package   : omniORB2
// wrongTranExn.cc            Created on: 4/1999
//                            Author    : Sai-Lai Lo (sll)
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
//   Implementation of CORBA::WrongTransaction.
//

#include <omniORB2/CORBA.h>


void
CORBA::
WrongTransaction::_raise()
{
  throw *this;
}


CORBA::Exception*
CORBA::
WrongTransaction::_NP_duplicate() const
{
  Exception* e = new WrongTransaction;
  return e;
}


const char*
CORBA::
WrongTransaction::_NP_mostDerivedTypeId() const
{
  return "Exception/WrongTransaction";
}


CORBA::WrongTransaction*
CORBA::
WrongTransaction::_narrow(Exception* e)
{
  return (WrongTransaction*)_NP_is_a(e, "Exception/WrongTransaction");
}
