// -*- Mode: C++; -*-
//                            Package   : omniORB2
// rope.h                     Created on: 6/2/96
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
  Revision 1.11.4.4  2000/03/27 17:41:31  sll
  Removed Sync class.
  Redefined the reference counting rule for Strand.
  New member Rope::oneCallPerConnection().

  Revision 1.11.4.3  2000/02/14 18:06:45  sll
  Support GIOP 1.2 fragment interleaving. This requires minor access control
  changes to the relevant classes.

  Revision 1.11.4.2  1999/09/25 17:00:11  sll
  Merged changes from omni2_8_develop branch.

  Revision 1.11.4.1  1999/09/15 20:18:13  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

  Revision 1.11.2.1  1999/09/21 20:37:15  sll
  -Simplified the scavenger code and the mechanism in which connections
   are shutdown. Now only one scavenger thread scans both incoming
   and outgoing connections. A separate thread do the actual shutdown.
  -omniORB::scanGranularity() now takes only one argument as there is
   only one scan period parameter instead of 2.
  -Trace messages in various modules have been updated to use the logger
   class.
  -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                 -ORBscanIncomingPeriod.

  Revision 1.11  1999/08/30 16:55:43  sll
  Replaced WrTestLock and heartbeat in WrLock with clicksDecrAndGet,
  clicksGet and clicksSet.

  Revision 1.10  1999/07/02 19:17:33  sll
  Removed inlined virtual dtor. Some compilers generate a copy of the
  dtor per compilation unit.

  Revision 1.9  1999/06/18 21:17:31  sll
  Updated copyright notice.

  Revision 1.8  1999/05/26 11:46:30  sll
  Replaced WrTimedLock with WrTestLock.
  Changed the operator() of Strand_iterator to increment the ref count
  of the strand it returns.
  Added new member is_unused() to class Strand. This is used to check
  if no Sync object is parking on the strand. This member is necessary
  because given the change in the Strand_iterator, is_idle() is no
  longer appropriate to test if no Sync object is using the strand.

  Revision 1.7  1999/03/17 12:43:40  djr
  Corrected error in Rope_var copy constructor.

  Revision 1.6  1999/01/25 18:06:24  sll
  Added comment on the side-effect of WrTimedLock and WrUnlock(Strand*).

  Revision 1.5  1998/03/19 19:59:01  sll
  Added comment to clarify the sync. requirement of newStrand().

  Revision 1.4  1997/12/09 20:34:36  sll
  Interfaces extended to provide more hooks to support new transports.
  Support for incoming and outgoing rope scavenger threads.

 * Revision 1.3  1997/05/06  16:10:38  sll
 * Public release.
 *
  */

#ifndef __ROPE_H__
#define __ROPE_H__

// A Rope is a bi-directional buffered stream connecting two address
// spaces. The connection point of each address space can be identified by
// an Endpoint. A Rope may be composed of one or more Strands. Each Strand
// represents a transport dependent connection. All strands of the same
// rope can be used interchangable for sending and receiving messages
// between the connected address spaces.

// Built on top of a strand, the GIOP_C and GIOP_S classes are defined to
// drive the General Inter-ORB Protocol (GIOP). The GIOP protocol
// is asymmetric.  GIOP_C provides the functions to drive the client side
// protocol.  GIOP_S provides the server side functions.

class Rope;
class Endpoint;
class cdrStream;
class Strand_iterator;
class Rope_iterator;
class giopStream;

class Strand {
public:

  // Strand object must be heap allocated.

  Strand(Rope *r);
  // Concurrency Control:
  //    MUTEX = r->pd_lock
  // Pre-condition:
  //	Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

protected:
  virtual ~Strand();
  // Concurrency Control:
  //    MUTEX = pd_rope->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit

public:
  virtual size_t MaxMTU() const = 0;
  // Maximum message transfer unit. This value is transport dependent.

  struct sbuf {
    void  *buffer;
    size_t size;
  };

