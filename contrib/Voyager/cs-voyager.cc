// -*- Mode: C++; -*-
//
// cs-voyager.cc
//
// Copyright (C) 2002 Duncan Grisby.
//
// You may redistribute and/or modify this software under the terms of
// the GNU Lesser General Public License.
//
// Description:
//    omniORB transmission code set for communicating with Recursion
//    Software Voyager.
//
// This work is based on information provided on an as-is basis by
// Recursion Software, Inc. (http://www.recursionsw.com) and is not
// endorsed or supported by Recursion Software.

#include <omniORB4/CORBA.h>
#include <omniORB4/linkHacks.h>
#include <omniORB4/omniInterceptors.h>
#include <codeSetUtil.h>
#include <initialiser.h>


// There are two parts to the Voyager wstring support. First is a
// transmission code set that knows how to marshal Unicode data with
// the scheme used by Voyager. Second is two interceptors, one for the
// client side and one for the server side, that choose the Voyager
// code set if GIOP 1.0 is being used.


OMNI_NAMESPACE_BEGIN(omni)

//
// Transmission code set
//

// We have to make up a code set id for Voyager's marshalling scheme.
// It must not clash with any standard ones that are in use.
static const CONV_FRAME::CodeSetId ID_Voyager = 0xff000001;
static const GIOP::Version GIOP10 = { 1,0 };

class TCS_W_Voyager : public omniCodeSet::TCS_W {
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

  TCS_W_Voyager()
    : omniCodeSet::TCS_W(ID_Voyager, "Voyager", omniCodeSet::CS_Other, GIOP10)
  { }

  virtual ~TCS_W_Voyager() {}
};

void
TCS_W_Voyager::marshalWChar(cdrStream& stream, omniCodeSet::UniChar uc)
{
  // Voyager expects wchars to always be big endian
  stream.alignOutput(omni::ALIGN_2);
  stream.marshalOctet((uc & 0xff00) >> 8);
  stream.marshalOctet(uc & 0x00ff);
}

void
TCS_W_Voyager::marshalWString(cdrStream& stream,
			      _CORBA_ULong len,
			      const omniCodeSet::UniChar* us)
{
  // Voyager uses an encoding of sequence<unsigned short>, with a
  // terminating null, EXCEPT that the wchars are always marshalled
  // big endian!  How very odd.

  CORBA::ULong mlen = len + 1;
  mlen >>= stream;

  if (omni::myByteOrder) {
    // We are little endian, so we must byte swap. Note that in the
    // very rare case that marshal_byte_swap is true, we will swap
    // twice, but it's so rare it's not worth making a special case.

    CORBA::UShort uc, sc;
    for (CORBA::ULong i=0; i < mlen; i++) {
      uc = us[i];
      sc = ((uc & 0xff00) >> 8) | ((uc & 0x00ff) << 8);
      sc >>= stream;
    }
  }
  else if (stream.marshal_byte_swap()) {
    CORBA::UShort uc;
    for (CORBA::ULong i=0; i < mlen; i++) {
      uc = us[i];
      uc >>= stream;
    }
  }
  else {
    // We're big endian and so is the stream, so we can do a direct send.
    stream.put_octet_array((const CORBA::Octet*)us, mlen*2, omni::ALIGN_2);
  }
}


omniCodeSet::UniChar
TCS_W_Voyager::unmarshalWChar(cdrStream& stream)
{
  CORBA::UShort us;
  us <<= stream;
  return us;
}

_CORBA_ULong
TCS_W_Voyager::unmarshalWString(cdrStream& stream,
				_CORBA_ULong bound,
				omniCodeSet::UniChar*& us)
{
  // Voyager apparently distinguishes between a null string, which it
  // marshals as having length zero, and an empty string, which it
  // marshals with length 1, to account for the terminating null.
  // Since we don't have null strings, we accept either form to be a
  // zero length string.

  _CORBA_ULong mlen; mlen <<= stream;

  if (mlen == 0) {
    us = omniCodeSetUtil::allocU(1);
    us[0] = 0;
    return 0;
  }

  us = omniCodeSetUtil::allocU(mlen);
  omniCodeSetUtil::HolderU uh(us);

  stream.unmarshalArrayUShort((_CORBA_UShort*)us, mlen);
  uh.drop();
  return mlen - 1;
}

