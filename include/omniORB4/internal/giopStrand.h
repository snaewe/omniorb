// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopStrand.h               Created on: 05/01/2001
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
  Revision 1.1.4.3  2001/07/13 15:20:56  sll
  New member safeDelete is now the only method to delete a strand.

  Revision 1.1.4.2  2001/06/13 20:11:37  sll
  Minor update to make the ORB compiles with MSVC++.

  Revision 1.1.4.1  2001/04/18 17:19:00  sll
  Big checkin with the brand new internal APIs.

  Revision 1.1.2.1  2001/02/23 16:47:04  sll
  Added new files.

  */

#ifndef __GIOPSTRAND_H__
#define __GIOPSTRAND_H__

#if !defined(_core_attr)
# if defined(_OMNIORB_LIBRARY)
#   define _core_attr
# else
#   define _core_attr _OMNIORB_NTDLL_IMPORT
# endif
#endif

#include <omniORB4/omniTransport.h>

OMNI_NAMESPACE_BEGIN(omni)

class giopStream;
class giopStreamImpl;
class giopWorker;
class giopServer;
class GIOP_S;
struct giopStream_Buffer;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class giopStreamList {
public:
  giopStreamList* next;
  giopStreamList* prev;

  giopStreamList() {
		next = this;
		prev = this;
	}

