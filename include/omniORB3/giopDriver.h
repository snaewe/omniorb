// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopDriver.h               Created on: 6/2/96
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
  Revision 1.1.2.1  1999/09/24 09:51:45  djr
  Moved from omniORB2 + some new files.

  Revision 1.11  1999/06/26 17:55:19  sll
  Added new exception return type terminateProcessing.

  Revision 1.10  1999/06/18 21:14:44  sll
  Updated copyright notice.

  Revision 1.9  1999/06/18 20:36:18  sll
  Replaced _LC_attr with _core_attr.

  Revision 1.8  1999/01/07 18:32:42  djr
  Replaced _OMNIORB_NTDLL_IMPORT with _LC_attr.

  Revision 1.7  1998/04/07 20:03:01  sll
   Replace _OMNIORB2_NTDLL_ specification on class GIOP_basetypes
  _OMNIORB_NTDLL_IMPORT on static member constants.

  Revision 1.6  1997/08/21 22:20:50  sll
  New system exception TRANSACTION_REQUIRED, TRANSACTION_ROLLEDBACK,
  INVALID_TRANSACTION and WRONG_TRANSACION.

 * Revision 1.5  1997/05/06  16:08:20  sll
 * Public release.
 *
  */

#ifndef __GIOPDRIVER_H__
#define __GIOPDRIVER_H__

#ifndef __OMNIOBJKEY_H__
#include <omniORB3/omniObjKey.h>
#endif

// The GIOP_C and GIOP_S classes are defined to drive the General Inter-ORB
// Protocol (GIOP). The GIOP protocol is asymmetric.  GIOP_C provides the
// functions to drive the client side protocol.  GIOP_S provides the server
// side functions.
//
// GIOP_C and GIOP_S are built on top of a strand. Direct access to the strand
// can be done via the NetBufferedStream class which is a public base
// class of GIOP_C and GIOP_S. Typically clients of GIOP_C and GIOP_S uses
// the NetBufferedStream interface to marshall and unmarshall arguments.
//
// Calling the constructors of GIOP_C and GIOP_S acquire a strand.
// GIOP_C and GIOP_S implements the appropriate exclusive access to the
// strand according to their internal states.

class GIOP_Basetypes {
public:
  class MessageHeader {
  public:
    typedef _CORBA_Char HeaderType[8];
    static _core_attr const HeaderType Request;
    static _core_attr const HeaderType Reply;
    static _core_attr const HeaderType CancelRequest;
    static _core_attr const HeaderType LocateRequest;
    static _core_attr const HeaderType LocateReply;
    static _core_attr const HeaderType CloseConnection;
    static _core_attr const HeaderType MessageError;
  };
  struct _SysExceptRepoID {
    _CORBA_Char *id;
    _CORBA_ULong len;

    _SysExceptRepoID(_CORBA_Char *i) {
      id = i; len = strlen((const char *)i);
    }
  };
  class SysExceptRepoID {
  public:
    static _core_attr const _SysExceptRepoID UNKNOWN;
    static _core_attr const _SysExceptRepoID BAD_PARAM;
    static _core_attr const _SysExceptRepoID NO_MEMORY;
    static _core_attr const _SysExceptRepoID IMP_LIMIT;
    static _core_attr const _SysExceptRepoID COMM_FAILURE;
    static _core_attr const _SysExceptRepoID INV_OBJREF;
    static _core_attr const _SysExceptRepoID OBJECT_NOT_EXIST;
    static _core_attr const _SysExceptRepoID NO_PERMISSION;
    static _core_attr const _SysExceptRepoID INTERNAL;
    static _core_attr const _SysExceptRepoID MARSHAL;
    static _core_attr const _SysExceptRepoID INITIALIZE;
    static _core_attr const _SysExceptRepoID NO_IMPLEMENT;
    static _core_attr const _SysExceptRepoID BAD_TYPECODE;
    static _core_attr const _SysExceptRepoID BAD_OPERATION;
    static _core_attr const _SysExceptRepoID NO_RESOURCES;
    static _core_attr const _SysExceptRepoID NO_RESPONSE;
    static _core_attr const _SysExceptRepoID PERSIST_STORE;
    static _core_attr const _SysExceptRepoID BAD_INV_ORDER;
    static _core_attr const _SysExceptRepoID TRANSIENT;
    static _core_attr const _SysExceptRepoID FREE_MEM;
    static _core_attr const _SysExceptRepoID INV_IDENT;
    static _core_attr const _SysExceptRepoID INV_FLAG;
    static _core_attr const _SysExceptRepoID INTF_REPOS;
    static _core_attr const _SysExceptRepoID BAD_CONTEXT;
    static _core_attr const _SysExceptRepoID OBJ_ADAPTER;
    static _core_attr const _SysExceptRepoID DATA_CONVERSION;
    static _core_attr const _SysExceptRepoID TRANSACTION_REQUIRED;
    static _core_attr const _SysExceptRepoID TRANSACTION_ROLLEDBACK;
    static _core_attr const _SysExceptRepoID INVALID_TRANSACTION;
    static _core_attr const _SysExceptRepoID WRONG_TRANSACTION;
    static _core_attr const _CORBA_ULong maxIDLen;
    static _core_attr const _CORBA_Char *version;
    static _core_attr const _CORBA_ULong versionLen;
#define omniORB_GIOP_Basetypes_SysExceptRepoID_maxIDLen 46
  };
  static _core_attr size_t max_giop_message_size;
};


