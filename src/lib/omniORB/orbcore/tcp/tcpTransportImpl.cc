// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpTransportImpl.cc        Created on: 29 Mar 2001
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
  Revision 1.1.2.1  2001/04/18 18:10:43  sll
  Big checkin with the brand new internal APIs.

*/

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <objectAdapter.h>
#include <tcp/tcpConnection.h>
#include <tcp/tcpAddress.h>
#include <tcp/tcpEndpoint.h>
#include <tcp/tcpTransportImpl.h>

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
tcpTransportImpl::tcpTransportImpl() : giopTransportImpl("giop:tcp") {
}

/////////////////////////////////////////////////////////////////////////
tcpTransportImpl::~tcpTransportImpl() {
}

/////////////////////////////////////////////////////////////////////////
giopEndpoint*
tcpTransportImpl::toEndpoint(const char* param) {

  const char* p = index(param,':');
  if (!p) return 0;
  IIOP::Address address;
  if (param == p) {
    const char* hostname = getenv(OMNIORB_USEHOSTNAME_VAR);
    if (hostname) address.host = hostname;
  }
  else {
    address.host = CORBA::string_alloc(p-param);
    strncpy(address.host,param,p-param);
    address.host[p-param] = '\0';
  }
  if (*(++p) != '\0') {
    int v;
    if (sscanf(p,"%d",&v) != 1) return 0;
    if (v < 0 || v > 65536) return 0;
    address.port = v;
  }
  else {
    address.port = 0;
  }
  return (giopEndpoint*)(new tcpEndpoint(address));
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpTransportImpl::isValid(const char* param) {
  
  const char* p = index(param,':');
  if (!p || param == p || *p == '\0') return 0;
  int v;
  if (sscanf(p+1,"%d",&v) != 1) return 0;
  if (v < 0 || v > 65536) return 0;
  return 1;
}

/////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
parseAddress(const char* param, IIOP::Address& address) {

  const char* p = index(param,':');
  if (!p || param == p || *p == '\0') return 0;
  address.host = CORBA::string_alloc(p-param);
  strncpy(address.host,param,p-param);
  address.host[p-param] = '\0';
  ++p;
  int v;
  if (sscanf(p,"%d",&v) != 1) return 0;
  if (v < 0 || v > 65536) return 0;
  address.port = v;
  return 1;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
tcpTransportImpl::toAddress(const char* param) {

  IIOP::Address address;
  if (parseAddress(param,address)) {
    return (giopAddress*)(new tcpAddress(address));
  }
  else {
    return 0;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpTransportImpl::addToIOR(const char* param) {

  IIOP::Address address;
  if (parseAddress(param,address)) {
    omniIOR::add_IIOP_ADDRESS(address);
    return 1;
  }
  return 0;
}


const tcpTransportImpl _the_tcpTransportImpl;

OMNI_NAMESPACE_END(omni)
