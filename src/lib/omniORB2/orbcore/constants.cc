// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
  Revision 1.14.4.1  1999/09/15 20:18:33  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

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

#include <omniORB2/CORBA.h>


// See the description of this variable in omniInternal.h
const char* omniORB_2_9 = "omniORB version 2.9.x";
const CORBA::Char          omni::myByteOrder = _OMNIORB_HOST_BYTE_ORDER_;
const omni::alignment_t    omni::max_alignment = ALIGN_8;

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
const IOP::ComponentId IOP::TAG_JAVA_CODEBASE = 25;
const IOP::ComponentId IOP::TAG_DCE_STRING_BINDING = 100;
const IOP::ComponentId IOP::TAG_DCE_BINDING_NAME = 101;
const IOP::ComponentId IOP::TAG_DCE_NO_PIPES = 102;
const IOP::ComponentId IOP::TAG_DCE_SEC_MECH = 103;

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
  { IOP::TAG_JAVA_CODEBASE, "TAG_JAVA_CODEBASE" },
  { IOP::TAG_DCE_STRING_BINDING, "TAG_DCE_STRING_BINDING" },
  { IOP::TAG_DCE_BINDING_NAME, "TAG_DCE_BINDING_NAME" },
  { IOP::TAG_DCE_NO_PIPES, "TAG_DCE_NO_PIPES" },
  { IOP::TAG_DCE_SEC_MECH, "TAG_DCE_SEC_MECH" },
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

///////////////////////////////////////////////////////////////////////////
//               Repository IDs                                          //
///////////////////////////////////////////////////////////////////////////

const CORBA::Char*
CORBA::Object::repositoryID = (CORBA::Char*) "IDL:omg.org/CORBA/Object:1.0";

// Length of the longest system exception IR repository ID
//   it is currently "IDL:omg.org/CORBA/TRANSACTION_ROLLEDBACK:1.0"
// the value include the terminating null character
const CORBA::ULong 
      GIOP_Basetypes::SysExceptRepoID::maxIDLen = 
           omniORB_GIOP_Basetypes_SysExceptRepoID_maxIDLen;

const CORBA::Char *
      GIOP_Basetypes::SysExceptRepoID::version = ((CORBA::Char *)":1.0");

const CORBA::ULong 
      GIOP_Basetypes::SysExceptRepoID::versionLen = 
             strlen((const char *)GIOP_Basetypes::
                                     SysExceptRepoID::version);

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::UNKNOWN ((CORBA::Char *)
						"IDL:omg.org/CORBA/UNKNOWN");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::BAD_PARAM ((CORBA::Char *)
					  "IDL:omg.org/CORBA/BAD_PARAM");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::NO_MEMORY ((CORBA::Char *)
					  "IDL:omg.org/CORBA/NO_MEMORY");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::IMP_LIMIT ((CORBA::Char *)
					  "IDL:omg.org/CORBA/IMP_LIMIT");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::COMM_FAILURE ((CORBA::Char *)
					     "IDL:omg.org/CORBA/COMM_FAILURE");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INV_OBJREF ((CORBA::Char *)
					   "IDL:omg.org/CORBA/INV_OBJREF");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::OBJECT_NOT_EXIST ((CORBA::Char *)
					 "IDL:omg.org/CORBA/OBJECT_NOT_EXIST");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::NO_PERMISSION ((CORBA::Char *)
				      "IDL:omg.org/CORBA/NO_PERMISSION");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INTERNAL ((CORBA::Char *)
						 "IDL:omg.org/CORBA/INTERNAL");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::MARSHAL ((CORBA::Char *)
						"IDL:omg.org/CORBA/MARSHAL");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INITIALIZE ((CORBA::Char *)
					   "IDL:omg.org/CORBA/INITIALIZE");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::NO_IMPLEMENT ((CORBA::Char *)
					     "IDL:omg.org/CORBA/NO_IMPLEMENT");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::BAD_TYPECODE ((CORBA::Char *)
					     "IDL:omg.org/CORBA/BAD_TYPECODE");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::BAD_OPERATION ((CORBA::Char *)
				      "IDL:omg.org/CORBA/BAD_OPERATION");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::NO_RESOURCES ((CORBA::Char *)
					     "IDL:omg.org/CORBA/NO_RESOURCES");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::NO_RESPONSE ((CORBA::Char *)
					    "IDL:omg.org/CORBA/NO_RESPONSE");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::PERSIST_STORE ((CORBA::Char *)
				      "IDL:omg.org/CORBA/PERSIST_STORE");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::BAD_INV_ORDER ((CORBA::Char *)
				      "IDL:omg.org/CORBA/BAD_INV_ORDER");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::TRANSIENT ((CORBA::Char *)
					  "IDL:omg.org/CORBA/TRANSIENT");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::FREE_MEM ((CORBA::Char *)
						 "IDL:omg.org/CORBA/FREE_MEM");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INV_IDENT ((CORBA::Char *)
					  "IDL:omg.org/CORBA/INV_IDENT");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INV_FLAG ((CORBA::Char *)
						 "IDL:omg.org/CORBA/INV_FLAG");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INTF_REPOS ((CORBA::Char *)
					   "IDL:omg.org/CORBA/INTF_REPOS");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::BAD_CONTEXT ((CORBA::Char *)
					    "IDL:omg.org/CORBA/BAD_CONTEXT");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::OBJ_ADAPTER ((CORBA::Char *)
					    "IDL:omg.org/CORBA/OBJ_ADAPTER");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::DATA_CONVERSION ((CORBA::Char *)
					"IDL:omg.org/CORBA/DATA_CONVERSION");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::TRANSACTION_REQUIRED ((CORBA::Char *)
				     "IDL:omg.org/CORBA/TRANSACTION_REQUIRED");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::TRANSACTION_ROLLEDBACK ((CORBA::Char *)
			       "IDL:omg.org/CORBA/TRANSACTION_ROLLEDBACK");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INVALID_TRANSACTION ((CORBA::Char *)
				    "IDL:omg.org/CORBA/INVALID_TRANSACTION");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::WRONG_TRANSACTION ((CORBA::Char *)
				  "IDL:omg.org/CORBA/WRONG_TRANSACTION");