class GIOP_C : public GIOP_Basetypes, public NetBufferedStream {
public:

  GIOP_C(Rope *r);
  ~GIOP_C();

  enum State { Idle,
	       RequestInProgress,
	       WaitingForReply,
	       ReplyIsBeingProcessed,
               Zombie
  };


  // A GIOP_C instance operates in one of the following states:
  //
  //       Idle                  -  no request has been initiated
  //       RequestInProgress     -  a request is being composed, arguments
  //                                can be marshalled into the
  //                                NetBufferedStream
  //       WaitingForReply       -  the request has been sent, no reply
  //                                has come in yet.
  //       ReplyIsBeingProcessed -  a reply has started arriving, results
  //                                can be unmarshalled from the
  //                                NetBufferedStream
  //       Zombie                -  the destructor has been called
  //
  // Calling the member functions InitialiseRequest(), ReceiveReply() and
  // RequestCompleted() cause the GIOP_C to change state. The state transitions
  // are as follows:
  //					+------------+
  //					|	     |  ~GIOP_C()
  //				   +----+---+	     |
  //				   | Zombie |<-------+
  //				   +--------+
  //                                    ^
  //   	       	       	       	       	| ~GIOP_C()
  //				       	|
  //        RequestCompleted()     +----+---+  	   InitialiseRequest()
  //             +---------------->|  Idle  |---------------+
  //             |                 +---+----+               |
  //             |                     |                    V
  //  +-----------------------+        |             +--------------------+
  //  | ReplyIsBeingProcessed |        |             |  RequestInProgress |
  //  +-----------------------+        |             +---------+----------+
  //		 ^                     |                       |
  //             |                     |                       |
  //             |                     | IssueLocateRequest()  |
  //             |      	       |		       |
  //    	 |      	       |		       |
  //    	 |      	       |		       |
  //    	 |      	       |		       |
  //    	 |                     V                       |
  //             |         +--------------------+	       |
  //             +---------|  WaitingForReply   |<-------------+
  //     <got a proper     +--------------------+     ReceiveReply()
  //      reply header while
  //      in ReceiveReply() or IssueLocateRequest()>
  //
  // The constructor initialises a GIOP_C to Idle state.
  //
  // When the destructor is called, the state is examined. If it is *not* in
  // Idle or WaitingForReply, there is something seriously wrong with the
  // NetBufferedStream. Also the state of the strand, on which the
  // NetBufferedStream is built, is unknown. For this reason, the strand
  // is marked as dying by the destructor. The destructor may be called
  // multiple times. It simply returns if the instance is already in Zombie
  // state.
  //
  // Calling the member functions InitialiseRequest(), ReceiveReply(),
  // RequestCompleted() and IssueLocateRequest() while the GIOP_C is not 
  // in the appropriate state as indicated above would cause a 
  // CORBA::INTERNAL() exception to be raised.
  //

