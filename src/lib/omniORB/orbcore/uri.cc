// -*- Mode: C++; -*-
//                            Package   : omniORB
// uri.cc                     Created on: 2000/04/03
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
//      Parsing for object reference URIs
//	*** PROPRIETARY INTERFACE ***
//      

// $Log$
// Revision 1.4.2.2  2005/01/06 23:10:41  dgrisby
// Big merge from omni4_0_develop.
//
// Revision 1.4.2.1  2003/03/23 21:02:00  dgrisby
// Start of omniORB 4.1.x development branch.
//
// Revision 1.2.2.16  2003/02/17 02:03:09  dgrisby
// vxWorks port. (Thanks Michael Sturm / Acterna Eningen GmbH).
//
// Revision 1.2.2.15  2002/02/13 16:03:17  dpg1
// Memory leak due to missing virtual destructor.
//
// Revision 1.2.2.14  2002/02/01 11:20:40  dpg1
// Silly bug in string_to_object wil nil objref.
//
// Revision 1.2.2.13  2001/12/04 14:32:27  dpg1
// Minor corbaloc bugs.
//
// Revision 1.2.2.12  2001/08/18 17:28:38  sll
// Fixed minor bug introduced by the previous update.
//
// Revision 1.2.2.11  2001/08/17 17:14:54  sll
// Moved handler initialisation into static initialisers.
//
// Revision 1.2.2.10  2001/08/03 17:41:25  sll
// System exception minor code overhaul. When a system exeception is raised,
// a meaning minor code is provided.
//
// Revision 1.2.2.9  2001/07/31 17:42:11  sll
// Cleanup String_var usage.
//
// Revision 1.2.2.8  2001/06/11 17:53:22  sll
//  The omniIOR ctor used by genior and corbaloc now has the option to
//  select whether to call interceptors and what set of interceptors to call.
//
// Revision 1.2.2.7  2001/06/08 17:12:22  dpg1
// Merge all the bug fixes from omni3_develop.
//
// Revision 1.2.2.6  2001/04/18 18:18:04  sll
// Big checkin with the brand new internal APIs.
//
// Revision 1.2.2.5  2000/11/22 14:39:55  dpg1
// Treat empty host name in corbaloc: as localhost, as per 2.4 spec.
//
// Revision 1.2.2.4  2000/11/20 11:58:39  dpg1
// No need to initialise omniIOR module before uri module.
//
// Revision 1.2.2.3  2000/11/09 12:27:59  dpg1
// Huge merge from omni3_develop, plus full long long from omni3_1_develop.
//
// Revision 1.2.2.2  2000/09/27 18:24:14  sll
// Use omniObjRef::_toString and _fromString. Use the new omniIOR class and
// createObjRef().
//
// Revision 1.2.2.1  2000/07/17 10:36:00  sll
// Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
//
// Revision 1.3  2000/07/13 15:25:54  dpg1
// Merge from omni3_develop for 3.0 release.
//
// Revision 1.1.2.6  2000/06/30 14:12:07  dpg1
// Minor fixes for FreeBSD.
//
// Revision 1.1.2.5  2000/06/27 15:40:58  sll
// Workaround for Cygnus gcc's inability to recognise _CORBA_Octet*& and
// CORBA::Octet*& are the same type.
//
// Revision 1.1.2.4  2000/06/22 10:40:17  dpg1
// exception.h renamed to exceptiondefs.h to avoid name clash on some
// platforms.
//
// Revision 1.1.2.3  2000/06/19 14:18:33  dpg1
// Explicit cast to (const char*) when using String_var with logger.
//
// Revision 1.1.2.2  2000/05/24 17:18:11  dpg1
// Rename IIOP::DEFAULT_PORT IIOP::DEFAULT_CORBALOC_PORT
//
// Revision 1.1.2.1  2000/04/27 10:52:29  dpg1
// Interoperable Naming Service
//
// Implementation of URI functions.
//

#include <stdlib.h>
#include <omniORB4/CORBA.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/minorCode.h>
#include <initRefs.h>
#include <SocketCollection.h>
#include <ctype.h>

OMNI_NAMESPACE_BEGIN(omni)

#define MAX_STRING_TO_OBJECT_CYCLES 10

