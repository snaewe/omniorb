// -*- Mode: C++; -*-
//                            Package   : omniORB2
// anyP.cc                    Created on: 04/08/98
//                            Author    : James Weatherall (jnw)
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
//    Private class used to implement ORB-internal Any functionality
//    This is done to avoid changing the CORBA header every time Any changes.
//

#include "anyP.h"

OMNI_NAMESPACE_BEGIN(omni)

// Constructor/destructor


AnyP::AnyP(CORBA::TypeCode_ptr tc)
{
  pd_mbuf = new cdrMemoryStream();
  pd_releaseptr = 0;
  pd_parser = new tcParser(*pd_mbuf, tc);
  pd_cached_data_ptr = 0;
}


AnyP::AnyP(CORBA::TypeCode_ptr tc, void* value, CORBA::Boolean release)
{
  // Create a read-only membufferedstream to read from the supplied buffer
  pd_mbuf = new cdrMemoryStream(value);
  pd_dataptr = value;
  pd_releaseptr = release;
  pd_parser = new tcParser(*pd_mbuf, tc);
  pd_cached_data_ptr = 0;
}


AnyP::AnyP(const AnyP* existing)
{
  pd_mbuf = new cdrMemoryStream();
  pd_releaseptr = 0;
  pd_parser = new tcParser(*pd_mbuf, existing->pd_parser->getTC());
  existing->pd_mbuf->rewindInputPtr();
  try {
    pd_parser->copyFrom(*existing->pd_mbuf);
  }
  catch(CORBA::MARSHAL&) {
    pd_mbuf->rewindPtrs();
  }
  pd_cached_data_ptr = 0;
}

AnyP::~AnyP()
{
  if (pd_parser != 0)
    delete pd_parser;
  if (pd_mbuf != 0)
    delete pd_mbuf;
  if (pd_releaseptr)
    delete [] (char*) pd_dataptr;
  if (pd_cached_data_ptr != 0)
    pd_cached_data_destructor(pd_cached_data_ptr);
  pd_cached_data_ptr = 0;
}

OMNI_NAMESPACE_END(omni)
