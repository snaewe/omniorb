// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixEndpoint.cc            Created on: 6 Aug 2001
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
  Revision 1.1.4.6  2006/04/28 18:40:46  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.5  2006/04/09 19:52:29  dgrisby
  More IPv6, endPointPublish parameter.

  Revision 1.1.4.4  2005/08/03 09:43:51  dgrisby
  Make sure accept() never blocks.

  Revision 1.1.4.3  2005/01/13 21:10:16  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.2  2005/01/06 23:10:59  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:01:57  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.9  2002/04/16 12:44:27  dpg1
  Fix SSL accept bug, clean up logging.

  Revision 1.1.2.8  2002/03/13 16:05:40  dpg1
  Transport shutdown fixes. Reference count SocketCollections to avoid
  connections using them after they are deleted. Properly close
  connections when in thread pool mode.

  Revision 1.1.2.7  2002/01/15 16:38:14  dpg1
  On the road to autoconf. Dependencies refactored, configure.ac
  written. No makefiles yet.

  Revision 1.1.2.6  2001/11/28 20:33:43  dpg1
  Minor Unix transport bugs.

  Revision 1.1.2.5  2001/08/23 10:11:16  sll
  Use AF_UNIX if AF_LOCAL is not defined.

  Revision 1.1.2.4  2001/08/17 17:12:42  sll
  Modularise ORB configuration parameters.

  Revision 1.1.2.3  2001/08/08 15:58:17  sll
  Set up the socket with the permission mode set in
  omniORB::unixTransportPermission.

  Revision 1.1.2.2  2001/08/07 15:42:17  sll
  Make unix domain connections distinguishable on both the server and client
  side.

  Revision 1.1.2.1  2001/08/06 15:47:44  sll
  Added support to use the unix domain socket as the local transport.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <orbParameters.h>
#include <SocketCollection.h>
#include <objectAdapter.h>
#include <unix/unixConnection.h>
#include <unix/unixAddress.h>
#include <unix/unixEndpoint.h>
#include <sys/un.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixEndpoint);

#ifndef AF_LOCAL
#  ifdef  AF_UNIX
#    define AF_LOCAL AF_UNIX
#  endif
#endif


OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
unixEndpoint::unixEndpoint(const char* filename) :
  SocketHolder(RC_INVALID_SOCKET),
  pd_new_conn_socket(RC_INVALID_SOCKET), pd_callback_func(0),
  pd_callback_cookie(0),
  pd_poked(0)
{
  pd_filename = filename;
}

/////////////////////////////////////////////////////////////////////////
unixEndpoint::~unixEndpoint() {
  if (pd_socket != RC_INVALID_SOCKET) {
    unlink(pd_filename);
    CLOSESOCKET(pd_socket);
    pd_socket = RC_INVALID_SOCKET;
  }
}

/////////////////////////////////////////////////////////////////////////
const char*
unixEndpoint::type() const {
  return "giop:unix";
}

/////////////////////////////////////////////////////////////////////////
const char*
unixEndpoint::address() const {
  return pd_addresses[0];
}

/////////////////////////////////////////////////////////////////////////
const _CORBA_Unbounded_Sequence_String*
unixEndpoint::addresses() const {
  return &pd_addresses;
}

/////////////////////////////////////////////////////////////////////////
static CORBA::Boolean
publish_one(const char*    	     publish_spec,
	    const char*    	     ep,
	    CORBA::Boolean 	     no_publish,
	    orbServer::EndpointList& published_eps)
{
  OMNIORB_ASSERT(!strncmp(ep, "giop:unix:", 10));

  CORBA::String_var to_add;

  if (!strncmp(publish_spec, "giop:unix:", 10)) {
    const char* file = publish_spec + 10;
    if (strlen(file) == 0)
      to_add = ep;
    else
      to_add = publish_spec;
  }
  else if (no_publish) {
    // Suppress all the other options
    return 0;
  }
  else if (omni::strMatch(publish_spec, "addr")) {
    to_add = ep;
  }
  else {
    // Don't understand the spec.
    return 0;
  }

  if (!omniObjAdapter::endpointInList(to_add, published_eps)) {
    if (omniORB::trace(20)) {
      omniORB::logger l;
      l << "Publish endpoint '" << to_add << "'\n";
    }
    giopEndpoint::addToIOR(to_add);
    published_eps.length(published_eps.length() + 1);
    published_eps[published_eps.length() - 1] = to_add._retn();
  }
  return 1;
}


