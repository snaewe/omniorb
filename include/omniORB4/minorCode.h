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
  Revision 1.1.6.2  2003/05/20 16:53:12  dgrisby
  Valuetype marshalling support.

  Revision 1.1.6.1  2003/03/23 21:04:16  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.15  2003/03/10 11:13:17  dgrisby
  BAD_PARAM with invalid fixed limits.

  Revision 1.1.4.14  2002/11/22 18:11:58  dgrisby
  Correct COMM_FAILURE minor code check.

  Revision 1.1.4.13  2002/02/26 14:08:34  dpg1
  Minor codes for omniORBpy.

  Revision 1.1.4.12  2002/01/21 11:51:52  dpg1
  Add TRANSIENT minor code for Python exception.

  Revision 1.1.4.11  2001/10/29 17:42:35  dpg1
  Support forward-declared structs/unions, ORB::create_recursive_tc().

  Revision 1.1.4.10  2001/10/17 16:47:07  dpg1
  New minor codes

  Revision 1.1.4.9  2001/09/24 10:41:07  dpg1
  Minor codes for Dynamic library and omniORBpy.

  Revision 1.1.4.8  2001/09/12 19:44:27  sll
  Added back MARSHAL_MessageSizeExceedLimit to report the error when it is
  not possible to tell whether this is the client or the server side.

  Revision 1.1.4.7  2001/08/15 10:15:45  dpg1
  New minor codes for POA and DSI errors.

  Revision 1.1.4.6  2001/08/03 17:49:33  sll
  New minor code and lookup table for translating a minor code to its memonic
  names.

  Revision 1.1.4.5  2001/07/31 16:38:43  sll
  Added GIOP BiDir related error code.

  Revision 1.1.4.4  2001/06/11 17:59:21  sll
  New minor code to indicate error in initialising a transport.

  Revision 1.1.4.3  2001/06/08 17:12:08  dpg1
  Merge all the bug fixes from omni3_develop.

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
#define OMNIORBMinorCode_13 OMNIORBMinorCode(13)
#define OMNIORBMinorCode_14 OMNIORBMinorCode(14)
#define OMNIORBMinorCode_15 OMNIORBMinorCode(15)
#define OMNIORBMinorCode_16 OMNIORBMinorCode(16)
#define OMNIORBMinorCode_17 OMNIORBMinorCode(17)
#define OMNIORBMinorCode_18 OMNIORBMinorCode(18)
#define OMNIORBMinorCode_19 OMNIORBMinorCode(19)
#define OMNIORBMinorCode_20 OMNIORBMinorCode(20)
#define OMNIORBMinorCode_21 OMNIORBMinorCode(21)
#define OMNIORBMinorCode_22 OMNIORBMinorCode(22)
#define OMNIORBMinorCode_23 OMNIORBMinorCode(23)
#define OMNIORBMinorCode_24 OMNIORBMinorCode(24)
#define OMNIORBMinorCode_25 OMNIORBMinorCode(25)
#define OMNIORBMinorCode_26 OMNIORBMinorCode(26)
#define OMNIORBMinorCode_27 OMNIORBMinorCode(27)
#define OMNIORBMinorCode_28 OMNIORBMinorCode(28)
#define OMNIORBMinorCode_29 OMNIORBMinorCode(29)
#define OMNIORBMinorCode_30 OMNIORBMinorCode(30)
#define OMNIORBMinorCode_31 OMNIORBMinorCode(31)
#define OMNIORBMinorCode_32 OMNIORBMinorCode(32)
#define OMNIORBMinorCode_33 OMNIORBMinorCode(33)
#define OMNIORBMinorCode_34 OMNIORBMinorCode(34)
#define OMNIORBMinorCode_35 OMNIORBMinorCode(35)
#define OMNIORBMinorCode_36 OMNIORBMinorCode(36)
#define OMNIORBMinorCode_37 OMNIORBMinorCode(37)
#define OMNIORBMinorCode_38 OMNIORBMinorCode(38)
#define OMNIORBMinorCode_39 OMNIORBMinorCode(39)
#define OMNIORBMinorCode_40 OMNIORBMinorCode(40)
#define OMNIORBMinorCode_41 OMNIORBMinorCode(41)
#define OMNIORBMinorCode_42 OMNIORBMinorCode(42)
#define OMNIORBMinorCode_43 OMNIORBMinorCode(43)
#define OMNIORBMinorCode_44 OMNIORBMinorCode(44)
#define OMNIORBMinorCode_45 OMNIORBMinorCode(45)
#define OMNIORBMinorCode_46 OMNIORBMinorCode(46)
#define OMNIORBMinorCode_47 OMNIORBMinorCode(47)
#define OMNIORBMinorCode_48 OMNIORBMinorCode(48)
#define OMNIORBMinorCode_49 OMNIORBMinorCode(49)
#define OMNIORBMinorCode_50 OMNIORBMinorCode(50)
#define OMNIORBMinorCode_51 OMNIORBMinorCode(51)
#define OMNIORBMinorCode_52 OMNIORBMinorCode(52)
#define OMNIORBMinorCode_53 OMNIORBMinorCode(53)
#define OMNIORBMinorCode_54 OMNIORBMinorCode(54)
#define OMNIORBMinorCode_55 OMNIORBMinorCode(55)
#define OMNIORBMinorCode_56 OMNIORBMinorCode(56)
#define OMNIORBMinorCode_57 OMNIORBMinorCode(57)
#define OMNIORBMinorCode_58 OMNIORBMinorCode(58)
#define OMNIORBMinorCode_59 OMNIORBMinorCode(59)
#define OMNIORBMinorCode_60 OMNIORBMinorCode(60)
#define OMNIORBMinorCode_61 OMNIORBMinorCode(61)
#define OMNIORBMinorCode_62 OMNIORBMinorCode(62)
#define OMNIORBMinorCode_63 OMNIORBMinorCode(63)
#define OMNIORBMinorCode_64 OMNIORBMinorCode(64)
#define OMNIORBMinorCode_65 OMNIORBMinorCode(65)
#define OMNIORBMinorCode_66 OMNIORBMinorCode(66)
#define OMNIORBMinorCode_67 OMNIORBMinorCode(67)
#define OMNIORBMinorCode_68 OMNIORBMinorCode(68)
#define OMNIORBMinorCode_69 OMNIORBMinorCode(69)
#define OMNIORBMinorCode_70 OMNIORBMinorCode(70)
#define OMNIORBMinorCode_71 OMNIORBMinorCode(71)
#define OMNIORBMinorCode_72 OMNIORBMinorCode(72)
#define OMNIORBMinorCode_73 OMNIORBMinorCode(73)
#define OMNIORBMinorCode_74 OMNIORBMinorCode(74)
#define OMNIORBMinorCode_75 OMNIORBMinorCode(75)
#define OMNIORBMinorCode_76 OMNIORBMinorCode(76)
#define OMNIORBMinorCode_77 OMNIORBMinorCode(77)
#define OMNIORBMinorCode_78 OMNIORBMinorCode(78)
#define OMNIORBMinorCode_79 OMNIORBMinorCode(79)
#define OMNIORBMinorCode_80 OMNIORBMinorCode(80)
#define OMNIORBMinorCode_81 OMNIORBMinorCode(81)
#define OMNIORBMinorCode_82 OMNIORBMinorCode(82)
#define OMNIORBMinorCode_83 OMNIORBMinorCode(83)
#define OMNIORBMinorCode_84 OMNIORBMinorCode(84)
#define OMNIORBMinorCode_85 OMNIORBMinorCode(85)
#define OMNIORBMinorCode_86 OMNIORBMinorCode(86)
#define OMNIORBMinorCode_87 OMNIORBMinorCode(87)
#define OMNIORBMinorCode_88 OMNIORBMinorCode(88)
#define OMNIORBMinorCode_89 OMNIORBMinorCode(89)
#define OMNIORBMinorCode_90 OMNIORBMinorCode(90)
#define OMNIORBMinorCode_91 OMNIORBMinorCode(91)
#define OMNIORBMinorCode_92 OMNIORBMinorCode(92)
#define OMNIORBMinorCode_93 OMNIORBMinorCode(93)
#define OMNIORBMinorCode_94 OMNIORBMinorCode(94)
#define OMNIORBMinorCode_95 OMNIORBMinorCode(95)
#define OMNIORBMinorCode_96 OMNIORBMinorCode(96)
#define OMNIORBMinorCode_97 OMNIORBMinorCode(97)
#define OMNIORBMinorCode_98 OMNIORBMinorCode(98)
#define OMNIORBMinorCode_99 OMNIORBMinorCode(99)
#define OMNIORBMinorCode_100 OMNIORBMinorCode(100)
#define OMNIORBMinorCode_101 OMNIORBMinorCode(101)
#define OMNIORBMinorCode_102 OMNIORBMinorCode(102)
#define OMNIORBMinorCode_103 OMNIORBMinorCode(103)
#define OMNIORBMinorCode_104 OMNIORBMinorCode(104)
#define OMNIORBMinorCode_105 OMNIORBMinorCode(105)
#define OMNIORBMinorCode_106 OMNIORBMinorCode(106)
#define OMNIORBMinorCode_107 OMNIORBMinorCode(107)
#define OMNIORBMinorCode_108 OMNIORBMinorCode(108)
#define OMNIORBMinorCode_109 OMNIORBMinorCode(109)
#define OMNIORBMinorCode_110 OMNIORBMinorCode(110)
#define OMNIORBMinorCode_111 OMNIORBMinorCode(111)
#define OMNIORBMinorCode_112 OMNIORBMinorCode(112)
#define OMNIORBMinorCode_113 OMNIORBMinorCode(113)
#define OMNIORBMinorCode_114 OMNIORBMinorCode(114)
#define OMNIORBMinorCode_115 OMNIORBMinorCode(115)

