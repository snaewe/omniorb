// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Principal.h          Created on: 2001/08/17
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
//    CORBA::Principal
//

/*
  $Log$
  Revision 1.1.4.1  2003/03/23 21:04:23  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.1  2001/08/17 13:39:48  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
////////////////////////////// Principal /////////////////////////////
//////////////////////////////////////////////////////////////////////

class Principal;
typedef Principal* Principal_ptr;
typedef Principal_ptr PrincipalRef;
//typedef _CORBA_PseudoObj_Var<Principal> Principal_var;
//typedef _CORBA_PseudoObj_Out<Principal,Principal_var> Principal_out;

class PrincipalID : public _CORBA_Unbounded_Sequence_Octet {
public:
  inline PrincipalID() {}
  inline PrincipalID(const PrincipalID& seq)
    : _CORBA_Unbounded_Sequence_Octet(seq) {}
  inline PrincipalID(CORBA::ULong max)
    : _CORBA_Unbounded_Sequence_Octet(max) {}
  inline PrincipalID(CORBA::ULong max, CORBA::ULong len, CORBA::Octet* val, CORBA::Boolean rel=0)
    : _CORBA_Unbounded_Sequence_Octet(max, len, val, rel) {}
  inline PrincipalID& operator = (const PrincipalID& seq) {
    _CORBA_Unbounded_Sequence_Octet::operator=(seq);
    return *this;
  };
};

class Principal {
public:
  static Principal_ptr _duplicate(Principal_ptr);
  static Principal_ptr _nil();
private:
  Principal(); // Not implemented yet
};
