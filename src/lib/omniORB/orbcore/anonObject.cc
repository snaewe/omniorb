// -*- Mode: C++; -*-
//                            Package   : omniORB
// anonObject.cc              Created on: 26/2/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996, 1999 AT&T Research Cambridge
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
//    Implementation of an anonymous object (no compile-time knowledge
//    of the interface).
//      
 
/*
  $Log$
  Revision 1.2  2000/07/04 15:22:59  dpg1
  Merge from omni3_develop.

  Revision 1.1.2.2  1999/09/27 11:01:10  djr
  Modifications to logging.

  Revision 1.1.2.1  1999/09/22 14:26:41  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <anonObject.h>


//////////////////////////////////////////////////////////////////////
/////////////////////////// omniAnonObjRef ///////////////////////////
//////////////////////////////////////////////////////////////////////

void*
omniAnonObjRef::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( !strcmp(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


omniAnonObjRef::~omniAnonObjRef() {}

//////////////////////////////////////////////////////////////////////
///////////////////////// omniAnonObjRef_pof /////////////////////////
//////////////////////////////////////////////////////////////////////

omniAnonObjRef_pof::~omniAnonObjRef_pof() {}


omniObjRef*
omniAnonObjRef_pof::newObjRef(const char* mostDerivedTypeId,
			      IOP::TaggedProfileList* profiles,
			      omniIdentity* id, omniLocalIdentity* lid)
{
  return new omniAnonObjRef(mostDerivedTypeId, profiles, id, lid);
}


CORBA::Boolean
omniAnonObjRef_pof::is_a(const char* repoId) const
{
  OMNIORB_ASSERT(repoId);

  return 0;
}
