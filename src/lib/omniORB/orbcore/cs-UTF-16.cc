// -*- Mode: C++; -*-
//                            Package   : omniORB
// cs-UTF-16.cc               Created on: 25/10/2000
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
//    Unicode / ISO 10646 UTF-16

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


class NCS_W_UTF_16 : public omniCodeSet::NCS_W {
public:

  virtual void marshalWChar(cdrStream& stream, omniCodeSet::TCS_W* tcs,
			    _CORBA_WChar c);

  virtual void marshalWString(cdrStream& stream, omniCodeSet::TCS_W* tcs,
			      _CORBA_ULong bound, const _CORBA_WChar* s);

  virtual _CORBA_WChar unmarshalWChar(cdrStream& stream,
				      omniCodeSet::TCS_W* tcs);

  virtual _CORBA_ULong unmarshalWString(cdrStream& stream,
					omniCodeSet::TCS_W* tcs,
					_CORBA_ULong bound,
					_CORBA_WChar*& s);

  NCS_W_UTF_16()
    : omniCodeSet::NCS_W(omniCodeSet::ID_UTF_16, "UTF-16",
			 omniCodeSet::CS_Other)
  { }

  virtual ~NCS_W_UTF_16() {}
};


class TCS_W_UTF_16 : public omniCodeSet::TCS_W {
public:

  virtual void marshalWChar  (cdrStream& stream, omniCodeSet::UniChar uc);
  virtual void marshalWString(cdrStream& stream,
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
					      const _CORBA_WChar* s);

  virtual _CORBA_Boolean fastUnmarshalWChar  (cdrStream&          stream,
					      omniCodeSet::NCS_W* ncs,
					      _CORBA_WChar&       c);

  virtual _CORBA_Boolean fastUnmarshalWString(cdrStream&          stream,
					      omniCodeSet::NCS_W* ncs,
					      _CORBA_ULong        bound,
					      _CORBA_ULong&       length,
					      _CORBA_WChar*&      s);

  TCS_W_UTF_16()
    : omniCodeSet::TCS_W(omniCodeSet::ID_UTF_16, "UTF-16",
			 omniCodeSet::CS_Other, omniCodeSetUtil::GIOP12)
  { }

  virtual ~TCS_W_UTF_16() {}
};


//
// Native code set
//

void
NCS_W_UTF_16::marshalWChar(cdrStream& stream,
			   omniCodeSet::TCS_W* tcs,
			   _CORBA_WChar wc)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  if (tcs->fastMarshalWChar(stream, this, wc)) return;

#if (SIZEOF_WCHAR == 4)
  if (wc > 0xffff)
    OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);
#endif
  tcs->marshalWChar(stream, wc);
}

void
NCS_W_UTF_16::marshalWString(cdrStream& stream,
			     omniCodeSet::TCS_W* tcs,
			     _CORBA_ULong bound,
			     const _CORBA_WChar* ws)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  if (tcs->fastMarshalWString(stream, this, bound, ws)) return;

  _CORBA_ULong len = _CORBA_WString_helper::len(ws) + 1;

  if (bound && len >= bound)
    OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

#if (SIZEOF_WCHAR == 2)
  tcs->marshalWString(stream, len, ws);
#else
  omniCodeSet::UniChar*    us = omniCodeSetUtil::allocU(len);
  omniCodeSetUtil::HolderU uh(us);
  _CORBA_WChar             wc;

  for (_CORBA_ULong i=0; i<len; i++) {
    wc = ws[i];
    if (wc > 0xffff) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);
    us[i] = wc;
  }
  tcs->marshalWString(stream, len, us);
#endif
}

_CORBA_WChar
NCS_W_UTF_16::unmarshalWChar(cdrStream& stream,
			     omniCodeSet::TCS_W* tcs)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  _CORBA_WChar wc;
  if (tcs->fastUnmarshalWChar(stream, this, wc)) return wc;

  return tcs->unmarshalWChar(stream);
}

