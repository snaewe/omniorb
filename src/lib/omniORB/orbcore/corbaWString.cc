// -*- Mode: C++; -*-
//                            Package   : omniORB
// corbaWString.cc            Created on: 27/10/2000
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//    Implementation of the WString interface.
//	

/*
  $Log$
  Revision 1.1.2.2  2000/11/15 17:20:23  sll
  Removed obsoluted marshalling functions.

  Revision 1.1.2.1  2000/10/27 15:42:07  dpg1
  Initial code set conversion support. Not yet enabled or fully tested.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <exceptiondefs.h>

const _CORBA_WChar*const _CORBA_WString_helper::empty_wstring = L"";


_CORBA_WChar*
CORBA::wstring_alloc(CORBA::ULong len)
{
  // We initialise the string to zero length to help prevent errors
  // if this string is copied before it is initialised.  This is easy
  // to do when assigning the returned value to a CORBA::WString_var.
  _CORBA_WChar* s = _CORBA_WString_helper::alloc(len);
  if( s )  *s = L'\0';
  return s;
}


void
CORBA::wstring_free(_CORBA_WChar* p)
{
  _CORBA_WString_helper::free(p);
}


_CORBA_WChar*
CORBA::wstring_dup(const _CORBA_WChar* p)
{
  if (p) return _CORBA_WString_helper::dup(p);
  return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// WString_member ///////////////////////////
//////////////////////////////////////////////////////////////////////

void
_CORBA_WString_member::operator <<= (cdrStream& s)
{
  if( _ptr && _ptr != _CORBA_WString_helper::empty_wstring )
    _CORBA_WString_helper::free(_ptr);
  _ptr = 0;

  _ptr = s.unmarshalWString();
}

void
_CORBA_WString_member::operator >>= (cdrStream& s) const {
  s.marshalWString(_ptr);
}

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Unbounded_Sequence__WString /////////////////
//////////////////////////////////////////////////////////////////////

void
_CORBA_Sequence__WString::operator >>= (cdrStream& s) const
{
  pd_len >>= s;

  for( CORBA::ULong i = 0; i < pd_len; i++ ) {
    _CORBA_WChar* p = pd_data[i];
    s.marshalWString(p);
  }
}


void
_CORBA_Sequence__WString::operator <<= (cdrStream& s)
{
  _CORBA_ULong slen;
  slen <<= s;

  if (!s.checkInputOverrun(1,slen) || (pd_bounded && slen > pd_max)) {
    OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
  }

  if (!pd_rel && slen <= pd_max) {
    // obtain ownership of the array and its elements (note that this isn't
    // the most effecient solution, but neither is invoking length!)
    copybuffer(pd_len);
  }

  length(slen);

  for( _CORBA_ULong i = 0; i < slen; i++ ) {
    _CORBA_WChar*& p = (_CORBA_WChar*&) pd_data[i];

    if( p ) { _CORBA_WString_helper::free(p); p = 0; }

    p = s.unmarshalWString();
  }
}
