// -*- Mode: C++; -*-
//                            Package   : omniORB
// constants.cc               Created on: 26/9/99
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
//	

/*
  $Log$
  Revision 1.4.2.3  2004/07/04 23:53:36  dgrisby
  More ValueType TypeCode and Any support.

  Revision 1.4.2.2  2003/09/26 16:12:54  dgrisby
  Start of valuetype support.

  Revision 1.4.2.1  2003/03/23 21:02:51  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.2.2.3  2001/10/17 16:44:02  dpg1
  Update DynAny to CORBA 2.5 spec, const Any exception extraction.

  Revision 1.2.2.2  2000/09/27 17:25:40  sll
  Changed include/omniORB3 to include/omniORB4.

  Revision 1.2.2.1  2000/07/17 10:35:40  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:03  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.1  1999/09/22 14:26:28  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif


OMNI_NAMESPACE_BEGIN(omni)

// See the description of this variable in omniInternal.h
const char* omniORB_4_1_dyn = "omniORB dynamic library version 4.1.x";

OMNI_NAMESPACE_END(omni)


//////////////////////////////////////////////////////////////////////
// Each pseudo object type must be assigned a magic number.
// This magic number is written into the member pd_magic of each instance
// The static function PR_is_valid() in each of the pseudo object class 
// can be used to test if the instance is indeed valid. If a random pointer 
// is passed to isvalid(), it is unlikely that the magic number would match.

const CORBA::ULong CORBA::Context::PR_magic         = 0x43545854U; // CTXT
const CORBA::ULong CORBA::ContextList::PR_magic     = 0x4354584CU; // CTXL
const CORBA::ULong CORBA::Environment::PR_magic     = 0x454E564CU; // ENVI
const CORBA::ULong CORBA::ExceptionList::PR_magic   = 0x4558434CU; // EXCL
const CORBA::ULong CORBA::NamedValue::PR_magic      = 0x4E56414CU; // NVAL
const CORBA::ULong CORBA::NVList::PR_magic          = 0x4E564C54U; // NVLT
const CORBA::ULong CORBA::Request::PR_magic         = 0x52455154U; // REQT
const CORBA::ULong CORBA::TypeCode::PR_magic        = 0x54594F4CU; // TCOL
const CORBA::ULong DynamicAny::DynAny::PR_magic     = 0x44594E54U; // DYNT
const CORBA::ULong CORBA::ValueBase::_PR_magic      = 0x56414C42U; // VALB
