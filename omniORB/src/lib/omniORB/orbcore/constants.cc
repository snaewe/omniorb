// -*- Mode: C++; -*-
//                            Package   : omniORB
// constants.cc               Created on: 15/3/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2007 Apasphere Ltd
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
//	

#include <omniORB4/CORBA.h>

OMNI_NAMESPACE_BEGIN(omni)

// See the description of this variable in omniInternal.h
const char* omniORB_4_2 = "omniORB version 4.2.x";

const CORBA::ULong omniORB_TAG_ORB_TYPE = 0x41545400; // ATT\x00

OMNI_NAMESPACE_END(omni)

//////////////////////////////////////////////////////////////////////////
//                GIOP AddressingDisposition  constants                 //
//////////////////////////////////////////////////////////////////////////
const GIOP::AddressingDisposition GIOP::KeyAddr = 0;
const GIOP::AddressingDisposition GIOP::ProfileAddr = 1;
const GIOP::AddressingDisposition GIOP::ReferenceAddr = 2;


//////////////////////////////////////////////////////////////////////////
//                IOR profile tag constants                             //
//////////////////////////////////////////////////////////////////////////
const IOP::ProfileId  IOP::TAG_INTERNET_IOP = 0;
const IOP::ProfileId  IOP::TAG_MULTIPLE_COMPONENTS = 1;
const IOP::ProfileId  IOP::TAG_SCCP_IOP = 2;
const CORBA::UShort   IIOP::DEFAULT_CORBALOC_PORT  = 2809;

const IOP::ComponentId IOP::TAG_ORB_TYPE = 0;
const IOP::ComponentId IOP::TAG_CODE_SETS = 1;
const IOP::ComponentId IOP::TAG_POLICIES = 2;
const IOP::ComponentId IOP::TAG_ALTERNATE_IIOP_ADDRESS = 3;
const IOP::ComponentId IOP::TAG_COMPLETE_OBJECT_KEY = 5;
const IOP::ComponentId IOP::TAG_ENDPOINT_ID_POSITION = 6;
const IOP::ComponentId IOP::TAG_LOCATION_POLICY = 12;
const IOP::ComponentId IOP::TAG_ASSOCIATION_OPTIONS = 13;
const IOP::ComponentId IOP::TAG_SEC_NAME = 14;
const IOP::ComponentId IOP::TAG_SPKM_1_SEC_MECH = 15;
const IOP::ComponentId IOP::TAG_SPKM_2_SEC_MECH = 16;
const IOP::ComponentId IOP::TAG_KERBEROSV5_SEC_MECH = 17;
const IOP::ComponentId IOP::TAG_CSI_ECMA_SECRET_SEC_MECH = 18;
const IOP::ComponentId IOP::TAG_CSI_ECMA_HYBRID_SEC_MECH = 19;
const IOP::ComponentId IOP::TAG_SSL_SEC_TRANS = 20;
const IOP::ComponentId IOP::TAG_CSI_ECMA_PUBLIC_SEC_MECH = 21;
const IOP::ComponentId IOP::TAG_GENERIC_SEC_MECH = 22;
const IOP::ComponentId IOP::TAG_FIREWALL_TRANS = 23;
const IOP::ComponentId IOP::TAG_SCCP_CONTACT_INFO = 24;
const IOP::ComponentId IOP::TAG_JAVA_CODEBASE = 25;
const IOP::ComponentId IOP::TAG_CSI_SEC_MECH_LIST = 33;
const IOP::ComponentId IOP::TAG_NULL_TAG = 34;
const IOP::ComponentId IOP::TAG_TLS_SEC_TRANS = 36;
const IOP::ComponentId IOP::TAG_DCE_STRING_BINDING = 100;
const IOP::ComponentId IOP::TAG_DCE_BINDING_NAME = 101;
const IOP::ComponentId IOP::TAG_DCE_NO_PIPES = 102;
const IOP::ComponentId IOP::TAG_DCE_SEC_MECH = 103;
const IOP::ComponentId IOP::TAG_INET_SEC_TRANS = 123;
const IOP::ComponentId IOP::TAG_GROUP = 90001; // XXX NEED THE REAL CONSTANT !!
const IOP::ComponentId IOP::TAG_PRIMARY = 90002; // XXX NEED THE REAL CONSTANT!
const IOP::ComponentId IOP::TAG_HEARTBEAT_ENABLED = 90003; 
                                               // XXX NEED THE REAL CONSTANT !