static omnivector<omniURI::URIHandler*> handlers;


char*
omniURI::objectToString(CORBA::Object_ptr obj)
{
  if (obj && obj->_NP_is_pseudo()) {
    OMNIORB_THROW(MARSHAL,MARSHAL_InvalidIOR,CORBA::COMPLETED_NO);
  }

  omniObjRef* objref = obj ? obj->_PR_getobj() : 0;

  return omniObjRef::_toString(objref);
}


CORBA::Object_ptr
omniURI::stringToObject(const char* uri, unsigned int cycles)
{
  if (!uri) OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);

  if (cycles > MAX_STRING_TO_OBJECT_CYCLES) {
    if (omniORB::trace(1)) {
      omniORB::logger l;
      l << "string_to_object reached recursion limit processing `"
	<< uri << "'.\n";
    }
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);
  }

  URIHandler* handler = 0;
  {
    omnivector<omniURI::URIHandler*>::iterator i = handlers.begin();
    omnivector<omniURI::URIHandler*>::iterator last = handlers.end();

    while (i != last) {
      if ((*i)->supports(uri)) {
	handler = *i;
	break;
      }
      i++;
    }
  }
  if (handler)
    return handler->toObject(uri, cycles);

  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadSchemeName, CORBA::COMPLETED_NO);
  return 0;
}


