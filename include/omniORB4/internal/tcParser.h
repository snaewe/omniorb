// -*- Mode: C++; -*-
//                            Package   : omniORB
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

#ifndef __TCPARSER_H__
#define __TCPARSER_H__

#include <omniORB4/tcDescriptor.h>

OMNI_NAMESPACE_BEGIN(omni)

class TypeCode_base;


_CORBA_MODULE tcParser
_CORBA_MODULE_BEG

// -=- Core functions

// All functions affect in/out pointers in streams

_CORBA_MODULE_FN
void copyStreamToStream(const CORBA::TypeCode_ptr tc,
			cdrStream& src,
			cdrStream& dest);
// Copy data from src stream to dest stream, using tc
// to control marshalling/unmarshalling.  Does not
// rewind or reset either stream.  tc must not be nil.
  
_CORBA_MODULE_FN
void copyTcDescriptorToStream(const CORBA::TypeCode_ptr tc,
			      const tcDescriptor &src,
			      cdrStream& dest);
// Read data from src tcDescriptor structure and marshals
// it, according to the supplied tc, into the stream.
// No (trappable) errors should occur, so no exceptions
// are thrown.
  
_CORBA_MODULE_FN
void copyStreamToTcDescriptor(const CORBA::TypeCode_ptr tc,
			      cdrStream& src,
			      tcDescriptor& dest);
// Read data from the stream, according to the tc, into
// the locations described by the supplied tcDescriptor
// Throws CORBA::MARSHAL on error.
  
_CORBA_MODULE_FN
void skip(const CORBA::TypeCode_ptr tc, cdrStream &s);
// Read and discard data of type <tc> from the stream.
// Throws a CORBA::MARSHAL exception if a marshalling error
// is encountered.
  
// -=- cdrMemoryStream helper functions
  
inline _CORBA_MODULE_FN
void copyStreamToMemStream_flush(const CORBA::TypeCode_ptr tc,
				 cdrStream& src,
				 cdrMemoryStream& dest) {
  dest.rewindPtrs();
  copyStreamToStream(tc, src, dest);
}
inline _CORBA_MODULE_FN
void copyTcDescriptorToMemStream_flush(const CORBA::TypeCode_ptr tc,
				       const tcDescriptor& src,
				       cdrMemoryStream &dest) {
  dest.rewindPtrs();
  copyTcDescriptorToStream(tc, src, dest);
} 
// Read data of type <tc> from a cdrStream or tcDescriptor to a memory
// stream but flush the memory stream first.
// Same exception behaviour as copyStreamToStream,
// copyTcDescriptorToStream functions.
  
inline _CORBA_MODULE_FN
void copyMemStreamToStream_rdonly(const CORBA::TypeCode_ptr tc,
				  const cdrMemoryStream& src,
				  cdrStream& dest) {
  cdrMemoryStream tmp_mbs(src, 1);
  copyStreamToStream(tc, tmp_mbs, dest);
}
inline _CORBA_MODULE_FN
void copyMemStreamToTcDescriptor_rdonly(const CORBA::TypeCode_ptr tc,
					const cdrMemoryStream& src,
					tcDescriptor& dest) {
  cdrMemoryStream tmp_mbs(src, 1);
  copyStreamToTcDescriptor(tc, tmp_mbs, dest);
}  
// Read data of type <tc> from a memory stream to a cdrStream
// or tcDescriptor.
// Data will be read through a read-only memory stream.
// Same exception behaviour as copyStreamToStream,
// copyStreamToTcDescriptor functions.
  
_CORBA_MODULE_END

OMNI_NAMESPACE_END(omni)

#endif  // __TCPARSER_H__







  