  virtual sbuf receive(size_t size,_CORBA_Boolean exactly,
		       int align,_CORBA_Boolean startMTU=0)=0;
  // Concurrency Control:
  //      RdLock()
  // Pre-condition:
  //      Must hold RdLock()
  // Post-condition:
  //      Still hold RdLock()
  //
  // If <exactly> is true, receive exactly <size> bytes from this strand
  // before returning. If <exactly> is false, receive at most <size> bytes
  // from this strand. If <exactly> is true, the value of <size> cannot be
  // larger than the return value of max_receive_buffer_size().
  //
  // If startMTU is TRUE (1), the caller expects what is coming in to be
  // the start of a request message. If the strand detects that is not the
  // case, it should shutdown itself and throw a COMM_FAILURE exception.
  //
  // The return structure <sbuf> contains a pointer to the data area and
  // the number of bytes actually received. The data pointer is guaranteed to
  // be aligned to the boundary specified in <align>.
  //
  // This strand retains ownership of the data buffer.
  // The data area contains the received data *until* the next call to 
  // receive() or giveback_received(). That is, the next receive() or
  // giveback_received() call automatically invalidates the
  // data pointer returned by this call.
  //
  // ******* To service this call, the strand may or may not need to 
  //         fetch more data from the network. For instance, it may be
  //         able to provide the data from its internal buffer.
  //
  // This function *should be* called only after the current thread has 
  // acquired the read lock of this strand. The thread should not relinquish 
  // the read lock until it has consumed all the data it gets from this 
  // function or has copied out the data.


  virtual void giveback_received(size_t leftover)=0;
  // Concurrency Control:
  //      RdLock()
  // Pre-condition:
  //      Must hold RdLock()
  // Post-condition:
  //      Still hold RdLock()
  //
  // This function gives back to the strand <leftover> bytes at the
  // end of the data area given out by the previous receive() call.
  // The value of <leftover> must be less than or equal to the size of
  // the received data.
  // This function *should be* called only after the current thread has
  // acquired the read lock of this strand.
  
  virtual size_t max_receive_buffer_size()=0;
  // Concurrency Control:
  //      None required.
  // The maximum size of the data area that can be returned by receive()

  virtual void receive_and_copy(sbuf b,_CORBA_Boolean startMTU=0) = 0;
  // Concurrency Control:
  //      RdLock()
  // Pre-condition:
  //      Must hold RdLock()
  // Post-condition:
  //      Still hold RdLock()
  //
  // Copying incoming data directly to the buffer points to by <b>.
  // Exactly <b.size> bytes are copied before returning.
  //
  // If startMTU is TRUE (1), the caller expects what is coming in to be
  // the start of a request message. If the strand detects that is not the
  // case, it should shutdown itself and throw a COMM_FAILURE exception.
  //
  // The semantics of this function is equivalent to a receive() followed by
  // a memcpy().
  //
  // This function may be used instead of receive() for bulk data,
  // such as a long sequence of Octets. The function allows for optimisation
  // within the strand to directly copy the data from network buffers to the
  // designated data area.
  // This function *should be* called only after the current thread has
  // acquired the read lock of this strand.

  virtual void skip(size_t size,_CORBA_Boolean startMTU=0) = 0;
  // Concurrency Control:
  //      RdLock()
  // Pre-condition:
  //      Must hold RdLock()
  // Post-condition:
  //      Still hold RdLock()
  //
  // Drop <size> bytes of incoming data from this strand before returning.
  // The semantics of this call is equivalent to one or more receive() 
  // calls until <size> bytes are "received".
  // This function *should be* called only after the current thread has
  // acquired the read lock of this strand.
  //
  // If startMTU is TRUE (1), the caller expects what is coming in to be
  // the start of a request message. If the strand detects that is not the
  // case, it should shutdown itself and throw a COMM_FAILURE exception.

  virtual sbuf reserve(size_t size,_CORBA_Boolean exactly,int align,
		       _CORBA_Boolean transmit=0,_CORBA_Boolean endMTU=0)=0;
  // Concurrencty Control:
  //       WrLock()
  // Pre-condition:
  //       Must hold WrLock()
  // Post-condition
  //       Still hold WrLock()
  //
  // if <exactly> is true, reserve exactly <size> bytes from the output
  // buffer of this strand. If <exactly> is false, reserve at most <size>
  // bytes from this strand. If <exactly> is true, the value of <size> cannot
  // be larger than the return value of max_reserve_buffer_size();
  //
  // If endMTU is TRUE(1), the caller indicates that it is reserving buffer
  // to write the last part of a request message.
  //
  //
  // The return structure <sbuf> contains a pointer to the data area and
  // the number of bytes actually reserved. The data pointer is guaranteed to
  // be aligned to the boundary specified in <align>.
  //
  // This strand retains the ownership of the data buffer.
  //
  // The caller can write to this data area *until* the next call to reserve()
  // or giveback_reserved(). In other words, the caller can use either of 
  // the functions to instruct the strand that all or part of the data area 
  // is now filled up with valid data. If <transmit> is true, any buffered data
  // will be transmitted immediately. If <transmit> is false, any buffered
  // data may be transmitted any time hereafter.
  //
  // This function *should be* called only after the current thread has
  // acquired the write lock of this strand.

