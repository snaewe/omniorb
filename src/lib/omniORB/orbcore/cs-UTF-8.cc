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
#include <codeSetUtil.h>

OMNI_NAMESPACE_BEGIN(omni)

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
  void marshalString(cdrStream& stream,
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

  TCS_C_UTF_8()
    : omniCodeSet::TCS_C(omniCodeSet::ID_UTF_8, "UTF-8",
			 omniCodeSet::CS_Other, omniCodeSetUtil::GIOP12)
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


//
// Native code set
//

void
NCS_C_UTF_8::marshalChar(cdrStream& stream,
			 omniCodeSet::TCS_C* tcs, _CORBA_Char c)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  if (tcs->fastMarshalChar(stream, this, c)) return;

  if ((c & 0x80) == 0)
    tcs->marshalChar(stream, (omniCodeSet::UniChar)c);
  else
    OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);
}

void
NCS_C_UTF_8::marshalString(cdrStream& stream, omniCodeSet::TCS_C* tcs,
			   _CORBA_ULong bound, _CORBA_ULong len, const char* s)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  if (tcs->fastMarshalString(stream, this, bound, len, s)) return;

  omniCodeSetUtil::BufferU ub;
  CORBA::ULong        	   lc;
  _CORBA_Char         	   c;
  int                  	   bytes;
  CORBA::Octet         	   error = 0;

  while (*s) {
    c     = *s++;
    bytes = utf8Count[c];
    lc    = c & utf8Mask[c];

    // This switch is an attempt to avoid overhead in pipelined
    // processors. Cases 3, 2 and 1 should drop through with no
    // branching code.
    switch (bytes) {
    case 6: OMNIORB_THROW(BAD_PARAM,       0, CORBA::COMPLETED_MAYBE);
    case 5:
    case 4: OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);
    case 3: c = *s++; lc = (lc << 6) | (c & 0x3f); error |= (c & 0xc0) ^ 0x80;
    case 2: c = *s++; lc = (lc << 6) | (c & 0x3f); error |= (c & 0xc0) ^ 0x80;
    case 1: c = *s++; lc = (lc << 6) | (c & 0x3f); error |= (c & 0xc0) ^ 0x80;
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
    // By testing the error here, rather than immediately after the
    // switch, we might avoid stalling the pipeline waiting for error
    // to become available.
    if (error) {
      // At least one extension byte was not of the form 10xxxxxx
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);
    }
  }
  // Null terminator
  ub.insert(0);
  tcs->marshalString(stream, ub.length() - 1, ub.buffer());
}


_CORBA_Char
NCS_C_UTF_8::unmarshalChar(cdrStream& stream, omniCodeSet::TCS_C* tcs)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  _CORBA_Char c;
  if (tcs->fastUnmarshalChar(stream, this, c)) return c;

  omniCodeSet::UniChar uc = tcs->unmarshalChar(stream);

  if (uc <= 0x7f)
    return uc;
  else
    OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_NO);

  return 0; // For broken compilers
}

_CORBA_ULong
NCS_C_UTF_8::unmarshalString(cdrStream& stream, omniCodeSet::TCS_C* tcs,
			     _CORBA_ULong bound, char*& s)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
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
	OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);
      }
      uc = us[i];
      if (uc < 0xdc00 || uc > 0xdfff) {
	// Value is not a valid second half to a surrogate pair
	OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);
      }
      lc = lc + uc - 0xdc00 + 0x10000;

      b.insert(0xf0 | ((lc & 0x001c0000) >> 18));
      b.insert(0x80 | ((lc & 0x0003f000) >> 12));
      b.insert(0x80 | ((lc & 0x00000fc0) >>  6));
      b.insert(0x80 | ((lc & 0x000000ef)      ));
    }
    else if (uc < 0xe000) {
      // Second half of surrogate pair not allowed on its own
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);
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
    OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);
}

void
TCS_C_UTF_8::marshalString(cdrStream& stream,
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
	OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);
      }
      uc = us[i];
      if (uc < 0xdc00 || uc > 0xdfff) {
	// Value is not a valid second half to a surrogate pair
	OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);
      }
      lc = lc + uc - 0xdc00 + 0x10000;

      b.insert(0xf0 | ((lc & 0x001c0000) >> 18));
      b.insert(0x80 | ((lc & 0x0003f000) >> 12));
      b.insert(0x80 | ((lc & 0x00000fc0) >>  6));
      b.insert(0x80 | ((lc & 0x000000ef)      ));
    }
    else if (uc < 0xe000) {
      // Second half of surrogate pair not allowed on its own
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);
    }
    else {
      b.insert(0xe0 | ((uc & 0xf000) >> 12));
      b.insert(0x80 | ((uc & 0x0fc0) >>  6));
      b.insert(0x80 | ((uc & 0x003f)      ));
    }
  }
  _CORBA_ULong mlen = b.length();
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
    OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_NO);

  return 0; // For broken compilers
}