  void InitialiseRequest(const void*          objkey,
			 const size_t         objkeysize,
			 const char*          opname,
			 const size_t         opnamesize,
			 const size_t         msgsize,
			 const _CORBA_Boolean oneway);
  // Initialise a Request message.
  // Note: <msgsize> is the size of the whole message including the
  //       GIOP message header and the Request message header.


  GIOP::ReplyStatusType ReceiveReply();

  void RequestCompleted(_CORBA_Boolean skip=0);

  GIOP::LocateStatusType IssueLocateRequest(const void   *objkey,
					    const size_t  objkeysize);

  static size_t RequestHeaderSize(const size_t objkeysize,
				  const size_t opnamesize);
  // Return the header size. This includes the size of the GIOP message
  // header and the Request message header.

  size_t MaxMessageSize() const { return pd_max_message_size; }
  // Returns the maximum size of a GIOP message (excluding the header) that
  // can be delivered or received. This value is the smallest of two
  // values: the ORB-wide limit and the transport dependent limit.
  //
  // If an incoming message exceeds this limit, the message will not be
  // unmarshalled and the CORBA::COMM_FAILURE exception will be raised. The
  // connection is closed as a result.
  //
  // If an outgoing message exceeds this limit, the CORBA::MARSHAL exception
  // will be raised.

private:
  State pd_state;
  _CORBA_ULong pd_request_id;
  _CORBA_Boolean pd_response_expected;
  size_t pd_max_message_size;

  void UnMarshallSystemException();

};


class GIOP_S : public GIOP_Basetypes, public NetBufferedStream {
public:

  static void dispatcher(Strand *s);
  // This is the first level dispatcher. Its task is to take GIOP
  // message(s) from a strand. It will handle one or more GIOP messages
  // until it receives a Request message (see below), when that happens
  // it will return to the caller after the request is processed.
  //
  // If the message is any of these:
  //       LocateRequest,
  //       CancelRequest,
  //       MessageError,
  //       CloseConnection
  // it is handled by the dispatcher.
  //
  // If the message is Request, the dispatcher would locate the object
  // implementation and perform an upcall to the stub level dispatcher of
  // that object.
  //

  GIOP_S(Strand *s);
  ~GIOP_S();

  enum State { Idle,
	       RequestIsBeingProcessed,
	       WaitingForReply,
	       ReplyIsBeingComposed,
	       Zombie
  };

  // A GIOP_S instance operates in one of the following states:
  //
  //      Idle                        - no request has been received
  //      RequestIsBeingProcessed     - a request is being processed, arguments
  //                                    can be unmarshalled from the
  //                                    NetBufferedStream
  //      WaitingForReply             - An upcall has been dispatched into
  //                                    the object implementation
  //      ReplyIsBeingComposed        - a reply is being composed, results
  //                                    can be marshalled into the
  //                                    NetBufferedStream
  //      Zombie                      - the destructor has been called
  //
  // Calling the member functions RequestReceived(), InitialiseReply() and
  // ReplyCompleted() cause the GIOP_S to change state. The state transitions
  // are as follows:
  //
  //					+------------+
  //					|	     |  ~GIOP_S()
  //				   +----+---+	     |
  //				   | Zombie |<-------+
  //				   +--------+
  //                                    ^
  //   	       	       	       	       	| ~GIOP_S()
  //	      pd_response_expected &&   |
  //          ReplyCompleted()     +----+---+  	   <dispatcher() got a valid
  //             +---------------->|  Idle  |---------------+  request>
  //             |                 +--------+               |
  //             |                      ^                   V
  //  +-----------------------+         |         +--------------------------+
  //  | ReplyIsBeingComposed  |         |         |  RequestIsBeingProcessed |
  //  +-----------------------+         |         +-----------------+--------+
  //		 ^                      | !pd_response_expected &&  |
  //             |                      | ReplyCompleted()          |
  //             |         +--------------------+                   |
  //             +---------|  WaitingForReply   |<------------------+
  //     InitialiseReply() +--------------------+     RequestReceived()
  //
  //
  // The dispatcher() encapsulates the upcall to the stub level dispatcher
  // with a try block that catches all system exceptions. The exception
  // handlers deal with an exception according to the state of the GIOP_S
  // at the time:
  //
  //    1. If a system exception is caught when arguments are unmarshalled or
  //       results are marshalled, i.e. the GIOP_S is either in
  //         RequestIsBeingProcessed or ReplyIsBeingComposed,
  //       something is seriously wrong in the marshalling or unmarshalling
  //       and cannot recover without destroying the strand. The destructor
  //       is called with the state unchanged.
  //
  //    2. If a system exception is caught while the strand is in
  //       WaitingForReply, the dispatcher() would completed internally the
  //       reply sequence and propagate the exception back to the client.
  //
  // The destructor examines the state, if it is *not* in Idle, there is
  // something seriously wrong with the NetBufferedStream. Also the state
  // of the strand, on which the NetBufferedStream is built, is unknown.
  // For this reason, the strand is marked as dying by the destructor. The
  // destructor may be called multiple times. It simply returns if the instance
  // is already in Zombie state.
  //
  // Calling the member functions RequestReceived(), InitialiseReply() and
  // ReplyCompleted() while the GIOP_S is not in the appropriate state as
  // indicated above would cause a CORBA::INTERNAL() exception to be raised.
  //

