// -*- Mode: C++; -*-
//                            Package   : omniORB2
// tcParser.h                 Created on: 8/1998
//                            Author1   : James Weatherall (jnw)
//                            Author2   : David Riddoch (djr)
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
// TypeCode-oriented data parser.
//
//  The tcParser class is initialised with a TypeCode and
// a cdrMemoryStream. The cdrMemoryStream is used to
// store data of type described by the associated TypeCode.
//
//  The operations <copyTo> and <copyFrom> are used to
// insert and extract the data from the cdrMemoryStream.
// Overloaded versions are provided to marshal the data
// into and out of cdrStream - this is
// used for (un)marshalling values of type Any.
//
//  In addition the data passed into and out of the internal
// cdrMemoryStream may be described by a tcDescriptor.
// The user of the tcParser will setup a tcDescriptor to
// describe where the data to be copied to/from the
// cdrMemoryStream is in memory. For simple types this is
// a pointer to the location in memory. For more complex
// types the tcDescriptor provides call-backs to provide
// additional information such as the length and data of
// a string, or to create a tcDescriptor for the members
// of a struct.
//

/*
  $Log$
  Revision 1.3.4.1  1999/09/15 20:18:21  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

*/

#ifndef __TCPARSER_H__
#define __TCPARSER_H__

#include <omniORB2/tcDescriptor.h>


class TypeCode_base;


class tcParser
{
public:
  tcParser(cdrMemoryStream& mbuf, CORBA::TypeCode_ptr tc)
    : pd_mbuf(mbuf), pd_tc(CORBA::TypeCode::_duplicate(tc)) {}
  // Pass in a cdrMemorystream to use and a TypeCode describing
  // how to arrange the data within it. The stream passed in MUST
  // exist for at least as long as this tcParser object. The
  // TypeCode will be duplicated by the tcParser for internal use
  // (and released by the destructor).
  //  <tc> must not be nil.

  void copyTo(cdrStream& buf, int rewind=1); 
  // Marshals the contents of the tcParser's mbuf into the
  // specified giopStream or a cdrMemoryStream, according to the
  // associated TypeCode. If <rewind> is true (default) then
  // the mbuf is rewound first.
  //  Throws CORBA::MARSHAL on error.

  void copyTo(tcDescriptor &tcdata, int rewind=1);
  // Loads the data in the internal buffer into the locations
  // described by the tcDescriptor structure.
  //  Throws CORBA::MARSHAL on error.


  void copyFrom(cdrStream& buf, int flush=1);
  // Marshals a value of the type given by the internal TypeCode
  // from the supplied stream into the internal buffer. If <flush>
  // is true (default) then it flushes the internal buffer first.
  //  Throws CORBA::MARSHAL on error.

  void copyFrom(tcDescriptor &tcdata, int flush=1);
  // Marshals a value of the type given by the internal
  // TypeCode, and described by <tcdata> into the internal
  // buffer. If <flush> is true (default) then it flushes
  // the internal buffer first.
  //  Should not ever fail - so no exceptions are thrown.

  CORBA::TypeCode_ptr getTC() const;
  // Return a pointer to the TC currently used to control
  // the behaviour of this parser.

  void replaceTC(CORBA::TypeCode_ptr tc);
  // If TypeCode::equivalent test on tc returns true when compared
  // with pd_tc, replace pd_tc with the value of tc.
  // Otherwise raises the BAD_TYPECODE exception

  static void skip(cdrStream&, CORBA::TypeCode_ptr tc);
  // Read and discard a value of type <tc> from the stream.
  // Throws a CORBA::MARSHAL exception if a marshalling error
  // is encountered.

private:
  
  // INTERNAL ROUTINES

  // appendItem
  // Adds an the data specified in the tcDescriptor to the
  // end of the mbuf, using the given typecode as a template.
  void appendItem(TypeCode_base *tc, tcDescriptor &tcdata);

  // appendSimpleItem
  // Used internally to add a simple (fixed size) datatype
  // to the buffer.
  void appendSimpleItem(CORBA::TCKind tck, tcDescriptor &tcdata);
  // fetchItem
  // Gets the datatype specified from the mbuf and places
  // it in the location specified by the tcDescriptor.
  void fetchItem(TypeCode_base *tc, tcDescriptor &tcdata);

  // fetchSimpleItem
  // Used internally to get a simple (fixed size) datatype
  // from the buffer.
  void fetchSimpleItem(CORBA::TCKind tck, tcDescriptor &tcdata);

  // FIELDS

  cdrMemoryStream& pd_mbuf;
  CORBA::TypeCode_var pd_tc;
};


#endif  // __TCPARSER_H__