// No need for the fast versions, since the TCS is Unicode based,
// which means the normal versions are just as fast.

_CORBA_Boolean
TCS_W_Voyager::fastMarshalWChar(cdrStream&          stream,
				omniCodeSet::NCS_W* ncs,
				_CORBA_WChar        c)
{
  return 0;
}

_CORBA_Boolean
TCS_W_Voyager::fastMarshalWString(cdrStream&          stream,
				  omniCodeSet::NCS_W* ncs,
				  _CORBA_ULong        bound,
				  _CORBA_ULong        len,
				  const _CORBA_WChar* s)
{
  return 0;
}

_CORBA_Boolean
TCS_W_Voyager::fastUnmarshalWChar(cdrStream&          stream,
				  omniCodeSet::NCS_W* ncs,
				  _CORBA_WChar&       c)
{
  return 0;
}

_CORBA_Boolean
TCS_W_Voyager::fastUnmarshalWString(cdrStream&          stream,
				    omniCodeSet::NCS_W* ncs,
				    _CORBA_ULong        bound,
				    _CORBA_ULong&       length,
				    _CORBA_WChar*&      s)
{
  return 0;
}

//
// Interceptors
//

static
CORBA::Boolean
clientSetVoyagerCodeSet(omniInterceptors::clientSendRequest_T::info_T& info)
{
  giopStrand& d = (giopStrand&)(info.giopstream);
  GIOP::Version ver = info.giopstream.version();

  if (ver.major == 1 && ver.minor == 0) {
    // Set code sets for Voyager
    if (!d.tcs_selected) {
      d.tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1, ver);
      d.tcs_w = omniCodeSet::getTCS_W(ID_Voyager, ver);
      d.tcs_selected = 1;
    }
    info.giopstream.TCS_C(d.tcs_c);
    info.giopstream.TCS_W(d.tcs_w);
  }
  return 1;
}

static
CORBA::Boolean
serverSetVoyagerCodeSet(omniInterceptors::serverReceiveRequest_T::info_T& info)
{
  giopStrand& d = (giopStrand&)(info.giop_s);
  GIOP::Version ver = info.giop_s.version();
  
  if (ver.major == 1 && ver.minor == 0) {
    // Set code sets for Voyager
    if (!d.tcs_selected) {
      d.tcs_c = omniCodeSet::getTCS_C(omniCodeSet::ID_8859_1, ver);
      d.tcs_w = omniCodeSet::getTCS_W(ID_Voyager, ver);
      OMNIORB_ASSERT(d.tcs_w);
      d.tcs_selected = 1;
    }
    info.giop_s.TCS_C(d.tcs_c);
    info.giop_s.TCS_W(d.tcs_w);
  }
  return 1;
}


//
// Module initialiser.
//

static TCS_W_Voyager _TCS_W_Voyager;

// Code set is registered at static initialisation time; interceptors
// are registered when ORB_init is called.

class CS_Voyager_init : public omniInitialiser {
public:
  CS_Voyager_init() {
    omniCodeSet::registerTCS_W(&_TCS_W_Voyager);
    omniInitialiser::install(this);
  }

  void attach() {
    omniORB::logs(10, "Initialise Voyager interceptors");
    omniInterceptors* interceptors = omniORB::getInterceptors();
    interceptors->clientSendRequest.add(clientSetVoyagerCodeSet);
    interceptors->serverReceiveRequest.add(serverSetVoyagerCodeSet);
  }
  void detach() {
  }
};

static CS_Voyager_init _CS_Voyager_init;

OMNI_NAMESPACE_END(omni)

OMNI_EXPORT_LINK_FORCE_SYMBOL(CS_Voyager);
