// -*- Mode: C++; -*-
//                            Package   : omniORB2
// ropeFactory.cc             Created on: 11/7/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//      
 
/*
  $Log$
  Revision 1.1  1997/01/08 17:26:29  sll
  Initial revision

  */

#ifndef __ROPEFACTORY_H__
#define __ROPEFACTORY_H__

class ropeFactory_iterator;
class ropeFactory {
public:
  ropeFactory();
  virtual ~ropeFactory();

  Rope *initIncoming(int &argc, char **argv,Anchor &a);
  void  incomingIsReady();
  Rope *initOutgoing(Anchor &a);

  friend ropeFactory_iterator;
private:
  ropeFactory *pd_next;
  rope *pd_rope;
};

class ropeFactory_iterator {
public:
  ropeFactory_iterator();
  ~ropeFactory_iterator() {}
  ropeFactory_iterator *operator() ();
private:
  ropeFactory *pd_f;
};

#endif // __ROPEFACTORY_H__
