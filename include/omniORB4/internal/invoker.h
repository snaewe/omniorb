// -*- Mode: C++; -*-
//                            Package   : omniORB
// invoker.h                  Created on: 20 Dec 2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
  Revision 1.1.6.2  2005/01/06 23:08:25  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.6.1  2003/03/23 21:03:45  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.3  2002/01/09 11:35:22  dpg1
  Remove separate omniAsyncInvoker library to save library overhead.

  Revision 1.1.4.2  2001/08/01 10:08:20  dpg1
  Main thread policy.

  Revision 1.1.4.1  2001/04/18 17:18:59  sll
  Big checkin with the brand new internal APIs.

  Revision 1.1.2.1  2001/02/23 16:47:02  sll
  Added new files.

*/

#ifndef __INVOKER_H__
#define __INVOKER_H__

#ifdef _core_attr
#  error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#  define _core_attr
#else
#  define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

  class ORBAsyncInvoker : public omniAsyncInvoker {
  public:
    ORBAsyncInvoker(unsigned int max=10000)
      : omniAsyncInvoker(max) {}

    virtual ~ORBAsyncInvoker();

    int  work_pending();
    void perform(unsigned long secs = 0, unsigned long nanosecs = 0);

  protected:
    int insert_dedicated(omniTask*);
    int cancel_dedicated(omniTask*);
  };

  extern _core_attr ORBAsyncInvoker* orbAsyncInvoker;

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __INVOKER_H__
