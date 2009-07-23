// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopStreamImpl.cc          Created on: 14/02/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2008 Apasphere Ltd
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
  Revision 1.1.6.5  2008/02/14 12:37:50  dgrisby
  New immediateAddressSwitch parameter.

  Revision 1.1.6.4  2006/02/22 14:56:36  dgrisby
  New endPointPublishHostname and endPointResolveNames parameters.

  Revision 1.1.6.3  2006/01/10 13:59:37  dgrisby
  New clientConnectTimeOutPeriod configuration parameter.

  Revision 1.1.6.2  2005/01/06 23:10:30  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.6.1  2003/03/23 21:02:14  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.7  2002/10/14 20:07:11  dgrisby
  Per objref / per thread timeouts.

  Revision 1.1.4.6  2002/04/29 11:52:51  dgrisby
  More fixes for FreeBSD, Darwin, Windows.

  Revision 1.1.4.5  2002/03/18 15:13:08  dpg1
  Fix bug with old-style ORBInitRef in config file; look for
  -ORBtraceLevel arg before anything else; update Windows registry
  key. Correct error message.

  Revision 1.1.4.4  2001/09/20 13:26:14  dpg1
  Allow ORB_init() after orb->destroy().

  Revision 1.1.4.3  2001/08/21 11:02:16  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.1.4.2  2001/08/17 17:12:38  sll
  Modularise ORB configuration parameters.

  Revision 1.1.4.1  2001/04/18 18:10:48  sll
  Big checkin with the brand new internal APIs.


*/

#include <omniORB4/CORBA.h>
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <initialiser.h>
#include <orbOptions.h>
#include <orbParameters.h>
#include <stdio.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////

GIOP::Version orbParameters::maxGIOPVersion = { 1, 2 };
//  Set the maximum GIOP version the ORB should support. The ORB tries
//  to match the <major>.<minor> version as specified. This function
//  should only be called before ORB_init(). Calling this function
//  after ORB_init()  does not cause the ORB to change its maximum
//  supported version, in this case the ORB just returns its version
//  number in <major>.<minor>.
//
//  Valid values = 1.0 | 1.1 | 1.2

CORBA::ULong orbParameters::giopMaxMsgSize = 2048 * 1024;
//   This value defines the ORB-wide limit on the size of GIOP message 
//   (excluding the header). If this limit is exceeded, the ORB will
//   refuse to send or receive the message and raise a MARSHAL exception.
//
//   Valid values = (n >= 8192)

orbParameters::timeValue orbParameters::clientCallTimeOutPeriod = {0,0};
//   Call timeout. On the client side, if a remote call takes longer
//   than the timeout value, the ORB will shutdown the connection and
//   raise a COMM_FAILURE.
//
//   Valid values = (n >= 0 in seconds) 
//                   0 --> no timeout. Block till a reply comes back

orbParameters::timeValue orbParameters::clientConnectTimeOutPeriod = {0,0};
//   Connect timeout. When a client has no existing connection to
//   communicate with a server, it must open a new connection before
//   performing the call. If this parameter is non-zero, it sets a
//   timeout specifically for establishing the connection. If the
//   timeout specified here is shorter than the overall timeout for
//   the call (set with clientCallTimeOutPeriod or per-object or
//   per-thread timeouts), the connect timeout is used for
//   establishing the connection, then additional time is permitted
//   for the call to complete. If the connect timeout is longer than
//   the normal call timeout, the deadline for the entire call is
//   extended to match the connect timeout.
//
//   If this parameter is zero, the normal call timeout applies to the
//   total time taken to perform the connect and the subsequent call.
//
//   Valid values = (n >= 0 in milliseconds) 
//                   0 --> same timeout (if any) as other calls

CORBA::Boolean orbParameters::supportPerThreadTimeOut = 0;
//   If true, each thread may have a timeout associated with it. This
//   gives a performance hit due to accessing per-thread data.
//
//   Valid values = 0 or 1

orbParameters::timeValue orbParameters::serverCallTimeOutPeriod = {0,0};
//   Call timeout. On the server side, if the ORB cannot completely 
//   unmarshal a call's arguments in the defined timeout, it shutdown the
//   connection.
//
//   Valid values = (n >= 0 in seconds) 
//                   0 --> no timeout.


CORBA::ULong  orbParameters::maxInterleavedCallsPerConnection = 5;
//  No. of interleaved calls per connection the server is prepared
//  to accept. If this number is exceeded, the connection is closed.
//
//  Valid values = (n >= 1) 