#define COMMA ,
#define DeclareValue(name,value) name = value

// Taken from CORBA Spec. 2.4 Table 4-3.

//  UNKNOWN
#define DECLARE_UNKNOWN_minors(code,sep) \
\
code( UNKNOWN_UserException,   OMGMinorCode(1) ) sep \
code( UNKNOWN_SystemException, OMGMinorCode(2) ) sep \
code( UNKNOWN_PythonException, OMNIORBMinorCode_98 )

enum UNKNOWN_minor {
  DECLARE_UNKNOWN_minors(DeclareValue,COMMA)
};

// BAD_PARAM
#define DECLARE_BAD_PARAM_minors(code,sep) \
\
code( BAD_PARAM_ValueFactoryFailure  	     , OMGMinorCode(1) ) sep \
code( BAD_PARAM_RIDAlreadyDefinedInIfR	     , OMGMinorCode(2) ) sep \
code( BAD_PARAM_NameAlreadyUsedInIfR 	     , OMGMinorCode(3) ) sep \
code( BAD_PARAM_TargetIsInvalidContainer     , OMGMinorCode(4) ) sep \
code( BAD_PARAM_InheritedNameClash   	     , OMGMinorCode(5) ) sep \
code( BAD_PARAM_IncorrectAbstractIntfType    , OMGMinorCode(6) ) sep \
code( BAD_PARAM_BadSchemeName        	     , OMGMinorCode(7) ) sep \
code( BAD_PARAM_BadAddress           	     , OMGMinorCode(8) ) sep \
code( BAD_PARAM_BadSchemeSpecificPart	     , OMGMinorCode(9) ) sep \
code( BAD_PARAM_BadURIOther          	     , OMGMinorCode(10) ) sep \
code( BAD_PARAM_NonAbstractBase      	     , OMGMinorCode(11) ) sep \
code( BAD_PARAM_SupportsTooManyConcreteIntfs , OMGMinorCode(12) ) sep \
code( BAD_PARAM_IncompleteTypeCode   	     , OMGMinorCode(13) ) sep \
code( BAD_PARAM_InvalidObjectId      	     , OMGMinorCode(14) ) sep \
code( BAD_PARAM_InvalidName          	     , OMGMinorCode(15) ) sep \
code( BAD_PARAM_InvalidRepositoryId   	     , OMGMinorCode(16) ) sep \
code( BAD_PARAM_InvalidMemberName    	     , OMGMinorCode(17) ) sep \
code( BAD_PARAM_DuplicateLabelValue  	     , OMGMinorCode(18) ) sep \
code( BAD_PARAM_IncompatibleDiscriminatorType, OMGMinorCode(19) ) sep \
code( BAD_PARAM_IllegitimateDiscriminatorType, OMGMinorCode(20) ) sep \
code( BAD_PARAM_NotAnException       	     , OMGMinorCode(21) ) sep \
code( BAD_PARAM_UnlistedUserException	     , OMGMinorCode(22) ) sep \
code( BAD_PARAM_WCharTCSNotKnown     	     , OMGMinorCode(23) ) sep \
code( BAD_PARAM_ServiceContextNotInRange     , OMGMinorCode(24) ) sep \
code( BAD_PARAM_EnumValueOutOfRange  	     , OMGMinorCode(25) ) sep \
code( BAD_PARAM_InvalidServiceCtxtId         , OMGMinorCode(26) ) sep \
code( BAD_PARAM_RegisterNilObject            , OMGMinorCode(27) ) sep \
code( BAD_PARAM_InvalidComponentId           , OMGMinorCode(28) ) sep \
code( BAD_PARAM_InvalidProfileId             , OMGMinorCode(29) ) sep \
code( BAD_PARAM_DuplicatePolicyType          , OMGMinorCode(30) ) sep \
code( BAD_PARAM_IndexOutOfRange      	     , OMNIORBMinorCode_21 ) sep \
code( BAD_PARAM_InvalidUnionDiscValue	     , OMNIORBMinorCode_27 ) sep \
code( BAD_PARAM_InvalidInitialSize   	     , OMNIORBMinorCode_29 ) sep \
code( BAD_PARAM_InvalidServant       	     , OMNIORBMinorCode_35 ) sep \
code( BAD_PARAM_IsPseudoObject       	     , OMNIORBMinorCode_37 ) sep \
code( BAD_PARAM_InvalidObjectRef     	     , OMNIORBMinorCode_43 ) sep \
code( BAD_PARAM_WCharOutOfRange      	     , OMNIORBMinorCode_46 ) sep \
code( BAD_PARAM_InternalInvariant    	     , OMNIORBMinorCode_49 ) sep \
code( BAD_PARAM_NullStringUnexpected 	     , OMNIORBMinorCode_57 ) sep \
code( BAD_PARAM_InvalidPOAName       	     , OMNIORBMinorCode_59 ) sep \
code( BAD_PARAM_LocalObjectExpected  	     , OMNIORBMinorCode_60 ) sep \
code( BAD_PARAM_InvalidSystemId      	     , OMNIORBMinorCode_61 ) sep \
code( BAD_PARAM_InvalidNVList        	     , OMNIORBMinorCode_64 ) sep \
code( BAD_PARAM_InvalidTypeCode      	     , OMNIORBMinorCode_70 ) sep \
code( BAD_PARAM_AnyDoesNotContainAString     , OMNIORBMinorCode_72 ) sep \
code( BAD_PARAM_EmptyContextPattern          , OMNIORBMinorCode_73 ) sep \
code( BAD_PARAM_InvalidContextName           , OMNIORBMinorCode_74 ) sep \
code( BAD_PARAM_InvalidContext               , OMNIORBMinorCode_77 ) sep \
code( BAD_PARAM_InvalidDynAny                , OMNIORBMinorCode_78 ) sep \
code( BAD_PARAM_InvalidException             , OMNIORBMinorCode_80 ) sep \
code( BAD_PARAM_InvalidExceptionList         , OMNIORBMinorCode_81 ) sep \
code( BAD_PARAM_InvalidEnvironment           , OMNIORBMinorCode_82 ) sep \
code( BAD_PARAM_InvalidAny                   , OMNIORBMinorCode_83 ) sep \
code( BAD_PARAM_InvalidNamedValue            , OMNIORBMinorCode_84 ) sep \
code( BAD_PARAM_InvalidRequest               , OMNIORBMinorCode_85 ) sep \
code( BAD_PARAM_InvalidContextList           , OMNIORBMinorCode_87 ) sep \
code( BAD_PARAM_WrongPythonType      	     , OMNIORBMinorCode_88 ) sep \
code( BAD_PARAM_IncompletePythonType 	     , OMNIORBMinorCode_92 ) sep \
code( BAD_PARAM_PythonValueOutOfRange	     , OMNIORBMinorCode_95 ) sep \
code( BAD_PARAM_EmbeddedNullInPythonString   , OMNIORBMinorCode_96 ) sep \
code( BAD_PARAM_StringIsTooLong              , OMNIORBMinorCode_100 ) sep \
code( BAD_PARAM_SequenceIsTooLong            , OMNIORBMinorCode_101 ) sep \
code( BAD_PARAM_WrongUnionMemberSelected     , OMNIORBMinorCode_107 ) sep \
code( BAD_PARAM_InvalidFixedPointLimits      , OMNIORBMinorCode_109 ) sep \
code( BAD_PARAM_AttemptToMarshalAbstractValue, OMNIORBMinorCode_115 )


