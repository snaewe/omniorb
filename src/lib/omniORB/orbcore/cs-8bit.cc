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
  Revision 1.1.4.4  2006/04/28 18:40:46  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.3  2005/12/08 14:22:31  dgrisby
  Better string marshalling performance; other minor optimisations.

  Revision 1.1.4.2  2003/05/20 16:53:16  dgrisby
  Valuetype marshalling support.

  Revision 1.1.4.1  2003/03/23 21:02:20  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.12  2001/10/17 16:47:08  dpg1
  New minor codes

  Revision 1.1.2.11  2001/08/24 10:10:44  dpg1
  Fix braindead bound check bug in string unmarshalling.

  Revision 1.1.2.10  2001/08/17 17:12:35  sll
  Modularise ORB configuration parameters.

  Revision 1.1.2.9  2001/08/03 17:41:20  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.2.8  2001/06/13 20:12:33  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.1.2.7  2001/04/18 18:18:09  sll
  Big checkin with the brand new internal APIs.

  Revision 1.1.2.6  2000/12/05 17:43:30  dpg1
  Check for input over-run in string and wstring unmarshalling.

  Revision 1.1.2.5  2000/11/22 14:37:59  dpg1
  Code set marshalling functions now take a string length argument.

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
#include <orbParameters.h>

OMNI_USING_NAMESPACE(omni)

//
// Native code set
//

void
omniCodeSet::NCS_C_8bit::marshalChar(cdrStream& stream,
				     omniCodeSet::TCS_C* tcs,
				     _CORBA_Char c)
{
  if (!tcs) OMNIORB_THROW(BAD_INV_ORDER, 
			  BAD_INV_ORDER_CodeSetNotKnownYet,
			  (CORBA::CompletionStatus)stream.completion());

  if (tcs->fastMarshalChar(stream, this, c)) return;

  omniCodeSet::UniChar uc = pd_toU[c];
  if (c && !uc) OMNIORB_THROW(DATA_CONVERSION, 
			      DATA_CONVERSION_BadInput,
			      (CORBA::CompletionStatus)stream.completion());


  tcs->marshalChar(stream, uc);
}

void
omniCodeSet::NCS_C_8bit::marshalString(cdrStream&          stream,
				       omniCodeSet::TCS_C* tcs,
				       _CORBA_ULong        bound,
				       _CORBA_ULong        len,
				       const char*         s)
{
  if (!tcs) OMNIORB_THROW(BAD_INV_ORDER, 
			  BAD_INV_ORDER_CodeSetNotKnownYet,
			  (CORBA::CompletionStatus)stream.completion());

  if (tcs->fastMarshalString(stream, this, bound, len, s))
    return;

  if (len == 0)
    len = strlen(s);

  omniCodeSet::UniChar*    us = omniCodeSetUtil::allocU(len+1);
  omniCodeSetUtil::HolderU uh(us);
  omniCodeSet::UniChar     uc;

  for (_CORBA_ULong i=0; i<=len; i++) {
    uc = pd_toU[(_CORBA_Char)(s[i])];
    if (!uc && s[i])
      OMNIORB_THROW(DATA_CONVERSION, 
		    DATA_CONVERSION_BadInput,
		    (CORBA::CompletionStatus)stream.completion());
    us[i] = uc;
  }
  tcs->marshalString(stream, bound, len, us);
}

_CORBA_Char
omniCodeSet::NCS_C_8bit::unmarshalChar(cdrStream& stream,
				       omniCodeSet::TCS_C* tcs)
{
  if (!tcs) OMNIORB_THROW(BAD_INV_ORDER, 
			  BAD_INV_ORDER_CodeSetNotKnownYet,
			  (CORBA::CompletionStatus)stream.completion());

  _CORBA_Char c;
  if (tcs->fastUnmarshalChar(stream, this, c)) return c;

  omniCodeSet::UniChar uc = tcs->unmarshalChar(stream);

  c = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
  if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 
			      DATA_CONVERSION_BadInput,
			      (CORBA::CompletionStatus)stream.completion());

  return c;
}

_CORBA_ULong
omniCodeSet::NCS_C_8bit::unmarshalString(cdrStream& stream,
					 omniCodeSet::TCS_C* tcs,
					 _CORBA_ULong bound, char*& s)
{
  if (!tcs) OMNIORB_THROW(BAD_INV_ORDER, 
			  BAD_INV_ORDER_CodeSetNotKnownYet,
			  (CORBA::CompletionStatus)stream.completion());

  _CORBA_ULong len;
  if (tcs->fastUnmarshalString(stream, this, bound, len, s)) return len;

  omniCodeSet::UniChar* us;
  len = tcs->unmarshalString(stream, bound, us);
  OMNIORB_ASSERT(us);

  omniCodeSetUtil::HolderU uh(us);

  s = omniCodeSetUtil::allocC(len+1);
  omniCodeSetUtil::HolderC h(s);

  omniCodeSet::UniChar uc;
  _CORBA_Char          c;

  for (_CORBA_ULong i=0; i<=len; i++) {
    uc = us[i];
    c  = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
    if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 
				DATA_CONVERSION_BadInput,
				(CORBA::CompletionStatus)stream.completion());
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
  if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 
			      DATA_CONVERSION_BadInput,
			      (CORBA::CompletionStatus)stream.completion());

  stream.marshalOctet(c);
}

