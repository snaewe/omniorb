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
#include <codeSetUtil.h>

#if (SIZEOF_WCHAR == 4)

class NCS_W_UCS_4 : public omniCodeSet::NCS_W {
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

  NCS_W_UCS_4()
    : omniCodeSet::NCS_W(omniCodeSet::ID_UCS_4, "UCS-4", omniCodeSet::CS_Other)
  { }

  virtual ~NCS_W_UCS_4() {}
};


void
NCS_W_UCS_4::marshalWChar(cdrStream& stream,
			  omniCodeSet::TCS_W* tcs,
			  _CORBA_WChar wc)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  if (tcs->fastMarshalWChar(stream, this, wc)) return;

  if (wc > 0xffff)
    OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_MAYBE);

  tcs->marshalWChar(stream, wc);
}


void
NCS_W_UCS_4::marshalWString(cdrStream& stream,
			    omniCodeSet::TCS_W* tcs,
			    _CORBA_ULong bound,
			    const _CORBA_WChar* ws)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  if (tcs->fastMarshalWString(stream, this, bound, ws)) return;

  _CORBA_ULong len = _CORBA_WString_helper::len(ws) + 1;

  if (bound && len >= bound)
    OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_MAYBE);

  omniCodeSetUtil::BufferU ub(len);
  _CORBA_WChar             wc;

  for (_CORBA_ULong i=0; i<len; i++) {
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
      OMNIORB_THROW(DATA_CONVERSION, 0, CORBA::COMPLETED_NO);
  }
  tcs->marshalWString(stream, len, ub.extract());
}


_CORBA_WChar
NCS_W_UCS_4::unmarshalWChar(cdrStream& stream,
			    omniCodeSet::TCS_W* tcs)
{
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
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
  if (!tcs) OMNIORB_THROW(BAD_PARAM, 0, CORBA::COMPLETED_NO);
  _CORBA_ULong len;
  if (tcs->fastUnmarshalWString(stream, this, bound, len, ws)) return len;

  omniCodeSet::UniChar* us;
  len = tcs->unmarshalWString(stream, bound, us);
  OMNIORB_ASSERT(us);
  OMNIORB_ASSERT(len > 0);

  omniCodeSetUtil::HolderU uh(us);
  omniCodeSetUtil::BufferW wb(len);

  omniCodeSet::UniChar uc;
  _CORBA_WChar         wc;

  for (_CORBA_ULong i=0; i<len; i++) {
    uc = us[i];

    if (uc < 0xd800) {
      wb.insert(uc);
    }
    else if (uc < 0xdc00) {
      // Surrogate pair
      wc = (uc - 0xd800) << 10;
      if (++i == len) {
	// No second half to surrogate pair
	OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);
      }
      uc = us[i];
      if (uc < 0xdc00 || uc > 0xdfff) {
	// Value is not a valid second half to a surrogate pair
	OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);
      }
      wc = wc + uc - 0xdc00 + 0x10000;
      wb.insert(wc);
    }
    else if (uc < 0xe000) {
      // Second half of surrogate pair not allowed on its own
      OMNIORB_THROW(MARSHAL, 0, CORBA::COMPLETED_MAYBE);
    }
    else {
      wb.insert(uc);
    }
  }
  OMNIORB_ASSERT(uc == 0); // Last char must be zero

  ws = wb.extract();
  return wb.length();
}


//
// Initialiser
//

static NCS_W_UCS_4 _NCS_W_UCS_4;

class CS_UCS_4_init {
public:
  CS_UCS_4_init() {
    omniCodeSet::registerNCS_W(&_NCS_W_UCS_4);
  }
};

static CS_UCS_4_init _CS_UCS_4_init;



#endif // (SIZEOF_WCHAR == 4)
