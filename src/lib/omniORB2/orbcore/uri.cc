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

// $Id$
// $Log$
// Revision 1.1.2.9  2000/11/21 12:24:44  dpg1
// corbaloc URIs accept an empty object address, to mean localhost on the
// default port.
//
// Revision 1.1.2.8  2000/11/21 10:59:27  dpg1
// Poperly throw INV_OBJREF for object references containing no profiles
// we understand.
//
// Revision 1.1.2.7  2000/09/13 11:45:05  dpg1
// Minor cut-and-paste error in URI handling meant that ior: was not
// accepted as a URI format.
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
#include <omniORB3/CORBA.h>
#include <omniORB3/Naming.hh>
#include <initialiser.h>
#include <ropeFactory.h>
#include <tcpSocket.h>
#include <exceptiondefs.h>
#include <omniORB3/omniURI.h>
#include <initRefs.h>
#include <ctype.h>

#define MAX_STRING_TO_OBJECT_CYCLES 10

#define MINOR_BAD_SCHEME_NAME          (CORBA::OMGVMCID | 7)
#define MINOR_BAD_ADDRESS              (CORBA::OMGVMCID | 8)
#define MINOR_BAD_SCHEME_SPECIFIC_PART (CORBA::OMGVMCID | 9)
#define MINOR_OTHER                    (CORBA::OMGVMCID | 10)


struct URIHandlerList {
  URIHandlerList(omniURI::URIHandler* h) : handler(h), next(0) {}

  omniURI::URIHandler* handler;
  URIHandlerList*      next;
};

static URIHandlerList*  the_URIHandlerList = 0;
static URIHandlerList*  the_URIHandlerTail = 0;
static omni_tracedmutex handler_lock;



char*
omniURI::objectToString(CORBA::Object_ptr obj)
{
  if (obj && obj->_NP_is_pseudo()) {
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
  }

  omniObjRef* objref = obj ? obj->_PR_getobj() : 0;

  if (objref) {
    return (char*) IOP::iorToEncapStr((const CORBA::Char*)
				      objref->_mostDerivedRepoId(),
				      objref->_iopProfiles());
  }
  else {
    IOP::TaggedProfileList p;
    return (char*) IOP::iorToEncapStr((const CORBA::Char*)"", &p);
  }
}


CORBA::Object_ptr
omniURI::stringToObject(const char* uri, unsigned int cycles)
{
  if (!uri) OMNIORB_THROW(BAD_PARAM, MINOR_OTHER, CORBA::COMPLETED_NO);

  if (cycles > MAX_STRING_TO_OBJECT_CYCLES) {
    if (omniORB::trace(1)) {
      omniORB::logger l;
      l << "string_to_object reached recursion limit processing `"
	<< uri << "'.\n";
    }
    OMNIORB_THROW(BAD_PARAM, MINOR_OTHER, CORBA::COMPLETED_NO);
  }

  URIHandler* handler = 0;
  {
    omni_tracedmutex_lock sync(handler_lock);
    for (URIHandlerList* hl = the_URIHandlerList; hl; hl = hl->next) {
      if (hl->handler->supports(uri)) {
	handler = hl->handler;
	break;
      }
    }
  }
  if (handler)
    return handler->toObject(uri, cycles);

  OMNIORB_THROW(BAD_PARAM, MINOR_BAD_SCHEME_NAME, CORBA::COMPLETED_NO);
  return 0;
}


CORBA::Boolean
omniURI::uriSyntaxIsValid(const char* uri)
{
  if (!uri) return 0;

  URIHandler* handler = 0;
  {
    omni_tracedmutex_lock sync(handler_lock);
    for (URIHandlerList* hl = the_URIHandlerList; hl; hl = hl->next) {
      if (hl->handler->supports(uri)) {
	handler = hl->handler;
	break;
      }
    }
  }
  if (handler) return handler->syntaxIsValid(uri);
  return 0;
}

void
omniURI::addURIHandler(URIHandler* handler)
{
  omni_tracedmutex_lock sync(handler_lock);

  URIHandlerList* entry = new URIHandlerList(handler);
  if (!entry) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);

  if (the_URIHandlerList) {
    the_URIHandlerTail->next = entry;
    the_URIHandlerTail       = entry;
  }
  else {
    the_URIHandlerList = entry;
    the_URIHandlerTail = entry;
  }
}


static void
deleteURIHandlers()
{
  omni_tracedmutex_lock sync(handler_lock);

  URIHandlerList *current, *next;

  for (current = the_URIHandlerList; current; current = next) {
    next = current->next;
    delete current->handler;
    delete current;
  }
  the_URIHandlerList = 0;
  the_URIHandlerTail = 0;
}


/////////////////////////////////////////////////////////////////////////////
// IOR: format
/////////////////////////////////////////////////////////////////////////////

