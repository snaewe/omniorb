// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpAddress.cc              Created on: 6 Aug 2001
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
  Revision 1.1.2.2  2001/08/07 15:42:17  sll
  Make unix domain connections distinguishable on both the server and client
  side.

  Revision 1.1.2.1  2001/08/06 15:47:44  sll
  Added support to use the unix domain socket as the local transport.


*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <unix/unixConnection.h>
#include <unix/unixAddress.h>
#include <stdio.h>
#include <omniORB4/linkHacks.h>
#include <sys/un.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixAddress);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
unixAddress::unixAddress(const char* filename) {

  pd_filename = (const char*) filename;
  pd_address_string = unixConnection::unToString(filename);
}

/////////////////////////////////////////////////////////////////////////
const char*
unixAddress::type() const {
  return "giop:unix";
}

/////////////////////////////////////////////////////////////////////////
const char*
unixAddress::address() const {
  return pd_address_string;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
unixAddress::duplicate() const {
  return new unixAddress(pd_filename);
}

/////////////////////////////////////////////////////////////////////////
giopActiveConnection*
unixAddress::Connect(unsigned long, unsigned long) const {


  struct sockaddr_un raddr;
  int  rc;
  SocketHandle_t sock;

  if ((sock = socket(AF_LOCAL,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return 0;
  }

  memset((void*)&raddr,0,sizeof(raddr));
  raddr.sun_family = AF_LOCAL;
  strncpy(raddr.sun_path, pd_filename, sizeof(raddr.sun_path) - 1);

  if (::connect(sock,(struct sockaddr *)&raddr,
                     sizeof(raddr)) == RC_SOCKET_ERROR) {
    CLOSESOCKET(sock);
    return 0;
  }
  return new unixActiveConnection(sock,pd_filename);
}


OMNI_NAMESPACE_END(omni)
