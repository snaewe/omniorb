// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopStreamImpl.cc          Created on: 14/02/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories, Cambridge
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
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.1.2.1  2001/02/23 16:46:56  sll
  Added new files.

*/

#include <omniORB4/CORBA.h>
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <initialiser.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
static giopStreamImpl* implHead = 0;
static giopStreamImpl* implMax = 0;
static GIOP::Version   implMaxVersion = { 255,255 };

////////////////////////////////////////////////////////////////////////
giopStreamImpl::giopStreamImpl(const GIOP::Version& v) : pd_next(0) {
  pd_version.major = v.major;
  pd_version.minor = v.minor;

  // Shared by the client and server side
  // Process message header
  outputMessageBegin             = 0;
  outputMessageEnd               = 0;
  inputMessageBegin              = 0;
  inputMessageEnd                = 0;
  sendMsgErrorMessage            = 0;

  // Client side
  // Process message header
  marshalRequestHeader           = 0;
  marshalLocateRequest           = 0;
  unmarshalReplyHeader           = 0;
  unmarshalLocateReply           = 0;

  // Server side
  // Process message header
  unmarshalWildCardRequestHeader = 0;
  unmarshalRequestHeader         = 0;
  unmarshalLocateRequest         = 0;
  marshalReplyHeader             = 0;
  sendSystemException            = 0;
  sendUserException              = 0;
  sendLocationForwardReply       = 0;
  sendLocateReply                = 0;


  // Shared by the client and the server side
  // Process message body
  inputRemaining                 = 0;
  getInputData                   = 0;
  max_input_buffer_size          = 0;
  skipInputData                  = 0;
  copyInputData                  = 0;
  outputRemaining                = 0;
  getReserveSpace                = 0;
  copyOutputData                 = 0;
  max_reserve_buffer_size        = 0;
  currentInputPtr                = 0;
  currentOutputPtr               = 0;

}

////////////////////////////////////////////////////////////////////////
giopStreamImpl::~giopStreamImpl() {
}


////////////////////////////////////////////////////////////////////////
void
giopStreamImpl::registerImpl(giopStreamImpl* impl) {

  // Insert implementation to the last of the queue
  giopStreamImpl** pp = &implHead;
  while (*pp) pp = &((*pp)->pd_next);
  impl->pd_next = 0;
  *pp = impl;

  if (implMax) {
    CORBA::UShort ver1, ver2;
    ver1 = ((CORBA::UShort)impl->pd_version.major << 8) + 
            impl->pd_version.minor;
    ver2 = ((CORBA::UShort)implMax->pd_version.major << 8) +
           implMax->pd_version.minor;
    if (ver1 <= ver2) return;
  }
  implMax = impl;
}


////////////////////////////////////////////////////////////////////////
giopStreamImpl*
giopStreamImpl::matchVersion(const GIOP::Version& v) {

  giopStreamImpl* p = implHead;

  while (p) {
    if (p->pd_version.major == v.major && p->pd_version.minor == v.minor)
      break;
    p = p->pd_next;
  }
  return p;
}

////////////////////////////////////////////////////////////////////////
giopStreamImpl*
giopStreamImpl::maxVersion() {
  return implMax;
}

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

extern omniInitialiser& omni_giopImpl10_initialiser_;
extern omniInitialiser& omni_giopImpl11_initialiser_;
extern omniInitialiser& omni_giopImpl12_initialiser_;

class omni_giopStreamImpl_initialiser : public omniInitialiser {
public:

  void attach() {
    omni_giopImpl10_initialiser_.attach();
    if (implMaxVersion.minor >= 1)
      omni_giopImpl11_initialiser_.attach();
    if (implMaxVersion.minor >= 2)
      omni_giopImpl12_initialiser_.attach();
    {
      if (omniORB::trace(25)) {
	GIOP::Version v = giopStreamImpl::maxVersion()->version();
	omniORB::logger log;
	log << " Maximum supported GIOP version is " << (int)v.major 
	    << "." << (int)v.minor << "\n";
      }
    }
  }

  void detach() { 
    omni_giopImpl10_initialiser_.detach();
    if (implMaxVersion.minor >= 1)
      omni_giopImpl11_initialiser_.detach();
    if (implMaxVersion.minor >= 2)
      omni_giopImpl12_initialiser_.detach();
  }

};

static omni_giopStreamImpl_initialiser initialiser;

omniInitialiser& omni_giopStreamImpl_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)


////////////////////////////////////////////////////////////////////////
void
omniORB::maxGIOPVersion(CORBA::Char& major, CORBA::Char& minor)
{
  _OMNI_NS(giopStreamImpl)* maximpl = _OMNI_NS(giopStreamImpl)::maxVersion();

  if (maximpl) {
    GIOP::Version v = maximpl->version();
    major = v.major;
    minor = v.minor;
  }
  else {
    _OMNI_NS(implMaxVersion).major = (major > 0) ? major : 1;
    _OMNI_NS(implMaxVersion).minor = minor;
  }
}

