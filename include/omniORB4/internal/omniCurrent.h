// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniCurrent.h              Created on: 2001/06/01
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Research Cambridge
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
//    Generic support for the kinds of Current interface.
//

/*
  $Log$
  Revision 1.1.2.1  2001/06/07 16:24:08  dpg1
  PortableServer::Current support.

*/

#ifndef __OMNICURRENT_H__
#define __OMNICURRENT_H__

#include <omnithread.h>
#include <omniORB4/omniORB.h>
#include <omniORB4/omniInternal.h>
#include <omniORB4/callDescriptor.h>

#if !defined(_core_attr)
# define _core_attr_defined
# if defined(_OMNIORB_LIBRARY)
#   define _core_attr
# else
#   define _core_attr _OMNIORB_NTDLL_IMPORT
# endif
#endif


class omniCurrent : public omni_thread::value_t {
public:
  omniCurrent(omni_thread* thread);
  // Should really be private but some compilers warn about a class
  // with only a private constructor and no friends.

  virtual ~omniCurrent();

  static void init();

  static inline omniCurrent* get()
  {
    if (!omniORB::supportCurrent) return 0;

    omni_thread* thr = omni_thread::self();
    if (!thr) return 0;

    omniCurrent* current = (omniCurrent*)(thr->get_value(thread_key));
    if (!current)
      current = new omniCurrent(thr);
    return current;
  }

  static inline omniCurrent* get(omni_thread* thr)
  {
    omniCurrent* current = (omniCurrent*)(thr->get_value(thread_key));
    if (!current)
      current = new omniCurrent(thr);
    return current;
  }

  // Accessors
  inline omniCallDescriptor* callDescriptor() { return pd_callDescriptor; }


  // Stack manipulation
  inline void pushCallDescriptor(omniCallDescriptor* desc)
  {
    desc->pd_current = this;
    desc->pd_current_next = pd_callDescriptor;
    pd_callDescriptor = desc;
  }

  inline void popCallDescriptor()
  {
    pd_callDescriptor->pd_current = 0;
    pd_callDescriptor = pd_callDescriptor->pd_current_next;
  }

private:
  static _core_attr omni_thread::key_t thread_key;

  omniCallDescriptor* pd_callDescriptor;

  // Not implemented
  omniCurrent(const omniCurrent&);
  omniCurrent& operator=(const omniCurrent&);
};

#ifdef _core_attr_defined
# undef _core_attr
# undef _cure_attr_defined
#endif

#endif // __OMNICURRENT_H__
