// -*- Mode: C++; -*-
//                            Package   : omniORB2
// bootstrap_i.h              Created on: 20/08/98
//                            Author    : Sai-Lai Lo
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
  Revision 1.4.6.1  1999/09/22 14:26:24  djr
  Major rewrite of orbcore to support POA.

  Revision 1.4  1999/05/25 18:10:46  sll
  Now CORBA::ORB::ObjectIdList and CORBA_InitialReference::ObjIdList
  are different types.

  Revision 1.3  1999/03/11 16:25:51  djr
  Updated copyright notice

  Revision 1.2  1998/08/25 18:52:09  sll
  Added dummy marshalling functions in serviceRecord to make gcc-2.7.2 happy.

  Revision 1.1  1998/08/21 19:28:15  sll
  Initial revision

*/


#ifndef __BOOTSTRAP_I_H__
#define __BOOTSTRAP_I_H__


#include <bootstrap.hh>


class CORBA_InitialReferences_i;


class omniInitialReferences {
public:
  static void set(const char* identifier,CORBA::Object_ptr obj);
  static CORBA::Object_ptr get(const char* identifier);
  static CORBA::ORB::ObjectIdList* list();

  static void initialise_bootstrap_agentImpl();

  static int invoke_bootstrap_agentImpl(GIOP_S&);
  // Returns 0 if there is no bootstrap agent.  May throw
  // the usual exceptions for an object invocation...

  static int is_bootstrap_agentImpl_initialised();
  // Returns true if a boostrap agent exists.

  static void initialise_bootstrap_agent(const char* host, CORBA::UShort port);

private:
  inline omniInitialReferences() {}
  omniInitialReferences(const omniInitialReferences&);
  omniInitialReferences& operator=(const omniInitialReferences&);
};


#endif  
