// -*- Mode: C++; -*-
//                            Package   : omniORB
// constants.cc               Created on: 15/3/96
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
//	

/*
  $Log$
  Revision 1.17.2.1  2003/03/23 21:02:24  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.15.2.6  2001/08/06 15:49:17  sll
  Added IOP component TAG_OMNIORB_UNIX_TRANS for omniORB specific local
  transport using the unix domain socket.

  Revision 1.15.2.5  2001/07/31 16:28:01  sll
  Added GIOP BiDir support.

  Revision 1.15.2.4  2001/04/18 18:18:11  sll
  Big checkin with the brand new internal APIs.

  Revision 1.15.2.3  2000/11/03 19:10:24  sll
  Use OMNIORB_FOR_EACH_SYS_EXCEPTION to define repository IDs.

  Revision 1.15.2.2  2000/09/27 17:51:58  sll
  New constants defined in the GIOP and IIOP modules.

  Revision 1.15.2.1  2000/07/17 10:35:51  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.16  2000/07/13 15:25:58  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.14.6.4  2000/07/12 13:07:17  dpg1
  The default corbaloc port is 2809, not 2089 as specified in the
  specification!

  Revision 1.14.6.3  2000/05/24 17:18:10  dpg1
  Rename IIOP::DEFAULT_PORT IIOP::DEFAULT_CORBALOC_PORT

  Revision 1.14.6.2  2000/04/27 10:40:52  dpg1
  Interoperable Naming Service

  Add IIOP default port.

  Revision 1.14.6.1  1999/09/22 14:26:43  djr
  Major rewrite of orbcore to support POA.

  Revision 1.14  1999/05/25 17:22:40  sll
  Added magic number constants for all the pseudo objects.

  Revision 1.13  1999/04/15 11:01:27  djr
  Updated version variable to omniORB_2_8

  Revision 1.12  1999/03/11 16:25:51  djr
  Updated copyright notice

  Revision 1.11  1999/01/07 15:16:15  djr
  Changed version to 2.7

  Revision 1.10  1998/08/05 18:05:05  sll
  Version variable is changed to omniORB_2_6 (previously omniORB_2_5).

  Revision 1.9  1998/04/07 19:31:18  sll
  Updated to use namespace and bool type.

// Revision 1.8  1998/01/27  16:17:45  ewc
// Changed version to 2.5
//
// Revision 1.7  1997/12/12  20:00:00  sll
// Defined new version variable omniORB_x_y.
//
  Revision 1.6  1997/12/09 18:31:47  sll
  New constant CORBA::Object::repositoryID.
  omni::myORBId and omni::myBOAId removed.

  Revision 1.5  1997/08/21 21:55:23  sll
  Added system exception TRANSACTION_REQUIRED, TRANSACTION_ROLLEDBACK,
  INVALID_TRANSACTION, WRONG_TRANSACTION.

// Revision 1.4  1997/05/06  15:07:47  sll
// Public release.
//
*/

#include <omniORB4/CORBA.h>

OMNI_NAMESPACE_BEGIN(omni)

// See the description of this variable in omniInternal.h
const char* omniORB_4_0 = "omniORB version 4.0.x";

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
const IOP::ComponentId IOP::TAG_DCE_STRING_BINDING = 100;
const IOP::ComponentId IOP::TAG_DCE_BINDING_NAME = 101;
const IOP::ComponentId IOP::TAG_DCE_NO_PIPES = 102;
const IOP::ComponentId IOP::TAG_DCE_SEC_MECH = 103;
const IOP::ComponentId IOP::TAG_INET_SEC_TRANS = 123;
const IOP::ComponentId IOP::TAG_GROUP = 90001; // XXX NEED THE REAL CONSTANT !!
const IOP::ComponentId IOP::TAG_PRIMARY = 90002; // XXX NEED THE REAL CONSTANT!
const IOP::ComponentId IOP::TAG_HEARTBEAT_ENABLED = 90003; 
                                               // XXX NEED THE REAL CONSTANT !

const IOP::ComponentId IOP::TAG_OMNIORB_BIDIR      = 0x41545401; 
const IOP::ComponentId IOP::TAG_OMNIORB_UNIX_TRANS = 0x41545402; 



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