enum BAD_PARAM_minor {
  DECLARE_BAD_PARAM_minors(DeclareValue,COMMA)
};

// IMP_LIMIT
#define DECLARE_IMP_LIMIT_minors(code,sep) \
\
code( IMP_LIMIT_NoUsableProfile      , OMGMinorCode(1) )

enum IMP_LIMIT_minor {
  DECLARE_IMP_LIMIT_minors(DeclareValue,COMMA)
};

// INV_OBJREF
#define DECLARE_INV_OBJREF_minors(code,sep) \
\
code( INV_OBJREF_WCharNotSupported        , OMGMinorCode(1) ) sep \
code( INV_OBJREF_CodesetComponentRequired , OMGMinorCode(2) ) sep \
code( INV_OBJREF_TryToInvokePseudoRemotely, OMNIORBMinorCode_23 ) sep \
code( INV_OBJREF_InvokeOnNilObjRef  	  , OMNIORBMinorCode_48 ) sep \
code( INV_OBJREF_CorruptedObjRef    	  , OMNIORBMinorCode_54 ) sep \
code( INV_OBJREF_InterfaceMisMatch  	  , OMNIORBMinorCode_55 ) sep \
code( INV_OBJREF_NoPythonTypeForPseudoObj , OMNIORBMinorCode_104 )


 
enum INV_OBJREF_minor {
  DECLARE_INV_OBJREF_minors(DeclareValue,COMMA)
};

