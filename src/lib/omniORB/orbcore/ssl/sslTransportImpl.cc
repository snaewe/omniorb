// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslTransportImpl.cc        Created on: 29 May 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2007 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
  Revision 1.1.4.7  2009/06/16 11:19:30  dgrisby
  Leak of empty host string in SSL transport.

  Revision 1.1.4.6  2009/05/06 16:14:49  dgrisby
  Update lots of copyright notices.

  Revision 1.1.4.5  2007/11/28 12:24:26  dgrisby
  Implement a tiny subset of CSIv2 to permit multiple SSL endpoints in IORs.

  Revision 1.1.4.4  2006/03/25 18:54:03  dgrisby
  Initial IPv6 support.

  Revision 1.1.4.3  2005/09/05 17:12:20  dgrisby
  Merge again. Mainly SSL transport changes.

  Revision 1.1.4.2  2005/03/30 23:35:58  dgrisby
  Another merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:01:59  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.9  2002/09/09 22:11:51  dgrisby
  SSL transport cleanup even if certificates are wrong.

  Revision 1.1.2.8  2002/08/23 14:18:38  dgrisby
  Avoid init exceptioni when SSL linked but not configured.

  Revision 1.1.2.7  2002/04/16 12:44:27  dpg1
  Fix SSL accept bug, clean up logging.

  Revision 1.1.2.6  2001/08/23 16:02:58  sll
  Implement getInterfaceAddress().

  Revision 1.1.2.5  2001/08/03 17:41:25  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.2.4  2001/07/31 16:16:22  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.3  2001/07/26 16:37:21  dpg1
  Make sure static initialisers always run.

  Revision 1.1.2.2  2001/06/18 20:27:56  sll
  Use strchr instead of index() for maximal portability.

  Revision 1.1.2.1  2001/06/11 18:11:05  sll
  *** empty log message ***

*/

#include <omniORB4/CORBA.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/giopEndpoint.h>
#include <omniORB4/omniURI.h>
#include <objectAdapter.h>
#include <omniORB4/sslContext.h>
#include <ssl/sslConnection.h>
#include <ssl/sslAddress.h>
#include <ssl/sslEndpoint.h>
#include <ssl/sslTransportImpl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <orbOptions.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(sslAddress);
OMNI_FORCE_LINK(sslConnection);
OMNI_FORCE_LINK(sslContext);
OMNI_FORCE_LINK(sslEndpoint);
OMNI_FORCE_LINK(sslActive);

OMNI_EXPORT_LINK_FORCE_SYMBOL(omnisslTP);


OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
sslTransportImpl::sslTransportImpl(sslContext* ctx) : 
  giopTransportImpl("giop:ssl"), pd_ctx(ctx) {
}

/////////////////////////////////////////////////////////////////////////
sslTransportImpl::~sslTransportImpl() {
}

