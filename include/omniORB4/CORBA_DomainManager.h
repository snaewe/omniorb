// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_DomainManager.h      Created on: 2001/08/17
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
//    CORBA::DomainManager
//

/*
  $Log$
  Revision 1.1.4.1  2003/03/23 21:04:24  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.1  2001/08/17 13:39:45  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////////// Domain Manager  ////////////////////////
//////////////////////////////////////////////////////////////////////

class DomainManager;
typedef class DomainManager* DomainManager_ptr;
typedef DomainManager_ptr DomainManagerRef;

class DomainManager {
public:
  Policy_ptr get_domain_policy(PolicyType policy_type);

  static DomainManager_ptr _duplicate(DomainManager_ptr p);
  static DomainManager_ptr _nil();

private:
  DomainManager();
};

_CORBA_MODULE_FN Boolean is_nil(DomainManager_ptr p);
_CORBA_MODULE_FN void release(DomainManager_ptr);

typedef _CORBA_PseudoObj_Var<DomainManager> DomainManager_var;
typedef _CORBA_PseudoObj_Member<DomainManager,DomainManager_var>
DomainManager_member;
typedef _CORBA_Pseudo_Unbounded_Sequence<DomainManager,DomainManager_member>
DomainManagerList;