GIOP::AddressingDisposition orbParameters::giopTargetAddressMode = GIOP::KeyAddr;
//  On the client side, if it is to use GIOP 1.2 or above to talk to a 
//  server, use this Target Address Mode.
//
//  Valid values = 0 (GIOP::KeyAddr)
//                 1 (GIOP::ProfileAddr)
//                 2 (GIOP::ReferenceAddr)

CORBA::Boolean orbParameters::strictIIOP = 1;
//   Enable vigorous check on incoming IIOP messages
//
//   In some (sloppy) IIOP implementations, the message size value in
//   the header can be larger than the actual body size, i.e. there is
//   garbage at the end. As the spec does not say the message size
//   must match the body size exactly, this is not a clear violation
//   of the spec.
//
//   If this flag is non-zero, the incoming message is expected to
//   be well behaved. Any messages that have garbage at the end will
//   be rejected.
//   
//   The default value of this flag is true, so invalid messages are
//   rejected. If you set it to zero, the ORB will silently skip the
//   unread part. The problem with this behaviour is that the header
//   message size may actually be garbage, caused by a bug in the
//   sender's code. The receiving thread may forever block on the
//   strand as it tries to read more data from it. In this case the
//   sender won't send any more as it thinks it has marshalled in all
//   the data.
//
//   Valid values = 0 or 1

CORBA::Boolean orbParameters::immediateRopeSwitch = 0;
//   Switch rope to use a new address immediately, rather than
//   retrying with the existing one.
//
//   Valid values = 0 or 1


////////////////////////////////////////////////////////////////////////
static giopStreamImpl* implHead = 0;
static giopStreamImpl* implMax = 0;

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
  sendLocateRequest              = 0;
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
  skipInputData                  = 0;
  copyInputData                  = 0;
  outputRemaining                = 0;
  getReserveSpace                = 0;
  copyOutputData                 = 0;
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
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class maxGIOPVersionHandler : public orbOptions::Handler {
public:

  maxGIOPVersionHandler() : 
    orbOptions::Handler("maxGIOPVersion",
			"maxGIOPVersion = 1.0 | 1.1 | 1.2",
			1,
			"-ORBmaxGIOPVersion < 1.0 | 1.1 | 1.2 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    unsigned int ma, mi;
    if ( sscanf(value, "%u.%u", &ma, &mi) != 2 || ma > 255 || mi > 255) {

      throw orbOptions::BadParam(key(),value,
				 "Expect 1.0 | 1.1 | 1.2");
    }
    orbParameters::maxGIOPVersion.major = ma;
    orbParameters::maxGIOPVersion.minor = mi;
  }

  void dump(orbOptions::sequenceString& result) {
    CORBA::String_var v(CORBA::string_alloc(3));

    sprintf(v,"%1d.%1d",orbParameters::maxGIOPVersion.major, orbParameters::maxGIOPVersion.minor);
    orbOptions::addKVString(key(),v,result);
  }

};

static maxGIOPVersionHandler maxGIOPVersionHandler_;

/////////////////////////////////////////////////////////////////////////////
class giopMaxMsgSizeHandler : public orbOptions::Handler {
public:

  giopMaxMsgSizeHandler() : 
    orbOptions::Handler("giopMaxMsgSize",
			"giopMaxMsgSize = n >= 8192",
			1,
			"-ORBgiopMaxMsgSize < n >= 8192 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 8192) {
      throw orbOptions::BadParam(key(),value,
				 "Invalid value, expect n >= 8192");
    }
    orbParameters::giopMaxMsgSize = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::giopMaxMsgSize,
			   result);
  }

};

static giopMaxMsgSizeHandler giopMaxMsgSizeHandler_;

/////////////////////////////////////////////////////////////////////////////
class clientCallTimeOutPeriodHandler : public orbOptions::Handler {
public:

  clientCallTimeOutPeriodHandler() : 
    orbOptions::Handler("clientCallTimeOutPeriod",
			"clientCallTimeOutPeriod = n >= 0 in msecs",
			1,
			"-ORBclientCallTimeOutPeriod < n >= 0 in msecs >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 "Expect n >= 0 in msecs");
    }
    orbParameters::clientCallTimeOutPeriod.secs = v / 1000;
    orbParameters::clientCallTimeOutPeriod.nanosecs = (v % 1000) * 1000000;
  }

  void dump(orbOptions::sequenceString& result) {
    CORBA::ULong v = orbParameters::clientCallTimeOutPeriod.secs * 1000 +
      orbParameters::clientCallTimeOutPeriod.nanosecs / 1000000;
    orbOptions::addKVULong(key(),v,result);
  }

};

