// -*- Mode: C++; -*-
//                            Package   : omniORB2
// objectManager.h            Created on: 30/9/97
//                            Author    : Sai Lai Lo (sll)
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
//	*** PROPRIETORY INTERFACE ***
// 

/*
 $Log$
 Revision 1.2  1999/03/11 16:25:55  djr
 Updated copyright notice

 Revision 1.1  1997/12/09 18:43:10  sll
 Initial revision

*/

#ifndef __OBJECTMANAGER_H__
#define __OBJECTMANAGER_H__

class omniObjectManager {
public:
  virtual ropeFactoryList* incomingRopeFactories() = 0;
  virtual Rope* defaultLoopBack() = 0;
  // Do not increment the reference count
  
  static omniObjectManager* root(CORBA::Boolean no_exception = 0) 
                                              throw(CORBA::OBJ_ADAPTER);
  // root()
  //    Returns the root object manager. If the object manager has not been
  //    initialised, throw a CORBA::OBJ_ADAPTER exception or returns 0 depends
  //    on whether no_exception is TRUE(1) or FALSE(0).
};

#endif // __OBJECTMANAGER_H__
