// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniInterceptors.h         Created on: 22/09/2000
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
  Revision 1.1.2.1  2000/09/27 16:54:08  sll
  *** empty log message ***

*/

#ifndef __OMNIINTERCEPTORS_H__
#define __OMNIINTERCEPTORS_H__

#ifndef __CORBA_H_EXTERNAL_GUARD__
#include <omniORB4/CORBA.h>
#endif

class omniInterceptorP;

class omniInterceptors {
 public:

  //////////////////////////////////////////////////////////////////
  class encodeIOR_T {
  public:
    typedef void (*interceptFunc)(omniIOR* ior);

    void add(interceptFunc);
    void remove(interceptFunc);

    encodeIOR_T();
    ~encodeIOR_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(omniIOR*); // ORB internal function


  };

  //////////////////////////////////////////////////////////////////
  class decodeIOR_T {
  public:
    typedef void (*interceptFunc)(omniIOR* ior);

    void add(interceptFunc);
    void remove(interceptFunc);

    decodeIOR_T();
    ~decodeIOR_T();

  private:
    omniInterceptorP* pd_ilist;

  public:
    void visit(omniIOR*);  // ORB internal function
  };

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  encodeIOR_T encodeIOR;
  decodeIOR_T decodeIOR;



  //////////////////////////////////////////////////////////////////
  friend class omni_interceptor_initialiser;

 private:
  omniInterceptors();
  ~omniInterceptors();
};

#endif // __OMNIINTERCEPTORS_H__
