// -*- Mode: C++; -*-
//                            Package   : omniORB2
// proxyFactory.h             Created on: 13/6/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
// 

/*
 $Log$
 Revision 1.1  1997/01/08 17:28:30  sll
 Initial revision

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

  friend proxyObjectFactory_iterator;

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
