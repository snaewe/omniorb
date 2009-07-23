// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcParser.h                 Created on: 8/1998
//                            Author1   : James Weatherall (jnw)
//                            Author2   : David Riddoch (djr)
//
//    Copyright (C) 2004      Apasphere Ltd
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
// The tcParser is used to maniuplate data described by TypeCodes.
//
// Functions are provided to copy from one stream to another, driven
// by the the TypeCode.
//

#ifndef __TCPARSER_H__
#define __TCPARSER_H__

#include <omniORB4/anyStream.h>

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
void skip(const CORBA::TypeCode_ptr tc, cdrStream &s);
// Read and discard data of type <tc> from the stream.
// Throws a CORBA::MARSHAL exception if a marshalling error
// is encountered.
  
// -=- cdrMemoryStream helper functions
  
inline _CORBA_MODULE_FN
void copyStreamToMemStream_flush(const CORBA::TypeCode_ptr tc,
				 cdrStream& src,
				 cdrAnyMemoryStream& dest) {
  dest.rewindPtrs();
  copyStreamToStream(tc, src, dest);
}
// Read data of type <tc> from a cdrStream but flush the memory
// stream first.
// Same exception behaviour as copyStreamToStream.
  
inline _CORBA_MODULE_FN
void copyMemStreamToStream_rdonly(const CORBA::TypeCode_ptr tc,
				  const cdrAnyMemoryStream& src,
				  cdrStream& dest) {
  cdrAnyMemoryStream tmp_mbs(src, 1);
  copyStreamToStream(tc, tmp_mbs, dest);
}
// Read data of type <tc> from a memory stream to a cdrStream.
// Data will be read through a read-only memory stream.
// Same exception behaviour as copyStreamToStream.

_CORBA_MODULE_END

OMNI_NAMESPACE_END(omni)

#endif  // __TCPARSER_H__







  
