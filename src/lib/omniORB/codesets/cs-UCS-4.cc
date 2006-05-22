// -*- Mode: C++; -*-
//                            Package   : omniORB
// cs-UCS-4.cc                Created on: 26/10/2000
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
//    Native code set for Unicode / ISO 10646 UCS-4

/*
  $Log$
  Revision 1.1.4.5  2006/05/22 15:44:52  dgrisby
  Make sure string length and body are never split across a chunk
  boundary.

  Revision 1.1.4.4  2005/12/08 14:22:31  dgrisby
  Better string marshalling performance; other minor optimisations.

  Revision 1.1.4.3  2005/01/06 23:09:43  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.2  2003/05/20 16:53:14  dgrisby
  Valuetype marshalling support.

  Revision 1.1.4.1  2003/03/23 21:02:51  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.6  2001/10/17 16:47:08  dpg1
  New minor codes

  Revision 1.1.2.5  2001/07/26 16:37:20  dpg1
  Make sure static initialisers always run.

  Revision 1.1.2.4  2001/04/19 09:18:43  sll
  Scoped where appropriate with the omni namespace.

  Revision 1.1.2.3  2000/12/05 17:43:30  dpg1
  Check for input over-run in string and wstring unmarshalling.

  Revision 1.1.2.2  2000/11/22 14:37:59  dpg1
  Code set marshalling functions now take a string length argument.

  Revision 1.1.2.1  2000/11/16 12:37:16  dpg1
  Implement UCS-4 transmission code set, and move it to the codeSets
  library.

  Revision 1.1.2.3  2000/11/10 15:41:36  dpg1
  Native code sets throw BAD_PARAM if they are given a null transmission
  code set.

  Revision 1.1.2.2  2000/11/02 10:16:27  dpg1
  Correct some minor errors in code set implementation. Remove Big5
  converter since it's wrong.

  Revision 1.1.2.1  2000/10/27 15:42:08  dpg1
  Initial code set conversion support. Not yet enabled or fully tested.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/linkHacks.h>
#include <codeSetUtil.h>

OMNI_NAMESPACE_BEGIN(omni)

#if (SIZEOF_WCHAR == 4)

class NCS_W_UCS_4 : public omniCodeSet::NCS_W {
public:

  virtual void marshalWChar(cdrStream& stream, omniCodeSet::TCS_W* tcs,
			    _CORBA_WChar c);

  virtual void marshalWString(cdrStream& stream, omniCodeSet::TCS_W* tcs,
			      _CORBA_ULong bound, _CORBA_ULong len,
			      const _CORBA_WChar* s);

  virtual _CORBA_WChar unmarshalWChar(cdrStream& stream,
				      omniCodeSet::TCS_W* tcs);

  virtual _CORBA_ULong unmarshalWString(cdrStream& stream,
					omniCodeSet::TCS_W* tcs,
					_CORBA_ULong bound,
					_CORBA_WChar*& s);

  NCS_W_UCS_4()
    : omniCodeSet::NCS_W(omniCodeSet::ID_UCS_4, "UCS-4", omniCodeSet::CS_Other)
  { }

  virtual ~NCS_W_UCS_4() {}
};


class TCS_W_UCS_4 : public omniCodeSet::TCS_W {
public:

  virtual void marshalWChar  (cdrStream& stream, omniCodeSet::UniChar uc);
  virtual void marshalWString(cdrStream& stream,
			      _CORBA_ULong bound,
			      _CORBA_ULong len,
			      const omniCodeSet::UniChar* us);

  virtual omniCodeSet::UniChar unmarshalWChar(cdrStream& stream);

  virtual _CORBA_ULong unmarshalWString(cdrStream& stream,
					_CORBA_ULong bound,
					omniCodeSet::UniChar*& us);

  // Fast marshalling functions. Return false if no fast case is
  // possible and Unicode functions should be used.
  virtual _CORBA_Boolean fastMarshalWChar    (cdrStream&          stream,
					      omniCodeSet::NCS_W* ncs,
					      _CORBA_WChar        c);

  virtual _CORBA_Boolean fastMarshalWString  (cdrStream&          stream,
					      omniCodeSet::NCS_W* ncs,
					      _CORBA_ULong        bound,
					      _CORBA_ULong        len,
					      const _CORBA_WChar* s);

  virtual _CORBA_Boolean fastUnmarshalWChar  (cdrStream&          stream,
					      omniCodeSet::NCS_W* ncs,
					      _CORBA_WChar&       c);

  virtual _CORBA_Boolean fastUnmarshalWString(cdrStream&          stream,
					      omniCodeSet::NCS_W* ncs,
					      _CORBA_ULong        bound,
					      _CORBA_ULong&       length,
					      _CORBA_WChar*&      s);

  TCS_W_UCS_4()
    : omniCodeSet::TCS_W(omniCodeSet::ID_UCS_4, "UCS-4",
			 omniCodeSet::CS_Other, omniCodeSetUtil::GIOP12)
  { }

  virtual ~TCS_W_UCS_4() {}
};


void
NCS_W_UCS_4::marshalWChar(cdrStream& stream,
			  omniCodeSet::TCS_W* tcs,
			  _CORBA_WChar wc)
{
  OMNIORB_CHECK_TCS_W_FOR_MARSHAL(tcs, stream);

  if (tcs->fastMarshalWChar(stream, this, wc)) return;

  if (wc > 0xffff)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WCharOutOfRange,
		  (CORBA::CompletionStatus)stream.completion());

  tcs->marshalWChar(stream, wc);
}


void
NCS_W_UCS_4::marshalWString(cdrStream&          stream,
			    omniCodeSet::TCS_W* tcs,
			    _CORBA_ULong        bound,
			    _CORBA_ULong        len,
			    const _CORBA_WChar* ws)
{
  OMNIORB_CHECK_TCS_W_FOR_MARSHAL(tcs, stream);

  if (tcs->fastMarshalWString(stream, this, bound, len, ws)) return;

  if (bound && len > bound)
    OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, 
		  (CORBA::CompletionStatus)stream.completion());

  omniCodeSetUtil::BufferU ub(len+1);
  _CORBA_WChar             wc;

  for (_CORBA_ULong i=0; i<=len; i++) {
    wc = ws[i];

    if (wc <= 0xffff) {
      ub.insert(wc);
    }
    else if (wc <= 0x10ffff) {
      // Surrogate pair
      wc -= 0x10000;
      ub.insert((wc >> 10)    + 0xd800);
      ub.insert((wc &  0x3ff) + 0xdc00);
    }
    else
      OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_CannotMapChar,
		    (CORBA::CompletionStatus)stream.completion());
  }
  tcs->marshalWString(stream, bound, len, ub.extract());
}


_CORBA_WChar
NCS_W_UCS_4::unmarshalWChar(cdrStream& stream,
			    omniCodeSet::TCS_W* tcs)
{
  OMNIORB_CHECK_TCS_W_FOR_UNMARSHAL(tcs, stream);

  _CORBA_WChar wc;
  if (tcs->fastUnmarshalWChar(stream, this, wc)) return wc;

  return tcs->unmarshalWChar(stream);
}

_CORBA_ULong
NCS_W_UCS_4::unmarshalWString(cdrStream& stream,
			      omniCodeSet::TCS_W* tcs,
			      _CORBA_ULong bound,
			      _CORBA_WChar*& ws)
{
  OMNIORB_CHECK_TCS_W_FOR_UNMARSHAL(tcs, stream);

  _CORBA_ULong len;
  if (tcs->fastUnmarshalWString(stream, this, bound, len, ws)) return len;

  omniCodeSet::UniChar* us;
  len = tcs->unmarshalWString(stream, bound, us);
  OMNIORB_ASSERT(us);

  omniCodeSetUtil::HolderU uh(us);
  omniCodeSetUtil::BufferW wb(len);

  omniCodeSet::UniChar uc;
  _CORBA_WChar         wc;

  for (_CORBA_ULong i=0; i<=len; i++) {
    uc = us[i];

    if (uc < 0xd800) {
      wb.insert(uc);
    }
    else if (uc < 0xdc00) {
      // Surrogate pair
      wc = (uc - 0xd800) << 10;
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
      wc = wc + uc - 0xdc00 + 0x10000;
      wb.insert(wc);
    }
    else if (uc < 0xe000) {
      // Second half of surrogate pair not allowed on its own
      OMNIORB_THROW(DATA_CONVERSION, 
		    DATA_CONVERSION_BadInput,
		    (CORBA::CompletionStatus)stream.completion());
    }
    else {
      wb.insert(uc);
    }
  }
  OMNIORB_ASSERT(uc == 0); // Last char must be zero

  ws = wb.extract();
  return wb.length() - 1;
}


//
// Transmission code set
//

void
TCS_W_UCS_4::marshalWChar(cdrStream& stream,
			  omniCodeSet::UniChar uc)
{
  // In GIOP 1.2, wchar is encoded as an octet containing a length,
  // followed by that number of octets representing the wchar. The
  // CORBA 2.3 spec is silent on endianness issues, and whether there
  // should be any padding. The 2.4 spec says that if TCS-W is UTF-16,
  // the wchar is marshalled big-endian, unless there is a Unicode
  // byte order mark telling us otherwise. That doesn't help us here,
  // since we're not transmitting UTF-16. We assume here that there is
  // no padding, and we use the stream's endianness.

  stream.declareArrayLength(omni::ALIGN_1, 5);

  stream.marshalOctet(4);

  _CORBA_ULong  tc = uc;
  _CORBA_Octet* p  = (_CORBA_Octet*)&tc;
  _CORBA_Octet  o;

  if (0xd800 <= uc && uc <= 0xe000 ) {
    // Part of a surrogate pair -- can't be sent
    OMNIORB_THROW(DATA_CONVERSION, 
		  DATA_CONVERSION_BadInput,
		  (CORBA::CompletionStatus)stream.completion());
  }

  if (stream.marshal_byte_swap()) {
    o = p[3]; stream.marshalOctet(o);
    o = p[2]; stream.marshalOctet(o);
    o = p[1]; stream.marshalOctet(o);
    o = p[0]; stream.marshalOctet(o);
  }
  else {
    o = p[0]; stream.marshalOctet(o);
    o = p[1]; stream.marshalOctet(o);
    o = p[2]; stream.marshalOctet(o);
    o = p[3]; stream.marshalOctet(o);
  }
}

void
TCS_W_UCS_4::marshalWString(cdrStream& stream,
			    _CORBA_ULong bound,
			    _CORBA_ULong len,
			    const omniCodeSet::UniChar* us)
{
  // Just to be different, wstring is marshalled without a terminating
  // null. Length is in octets.
  _CORBA_ULong mlen = len * 4;

  stream.declareArrayLength(omni::ALIGN_4, mlen + 4);
  mlen >>= stream;

  _CORBA_ULong         tc;
  omniCodeSet::UniChar uc;
  
  for (_CORBA_ULong i=0; i < len; i++) {
    uc = us[i];

    if (uc < 0xd800) {
      tc = uc;
    }
    else if (uc < 0xdc00) {
      // Surrogate pair
      tc = (uc - 0xd800) << 10;
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
      tc = tc + uc - 0xdc00 + 0x10000;
    }
    else if (uc < 0xe000) {
      // Second half of surrogate pair not allowed on its own
      OMNIORB_THROW(DATA_CONVERSION, 
		    DATA_CONVERSION_BadInput,
		    (CORBA::CompletionStatus)stream.completion());
      tc = 0; // To shut paranoid compilers up
    }
    else {
      tc = uc;
    }
    tc >>= stream;
  }
}

omniCodeSet::UniChar
TCS_W_UCS_4::unmarshalWChar(cdrStream& stream)
{
  omniCodeSet::UniChar uc;
  _CORBA_Octet         len = stream.unmarshalOctet();
  _CORBA_Octet         o;

  switch (len) {
  case 0:
    uc = 0; // Evil but it might happen, I suppose
    break;
  case 1:
    o  = stream.unmarshalOctet();
    uc = o;
    break;
  case 2:
    {
      _CORBA_Octet* p = (_CORBA_Octet*)&uc;
      if (stream.unmarshal_byte_swap()) {
	o = stream.unmarshalOctet(); p[1] = o;
	o = stream.unmarshalOctet(); p[0] = o;
      }
      else {
	o = stream.unmarshalOctet(); p[0] = o;
	o = stream.unmarshalOctet(); p[1] = o;
      }
    }
    break;
  case 4:
    {
      _CORBA_ULong tc;
      _CORBA_Octet* p = (_CORBA_Octet*)&tc;
      if (stream.unmarshal_byte_swap()) {
	o = stream.unmarshalOctet(); p[3] = o;
	o = stream.unmarshalOctet(); p[2] = o;
	o = stream.unmarshalOctet(); p[1] = o;
	o = stream.unmarshalOctet(); p[0] = o;
      }
      else {
	o = stream.unmarshalOctet(); p[0] = o;
	o = stream.unmarshalOctet(); p[1] = o;
	o = stream.unmarshalOctet(); p[2] = o;
	o = stream.unmarshalOctet(); p[3] = o;
      }
      if (tc > 0xffff)
	OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_CannotMapChar,
		      (CORBA::CompletionStatus)stream.completion());
      uc = tc;
    }
    break;
  default:
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidWCharSize,
		  (CORBA::CompletionStatus)stream.completion());
  }
  return uc;
}

_CORBA_ULong
TCS_W_UCS_4::unmarshalWString(cdrStream& stream,
			      _CORBA_ULong bound,
			      omniCodeSet::UniChar*& us)
{
  _CORBA_ULong mlen; mlen <<= stream;

  if (mlen % 4)
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidWCharSize,
		  (CORBA::CompletionStatus)stream.completion());

  _CORBA_ULong len = mlen / 4; // Note no terminating null in marshalled form

  if (bound && len > bound)
    OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, 
		  (CORBA::CompletionStatus)stream.completion());

  if (!stream.checkInputOverrun(1, mlen))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)stream.completion());

  omniCodeSetUtil::BufferU ub(len + 1);
  _CORBA_ULong             tc;
  _CORBA_ULong             i;

  for (i=0; i < len; i++) {
    tc <<= stream;

    if (tc <= 0xffff) {
      ub.insert(tc);
    }
    else if (tc <= 0x10ffff) {
      // Surrogate pair
      tc -= 0x10000;
      ub.insert((tc >> 10)    + 0xd800);
      ub.insert((tc &  0x3ff) + 0xdc00);
    }
    else
      OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_CannotMapChar,
		    (CORBA::CompletionStatus)stream.completion());
  }
  ub.insert(0); // Null terminator
  us = ub.extract();
  return ub.length() - 1;
}


_CORBA_Boolean
TCS_W_UCS_4::fastMarshalWChar(cdrStream&          stream,
			      omniCodeSet::NCS_W* ncs,
			      _CORBA_WChar        wc)
{
  if (ncs->id() == id()) { // Null transformation

    stream.declareArrayLength(omni::ALIGN_1, 5);
    stream.marshalOctet(4);

    _CORBA_Octet* p = (_CORBA_Octet*)&wc;
    _CORBA_Octet  o;

    if (stream.marshal_byte_swap()) {
      o = p[3]; stream.marshalOctet(o);
      o = p[2]; stream.marshalOctet(o);
      o = p[1]; stream.marshalOctet(o);
      o = p[0]; stream.marshalOctet(o);
    }
    else {
      o = p[0]; stream.marshalOctet(o);
      o = p[1]; stream.marshalOctet(o);
      o = p[2]; stream.marshalOctet(o);
      o = p[3]; stream.marshalOctet(o);
    }
    return 1;
  }
  return 0;
}

_CORBA_Boolean
TCS_W_UCS_4::fastMarshalWString(cdrStream&          stream,
				omniCodeSet::NCS_W* ncs,
				_CORBA_ULong        bound,
				_CORBA_ULong        len,
				const _CORBA_WChar* ws)
{
  if (ncs->id() == id()) { // Null transformation

    if (bound && len > bound)
      OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, 
		    (CORBA::CompletionStatus)stream.completion());

    _CORBA_ULong mlen = len * 4; mlen >>= stream;

    if (stream.marshal_byte_swap()) {
      stream.declareArrayLength(omni::ALIGN_4, mlen);
      _CORBA_ULong tc;
      for (_CORBA_ULong i=0; i<len; i++) {
	tc = ws[i]; tc >>= stream;
      }
    }
    else {
      stream.put_octet_array((const _CORBA_Char*)ws, mlen, omni::ALIGN_4);
    }
    return 1;
  }
  return 0;
}


_CORBA_Boolean
TCS_W_UCS_4::fastUnmarshalWChar(cdrStream&          stream,
				omniCodeSet::NCS_W* ncs,
				_CORBA_WChar&       wc)
{
  if (ncs->id() == id()) { // Null transformation
    _CORBA_Octet  len = stream.unmarshalOctet();
    _CORBA_Octet  o;

    switch (len) {
    case 0:
      wc = 0; // Evil but it might happen, I suppose
      break;
    case 1:
      o = stream.unmarshalOctet();
      wc = o;
      break;
    case 2:
      {
	_CORBA_UShort tc;
	_CORBA_Octet* p = (_CORBA_Octet*)&tc;
	if (stream.unmarshal_byte_swap()) {
	  o = stream.unmarshalOctet(); p[1] = o;
	  o = stream.unmarshalOctet(); p[0] = o;
	}
	else {
	  o = stream.unmarshalOctet(); p[0] = o;
	  o = stream.unmarshalOctet(); p[1] = o;
	}
	wc = tc;
      }
      break;
    case 4:
      {
	_CORBA_Octet* p = (_CORBA_Octet*)&wc;
	if (stream.unmarshal_byte_swap()) {
	  o = stream.unmarshalOctet(); p[3] = o;
	  o = stream.unmarshalOctet(); p[2] = o;
	  o = stream.unmarshalOctet(); p[1] = o;
	  o = stream.unmarshalOctet(); p[0] = o;
	}
	else {
	  o = stream.unmarshalOctet(); p[0] = o;
	  o = stream.unmarshalOctet(); p[1] = o;
	  o = stream.unmarshalOctet(); p[2] = o;
	  o = stream.unmarshalOctet(); p[3] = o;
	}
      }
      break;
    default:
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidWCharSize,
		    (CORBA::CompletionStatus)stream.completion());
    }
    return 1;
  }
  return 0;
}

_CORBA_Boolean
TCS_W_UCS_4::fastUnmarshalWString(cdrStream&          stream,
				  omniCodeSet::NCS_W* ncs,
				  _CORBA_ULong        bound,
				  _CORBA_ULong&       len,
				  _CORBA_WChar*&      ws)
{
  if (ncs->id() == id()) { // Null transformation

    _CORBA_ULong mlen; mlen <<= stream;

    if (mlen % 4)
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidWCharSize,
		    (CORBA::CompletionStatus)stream.completion());

    len = mlen / 4; // Note no terminating null in marshalled form

    if (bound && len > bound)
      OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, 
		    (CORBA::CompletionStatus)stream.completion());

    if (!stream.checkInputOverrun(1, mlen))
      OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		    (CORBA::CompletionStatus)stream.completion());

    ws = omniCodeSetUtil::allocW(len + 1);
    omniCodeSetUtil::HolderW wh(ws);

    stream.unmarshalArrayULong((_CORBA_ULong*)ws, len);

    ws[len] = 0; // Null terminator

    wh.drop();
    return 1;
  }
  return 0;
}


//
// Initialiser
//

static NCS_W_UCS_4 _NCS_W_UCS_4;
static TCS_W_UCS_4 _TCS_W_UCS_4;

class CS_UCS_4_init {
public:
  CS_UCS_4_init() {
    omniCodeSet::registerNCS_W(&_NCS_W_UCS_4);
    omniCodeSet::registerTCS_W(&_TCS_W_UCS_4);
  }
};

static CS_UCS_4_init _CS_UCS_4_init;


#endif // (SIZEOF_WCHAR == 4)

OMNI_NAMESPACE_END(omni)

OMNI_EXPORT_LINK_FORCE_SYMBOL(CS_UCS_4);
