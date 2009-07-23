// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_ExceptionList.h      Created on: 2001/08/17
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
//    CORBA::ExceptionList
//

/*
  $Log$
  Revision 1.1.2.2  2001/11/06 15:41:34  dpg1
  Reimplement Context. Remove CORBA::Status. Tidying up.

  Revision 1.1.2.1  2001/08/17 13:39:46  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////// ExceptionList ///////////////////////////
//////////////////////////////////////////////////////////////////////

class ExceptionList;
typedef ExceptionList* ExceptionList_ptr;
typedef ExceptionList_ptr ExceptionListRef;
typedef _CORBA_PseudoObj_Var<ExceptionList> ExceptionList_var;
typedef _CORBA_PseudoObj_Out<ExceptionList,ExceptionList_var> ExceptionList_out;

class ExceptionList {
public:
  virtual ~ExceptionList();

  virtual ULong count() const = 0;
  virtual void add(TypeCode_ptr tc) = 0;
  virtual void add_consume(TypeCode_ptr tc) = 0;
  // Consumes <tc>.

  virtual TypeCode_ptr item(ULong index) = 0;
  // Retains ownership of return value.

  virtual void remove(ULong index) = 0;

  virtual Boolean NP_is_nil() const = 0;
  virtual ExceptionList_ptr NP_duplicate() = 0;

  static ExceptionList_ptr _duplicate(ExceptionList_ptr);
  static ExceptionList_ptr _nil();

  // OMG Interface:

  OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(Bounds, _dyn_attr)

    static inline _CORBA_Boolean PR_is_valid(ExceptionList_ptr p ) {
    return ((p) ? (p->pd_magic == PR_magic) : 1);
  }

  static _dyn_attr const _CORBA_ULong PR_magic;

protected:
  ExceptionList() { pd_magic = PR_magic; }

private:
  _CORBA_ULong pd_magic;

  ExceptionList(const ExceptionList& el);
  ExceptionList& operator=(const ExceptionList&);
};
