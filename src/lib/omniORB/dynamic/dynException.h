// -*- Mode: C++; -*-
//                            Package   : omniORB2
// dynException.h             Created on: 6/99
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
//	*** PROPRIETORY INTERFACE ***

#ifndef __DYNEXCEPTION_H__
#define __DYNEXCEPTION_H__

extern CORBA::Boolean isaSystemException(const CORBA::Any* a);
// Return true if the any value contains a system exception.
// XXX It is almost an arbitrary decision to define this function
//     within the dynException module. The only reason is because
//     the any insertion operators are implemented in the module
//     and it is a good thing to have the test function implemented
//     in the same module.

#endif  // __DYNEXCEPTION_H__
