// -*- Mode: C++; -*-
//                            Package   : omniORB2
// policy.cc                  Created on: 30/4/1999
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
//  The implementation of CORBA::Policy object
//    XXX - Just scaffolding at the moment
//

/*
 $Log$
 Revision 1.1  1999/05/25 17:04:02  sll
 Initial revision

*/

#include <omniORB2/CORBA.h>

_init_in_def_( const CORBA::ULong CORBA::SecConstruction = 1;)

CORBA::PolicyType
CORBA::
Policy::policy_type() {
  return 0;
}

CORBA::Policy_ptr
CORBA::
Policy::copy() {
  return _duplicate(this);
}

void
CORBA::
Policy::destroy() {
}

CORBA::Policy_ptr
CORBA::
Policy::_duplicate(CORBA::Policy_ptr p) {
  return p;
}

CORBA::Policy_ptr
CORBA::
Policy::_nil() {
  return 0;
}

CORBA::Boolean
CORBA::
is_nil(CORBA::Policy_ptr p)
{
  return (p)?0:1;
}

void
CORBA::release(CORBA::Policy_ptr)
{
  return;
}

void
CORBA::
ConstructionPolicy::make_domain_manager(CORBA::InterfaceDef_ptr,
					CORBA::Boolean)
{
  return;
}

CORBA::ConstructionPolicy_ptr
CORBA::
ConstructionPolicy::_duplicate(CORBA::ConstructionPolicy_ptr p) {
  return p;
}

CORBA::ConstructionPolicy_ptr
CORBA::
ConstructionPolicy::_nil() {
  return 0;
}

CORBA::Boolean
CORBA::
is_nil(CORBA::ConstructionPolicy_ptr p)
{
  return (p)?0:1;
}

void
CORBA::release(CORBA::ConstructionPolicy_ptr)
{
  return;
}
