// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixTransportImpl.cc       Created on: 6 Aug 2001
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
  Revision 1.1.2.1  2001/08/06 15:47:45  sll
  Added support to use the unix domain socket as the local transport.

*/

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <objectAdapter.h>
#include <SocketCollection.h>
#include <unix/unixConnection.h>
#include <unix/unixAddress.h>
#include <unix/unixEndpoint.h>
#include <unix/unixTransportImpl.h>
#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(unixAddress);
OMNI_FORCE_LINK(unixConnection);
OMNI_FORCE_LINK(unixEndpoint);
OMNI_FORCE_LINK(unixActive);

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixTransportImpl);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
unixTransportImpl::unixTransportImpl() : giopTransportImpl("giop:unix") {
}

/////////////////////////////////////////////////////////////////////////
unixTransportImpl::~unixTransportImpl() {
}

/////////////////////////////////////////////////////////////////////////
giopEndpoint*
unixTransportImpl::toEndpoint(const char* param) {

  if (param) 
    return (giopEndpoint*)(new unixEndpoint(param));
  else
    return 0;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixTransportImpl::isValid(const char* param) {

  if (!param || strlen(param) == 0) return 0;
  return 1;
}


/////////////////////////////////////////////////////////////////////////
giopAddress*
unixTransportImpl::toAddress(const char* param) {

  if (param) {
    return (giopAddress*)(new unixAddress(param));
  }
  else {
    return 0;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixTransportImpl::addToIOR(const char* param) {

  if (param) {
    omniIOR::add_TAG_OMNIORB_UNIX_TRANS(param);
    return 1;
  }
  return 0;
}


const unixTransportImpl _the_unixTransportImpl;

OMNI_NAMESPACE_END(omni)
