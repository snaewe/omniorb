// -*- Mode: C++; -*-
//                            	 Package   : omniORB
// CORBA_UnknownUserException.h	 Created on: 2001/08/17
//                            	 Author    : Duncan Grisby (dpg1)
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
//    CORBA::UnknownUserException
//

/*
  $Log$
  Revision 1.1.2.2  2001/10/17 16:44:00  dpg1
  Update DynAny to CORBA 2.5 spec, const Any exception extraction.

  Revision 1.1.2.1  2001/08/17 13:39:49  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////// UnknownUserException ////////////////////////
//////////////////////////////////////////////////////////////////////

class UnknownUserException : public UserException {
public:
  virtual ~UnknownUserException();

  UnknownUserException(Any* ex);
  // Consumes <ex> which MUST be a UserException.

  inline UnknownUserException(const UnknownUserException& ex)
    : UserException(ex) {
    pd_exception = new Any(*ex.pd_exception);
  }
  UnknownUserException& operator=(const UnknownUserException& ex) {
    UserException::operator=(ex);
    if (pd_exception) delete pd_exception;
    pd_exception = new Any(*ex.pd_exception);
    return *this;
  }

  Any& exception();

  virtual void _raise() const;
  static UnknownUserException* _downcast(Exception*);
  static const UnknownUserException* _downcast(const Exception*);
  static inline UnknownUserException* _narrow(Exception* e) {
    return _downcast(e);
  }

  static _dyn_attr Exception::insertExceptionToAny    insertToAnyFn;
  static _dyn_attr Exception::insertExceptionToAnyNCP insertToAnyFnNCP;

private:
  virtual Exception* _NP_duplicate() const;
  virtual const char* _NP_typeId() const;
  virtual const char* _NP_repoId(int* size) const;
  virtual void _NP_marshal(cdrStream&) const;

  Any* pd_exception;
};