_CORBA_ULong
NCS_W_UTF_16::unmarshalWString(cdrStream& stream,
			       omniCodeSet::TCS_W* tcs,
			       _CORBA_ULong bound,
			       _CORBA_WChar*& ws)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  _CORBA_ULong len;
  if (tcs->fastUnmarshalWString(stream, this, bound, len, ws)) return len;

  omniCodeSet::UniChar* us;
  len = tcs->unmarshalWString(stream, bound, us);
  OMNIORB_ASSERT(us);
  OMNIORB_ASSERT(len > 0);

#if (SIZEOF_WCHAR == 2)
  ws = us;
  return len;
#else
  omniCodeSetUtil::HolderU uh(us);

  ws = omniCodeSetUtil::allocW(len);
  omniCodeSetUtil::HolderW wh(ws);

  for (_CORBA_ULong i=0; i<len; i++)
    ws[i] = us[i];

  wh.drop();
  return len;
#endif
}


void
TCS_W_UTF_16::marshalWChar(cdrStream& stream, omniCodeSet::UniChar uc)
{
  // The 2.4 spec says that if TCS-W is UTF-16, the wchar is
  // marshalled big-endian, unless there is a Unicode byte order mark
  // telling us otherwise. Here, we never send a byte order mark, so
  // the value is always big-endian.

  _CORBA_Octet o;

  o = 2;                  stream.marshalOctet(o);
  o = (uc & 0xff00) >> 8; stream.marshalOctet(o);
  o = (uc & 0x00ff);      stream.marshalOctet(o);
}

void
TCS_W_UTF_16::marshalWString(cdrStream& stream,
			     _CORBA_ULong len,
			     const omniCodeSet::UniChar* us)
{
  // The CORBA 2.4 spec says that for UTF-16, if there is no BOM,
  // values are sent big-endian, regardless of the endianness of the
  // rest of the stream. This will cause errors with 2.3 ORBs which
  // chose to use stream endian, but there's nothing much we can do.
  // Here, we always send a BOM, so we can transmit using our native
  // endian.

  // Just to be different, wstring is marshalled without a terminating
  // null. Length is in octets.
  _CORBA_ULong mlen = len * 2;  // len - 1 to strip null, + 1 for BOM
  mlen >>= stream;

  // Send a suitable BOM so that we can marshal with native endian,
  // even if the rest of the stream is byte-swapped.
  if (stream.marshal_byte_swap()) {
    _CORBA_UShort tc = 0xfffe; tc >>= stream;
  }
  else {
    _CORBA_UShort tc = 0xfeff; tc >>= stream;
  }
  stream.put_octet_array((const _CORBA_Octet*)us, mlen-2, omni::ALIGN_2);
}


omniCodeSet::UniChar
TCS_W_UTF_16::unmarshalWChar(cdrStream& stream)
{
  // CORBA 2.4 implies there are two valid values for the char
  // sequence length, 2 and 4. If 2, assume the bytes are big-endian;
  // if 4, assume the first 2 are a BOM telling us the endianness of
  // the next two...

  _CORBA_Octet o;
  o = stream.unmarshalOctet();

  omniCodeSet::UniChar uc;

  if (o == 2) {
    // Big endian
    o = stream.unmarshalOctet(); uc  = o << 8;
    o = stream.unmarshalOctet(); uc |= o;
    return uc;
  }
  else if (o == 4) {
    // BOM
    o = stream.unmarshalOctet(); uc  = o << 8;
    o = stream.unmarshalOctet(); uc |= o;

    if (uc == 0xfeff) {
      // Big endian
      o = stream.unmarshalOctet(); uc  = o << 8;
      o = stream.unmarshalOctet(); uc |= o;
      return uc;
    }
    else if (uc == 0xfffe) {
      // Little endian
      o = stream.unmarshalOctet(); uc  = o;
      o = stream.unmarshalOctet(); uc |= o << 8;
      return uc;
    }
    else {
      OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_NO);
    }
  }
  OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_NO);
  return 0; // For broken compilers
}

