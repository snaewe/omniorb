// -*- Mode: C++; -*-
//                          Package   : omniNames
// NamingContext_i.h        Author    : Tristan Richardson (tjr)
//
//    Copyright (C) 1997-1999 AT&T Laboratories Cambridge
//
//  This file is part of omniNames.
//
//  omniNames is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//

#ifndef _NamingContext_i_h_
#define _NamingContext_i_h_

#include <ReadersWritersLock.h>
#include <log.h>
#include <omniORB3/Naming.hh>

extern PortableServer::POA_var the_poa;
extern PortableServer::POA_var the_ins_poa;


class ObjectBinding;

class NamingContext_i : public POA_CosNaming::NamingContextExt,
			public PortableServer::RefCountServantBase
{

  friend class ObjectBinding;
  friend class omniNameslog;

public:

  NamingContext_i(PortableServer::POA_ptr poa,
		  const PortableServer::ObjectId& id,
		  omniNameslog* l);

  PortableServer::ObjectId* PR_id() {
    return nc_poa->servant_to_id(this);
  }

  //
  // CosNaming::NamingContext operations:
  //

  void bind(const CosNaming::Name& n, CORBA::Object_ptr obj) {
    bind_helper(n, obj, CosNaming::nobject, 0);
  }

  void rebind(const CosNaming::Name& n, CORBA::Object_ptr obj) {
    bind_helper(n, obj, CosNaming::nobject, 1);
  }

  void bind_context(const CosNaming::Name& n,
		    CosNaming::NamingContext_ptr nc) {
    bind_helper(n, nc, CosNaming::ncontext, 0);
  }

  void rebind_context(const CosNaming::Name& n,
		      CosNaming::NamingContext_ptr nc) {
    bind_helper(n, nc, CosNaming::ncontext, 1);
  }

  CORBA::Object_ptr resolve(const CosNaming::Name& n);

  void unbind(const CosNaming::Name& n);

  CosNaming::NamingContext_ptr new_context();

  CosNaming::NamingContext_ptr bind_new_context(const CosNaming::Name& n);

  void destroy();
  void list(CORBA::ULong how_many, CosNaming::BindingList_out bl,
	    CosNaming::BindingIterator_out bi);

  //
  // CosNaming::NamingContextExt operations
  //

  char*             to_string  (const CosNaming::Name& n);
  CosNaming::Name*  to_name    (const char*            sn);
  char*             to_url     (const char*            addr, const char* sn);
  CORBA::Object_ptr resolve_str(const char*            n);


private:

  omniNameslog* redolog;
  PortableServer::POA_ptr nc_poa; // The POA this NamingContext is
                                  // activated in

  //
  // This multiple-readers, single-writer lock is used to control access to
  // all NamingContexts served by this process.
  //

  static ReadersWritersLock lock;

  //
  // We keep a list of all NamingContexts served by this process.  The
  // order in the list is simply that of creation.  This has nothing to do
  // with any naming hierarchy, except that the headContext is always the
  // "root" context since it is created first.
  //

  static NamingContext_i* headContext;
  static NamingContext_i* tailContext;
  NamingContext_i* next;
  NamingContext_i* prev;

  //
  // The following represent the list of bindings within this context.
  //

  ObjectBinding* headBinding;
  ObjectBinding* tailBinding;
  unsigned long size;

  //
  // These are private routines which do most of the job of the various
  // IDL operations.
  //

  void bind_helper(const CosNaming::Name& n, CORBA::Object_ptr obj,
		   CosNaming::BindingType t, CORBA::Boolean rebind);

  ObjectBinding* resolve_simple(const CosNaming::Name& name);

  CosNaming::NamingContext_ptr resolve_compound(const CosNaming::Name& name,
						CosNaming::Name& restOfName);

  // remember the destructor for an object should never be called explicitly.
  ~NamingContext_i();
};

#endif
