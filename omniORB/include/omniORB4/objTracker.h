// -*- Mode: C++; -*-
//                            Package   : omniORB
// nilTracker.h               Created on: 2001/09/13
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//    Tracking "static" heap-allocated objects so they can be deleted
//    just before exit.

#ifndef __OBJTRACKER_H__
#define __OBJTRACKER_H__

OMNI_NAMESPACE_BEGIN(omni)

class omniTrackedObject {
public:
  virtual ~omniTrackedObject() {}
};

void registerNilCorbaObject(CORBA::Object_ptr obj);
// Used to register nil CORBA::Objects, plus nil psuedo objects
// derived from CORBA::Object.

void registerTrackedObject(omniTrackedObject* obj);
// Used to register other tracked objects.

OMNI_NAMESPACE_END(omni)

#endif // __OBJTRACKER_H__