void
omniCodeSet::TCS_C_8bit::marshalString(cdrStream& stream,
				       _CORBA_ULong bound,
				       _CORBA_ULong len,
				       const omniCodeSet::UniChar* us)
{
  if (bound && len > bound)
    OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		  (CORBA::CompletionStatus)stream.completion());

  len++;
  len >>= stream;

  _CORBA_Char          c;
  omniCodeSet::UniChar uc;

  stream.declareArrayLength(omni::ALIGN_1, len);
  for (_CORBA_ULong i=0; i<len; i++) {
    uc = us[i];
    c = pd_fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
    if (uc && !c)  OMNIORB_THROW(DATA_CONVERSION, 
				DATA_CONVERSION_BadInput,
				(CORBA::CompletionStatus)stream.completion());
    stream.marshalOctet(c);
  }
}

omniCodeSet::UniChar
omniCodeSet::TCS_C_8bit::unmarshalChar(cdrStream& stream)
{
  _CORBA_Char c;
  c = stream.unmarshalOctet();

  omniCodeSet::UniChar uc = pd_toU[c];
  if (c && !uc)  OMNIORB_THROW(DATA_CONVERSION, 
			       DATA_CONVERSION_BadInput,
			       (CORBA::CompletionStatus)stream.completion());
  return uc;
}

_CORBA_ULong
omniCodeSet::TCS_C_8bit::unmarshalString(cdrStream& stream,
					 _CORBA_ULong bound,
					 omniCodeSet::UniChar*& us)
{
  _CORBA_ULong mlen; mlen <<= stream;  // Includes terminating null

  if (mlen == 0) {
    if (orbParameters::strictIIOP) {
      omniORB::logs(1, "Error: received an invalid zero length string.");
      OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, 
		    (CORBA::CompletionStatus)stream.completion());
    }
    else {
      omniORB::logs(1, "Warning: received an invalid zero length string. "
		    "Substituted with a proper empty string.");
      us = omniCodeSetUtil::allocU(1);
      us[0] = 0;
      return 0;
    }
  }

  if (bound && mlen-1 > bound)
    OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		  (CORBA::CompletionStatus)stream.completion());

  if (!stream.checkInputOverrun(1, mlen))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)stream.completion());

  us = omniCodeSetUtil::allocU(mlen);
  omniCodeSetUtil::HolderU uh(us);

  _CORBA_Char          c;
  omniCodeSet::UniChar uc;

  for (_CORBA_ULong i=0; i < mlen; i++) {
    c = stream.unmarshalOctet();
    uc = pd_toU[c];
    if (c && !uc) OMNIORB_THROW(DATA_CONVERSION, 
				DATA_CONVERSION_BadInput,
				(CORBA::CompletionStatus)stream.completion());
    us[i] = uc;
  }
  if (uc != 0) // String must end with null
    OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, 
		  (CORBA::CompletionStatus)stream.completion());

  uh.drop();
  return mlen - 1; // Length without terminating null
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
					   _CORBA_ULong        len,
					   const char*         s)
{
  if (ncs->id() == id()) { // Null transformation
    if (len == 0) {
      len = stream.marshalRawString(s);

      if (bound && len-1 > bound)
	OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		      (CORBA::CompletionStatus)stream.completion());
    }
    else {
      if (bound && len > bound)
	OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		      (CORBA::CompletionStatus)stream.completion());
      len++;
      len >>= stream;
      stream.put_octet_array((const _CORBA_Octet*)s, len);
    }
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
    _CORBA_ULong mlen; mlen <<= stream;

    if (mlen == 0) {
      if (orbParameters::strictIIOP) {
	omniORB::logs(1, "Error: received an invalid zero length string.");
	OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, 
		      (CORBA::CompletionStatus)stream.completion());
      }
      else {
	omniORB::logs(1, "Warning: received an invalid zero length string. "
		      "Substituted with a proper empty string.");
	s = omniCodeSetUtil::allocC(1);
	s[0] = '\0';
	return 0;
      }
    }

    if (bound && mlen-1 > bound)
      OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		    (CORBA::CompletionStatus)stream.completion());

    if (!stream.checkInputOverrun(1, mlen))
      OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		    (CORBA::CompletionStatus)stream.completion());

    s = omniCodeSetUtil::allocC(mlen);
    omniCodeSetUtil::HolderC h(s);

    stream.get_octet_array((_CORBA_Octet*)s, mlen);
    if (s[mlen-1] != '\0') 
      OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, 
		    (CORBA::CompletionStatus)stream.completion());


    h.drop();
    len = mlen - 1; // Return length without terminating null
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