static clientCallTimeOutPeriodHandler clientCallTimeOutPeriodHandler_;

/////////////////////////////////////////////////////////////////////////////
class supportPerThreadTimeOutHandler : public orbOptions::Handler {
public:

  supportPerThreadTimeOutHandler() : 
    orbOptions::Handler("supportPerThreadTimeOut",
			"supportPerThreadTimeOut = 0 or 1",
			1,
			"-ORBsupportPerThreadTimeOut < 0 | 1 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::supportPerThreadTimeOut = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::supportPerThreadTimeOut,
			     result);
  }
};

static supportPerThreadTimeOutHandler supportPerThreadTimeOutHandler_;

/////////////////////////////////////////////////////////////////////////////
class clientConnectTimeOutPeriodHandler : public orbOptions::Handler {
public:

  clientConnectTimeOutPeriodHandler() : 
    orbOptions::Handler("clientConnectTimeOutPeriod",
			"clientConnectTimeOutPeriod = n >= 0 in msecs",
			1,
			"-ORBclientConnectTimeOutPeriod < n >= 0 in msecs >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 "Expect n >= 0 in msecs");
    }
    orbParameters::clientConnectTimeOutPeriod.secs = v / 1000;
    orbParameters::clientConnectTimeOutPeriod.nanosecs = (v % 1000) * 1000000;
  }

  void dump(orbOptions::sequenceString& result) {
    CORBA::ULong v = orbParameters::clientConnectTimeOutPeriod.secs * 1000 +
      orbParameters::clientConnectTimeOutPeriod.nanosecs / 1000000;
    orbOptions::addKVULong(key(),v,result);
  }

};

static clientConnectTimeOutPeriodHandler clientConnectTimeOutPeriodHandler_;



/////////////////////////////////////////////////////////////////////////////
class serverCallTimeOutPeriodHandler : public orbOptions::Handler {
public:

  serverCallTimeOutPeriodHandler() : 
    orbOptions::Handler("serverCallTimeOutPeriod",
			"serverCallTimeOutPeriod = n >= 0 in msecs",
			1,
			"-ORBserverCallTimeOutPeriod < n >= 0 in msecs >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 "Expect n >= 0 in msecs");
    }
    orbParameters::serverCallTimeOutPeriod.secs = v / 1000;
    orbParameters::serverCallTimeOutPeriod.nanosecs = (v % 1000) * 1000000;
  }

  void dump(orbOptions::sequenceString& result) {
    CORBA::ULong v = orbParameters::serverCallTimeOutPeriod.secs * 1000 +
      orbParameters::serverCallTimeOutPeriod.nanosecs / 1000000;
    orbOptions::addKVULong(key(),v,result);
  }

};

static serverCallTimeOutPeriodHandler serverCallTimeOutPeriodHandler_;

/////////////////////////////////////////////////////////////////////////////
class maxInterleavedCallsPerConnectionHandler : public orbOptions::Handler {
public:

  maxInterleavedCallsPerConnectionHandler() : 
    orbOptions::Handler("maxInterleavedCallsPerConnection",
			"maxInterleavedCallsPerConnection = n > 0",
			1,
			"-ORBmaxInterleavedCallsPerConnection < n > 0 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 1) {
      throw orbOptions::BadParam(key(),value,
			 orbOptions::expect_greater_than_zero_ulong_msg);
    }
    orbParameters::maxInterleavedCallsPerConnection = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::maxInterleavedCallsPerConnection,
			   result);
  }

};

static maxInterleavedCallsPerConnectionHandler maxInterleavedCallsPerConnectionHandler_;


/////////////////////////////////////////////////////////////////////////////
static const char* targetaddress_msg = "Expect a value of 0, 1 or 2";

class giopTargetAddressModeHandler : public orbOptions::Handler {
public:

