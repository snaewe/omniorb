// -*- Mode: C++; -*-
//                            Package   : omniORB2
// proxyFactory.h             Created on: 13/6/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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
//	*** PROPRIETORY INTERFACE ***
//

/*
 $Log$
 Revision 1.2.2.2  2000/09/27 17:03:57  sll
 Changed newObjRef signature to accept omniIOR* as an argument.

 Revision 1.2.2.1  2000/07/17 10:35:37  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

 Revision 1.3  2000/07/13 15:26:04  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.1.2.1  1999/09/24 09:51:52  djr
 Moved from omniORB2 + some new files.

 Revision 1.7  1999/01/07 18:40:32  djr
 Changes to support split of omniORB library into two.

 Revision 1.6  1998/04/07 19:56:44  sll
 Replace _OMNIORB2_NTDLL_ specification on class proxyObjectFactory with
 _OMNIORB_NTDLL_IMPORT on static member variables.

 * Revision 1.5  1997/05/06  16:10:05  sll
 * Public release.
 *
*/

#ifndef __OMNI_PROXYFACTORY_H__
#define __OMNI_PROXYFACTORY_H__


class omniLocalIdentity;
class omniObjRef;


class proxyObjectFactory {
public:
  virtual ~proxyObjectFactory();
  proxyObjectFactory(const char* repoId);
  // This constructor inserts this object into the list
  // of object factories.
  //  Assumes that <repoId> will remain valid for the
  // lifetime of this object.

  static void shutdown();
  // Frees resources -- only called on ORB shutdown.  Does not
  // release the pof's themselves, the stubs take care of that.

  static proxyObjectFactory* lookup(const char* repoId);

  inline const char* irRepoId() const { return pd_repoId; }
  // Returns the Interface Repository ID for proxies this
  // factory can instantiate.

  virtual omniObjRef* newObjRef(omniIOR* ior,
				omniIdentity* id, omniLocalIdentity* lid) = 0;
  // Returns a new object reference. Consumes <profiles>.

  virtual _CORBA_Boolean is_a(const char* base_repoId) const = 0;
  // Must return true if <base_repoId> is the interface
  // repository ID of a base interface. Need not recognise
  // CORBA::Object as a base interface.

private:
  const char* pd_repoId;
};


#endif // __OMNI_PROXYFACTORY_H__