  virtual sbuf reserve_and_startMTU(size_t size,_CORBA_Boolean exactly,
				    int align,_CORBA_Boolean transmit=0,
				    _CORBA_Boolean at_most_once=0)=0;
  // The function is the same as reserve() except that it is used by
  // the caller to indicate that it is reserving buffer to write the beginning
  // of a request message.

  virtual void giveback_reserved(size_t leftover,_CORBA_Boolean transmit=0,
				 _CORBA_Boolean endMTU=0)=0;
  // Concurrencty Control:
  //       WrLock()
  // Pre-condition:
  //       Must hold WrLock()
  // Post-condition
  //       Still hold WrLock()
  //
  // This function gives back to the strand <leftover> bytes of space at
  // the end of the data area reserved by the previous reserve() call. The
  // strand would consider this area unfilled and would not transmit its
  // content. Any part of the data area not returned by this call are taken
  // as filled with valid data. If <transmit> is true, any buffered data
  // will be transmitted immediately. If <transmit> is false, any buffered
  // data may be transmitted any time hereafter.
  //
  // The value of <leftover> must be less than or equal to the size of the 
  // data area reserved.
  //
  // If endMTU is TRUE(1), the caller indicates that it has written
  // the last part of a request message.
  //
  // This function *should be* called only after the current thread has
  // acquired the write lock of this strand.

  virtual size_t max_reserve_buffer_size() = 0;
  // Concurrency Control:
  //       None required
  // The maximum size of the data area that can be returned by reserve()

  virtual void reserve_and_copy(sbuf b,_CORBA_Boolean transmit=0,
				_CORBA_Boolean endMTU=0) = 0;
  // Concurrency Control:
  //       WrLock()
  // Pre-condition:
  //       Must hold WrLock()
  // Post-condition
  //       Still hold WrLock()
  //
  // Transmit data from the buffer points to by <b>. The semantics of this
  // function is equivalent to one or more calls to reserve() and memcpy().
  //
  // This function may be used instead of reserve() for bulk data,
  // such as a long sequence of Octets. The function allows for optimisation
  // within the strand to directly copy the data to network buffers.
  // If <transmit> is true, any buffered data will be transmitted immediately. 
  // If <transmit> is false, any buffered data may be transmitted any time 
  // hereafter.
  //
  // If endMTU is TRUE(1), the caller indicates that it is reserving buffer
  // to write the last part of a request message.
  //
  // This function *should be* called only after the current thread has
  // acquired the write lock of this strand.

  _CORBA_ULong sequenceNumber() { return pd_seqNumber++; }
  // Concurrency Control:
  //       WrLock()
  // Pre-condition:
  //       Must hold WrLock()
  // Post-condition
  //       Still hold WrLock()
  //
  // The integer returned by this function is guaranteed to be different
  // from those returned in previous calls. A client may use this number
  // to tag the messages sent via a strand.

  void shutdown();
  // Concurrency Control:
  //    MUTEX = pd_rope->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //	Still hold <MUTEX> on exit
  //
  // Signal to any thread currently using this strand to give up because
  // this strand is being shut down.
  // Never returns an exception and never blocks

  virtual void real_shutdown() = 0;
  // Concurrency Control:
  //      None
  //
  // This helper function is provided by the strand implementation to 
  // change the state of the network connection to "shutdown". In other
  // words, any future OS calls to send or receive via the connection
  // would return an error.
  // Notice that this function may be called by one thread while another
  // is blocking on a receive or a send on the network connection.

  void incrRefCount(_CORBA_Boolean held_rope_mutex = 0);
  // Concurrency Control:
  //      MUTEX = pd_rope->pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX> on enter if held_rope_mutex == FALSE
  //      Hold <MUTEX> on enter if held_rope_mutex == TRUE              
  // Post-condition:
  //      Restore <MUTEX> to the same state as indicated by held_rope_mutex
  //
  // Increment the reference count by one. Call this once when a thread
  // holds a reference/pointer to this object.

