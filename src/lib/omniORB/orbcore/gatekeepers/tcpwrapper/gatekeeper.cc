// -*- Mode: C++; -*-
//                            Package   : omniORB2
// gatekeeper.cc              Created on: 24/11/97
//                            Author    : Ben Harris (bjh)
//
//    Copyright (C) 1997-1999 AT&T Laboratories Cambridge
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
//	gatekeeper implementation using Wietse Venema's tcp_wrappers library.
//

#include <omniORB4/CORBA.h>
#include <giopStrand.h>
#include <libcWrapper.h>
#include <gatekeeper.h>
#include <tcp/tcpConnection.h>

extern "C" {
#ifndef __STDC__
#define __STDC__
#endif
#include <tcpd.h>
  int deny_severity;
  int allow_severity;
}

OMNI_NAMESPACE_BEGIN(omni)

const char*
gateKeeper::version()
{
  return (const char*)"tcpwrapGK 1.0 - based on tcp_wrappers_7.6 ";
}

CORBA::Boolean
gateKeeper::checkConnect(Strand *s) {
  // This code is stolen (heavily modified) from tcpd.c in
  // tcp_wrappers_7.6.  Hence:
  /************************************************************************
  * Copyright 1995 by Wietse Venema.  All rights reserved.  Some individual
  * files may be covered by other copyrights.
  *
  * This material was originally written and compiled by Wietse Venema at
  * Eindhoven University of Technology, The Netherlands, in 1990, 1991,
  * 1992, 1993, 1994 and 1995.
  *
  * Redistribution and use in source and binary forms are permitted
  * provided that this entire copyright notice is duplicated in all such
  * copies.
  *
  * This software is provided "as is" and without any expressed or implied
  * warranties, including, without limitation, the implied warranties of
  * merchantibility and fitness for any particular purpose.
  ************************************************************************/

  giopConnection* connection = ((giopStrand*)s)->connection;
  if (strncmp(connection->peeraddress(),"giop:tcp",sizeof("giop:tcp")))
    return 1;

  {
    // libwrap isn't MT-safe
    omni_mutex_lock l(LibcWrapper::non_reentrant);

    struct request_info request;
    /*
     * Find out the endpoint addresses of this conversation. Host name
     * lookups and double checks will be done on demand.
     */

    request_init(&request, RQ_DAEMON, (char *) "",
		 RQ_FILE, ((tcpConnection*)connection)->handle(), 0);
    fromhost(&request);
    /*
     * Check whether this host can access the service in argv[0]. The
     * access-control code invokes optional shell commands as specified in
     * the access-control tables.
     */
    if (!hosts_access(&request)) {
      // refuse just exits, which is no good.
      // refuse(&request);
      if (omniORB::traceLevel >= 1) {
	omniORB::logger log;
	log << "refused connect from " << eval_client(&request) 
	    << "\n";
      }
      // syslog(deny_severity, "refused connect from %s", eval_client(request));
      return 0;
      // What happens if I throw CORBA::NO_PERMISSION here?
    } else {
      // syslog(allow_severity, "connect from %s", eval_client(&request));
      if (omniORB::traceLevel >= 5) {
	omniORB::logger log;
	log << "connect from " << eval_client(&request) << "\n";
      }
      return 1;
    }
  }
}

char *&gateKeeper::denyFile = hosts_deny_table;
char *&gateKeeper::allowFile = hosts_allow_table;

OMNI_NAMESPACE_END(omni)