  giopTargetAddressModeHandler() : 
    orbOptions::Handler("giopTargetAddressMode",
			"giopTargetAddressMode = 0,1,2",
			1,
			"-ORBgiopTargetAddressMode < 0 | 1 | 2 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,targetaddress_msg);
    }
    switch (v) {
    case 0:
      orbParameters::giopTargetAddressMode = GIOP::KeyAddr;
      break;
    case 1:
      orbParameters::giopTargetAddressMode = GIOP::ProfileAddr;
      break;
    case 2:
      orbParameters::giopTargetAddressMode = GIOP::ReferenceAddr;
      break;
    default:
      throw orbOptions::BadParam(key(),value,targetaddress_msg);
      break;
    }
  }

  void dump(orbOptions::sequenceString& result) {
    const char* v;
    if (orbParameters::giopTargetAddressMode == GIOP::KeyAddr)
      v = "KeyAddr"; 
    else if (orbParameters::giopTargetAddressMode == GIOP::ProfileAddr)
      v = "ProfileAddr";
    else if (orbParameters::giopTargetAddressMode == GIOP::ReferenceAddr)
      v = "ReferenceAddr";
    else 
      v = "Illegal value";
    orbOptions::addKVString(key(),v,result);
  }

};

static giopTargetAddressModeHandler giopTargetAddressModeHandler_;


/////////////////////////////////////////////////////////////////////////////
class strictIIOPHandler : public orbOptions::Handler {
public:

  strictIIOPHandler() : 
    orbOptions::Handler("strictIIOP",
			"strictIIOP = 0 or 1",
			1,
			"-ORBstrictIIOP < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::strictIIOP = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::strictIIOP,
			     result);
  }
};

static strictIIOPHandler strictIIOPHandler_;

/////////////////////////////////////////////////////////////////////////////
class immediateRopeSwitchHandler : public orbOptions::Handler {
public:

  immediateRopeSwitchHandler() : 
    orbOptions::Handler("immediateAddressSwitch",
			"immediateAddressSwitch = 0 or 1",
			1,
			"-ORBimmediateAddressSwitch < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::immediateRopeSwitch = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::immediateRopeSwitch,
			     result);
  }
};

static immediateRopeSwitchHandler immediateRopeSwitchHandler_;



/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

extern omniInitialiser& omni_giopImpl10_initialiser_;
extern omniInitialiser& omni_giopImpl11_initialiser_;
extern omniInitialiser& omni_giopImpl12_initialiser_;

class omni_giopStreamImpl_initialiser : public omniInitialiser {
public:

  omni_giopStreamImpl_initialiser() {
    orbOptions::singleton().registerHandler(maxGIOPVersionHandler_);
    orbOptions::singleton().registerHandler(giopMaxMsgSizeHandler_);
    orbOptions::singleton().registerHandler(clientCallTimeOutPeriodHandler_);
    orbOptions::singleton().registerHandler(supportPerThreadTimeOutHandler_);
    orbOptions::singleton().registerHandler(clientConnectTimeOutPeriodHandler_);
    orbOptions::singleton().registerHandler(serverCallTimeOutPeriodHandler_);
    orbOptions::singleton().registerHandler(maxInterleavedCallsPerConnectionHandler_);
    orbOptions::singleton().registerHandler(giopTargetAddressModeHandler_);
    orbOptions::singleton().registerHandler(strictIIOPHandler_);
    orbOptions::singleton().registerHandler(immediateRopeSwitchHandler_);
  }

  void attach() {
    OMNIORB_ASSERT(implHead == 0);
    OMNIORB_ASSERT(implMax  == 0);
    omni_giopImpl10_initialiser_.attach();
    if (orbParameters::maxGIOPVersion.minor >= 1)
      omni_giopImpl11_initialiser_.attach();
    if (orbParameters::maxGIOPVersion.minor >= 2)
      omni_giopImpl12_initialiser_.attach();
    {
      if (omniORB::trace(25)) {
	GIOP::Version v = giopStreamImpl::maxVersion()->version();
	omniORB::logger log;
	log << "Maximum supported GIOP version is " << (int)v.major 
	    << "." << (int)v.minor << "\n";
      }
    }
  }

  void detach() { 
    omni_giopImpl10_initialiser_.detach();
    if (orbParameters::maxGIOPVersion.minor >= 1)
      omni_giopImpl11_initialiser_.detach();
    if (orbParameters::maxGIOPVersion.minor >= 2)
      omni_giopImpl12_initialiser_.detach();
    implHead = 0;
    implMax  = 0;
  }
};

static omni_giopStreamImpl_initialiser initialiser;

omniInitialiser& omni_giopStreamImpl_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)