_CORBA_ULong
TCS_W_UTF_16::unmarshalWString(cdrStream& stream,
			       _CORBA_ULong bound,
			       omniCodeSet::UniChar*& us)
{
  // This complies to CORBA 2.4, and strips off a BOM if one is
  // found. Of course, if a 2.3 ORB sends a string whose first
  // character is a BOM, we'll strip it erroneously. Oh well.

  _CORBA_ULong mlen; mlen <<= stream;

  if (mlen % 2)
    OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);

  _CORBA_ULong len = mlen / 2; // Note no terminating null in marshalled form

  // If there is a BOM, this will allocate one character too many, but
  // never mind.
  us = omniCodeSetUtil::allocU(len + 1);
  omniCodeSetUtil::HolderU uh(us);

  _CORBA_UShort uc; // Not UniChar, since if UniChar is wchar_t, there
                    // is no stream extraction operator for it
  uc <<= stream;

  if (uc == 0xfeff) {
    // BOM is in stream endian...
    len--;

    if (bound && len > bound)
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

    stream.unmarshalArrayUShort((_CORBA_UShort*)us, len);
  }
  else if (uc == 0xfffe) {
    // BOM is not in stream endian
    len--;

    if (bound && len > bound)
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

    stream.get_octet_array((_CORBA_Octet*)us, len*2, omni::ALIGN_2);

    if (!stream.unmarshal_byte_swap()) {
      for (_CORBA_ULong i=0; i < len; i++) {
	uc    = us[i];
	us[i] = ((uc & 0xff00) >> 8) | ((uc & 0x00ff) << 8);
      }
    }
  }
  else {
    // No BOM at all, so big endian
    if (omniORB::trace(15)) {
      omniORB::logger l;
      l << "Received UTF-16 string with no byte order mark.\n";
    }

    if (bound && len > bound)
      OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

    // If we swapped the first wchar getting it out of the stream,
    // swap it back. Might be a waste of time, but it makes things
    // simpler.
    if (stream.unmarshal_byte_swap())
      us[0] = ((uc & 0xff00) >> 8) | ((uc & 0x00ff) << 8);
    else
      us[0] = uc;

    // Read rest of the string
    stream.get_octet_array((_CORBA_Octet*)(us+1), (len-1)*2, omni::ALIGN_2);

    if (omni::myByteOrder) {
      // We are little endian
      for (_CORBA_ULong i=0; i < len; i++) {
	uc    = us[i];
	us[i] = ((uc & 0xff00) >> 8) | ((uc & 0x00ff) << 8);
      }
    }
  }
  us[len] = 0;
  uh.drop();
  return len+1;
}


// Fast functions are currently empty. It may be worth putting in
// special cases for non-Unicode 16 bit code sets and UCS-4. However,
// it's probably quicker to fill a memory buffer and marshal it in one
// lump than to marshal lots of individual 16-bit values.

_CORBA_Boolean
TCS_W_UTF_16::fastMarshalWChar(cdrStream&          stream,
			       omniCodeSet::NCS_W* ncs,
			       _CORBA_WChar        c)
{
  return 0;
}

_CORBA_Boolean
TCS_W_UTF_16::fastMarshalWString(cdrStream&          stream,
				 omniCodeSet::NCS_W* ncs,
				 _CORBA_ULong        bound,
				 const _CORBA_WChar* s)
{
  return 0;
}

_CORBA_Boolean
TCS_W_UTF_16::fastUnmarshalWChar(cdrStream&          stream,
				 omniCodeSet::NCS_W* ncs,
				 _CORBA_WChar&       c)
{
  return 0;
}

_CORBA_Boolean
TCS_W_UTF_16::fastUnmarshalWString(cdrStream&          stream,
				   omniCodeSet::NCS_W* ncs,
				   _CORBA_ULong        bound,
				   _CORBA_ULong&       length,
				   _CORBA_WChar*&      s)
{
  return 0;
}


//
// Initialiser
//

static NCS_W_UTF_16 _NCS_W_UTF_16;
static TCS_W_UTF_16 _TCS_W_UTF_16;

class CS_UTF_16_init {
public:
  CS_UTF_16_init() {
    omniCodeSet::registerNCS_W(&_NCS_W_UTF_16);
    omniCodeSet::registerTCS_W(&_TCS_W_UTF_16);
  }
};

static CS_UTF_16_init _CS_UTF_16_init;