const IOP::ComponentId IOP::TAG_OMNIORB_BIDIR         	      = 0x41545401;
const IOP::ComponentId IOP::TAG_OMNIORB_UNIX_TRANS    	      = 0x41545402;
const IOP::ComponentId IOP::TAG_OMNIORB_PERSISTENT_ID 	      = 0x41545403;
const IOP::ComponentId IOP::TAG_OMNIORB_RESTRICTED_CONNECTION = 0x41545404;


static struct {
  IOP::ComponentId id;
  const char*      name;
} componentIDName[] = {
  // Must arrange in ascending order of ComponentId.
  { IOP::TAG_ORB_TYPE, "TAG_ORB_TYPE" },
  { IOP::TAG_CODE_SETS, "TAG_CODE_SETS" },
  { IOP::TAG_POLICIES, "TAG_POLICIES" },
  { IOP::TAG_ALTERNATE_IIOP_ADDRESS, "TAG_ALTERNATE_IIOP_ADDRESS" },
  { IOP::TAG_COMPLETE_OBJECT_KEY, "TAG_COMPLETE_OBJECT_KEY" },
  { IOP::TAG_ENDPOINT_ID_POSITION, "TAG_ENDPOINT_ID_POSITION" },
  { IOP::TAG_LOCATION_POLICY, "TAG_LOCATION_POLICY" },
  { IOP::TAG_ASSOCIATION_OPTIONS, "TAG_ASSOCIATION_OPTIONS" },
  { IOP::TAG_SEC_NAME, "TAG_SEC_NAME" },
  { IOP::TAG_SPKM_1_SEC_MECH, "TAG_SPKM_1_SEC_MECH" },
  { IOP::TAG_SPKM_2_SEC_MECH,  "TAG_SPKM_2_SEC_MECH" },
  { IOP::TAG_KERBEROSV5_SEC_MECH, "TAG_KERBEROSV5_SEC_MECH" },
  { IOP::TAG_CSI_ECMA_SECRET_SEC_MECH, "TAG_CSI_ECMA_SECRET_SEC_MECH" },
  { IOP::TAG_CSI_ECMA_HYBRID_SEC_MECH, "TAG_CSI_ECMA_HYBRID_SEC_MECH" },
  { IOP::TAG_SSL_SEC_TRANS, "TAG_SSL_SEC_TRANS" },
  { IOP::TAG_CSI_ECMA_PUBLIC_SEC_MECH, "TAG_CSI_ECMA_PUBLIC_SEC_MECH" },
  { IOP::TAG_GENERIC_SEC_MECH, "TAG_GENERIC_SEC_MECH" },
  { IOP::TAG_FIREWALL_TRANS, "TAG_FIREWALL_TRANS" },
  { IOP::TAG_SCCP_CONTACT_INFO, "TAG_SCCP_CONTACT_INFO" },
  { IOP::TAG_JAVA_CODEBASE, "TAG_JAVA_CODEBASE" },
  { IOP::TAG_DCE_STRING_BINDING, "TAG_DCE_STRING_BINDING" },
  { IOP::TAG_DCE_BINDING_NAME, "TAG_DCE_BINDING_NAME" },
  { IOP::TAG_DCE_NO_PIPES, "TAG_DCE_NO_PIPES" },
  { IOP::TAG_DCE_SEC_MECH, "TAG_DCE_SEC_MECH" },
  { IOP::TAG_INET_SEC_TRANS, "TAG_INET_SEC_TRANS" },
  { IOP::TAG_GROUP, "TAG_GROUP" },
  { IOP::TAG_PRIMARY, "TAG_PRIMARY" },
  { IOP::TAG_HEARTBEAT_ENABLED, "TAG_HEARTBEAT_ENABLED" },
  { IOP::TAG_OMNIORB_BIDIR, "TAG_OMNIORB_BIDIR" },
  { IOP::TAG_OMNIORB_UNIX_TRANS, "TAG_OMNIORB_UNIX_TRANS" },
  { IOP::TAG_OMNIORB_PERSISTENT_ID, "TAG_OMNIORB_PERSISTENT_ID" },
  { IOP::TAG_OMNIORB_RESTRICTED_CONNECTION, "TAG_OMNIORB_RESTRICTED_CONNECTION" },
  { 0, 0 }
};


const char*
IOP::ComponentIDtoName(IOP::ComponentId v)
{
  static int tablesize = 0;

  if (!tablesize) {
    while (componentIDName[tablesize].name) tablesize++;
  }

  int top = tablesize;
  int bottom = 0;

  do {
    int index = (top + bottom) >> 1;
    IOP::ComponentId id = componentIDName[index].id;
    if (id == v) {
      return componentIDName[index].name;
    }
    else if (id > v) {
      top = index;
    }
    else {
      bottom = index + 1;
    }
  } while (top != bottom);

  return 0;
}