//////////////////////////////////////////////////////////////////////////
//                     Magic numbers                                    //
//////////////////////////////////////////////////////////////////////////

// Each pseudo object type must be assigned a magic number.
// This magic number is written into the member pd_magic of each instance
// The static function PR_is_valid() in each of the pseudo object class 
// can be used to test if the instance is indeed valid. If a random pointer 
// is passed to isvalid(), it is unlikely that the magic number would match.
const CORBA::ULong CORBA::BOA::PR_magic             = 0x424F4154U; // BOAT
const CORBA::ULong CORBA::Context::PR_magic         = 0x43545854U; // CTXT
const CORBA::ULong CORBA::ContextList::PR_magic     = 0x4354584CU; // CTXL
const CORBA::ULong CORBA::DynAny::PR_magic          = 0x44594E54U; // DYNT
const CORBA::ULong CORBA::Environment::PR_magic     = 0x454E564CU; // ENVI
const CORBA::ULong CORBA::Exception::PR_magic       = 0x45584354U; // EXCT
const CORBA::ULong CORBA::ExceptionList::PR_magic   = 0x4558434CU; // EXCL
const CORBA::ULong CORBA::NamedValue::PR_magic      = 0x4E56414CU; // NVAL
const CORBA::ULong CORBA::NVList::PR_magic          = 0x4E564C54U; // NVLT
const CORBA::ULong CORBA::Object::PR_magic          = 0x434F424AU; // COBJ
const CORBA::ULong CORBA::ORB::PR_magic             = 0x4F524254U; // ORBT
const CORBA::ULong CORBA::Request::PR_magic         = 0x52455154U; // REQT
const CORBA::ULong CORBA::TypeCode::PR_magic        = 0x54594F4CU; // TCOL


//////////////////////////////////////////////////////////////////////////
//                     Misc                                             //
//////////////////////////////////////////////////////////////////////////


#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise. Its a bug.
namespace omniORB {
  extern const unsigned int hash_table_size = 103;
}
#else
const unsigned int omniORB::hash_table_size = 103;
#endif

#if defined(__GNUG__)

// The following template classes are defined before the template functions
// inline void _CORBA_Sequence<T>::operator<<= (NetBufferedStream &s) etc
// are defined.
// G++ (2.7.2 or may be later versions as well) does not compile in the
// template functions as a result.
// The following is a workaround which explicitly instantiate the classes
// again.

#if 0
//??
template class _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Octet,1,1>;
template class _CORBA_Sequence<_CORBA_Octet>;
#endif


#endif