CORBA::Boolean
omniURI::uriSyntaxIsValid(const char* uri)
{
  if (!uri) return 0;

  URIHandler* handler = 0;
  {
    omnivector<omniURI::URIHandler*>::iterator i = handlers.begin();
    omnivector<omniURI::URIHandler*>::iterator last = handlers.end();

    while (i != last) {
      if ((*i)->supports(uri)) {
	handler = *i;
	break;
      }
      i++;
    }
  }
  if (handler) return handler->syntaxIsValid(uri);
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
// IOR: format
/////////////////////////////////////////////////////////////////////////////

class iorURIHandler : public omniURI::URIHandler {
public:
  CORBA::Boolean    supports     (const char* uri);
  CORBA::Object_ptr toObject     (const char* uri, unsigned int);
  CORBA::Boolean    syntaxIsValid(const char* uri);
};

CORBA::Boolean
iorURIHandler::supports(const char* uri)
{
  return ((uri[0] == 'I' || uri[0] == 'i') &&
	  (uri[1] == 'O' || uri[1] == 'o') &&
	  (uri[2] == 'R' || uri[2] == 'r') &&
	  (uri[3] == ':'));
}

CORBA::Object_ptr
iorURIHandler::toObject(const char* sior, unsigned int)
{
  omniObjRef* objref = omniObjRef::_fromString(sior);
  if (!objref)
    return CORBA::Object::_nil();

  return (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}

CORBA::Boolean
iorURIHandler::syntaxIsValid(const char* sior)
{
  // Just check that the IOR is a sequence of hex digits
  int i;
  for (i=4; sior[i]; i++) {
    if (!((sior[i] >= '0' && sior[i] <= '9') ||
	  (sior[i] >= 'a' && sior[i] <= 'f') ||
	  (sior[i] >= 'A' && sior[i] <= 'F')))
      return 0;
  }
  if (i == 4 || i % 2) {
    // No digits, or odd number
    return 0;
  }
  return 1;
}



/////////////////////////////////////////////////////////////////////////////
// corbaloc: format
/////////////////////////////////////////////////////////////////////////////


class corbalocURIHandler : public omniURI::URIHandler {
public:
  CORBA::Boolean    supports     (const char* uri);
  CORBA::Object_ptr toObject     (const char* uri, unsigned int cycles);
  CORBA::Boolean    syntaxIsValid(const char* uri);

  // This function actually does the work
  static CORBA::Object_ptr locToObject(const char*& c,
				       unsigned int cycles,
				       const char*  def_key = 0);

  class ObjAddr {
  public:
    ObjAddr() : next_(0) {}

    virtual ~ObjAddr() {}

    static ObjAddr* parse(const char*& c);

    enum AddrKind { rir, iiop, uiop, ssliop };
    virtual AddrKind kind() = 0;

    ObjAddr* next_;
  };

  class ObjAddrList {
  public:
    ObjAddrList() : head_(0), tail_(0) {}

    ~ObjAddrList() {
      ObjAddr *c, *n;
      for (c = head_; c; c = n) { n = c->next_;	delete c; }
    }
    void append(ObjAddr* addr) {
      if (head_) {
	tail_->next_ = addr;
	tail_        = addr;
      }
      else {
	head_ = addr;
	tail_ = addr;
      }
    }
    ObjAddr* head_;
    ObjAddr* tail_;
  };

  class RirObjAddr : public ObjAddr {
  public:
    RirObjAddr(const char*& c) {};
    virtual ~RirObjAddr()      {};
    ObjAddr::AddrKind kind()   { return ObjAddr::rir; }
  };

  class IiopObjAddr : public ObjAddr {
  public:
    IiopObjAddr(const char*& c);
    virtual ~IiopObjAddr() {};

    ObjAddr::AddrKind kind() { return ObjAddr::iiop; }

    CORBA::Char   minver() { return minver_; }
    CORBA::Char   majver() { return majver_; }
    const char*   host()   { return (const char*)host_; }
    CORBA::UShort port()   { return port_; }
    
  private:
    CORBA::Char       majver_;
    CORBA::Char       minver_;
    CORBA::String_var host_;
    CORBA::UShort     port_;
  };

  class SsliopObjAddr : public IiopObjAddr {
  public:
    SsliopObjAddr(const char*& c) : IiopObjAddr(c) {};

    ObjAddr::AddrKind kind() { return ObjAddr::ssliop; }
    
  private:
  };

  class UiopObjAddr : public ObjAddr {
  public:
    UiopObjAddr(const char*& c);
    virtual ~UiopObjAddr() {};

    ObjAddr::AddrKind kind() { return ObjAddr::uiop; }

    CORBA::Char   minver()   { return minver_; }
    CORBA::Char   majver()   { return majver_; }
    const char*   filename() { return (const char*)filename_; }
    
  private:
    CORBA::Char       majver_;
    CORBA::Char       minver_;
    CORBA::String_var filename_;
  };

  // Object containing all the parsed data for a corbaloc:
  class Parsed {
  public:
    Parsed(const char*& c, const char* def_key);
    ~Parsed() {};

    ObjAddrList       addrList_;
    unsigned int      addr_count_;
    CORBA::Boolean    is_rir_;
    CORBA::String_var key_;
    unsigned int      key_size_;
  };
};

CORBA::Boolean
corbalocURIHandler::supports(const char* uri)
{
  return !strncmp(uri, "corbaloc:", 9);
}

CORBA::Object_ptr
corbalocURIHandler::toObject(const char* uri, unsigned int cycles)
{
  const char* c = uri + 9;

  CORBA::Object_ptr obj = locToObject(c, cycles);

  if (*c != '\0') {
    // uri didn't end with the object key
    CORBA::release(obj);
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadSchemeSpecificPart,
		  CORBA::COMPLETED_NO);
  }
  return obj;
}

CORBA::Boolean
corbalocURIHandler::syntaxIsValid(const char* uri)
{
  const char* c = uri + 9;
  try {
    Parsed parsed(c, 0);
  }
  catch (...) {
    return 0;
  }
  if (*c != '\0') {
    // uri didn't end with the object key
    return 0;
  }
  return 1;
}


corbalocURIHandler::ObjAddr*
corbalocURIHandler::ObjAddr::parse(const char*& c)
{
  if (!strncmp(c, ":", 1)) {
    c += 1;
    return new corbalocURIHandler::IiopObjAddr(c);
  }
  if (!strncmp(c, "iiop:", 5)) {
    c += 5;
    return new corbalocURIHandler::IiopObjAddr(c);
  }
  if (!strncmp(c, "ssliop:", 7)) {
    c += 7;
    return new corbalocURIHandler::SsliopObjAddr(c);
  }
  if (!strncmp(c, "omniunix:", 9)) {
    c += 9;
    return new corbalocURIHandler::UiopObjAddr(c);
  }
  if (!strncmp(c, "rir:", 4)) {
    c += 4;
    return new corbalocURIHandler::RirObjAddr(c);
  }
  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadSchemeName, CORBA::COMPLETED_NO);
  return 0;
}


