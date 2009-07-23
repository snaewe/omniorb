// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniPolicy.h               Created on: 2001/11/07
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
//    omniORB specific policies
//

#ifndef __OMNIPOLICY_H__
#define __OMNIPOLICY_H__

_CORBA_MODULE omniPolicy

_CORBA_MODULE_BEG

typedef CORBA::UShort LocalShortcutPolicyValue;

_CORBA_MODULE_VARINT
const LocalShortcutPolicyValue LOCAL_CALLS_THROUGH_POA _init_in_decl_( = 0 );

_CORBA_MODULE_VARINT
const LocalShortcutPolicyValue LOCAL_CALLS_SHORTCUT    _init_in_decl_( = 1 );

_CORBA_MODULE_VARINT
const CORBA::PolicyType LOCAL_SHORTCUT_POLICY_TYPE
                                              _init_in_decl_( = 0x41545401 );

OMNIORB_DECLARE_POLICY_OBJECT(LocalShortcutPolicy, 0x41545401)

_CORBA_MODULE_END


#endif // __OMNIPOLICY_H__
