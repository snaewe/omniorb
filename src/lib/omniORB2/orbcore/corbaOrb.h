// -*- Mode: C++; -*-
//                            Package   : omniORB
// corbaOrb.h                 Created on: 22/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//    Implementation of the ORB interface.
//

/*
  $Log$
  Revision 1.1.2.1  1999/09/22 14:26:46  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __CORBAORB_H__
#define __CORBAORB_H__


class omniOrbORB : public CORBA::ORB {
public:
  virtual ~omniOrbORB();
  omniOrbORB(int nil);

  virtual char* object_to_string(CORBA::Object_ptr);
  virtual CORBA::Object_ptr string_to_object(const char*);

  virtual CORBA::ORB::ObjectIdList* list_initial_services();
  virtual CORBA::Object_ptr resolve_initial_references(const char* id);

  virtual CORBA::Boolean work_pending();
  virtual void perform_work();
  virtual void run();
  virtual void shutdown(CORBA::Boolean);
  virtual void destroy();

  virtual CORBA::Boolean _is_a(const char*);
  virtual CORBA::Boolean _non_existent();
  virtual CORBA::Boolean _is_equivalent(CORBA::Object_ptr);
  virtual CORBA::ULong   _hash(CORBA::ULong);
  virtual void* _ptrToObjRef(const char*);
  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Override CORBA::Object.

  void actual_shutdown();

  inline void incrRefCount() { pd_refCount++; }
  // Must hold <orb_lock>.

private:
  void do_shutdown(CORBA::Boolean wait_for_completion);

  int pd_refCount;
  int pd_destroyed;
  int pd_shutdown;
  int pd_shutdown_in_progress;
  // All protected by <orb_lock>.
};


#endif  // __CORBAORB_H__