// MARSHAL
#define DECLARE_MARSHAL_minors(code,sep) \
\
code( MARSHAL_NoValueFactory                  , OMGMinorCode(1) ) sep \
code( MARSHAL_ServerRequestWrongOrder         , OMGMinorCode(2) ) sep \
code( MARSHAL_ServerRequestNVList             , OMGMinorCode(3) ) sep \
code( MARSHAL_LocalObject                     , OMGMinorCode(4) ) sep \
code( MARSHAL_WCharSentByGIOP10Client         , OMGMinorCode(5) ) sep \
code( MARSHAL_WCharSentByGIOP10Server         , OMGMinorCode(6) ) sep \
code( MARSHAL_InvalidVariableLenComponentSize , OMNIORBMinorCode_9 ) sep \
code( MARSHAL_PassEndOfMessage                , OMNIORBMinorCode_10 ) sep \
code( MARSHAL_MessageSizeExceedLimit          , OMNIORBMinorCode_69 ) sep \
code( MARSHAL_MessageSizeExceedLimitOnClient  , OMNIORBMinorCode_11 ) sep \
code( MARSHAL_MessageSizeExceedLimitOnServer  , OMNIORBMinorCode_12 ) sep \
code( MARSHAL_SequenceIsTooLong               , OMNIORBMinorCode_18 ) sep \
code( MARSHAL_StringIsTooLong                 , OMNIORBMinorCode_19 ) sep \
code( MARSHAL_WStringIsTooLong                , OMNIORBMinorCode_20 ) sep \
code( MARSHAL_StringNotEndWithNull            , OMNIORBMinorCode_22 ) sep \
code( MARSHAL_InvalidEnumValue                , OMNIORBMinorCode_26 ) sep \
code( MARSHAL_AttemptToWriteToReadOnlyBuf     , OMNIORBMinorCode_28 ) sep \
code( MARSHAL_InvalidFixedValue               , OMNIORBMinorCode_42 ) sep \
code( MARSHAL_InvalidWCharSize                , OMNIORBMinorCode_47 ) sep \
code( MARSHAL_InvalidIOR                      , OMNIORBMinorCode_52 ) sep \
code( MARSHAL_ExceptionInDSINotPropagated     , OMNIORBMinorCode_67 ) sep \
code( MARSHAL_InvalidContextList              , OMNIORBMinorCode_79 ) sep \
code( MARSHAL_InvalidIndirection              , OMNIORBMinorCode_90 ) sep \
code( MARSHAL_InvalidTypeCodeKind             , OMNIORBMinorCode_91 ) sep \
code( MARSHAL_MessageTooLong                  , OMNIORBMinorCode_93 ) sep \
code( MARSHAL_CannotReserveOutputSpace        , OMNIORBMinorCode_110 ) sep \
code( MARSHAL_InvalidChunkedEncoding          , OMNIORBMinorCode_111 ) sep \
code( MARSHAL_InvalidValueTag                 , OMNIORBMinorCode_112 ) sep \
code( MARSHAL_NoRepoIdInValueType             , OMNIORBMinorCode_113 ) sep \
code( MARSHAL_TypeIsNotAValueType             , OMNIORBMinorCode_114 )