_CORBA_ULong
TCS_C_UTF_8::unmarshalString(cdrStream& stream,
			     _CORBA_ULong bound, omniCodeSet::UniChar*& us)
{
  _CORBA_ULong len; len <<= stream;

  if (len == 0)
    OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_NO);

  if (bound && len >= bound)
    OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);

  if (!stream.checkInputOverrun(1, len))
    OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

  omniCodeSetUtil::BufferU ub;
  CORBA::ULong         	   lc;
  _CORBA_Char          	   c;
  int                  	   bytes;
  CORBA::Octet         	   error = 0;

  for (_CORBA_ULong i=0; i < len; i++) {
    c   = stream.unmarshalOctet();
    bytes = utf8Count[c];
    lc    = c & utf8Mask[c];

    // This switch is an attempt to avoid overhead in pipelined
    // processors. Cases 3, 2 and 1 should drop through with no
    // branching code.
    switch (bytes) {
    case 6: OMNIORB_THROW(BAD_PARAM,       0, CORBA::COMPLETED_MAYBE);
    case 5:
    case 4: OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);
    case 3:
      c = stream.unmarshalOctet(); i++;
      lc = (lc << 6) | (c & 0x3f); error |= (c & 0xc0) ^ 0x80;
    case 2:
      c = stream.unmarshalOctet(); i++;
      lc = (lc << 6) | (c & 0x3f); error |= (c & 0xc0) ^ 0x80;
    case 1:
      c = stream.unmarshalOctet(); i++;
      lc = (lc << 6) | (c & 0x3f); error |= (c & 0xc0) ^ 0x80;
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
    // By testing the error here, rather than immediately after the
    // switch, we might avoid stalling the pipeline waiting for error
    // to become available.
    if (error) {
      // At least one extension byte was not of the form 10xxxxxx
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);
    }
  }
  if (lc != 0) // Check for null-terminator
    OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

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
    if (bound && len > bound)
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

    _CORBA_ULong mlen = len + 1;
    mlen >>= stream;
    stream.put_octet_array((const _CORBA_Octet*)s, mlen);
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
	OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);
      }
      else {
	b.insert(0xe0 | ((uc & 0xf000) >> 12));
	b.insert(0x80 | ((uc & 0x0fc0) >>  6));
	b.insert(0x80 | ((uc & 0x003f)      ));
      }
    }
    b.insert(0); // Null terminator
    _CORBA_ULong mlen = b.length();

    if (bound && mlen >= bound)
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

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

    if (mlen == 0) // Zero length is invalid
      OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

    if (bound && mlen >= bound)
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

    if (!stream.checkInputOverrun(1, mlen))
      OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

    s = omniCodeSetUtil::allocC(mlen);
    omniCodeSetUtil::HolderC h(s);

    stream.get_octet_array((_CORBA_Octet*)s, mlen);
    if (s[mlen-1] != '\0') OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

    h.drop();
    len = mlen - 1;
    return 1;
  }
  else if (ncs->kind() == omniCodeSet::CS_8bit) { // Simple 8-bit set

    const _CORBA_Char** fromU = ((omniCodeSet::NCS_C_8bit*)ncs)->fromU();

    _CORBA_ULong mlen; mlen <<= stream;

    if (mlen == 0)
      OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_NO);

    if (bound && mlen >= bound)
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);

    if (!stream.checkInputOverrun(1, mlen))
      OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

    omniCodeSetUtil::BufferC        b; // *** Could initialise to mlen here
    omniCodeSet::UniChar 	    uc;
    _CORBA_Char          	    c;
    int                  	    bytes;
    CORBA::Octet         	    error = 0;

    for (_CORBA_ULong i=0; i < mlen; i++) {
      c   = stream.unmarshalOctet();
      bytes = utf8Count[c];
      uc    = c & utf8Mask[c];

      // This switch is an attempt to avoid overhead in pipelined
      // processors. Cases 2 and 1 should drop through with no
      // branching code.
      switch (bytes) {
      case 6: OMNIORB_THROW(BAD_PARAM,       0, CORBA::COMPLETED_MAYBE);
      case 5:
      case 4:
      case 3: OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);
      case 2:
	c = stream.unmarshalOctet(); i++;
	uc = (uc << 6) | (c & 0x3f); error |= (c & 0xc0) ^ 0x80;
      case 1:
	c = stream.unmarshalOctet(); i++;
	uc = (uc << 6) | (c & 0x3f); error |= (c & 0xc0) ^ 0x80;
      }
      c = fromU[(uc & 0xff00) >> 8][uc & 0x00ff];
      if (uc && !c) OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);
      b.insert(c);

      if (error) {
	// At least one extension byte was not of the form 10xxxxxx
	OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);
      }
    }
    if (uc != 0) // Check for null-terminator
      OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

    s   = b.extract();
    len = b.length() - 1;
    return 1;
  }
  return 0;
}



//
// Initialiser
//

static NCS_C_UTF_8 _NCS_C_UTF_8;
static TCS_C_UTF_8 _TCS_C_UTF_8;

class CS_UTF_8_init {
public:
  CS_UTF_8_init() {
    omniCodeSet::registerNCS_C(&_NCS_C_UTF_8);
    omniCodeSet::registerTCS_C(&_TCS_C_UTF_8);
  }
};

static CS_UTF_8_init _CS_UTF_8_init;

OMNI_NAMESPACE_END(omni)
