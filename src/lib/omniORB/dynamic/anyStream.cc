// -*- Mode: C++; -*-
//                            Package   : omniORB
// anyStream.cc               Created on: 2004/06/22
//                            Author    : Duncan Grisby
//
//
//    Copyright (C) 2004 Apasphere Ltd.
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
//    cdrMemoryStream extension used by Anys.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/anyStream.h>

OMNI_USING_NAMESPACE(omni)

cdrAnyMemoryStream::cdrAnyMemoryStream() : cdrMemoryStream() {}

cdrAnyMemoryStream::cdrAnyMemoryStream(const cdrAnyMemoryStream& s,
				       CORBA::Boolean read_only)
  : cdrMemoryStream(s, read_only)
{
  cdrAnyMemoryStream* ns = OMNI_CONST_CAST(cdrAnyMemoryStream*, &s);

  omniTypedefs::ValueBaseSeq* seq = ns->pd_values.operator->();

  if (seq) {
    if (read_only) {
      // Use same sequence buffer
      pd_values = new omniTypedefs::ValueBaseSeq(seq->length(),
						 seq->length(),
						 seq->NP_data());
    }
    else {
      // Copy sequence
      pd_values = new omniTypedefs::ValueBaseSeq(*seq);
    }
  }
}

cdrAnyMemoryStream::cdrAnyMemoryStream(void* d, CORBA::Boolean release)
  : cdrMemoryStream(d)
{
  if (release)
    pd_readonly_and_external_buffer = 0;
}

cdrAnyMemoryStream& 
cdrAnyMemoryStream::operator=(const cdrMemoryStream& s)
{
  clearValueSeq();
  cdrMemoryStream::operator=(s);
  return *this;
}


cdrAnyMemoryStream::~cdrAnyMemoryStream() {}


void*
cdrAnyMemoryStream::ptrToClass(int* cptr)
{
  if (cptr == &cdrAnyMemoryStream::_classid) return (cdrAnyMemoryStream*)this;
  if (cptr == &cdrMemoryStream	 ::_classid) return (cdrMemoryStream*)	 this;
  if (cptr == &cdrStream      	 ::_classid) return (cdrStream*)      	 this;
  return 0;
}

int cdrAnyMemoryStream::_classid;
