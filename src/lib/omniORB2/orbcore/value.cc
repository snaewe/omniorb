// -*- Mode: C++; -*-
//                            Package   : omniORB
// value.cc                   Created on: 27/9/2000
//                            Author    : David Scott (djs)
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
//    Placeholder for valuetype code
//

/*
 $Log$
 Revision 1.1.2.2  2000/09/27 22:49:33  djs
 CORBA::ValueBase now included in build with more dummy methods

 Revision 1.1.2.1  2000/09/27 17:13:09  djs
 Struct member renaming
 Added command line options
 Added CORBA::ValueBase (just to do reference counting)
 General refactoring

 $Id$
*/

#include <omniORB3/CORBA.h>
#include <exceptiondefs.h>

CORBA::ValueBase::ValueBase(const CORBA::ValueBase& v){ }

CORBA::ValueBase *CORBA::ValueBase::_add_ref(){
  omni_mutex_lock lock(pd_state_lock);
  if (pd_refcount < 1)
    omniORB::log << "Warning: calling _add_ref on a ValueBase whose "
		 << "reference count is already zero";
  pd_refcount++;
  return this;
}

void CORBA::ValueBase::_remove_ref(){
  pd_state_lock.lock();
  int done = --pd_refcount > 0;
  pd_state_lock.unlock();
  if (done) return;
  
  delete this;
}

CORBA::ULong CORBA::ValueBase::_refcount_value(){
  CORBA::ULong value;
  pd_state_lock.lock();
  value = pd_refcount;
  pd_state_lock.unlock();
  
  return value;
}

CORBA::ValueBase* CORBA::ValueBase::_copy_value(){
  OMNIORB_THROW(BAD_PARAM,0,CORBA::COMPLETED_NO);
  return NULL; // never reached, pacify compiler
}


