// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corba_operators.h          Created on: 14/6/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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

#ifndef __CORBA_OPERATORS_H__
#define __CORBA_OPERATORS_H__

extern void operator<<=(CORBA::Any&, const CORBA::Exception&);
extern void operator<<=(CORBA::Any&, const CORBA::Exception*);

#define EXCEPTION_OPERATORS(fqname) \
extern void operator<<=(CORBA::Any&, const CORBA::fqname&); \
extern void operator<<=(CORBA::Any&, const CORBA::fqname*); \
extern CORBA::Boolean operator>>=(const CORBA::Any&, CORBA::fqname*&);


EXCEPTION_OPERATORS (UNKNOWN)
EXCEPTION_OPERATORS (BAD_PARAM)
EXCEPTION_OPERATORS (NO_MEMORY)
EXCEPTION_OPERATORS (IMP_LIMIT)
EXCEPTION_OPERATORS (COMM_FAILURE)
EXCEPTION_OPERATORS (INV_OBJREF)
EXCEPTION_OPERATORS (OBJECT_NOT_EXIST)
EXCEPTION_OPERATORS (NO_PERMISSION)
EXCEPTION_OPERATORS (INTERNAL)
EXCEPTION_OPERATORS (MARSHAL)
EXCEPTION_OPERATORS (INITIALIZE)
EXCEPTION_OPERATORS (NO_IMPLEMENT)
EXCEPTION_OPERATORS (BAD_TYPECODE)
EXCEPTION_OPERATORS (BAD_OPERATION)
EXCEPTION_OPERATORS (NO_RESOURCES)
EXCEPTION_OPERATORS (NO_RESPONSE)
EXCEPTION_OPERATORS (PERSIST_STORE)
EXCEPTION_OPERATORS (BAD_INV_ORDER)
EXCEPTION_OPERATORS (TRANSIENT)
EXCEPTION_OPERATORS (FREE_MEM)
EXCEPTION_OPERATORS (INV_IDENT)
EXCEPTION_OPERATORS (INV_FLAG)
EXCEPTION_OPERATORS (INTF_REPOS)
EXCEPTION_OPERATORS (BAD_CONTEXT)
EXCEPTION_OPERATORS (OBJ_ADAPTER)
EXCEPTION_OPERATORS (DATA_CONVERSION)
EXCEPTION_OPERATORS (TRANSACTION_REQUIRED)
EXCEPTION_OPERATORS (TRANSACTION_ROLLEDBACK)
EXCEPTION_OPERATORS (INVALID_TRANSACTION)
EXCEPTION_OPERATORS (WRONG_TRANSACTION)
EXCEPTION_OPERATORS (UnknownUserException)
EXCEPTION_OPERATORS (WrongTransaction)
EXCEPTION_OPERATORS (ContextList::Bounds)
EXCEPTION_OPERATORS (ExceptionList::Bounds)
EXCEPTION_OPERATORS (NVList::Bounds)
EXCEPTION_OPERATORS (TypeCode::Bounds)
EXCEPTION_OPERATORS (TypeCode::BadKind)
EXCEPTION_OPERATORS (DynAny::Invalid)
EXCEPTION_OPERATORS (DynAny::InvalidValue)
EXCEPTION_OPERATORS (DynAny::TypeMismatch)
EXCEPTION_OPERATORS (DynAny::InvalidSeq)
EXCEPTION_OPERATORS (ORB::InconsistentTypeCode)
EXCEPTION_OPERATORS (ORB::InvalidName)

#undef EXCEPTION_OPERATORS

#endif  // __CORBA_OPERATORS_H__
