// -*- Mode: C++; -*-
//                            Package   : omniORB2
// interceptors.cc            Created on: 22/09/2000
//                            Author    : Sai Lai Lo (sll)
//
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
  Revision 1.1.2.1  2000/09/27 17:30:30  sll
  *** empty log message ***

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/omniInterceptors.h>
#include <exceptiondefs.h>
#include <initialiser.h>

omniInterceptors::omniInterceptors() {}
omniInterceptors::~omniInterceptors() {}

class omniInterceptorP {
public:
  struct elmT {
    void* func;
    elmT* next;
  };

  void add(void* func) {
    elmT** ep = &head;

    while (*ep) {
      if ((*ep)->func == func) return;
      ep = &((*ep)->next);
    }
    
    elmT* np = new elmT();
    np->func = func;
    np->next = *ep;
    *ep = np;
  }

  void remove(void* func) {
    elmT** ep = &head;

    while (*ep) {
      if ((*ep)->func == func) {
	elmT* p = *ep;
	*ep = p->next;
	delete p;
	return;
      }
      ep = &((*ep)->next);
    }
  }

  omniInterceptorP() : head(0) {}
  ~omniInterceptorP() {
    elmT** ep = &head;

    while (*ep) {
      elmT* p = *ep;
      *ep = p->next;
      delete p;
    }
  }

  elmT* head;
};


#define INTERCEPTOR_IMPLEMENTATION(interceptor) \
omniInterceptors::interceptor::interceptor() { \
  pd_ilist = new omniInterceptorP(); \
} \
\
omniInterceptors::interceptor::~interceptor() { \
  delete pd_ilist; \
} \
\
void omniInterceptors::interceptor::add(\
           omniInterceptors::interceptor::interceptFunc f) { \
  pd_ilist->add((void*)f); \
} \
\
void omniInterceptors::interceptor::remove(\
            omniInterceptors::interceptor::interceptFunc f) { \
  pd_ilist->remove((void*)f); \
}


#define INTERCEPTOR_IMPLEMENTATION_1ARG(interceptor,argt,argn) \
INTERCEPTOR_IMPLEMENTATION(interceptor) \
void omniInterceptors::interceptor::visit(argt argn) { \
  omniInterceptorP::elmT* ep = pd_ilist->head; \
  while (ep) { \
    (*((omniInterceptors::interceptor::interceptFunc)(ep->func)))(argn); \
    ep = ep->next; \
  } \
}


INTERCEPTOR_IMPLEMENTATION_1ARG(encodeIOR_T,omniIOR*,ior)
INTERCEPTOR_IMPLEMENTATION_1ARG(decodeIOR_T,omniIOR*,ior)


#undef INTERCEPTOR_IMPLEMENTATION
#undef INTERCEPTOR_IMPLEMENTATION_1ARG

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
    }
  }

  omniInterceptors* pd_interceptors;
};

static omni_interceptor_initialiser initialiser;

omniInitialiser& omni_interceptor_initialiser_ = initialiser;

/////////////////////////////////////////////////////////////////////////////
omniInterceptors*
omniORB::getInterceptors() {
  if (!initialiser.pd_interceptors) 
    OMNIORB_THROW(INITIALIZE,0, CORBA::COMPLETED_NO);

  return initialiser.pd_interceptors;
}