const IOP::ServiceID IOP::TransactionService = 0;
const IOP::ServiceID IOP::CodeSets = 1; 
const IOP::ServiceID IOP::ChainBypassCheck = 2;
const IOP::ServiceID IOP::ChainBypassInfo = 3;
const IOP::ServiceID IOP::LogicalThreadId = 4;
const IOP::ServiceID IOP::BI_DIR_IIOP = 5;
const IOP::ServiceID IOP::SendingContextRunTime = 6;
const IOP::ServiceID IOP::INVOCATION_POLICIES = 7;
const IOP::ServiceID IOP::FORWARDED_IDENTITY = 8;
const IOP::ServiceID IOP::UnknownExceptionInfo = 9;
const IOP::ServiceID IOP::RTCorbaPriority = 10;
const IOP::ServiceID IOP::RTCorbaPriorityRange = 11;
const IOP::ServiceID IOP::GROUP_VERSION = 90001;// XXX NEED THE REAL CONSTANT !
const IOP::ServiceID IOP::REQUEST = 90002; // XXX NEED THE REAL CONSTANT !

const IOP::ServiceID IOP::OMNIORB_RESTRICTED_CONNECTION = 0x41545404;

static struct {
  IOP::ServiceID id;
  const char* name;
} ServiceIDName[] = {
  // Must arrange in ascending order of ServiceId.
  { IOP::TransactionService, "TransactionService" }, 
  { IOP::CodeSets, "CodeSets" }, 
  { IOP::ChainBypassCheck, "ChainBypassCheck" },
  { IOP::ChainBypassInfo, "ChainBypassInfo" },
  { IOP::LogicalThreadId, "LogicalThreadId" },
  { IOP::BI_DIR_IIOP, "BI_DIR_IIOP" }, 
  { IOP::SendingContextRunTime, "SendingContextRunTime" },
  { IOP::INVOCATION_POLICIES, "INVOCATION_POLICIES" },
  { IOP::FORWARDED_IDENTITY, "FORWARDED_IDENTITY" },
  { IOP::UnknownExceptionInfo, "UnknownExceptionInfo" },
  { IOP::RTCorbaPriority, "RTCorbaPriority" },
  { IOP::RTCorbaPriorityRange, "RTCorbaPriorityRange" },
  { IOP::OMNIORB_RESTRICTED_CONNECTION, "OMNIORB_RESTRICTED_CONNECTION" },
  { 0, 0 }
};

const char*
IOP::ServiceIDtoName(IOP::ServiceID v)
{
  static int tablesize = 0;

  if (!tablesize) {
    while (ServiceIDName[tablesize].name) tablesize++;
  }

  int top = tablesize;
  int bottom = 0;

  do {
    int index = (top + bottom) >> 1;
    IOP::ServiceID id = ServiceIDName[index].id;
    if (id == v) {
      return ServiceIDName[index].name;
    }
    else if (id > v) {
      top = index;
    }
    else {
      bottom = index + 1;
    }
  } while (top != bottom);

  return 0;
}

//////////////////////////////////////////////////////////////////////
// Each pseudo object type must be assigned a magic number.
// This magic number is written into the member pd_magic of each instance
// The static function PR_is_valid() in each of the pseudo object class 
// can be used to test if the instance is indeed valid. If a random pointer 
// is passed to isvalid(), it is unlikely that the magic number would match.

const CORBA::ULong CORBA::Exception::PR_magic       = 0x45584354U; // EXCT
const CORBA::ULong CORBA::Object::_PR_magic         = 0x434F424AU; // COBJ


////////////////////////////////////////////////////////////////////////////
//             ValueType TypeCode related constants                       //
////////////////////////////////////////////////////////////////////////////

#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the variables external linkage otherwise. Its a bug.
namespace CORBA {

_init_in_def_( const ValueModifier VM_NONE        = 0; )
_init_in_def_( const ValueModifier VM_CUSTOM      = 1; )
_init_in_def_( const ValueModifier VM_ABSTRACT    = 2; )
_init_in_def_( const ValueModifier VM_TRUNCATABLE = 3; )

};
#else
_init_in_def_( const CORBA::ValueModifier CORBA::VM_NONE        = 0; )
_init_in_def_( const CORBA::ValueModifier CORBA::VM_CUSTOM      = 1; )
_init_in_def_( const CORBA::ValueModifier CORBA::VM_ABSTRACT    = 2; )
_init_in_def_( const CORBA::ValueModifier CORBA::VM_TRUNCATABLE = 3; )
#endif
