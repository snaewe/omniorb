// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniLifeCycle.cc           Created on: 1997/0920
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 1997 Olivetti & Oracle Research Laboratory
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

// $Log$
// Revision 1.2  1997/12/10 11:39:33  sll
// Updated life cycle runtime.
//
// Revision 1.1  1997/09/20  17:04:23  dpg1
// Initial revision
//
// Revision 1.1  1997/09/20  17:04:23  dpg1
// Initial revision
//


#include <omniORB2/omniLC.h>

void
omniLC::_wrap_proxy::_register_wrap(omniObject *obj) {
  {
    omniRopeAndKey l;
    obj->getRopeAndKey(l);
    _wrapped_key = *((omniObjectKey *)l.key());
  }
  omniObject::wrappedObjectTableLock.lock();
  omniLC::_wrap_proxy **p = (omniLC::_wrap_proxy **)
    (&omniObject::wrappedObjectTable[omniORB::hash(_wrapped_key)]);
  _next_wrap_proxy = *p;
  *p = this;
  omniObject::wrappedObjectTableLock.unlock();
}

void
omniLC::_wrap_proxy::_unregister_wrap() {
  omniObject::wrappedObjectTableLock.lock();
  omniLC::_wrap_proxy **p = (omniLC::_wrap_proxy **)
    (&omniObject::wrappedObjectTable[omniORB::hash(_wrapped_key)]);
  while (*p) {
    if (*p == this) {
      *p = _next_wrap_proxy;
      break;
    }
    p = &((*p)->_next_wrap_proxy);
  }
  _next_wrap_proxy = 0;
  omniObject::wrappedObjectTableLock.unlock();
}

void
omniLC::_wrap_proxy::_reset_wraps(omniObject *obj) {
  omniObjectKey k;
  {
    omniRopeAndKey l;
    obj->getRopeAndKey(l);
    k = *((omniObjectKey *)l.key());
  }
  omniObject::wrappedObjectTableLock.lock();
  omniLC::_wrap_proxy **p = (omniLC::_wrap_proxy **)
    (&omniObject::wrappedObjectTable[omniORB::hash(k)]);
  while (*p) {
    if ((*p)->_wrapped_key == k) {
      (*p)->_reset_proxy();
      *p = (*p)->_next_wrap_proxy;
    }
    else
      p = &((*p)->_next_wrap_proxy);
  }
  omniObject::wrappedObjectTableLock.unlock();
}
