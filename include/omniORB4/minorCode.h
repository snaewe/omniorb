// -*- Mode: C++; -*-
//                            Package   : omniORB
// minorCode.h                Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
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
//	*** PROPRIETORY INTERFACE ***
//

/*
  $Log$
  Revision 1.1.4.2  2001/05/11 14:28:14  sll
  Separate  minor code MARSHAL_MessageSizeExceedLimit into distinct client and
  server side codes.

  Revision 1.1.4.1  2001/04/18 17:26:29  sll
  Big checkin with the brand new internal APIs.

  */

#ifndef __MINORCODE__
#define __MINORCODE__

OMNI_NAMESPACE_BEGIN(omni)

// Define all the minor codes that the ORB may use when it raises
// system exceptions

#define OMGMinorCode(x) (0x4f4d0000 | x)
#define OMNIORBMinorCode(x) (0x41540000 | x)

#define OMNIORBMinorCode_1 OMNIORBMinorCode(1)
#define OMNIORBMinorCode_2 OMNIORBMinorCode(2)
#define OMNIORBMinorCode_3 OMNIORBMinorCode(3)
#define OMNIORBMinorCode_4 OMNIORBMinorCode(4)
#define OMNIORBMinorCode_5 OMNIORBMinorCode(5)
#define OMNIORBMinorCode_6 OMNIORBMinorCode(6)
#define OMNIORBMinorCode_7 OMNIORBMinorCode(7)
#define OMNIORBMinorCode_8 OMNIORBMinorCode(8)
#define OMNIORBMinorCode_9 OMNIORBMinorCode(9)
#define OMNIORBMinorCode_10 OMNIORBMinorCode(10)
#define OMNIORBMinorCode_11 OMNIORBMinorCode(11)
#define OMNIORBMinorCode_12 OMNIORBMinorCode(12)

 
// Taken from CORBA Spec. 2.4 Table 4-3.

enum UNKNOWN_minor {
  UNKNOWN_UserException                   = OMGMinorCode(1),
  UNKNOWN_SystemException                 = OMGMinorCode(2)
};

enum BAD_PARAM_minor {
  // XXX TODO Give the minor code a more descriptive names.
  BAD_PARAM_1                             = OMGMinorCode(1),
  BAD_PARAM_2                             = OMGMinorCode(2),
  BAD_PARAM_3                             = OMGMinorCode(3),
  BAD_PARAM_4                             = OMGMinorCode(4),
  BAD_PARAM_5                             = OMGMinorCode(5),
  BAD_PARAM_6                             = OMGMinorCode(6),
  BAD_PARAM_7                             = OMGMinorCode(7),
  BAD_PARAM_8                             = OMGMinorCode(8),
  BAD_PARAM_9                             = OMGMinorCode(9),
  BAD_PARAM_10                            = OMGMinorCode(10),
  BAD_PARAM_11                            = OMGMinorCode(11),
  BAD_PARAM_12                            = OMGMinorCode(12),
  BAD_PARAM_13                            = OMGMinorCode(13),
  BAD_PARAM_14                            = OMGMinorCode(14),
  BAD_PARAM_15                            = OMGMinorCode(15),
  BAD_PARAM_16                            = OMGMinorCode(16),
  BAD_PARAM_17                            = OMGMinorCode(17),
  BAD_PARAM_18                            = OMGMinorCode(18),
  BAD_PARAM_19                            = OMGMinorCode(19),
  BAD_PARAM_20                            = OMGMinorCode(20),
  BAD_PARAM_21                            = OMGMinorCode(21),
  BAD_PARAM_22                            = OMGMinorCode(22),
  BAD_PARAM_23                            = OMGMinorCode(23),
  BAD_PARAM_24                            = OMGMinorCode(24),
  BAD_PARAM_25                            = OMGMinorCode(25)
};

enum IMP_LIMIT_minor {
  IMP_LIMIT_NoUsableProfile               = OMGMinorCode(1)
};

enum INV_OBJREF_minor {
  INV_OBJREF_WCharNotSupported            = OMGMinorCode(1)
};

