// -*- Mode: C++; -*-
//                            Package   : omniORB
// poastubs.cc                Created on: 30/07/2001
//                            Author1   : Sai-Lai Lo
//                            Author2   : Duncan Grisby
//
//    Copyright (C) 2004 Apasphere Ltd.
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
// Description:
//
//    TypeCodes and Any operators for enums defined in the POA module.

/*
  $Log$
  Revision 1.1.4.5  2007/02/26 12:41:00  dgrisby
  Fix duplicated POA enum Any operators. Thanks Thomas Richter.

  Revision 1.1.4.4  2004/07/23 10:29:58  dgrisby
  Completely new, much simpler Any implementation.

  Revision 1.1.4.3  2004/04/02 13:26:24  dgrisby
  Start refactoring TypeCode to support value TypeCodes, start of
  abstract interfaces support.

  Revision 1.1.4.2  2003/09/26 16:12:54  dgrisby
  Start of valuetype support.

  Revision 1.1.4.1  2003/03/23 21:02:47  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.3  2001/08/29 13:41:02  dpg1
  jnw's fix for compilers with variable sizeof(enum)

  Revision 1.1.2.2  2001/08/03 18:11:41  sll
  Added MAIN_THREAD_MODEL to the typecode of ThreadPolicyValue.

  Revision 1.1.2.1  2001/07/31 16:04:06  sll
  Added ORB::create_policy() and associated types and operators.

*/

#include <omniORB4/CORBA.h>

OMNI_USING_NAMESPACE(omni)

#include <omniORB4/poa_enumsDynSK.cc>
