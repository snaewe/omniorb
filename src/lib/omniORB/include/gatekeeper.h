// -*- Mode: C++; -*-
//                            Package   : omniORB2
// gatekeeper.h               Created on: 24/11/97
//                            Author    : Ben Harris (bjh)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//	

#ifndef __GATEKEEPER_H__
#define __GATEKEERER_H__

OMNI_NAMESPACE_BEGIN(omni)

//
// This is a first cut of an interface for plugging in different access 
// control policies to decide for the ORB whether to accept or refuse a 
// connection from clients.
//

class gateKeeper {
public:

  // version
  //   Returns the version string of the implementation.
  static const char* version();

  // checkConnect
  // Called in a server and whenever a new connection from a client is 
  // established. This is done before the server will accept any request from
  // that client. Returns 1 if the connection can be accepted.
  // Otherwise, returns 0 and the connection will be shutdown immediately.
  // For the moment, the only piece of information provided is the strand,
  // based on which the remote endpoint can be determined. This allows
  // host based access control to be exercised. In future, a richer interface
  // might be needed to provide more refined control.
  //
  // This function should be thread-safe.
  //
  static CORBA::Boolean checkConnect(Strand *s);

  // Access control policies are stored in allowFile and denyFile and are
  // read by the implementation of this interface. The content of these
  // files are implmenetation dependent.
  // The configuration option GATEKEEPER_ALLOWFILE and GATEKEEPER_DENYFILE
  // can be defined in the omniORB configuration file to identify the
  // pathname of these files.
  static char *&denyFile;
  static char *&allowFile;
};

OMNI_NAMESPACE_END(omni)

#endif
