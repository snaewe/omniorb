// -*- Mode: C++; -*-
//                            Package   : omniORB2
// GIOP.h                     Created on: 8/2/96
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
//       C++ mapping of the OMG GIOP module
//       Reference: CORBA V2.0 12.8.1
//	
//	

/*
  $Log$
  Revision 1.2.2.2  2000/09/27 17:07:07  sll
  Updated to include definitions for GIOP 1.1 and 1.2.

  Revision 1.2.2.1  2000/07/17 10:35:33  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:05  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.1  1999/09/24 09:51:40  djr
  Moved from omniORB2 + some new files.

  Revision 1.3  1999/06/18 21:12:56  sll
  Updated copyright notice.

  Revision 1.2  1997/05/06 16:06:27  sll
  Public release.

*/

#ifndef __OMNIORB_GIOP_H__
#define __OMNIORB_GIOP_H__

#include <omniORB4/CORBA_sysdep.h>
#include <omniORB4/CORBA_basetypes.h>
#include <omniORB4/IOP.h>


class GIOP {
public:
  enum MsgType {
    Request = 0,                             // sent by client
    Reply = 1,                               // by server
    CancelRequest = 2,                       // by client
    LocateRequest = 3,                       // by client
    LocateReply = 4,                         // by server
    CloseConnection = 5,                     // by server
    MessageError = 6,                        // by both
    Fragment = 7                             // by both
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

  typedef _CORBA_Short AddressingDisposition;
  static _core_attr const AddressingDisposition KeyAddr;
  static _core_attr const AddressingDisposition ProfileAddr;
  static _core_attr const AddressingDisposition ReferenceAddr;

  struct IORAddressingInfo {
    _CORBA_ULong  selected_profile_index;
    IOP::IOR      ior;
  };


  class RequestHeader {
  public:
    IOP::ServiceContextList	service_context;
    _CORBA_ULong                request_id;
    _CORBA_Boolean              response_expected;
    _CORBA_Unbounded_Sequence__Octet object_key;
    _CORBA_Char*                operation;
    _CORBA_Unbounded_Sequence__Octet requesting_principal;
  };

  enum ReplyStatusType {
    NO_EXCEPTION,
    USER_EXCEPTION,
    SYSTEM_EXCEPTION,
    LOCATION_FORWARD,
    LOCATION_FORWARD_PERM,   // GIOP 1.2
    NEEDS_ADDRESSING_MODE    // GIOP 1.2 
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
    _CORBA_Unbounded_Sequence__Octet object_key;
  };

  enum LocateStatusType {
    UNKNOWN_OBJECT,
    OBJECT_HERE,
    OBJECT_FORWARD,
    OBJECT_FORWARD_PERM,      // GIOP 1.2
    LOC_SYSTEM_EXCEPTION,     // GIOP 1.2
    LOC_NEEDS_ADDRESSING_MODE // GIOP 1.2
  };

  struct LocateReplyHeader {
    _CORBA_ULong	request_id;
    LocateStatusType	locate_status;
  };
};


#endif // __OMNIORB_GIOP_H__
