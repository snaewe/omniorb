// -*- Mode: C++; -*-
//                            Package   : omniORB2
// msvcdllstub.cc                 Created on: 27/5/99
//                            Author    : Sai Lai Lo (sll)
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
//	The only purpose of this file is to make it possible to link
//      the orbcore DLL under MSVC++ 5.0. None of these functions are
//      used in the DLL. 
//      They are required because MSVC++ generates references to these
//      functions when it generates the destructors for the template 
//      sequence classes that are defined in the headers *even though*
//      the destructors are defined as inline. Call this a feature or
//      a bug I don't care!
//	

#define ENABLE_CLIENT_IR_SUPPORT
#include <omniORB3/CORBA.h>


CORBA::TypeCode_member::~TypeCode_member() {}
CORBA::Any::~Any() {}

void CORBA::IDLType_Helper::release(CORBA::IDLType_ptr) {}

CORBA::Boolean CORBA::IDLType_Helper::is_nil(CORBA::IDLType_ptr) {
  return 0; 
}

void CORBA::InterfaceDef_Helper::release(CORBA::InterfaceDef_ptr) {}

CORBA::Boolean CORBA::InterfaceDef_Helper::is_nil(CORBA::InterfaceDef_ptr) {
  return 0; 
}

void CORBA::Contained_Helper::release(CORBA::Contained_ptr) {}

CORBA::Boolean CORBA::Contained_Helper::is_nil(CORBA::Contained_ptr) {
  return 0; 
}

void CORBA::ExceptionDef_Helper::release(CORBA::ExceptionDef_ptr) {}

CORBA::Boolean CORBA::ExceptionDef_Helper::is_nil(CORBA::ExceptionDef_ptr) {
  return 0; 
}