  void RequestReceived(_CORBA_Boolean skip=0);

  void InitialiseReply(const GIOP::ReplyStatusType status, size_t msgsize);
  // Initialise a Reply message
  // Note: <msgsize> is the size of the whole message including the
  //       GIOP message header and the Request message header

  void ReplyCompleted();

  static size_t ReplyHeaderSize();
  // Return the reply header size. This includes the size of the GIOP message
  // header and the Reply message header

  inline size_t MaxMessageSize() const { return pd_max_message_size; }
  // Returns the maximum size of a GIOP message (excluding the header) that
  // can be delivered or received. This value is the smallest of two
  // values: the ORB-wide limit and the transport dependent limit.
  //
  // If an incoming message exceeds this limit, a MessageError message
  // will be sent and the CORBA::COMM_FAILURE exception will be raised. The
  // connection is closed as a result.
  //
  // If an outgoing message exceeds this limit, the CORBA::MARSHAL exception
  // will be raised.

  inline const _CORBA_Octet* key() const { return pd_key.key(); }
  inline int keysize() const { return pd_key.size(); }
  inline const char* operation() const { return (const char*) pd_operation; }
  inline _CORBA_Boolean response_expected() const {
    return pd_response_expected;
  }
  inline void set_user_exceptions(const char*const* ues, int n) {
    pd_user_exns = ues;  pd_n_user_exns = n;
  }
  // These may only be called in the context of an operation invocation.

  void SendMsgErrorMessage();

  class terminateProcessing {
  public:
    // This exception is thrown when the processing of the current giop
    // request should terminte.
    inline terminateProcessing() {}
    inline ~terminateProcessing() {}
  };

private:
  State pd_state;
  _CORBA_ULong   pd_request_id;
  _CORBA_Boolean pd_response_expected;
  size_t         pd_max_message_size;

#define OMNIORB_GIOPDRIVER_GIOP_S_INLINE_BUF_SIZE 32

  _CORBA_Octet*  pd_operation;
  _CORBA_Octet   pd_op_buffer[OMNIORB_GIOPDRIVER_GIOP_S_INLINE_BUF_SIZE];
  _CORBA_Octet*  pd_principal;
  _CORBA_Octet   pd_pr_buffer[OMNIORB_GIOPDRIVER_GIOP_S_INLINE_BUF_SIZE];

  omniObjKey pd_key;
  // In the context of an operation invocation, this
  // is the object key being invoked.

  const char*const* pd_user_exns;
  int               pd_n_user_exns;
  // A list of the user exceptions that current operation may
  // 

  void HandleRequest(_CORBA_Boolean byteorder);
  void HandleLocateRequest(_CORBA_Boolean byteorder);
  void HandleCancelRequest(_CORBA_Boolean byteorder);
  void HandleMessageError();
  void HandleCloseConnection();
};


#endif // __GIOPDRIVER_H__
