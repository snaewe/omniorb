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

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif


// See the description of this variable in omniInternal.h
const char* omniORB_3_0 = "omniORB version 3.0.x";


const IOP::ProfileId       IOP::TAG_INTERNET_IOP        = 0;
const IOP::ProfileId       IOP::TAG_MULTIPLE_COMPONENTS = 1;
const IOP::ServiceID       IOP::TransactionService      = 0;
const CORBA::Char          IIOP::current_major          = 1;
const CORBA::Char          IIOP::current_minor          = 0;
const CORBA::UShort        IIOP::DEFAULT_CORBALOC_PORT  = 2809;


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

//////////////////////////////////////////////////////////////////////
// Each pseudo object type must be assigned a magic number.
// This magic number is written into the member pd_magic of each instance
// The static function PR_is_valid() in each of the pseudo object class 
// can be used to test if the instance is indeed valid. If a random pointer 
// is passed to isvalid(), it is unlikely that the magic number would match.

const CORBA::ULong CORBA::Exception::PR_magic       = 0x45584354U; // EXCT
const CORBA::ULong CORBA::Object::_PR_magic         = 0x434F424AU; // COBJ