CORBA::Boolean
unixEndpoint::publish(const orbServer::PublishSpecs& publish_specs,
		      CORBA::Boolean 	      	     all_specs,
		      CORBA::Boolean 	      	     all_eps,
		      orbServer::EndpointList& 	     published_eps)
{
  CORBA::ULong i, j;
  CORBA::Boolean result = 0;

  for (i=0; i < pd_addresses.length(); ++i) {

    CORBA::Boolean ok = 0;
    
    for (j=0; j < publish_specs.length(); ++j) {
      if (omniORB::trace(25)) {
	omniORB::logger l;
	l << "Try to publish '" << publish_specs[j]
	  << "' for endpoint " << pd_addresses[i] << "\n";
      }
      ok = publish_one(publish_specs[j], pd_addresses[i], no_publish(),
		       published_eps);
      result |= ok;

      if (ok && !all_specs)
	break;
    }
    if (result && !all_eps)
      break;
  }
  return result;
}


/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixEndpoint::Bind() {

  OMNIORB_ASSERT(pd_socket == RC_INVALID_SOCKET);

  if ((pd_socket = socket(AF_LOCAL,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return 0;
  }

  unlink(pd_filename);

  SocketSetCloseOnExec(pd_socket);

  struct sockaddr_un addr;

  memset((void*)&addr,0,sizeof(addr));
  addr.sun_family = AF_LOCAL;
  strncpy(addr.sun_path, pd_filename, sizeof(addr.sun_path) - 1);

  if (::bind(pd_socket,(struct sockaddr *)&addr,
	               sizeof(addr)) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }

  if (::chmod(pd_filename,orbParameters::unixTransportPermission & 0777) < 0) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Error: cannot change permission of " << pd_filename
	  << " to " << (orbParameters::unixTransportPermission & 0777) << "\n";
    }
    CLOSESOCKET(pd_socket);
    return 0;
  }

  if (listen(pd_socket,5) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_socket);
    return 0;
  }

  pd_addresses.length(1);
  pd_addresses[0] = unixConnection::unToString(pd_filename);

  // Never block in accept
  SocketSetnonblocking(pd_socket);

  // Add the socket to our SocketCollection.
  addSocket(this);

  return 1;
}

/////////////////////////////////////////////////////////////////////////
void
unixEndpoint::Poke() {

  unixAddress* target = new unixAddress(pd_filename);
  pd_poked = 1;
  if (!target->Poke()) {
    if (omniORB::trace(5)) {
      omniORB::logger log;
      log << "Warning: fail to connect to myself ("
	  << (const char*) pd_addresses[0] << ") via unix socket.\n";
    }
  }
  delete target;
}

/////////////////////////////////////////////////////////////////////////
void
unixEndpoint::Shutdown() {
  SHUTDOWNSOCKET(pd_socket);
  removeSocket(this);
  decrRefCount();
  omniORB::logs(20, "Unix endpoint shut down.");
}

/////////////////////////////////////////////////////////////////////////
giopConnection*
unixEndpoint::AcceptAndMonitor(giopConnection::notifyReadable_t func,
			      void* cookie) {

  OMNIORB_ASSERT(pd_socket != RC_INVALID_SOCKET);

  pd_callback_func = func;
  pd_callback_cookie = cookie;
  setSelectable(1,0,0);

  while (1) {
    pd_new_conn_socket = RC_INVALID_SOCKET;
    if (!Select()) break;
    if (pd_new_conn_socket != RC_INVALID_SOCKET) {
      return  new unixConnection(pd_new_conn_socket,this,pd_filename,0);
    }
    if (pd_poked)
      return 0;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixEndpoint::notifyReadable(SocketHolder* sh) {

  if (sh == (SocketHolder*)this) {
    // New connection
    SocketHandle_t sock;
again:
    sock = ::accept(pd_socket,0,0);
    if (sock == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EBADF) {
        omniORB::logs(20, "accept() returned EBADF, unable to continue");
        return 0;
      }
      else if (ERRNO == RC_EINTR) {
        omniORB::logs(20, "accept() returned EINTR, trying again");
        goto again;
      }
      else if (ERRNO == RC_EAGAIN) {
        omniORB::logs(20, "accept() returned EAGAIN, will try later");
      }
      if (omniORB::trace(20)) {
        omniORB::logger log;
        log << "accept() failed with unknown error " << ERRNO << "\n";
      }
    }
    else {
      // On some platforms, the new socket inherits the non-blocking
      // setting from the listening socket, so we set it blocking here
      // just to be sure.
      SocketSetblocking(sock);

      pd_new_conn_socket = sock;
    }
    setSelectable(1,0,1);
    return 1;
  }
  else {
    // Existing connection
    pd_callback_func(pd_callback_cookie,(unixConnection*)sh);
    return 1;
  }
}

OMNI_NAMESPACE_END(omni)
