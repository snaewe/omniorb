// -*- Mode: C++; -*-
//                            Package   : omniORB
// current.cc                 Created on: 22 Nov 2000
//                            Author    : Sai-Lai Lo
//
//    Copyright (C) 2000 AT&T Laboratories- Cambridge
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
//

#include <omniORB4/CORBA.h>

CORBA::Current::Current(int nil) { _PR_setobj((omniObjRef*)(nil ? 0:1)); }

CORBA::Current::~Current() {}

CORBA::Current_ptr
CORBA::Current::_duplicate(CORBA::Current_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}

CORBA::Current_ptr
CORBA::Current::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) || !obj->_NP_is_pseudo() )  return _nil();

  Current_ptr p = (Current_ptr) obj->_ptrToObjRef(_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}


class omniNilCurrent : public CORBA::Current {
public:
  omniNilCurrent() : CORBA::Current(0) {}
  ~omniNilCurrent() {}
};

CORBA::Current_ptr
CORBA::Current::_nil()
{
  static omniNilCurrent* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilCurrent();
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


const char*
CORBA::Current::_PD_repoId = "IDL:omg.org/CORBA/Current:1.0";