  void insert(giopStreamList& head);
  void remove();
  static CORBA::Boolean is_empty(giopStreamList& head);

private:
  giopStreamList(const giopStreamList&);
  giopStreamList& operator=(const giopStreamList&);
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class giopStrand : public Strand {
 public:

  giopStrand(const giopAddress*);
  // Ctor for an active strand. I.e. those that are used to connect to
  // a remote address space.
  // When a connection is establshed, the refernce count goes to 1.
  //
  // No thread safety precondition


  giopStrand(giopConnection*,giopServer*);
  // Ctor for a passive strand. I.e. those that are created because a
  // client has connected to this address space.
  // Increment the reference count on the connection.
  //
  // No thread safety precondition


public:

  CORBA::Boolean safeDelete(CORBA::Boolean forced = 0);
  // This should be the *ONLY* method to call to delete a strand.
  // Return TRUE if the strand can be considered deleted.
  //
  // The function check if this connection is satisfied before it returns
  // true:
  //
  //    giopStreamList::is_empty(clients) &&
  //    giopStreamList::is_empty(servers) &&
  //    giopStream::noLockWaiting(this)
  //
  // If the function returns true, the above condition becomes an invariant
  // and should not be violated until the dtor of the strand is called.
  //
  // The <forced> flag, if set explicitly to 1, causes the function to
  // skip checking for the above condition. Instead it just go ahead as if
  // the condition is met. This flag is used for internal implementation
  // and should not be used by any client of this class.
  //
  // Internally, the strand may stays on a bit longer until the connection's
  // reference count goes to 0 as well.
  //
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock unless forced == 1.

private:
  CORBA::Boolean pd_safelyDeleted;

public:

  CORBA::Boolean deletePending();
  // Return true, if safeDelete() has been called and it returns true.
  //
  // If this method returns true, the following invariant is always true
  // until the strand is deleted: 
  //
  //    giopStreamList::is_empty(clients) &&
  //    giopStreamList::is_empty(servers) &&
  //    giopStream::noLockWaiting(this)
  //
  // The caller must not do anything that would cause the strand to violate
  // this invariant. For example, attempt or wait to acquire a lock on the
  // strand, or to queue any GIOP_S or GIOP_C object to the strand.
  //
  // 
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock.


  GIOP_S* acquireServer(giopWorker*);
  // Acquire a GIOP_S from the strand. Normally this is only  done on
  // passive strands. However, it can also be used for active strands when
  // they become birectional, i.e. BiDir == 1.
  //
  // Return 0 if a GIOP_S cannot be acquired.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.

  void   releaseServer(IOP_S*);
  // Release the GIOP_S to the strand. The GIOP_S must have been acquired
  // previously through acquireServer from this strand. Passing in a GIOP_S
  // from a different strand would result in undefined behaviour.
  //
  // Thread Safety preconditions:
  //    Caller must not hold omniTransportLock, it is used internally for
  //    synchronisation.


  enum State { ACTIVE,  // The strand is in active use

	       DYING,   // Something terminally wrong has happened to the
	                // strand, it should be removed at the earliest
	                // convenient time.

	       TIMEDOUT // This strand can still be used when required but
       	                // it can also be removed if resources is scarce.
  };

  State state() const { return pd_state; }
  // No thread safety precondition, use with extreme care

  void state(State s) { pd_state = s; }
  // No thread safety precondition, use with extreme care

  void resetIdleCounter(CORBA::ULong nbeats) { idlebeats = nbeats; }
  // Reset the idle counter to <nbeats> so that the idle countdown starts from
  // that value.
  //
  // No thread safety precondition, use with extreme care

  ////////////////////////////////////////////////////////////////////////
  // When idlebeats go to 0, the strand has been idle for a sufficently
  // long time and should be deleted.
  // This variable SHOULD NOT be manipulated outside the implementation of
  // giopStrand.
  CORBA::Long        idlebeats;


  giopStreamList      servers;
  giopStreamList      clients;
  // a strand may have more than one giopStream instances associated with
  // it. Mostly this is because from GIOP 1.2 onwards, requests can be
  // interleaved on associated connection. Each of these request is
  // represented by a giopStream instance. They are linked together by
  // servers and clients.
  //   servers - all the GIOP_S that is serving calls for this strand
  //   clients - all the GIOP_C that is doing invocation using this strand
  //
  // Except when a strand is used to support bidirectional GIOP, only one of
  // the list will be populated (because plain GIOP is asymetric and one
  // end is either a client or a server but not both). With bidirectional GIOP,
  // both list may be populated.

  CORBA::Boolean      biDir;
  // Indicate if the strand is used for bidirectional GIOP.

  inline CORBA::Boolean isClient() { return (server == 0); }
  // Return TRUE if this is an active strand on the client side. Unless
  // biDir is TRUE, only those messages expected by a GIOP client can be
  // received from this connection.

  const giopAddress*  address;
  // address is provided as ctor arg if this is a active strand, otherwise
  // it is 0.

  giopConnection*     connection;
  // connection is provided as ctor arg if this is a passive strand
  // otherwise it is obtained by address->connect().

  giopServer*         server;
  // server is provided as ctor arg if this is a passive strand
  // otherwise it is 0.

  CORBA::Boolean      gatekeeper_checked;
  // only applies to passive strand. TRUE(1) means that the gatekeeper
  // has checked this connection. This flag is set by giopWorker and is
  // not manipulated by the strand class.

  CORBA::Boolean      first_use;
  // only applies to active strand. TRUE(1) means this connection has
  // not been used to carry an invocation before.
  // This flag is set to 1 by ctor and reset to 0 by GIOP_C.

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  // The following are data structures used by the giopStream instances
  // associated with this strand AND SHOULD NOT BE manipulated by the Strand
  // class!!!
  CORBA::Boolean      tcs_selected;
  omniCodeSet::TCS_C* tcs_c;
  omniCodeSet::TCS_W* tcs_w;
  GIOP::Version       version;
  giopStreamImpl*     giopImpl;
  // The transmission codesets for char, wchar, string and wstring are
  // selected by the client based on the codeset info in the IOR. The
  // client informs the server of its selection using a codeset service
  // context. This is done only once per lifetime of a connection (strand).
  // If <tcs_selected> == 1,
  //   <tcs_c>, <tcs_w> and <version> records the chosen code set convertors
  //   and the GIOP version for which the convertors apply.


  // conditional variables and counters to implement giopStream locking
  // functions.
  omni_tracedcondition rdcond;
  int                  rd_nwaiting;
  omni_tracedcondition wrcond;
  int                  wr_nwaiting;


  CORBA::ULong newSeqNumber();
  // Return a number suitable for use as the GIOP request id.
  //
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock.

private:
  CORBA::ULong         seqNumber;
  // monotonically increasing number to be used as the GIOP request id.


public:
  giopStream_Buffer*   head;
  giopStream_Buffer*   spare;

public:
  static _core_attr StrandList  active;
  static _core_attr StrandList  active_timedout;
  static _core_attr StrandList  passive;
  // Throughout the lifetime of a strand, it is a member of one and only one
  // of the lists:
  //   active           - the ORB uses this connection in the role of a client
  //                      it is 'active' in the sense that the connection was
  //                      initiated by this ORB
  //   active_timedout  - the connection was previously active and has been
  //                      idled for some time. It will be deleted soon.
  //   passive          - the ORB uses this connection in the role of a server
  //                      it is 'passive' because the connection was initiated
  //                      by the remote party.
  //


  struct timeValue {
    unsigned long secs;
    unsigned long nanosecs;
  };

  static _core_attr CORBA::ULong idleOutgoingBeats;
  // Number to instantiate idlebeats when the active strand becomes idle.

  static _core_attr CORBA::ULong idleIncomingBeats;
  // idleIncomingBeats * scanPeriod == no. of sec. a passive strand should
  // be allowed to stay idle.

  static _core_attr CORBA::ULong scanPeriod;
  // Time in sec. the scavenger waits before doing a scan. ScanPeriod == 0
  // means do not scan.

  static _core_attr timeValue outgoingCallTimeOut;
  static _core_attr timeValue incomingCallTimeOut;

public:
  void deleteStrandAndConnection(CORBA::Boolean forced=0);
  // Decrement connection's reference count. If it goes to 0, delete
  // this strand as well. This call ensures that both the strand and
  // connection die at the same time.
  //
    // Thread Safety preconditions:
  //    Caller must hold omniTransportLock unless forced == 1.

#ifdef __GNUG__
  friend class keep_gcc_happy;
#endif

private:
  virtual ~giopStrand();

  State         pd_state;

  giopStrand(const giopStrand&);
  giopStrand& operator=(const giopStrand&);

};

OMNI_NAMESPACE_END(omni)

#endif // __GIOPSTRAND_H__
