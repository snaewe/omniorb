// -*- Mode: C++; -*-
//                            Package   : omniORB
// anonObject.h               Created on: 26/2/99
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
  Revision 1.1.2.3  1999/10/16 13:22:52  djr
  Changes to support compiling on MSVC.

  Revision 1.1.2.2  1999/10/04 17:08:30  djr
  Some more fixes/MSVC work-arounds.

  Revision 1.1.2.1  1999/09/22 14:26:42  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __ANONOBJECT_H__
#define __ANONOBJECT_H__

#ifndef __OMNIOBJREF_H__
#include <omniORB3/omniObjRef.h>
#endif


//
// An omniAnonObjRef is used as a proxyObject when no proxyObjectFactory
// class for a give interface repository ID is found.
//  Of course, one can only use such an object as a CORBA::Object_ptr and
// pass it around as the type "Object" in IDL operations and attributes.
// See also the comments in omni::createObjRef().
//

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniAnonObjRef ///////////////////////////
//////////////////////////////////////////////////////////////////////

class omniAnonObjRef : public virtual omniObjRef,
		       public virtual CORBA::Object
{
public:
  inline omniAnonObjRef(const char* mostDerivedTypeId,
			IOP::TaggedProfileList* profiles,
			omniIdentity* id, omniLocalIdentity* lid)
  : omniObjRef(CORBA::Object::_PD_repoId, mostDerivedTypeId,
	       profiles, id, lid)
    { _PR_setobj(this); }

protected:
  virtual void* _ptrToObjRef(const char* repoId);
  virtual ~omniAnonObjRef();

private:
  omniAnonObjRef(const omniAnonObjRef&);
  omniAnonObjRef& operator = (const omniAnonObjRef&);
};

//////////////////////////////////////////////////////////////////////
///////////////////////// omniAnonObjRef_pof /////////////////////////
//////////////////////////////////////////////////////////////////////

class omniAnonObjRef_pof : public proxyObjectFactory {
public:
  virtual ~omniAnonObjRef_pof();
  inline omniAnonObjRef_pof()
    : proxyObjectFactory(CORBA::Object::_PD_repoId) {}

  virtual omniObjRef* newObjRef(const char* mostDerivedTypeId,
				IOP::TaggedProfileList* profiles,
				omniIdentity* id, omniLocalIdentity* lid);
  virtual CORBA::Boolean is_a(const char* base_repoId) const;
};


#endif
