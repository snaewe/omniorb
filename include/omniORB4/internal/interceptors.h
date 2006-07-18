// -*- Mode: C++; -*-
//                            Package   : omniORB
// interceptors.h             Created on: 2002/03/21
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002 AT&T Laboratories, Cambridge
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
  Revision 1.1.4.2  2006/07/18 16:21:23  dgrisby
  New experimental connection management extension; ORB core support
  for it.

  Revision 1.1.4.1  2003/03/23 21:03:46  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.3  2002/11/26 14:51:49  dgrisby
  Implement missing interceptors.

  Revision 1.1.2.2  2002/09/10 23:17:11  dgrisby
  Thread interceptors.

  Revision 1.1.2.1  2002/03/27 11:44:51  dpg1
  Check in interceptors things left over from last week.

*/

#ifndef __INTERCEPTORS_H__
#define __INTERCEPTORS_H__

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif


OMNI_NAMESPACE_BEGIN(omni)

class omniInterceptorP {
public:
  struct elmT {
    void* func;
    elmT* next;
  };

  static _core_attr elmT* encodeIOR;
  static _core_attr elmT* decodeIOR;
  static _core_attr elmT* clientSendRequest;
  static _core_attr elmT* clientReceiveReply;
  static _core_attr elmT* serverReceiveRequest;
  static _core_attr elmT* serverSendReply;
  static _core_attr elmT* serverSendException;
  static _core_attr elmT* createIdentity;
  static _core_attr elmT* createORBServer;
  static _core_attr elmT* createPolicy;
  static _core_attr elmT* createThread;
  static _core_attr elmT* assignUpcallThread;

#define VISIT_FUNCTION(name) \
  static inline void visit(omniInterceptors::name##_T::info_T& info) { \
    for (elmT* e = name; e; e = e->next) { \
      if (!(*((omniInterceptors::name##_T::interceptFunc)(e->func)))(info)) \
	return; \
    } \
  }

  VISIT_FUNCTION(encodeIOR)
  VISIT_FUNCTION(decodeIOR)
  VISIT_FUNCTION(clientSendRequest)
  VISIT_FUNCTION(clientReceiveReply)
  VISIT_FUNCTION(serverReceiveRequest)
  VISIT_FUNCTION(serverSendReply)
  VISIT_FUNCTION(serverSendException)
  VISIT_FUNCTION(createIdentity)
  VISIT_FUNCTION(createORBServer)
  VISIT_FUNCTION(createPolicy)

#undef VISIT_FUNCTION
};


OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __INTERCEPTORS_H__
