// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_String.h             Created on: 2001/08/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//    CORBA::String and CORBA::WString
//

/*
  $Log$
  Revision 1.1.2.1  2001/08/17 13:39:48  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////////////// String ///////////////////////////////
//////////////////////////////////////////////////////////////////////

_CORBA_MODULE_FN char* string_alloc(ULong len);
_CORBA_MODULE_FN void string_free(char*);
_CORBA_MODULE_FN char* string_dup(const char*);

typedef _CORBA_String_var String_var;
typedef _CORBA_String_member String_member;
typedef _CORBA_String_inout String_INOUT_arg;
typedef _CORBA_String_out String_OUT_arg;
typedef String_OUT_arg String_out;

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// WString //////////////////////////////
  //////////////////////////////////////////////////////////////////////

_CORBA_MODULE_FN _CORBA_WChar* wstring_alloc(ULong len);
_CORBA_MODULE_FN void          wstring_free(_CORBA_WChar*);
_CORBA_MODULE_FN _CORBA_WChar* wstring_dup(const _CORBA_WChar*);

typedef _CORBA_WString_var    WString_var;
typedef _CORBA_WString_member WString_member;
typedef _CORBA_WString_inout  WString_INOUT_arg;
typedef _CORBA_WString_out    WString_OUT_arg;
typedef WString_OUT_arg       WString_out;
