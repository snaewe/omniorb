// -*- Mode: C++; -*-
//                            Package   : omniORB2
// initRefs.h                 Created on: 20/08/98
//                            Author    : Sai-Lai Lo
//
//    Copyright (C) 1996-2000 AT&T Laboratories Cambridge
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
//	*** PROPRIETARY INTERFACE ***
//	

/*
  $Log$
  Revision 1.1.2.2  2000/06/27 15:33:53  sll
  Changed #include to match the new location of the stubs.

  Revision 1.1.2.1  2000/04/27 10:39:18  dpg1
  Interoperable Naming Service

  Moved from bootstrap_i.h, and extended.

  Revision 1.4.6.2  1999/09/27 08:48:31  djr
  Minor corrections to get rid of warnings.

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


#ifndef __INITREFS_H__
#define __INITREFS_H__


#include <omniORB3/bootstrap.hh>


class CORBA_InitialReferences_i;


class omniInitialReferences {
public:
  static CORBA::Boolean setFromFile(const char* identifier, const char* uri);
  static CORBA::Boolean setFromArgs(const char* identifier, const char* uri);
  // Set initial references from the configuration file and the
  // command line arguments respectively. Return true if the uri looks
  // syntactically valid, false if it is definitely invalid.

  static void setDefaultInitRefFromFile(const char* defInit);
  static void setDefaultInitRefFromArgs(const char* defInit);
  // Default string set by -ORBDefaultInitRef

  static CORBA::Object_ptr resolve(const char*  identifier,
				   unsigned int cycles = 0);
  // Real implementation of ORB::resolve_initial_references(). cycles
  // is used to count recursive calls within stringToObject, and bail
  // out if we loop too much. Responsible for returning pseudo objects
  // (like "RootPOA") as well as normal CORBA objects.

  static CORBA::ORB::ObjectIdList* list();
  // Real implementation of ORB::list_initial_services().


  // Deprecated INIT bootagent functions:

  static void remFromFile(const char* identifier);
  // Remove the specified identifier from the file list. Used to
  // remove NameService and InterfaceRepository if -ORBInitialHost is
  // given on the command line.

  static void remFromArgs(const char* identifier);
  // Not used, just here for symmetry

  static void initialise_bootstrap_agentImpl();

  static int invoke_bootstrap_agentImpl(GIOP_S&);
  // Returns 0 if there is no bootstrap agent.  May throw
  // the usual exceptions for an object invocation...

  static int is_bootstrap_agentImpl_initialised();
  // Returns true if a boostrap agent exists.

  static void initialise_bootstrap_agent(const char* host, CORBA::UShort port);
};


#endif  
