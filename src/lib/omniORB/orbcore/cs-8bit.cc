// -*- Mode: C++; -*-
//                            Package   : omniORB
// cs-8bit.cc                 Created on: 24/10/2000
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
//    8 bit code sets

/*
  $Log$
  Revision 1.1.2.4  2000/11/16 12:33:44  dpg1
  Minor fixes to permit use of UShort as WChar.

  Revision 1.1.2.3  2000/11/10 15:41:36  dpg1
  Native code sets throw BAD_PARAM if they are given a null transmission
  code set.

  Revision 1.1.2.2  2000/11/03 18:49:17  sll
  Separate out the marshalling of byte, octet and char into 3 set of distinct
  marshalling functions.
  Renamed put_char_array and get_char_array to put_octet_array and
  get_octet_array.
  New string marshal member functions.

  Revision 1.1.2.1  2000/10/27 15:42:08  dpg1
  Initial code set conversion support. Not yet enabled or fully tested.

*/

#include <omniORB4/CORBA.h>
#include <codeSetUtil.h>

//
// Native code set
//

void
omniCodeSet::NCS_C_8bit::marshalChar(cdrStream& stream,
				     omniCodeSet::TCS_C* tcs,
				     _CORBA_Char c)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  if (tcs->fastMarshalChar(stream, this, c)) return;

  omniCodeSet::UniChar uc = pd_toU[c];
  if (c && !uc) OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);

  tcs->marshalChar(stream, uc);
}

void
omniCodeSet::NCS_C_8bit::marshalString(cdrStream& stream,
				       omniCodeSet::TCS_C* tcs,
				       _CORBA_ULong bound, const char* s)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  if (tcs->fastMarshalString(stream, this, bound, s)) return;

  _CORBA_ULong len = strlen(s) + 1;

  if (bound && len >= bound)
    OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

  omniCodeSet::UniChar*    us = omniCodeSetUtil::allocU(len);
  omniCodeSetUtil::HolderU uh(us);
  omniCodeSet::UniChar     uc;

  for (_CORBA_ULong i=0; i<len; i++) {
    uc = pd_toU[(_CORBA_Char)(s[i])];
    if (s[i] && !uc) OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);
    us[i] = uc;
  }
  tcs->marshalString(stream, len, us);
}

_CORBA_Char
omniCodeSet::NCS_C_8bit::unmarshalChar(cdrStream& stream,
				       omniCodeSet::TCS_C* tcs)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  _CORBA_Char c;
  if (tcs->fastUnmarshalChar(stream, this, c)) return c;

  omniCodeSet::UniChar uc = tcs->unmarshalChar(stream);

  c = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
  if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);

  return c;
}

_CORBA_ULong
omniCodeSet::NCS_C_8bit::unmarshalString(cdrStream& stream,
					 omniCodeSet::TCS_C* tcs,
					 _CORBA_ULong bound, char*& s)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  _CORBA_ULong len;
  if (tcs->fastUnmarshalString(stream, this, bound, len, s)) return len;

  omniCodeSet::UniChar* us;
  len = tcs->unmarshalString(stream, bound, us);
  OMNIORB_ASSERT(us);
  OMNIORB_ASSERT(len > 0);

  omniCodeSetUtil::HolderU uh(us);

  s = omniCodeSetUtil::allocC(len);
  omniCodeSetUtil::HolderC h(s);

  omniCodeSet::UniChar uc;
  _CORBA_Char          c;

  for (_CORBA_ULong i=0; i<len; i++) {
    uc = us[i];
    c  = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
    if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);
    s[i] = c;
  }
  h.drop();
  return len;
}


//
// Transmission code set
//

void
omniCodeSet::TCS_C_8bit::marshalChar(cdrStream& stream,
				     omniCodeSet::UniChar uc)
{
  _CORBA_Char c = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
  if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);

  stream.marshalOctet(c);
}

void
omniCodeSet::TCS_C_8bit::marshalString(cdrStream& stream,
				       _CORBA_ULong len,
				       const omniCodeSet::UniChar* us)
{
  len >>= stream;

  _CORBA_Char          c;
  omniCodeSet::UniChar uc;
  
  for (_CORBA_ULong i=0; i<len; i++) {
    uc = us[i];
    c = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
    if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);
    stream.marshalOctet(c);
  }
}

omniCodeSet::UniChar
omniCodeSet::TCS_C_8bit::unmarshalChar(cdrStream& stream)
{
  _CORBA_Char c;
  c = stream.unmarshalOctet();

  omniCodeSet::UniChar uc = pd_toU[c];
  if (c && !uc) OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);

  return uc;
}

_CORBA_ULong
omniCodeSet::TCS_C_8bit::unmarshalString(cdrStream& stream,
					 _CORBA_ULong bound,
					 omniCodeSet::UniChar*& us)
{
  _CORBA_ULong len;
  len <<= stream;

  if (len == 0) // Zero length is invalid. *** Maybe treat as zero length?
    OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

  if (bound && len >= bound)
    OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

  us = omniCodeSetUtil::allocU(len);
  omniCodeSetUtil::HolderU uh(us);

  _CORBA_Char          c;
  omniCodeSet::UniChar uc;

  for (_CORBA_ULong i=0; i < len; i++) {
    c = stream.unmarshalOctet();
    uc = pd_toU[c];
    if (c && !uc) OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);
    us[i] = uc;
  }
  if (uc != 0) // String must end with null
    OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

  uh.drop();
  return len;
}

_CORBA_Boolean
omniCodeSet::TCS_C_8bit::fastMarshalChar(cdrStream&          stream,
					 omniCodeSet::NCS_C* ncs,
					 _CORBA_Char         c)
{
  if (ncs->id() == id()) { // Null transformation
    stream.marshalOctet(c);
    return 1;
  }
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C_8bit::fastMarshalString(cdrStream&          stream,
					   omniCodeSet::NCS_C* ncs,
					   _CORBA_ULong        bound,
					   const char*         s)
{
  if (ncs->id() == id()) { // Null transformation
    _CORBA_ULong len = strlen(s) + 1;
    if (bound && len >= bound)
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

    len >>= stream;
    stream.put_octet_array((const _CORBA_Octet*)s, len);
    return 1;
  }
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C_8bit::fastUnmarshalChar(cdrStream&          stream,
					   omniCodeSet::NCS_C* ncs,
					   _CORBA_Char&        c)
{
  if (ncs->id() == id()) { // Null transformation
    c = stream.unmarshalOctet();
    return 1;
  }
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C_8bit::fastUnmarshalString(cdrStream&          stream,
					     omniCodeSet::NCS_C* ncs,
					     _CORBA_ULong        bound,
					     _CORBA_ULong&       len,
					     char*&              s)
{
  if (ncs->id() == id()) { // Null transformation
    len <<= stream;

    if (len == 0) // Zero length is invalid
      OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

    if (bound && len >= bound)
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

    s = omniCodeSetUtil::allocC(len);
    omniCodeSetUtil::HolderC h(s);

    stream.get_octet_array((_CORBA_Octet*)s, len);
    if (s[len-1] != '\0') OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

    h.drop();
    return 1;
  }
  return 0;
}

const _CORBA_Char omniCodeSet::empty8BitTable[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
