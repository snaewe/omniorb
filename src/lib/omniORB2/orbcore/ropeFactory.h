// -*- Mode: C++; -*-
//                            Package   : omniORB2
// ropeFactory.cc             Created on: 11/7/96
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
  Revision 1.2  1997/05/06 15:27:39  sll
  Public release.

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