enum MARSHAL_minor {
  DECLARE_MARSHAL_minors(DeclareValue,COMMA)
};

// BAD_TYPECODE
#define DECLARE_BAD_TYPECODE_minors(code,sep) \
\
code( BAD_TYPECODE_Incomplete        	, OMGMinorCode(1) ) sep \
code( BAD_TYPECODE_IllegitimateMember	, OMGMinorCode(2) ) sep \
code( BAD_TYPECODE_TypeCodeIsNil     	, OMNIORBMinorCode_71 ) sep \
code( BAD_TYPECODE_InvalidOperation  	, OMNIORBMinorCode_89 ) sep \
code( BAD_TYPECODE_UnknownKind          , OMNIORBMinorCode_94 ) sep \
code( BAD_TYPECODE_UnresolvedRecursiveTC, OMNIORBMinorCode_105 ) sep \
code( BAD_TYPECODE_InvalidIndirection   , OMNIORBMinorCode_108 )

enum BAD_TYPECODE_minor {
  DECLARE_BAD_TYPECODE_minors(DeclareValue,COMMA)  
};

// NO_IMPLEMENT
#define DECLARE_NO_IMPLEMENT_minors(code,sep) \
\
code( NO_IMPLEMENT_NoValueImpl        , OMGMinorCode(1) ) sep \
code( NO_IMPLEMENT_IncompatibleVersion, OMGMinorCode(2) ) sep \
code( NO_IMPLEMENT_NoUsableProfile    , OMGMinorCode(3) ) sep \
code( NO_IMPLEMENT_DIIOnLocalObject   , OMGMinorCode(4) ) sep \
code( NO_IMPLEMENT_Unsupported        , OMNIORBMinorCode_36 ) sep \
code( NO_IMPLEMENT_NoPythonMethod     , OMNIORBMinorCode_99 )

