// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Environment.h        Created on: 2001/08/17
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
//    CORBA::Environment
//

/*
  $Log$
  Revision 1.1.2.1  2001/08/17 13:39:45  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////////// Environment ////////////////////////////
//////////////////////////////////////////////////////////////////////

class Environment;
typedef Environment* Environment_ptr;
typedef Environment_ptr EnvironmentRef;
typedef _CORBA_PseudoObj_Var<Environment> Environment_var;
typedef _CORBA_PseudoObj_Out<Environment,Environment_var> Environment_out;

class Environment {
public:
  Environment();     // Allows Environment instance to be created on
  // stack or by x = new Environment().
  virtual ~Environment();

  virtual void exception(Exception*);
  virtual Exception* exception() const;
  virtual void clear();

  virtual Boolean NP_is_nil() const;
  virtual Environment_ptr NP_duplicate();

  static Environment_ptr _duplicate(Environment_ptr);
  static Environment_ptr _nil();

  static inline _CORBA_Boolean PR_is_valid(Environment_ptr p ) {
    return ((p) ? (p->pd_magic == PR_magic) : 1);
  }

  static _dyn_attr const _CORBA_ULong PR_magic;

private:
  CORBA::Exception* pd_exception;
  _CORBA_ULong      pd_magic;

  Environment(const Environment&);
  Environment& operator=(const Environment&);

public:
  CORBA::Boolean pd_is_pseudo;
};
