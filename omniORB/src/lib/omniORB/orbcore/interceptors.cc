// -*- Mode: C++; -*-
//                            Package   : omniORB2
// interceptors.cc            Created on: 22/09/2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2006 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories, Cambridge
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
  Revision 1.1.4.2  2006/07/19 09:26:47  dgrisby
  Forgot to add list of policy interceptors.

  Revision 1.1.4.1  2003/03/23 21:02:13  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.7  2002/11/26 14:51:52  dgrisby
  Implement missing interceptors.

  Revision 1.1.2.6  2002/09/10 23:17:11  dgrisby
  Thread interceptors.

  Revision 1.1.2.5  2002/03/27 11:44:52  dpg1
  Check in interceptors things left over from last week.

  Revision 1.1.2.4  2001/08/03 17:41:22  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.2.3  2001/04/18 18:18:08  sll
  Big checkin with the brand new internal APIs.

  Revision 1.1.2.2  2000/11/15 17:23:52  sll
  Added interceptors along the giop request processing path.

  Revision 1.1.2.1  2000/09/27 17:30:30  sll
  *** empty log message ***

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/IOP_S.h>
#include <omniORB4/IOP_C.h>
#include <omniORB4/omniServer.h>
#include <omniORB4/omniInterceptors.h>
#include <interceptors.h>
#include <exceptiondefs.h>
#include <initialiser.h>

OMNI_NAMESPACE_BEGIN(omni)

omniInterceptors::omniInterceptors() {}
omniInterceptors::~omniInterceptors() {}

omniInterceptorP::elmT* omniInterceptorP::encodeIOR              = 0;
omniInterceptorP::elmT* omniInterceptorP::decodeIOR              = 0;
omniInterceptorP::elmT* omniInterceptorP::clientSendRequest      = 0;
omniInterceptorP::elmT* omniInterceptorP::clientReceiveReply     = 0;
omniInterceptorP::elmT* omniInterceptorP::serverReceiveRequest   = 0;
omniInterceptorP::elmT* omniInterceptorP::serverSendReply        = 0;
omniInterceptorP::elmT* omniInterceptorP::serverSendException    = 0;
omniInterceptorP::elmT* omniInterceptorP::createIdentity         = 0;
omniInterceptorP::elmT* omniInterceptorP::createORBServer        = 0;
omniInterceptorP::elmT* omniInterceptorP::createPolicy           = 0;
omniInterceptorP::elmT* omniInterceptorP::createThread           = 0;
omniInterceptorP::elmT* omniInterceptorP::assignUpcallThread     = 0;


static void list_add(omniInterceptorP::elmT** ep, void* func)
{
  while (*ep) {
    if ((*ep)->func == func) return;
    ep = &((*ep)->next);
  }
    
  omniInterceptorP::elmT* np = new omniInterceptorP::elmT();
  np->func = func;
  np->next = *ep;
  *ep = np;
}

static void list_remove(omniInterceptorP::elmT** ep, void* func)
{
  while (*ep) {
    if ((*ep)->func == func) {
      omniInterceptorP::elmT* p = *ep;
      *ep = p->next;
      delete p;
      return;
    }
    ep = &((*ep)->next);
  }
}

static void list_del(omniInterceptorP::elmT** ep)
{
  while (*ep) {
    omniInterceptorP::elmT* p = *ep;
    *ep = p->next;
    delete p;
  }
}

#define INTERCEPTOR_IMPLEMENTATION(interceptor) \
void omniInterceptors::interceptor##_T::add(\
           omniInterceptors::interceptor##_T::interceptFunc f) { \
  list_add(&omniInterceptorP::interceptor, (void*)f); \
} \
\
void omniInterceptors::interceptor##_T::remove(\
            omniInterceptors::interceptor##_T::interceptFunc f) { \
  list_remove(&omniInterceptorP::interceptor, (void*)f); \
}

INTERCEPTOR_IMPLEMENTATION(encodeIOR)
INTERCEPTOR_IMPLEMENTATION(decodeIOR)
INTERCEPTOR_IMPLEMENTATION(clientSendRequest)
INTERCEPTOR_IMPLEMENTATION(clientReceiveReply)
INTERCEPTOR_IMPLEMENTATION(serverReceiveRequest)
INTERCEPTOR_IMPLEMENTATION(serverSendReply)
INTERCEPTOR_IMPLEMENTATION(serverSendException)
INTERCEPTOR_IMPLEMENTATION(createIdentity)
INTERCEPTOR_IMPLEMENTATION(createORBServer)
INTERCEPTOR_IMPLEMENTATION(createPolicy)
INTERCEPTOR_IMPLEMENTATION(createThread)
INTERCEPTOR_IMPLEMENTATION(assignUpcallThread)

#undef INTERCEPTOR_IMPLEMENTATION

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_interceptor_initialiser : public omniInitialiser {
public:

  omni_interceptor_initialiser() : pd_interceptors(0) {}

  void attach() {
    if (!pd_interceptors) pd_interceptors = new omniInterceptors();
  }

  void detach() {
    if (pd_interceptors) {
      delete pd_interceptors;
      pd_interceptors = 0;
      list_del(&omniInterceptorP::encodeIOR);
      list_del(&omniInterceptorP::decodeIOR);
      list_del(&omniInterceptorP::clientSendRequest);
      list_del(&omniInterceptorP::clientReceiveReply);
      list_del(&omniInterceptorP::serverReceiveRequest);
      list_del(&omniInterceptorP::serverSendReply);
      list_del(&omniInterceptorP::serverSendException);
      list_del(&omniInterceptorP::createIdentity);
      list_del(&omniInterceptorP::createORBServer);
      list_del(&omniInterceptorP::createPolicy);
      list_del(&omniInterceptorP::createThread);
      list_del(&omniInterceptorP::assignUpcallThread);
    }
  }

  omniInterceptors* pd_interceptors;
};

static omni_interceptor_initialiser initialiser;

omniInitialiser& omni_interceptor_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)
/////////////////////////////////////////////////////////////////////////////
omniInterceptors*
omniORB::getInterceptors() {
  if (!initialiser.pd_interceptors) 
    OMNIORB_THROW(INITIALIZE,INITIALIZE_FailedLoadLibrary,
		  CORBA::COMPLETED_NO);

  return initialiser.pd_interceptors;
}