static void
ParseVersionNumber(const char*& c, CORBA::Char& majver, CORBA::Char& minver)
{
  const char* p;
  for (p=c;
       *p && *p != '@' && *p != ':' && *p != ',' && *p != '/' && *p != '#';
       p++);

  if (*p == '@') {
    // Found a version number
    unsigned long l;

    // Major
    p = c;
    if (!isdigit(*p)) OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadSchemeSpecificPart,
				    CORBA::COMPLETED_NO);
    l = strtoul(p, (char**)&p, 10);
    if (l > 0xff)     OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadSchemeSpecificPart,
				    CORBA::COMPLETED_NO);
    majver = l;

    // Minor
    if (*p++ != '.')  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadSchemeSpecificPart,
				    CORBA::COMPLETED_NO);
    if (!isdigit(*p)) OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadSchemeSpecificPart,
				    CORBA::COMPLETED_NO);
    l = strtoul(p, (char**)&p, 10);
    if (l > 0xff)     OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadSchemeSpecificPart,
				    CORBA::COMPLETED_NO);
    minver = l;

    if (*p != '@')    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadSchemeSpecificPart,
				    CORBA::COMPLETED_NO);
    c = p + 1;
  }
  else {
    majver = 1;
    minver = 0;
  }
}


corbalocURIHandler::UiopObjAddr::UiopObjAddr(const char*& c)
{
  const char* p;
  ParseVersionNumber(c, majver_, minver_);

  for (p=c; *p && *p != ':' && *p != ',' && *p != '#'; p++);

  if (p == c) OMNIORB_THROW(BAD_PARAM,
                            BAD_PARAM_BadSchemeSpecificPart,
			    CORBA::COMPLETED_NO);

  filename_ = CORBA::string_alloc(1 + p - c);
  char* f = (char*)filename_;

  for (; c != p; c++, f++) *f = *c;
  *f = '\0';

  if (*c == ':') {
    // Object key should follow.
    ++c;
  }
}


corbalocURIHandler::IiopObjAddr::IiopObjAddr(const char*& c)
{
  if (*c == '\0' || *c == ',' || *c == '/' || *c == '#') {
    // Empty host name -- use localhost, default port
    host_   = CORBA::string_dup("localhost");
    port_   = IIOP::DEFAULT_CORBALOC_PORT;
    majver_ = 1;
    minver_ = 0;
    return;
  }

  const char* p;
  ParseVersionNumber(c, majver_, minver_);

  for (p=c; *p && *p != ':' && *p != ',' && *p != '/' && *p != '#'; p++);

  if (p == c) OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_BadSchemeSpecificPart,
			    CORBA::COMPLETED_NO);

  host_ = CORBA::string_alloc(1 + p - c);
  char* h = (char*)host_;

  for (; c != p; c++, h++) *h = *c;
  *h = '\0';

  if (*c == ':') {
    // Port number follows
    c++;
    if (isdigit(*c)) {
      unsigned long l;
      l = strtoul(c, (char**)&c, 10);
      if (l > 0xffff) OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadAddress,
				    CORBA::COMPLETED_NO);
      port_ = l;
    }
    else {
      port_ = IIOP::DEFAULT_CORBALOC_PORT;
    }
  }
  else {
    port_ = IIOP::DEFAULT_CORBALOC_PORT;
  }
}


static inline int
validKeyChar(const char c)
{
  return ((c >= 'A' && c <= 'Z') ||
	  (c >= 'a' && c <= 'z') ||
	  (c >= '0' && c <= '9') ||
	  c == ';' || c == '/' || c == '?' || c == ':' || c == '@' ||
	  c == '&' || c == '=' || c == '+' || c == '$' || c == ',' ||
	  c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
	  c == '*' || c == '(' || c == ')' || c == '\'');
}