  void decrRefCount(_CORBA_Boolean held_rope_mutex = 0);
  // Concurrency Control:
  //      MUTEX = pd_rope->pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX> on enter if held_rope_mutex == FALSE
  //      Hold <MUTEX> on enter if held_rope_mutex == TRUE              
  // Post-condition:
  //      Restore <MUTEX> to the same state as indicated by held_rope_mutex
  //
  // Decrement reference count. If reference count == 0 and 
  // _strandIsDying() is TRUE, delete the strand.
  // Call this once when a thread previously holding a reference/pointer to
  // this object no longer does so. Caller should not assume that a pointer
  // to this object remains valid after this call as the object could have
  // been deleted.

  void setReUseFlag();
  // Concurrency Control:
  //    MUTEX = pd_rope->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //	Still hold <MUTEX> on exit
  //
  // Mark this strand as a reused connection. This affects the behaviour
  // of the strand when it encounters error.
  // If the reuse flag is not set, a CORBA::COMM_FAILURE is raised.
  // Otherwise, a CORBA::TRANSIENT is raised.
  // The upper layers may catch TRANSIENT and retries the invocation again.
  // The default value of the reuse flag is not set.

  void raiseException(_CORBA_ULong minor,_CORBA_ULong completed);
  // Concurrency Control:
  //    None.
  //
  // This function never returns. Instead it throws either CORBA::COMM_FAILURE
  // or CORBA::TRANSIENT depending on the state of the reuse flag (see
  // setReUseFlag().
  // The arguments are passed verbatim to the ctor of the system exceptions.


  _CORBA_Boolean _strandIsDying() { return pd_dying; }

protected:

  void _setStrandIsDying() { pd_dying = 1; return; }

  friend class giopStream;
  friend class Strand_iterator;
  friend class Rope;
  friend class Rope_iterator;

private:
  omni_condition  pd_rdcond;
  int             pd_rd_nwaiting;

  omni_condition  pd_wrcond;
  int             pd_wr_nwaiting;

  giopStream     *pd_head;
  Strand         *pd_next;
  Rope           *pd_rope;
  _CORBA_Boolean  pd_dying;
  int		  pd_refcount;
  _CORBA_ULong    pd_seqNumber;

  _CORBA_Boolean  pd_reuse;

  GIOP::Version   pd_giop_version;
  _CORBA_Boolean  pd_giop_biDir;

  int             pd_clicks;
  void setClicks(int c) { pd_clicks = c; }
  int  Clicks() { return pd_clicks; }
  int  decrClicks() { return --pd_clicks; }
  int  incrClicks() { return ++pd_clicks; }

public:
  // XXX Temporary
  _CORBA_Boolean garbageCollect();

public:
  Strand         *pd_ripper_next;

  // Make the default constructor private. This traps at compile time
  // any attempt to allocate an array of objects using the new operator.
  Strand();
  Strand(const Strand&);
  Strand &operator=(const Strand&);
};

class Endpoint {
public:
  Endpoint(_CORBA_Char *protocol) {
    pd_protocolname = new _CORBA_Char [strlen((char *)protocol)+1];
    strcpy((char *)pd_protocolname,(char *)protocol);
    return;
  }

  Endpoint(const Endpoint &e) {
    pd_protocolname = new _CORBA_Char [strlen((char *)e.pd_protocolname)+1];
    strcpy((char *)pd_protocolname,(char *)e.pd_protocolname);
    return;
  }

  Endpoint &operator=(const Endpoint &e) {
    delete [] pd_protocolname;
    pd_protocolname = new _CORBA_Char [strlen((char *)e.pd_protocolname)+1];
    strcpy((char *)pd_protocolname,(char *)e.pd_protocolname);
    return *this;
  }

  virtual ~Endpoint();

  _CORBA_Boolean is_protocol(const _CORBA_Char *name) const {
    if (strcmp((const char *)name,(char *)pd_protocolname) == 0) {
      return 1;
    }
    else {
      return 0;
    }
  }

  _CORBA_Char* protocol() const {
    return pd_protocolname;
  }

private:
  _CORBA_Char * pd_protocolname;
  Endpoint();
};

class Endpoint_var {
public:
  Endpoint_var() : pd_p(0) {}
  Endpoint_var(Endpoint* p) : pd_p(p) {}
  ~Endpoint_var() { if (pd_p) delete pd_p; }
  Endpoint_var& operator=(Endpoint* p) {
    if (pd_p) delete pd_p;
    pd_p = p;
    return *this;
  }
  operator Endpoint*() const { return pd_p; }

