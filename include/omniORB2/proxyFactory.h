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
 Revision 1.6  1998/04/07 19:56:44  sll
 Replace _OMNIORB2_NTDLL_ specification on class proxyObjectFactory with
 _OMNIORB_NTDLL_IMPORT on static member variables.

 * Revision 1.5  1997/05/06  16:10:05  sll
 * Public release.
 *
*/

#ifndef __PROXYFACTORY_H__
#define __PROXYFACTORY_H__

class proxyObjectFactory_iterator;
class proxyObjectFactory {
public:
  proxyObjectFactory();
  virtual ~proxyObjectFactory();
  
  virtual const char *irRepoId() const = 0;
  // returns the Interface Repository ID.
       
  virtual CORBA::Object_ptr newProxyObject(Rope *r,
					   _CORBA_Octet *key,
					   size_t keysize,
					   IOP::TaggedProfileList *profiles,
					   _CORBA_Boolean release) = 0;
  // returns a new proxy object.

  virtual _CORBA_Boolean  is_a(const char *base_repoId) const = 0;
  // Return true if <base_repoId> is the interface repository ID of
  // a base interface.

  friend class proxyObjectFactory_iterator;
  static _OMNIORB_NTDLL_IMPORT proxyObjectFactory* proxyStubs;

private:
  proxyObjectFactory *pd_next;
};

class proxyObjectFactory_iterator {
public:
  proxyObjectFactory_iterator();
  ~proxyObjectFactory_iterator() {}
  proxyObjectFactory *operator() ();
private:
  proxyObjectFactory *pd_f;
};

#endif // __PROXYFACTORY_H__
