// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopEndpoint.cc            Created on: 29 Mar 2001
//                            Author    : Sai Lai Lo (sll)
//
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
  Revision 1.1.2.5  2001/07/26 16:37:21  dpg1
  Make sure static initialisers always run.

  Revision 1.1.2.4  2001/07/25 14:22:02  dpg1
  Same old static initialiser problem, this time with transports.

  Revision 1.1.2.3  2001/07/13 15:13:32  sll
  giopConnection is now reference counted.

  Revision 1.1.2.2  2001/06/13 20:13:15  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.1.2.1  2001/04/18 18:10:51  sll
  Big checkin with the brand new internal APIs.


*/

#include <omniORB4/CORBA.h>
#include <omniORB4/omniTransport.h>
#include <omniORB4/giopEndpoint.h>
#include <omniORB4/linkHacks.h>
#include <initialiser.h>

//
// Make sure built-in transports are always linked

OMNI_FORCE_LINK(tcpTransportImpl);


OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
static giopTransportImpl*& implHead() {
  static giopTransportImpl* head_ = 0;
  return head_;
}


////////////////////////////////////////////////////////////////////////
static
giopTransportImpl*
matchType(const char* uri,const char*& param,CORBA::Boolean allowShortHand) {

  const char* protocol = uri;
  param = 0;

  CORBA::String_var expanded;
  if (allowShortHand) {
    const char* p1 = strchr(uri,':');
    // Either the uri is of the form ":xxxx:xxxx" or "xxx::xxxx" do we
    // have to expand.
    if (p1 && (uri == p1 || *(p1+1) == ':')) {
      const char* p2 = strchr(p1+1,':');
      if (p2) {
	CORBA::ULong len = (p1 - uri) + (p2 - p1) + sizeof("giop:tcp");
	expanded = CORBA::string_alloc(len);
	if (uri == p1) {
	  strcpy(expanded,"giop");
	}
	else {
	  strncpy(expanded,uri,(p1-uri));
	  ((char*) expanded)[p1-uri] = '\0';
	}
	if (p2 == p1+1) {
	  strcat(expanded,":tcp");
	}
	else {
	  len = strlen(expanded);
	  strncat(expanded,p1,(p2-p1));
	  ((char*)expanded)[len+(p2-p1)] = '\0';
	}
	protocol = expanded;
	param = p2 + 1;
      }
    }
  }

  giopTransportImpl* impl = implHead();

  while (impl) {
    int len = strlen(impl->type);
    if (strncmp(protocol,impl->type,len) == 0) {
      if (param == 0) {
	param = uri + len + 1;
	if (*param == '\0' || *(param-1) != ':') impl = 0;
      }
      break;
    }
    impl = impl->next;
  }
  return impl;
}


////////////////////////////////////////////////////////////////////////
giopAddress*
giopAddress::str2Address(const char* address) {

  const char* param;
  giopTransportImpl* impl = matchType(address,param,1);
  if (impl) {
    return impl->toAddress(param);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
giopEndpoint*
giopEndpoint::str2Endpoint(const char* endpoint) {

  const char* param;
  giopTransportImpl* impl = matchType(endpoint,param,1);
  if (impl) {
    return impl->toEndpoint(param);
  }
  return 0;

}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopEndpoint::strIsValidEndpoint(const char* endpoint) {

  const char* param;
  giopTransportImpl* impl = matchType(endpoint,param,0);
  if (impl) {
    return impl->isValid(param);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopEndpoint::addToIOR(const char* endpoint) {

  const char* param;
  giopTransportImpl* impl = matchType(endpoint,param,0);
  if (impl) {
    return impl->addToIOR(param);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
giopTransportImpl::giopTransportImpl(const char* t) : type(t), next(0) {
  giopTransportImpl** pp = &implHead();
  while (*pp) pp = &((*pp)->next);
  *pp = this;
}

////////////////////////////////////////////////////////////////////////
giopTransportImpl::~giopTransportImpl() {
}

////////////////////////////////////////////////////////////////////////
void
giopConnection::incrRefCount() {

  ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);
  OMNIORB_ASSERT(pd_refcount >= 0);
  pd_refcount++;
}

////////////////////////////////////////////////////////////////////////
int
giopConnection::decrRefCount(CORBA::Boolean forced) {
  
  if (!forced) {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,1);
  }
  int rc = --pd_refcount;
  OMNIORB_ASSERT(rc >= 0);
  if (rc == 0)
    delete this;
  return rc;
}


OMNI_NAMESPACE_END(omni)
