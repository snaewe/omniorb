// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniTransport.h            Created on: 05/01/2001
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
  Revision 1.1.2.1  2001/02/23 16:47:08  sll
  Added new files.

  */

#ifndef __OMNITRANSPORT_H__
#define __OMNITRANSPORT_H__

class omniCallDescriptor;

OMNI_NAMESPACE_BEGIN(omni)

// A Rope is an abstraction through which a client can connect to a
// remote address space.
//
// A rope creates network connections to the remote address on demand.
// At any time, there can be 0 to n number of network connections associated
// with each rope.
//
// Each network connection is represented by a Strand.

extern omni_tracedmutex* omniTransportLock;

class IOP_C;
class IOP_S;
class Rope;
class Strand;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class Marshaller {
public:
  virtual void marshal(cdrStream&) = 0;
};

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
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class IOP_C_Holder {
public:
  IOP_C_Holder(const omniIOR*,Rope*,omniCallDescriptor*);
  ~IOP_C_Holder();

  IOP_C* operator->() { return pd_iop_c; }

  operator IOP_C& () { return *pd_iop_c; }

private:
  Rope*  pd_rope;
  IOP_C* pd_iop_c;
  
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class IOP_S {
public:

  IOP_S() {}
  virtual ~IOP_S() {}

  virtual cdrStream& getStream() = 0;

  virtual const char* operation_name() const = 0;

  enum State { UnUsed,
	       InputFullyBuffered,
	       InputPartiallyBuffered,
	       Idle,
	       RequestIsBeingProcessed,
	       WaitingForReply,
	       ReplyIsBeingComposed,
	       Zombie
  };

  // A IOP_S instance operates in one of the following states:
  //
  //      Idle                        - no request has been received
  //      RequestIsBeingProcessed     - a request is being processed, arguments
  //                                    can be unmarshalled from the
  //                                    cdrStream
  //      WaitingForReply             - An upcall has been dispatched into
  //                                    the object implementation
  //      ReplyIsBeingComposed        - a reply is being composed, results
  //                                    can be marshalled into the
  //                                    cdrStream
  //      Zombie                      - the destructor has been called
  //
  // Calling the member functions RequestReceived(), InitialiseReply() and
  // ReplyCompleted() cause the IOP_S to change state. The state transitions
  // are as follows:
  //
  //					+------------+
  //					|	     |  ~IOP_S()
  //				   +----+---+	     |
  //				   | Zombie |<-------+
  //				   +--------+
  //                                    ^
  //   	       	       	       	       	| ~IOP_S()
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
  // handlers deal with an exception according to the state of the IOP_S
  // at the time:
  //
  //    1. If a system exception is caught when arguments are unmarshalled or
  //       results are marshalled, i.e. the IOP_S is either in
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
  // something seriously wrong with the cdrStream. Also the state
  // of the strand, on which the cdrStream is built, is unknown.
  // For this reason, the strand is marked as dying by the destructor. The
  // destructor may be called multiple times. It simply returns if the instance
  // is already in Zombie state.
  //
  // Calling the member functions RequestReceived(), InitialiseReply() and
  // ReplyCompleted() while the IOP_S is not in the appropriate state as
  // indicated above would cause a CORBA::INTERNAL() exception to be raised.
  //

  virtual void ReceiveRequest(omniCallDescriptor&) = 0;

  virtual void SkipRequestBody() = 0;

  virtual void SendReply() = 0;
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class StrandList {
public:
  StrandList* next;
  StrandList* prev;

  StrandList() : next(this), prev(this) {}

  void insert(StrandList& head);
  void remove();
  static _CORBA_Boolean is_empty(StrandList& head);

private:
  StrandList(const StrandList&);
  StrandList& operator=(const StrandList&);
};



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class Rope {
public:

  Rope();

  virtual ~Rope();

  virtual IOP_C* acquireClient(const omniIOR*,omniCallDescriptor*) = 0;
  virtual void releaseClient(IOP_C*) = 0;

  virtual void incrRefCount() = 0;
  virtual void decrRefCount() = 0;


  class Link {
  public:
    Link* next;
    Link* prev;

    Link() : next(this), prev(this) {}

    void insert(Link& head);
    void remove();
    static _CORBA_Boolean is_empty(Link& head);

  private:
    Link(const Link&);
    Link& operator=(const Link&);
  };

  friend class Strand;

protected:
  Link pd_strands; // this is a list of strands that connects to the same
                   // remote address space.

private:
  Rope(const Rope&);
  Rope& operator=(const Rope&);
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class Strand : public Rope::Link, public StrandList {
public:

  Strand();
  virtual ~Strand();

private:
  Strand(const Strand&);
  Strand& operator=(const Strand&);
};

OMNI_NAMESPACE_END(omni)

#endif // __ROPE_H__
