// -*- Mode: C++; -*-
//                            Package   : omniORB2
// IOP_C.h                    Created on: 05/01/2001
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
  Revision 1.1.2.2  2001/05/02 14:20:15  sll
  Make sure that getStream() is used instead of casting to get a cdrStream
  from a IOP_C and IOP_S.

  Revision 1.1.2.1  2001/04/18 17:26:29  sll
  Big checkin with the brand new internal APIs.

 */

#ifndef __IOP_C_H__
#define __IOP_C_H__

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class IOP_C {
public:
  IOP_C() {}
  virtual ~IOP_C() {}

  enum State { UnUsed,
	       Idle,
	       RequestInProgress,
	       WaitingForReply,
	       ReplyIsBeingProcessed,
               Zombie
  };

  virtual cdrStream& getStream() = 0;

  // A IOP_C instance operates in one of the following states:
  //
  //       Idle                  -  no request has been initiated
  //       RequestInProgress     -  a request is being composed, arguments
  //                                can be marshalled into the
  //                                cdrStream
  //       WaitingForReply       -  the request has been sent, no reply
  //                                has come in yet.
  //       ReplyIsBeingProcessed -  a reply has started arriving, results
  //                                can be unmarshalled from the
  //                                cdrStream
  //       Zombie                -  the destructor has been called
  //
  // Calling the member functions InitialiseRequest(), ReceiveReply() and
  // RequestCompleted() cause the IOP_C to change state. The state transitions
  // are as follows:
  //					+------------+
  //					|	     |  ~IOP_C()
  //				   +----+---+	     |
  //				   | Zombie |<-------+
  //				   +--------+
  //                                    ^
  //   	       	       	       	       	| ~IOP_C()
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
  // The constructor initialises a IOP_C to Idle state.
  //
  // When the destructor is called, the state is examined. If it is *not* in
  // Idle or WaitingForReply, there is something seriously wrong with the
  // cdrStream. Also the state of the strand, on which the
  // cdrStream is built, is unknown. For this reason, the strand
  // is marked as dying by the destructor. The destructor may be called
  // multiple times. It simply returns if the instance is already in Zombie
  // state.
  //
  // Calling the member functions InitialiseRequest(), ReceiveReply(),
  // RequestCompleted() and IssueLocateRequest() while the IOP_C is not 
  // in the appropriate state as indicated above would cause a 
  // CORBA::INTERNAL() exception to be raised.
  //

  virtual void InitialiseRequest() = 0;

  virtual GIOP::ReplyStatusType ReceiveReply() = 0;

  virtual void RequestCompleted(_CORBA_Boolean skip=0) = 0;

  virtual GIOP::LocateStatusType IssueLocateRequest() = 0;

private:
  IOP_C(const IOP_C&);
  IOP_C& operator=(const IOP_C&);

  operator cdrStream& ();
  // This is to make sure that we do not have any code that blindly cast
  // a reference to this object to a cdrStream&. All code should use
  // getStream() instead.
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class IOP_C_Holder {
public:
  IOP_C_Holder(const omniIOR*,const _CORBA_Octet*,_CORBA_ULong,Rope*,
	       omniCallDescriptor*);
  ~IOP_C_Holder();

  IOP_C* operator->() { return pd_iop_c; }

  operator IOP_C& () { return *pd_iop_c; }

private:
  Rope*  pd_rope;
  IOP_C* pd_iop_c;
  
};

OMNI_NAMESPACE_END(omni)

#endif // __IOP_C_H__