static char*
unescapeKey(const char*& c, unsigned int& key_size)
{
  const char* p;
  for (p=c; *p && *p != '#'; p++);

  char* key = CORBA::string_alloc(1 + p - c);
  char* k   = key;
  key_size  = 0;

  for (; c != p; c++, k++) {
    key_size++;
    if (validKeyChar(*c)) {
      *k = *c;
    }
    else if (*c == '%') {
      // Escape char
      c++;
      if      (*c >= '0' && *c <= '9') *k = (*c - '0')      << 4;
      else if (*c >= 'A' && *c <= 'F') *k = (*c - 'A' + 10) << 4;
      else if (*c >= 'a' && *c <= 'f') *k = (*c - 'a' + 10) << 4;
      else {
	CORBA::string_free(key);
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);
      }
      c++;
      if      (*c >= '0' && *c <= '9') *k |= (*c - '0');
      else if (*c >= 'A' && *c <= 'F') *k |= (*c - 'A' + 10);
      else if (*c >= 'a' && *c <= 'f') *k |= (*c - 'a' + 10);
      else {
	CORBA::string_free(key);
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);
      }
    }
    else {
      CORBA::string_free(key);
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);
    }
  }
  *k = '\0';
  return key;
}

corbalocURIHandler::
Parsed::Parsed(const char*& c, const char* def_key)
{
  ObjAddr* addr;
  is_rir_     = 0;
  addr_count_ = 1;

  addr = ObjAddr::parse(c);
  if (!addr) 
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);

  if (addr->kind() == ObjAddr::rir) is_rir_ = 1;
  addrList_.append(addr);

  while (*c == ',') {
    c++;
    addr_count_++;
    addr = ObjAddr::parse(c);
    if (!addr) 
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);
    if (addr->kind() == ObjAddr::rir) is_rir_ = 1;
    addrList_.append(addr);
  }
  if (is_rir_ && (addr_count_ != 1))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);

  if (*c == '/') {
    // Key string follows
    c++;
    key_ = unescapeKey(c, key_size_);
  }
  else if (def_key) {
    key_      = def_key; // Copying assignment of default key
    key_size_ = strlen(key_);
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);
}


CORBA::Object_ptr
corbalocURIHandler::locToObject(const char*& c, unsigned int cycles,
				const char* def_key)
{
  Parsed parsed(c, def_key);

  if (parsed.is_rir_) {
    try {
      return omniInitialReferences::resolve(parsed.key_, cycles+1);
    }
    catch (CORBA::ORB::InvalidName& ex) {
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);
    }
  }
  else {
    // Protocols other than rir

    IIOP::Address* addrlist = new IIOP::Address[parsed.addr_count_];
    int iiop_addr_count = 0;
    IOP::MultipleComponentProfile tagged_components;

    GIOP::Version ver;
    ver.major = 127; ver.minor = 127;

    char  self[64];
    char* selfp = 0;

    ObjAddr* addr;
    int i;
    for (i=0, addr = parsed.addrList_.head_; 
	 addr; 
	 addr = addr->next_, i++) 
      {
	switch (addr->kind()) {
	case ObjAddr::iiop:
	case ObjAddr::ssliop:
	  {
	    IiopObjAddr* iaddr = (IiopObjAddr*)addr;
	    addrlist[iiop_addr_count].host = iaddr->host();
	    addrlist[iiop_addr_count].port = iaddr->port();
	    if (iaddr->majver() < ver.major ||
		iaddr->minver() < ver.minor) {
	      ver.major = iaddr->majver();
	      ver.minor = iaddr->minver();
	    }
            ++iiop_addr_count;
            
            if (addr->kind() == ObjAddr::ssliop) {
	      addrlist[iiop_addr_count - 1].port = 0;
	      ver.major = 1;
	      ver.minor = 2;

	      CORBA::ULong index = tagged_components.length();
	      tagged_components.length(index+1);
	      IOP::TaggedComponent& c = tagged_components[index];
	      c.tag = IOP::TAG_SSL_SEC_TRANS;
	      cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
	      CORBA::UShort zero = 0;
	      zero >>= s;
	      zero >>= s;
	      iaddr->port() >>= s;

	      CORBA::Octet* p;
	      CORBA::ULong max, len;
	      s.getOctetStream(p,max,len);
	      c.component_data.replace(max,len,p,1);
            }
	  }
	  break;
	case ObjAddr::uiop:
	  {
	    if (!selfp) {
	      if (gethostname(&self[0],64) == RC_SOCKET_ERROR) {
		omniORB::logs(1, "Cannot get the name of this host.");
		self[0] = '\0';
	      }
	      selfp = self;
	    }
            UiopObjAddr* uiop_addr = (UiopObjAddr*)addr;
            CORBA::ULong index = tagged_components.length();
            tagged_components.length(index+1);
            IOP::TaggedComponent& c = tagged_components[index];
            c.tag = IOP::TAG_OMNIORB_UNIX_TRANS;
            cdrEncapsulationStream s(CORBA::ULong(0),CORBA::Boolean(1));
            s.marshalRawString(self);
            s.marshalRawString(uiop_addr->filename());

            CORBA::Octet* p;
	    CORBA::ULong max, len;
	    s.getOctetStream(p,max,len);
            c.component_data.replace(max,len,p,1);
	  }
	  break;
	default:
	  OMNIORB_ASSERT(0);
	}
      }
    
    _CORBA_Unbounded_Sequence_Octet key;
    key.replace(parsed.key_size_,
		parsed.key_size_,
		(CORBA::Octet*)(const char*)parsed.key_,0);

    if (iiop_addr_count <= 0) {
      OMNIORB_ASSERT(selfp);
      ver.major = 1;
      ver.minor = 2;
      addrlist[0].host = (const char *)self;
      addrlist[0].port = 0;
      ++iiop_addr_count;
    }

    omniIOR* ior = new omniIOR((const char*)"",
			       key,
			       addrlist,iiop_addr_count,
			       ver,omniIOR::NoInterceptor,
                               &tagged_components);
    delete [] addrlist;

    omniObjRef* objref = omni::createObjRef(CORBA::Object::_PD_repoId,ior,0);

    OMNIORB_ASSERT(objref);

    return (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
  }
  OMNIORB_ASSERT(0);
  return 0;
}