  Endpoint*& out() { if (pd_p) { delete pd_p; pd_p = 0; } return pd_p; }
  Endpoint* _retn() { Endpoint* tmp = pd_p; pd_p = 0; return tmp; }
  

private:
  Endpoint* pd_p;
  Endpoint_var& operator=(const Endpoint_var&);
};

class Strand_iterator {
public:
  Strand_iterator(const Rope *r,_CORBA_Boolean held_rope_mutex = 0);
  // Concurrency Control:
  //      MUTEX = r->pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX> on enter if held_rope_mutex == FALSE
  //      Hold <MUTEX> on enter if held_rope_mutex == TRUE              
  // Post-condition:
  //      Hold <MUTEX> on exit

  ~Strand_iterator();
  // Concurrency Control:
  //      MUTEX = pd_rope->pd_lock
  // Pre-condition:
  //      Hold <MUTEX> on enter
  // Post-condition:
  //      Does not hold <MUTEX> on exit if pd_leave_mutex == FALSE
  //	  Hold <MUTEX> on exit if pd_leave_mutex == TRUE

  Strand *operator() ();
  // Return the next Strand. The reference count of the returned strand
  // has been incremented by 1. In the next call to operator(), or in the
  // dtor of Strand_iterator, the reference count will be decremented.

private:
  const Rope    *pd_rope;
  _CORBA_Boolean pd_leave_mutex;
  _CORBA_Boolean pd_initialised;
  Strand *pd_s;
  Strand_iterator();
};

class Anchor {
public:
  Anchor();
  ~Anchor();

private:
  friend class Rope;
  friend class Rope_iterator;

  omni_mutex   pd_lock;
  Rope        *pd_head;
};



class Rope {
public:

  // Rope objects must be heap allocated.

  Rope(Anchor *a,
       unsigned int maxStrands);
  // Concurrency Control:
  //    MUTEX = a->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

  virtual ~Rope();
  // Concurrency Control:
  //    MUTEX = pd_anchor->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit

  virtual void CutStrands(_CORBA_Boolean held_rope_mutex = 0);
  // Concurrency Control:
  //      MUTEX = pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX> on enter if held_rope_mutex == FALSE
  //      Hold <MUTEX> on enter if held_rope_mutex == TRUE              
  // Post-condition:
  //      Restore <MUTEX> to the same state as indicated by held_rope_mutex

  virtual _CORBA_Boolean is_incoming() const = 0;
  // Return TRUE (1) if this is an incoming rope;
  // Concurrency Control:
  //      None required.

  virtual _CORBA_Boolean is_outgoing() const = 0;
  // Return TRUE (1) if this is an outgoing rope;
  // Concurrency Control:
  //      None required.

  virtual _CORBA_Boolean remote_is(Endpoint *&e) = 0;
  // Returns FALSE if this is not an outgoing rope.
  // If e == 0, returns the remote endpoint in e and returns TRUE
  // else compare e with the remote endpoint and returns TRUE if they are the
  // same.
  // Concurrency Control:
  //      None required.

  virtual _CORBA_Boolean this_is(Endpoint *&e) = 0;
  // Returns FALSE if this is not an incoming rope
  // If e == 0, returns this endpoint in e and returns TRUE
  // else compare e with this endpoint and returns TRUE if they are the same.
  // Concurrency Control:
  //      None required.

  virtual _CORBA_Boolean oneCallPerConnection();
  // Returns TRUE if this is an outgoing rope and there *CANNOT* be more
  // than one call in progress on the same strand at the same time.
  // Otherwise returns FALSE.

  void incrRefCount(_CORBA_Boolean held_anchor_mutex = 0);
  // Concurrency Control:
  //      MUTEX = pd_anchor->pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX> on enter if held_anchor_mutex == FALSE
  //      Hold <MUTEX> on enter if held_anchor_mutex == TRUE              
  // Post-condition:
  //      Restore <MUTEX> to the same state as indicated by held_anchor_mutex

  void decrRefCount(_CORBA_Boolean held_anchor_mutex = 0);
  // Concurrency Control:
  //      MUTEX = pd_anchor->pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX> on enter if held_anchor_mutex == FALSE
  //      Hold <MUTEX> on enter if held_anchor_mutex == TRUE              
  // Post-condition:
  //      Restore <MUTEX> to the same state as indicated by held_anchor_mutex
  //
  // Decrement reference count. If reference count == 0, shutdown all strands
  // that are attached to this rope. Furthermore, if no strand is attached,
  // call delete on this rope.