enum MARSHAL_minor {
  MARSHAL_NoValueFactory                  = OMGMinorCode(1),
  MARSHAL_ServerRequestWrongOrder         = OMGMinorCode(2),
  MARSHAL_ServerRequestNVList             = OMGMinorCode(3),
  MARSHAL_LocalObject                     = OMGMinorCode(4),
  MARSHAL_InvalidVariableLenComponentSize = OMNIORBMinorCode_9,
  MARSHAL_PassEndOfMessage                = OMNIORBMinorCode_10,
  MARSHAL_MessageSizeExceedLimitOnClient  = OMNIORBMinorCode_11,
  MARSHAL_MessageSizeExceedLimitOnServer  = OMNIORBMinorCode_12
};

enum BAD_TYPECODE_minor {
  BAD_TYPECODE_InComplete                 = OMGMinorCode(1),
  BAD_TYPECODE_IllegitimateMember         = OMGMinorCode(2)
};


enum COMM_FAILURE_minor {
  COMM_FAILURE_MarshalArguments           = OMNIORBMinorCode_3,
  COMM_FAILURE_UnMarshalArguments         = OMNIORBMinorCode_4,
  COMM_FAILURE_MarshalResults             = OMNIORBMinorCode_5,
  COMM_FAILURE_UnMarshalResults           = OMNIORBMinorCode_6,
  COMM_FAILURE_WaitingForReply            = OMNIORBMinorCode_7
};

inline CORBA::Boolean is_COMM_FAILURE_minor(CORBA::ULong w) {
  return (w >= COMM_FAILURE_MarshalResults &&
	  w <= COMM_FAILURE_WaitingForReply);
}

enum NO_IMPLEMENT_minor {
  NO_IMPLEMENT_NoValueImpl                = OMGMinorCode(1),
  NO_IMPLEMENT_IncompatibleVersion        = OMGMinorCode(2),
  NO_IMPLEMENT_NoUsableProfile            = OMGMinorCode(3),
  NO_IMPLEMENT_DIIOnLocalObject           = OMGMinorCode(4)
};

enum BAD_INV_ORDER_minor {
  BAD_INV_ORDER_1                         = OMGMinorCode(1),
  BAD_INV_ORDER_2                         = OMGMinorCode(2),
  BAD_INV_ORDER_WouldDeadLock             = OMGMinorCode(3),
  BAD_INV_ORDER_ORBHasShutdown            = OMGMinorCode(4),
  BAD_INV_ORDER_5                         = OMGMinorCode(5),
  BAD_INV_ORDER_6                         = OMGMinorCode(6),
  BAD_INV_ORDER_7                         = OMGMinorCode(7),
  BAD_INV_ORDER_8                         = OMGMinorCode(8),
  BAD_INV_ORDER_9                         = OMGMinorCode(9),
  BAD_INV_ORDER_10                        = OMGMinorCode(10),
  BAD_INV_ORDER_11                        = OMGMinorCode(11),
  BAD_INV_ORDER_12                        = OMGMinorCode(12),
  BAD_INV_ORDER_13                        = OMGMinorCode(13)
};

enum TRANSIENT_minor {
  TRANSIENT_POANoResource                 = OMGMinorCode(1),
  TRANSIENT_NoUsableProfile               = OMGMinorCode(2),
  TRANSIENT_FailedOnForwarded             = OMNIORBMinorCode_1,
  TRANSIENT_ConnectFailed                 = OMNIORBMinorCode_2,
  TRANSIENT_CallTimedout                  = OMNIORBMinorCode_8
};

enum OBJ_ADAPTER_minor {
  OBJ_ADAPTER_POAUnknownAdapter           = OMGMinorCode(1),
  OBJ_ADAPTER_NoServant                   = OMGMinorCode(2),
  OBJ_ADAPTER_NoDefaultServant            = OMGMinorCode(3),
  OBJ_ADAPTER_NoServantManager            = OMGMinorCode(4),
  OBJ_ADAPTER_WrongIncarnatePolicy        = OMGMinorCode(5)
};


enum DATA_CONVERSION_minor {
  DATA_CONVERSION_CannotMapChar           = OMGMinorCode(1)
};

enum OBJECT_NOT_EXIST_minor {
  OBJECT_NOT_EXIST_NoMatch                = OMGMinorCode(1),
  Object_Not_Exist_IncarnateFailed        = OMGMinorCode(2)
};



OMNI_NAMESPACE_END(omni)

#endif // __MINORCODE__