#if !(defined(__vxWorks__) && defined(__vxNames__))
/////////////////////////////////////////////////////////////////////////////
// corbaname: format
/////////////////////////////////////////////////////////////////////////////

class corbanameURIHandler : public omniURI::URIHandler {
public:
  CORBA::Boolean    supports     (const char* uri);
  CORBA::Object_ptr toObject     (const char* uri, unsigned int cycles);
  CORBA::Boolean    syntaxIsValid(const char* uri);
};


CORBA::Boolean
corbanameURIHandler::supports(const char* uri)
{
  return !strncmp(uri, "corbaname:", 10);
}

CORBA::Object_ptr
corbanameURIHandler::toObject(const char* uri, unsigned int cycles)
{
  const char* c = uri + 10;

  CORBA::Object_var obj = corbalocURIHandler::locToObject(c, cycles,
							  "NameService");
  if (*c == '\0') {
    // Just return a reference to the name service
    return obj._retn();
  }

  if (*c != '#') {
    // uri didn't end with the name string
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_BadSchemeSpecificPart,
		  CORBA::COMPLETED_NO);
  }

  // Resolve the name from the name service
  CORBA::String_var   sname;
  CosNaming::Name_var name;

  try {
    c++;
    unsigned int key_size;
    sname = unescapeKey(c, key_size);
    name  = omniURI::stringToName(sname);
  }
  catch (CosNaming::NamingContext::InvalidName& ex) {
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_BadSchemeSpecificPart,
		  CORBA::COMPLETED_NO);
  }
  try {
    CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(obj);
    if (CORBA::is_nil(nc)) {
      if (omniORB::trace(10)) {
	omniORB::logger l;
	l << "string_to_object failed to narrow naming service reference "
	  << "in processing `" << uri << "'\n";
      }
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_BadSchemeSpecificPart,
		    CORBA::COMPLETED_NO);
    }
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "string_to_object attempting to resolve `" << (const char*)sname
	<< "' from naming service\n";
    }
    CORBA::Object_ptr result = nc->resolve(name);
    return result;
  }
  catch (CosNaming::NamingContext::NotFound& ex) {
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "string_to_object received a NotFound exception trying to "
	<< "resolve `" << (const char*)sname << "' from naming service\n";
    }
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);
  }
  catch (CosNaming::NamingContext::CannotProceed& ex) {
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "string_to_object received a CannotProceed exception trying to "
	<< "resolve `" << (const char*)sname << "' from naming service\n";
    }
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);
  }
  catch (CosNaming::NamingContext::InvalidName& ex) {
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "string_to_object received an InvalidName exception trying to "
	<< "resolve `" << (const char*)sname << "' from naming service\n";
    }
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_BadURIOther, CORBA::COMPLETED_NO);
  }
  // Never reach here
  OMNIORB_ASSERT(0);
  return 0;
}