  Anchor* anchor() const { return pd_anchor; }

  friend class Strand;
  friend class Strand_iterator;
  friend class Rope_iterator;
  friend class giopStream;

#ifndef __DECCXX

protected:

#endif

  omni_mutex     pd_lock;
  omni_condition pd_cond;
  int            pd_nwaiting;

  void waitForIdleStrand();
  // Concurrency Control:
  //     MUTEX = pd_lock
  // Pre-condition:
  //     Must hold <MUTEX> on entry
  // Post-condition:
  //     Must hold <MUTEX> on exit, even if an exception is raised
  //
  // Block until one of the following conditions:
  //     1. A strand is now unused, i.e. its pd_refcount == 0
  //     2. The no. of strand drops below pd_maxStrands.
  // If one of the two conditions is met, returns.
  // Spurious wakeups may cause this call to returns. Should always check
  // again if there is a free strand.

  virtual Strand *newStrand() = 0;
  // Concurrency Control:
  //     MUTEX = pd_lock
  // Pre-condition:
  //     Must hold <MUTEX> on entry
  // Post-condition:
  //     Must hold <MUTEX> on exit, even if an exception is raised
  // 
  // Care should be taken so that newStrand() *DO NOT BLOCK* waiting
  // to connect to a remote address space *INDEFINITELY*. This is bad
  // because holding the mutex on pd_lock for an extended period can
  // cause other threads which is totally unrelated to this rope to block
  // as well:- 
  //    One immediate consequence of holding pd_lock is that the
  //    outScavenger will be blocked on pd_lock when it is scanning
  //    for idle strands. This in turn blockout any thread trying to lock
  //    rope->pd_anchor->pd_lock. This is really bad because no new rope
  //    can be added to the anchor.
  //
  // The recommended way to implement newStrand() is to create a new
  // strand instance but delay connecting to the remote address space
  // until the first send or recv. See the tcpSocketStrand implementation
  // for an example.
  
private:


  unsigned int    pd_maxStrands;

  Strand         *pd_head;
  Rope           *pd_next;
  Anchor         *pd_anchor;
  int             pd_refcount;

  Rope();
};

class Rope_var {
public:
  inline Rope_var() : _ptr(0) {}

  inline Rope_var(Rope* p) { _ptr = p; }

  inline ~Rope_var() {
    if (_ptr)
      _ptr->decrRefCount();
  }

  inline Rope_var(const Rope_var& p) {
    if (p._ptr) {
      p._ptr->incrRefCount();
    }
    _ptr = p._ptr;
  }

  inline Rope_var& operator= (const Rope_var& p) {
    if (_ptr) {
      _ptr->decrRefCount();
      _ptr = 0;
    }
    if (p._ptr) {
      p._ptr->incrRefCount();
    }
    _ptr = p._ptr;
    return *this;
  }

  inline Rope_var& operator= (Rope* p) {
    if (_ptr) {
      _ptr->decrRefCount();
      _ptr = 0;
    }
    _ptr = p;
    return *this;
  }
  
  inline Rope* operator->() const { return _ptr; }

  inline operator Rope*() const { return _ptr; }

  inline Rope* _retn() { Rope* p = _ptr; _ptr = 0; return p; }

  Rope* _ptr;
};

class ropeFactory;

class Rope_iterator {
public:
  Rope_iterator(const Anchor *a);
  // Concurrency Control:
  //     MUTEX = a->pd_lock
  // Pre-condition:
  //     Does not hold <MUTEX> on entry
  // Post-condition:
  //     Hold <MUTEX> on exit

  Rope_iterator(ropeFactory* rf);
  // Concurrency Control:
  //     MUTEX = rf->anchor()->pd_lock
  // Pre-condition:
  //     Does not hold <MUTEX> on entry
  // Post-condition:
  //     Hold <MUTEX> on exit


  ~Rope_iterator();
  // Concurrency Control:
  //     MUTEX = a->pd_lock
  // Pre-condition:
  //     Hold <MUTEX> on enter
  // Post-condition:
  //     Does not hold <MUTEX> on enxit
  
  Rope *operator() ();

private:
  const Anchor   *pd_anchor;
  _CORBA_Boolean  pd_initialised;
  Rope *pd_r;
  Rope_iterator();
};

#endif // __ROPE_H__
