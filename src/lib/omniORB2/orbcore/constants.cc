// -*- Mode: C++; -*-
//                            Package   : omniORB2
// constants.cc               Created on: 15/3/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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
const char* omniORB_2_7 = "omniORB version 2.7.x";


const IOP::ProfileId       IOP::TAG_INTERNET_IOP = 0;
const IOP::ProfileId       IOP::TAG_MULTIPLE_COMPONENTS = 1;
const IOP::ServiceID       IOP::TransactionService = 0;
const CORBA::Char          IIOP::current_major = 1;
const CORBA::Char          IIOP::current_minor = 0;
const CORBA::Char          omni::myByteOrder = _OMNIORB_HOST_BYTE_ORDER_;
const omni::alignment_t    omni::max_alignment = ALIGN_8;


const CORBA::Char*
CORBA::Object::repositoryID = (CORBA::Char*) "IDL:omg.org/CORBA/Object:1.0";


const GIOP_Basetypes::MessageHeader::HeaderType
         GIOP_Basetypes::MessageHeader::Request = {
	   'G','I','O','P',
	   1,0,
	   _OMNIORB_HOST_BYTE_ORDER_,
	   GIOP::Request
	 };

const GIOP_Basetypes::MessageHeader::HeaderType 
         GIOP_Basetypes::MessageHeader::Reply = {
	   'G','I','O','P',
	   1,0,
	   _OMNIORB_HOST_BYTE_ORDER_,
	   GIOP::Reply
	 };

const GIOP_Basetypes::MessageHeader::HeaderType 
         GIOP_Basetypes::MessageHeader::CancelRequest = {
	   'G','I','O','P',
	   1,0,
	   _OMNIORB_HOST_BYTE_ORDER_,
	   GIOP::CancelRequest
	 };

const GIOP_Basetypes::MessageHeader::HeaderType 
         GIOP_Basetypes::MessageHeader::LocateRequest = {
	   'G','I','O','P',
	   1,0,
	   _OMNIORB_HOST_BYTE_ORDER_,
	   GIOP::LocateRequest
	 };

const GIOP_Basetypes::MessageHeader::HeaderType
         GIOP_Basetypes::MessageHeader::LocateReply = {
	   'G','I','O','P',
	   1,0,
	   _OMNIORB_HOST_BYTE_ORDER_,
	   GIOP::LocateReply
	 };

const GIOP_Basetypes::MessageHeader::HeaderType 
         GIOP_Basetypes::MessageHeader::CloseConnection= {
	   'G','I','O','P',
	   1,0,
	   _OMNIORB_HOST_BYTE_ORDER_,
	   GIOP::CloseConnection
	 };

const GIOP_Basetypes::MessageHeader::HeaderType 
         GIOP_Basetypes::MessageHeader::MessageError = {
	   'G','I','O','P',
	   1,0,
	   _OMNIORB_HOST_BYTE_ORDER_,
	   GIOP::MessageError
	 };

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