enum NO_IMPLEMENT_minor {
  DECLARE_NO_IMPLEMENT_minors(DeclareValue,COMMA)  
};

// NO_RESOURCES
#define DECLARE_NO_RESOURCES_minors(code,sep) \
\
code ( NO_RESOURCES_PIOperationNotSupported, OMGMinorCode(1) ) sep \
code ( NO_RESOURCES_CodeSetNotSupported    , OMNIORBMinorCode_25 ) sep \
code ( NO_RESOURCES_InitialRefNotFound     , OMNIORBMinorCode_51 )

enum NO_RESOURCES_minor {
  DECLARE_NO_RESOURCES_minors(DeclareValue,COMMA)  
};

// BAD_INV_ORDER
#define DECLARE_BAD_INV_ORDER_minors(code,sep) \
\
code( BAD_INV_ORDER_DependencyPreventsDestruction , OMGMinorCode(1) ) sep \
code( BAD_INV_ORDER_ObjectIndestructible     	  , OMGMinorCode(2) ) sep \
code( BAD_INV_ORDER_WouldDeadLock     	     	  , OMGMinorCode(3) ) sep \
code( BAD_INV_ORDER_ORBHasShutdown    	     	  , OMGMinorCode(4) ) sep \
code( BAD_INV_ORDER_RequestUsedMoreThanOnce  	  , OMGMinorCode(5) ) sep \
code( BAD_INV_ORDER_ServantManagerAlreadySet 	  , OMGMinorCode(6) ) sep \
code( BAD_INV_ORDER_ArgumentsCalledOutOfOrder	  , OMGMinorCode(7) ) sep \
code( BAD_INV_ORDER_CtxCalledOutOfOrder	     	  , OMGMinorCode(8) ) sep \
code( BAD_INV_ORDER_SetResultCalledOutOfOrder	  , OMGMinorCode(9) ) sep \
code( BAD_INV_ORDER_RequestAlreadySent    	  , OMGMinorCode(10) ) sep \
code( BAD_INV_ORDER_RequestNotSentYet        	  , OMGMinorCode(11) ) sep \
code( BAD_INV_ORDER_ResultAlreadyReceived    	  , OMGMinorCode(12) ) sep \
code( BAD_INV_ORDER_RequestIsSynchronous     	  , OMGMinorCode(13) ) sep \
code( BAD_INV_ORDER_InvalidPortableInterceptorCall, OMGMinorCode(14) ) sep \
code( BAD_INV_ORDER_ServiceContextIdAlreadyExists , OMGMinorCode(15) ) sep \
code( BAD_INV_ORDER_PolicyFactoryTypeAlreadyExists, OMGMinorCode(16) ) sep \
code( BAD_INV_ORDER_POACreationDuringDestruction  , OMGMinorCode(17) ) sep \
code( BAD_INV_ORDER_CodeSetNotKnownYet	     	  , OMNIORBMinorCode_24 ) sep \
code( BAD_INV_ORDER_ArgumentsNotCalled            , OMNIORBMinorCode_65 ) sep \
code( BAD_INV_ORDER_ErrorInDynamicImplementation  , OMNIORBMinorCode_66 ) sep \
code( BAD_INV_ORDER_RequestConfiguredOutOfOrder   , OMNIORBMinorCode_86 ) sep \
code( BAD_INV_ORDER_DynAnyNotInitialised          , OMNIORBMinorCode_103 )

