// -*- Mode: C++; -*-
//                            Package   : omniORB
// remoteGroupIdentity.cc     Created on: 18/01/01
//                            Author    : Bob Gruber (reg)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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

/*
  $Log$
  Revision 1.1.2.1  2001/02/23 19:35:21  sll
  Merged interim FT client stuff.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <remoteGroupIdentity.h>
#include <exceptiondefs.h>

//////////////////////////////////////////////////////////////////////

omniRemoteGroupIdentity::omniRemoteGroupIdentity(omniIOR* ior)
{
  OMNIORB_ASSERT(0); // should not be called
}

void
omniRemoteGroupIdentity::dispatch(omniCallDescriptor& call_desc)
{
  OMNIORB_ASSERT(0); // should not be called
}

void
omniRemoteGroupIdentity::gainObjRef(omniObjRef*)
{
  OMNIORB_ASSERT(0); // should not be called
}


void
omniRemoteGroupIdentity::loseObjRef(omniObjRef*)
{
  OMNIORB_ASSERT(0); // should not be called
}


void
omniRemoteGroupIdentity::locateRequest()
{
  OMNIORB_ASSERT(0); // should not be called
}


omniRemoteGroupIdentity::~omniRemoteGroupIdentity()
{
  OMNIORB_ASSERT(0); // should not be called
}

