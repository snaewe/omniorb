// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_TypeCode_member.h    Created on: 2001/08/17
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
//    CORBA::TypeCode_member
//

/*
  $Log$
  Revision 1.1.4.1  2003/03/23 21:04:22  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.1  2001/08/17 13:39:49  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_member //////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_var;

class TypeCode_member {
public:
  TypeCode_member();
  inline TypeCode_member(TypeCode_ptr p) : _ptr(p) {}
  TypeCode_member(const TypeCode_member& p);
  ~TypeCode_member();

  TypeCode_member& operator=(TypeCode_ptr p);
  TypeCode_member& operator=(const TypeCode_member& p);
  TypeCode_member& operator=(const TypeCode_var& p);

  inline TypeCode_ptr operator->() const { return _ptr; }
  inline operator TypeCode_ptr() const   { return _ptr; }

  TypeCode_ptr _ptr;

  void operator>>=(cdrStream&) const;
  void operator<<=(cdrStream&);
};


class InterfaceDef;
class _objref_InterfaceDef;

class ImplementationDef {}; // Not used.
typedef ImplementationDef* ImplementationDef_ptr;
typedef ImplementationDef_ptr ImplementationDefRef;

class OperationDef;
class _objref_OperationDef;

class                     ServerRequest;
typedef ServerRequest*    ServerRequest_ptr;
typedef ServerRequest_ptr ServerRequestRef;

class                                                Request;
typedef Request*                                     Request_ptr;
typedef Request_ptr                                  RequestRef;
typedef _CORBA_PseudoObj_Var<Request>                Request_var;
typedef _CORBA_PseudoObj_Out<Request,Request_var>    Request_out;
typedef _CORBA_PseudoObj_Member<Request,Request_var> Request_member;