/////////////////////////////////////////////////////////////////////////
giopEndpoint*
sslTransportImpl::toEndpoint(const char* param) {

  IIOP::Address address;

  char* host = omniURI::extractHostPort(param, address.port);
  if (!host)
    return 0;

  if (*host == '\0') {
    // No name in param -- try environment variable.
    const char* hostname = getenv(OMNIORB_USEHOSTNAME_VAR);
    if (hostname)
      address.host = hostname;

    CORBA::string_free(host);
  }
  else {
    address.host = host;
  }
  return (giopEndpoint*)(new sslEndpoint(address, pd_ctx));
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
sslTransportImpl::isValid(const char* param) {
  
  return omniURI::validHostPort(param);
}

/////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
parseAddress(const char* param, IIOP::Address& address) {

  char* host = omniURI::extractHostPort(param, address.port);
  if (!host)
    return 0;

  address.host = host;
  return 1;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
sslTransportImpl::toAddress(const char* param) {

  IIOP::Address address;
  if (parseAddress(param,address)) {
    return (giopAddress*)(new sslAddress(address,pd_ctx));
  }
  else {
    return 0;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
sslTransportImpl::addToIOR(const char* param) {

  IIOP::Address address;
  if (parseAddress(param,address)) {
    // XXX, hard-wired security options to:
    //       Integrity (0x02) Confidentiality (0x04) |
    //       EstablishTrustInTarget (0x20) | EstablishTrustInClient (0x40)
    // In future, this should be expanded configurable options.
    omniIOR::add_TAG_SSL_SEC_TRANS(address,0x66,0x66);
    return 1;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
const omnivector<const char*>* 
sslTransportImpl::getInterfaceAddress() {
  return giopTransportImpl::getInterfaceAddress("giop:tcp");
}


/////////////////////////////////////////////////////////////////////////////
class sslCAFileHandler : public orbOptions::Handler {
public:

  sslCAFileHandler() : 
    orbOptions::Handler("sslCAFile",
			"sslCAFile = <certificate authority file>",
			1,
			"-ORBsslCAFile <certificate authority file>") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam)
  {    
    sslContext::certificate_authority_file = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
			    sslContext::certificate_authority_file ?
			    sslContext::certificate_authority_file : "<unset>",
			    result);
  }
};

static sslCAFileHandler sslCAFileHandler_;


/////////////////////////////////////////////////////////////////////////////
class sslKeyFileHandler : public orbOptions::Handler {
public:

  sslKeyFileHandler() : 
    orbOptions::Handler("sslKeyFile",
			"sslKeyFile = <key file>",
			1,
			"-ORBsslKeyFile <key file>") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam)
  {    
    sslContext::key_file = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
			    sslContext::key_file ?
			    sslContext::key_file : "<unset>",
			    result);
  }
};

static sslKeyFileHandler sslKeyFileHandler_;


/////////////////////////////////////////////////////////////////////////////
class sslKeyPasswordHandler : public orbOptions::Handler {
public:

  sslKeyPasswordHandler() : 
    orbOptions::Handler("sslKeyPassword",
			"sslKeyPassword = <key file password>",
			1,
			"-ORBsslKeyPassword <key file password>") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam)
  {    
    sslContext::key_file_password = CORBA::string_dup(value);
  }

  void dump(orbOptions::sequenceString& result)
  {
    orbOptions::addKVString(key(),
			    sslContext::key_file_password ? "****" : "<unset>",
			    result);
  }
};

static sslKeyPasswordHandler sslKeyPasswordHandler_;


/////////////////////////////////////////////////////////////////////////////
class sslVerifyModeHandler : public orbOptions::Handler {
public:

  sslVerifyModeHandler() : 
    orbOptions::Handler("sslVerifyMode",
			"sslVerifyMode = <mode>",
			1,
			"-ORBsslVerifyMode < \"none\" | \"peer[,fail][,once]\" >")
  {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam)
  {    
    if (!strcmp(value, "none")) {
      sslContext::verify_mode = 0;
      return;
    }

    int mode = 0;
    CORBA::String_var val(value); // Copy
    char* valc = (char*)val;
    char* c;

    while (*valc) {
      for (c=valc; *c && *c != ','; ++c) {}

      if (*c == ',')
	*c++ = '\0';

      if (!strcmp(valc, "peer"))
	mode |= SSL_VERIFY_PEER;

      else if (!strcmp(valc, "fail"))
	mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;

      else if (!strcmp(valc, "once"))
	mode |= SSL_VERIFY_CLIENT_ONCE;

      else
	throw orbOptions::BadParam(key(), valc, "Invalid verify option");
      
      valc = c;
    }
    sslContext::verify_mode = mode;
  }

  void dump(orbOptions::sequenceString& result)
  {
    char buf[20];

    buf[0] = '\0';

    if (sslContext::verify_mode & SSL_VERIFY_PEER)
      strcpy(buf, "peer");
    else
      strcpy(buf, "none");
      
    if (sslContext::verify_mode & SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
      strcat(buf, ",fail");

    if (sslContext::verify_mode & SSL_VERIFY_CLIENT_ONCE)
      strcat(buf, ",once");

    orbOptions::addKVString(key(), buf, result);
  }
};

static sslVerifyModeHandler sslVerifyModeHandler_;



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
static sslTransportImpl* _the_sslTransportImpl = 0;

class omni_sslTransport_initialiser : public omniInitialiser {
public:

  omni_sslTransport_initialiser() {
    orbOptions::singleton().registerHandler(sslCAFileHandler_);
    orbOptions::singleton().registerHandler(sslKeyFileHandler_);
    orbOptions::singleton().registerHandler(sslKeyPasswordHandler_);
    orbOptions::singleton().registerHandler(sslVerifyModeHandler_);
    omniInitialiser::install(this);
  }

  void attach() {
    if (_the_sslTransportImpl) return;

    if (!sslContext::singleton) {

      if (omniORB::trace(5)) {
	omniORB::logger log;
	log << "No SSL context object supplied, attempt to create one "
	    << "with the default ctor.\n";
      }
      struct stat sb;

      if (!sslContext::certificate_authority_file || 
	  stat(sslContext::certificate_authority_file,&sb) < 0) {
	if (omniORB::trace(1)) {
	  omniORB::logger log;
	  log << "Warning: SSL CA certificate file is not set "
	      << "or cannot be found. SSL transport disabled.\n";
	}
	return;
      }
      
      // Create the default singleton
      sslContext::singleton =
	new sslContext(sslContext::certificate_authority_file,
		       sslContext::key_file,
		       sslContext::key_file_password);
    }
    sslContext::singleton->internal_initialise();
    _the_sslTransportImpl = new sslTransportImpl(sslContext::singleton);
  }

  void detach() { 
    if (_the_sslTransportImpl) delete _the_sslTransportImpl;
    _the_sslTransportImpl = 0;
    if (sslContext::singleton) delete sslContext::singleton;
    sslContext::singleton = 0;
  }


};

static omni_sslTransport_initialiser initialiser;

OMNI_NAMESPACE_END(omni)
