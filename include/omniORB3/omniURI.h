// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniURI.h                  Created on: 2000/04/03
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

#include <iostream.h>

// $Id$
// $Log$
// Revision 1.2  2000/07/04 15:23:32  dpg1
// Merge from omni3_develop.
//
// Revision 1.1.2.1  2000/04/27 10:37:12  dpg1
// Interoperable Naming Service
//
// All URI related function definitions.
//

#ifndef _omniURI_h_
#define _omniURI_h_


#include <omniORB3/CORBA.h>
#include <omniORB3/Naming.hh>


class omniURI {
public:

  // The omniURI class contains all functions which manipulate object
  // URIs, and convert them to-and-from CORBA::Objects.

  static char* objectToString(CORBA::Object_ptr obj);
  // Return a stringified IOR for the given object reference.
  //  Does not throw any exceptions.

  static CORBA::Object_ptr stringToObject(const char*  uri,
					  unsigned int cycles = 0);
  // Converts the given URI to an object reference. Currently supports
  // IOR:, corbaloc: and corbaname: URIs.
  //
  // cycles is used to count recursive calls to stringToObject, and
  // bail out if we loop too much.
  //
  // Throws CORBA::MARSHAL and CORBA::BAD_PARAM

  static CORBA::Boolean uriSyntaxIsValid(const char* uri);
  // Return true if the given URI is syntactically valid, false
  // otherwise.
  //  Does not throw any exceptions.


  // URIs are parsed and validated by objects derived from URIHandler
  class URIHandler {
  public:
    virtual CORBA::Boolean supports(const char* uri) = 0;
    // Returns true if the handler can parse the URI, false otherwise
    //  Does not throw any exceptions.

    virtual CORBA::Object_ptr toObject(const char* uri,
				       unsigned int cycles) = 0;
    // Convert the given URI to an object reference. If the processing
    // involves a (potential) recursive call to stringToObject(),
    // cycles should be incremented.
    //  Throws CORBA system exceptions

    virtual CORBA::Boolean syntaxIsValid(const char* uri) = 0;
    // Return true if the URI is syntactically valid.
    //  Does not throw any exceptions.
  };

  static void addURIHandler(URIHandler* handler);
  // Add a URI handler.


  // The following functions implement the stringified name operations
  // of CosNaming::NamingContextExt. They are available here to avoid
  // the overhead of remote calls just to do some string bashing.

  static CosNaming::Name* stringToName(const char* sname);
  // Convert a stringified CosNaming::Name into a CosNaming::Name. The
  // caller is responsible for freeing it.

  static char* nameToString(const CosNaming::Name& name);
  // Convert the CosNaming::Name into a stringified name. Throws
  // CosNaming::NamingContext::InvalidName if the name sequence has
  // zero length.

  static char* addrAndNameToURI(const char* addr, const char* sn);
  // Convert the given address and stringified name into a corbaname:
  // URI. Throws CosNaming::NamingContextExt::InvalidAddress if the
  // address syntax is invalid; CosNaming::NamingContext::InvalidName
  // if the name syntax is invalid. It does not check if the name
  // actually exists in the specified naming service.
};


#endif // _omniURI_h_