CORBA::Boolean
corbanameURIHandler::syntaxIsValid(const char* uri)
{
  const char* c = uri + 10;
  try {
    corbalocURIHandler::Parsed parsed(c, "NameService");

    if (*c == '\0') {
      // Just a reference to the name service itself
      return 1;
    }
    if (*c != '#') {
      // uri didn't end with the name string
      return 0;
    }
    c++;
    unsigned int key_size;
    CORBA::String_var   sname;
    sname = unescapeKey(c, key_size);
    CosNaming::Name_var name  = omniURI::stringToName(sname);
  }
  catch (...) {
    return 0;
  }
  return 1;
}


// CosNaming::NamingContextExt operations

CosNaming::Name*
omniURI::stringToName(const char* sname)
{
  // *** This is ugly...

  if (!sname)         OMNIORB_THROW(BAD_PARAM,BAD_PARAM_NullStringUnexpected,
				    CORBA::COMPLETED_NO);
  if (*sname == '\0') throw CosNaming::NamingContext::InvalidName();

  unsigned int      i, j;
  unsigned int      component = 0;
  unsigned int      len       = strlen(sname);
  char*             bufp      = CORBA::string_alloc(len+1);
  CORBA::String_var buf(bufp);

  enum { s_id, s_kind } state = s_id;

  CosNaming::Name*    namep = new CosNaming::Name;
  CosNaming::Name_var name(namep);

  name->length(1);

  for (i=0,j=0; i <= len; i++) { // Yes, I really do mean <=
    if (sname[i] == '\\') {
      i++;
      if (sname[i] != '\\' && sname[i] != '/' && sname[i] != '.')
	throw CosNaming::NamingContext::InvalidName();
    }
    else if (sname[i] == '.') {
      if (state == s_id) {
	bufp[j] = '\0';
	name[component].id = (const char*)buf;
	j = 0;
	state = s_kind;
      }
      else throw CosNaming::NamingContext::InvalidName();
      continue;
    }
    else if (sname[i] == '/' || sname[i] == '\0') {
      if (state == s_id) {
	if (j == 0) throw CosNaming::NamingContext::InvalidName();
	bufp[j] = '\0';
	name[component].id   = (const char*)bufp;
	name[component].kind = (const char*)"";
      }
      else {
	if (j == 0 && ((const char*)(name[component].id))[0] != '\0') {
	  // Trailing . is only allowed if the id is empty
	  throw CosNaming::NamingContext::InvalidName();
	}
	bufp[j] = '\0';
	name[component].kind = (const char*)buf;
      }
      j = 0;
      state = s_id;
      if (sname[i] == '/') {
	component++;
	name->length(component+1);
      }
      continue;
    }
    bufp[j++] = sname[i];
  }
  return name._retn();
}

char*
omniURI::nameToString(const CosNaming::Name& name)
{
  // The strategy here is to scan through the whole Name, figuring out
  // how long the string will be. Then allocate a string of the right
  // length, and scan through the name again, writing the string.

  if (name.length() == 0) throw CosNaming::NamingContext::InvalidName();

  CORBA::ULong i;
  CORBA::ULong slen = 0;

  const char* c;

  for (i=0; i < name.length(); i++) {
    // Space for id
    for (c = name[i].id; *c; c++) {
      if (*c == '/' || *c == '.' || *c == '\\') slen++;
      slen++;
    }
    // Space for '.' if there is a kind, or no id
    if (((const char*)(name[i].id  ))[0] == '\0' || 
	((const char*)(name[i].kind))[0] != '\0')
      slen++;

    // Space for kind
    for (c = name[i].kind; *c; c++) {
      if (*c == '/' || *c == '.' || *c == '\\') slen++;
      slen++;
    }
    // Space for '/' or '\0'
    slen++;
  }

  char* sname = CORBA::string_alloc(slen);
  char* s     = sname;

  for (i=0; i < name.length(); i++) {
    // id
    for (c = name[i].id; *c; c++) {
      if (*c == '/' || *c == '.' || *c == '\\') *s++ = '\\';
      *s++ = *c;
    }
    // '.' if there is a kind, or no id
    if (((const char*)(name[i].id  ))[0] == '\0' || 
	((const char*)(name[i].kind))[0] != '\0')
      *s++ = '.';

    // kind
    for (c = name[i].kind; *c; c++) {
      if (*c == '/' || *c == '.' || *c == '\\') *s++ = '\\';
      *s++ = *c;
    }
    // '/' (overwritten by '\0' at the end)
    *s++ = '/';
  }
  sname[slen-1] = '\0';

  return sname;
}