class iorURIHandler : public omniURI::URIHandler {
public:
  CORBA::Boolean    supports     (const char* uri);
  CORBA::Object_ptr toObject     (const char* uri, unsigned int cycles);
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
iorURIHandler::toObject(const char* sior, unsigned int cycles)
{
  _CORBA_Char* repoId; // Instead of using CORBA::Char because
                       // some brain-dead compilers does not realise
                       // _CORBA_Char and CORBA::Char are the same type
                       // and would not take a CORBA::Char* as an
                       // argument that requires a _CORBA_Char*&.
  IOP::TaggedProfileList* profiles;

  IOP::EncapStrToIor((const CORBA::Char*) sior,
		     repoId,
		     profiles);

  if( *repoId == '\0' && profiles->length() == 0 ) {
    // nil object reference
    delete[] repoId;
    delete profiles;
    return CORBA::Object::_nil();
  }
  omniObjRef* objref = omni::createObjRef((const char*)repoId, 
					  CORBA::Object::_PD_repoId,
					  profiles, 1, 0);
  delete[] repoId;

  if (!objref)
    OMNIORB_THROW(INV_OBJREF, 0, CORBA::COMPLETED_NO);

  return (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}

CORBA::Boolean
iorURIHandler::syntaxIsValid(const char* sior)
{
  try {
    _CORBA_Char* repoId; // Instead of using CORBA::Char because
                       // some brain-dead compilers does not realise
                       // _CORBA_Char and CORBA::Char are the same type
                       // and would not take a CORBA::Char* as an
                       // argument that requires a _CORBA_Char*&.
    IOP::TaggedProfileList* profiles;
    IOP::EncapStrToIor((const CORBA::Char*)sior,
		       repoId,
		       profiles);
    delete [] repoId;
    delete profiles;
    return 1;
  }
  catch (...) {
  }
  return 0;
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

    static ObjAddr* parse(const char*& c);

    enum AddrKind { rir, iiop };
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
    OMNIORB_THROW(BAD_PARAM, MINOR_BAD_SCHEME_SPECIFIC_PART,
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
  if (!strncmp(c, "rir:", 4)) {
    c += 4;
    return new corbalocURIHandler::RirObjAddr(c);
  }
  OMNIORB_THROW(BAD_PARAM, MINOR_BAD_SCHEME_NAME, CORBA::COMPLETED_NO);
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
    if (!isdigit(*p)) OMNIORB_THROW(BAD_PARAM, MINOR_BAD_SCHEME_SPECIFIC_PART,
				    CORBA::COMPLETED_NO);
    l = strtoul(p, (char**)&p, 10);
    if (l > 0xff)     OMNIORB_THROW(BAD_PARAM, MINOR_BAD_SCHEME_SPECIFIC_PART,
				    CORBA::COMPLETED_NO);
    majver = l;

    // Minor
    if (*p++ != '.')  OMNIORB_THROW(BAD_PARAM, MINOR_BAD_SCHEME_SPECIFIC_PART,
				    CORBA::COMPLETED_NO);
    if (!isdigit(*p)) OMNIORB_THROW(BAD_PARAM, MINOR_BAD_SCHEME_SPECIFIC_PART,
				    CORBA::COMPLETED_NO);
    l = strtoul(p, (char**)&p, 10);
    if (l > 0xff)     OMNIORB_THROW(BAD_PARAM, MINOR_BAD_SCHEME_SPECIFIC_PART,
				    CORBA::COMPLETED_NO);
    minver = l;

    if (*p != '@')    OMNIORB_THROW(BAD_PARAM, MINOR_BAD_SCHEME_SPECIFIC_PART,
				    CORBA::COMPLETED_NO);
    c = p + 1;
  }
  else {
    majver = 1;
    minver = 0;
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
			    MINOR_BAD_SCHEME_SPECIFIC_PART,
			    CORBA::COMPLETED_NO);

  host_ = CORBA::string_alloc(1 + p - c);
  char* h = (char*)host_;
  if (!h) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);

  for (; c != p; c++, h++) *h = *c;
  *h = '\0';

  if (*c == ':') {
    // Port number follows
    c++;
    if (isdigit(*c)) {
      unsigned long l;
      l = strtoul(c, (char**)&c, 10);
      if (l > 0xffff) OMNIORB_THROW(BAD_PARAM, MINOR_BAD_ADDRESS,
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
  if (!key) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
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
	OMNIORB_THROW(BAD_PARAM, MINOR_OTHER, CORBA::COMPLETED_NO);
      }
      c++;
      if      (*c >= '0' && *c <= '9') *k |= (*c - '0');
      else if (*c >= 'A' && *c <= 'F') *k |= (*c - 'A' + 10);
      else if (*c >= 'a' && *c <= 'f') *k |= (*c - 'a' + 10);
      else {
	CORBA::string_free(key);
	OMNIORB_THROW(BAD_PARAM, MINOR_OTHER, CORBA::COMPLETED_NO);
      }
    }
    else {
      CORBA::string_free(key);
      OMNIORB_THROW(BAD_PARAM, MINOR_OTHER, CORBA::COMPLETED_NO);
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
  if (!addr) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
  if (addr->kind() == ObjAddr::rir) is_rir_ = 1;
  addrList_.append(addr);

  while (*c == ',') {
    c++;
    addr_count_++;
    addr = ObjAddr::parse(c);
    if (!addr) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
    if (addr->kind() == ObjAddr::rir) is_rir_ = 1;
    addrList_.append(addr);
  }
  if (is_rir_ && (addr_count_ != 1))
    OMNIORB_THROW(BAD_PARAM, MINOR_OTHER, CORBA::COMPLETED_NO);

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
    OMNIORB_THROW(BAD_PARAM, MINOR_OTHER, CORBA::COMPLETED_NO);
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
      OMNIORB_THROW(BAD_PARAM, MINOR_OTHER, CORBA::COMPLETED_NO);
    }
  }
  else {
    // Protocols other than rir
    const ropeFactoryType* iiopFactory;
    {
      ropeFactory_iterator next(globalOutgoingRopeFactories);
      const ropeFactory* f;
      while ((f = next())) {
	iiopFactory = f->getType();
	if (iiopFactory->is_IOPprofileId(IOP::TAG_INTERNET_IOP))
	  break;
      }
      if (!f) OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
    }
    IOP::TaggedProfileList* profiles =
      new IOP::TaggedProfileList(parsed.addr_count_);

    if (!profiles) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);

    profiles->length(parsed.addr_count_);

    int i;
    ObjAddr* addr;
    for (i=0, addr = parsed.addrList_.head_; addr; addr = addr->next_, i++) {
      switch (addr->kind()) {
      case ObjAddr::iiop:
	{
	  IiopObjAddr* iaddr = (IiopObjAddr*)addr;
	  tcpSocketEndpoint ep((CORBA::Char*)iaddr->host(), iaddr->port());

	  iiopFactory->
            encodeIOPprofile((Endpoint*)&ep,
			     (const CORBA::Octet*)(const char*)parsed.key_,
			     parsed.key_size_,
			     (*profiles)[i]);
	}
	break;
      default:
	OMNIORB_ASSERT(0);
      }
    }
    omniObjRef* objref = omni::createObjRef("", CORBA::Object::_PD_repoId,
					    profiles, 1, 0);
    OMNIORB_ASSERT(objref);

    return (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
  }
  OMNIORB_ASSERT(0);
  return 0;
}



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
		  MINOR_BAD_SCHEME_SPECIFIC_PART,
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
		  MINOR_BAD_SCHEME_SPECIFIC_PART,
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
		    MINOR_BAD_SCHEME_SPECIFIC_PART,
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
    OMNIORB_THROW(BAD_PARAM, MINOR_OTHER, CORBA::COMPLETED_NO);
  }
  catch (CosNaming::NamingContext::CannotProceed& ex) {
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "string_to_object received a CannotProceed exception trying to "
	<< "resolve `" << (const char*)sname << "' from naming service\n";
    }
    OMNIORB_THROW(BAD_PARAM, MINOR_OTHER, CORBA::COMPLETED_NO);
  }
  catch (CosNaming::NamingContext::InvalidName& ex) {
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "string_to_object received an InvalidName exception trying to "
	<< "resolve `" << (const char*)sname << "' from naming service\n";
    }
    OMNIORB_THROW(BAD_PARAM, MINOR_OTHER, CORBA::COMPLETED_NO);
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
    CORBA::String_var   sname = unescapeKey(c, key_size);
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

  if (!sname)         OMNIORB_THROW(BAD_PARAM,0,CORBA::COMPLETED_NO);
  if (*sname == '\0') throw CosNaming::NamingContext::InvalidName();

  unsigned int      i, j;
  unsigned int      component = 0;
  unsigned int      len       = strlen(sname);
  char*             bufp      = CORBA::string_alloc(len+1);
  if (!bufp) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
  CORBA::String_var buf(bufp);

  enum { s_id, s_kind } state = s_id;

  CosNaming::Name*    namep = new CosNaming::Name;
  if (!namep) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
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


/////////////////////////////////////////////////////////////////////////////
// initialiser
/////////////////////////////////////////////////////////////////////////////

class omni_uri_initialiser : public omniInitialiser {
public:
  void attach() {
    omniURI::addURIHandler(new iorURIHandler);
    omniURI::addURIHandler(new corbalocURIHandler);
    omniURI::addURIHandler(new corbanameURIHandler);
  }
  void detach() {
    deleteURIHandlers();
  }
};

static omni_uri_initialiser initialiser;
omniInitialiser& omni_uri_initialiser_ = initialiser;