enum BAD_INV_ORDER_minor {
  DECLARE_BAD_INV_ORDER_minors(DeclareValue,COMMA)  
};

// TRANSIENT
#define DECLARE_TRANSIENT_minors(code,sep) \
\
code( TRANSIENT_POANoResource         	  , OMGMinorCode(1) ) sep \
code( TRANSIENT_NoUsableProfile       	  , OMGMinorCode(2) ) sep \
code( TRANSIENT_RequestCancelled      	  , OMGMinorCode(3) ) sep \
code( TRANSIENT_POADestroyed          	  , OMGMinorCode(4) ) sep \
code( TRANSIENT_FailedOnForwarded     	  , OMNIORBMinorCode_1 ) sep \
code( TRANSIENT_ConnectFailed         	  , OMNIORBMinorCode_2 ) sep \
code( TRANSIENT_CallTimedout          	  , OMNIORBMinorCode_8 ) sep \
code( TRANSIENT_BiDirConnIsGone       	  , OMNIORBMinorCode_14 ) sep \
code( TRANSIENT_BiDirConnUsedWithNoPOA	  , OMNIORBMinorCode_16 ) sep \
code( TRANSIENT_ConnectionClosed      	  , OMNIORBMinorCode_17 ) sep \
code( TRANSIENT_ObjDeactivatedWhileHolding, OMNIORBMinorCode_62 ) sep \
code( TRANSIENT_PythonExceptionInORB      , OMNIORBMinorCode_106 )

enum TRANSIENT_minor {
  DECLARE_TRANSIENT_minors(DeclareValue,COMMA)  
};

// OBJ_ADAPTER
#define DECLARE_OBJ_ADAPTER_minors(code,sep) \
\
code( OBJ_ADAPTER_POAUnknownAdapter   , OMGMinorCode(1) ) sep \
code( OBJ_ADAPTER_NoServant           , OMGMinorCode(2) ) sep \
code( OBJ_ADAPTER_NoDefaultServant    , OMGMinorCode(3) ) sep \
code( OBJ_ADAPTER_NoServantManager    , OMGMinorCode(4) ) sep \
code( OBJ_ADAPTER_WrongIncarnatePolicy, OMGMinorCode(5) ) sep \
code( OBJ_ADAPTER_BiDirNotAllowed     , OMNIORBMinorCode_15 ) sep \
code( OBJ_ADAPTER_BOANotInitialised   , OMNIORBMinorCode_33 ) sep \
code( OBJ_ADAPTER_POANotInitialised   , OMNIORBMinorCode_53 ) sep \
code( OBJ_ADAPTER_ServantAlreadyActive, OMNIORBMinorCode_63 ) sep \
code( OBJ_ADAPTER_IncompatibleServant , OMNIORBMinorCode_97 )

enum OBJ_ADAPTER_minor {
  DECLARE_OBJ_ADAPTER_minors(DeclareValue,COMMA)  
};

// DATA_CONVERSION
#define DECLARE_DATA_CONVERSION_minors(code,sep) \
\
code( DATA_CONVERSION_CannotMapChar   , OMGMinorCode(1) ) sep \
code( DATA_CONVERSION_RangeError      , OMNIORBMinorCode_40 ) sep \
code( DATA_CONVERSION_BadInput        , OMNIORBMinorCode_41 )

enum DATA_CONVERSION_minor {
  DECLARE_DATA_CONVERSION_minors(DeclareValue,COMMA)  
};

// OBJECT_NOT_EXIST
#define DECLARE_OBJECT_NOT_EXIST_minors(code,sep) \
\
code( OBJECT_NOT_EXIST_NoMatch           , OMGMinorCode(1) ) sep \
code( OBJECT_NOT_EXIST_IncarnateFailed   , OMGMinorCode(2) ) sep \
code( OBJECT_NOT_EXIST_BOANotInitialised , OMNIORBMinorCode_34 ) sep \
code( OBJECT_NOT_EXIST_POANotInitialised , OMNIORBMinorCode_58 ) sep \
code( OBJECT_NOT_EXIST_DynAnyDestroyed   , OMNIORBMinorCode_102 )

