// -*- Mode: C++; -*-
//                            Package   : omniORB2
// BiDirPolicy.h              Created on: 30/7/2001
//                            Author    : Sai Lai Lo (sll)
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
//	*** PROPRIETORY INTERFACE ***
//

/*
  $Log$
  Revision 1.1.2.1  2001/07/31 16:10:39  sll
  Added GIOP BiDir support.

  */


#ifndef __BIDIRPOLICY_H__
#define __BIDIRPOLICY_H__

_CORBA_MODULE BiDirPolicy

_CORBA_MODULE_BEG

typedef CORBA::UShort BidirectionalPolicyValue;

const BidirectionalPolicyValue NORMAL             _init_in_decl_( = 0 );
const BidirectionalPolicyValue BOTH               _init_in_decl_( = 1 );

const CORBA::PolicyType BIDIRECTIONAL_POLICY_TYPE _init_in_decl_( = 37 );

OMNIORB_DECLARE_POLICY_OBJECT(BidirectionalPolicy, BIDIRECTIONAL_POLICY_TYPE)


_CORBA_MODULE_END



#endif // __BIDIRPOLICY_H__