char*
omniURI::addrAndNameToURI(const char* addr, const char* sn)
{
  // The strategy here is basically the same as that for
  // nameToString() -- scan the string name to see how long it will be
  // with escape chars, the allocate the string, and copy the data.

  // The Naming Service specification is unlear as to whether the
  // address string should have invalid characters escaped. It seems
  // most sensible not to escape the address, not least because it may
  // contain address components which we don't understand, and thus
  // can't reliably escape. Note that we can't escape the string as a
  // precaution, since escaping a previously-escaped string escapes
  // the original % escape characters.

  if (!addr || *addr == '\0')
    throw CosNaming::NamingContextExt::InvalidAddress();

  if (!sn)
    throw CosNaming::NamingContext::InvalidName();

  unsigned int len     = 0;
  unsigned int addrlen = strlen(addr);
  const char*  c;

  // Check that the address is valid
  try {
    c = addr;
    corbalocURIHandler::Parsed parsed(c, "NameService");
    if (*c != '\0')
      throw CosNaming::NamingContextExt::InvalidAddress();
  }
  catch (CORBA::BAD_PARAM& ex) {
    throw CosNaming::NamingContextExt::InvalidAddress();
  }

  // Check that the stringified name is valid
  if (*sn != '\0') {
    CosNaming::Name_var name = omniURI::stringToName(sn);
    // stringToName() throws InvalidName if the name is invalid
  }

  for (c=sn; *c; len++,c++)
    if (!validKeyChar(*c))
      len += 2; // Invalid chars take the form %xx

  len += 10 /* corbaname: */ + addrlen + 1 /* # */ + 1 /* \0 */;

  char* url = CORBA::string_alloc(len);
  char* u   = url;

  strcpy(url, "corbaname:");
  u += 10;

  strcpy(u, addr);
  u += addrlen;

  // If the string name is empty, we're done
  if (*sn == '\0') {
    *u = '\0';
    return url;
  }
  *u++ = '#';

  for (c=sn; *c; c++) {
    if (validKeyChar(*c))
      *u++ = *c;
    else {
      // Construct a hex escape
      *u++ = '%';
      int v = (*c & 0xf0) >> 4;
      if (v < 10)
	*u++ = '0' + v;
      else
	*u++ = 'a' + v - 10;
      v = *c & 0xf;
      if (v < 10)
	*u++ = '0' + v;
      else
	*u++ = 'a' + v - 10;
    }
  }
  *u = '\0';
  return url;
}

#endif // !(defined(__vxWorks__) && defined(__vxNames__))

/////////////////////////////////////////////////////////////////////////////
// initialiser
/////////////////////////////////////////////////////////////////////////////
static iorURIHandler       iorURIHandler_;
static corbalocURIHandler  corbalocURIHandler_;

#if !(defined(__vxWorks__) && defined(__vxNames__))
static corbanameURIHandler corbanameURIHandler_;
#endif // !(defined(__vxWorks__) && defined(__vxNames__))

// No need to register the initialiser to ORB_init unless attach () does
// something.
class omni_uri_initialiser : public omniInitialiser {
public:
  omni_uri_initialiser() {
    handlers.push_back(&iorURIHandler_);
    handlers.push_back(&corbalocURIHandler_);
#if !(defined(__vxWorks__) && defined(__vxNames__))
    handlers.push_back(&corbanameURIHandler_);
#endif // !(defined(__vxWorks__) && defined(__vxNames__))  
  }
  void attach() {}
  void detach() {}
};

static omni_uri_initialiser initialiser;
omniInitialiser& omni_uri_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
