// -*- Mode: C++; -*-
//                            Package   : omniORB2
// GIOP.h                     Created on: 8/2/96
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
//       C++ mapping of the OMG GIOP module
//       Reference: CORBA V2.0 12.8.1
//	
//	

/*
  $Log$
  Revision 1.2  1997/05/06 16:06:27  sll
  Public release.

 */

#ifndef __GIOP_H__
#define __GIOP_H__

#include <omniORB2/CORBA_sysdep.h>
#include <omniORB2/CORBA_basetypes.h>
#include <omniORB2/IOP.h>

class GIOP {
public:
  enum MsgType {
    Request = 0,                             // sent by client
    Reply = 1,                               // by server
    CancelRequest = 2,                       // by client
    LocateRequest = 3,                       // by client
    LocateReply = 4,                         // by server
    CloseConnection = 5,                     // by server
    MessageError = 6                         // by both
  };

  struct Version {
    _CORBA_Char major;
    _CORBA_Char minor;
  };

  struct MessageHeader {
    _CORBA_Char	     magic [4];		      // "GIOP"
    Version	     GIOP_version;
    _CORBA_Boolean   byte_order;	      // 0 = big, 1 = little
    _CORBA_Octet     message_type;
    _CORBA_ULong     message_size;
  };

  class RequestHeader {
  public:
    IOP::ServiceContextList	service_context;
    _CORBA_ULong                request_id;
    _CORBA_Boolean              response_expected;
    _CORBA_Unbounded_Sequence_Octet object_key;
    _CORBA_Char*                operation;
    _CORBA_Unbounded_Sequence_Octet requesting_principal;
  };

  enum ReplyStatusType {
    NO_EXCEPTION,
    USER_EXCEPTION,
    SYSTEM_EXCEPTION,
    LOCATION_FORWARD
  };

  class ReplyHeader {
  public:
    IOP::ServiceContextList	service_context;
    _CORBA_ULong                request_id;
    ReplyStatusType             reply_status;
  };

  struct CancelRequestHeader {
    _CORBA_ULong  request_id;
  };

  struct LocateRequestHeader {
  public:
    _CORBA_ULong          request_id;
    _CORBA_Unbounded_Sequence_Octet object_key;
  };

  enum LocateStatusType {
    UNKNOWN_OBJECT,
    OBJECT_HERE,
    OBJECT_FORWARD
  };

  struct LocateReplyHeader {
    _CORBA_ULong	request_id;
    LocateStatusType	locate_status;
  };
};

#endif // __GIOP_H__
