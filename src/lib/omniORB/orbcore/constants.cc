// -*- Mode: C++; -*-
//                            Package   : omniORB2
// constants.cc               Created on: 15/3/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.1  1997/01/08 17:26:01  sll
  Initial revision

  */

#include <omniORB2/CORBA.h>

const IOP::ProfileId       IOP::TAG_INTERNET_IOP = 0;
const IOP::ProfileId       IOP::TAG_MULTIPLE_COMPONENTS = 1;
const IOP::ServiceID       IOP::TransactionService = 0;
const CORBA::Char          IIOP::current_major = 1;
const CORBA::Char          IIOP::current_minor = 0;
const CORBA::Boolean       omniORB::myByteOrder = _OMNIORB_HOST_BYTE_ORDER_;
const omniORB::alignment_t omniORB::max_alignment = ALIGN_8;

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
//   it is currently "IDL:CORBA/OBJECT_NOT_EXIST:1.0"
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
                                                        "IDL:CORBA/UNKNOWN");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::BAD_PARAM ((CORBA::Char *)
                                                        "IDL:CORBA/BAD_PARAM");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::NO_MEMORY ((CORBA::Char *)
                                                        "IDL:CORBA/NO_MEMORY");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::IMP_LIMIT ((CORBA::Char *)
                                                        "IDL:CORBA/IMP_LIMIT");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::COMM_FAILURE ((CORBA::Char *)
                                                        "IDL:CORBA/COMM_FAILURE");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INV_OBJREF ((CORBA::Char *)
                                                        "IDL:CORBA/INV_OBJREF");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::OBJECT_NOT_EXIST ((CORBA::Char *)
                                                        "IDL:CORBA/OBJECT_NOT_EXIST");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::NO_PERMISSION ((CORBA::Char *)
                                                        "IDL:CORBA/NO_PERMISSION");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INTERNAL ((CORBA::Char *)
                                                        "IDL:CORBA/INTERNAL");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::MARSHAL ((CORBA::Char *)
                                                        "IDL:CORBA/MARSHAL");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INITIALIZE ((CORBA::Char *)
                                                        "IDL:CORBA/INITIALIZE");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::NO_IMPLEMENT ((CORBA::Char *)
                                                        "IDL:CORBA/NO_IMPLEMENT");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::BAD_TYPECODE ((CORBA::Char *)
                                                        "IDL:CORBA/BAD_TYPECODE");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::BAD_OPERATION ((CORBA::Char *)
                                                        "IDL:CORBA/BAD_OPERATION");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::NO_RESOURCES ((CORBA::Char *)
                                                        "IDL:CORBA/NO_RESOURCES");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::NO_RESPONSE ((CORBA::Char *)
                                                        "IDL:CORBA/NO_RESPONSE");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::PERSIST_STORE ((CORBA::Char *)
                                                        "IDL:CORBA/PERSIST_STORE");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::BAD_INV_ORDER ((CORBA::Char *)
                                                        "IDL:CORBA/BAD_INV_ORDER");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::TRANSIENT ((CORBA::Char *)
                                                        "IDL:CORBA/TRANSIENT");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::FREE_MEM ((CORBA::Char *)
                                                        "IDL:CORBA/FREE_MEM");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INV_IDENT ((CORBA::Char *)
                                                        "IDL:CORBA/INV_IDENT");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INV_FLAG ((CORBA::Char *)
                                                        "IDL:CORBA/INV_FLAG");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::INTF_REPOS ((CORBA::Char *)
                                                        "IDL:CORBA/INTF_REPOS");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::BAD_CONTEXT ((CORBA::Char *)
                                                        "IDL:CORBA/BAD_CONTEXT");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::OBJ_ADAPTER ((CORBA::Char *)
                                                        "IDL:CORBA/OBJ_ADAPTER");

const GIOP_Basetypes::_SysExceptRepoID 
      GIOP_Basetypes::SysExceptRepoID::DATA_CONVERSION ((CORBA::Char *)
                                                        "IDL:CORBA/DATA_CONVERSION");

const unsigned int omniObjectKey::hash_table_size = 103;

#if defined(__GNUG__)

// The following template classes are defined before the template functions
// inline void _CORBA_Sequence<T>::operator<<= (NetBufferedStream &s) etc
// are defined.
// G++ (2.7.2 or may be later versions as well) does not compile in the
// template functions as a result.
// The following is a workaround which explicitly instantiate the classes
// again.

template class _CORBA_Sequence<_CORBA_Boolean>;
template class _CORBA_Sequence<_CORBA_Char>;
template class _CORBA_Sequence<_CORBA_Octet>;
template class _CORBA_Sequence<_CORBA_Short>;
template class _CORBA_Sequence<_CORBA_UShort>;
template class _CORBA_Sequence<_CORBA_Long>;
template class _CORBA_Sequence<_CORBA_ULong>;
#ifndef NO_FLOAT
template class _CORBA_Sequence<_CORBA_Float>;
template class _CORBA_Sequence<_CORBA_Double>;
#endif

#endif
