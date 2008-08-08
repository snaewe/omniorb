// -*- Mode: C++; -*-
//                            Package   : omniORB
// cs-UTF-8.cc                Created on: 20/10/2000
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
//    Unicode / ISO 10646 UTF-8 code set

/*
  $Log$
  Revision 1.1.4.5  2008/08/08 16:52:56  dgrisby
  Option to validate untransformed UTF-8; correct data conversion minor
  codes; better logging for MessageErrors.

  Revision 1.1.4.4  2006/04/28 18:40:46  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.3  2005/12/08 14:22:31  dgrisby
  Better string marshalling performance; other minor optimisations.

  Revision 1.1.4.2  2005/01/06 23:10:14  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:02:20  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.13  2001/10/17 16:47:09  dpg1
  New minor codes

  Revision 1.1.2.12  2001/08/24 10:10:44  dpg1
  Fix braindead bound check bug in string unmarshalling.

  Revision 1.1.2.11  2001/08/17 17:12:36  sll
  Modularise ORB configuration parameters.

  Revision 1.1.2.10  2001/08/03 17:41:20  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.2.9  2001/07/26 16:37:20  dpg1
  Make sure static initialisers always run.

  Revision 1.1.2.8  2001/04/18 18:18:09  sll
  Big checkin with the brand new internal APIs.

  Revision 1.1.2.7  2000/12/05 17:43:31  dpg1
  Check for input over-run in string and wstring unmarshalling.

  Revision 1.1.2.6  2000/11/22 14:38:00  dpg1
  Code set marshalling functions now take a string length argument.

  Revision 1.1.2.5  2000/11/17 19:12:07  dpg1
  Better choice of exceptions in UTF-8.

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

  Revision 1.1.2.1  2000/10/27 15:42:09  dpg1
  Initial code set conversion support. Not yet enabled or fully tested.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/linkHacks.h>
#include <codeSetUtil.h>
#include <orbParameters.h>
#include <orbOptions.h>


OMNI_NAMESPACE_BEGIN(omni)

static CORBA::Boolean validateUTF8 = 0;


class NCS_C_UTF_8 : public omniCodeSet::NCS_C {
public:

  void marshalChar(cdrStream& stream, omniCodeSet::TCS_C* tcs, _CORBA_Char c);

  void marshalString(cdrStream& stream, omniCodeSet::TCS_C* tcs,
		     _CORBA_ULong bound, _CORBA_ULong len, const char* s);

  _CORBA_Char unmarshalChar(cdrStream& stream, omniCodeSet::TCS_C* tcs);

  _CORBA_ULong unmarshalString(cdrStream& stream, omniCodeSet::TCS_C* tcs,
			       _CORBA_ULong bound, char*& s);

  NCS_C_UTF_8()
    : omniCodeSet::NCS_C(omniCodeSet::ID_UTF_8, "UTF-8", omniCodeSet::CS_Other)
  { }

  virtual ~NCS_C_UTF_8() {}
};


class TCS_C_UTF_8 : public omniCodeSet::TCS_C {
public:

  // Unicode based marshalling
  void marshalChar  (cdrStream& stream, omniCodeSet::UniChar uc);
  void marshalString(cdrStream& stream, _CORBA_ULong bound,
		     _CORBA_ULong len, const omniCodeSet::UniChar* us);

  omniCodeSet::UniChar unmarshalChar(cdrStream& stream);

  _CORBA_ULong unmarshalString(cdrStream& stream,
			       _CORBA_ULong bound, omniCodeSet::UniChar*& us);

  // Fast marshalling functions. Return false if no fast case is
  // possible and UCS-2 functions should be used.
  _CORBA_Boolean fastMarshalChar    (cdrStream&          stream,
				     omniCodeSet::NCS_C* ncs,
				     _CORBA_Char         c);

  _CORBA_Boolean fastMarshalString  (cdrStream&          stream,
				     omniCodeSet::NCS_C* ncs,
				     _CORBA_ULong        bound,
				     _CORBA_ULong        len,
				     const char*         s);

  _CORBA_Boolean fastUnmarshalChar  (cdrStream&          stream,
				     omniCodeSet::NCS_C* ncs,
				     _CORBA_Char&        c);
  
  _CORBA_Boolean fastUnmarshalString(cdrStream&          stream,
				     omniCodeSet::NCS_C* ncs,
				     _CORBA_ULong        bound,
				     _CORBA_ULong&       length,
				     char*&              s);

  void validateString(const char* s, CORBA::CompletionStatus completion);

  TCS_C_UTF_8(GIOP::Version v)
    : omniCodeSet::TCS_C(omniCodeSet::ID_UTF_8, "UTF-8",
			 omniCodeSet::CS_Other, v)
  { }

  virtual ~TCS_C_UTF_8() {}
};

// Table indicating how many bytes follow the first byte of a UTF-8 sequence
static CORBA::Octet utf8Count[256] = {
  // 0xxxxxxx no more bytes
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  // 10xxxxxx is invalid
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,

  // 110xxxxx one more byte
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

  // 1110xxxx two more bytes
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

  // 11110xxx three more bytes (UTF-16)
  3, 3, 3, 3, 3, 3, 3, 3,

  // 111110xx four more bytes. Too big for UTF-16
  4, 4, 4, 4,

  // 111111xx five more bytes. *** How does this work?
  5, 5, 5, 5
};

// Mask to remove the prefix bits from the first byte of a UTF-8 sequence
static CORBA::Char utf8Mask[256] = {
  // 0xxxxxxx
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
  0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,

  // 10xxxxxx invalid
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  // 110xxxxx
  0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
  0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
  0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
  0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f,

  // 1110xxxx
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,

  // 11110xxx
  0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,

  // 111110xx and 111111xx
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03
};


// Validate an extension character

static inline void
validateExt(_CORBA_Char c, CORBA::CompletionStatus completion)
{
  if ((c & 0xc0) ^ 0x80)
	OMNIORB_THROW(DATA_CONVERSION,
		      DATA_CONVERSION_BadInput,
		      completion);
}




//
// Native code set
//

void
NCS_C_UTF_8::marshalChar(cdrStream& stream,
			 omniCodeSet::TCS_C* tcs, _CORBA_Char c)
{
  if (!tcs) OMNIORB_THROW(BAD_INV_ORDER, 
			  BAD_INV_ORDER_CodeSetNotKnownYet,
			  (CORBA::CompletionStatus)stream.completion());

  if (tcs->fastMarshalChar(stream, this, c)) return;

  if ((c & 0x80) == 0)
    tcs->marshalChar(stream, (omniCodeSet::UniChar)c);
  else
    OMNIORB_THROW(DATA_CONVERSION, 
		  DATA_CONVERSION_BadInput,
		  (CORBA::CompletionStatus)stream.completion());

}

void
NCS_C_UTF_8::marshalString(cdrStream& stream, omniCodeSet::TCS_C* tcs,
			   _CORBA_ULong bound, _CORBA_ULong len, const char* s)
{
  if (!tcs) OMNIORB_THROW(BAD_INV_ORDER, 
			  BAD_INV_ORDER_CodeSetNotKnownYet,
			  (CORBA::CompletionStatus)stream.completion());

  if (tcs->fastMarshalString(stream, this, bound, len, s)) return;

  CORBA::CompletionStatus completion =
    (CORBA::CompletionStatus)stream.completion();

  omniCodeSetUtil::BufferU ub;
  CORBA::ULong        	   lc;
  _CORBA_Char         	   c;
  int                  	   bytes;

  while (*s) {
    c     = *s++;
    bytes = utf8Count[c];
    lc    = c & utf8Mask[c];

    switch (bytes) {
    case 6:
    case 5:
    case 4: OMNIORB_THROW(DATA_CONVERSION, 
			  DATA_CONVERSION_CannotMapChar,
			  completion);
    case 3: c = *s++; lc = (lc << 6) | (c & 0x3f); validateExt(c, completion);
    case 2: c = *s++; lc = (lc << 6) | (c & 0x3f); validateExt(c, completion);
    case 1: c = *s++; lc = (lc << 6) | (c & 0x3f); validateExt(c, completion);
    }
    if (lc <= 0xffff) {
      // Single unicode char
      ub.insert(lc);
    }
    else {
      // Surrogate pair
      lc -= 0x10000;
      ub.insert((lc >> 10)    + 0xd800);
      ub.insert((lc &  0x3ff) + 0xdc00);
    }
  }
  // Null terminator
  len = ub.length();
  ub.insert(0);
  tcs->marshalString(stream, bound, len, ub.buffer());
}


_CORBA_Char
NCS_C_UTF_8::unmarshalChar(cdrStream& stream, omniCodeSet::TCS_C* tcs)
{
  if (!tcs) OMNIORB_THROW(BAD_INV_ORDER, 
			  BAD_INV_ORDER_CodeSetNotKnownYet,
			  (CORBA::CompletionStatus)stream.completion());

  _CORBA_Char c;
  if (tcs->fastUnmarshalChar(stream, this, c)) return c;

  omniCodeSet::UniChar uc = tcs->unmarshalChar(stream);

  if (uc <= 0x7f)
    return uc;
  else
    OMNIORB_THROW(DATA_CONVERSION, 
		  DATA_CONVERSION_BadInput,
		  (CORBA::CompletionStatus)stream.completion());

  return 0; // For broken compilers
}

_CORBA_ULong
NCS_C_UTF_8::unmarshalString(cdrStream& stream, omniCodeSet::TCS_C* tcs,
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
  omniCodeSetUtil::BufferC b;
  omniCodeSet::UniChar     uc;

  for (_CORBA_ULong i=0; i<=len; i++) {
    uc = us[i];

    if      (uc < 0x0080) {
      b.insert(uc);
    }
    else if (uc < 0x0800) {
      b.insert(0xc0 | ((uc & 0x07c0) >>  6));
      b.insert(0x80 | ((uc & 0x003f)      ));
    }
    else if (uc < 0xd800) {
      b.insert(0xe0 | ((uc & 0xf000) >> 12));
      b.insert(0x80 | ((uc & 0x0fc0) >>  6));
      b.insert(0x80 | ((uc & 0x003f)      ));
    }
    else if (uc < 0xdc00) {
      // Surrogate pair
      _CORBA_ULong lc = (uc - 0xd800) << 10;
      if (++i == len) {
	// No second half to surrogate pair
	OMNIORB_THROW(DATA_CONVERSION, 
		      DATA_CONVERSION_BadInput,
		      (CORBA::CompletionStatus)stream.completion());
      }
      uc = us[i];
      if (uc < 0xdc00 || uc > 0xdfff) {
	// Value is not a valid second half to a surrogate pair
	OMNIORB_THROW(DATA_CONVERSION, 
		      DATA_CONVERSION_BadInput,
		      (CORBA::CompletionStatus)stream.completion());
      }
      lc = lc + uc - 0xdc00 + 0x10000;

      b.insert(0xf0 | ((lc & 0x001c0000) >> 18));
      b.insert(0x80 | ((lc & 0x0003f000) >> 12));
      b.insert(0x80 | ((lc & 0x00000fc0) >>  6));
      b.insert(0x80 | ((lc & 0x000000ef)      ));
    }
    else if (uc < 0xe000) {
      // Second half of surrogate pair not allowed on its own
      OMNIORB_THROW(DATA_CONVERSION,
		    DATA_CONVERSION_BadInput,
		    (CORBA::CompletionStatus)stream.completion());
    }
    else {
      b.insert(0xe0 | ((uc & 0xf000) >> 12));
      b.insert(0x80 | ((uc & 0x0fc0) >>  6));
      b.insert(0x80 | ((uc & 0x003f)      ));
    }
  }
  OMNIORB_ASSERT(uc == 0); // Last char must be zero

  s = b.extract();
  return b.length() - 1;
}


//
// Transmission code set
//

void
TCS_C_UTF_8::marshalChar(cdrStream& stream, omniCodeSet::UniChar uc)
{
  if (uc < 0x80) {
    _CORBA_Char c = uc;
    stream.marshalOctet(c);
  }
  else
    OMNIORB_THROW(DATA_CONVERSION, 
		  DATA_CONVERSION_BadInput,
		  (CORBA::CompletionStatus)stream.completion());
}

void
TCS_C_UTF_8::marshalString(cdrStream& stream,
			   _CORBA_ULong bound,
			   _CORBA_ULong len,
			   const omniCodeSet::UniChar* us)
{
  omniCodeSetUtil::BufferC b;
  omniCodeSet::UniChar     uc;

  for (_CORBA_ULong i=0; i<=len; i++) {
    uc = us[i];

    if      (uc < 0x0080) {
      b.insert(uc);
    }
    else if (uc < 0x0800) {
      b.insert(0xc0 | ((uc & 0x07c0) >>  6));
      b.insert(0x80 | ((uc & 0x003f)      ));
    }
    else if (uc < 0xd800) {
      b.insert(0xe0 | ((uc & 0xf000) >> 12));
      b.insert(0x80 | ((uc & 0x0fc0) >>  6));
      b.insert(0x80 | ((uc & 0x003f)      ));
    }
    else if (uc < 0xdc00) {
      // Surrogate pair
      _CORBA_ULong lc = (uc - 0xd800) << 10;
      if (++i == len) {
	// No second half to surrogate pair
	OMNIORB_THROW(DATA_CONVERSION, 
		      DATA_CONVERSION_BadInput,
		      (CORBA::CompletionStatus)stream.completion());
      }
      uc = us[i];
      if (uc < 0xdc00 || uc > 0xdfff) {
	// Value is not a valid second half to a surrogate pair
	OMNIORB_THROW(DATA_CONVERSION, 
		      DATA_CONVERSION_BadInput,
		      (CORBA::CompletionStatus)stream.completion());
      }
      lc = lc + uc - 0xdc00 + 0x10000;

      b.insert(0xf0 | ((lc & 0x001c0000) >> 18));
      b.insert(0x80 | ((lc & 0x0003f000) >> 12));
      b.insert(0x80 | ((lc & 0x00000fc0) >>  6));
      b.insert(0x80 | ((lc & 0x000000ef)      ));
    }
    else if (uc < 0xe000) {
      // Second half of surrogate pair not allowed on its own
      OMNIORB_THROW(DATA_CONVERSION, 
		    DATA_CONVERSION_BadInput,
		    (CORBA::CompletionStatus)stream.completion());
    }
    else {
      b.insert(0xe0 | ((uc & 0xf000) >> 12));
      b.insert(0x80 | ((uc & 0x0fc0) >>  6));
      b.insert(0x80 | ((uc & 0x003f)      ));
    }
  }
  _CORBA_ULong mlen = b.length();

  if (bound && mlen-1 > bound)
    OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		  (CORBA::CompletionStatus)stream.completion());

  mlen >>= stream;
  stream.put_octet_array((const _CORBA_Octet*)b.buffer(), mlen);
}


omniCodeSet::UniChar
TCS_C_UTF_8::unmarshalChar(cdrStream& stream)
{
  _CORBA_Char c;
  c = stream.unmarshalOctet();

  if (c < 0x80)
    return c;
  else
    OMNIORB_THROW(DATA_CONVERSION, 
		  DATA_CONVERSION_BadInput,
		  (CORBA::CompletionStatus)stream.completion());

  return 0; // For broken compilers
}

_CORBA_ULong
TCS_C_UTF_8::unmarshalString(cdrStream& stream,
			     _CORBA_ULong bound, omniCodeSet::UniChar*& us)
{
  CORBA::CompletionStatus completion =
    (CORBA::CompletionStatus)stream.completion();

  _CORBA_ULong len; len <<= stream;

  if (len == 0) {
    if (orbParameters::strictIIOP) {
      omniORB::logs(1, "Error: received an invalid zero length string.");
      OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, completion);
    }
    else {
      omniORB::logs(1, "Warning: received an invalid zero length string. "
		    "Substituted with a proper empty string.");
      us = omniCodeSetUtil::allocU(1);
      us[0] = 0;
      return 0;
    }
  }

  if (bound && len-1 > bound)
    OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, completion);


  if (!stream.checkInputOverrun(1, len))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage, completion);


  omniCodeSetUtil::BufferU ub;
  CORBA::ULong         	   lc;
  _CORBA_Char          	   c;
  int                  	   bytes;

  for (_CORBA_ULong i=0; i < len; i++) {
    c     = stream.unmarshalOctet();
    bytes = utf8Count[c];
    lc    = c & utf8Mask[c];

    switch (bytes) {
    case 6: OMNIORB_THROW(DATA_CONVERSION, 
			  DATA_CONVERSION_BadInput,
			  completion);
    case 5:
    case 4: OMNIORB_THROW(DATA_CONVERSION, 
			  DATA_CONVERSION_CannotMapChar,
			  completion);
    case 3:
      c = stream.unmarshalOctet(); i++;
      lc = (lc << 6) | (c & 0x3f); validateExt(c, completion);
    case 2:
      c = stream.unmarshalOctet(); i++;
      lc = (lc << 6) | (c & 0x3f); validateExt(c, completion);
    case 1:
      c = stream.unmarshalOctet(); i++;
      lc = (lc << 6) | (c & 0x3f); validateExt(c, completion);
    }
    if (lc <= 0xffff) {
      // Single unicode char
      ub.insert(lc);
    }
    else {
      // Surrogate pair
      lc -= 0x10000;
      ub.insert((lc >> 10)    + 0xd800);
      ub.insert((lc &  0x3ff) + 0xdc00);
    }
  }
  if (lc != 0) // Check for null-terminator
    OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, completion);

  us = ub.extract();
  return ub.length() - 1;
}


_CORBA_Boolean
TCS_C_UTF_8::fastMarshalChar(cdrStream&          stream,
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
TCS_C_UTF_8::fastMarshalString(cdrStream&          stream,
			       omniCodeSet::NCS_C* ncs,
			       _CORBA_ULong        bound,
			       _CORBA_ULong        len,
			       const char*         s)
{
  if (ncs->id() == id()) { // Null transformation
    if (validateUTF8)
      validateString(s, (CORBA::CompletionStatus)stream.completion());

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

      _CORBA_ULong mlen = len + 1;
      mlen >>= stream;
      stream.put_octet_array((const _CORBA_Octet*)s, mlen);
    }
    return 1;
  }
  else if (ncs->kind() == omniCodeSet::CS_8bit) { // Simple 8 bit code set

    const omniCodeSet::UniChar* toU = ((omniCodeSet::NCS_C_8bit*)ncs)->toU();

    omniCodeSetUtil::BufferC b;
    omniCodeSet::UniChar     uc;

    while (*s) {
      uc = toU[(_CORBA_Char)*s++];

      if      (uc < 0x0080) {
	b.insert(uc);
      }
      else if (uc < 0x0800) {
	b.insert(0xc0 | ((uc & 0x07c0) >>  6));
	b.insert(0x80 | ((uc & 0x003f)      ));
      }
      else if (uc < 0xd800) {
	b.insert(0xe0 | ((uc & 0xf000) >> 12));
	b.insert(0x80 | ((uc & 0x0fc0) >>  6));
	b.insert(0x80 | ((uc & 0x003f)      ));
      }
      else if (uc < 0xe000) {
	// Surrogate pairs shouldn't happen
	OMNIORB_THROW(DATA_CONVERSION, 
		      DATA_CONVERSION_BadInput,
		      (CORBA::CompletionStatus)stream.completion());
      }
      else {
	b.insert(0xe0 | ((uc & 0xf000) >> 12));
	b.insert(0x80 | ((uc & 0x0fc0) >>  6));
	b.insert(0x80 | ((uc & 0x003f)      ));
      }
    }
    b.insert(0); // Null terminator
    _CORBA_ULong mlen = b.length();

    if (bound && mlen-1 > bound)
      OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, 
		    (CORBA::CompletionStatus)stream.completion());

    mlen >>= stream;
    stream.put_octet_array((const _CORBA_Octet*)b.buffer(), mlen);
    return 1;
  }
  return 0;
}

_CORBA_Boolean
TCS_C_UTF_8::fastUnmarshalChar(cdrStream&          stream,
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
TCS_C_UTF_8::fastUnmarshalString(cdrStream&          stream,
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

    if (validateUTF8)
      validateString(s, (CORBA::CompletionStatus)stream.completion());

    h.drop();
    len = mlen - 1;
    return 1;
  }
  else if (ncs->kind() == omniCodeSet::CS_8bit) { // Simple 8-bit set

    CORBA::CompletionStatus completion =
      (CORBA::CompletionStatus)stream.completion();

    const _CORBA_Char** fromU = ((omniCodeSet::NCS_C_8bit*)ncs)->fromU();

    _CORBA_ULong mlen; mlen <<= stream;

    if (mlen == 0) {
      if (orbParameters::strictIIOP) {
	omniORB::logs(1, "Error: received an invalid zero length string.");
	OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, completion);
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
      OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, completion);

    if (!stream.checkInputOverrun(1, mlen))
      OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage, completion);

    omniCodeSetUtil::BufferC        b; // *** Could initialise to mlen here
    omniCodeSet::UniChar 	    uc;
    _CORBA_Char          	    c;
    int                  	    bytes;

    for (_CORBA_ULong i=0; i < mlen; i++) {
      c     = stream.unmarshalOctet();
      bytes = utf8Count[c];
      uc    = c & utf8Mask[c];

      switch (bytes) {
      case 6: OMNIORB_THROW(DATA_CONVERSION, 
			    DATA_CONVERSION_BadInput,
			    completion);
      case 5:
      case 4:
      case 3: OMNIORB_THROW(DATA_CONVERSION, 
			    DATA_CONVERSION_CannotMapChar,
			    completion);
      case 2:
	c = stream.unmarshalOctet(); i++;
	uc = (uc << 6) | (c & 0x3f); validateExt(c, completion);
      case 1:
	c = stream.unmarshalOctet(); i++;
	uc = (uc << 6) | (c & 0x3f); validateExt(c, completion);
      }
      c = fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
      if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 
				  DATA_CONVERSION_CannotMapChar,
				  completion);

      b.insert(c);
    }
    if (uc != 0) // Check for null-terminator
      OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, completion);

    s   = b.extract();
    len = b.length() - 1;
    return 1;
  }
  return 0;
}


void
TCS_C_UTF_8::validateString(const char* s, CORBA::CompletionStatus completion)
{
  // Check that string is valid UTF-8 data.

  int bytes;

  while (*s) {
    bytes = utf8Count[*s++];

    switch (bytes) {
    case 6:
    case 5: OMNIORB_THROW(DATA_CONVERSION,
			  DATA_CONVERSION_BadInput,
			  completion);
    case 4: validateExt(*s++, completion);
    case 3: validateExt(*s++, completion);
    case 2: validateExt(*s++, completion);
    case 1: validateExt(*s++, completion);
    }
  }
}


//
// Initialiser
//

/////////////////////////////////////////////////////////////////////////////
class validateUTF8Handler : public orbOptions::Handler {
public:

  validateUTF8Handler() : 
    orbOptions::Handler("validateUTF8",
			"validateUTF8 = 0 or 1",
			1,
			"-ORBvalidateUTF8 < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    validateUTF8 = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(), validateUTF8, result);
  }
};

static validateUTF8Handler validateUTF8Handler_;



static NCS_C_UTF_8 _NCS_C_UTF_8;
static TCS_C_UTF_8 _TCS_C_UTF_8_11(omniCodeSetUtil::GIOP11);
static TCS_C_UTF_8 _TCS_C_UTF_8_12(omniCodeSetUtil::GIOP12);

class CS_UTF_8_init {
public:
  CS_UTF_8_init() {
    orbOptions::singleton().registerHandler(validateUTF8Handler_);
    omniCodeSet::registerNCS_C(&_NCS_C_UTF_8);
    omniCodeSet::registerTCS_C(&_TCS_C_UTF_8_11);
    omniCodeSet::registerTCS_C(&_TCS_C_UTF_8_12);
  }
};

static CS_UTF_8_init _CS_UTF_8_init;

OMNI_NAMESPACE_END(omni)

OMNI_EXPORT_LINK_FORCE_SYMBOL(CS_UTF_8);