enum OBJECT_NOT_EXIST_minor {
  DECLARE_OBJECT_NOT_EXIST_minors(DeclareValue,COMMA)  
};

// INV_POLICY
#define DECLARE_INV_POLICY_minors(code,sep) \
\
code( INV_POLICY_CannotReconcileIORWithOverride , OMGMinorCode(1) ) sep \
code( INV_POLICY_InvalidPolicyType              , OMGMinorCode(2) ) sep \
code( INV_POLICY_NoPolicyFactoryForPolicyType   , OMGMinorCode(3) )

enum INV_POLICY_minor {
  DECLARE_INV_POLICY_minors(DeclareValue,COMMA)  
};


// BAD_OPERATION
#define DECLARE_BAD_OPERATION_minors(code,sep) \
\
code( BAD_OPERATION_UnRecognisedOperationName, OMNIORBMinorCode_38 )

enum BAD_OPERATION_minor {
  DECLARE_BAD_OPERATION_minors(DeclareValue,COMMA)  
};


// COMM_FAILURE
#define DECLARE_COMM_FAILURE_minors(code,sep) \
\
code( COMM_FAILURE_MarshalArguments   , OMNIORBMinorCode_3 ) sep \
code( COMM_FAILURE_UnMarshalArguments , OMNIORBMinorCode_4 ) sep \
code( COMM_FAILURE_MarshalResults     , OMNIORBMinorCode_5 ) sep \
code( COMM_FAILURE_UnMarshalResults   , OMNIORBMinorCode_6 ) sep \
code( COMM_FAILURE_WaitingForReply    , OMNIORBMinorCode_7 )

enum COMM_FAILURE_minor {
  DECLARE_COMM_FAILURE_minors(DeclareValue,COMMA)  
};

// INTF_REPOS
#define DECLARE_INTF_REPOS_minors(code,sep) \
\
code( INTF_REPOS_NotAvailable                , OMNIORBMinorCode_39 ) sep \
code( INTF_REPOS_PrimaryInterfaceReturnedZero, OMNIORBMinorCode_68 )

enum INTF_REPOS_minor {
  DECLARE_INTF_REPOS_minors(DeclareValue,COMMA)  
};

// INITIALIZE
#define DECLARE_INITIALIZE_minors(code,sep) \
\
code( INITIALIZE_TransportError       , OMNIORBMinorCode_13 ) sep \
code( INITIALIZE_InvalidORBInitArgs   , OMNIORBMinorCode_30 ) sep \
code( INITIALIZE_FailedBOAInit        , OMNIORBMinorCode_31 ) sep \
code( INITIALIZE_FailedPOAInit        , OMNIORBMinorCode_32 ) sep \
code( INITIALIZE_FailedORBInit        , OMNIORBMinorCode_44 ) sep \
code( INITIALIZE_FailedLoadLibrary    , OMNIORBMinorCode_45 ) sep \
code( INITIALIZE_ConfigFileError      , OMNIORBMinorCode_50 ) sep \
code( INITIALIZE_NotOmniThread        , OMNIORBMinorCode_56 )


enum INITIALIZE_minor {
  DECLARE_INITIALIZE_minors(DeclareValue,COMMA)  
};

// BAD_CONTEXT
#define DECLARE_BAD_CONTEXT_minors(code,sep) \
\
code( BAD_CONTEXT_NoMatchingProperty    , OMNIORBMinorCode_75 ) sep \
code( BAD_CONTEXT_StartingScopeNotFound , OMNIORBMinorCode_76 )


enum BAD_CONTEXT_minor {
  DECLARE_BAD_CONTEXT_minors(DeclareValue,COMMA)
};

inline CORBA::Boolean is_COMM_FAILURE_minor(CORBA::ULong w) {
  return (w >= COMM_FAILURE_MarshalArguments &&
	  w <= COMM_FAILURE_WaitingForReply);
}


struct minorCodeLookup {
  CORBA::ULong value;
  const char*  name;
};


#define DeclareMinorCodeLookup(name) \
extern _core_attr minorCodeLookup name##_LookupTable[];

OMNIORB_FOR_EACH_SYS_EXCEPTION(DeclareMinorCodeLookup)

extern const char* minorCode2String(const minorCodeLookup table[],
				    CORBA::ULong code);
// Return the string representation of the code in the table. Return 0 if 
// the code does not have an entry in the lookup table

#undef DeclareValue
#undef DeclareMinorCodeLookup

OMNI_NAMESPACE_END(omni)


#endif // __MINORCODE__
