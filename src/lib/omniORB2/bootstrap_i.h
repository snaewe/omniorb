// -*- Mode: C++; -*-
//                            Package   : omniORB2
// bootstrap_i.h              Created on: 20/08/98
//                            Author    : Sai-Lai Lo
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
  Revision 1.1  1998/08/21 19:28:15  sll
  Initial revision

  */


#ifndef __BOOTSTRAP_I_H__
#define __BOOTSTRAP_I_H__


#include <bootstrap.hh>

class CORBA_InitialReferences_i;

class omniInitialReferences {
 public:
  void set(const char* identifier,CORBA::Object_ptr obj);
  CORBA::Object_ptr get(const char* identifier);
  CORBA::ORB::ObjectIdList* list();

  void initialise_bootstrap_agentImpl();
  CORBA_InitialReferences_i* has_bootstrap_agentImpl();

  void initialise_bootstrap_agent(const char* host,CORBA::UShort port);

  static omniInitialReferences* singleton();
 private:

  // pd_bootagent is the object reference to the initialisation agent
  // from which we can get initial object references for services such
  // as the NameService.
  CORBA_InitialReferences_var pd_bootagent;

  // pd_bootagentImpl, if initialised, is the implementation of the
  // interface of the initialisation agent in this address space. Once
  // the object is initialised, this address space will respond to
  // remote invocations from client requests for initial object references.
  CORBA_InitialReferences_i*  pd_bootagentImpl;

  struct serviceRecord {
    CORBA::String_member id;
    CORBA::Object_member ref;
  };

  _CORBA_Unbounded_Sequence< serviceRecord > pd_serviceList;

  omniInitialReferences();
  omniInitialReferences(const omniInitialReferences&);
  omniInitialReferences& operator=(const omniInitialReferences&);
};

class CORBA_InitialReferences_i : public _sk_CORBA_InitialReferences {
public:
  CORBA::Object_ptr get (const char* id) {
    return omniInitialReferences::singleton()->get(id);
  }
  CORBA::ORB::ObjectIdList* list () {
    return omniInitialReferences::singleton()->list();
  }
  CORBA_InitialReferences_i(CORBA::BOA_ptr boa) {
    _obj_is_ready(boa);
  }
private:
  CORBA_InitialReferences_i();
  CORBA_InitialReferences_i(const CORBA_InitialReferences_i&);
  CORBA_InitialReferences_i& operator=(const CORBA_InitialReferences_i&);
};


#endif  

