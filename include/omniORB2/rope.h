// -*- Mode: C++; -*-
//                            Package   : omniORB2
// rope.h                     Created on: 6/2/96
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
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.3  1997/05/06 16:10:38  sll
  Public release.

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
class NetBufferedStream;
class Strand_iterator;

class Strand {
public:

  Strand(Rope *r,_CORBA_Boolean heapAllocated = 0);
  // Concurrency Control:
  //    MUTEX = r->pd_lock
  // Pre-condition:
  //	Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

  virtual ~Strand();
  // Concurrency Control:
  //    MUTEX = pd_rope->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit

  virtual size_t MaxMTU() const = 0;
  // Maximum message transfer unit. This value is transport dependent.

  struct sbuf {
    void  *buffer;
    size_t size;
  };

  virtual sbuf receive(size_t size,_CORBA_Boolean exactly,int align)=0;
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

  virtual void receive_and_copy(sbuf b) = 0;
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
  // The semantics of this function is equivalent to a receive() followed by
  // a memcpy().
  //
  // This function may be used instead of receive() for bulk data,
  // such as a long sequence of Octets. The function allows for optimisation
  // within the strand to directly copy the data from network buffers to the
  // designated data area.
  // This function *should be* called only after the current thread has
  // acquired the read lock of this strand.

  virtual void skip(size_t size) = 0;
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

  virtual sbuf reserve(size_t size,_CORBA_Boolean exactly,int align,
		       _CORBA_Boolean transmit=0)=0;
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


  virtual void giveback_reserved(size_t leftover,_CORBA_Boolean transmit=0)=0;
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
  // This function *should be* called only after the current thread has
  // acquired the write lock of this strand.

  virtual size_t max_reserve_buffer_size() = 0;
  // Concurrency Control:
  //       None required
  // The maximum size of the data area that can be returned by reserve()

  virtual void reserve_and_copy(sbuf b,_CORBA_Boolean transmit=0) = 0;
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

  virtual void shutdown() = 0;
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

  void incrRefCount(_CORBA_Boolean held_rope_mutex = 0);
  // Concurrency Control:
  //      MUTEX = pd_rope->pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX> on enter if held_rope_mutex == FALSE
  //      Hold <MUTEX> on enter if held_rope_mutex == TRUE              
  // Post-condition:
  //      Restore <MUTEX> to the same state as indicated by held_rope_mutex

  void decrRefCount(_CORBA_Boolean held_rope_mutex = 0);
  // Concurrency Control:
  //      MUTEX = pd_rope->pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX> on enter if held_rope_mutex == FALSE
  //      Hold <MUTEX> on enter if held_rope_mutex == TRUE              
  // Post-condition:
  //      Restore <MUTEX> to the same state as indicated by held_rope_mutex

  _CORBA_Boolean is_idle(_CORBA_Boolean held_rope_mutex = 0);
  // Concurrency Control:
  //      MUTEX = pd_rope->pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX> on enter if held_rope_mutex == FALSE
  //      Hold <MUTEX> on enter if held_rope_mutex == TRUE              
  // Post-condition:
  //      Restore <MUTEX> to the same state as indicated by held_rope_mutex


  class Sync {

  public:
    Sync(Strand *s,_CORBA_Boolean RdLock=1,_CORBA_Boolean WrLock=1);
    // Concurrency Control:
    //    MUTEX = s->pd_rope->pd_lock
    // Pre-condition:
    //    Does not hold <MUTEX> on entry
    // Post-condition:
    //    Does not hold <MUTEX> on exit, even if an exception is raised.
    //
    // Exclusive access to a Strand is provided by instantiating a Sync
    // object. Initial locking modes can be specified in the constructor
    // of the Sync object. The constructor also enters the Sync object
    // to a private queue of the Strand. The iterator Sync_iterator can
    // be used to go through all the Sync objects that are associated with
    // a Strand.
    //
    // Caller must ensure that the reference count of the argument strand <s>
    // is non-zero. This ctor also increment the reference count of the
    // strand internally using Strand::incrRefCount().

    Sync(Rope *r,_CORBA_Boolean RdLock=1,_CORBA_Boolean WrLock=1);
    // Concurrency Control:
    //    MUTEX = r->pd_lock
    // Pre-condition:
    //	  Does not hold <MUTEX> on entry
    // Post-condition:
    //	  Does not hold <MUTEX> on exit, even if an exception is raised.
    //
    // A strand from the argument <r> is grabbed by this Sync object.
    // This ctor also increment the reference count of the strand internally
    // using Strand::incrRefCount().

    ~Sync();
    // Concurrency Control:
    // 	  MUTEX = pd_strand->pd_rope->pd_lock
    // Pre-condition:
    //    Does not hold <MUTEX> on entry
    // Post-condition:
    //    Does not hold <MUTEX> on exit
    //    
    // IMPORTANT: the destructor DOES NOT check whether a read or a write
    //            lock is held by this object. Make sure that any lock
    //            is released, using RdUnlock() and WrUnlock(), before this
    //            destructor is called.
    //
    // This dtor decrement the reference count of the strand by calling
    // Strand::decrRefCount(). If the reference count reaches 0 
    // (Strand::is_idle() returns TRUE) and the strand has been flagged for
    // closing down (Strand::StrandIsDying() returns TRUE), the dtor of the
    // strand is called.

  protected:

    void RdLock(_CORBA_Boolean held_rope_mutex=0);
    void WrLock(_CORBA_Boolean held_rope_mutex=0);
    void RdUnlock(_CORBA_Boolean held_rope_mutex=0);
    void WrUnlock(_CORBA_Boolean held_rope_mutex=0);
    // IMPORTANT: to avoid deadlock, the following protocol MUST BE obeyed.
    //            1. Acquire Read lock before Write Lock.
    //            2. Never acquire a Read lock while holding a Write lock.
    //               Must release the Read lock first.
    // Concurrency Control:
    // 	  MUTEX = pd_strand->pd_rope->pd_lock
    // Pre-condition:
    //      Does not hold <MUTEX> on enter if held_rope_mutex == FALSE
    //      Hold <MUTEX> on enter if held_rope_mutex == TRUE              
    // Post-condition:
    //      Restore <MUTEX> to the same state as indicated by held_rope_mutex

    Strand *get_strand() { return pd_strand; }
    // Concurrency Control:
    //     None required.

  void setStrandDying();

  private:
    Sync *pd_next;
    Strand *pd_strand;

    Sync();
  };

protected:

  void setStrandIsDying() { pd_dying = 1; return; }
  _CORBA_Boolean StrandIsDying() { return pd_dying; }

  friend class Sync;
  friend class Strand_iterator;
  friend class Rope;
private:
  omni_condition  pd_rdcond;
  int             pd_rd_nwaiting;

  omni_condition  pd_wrcond;
  int             pd_wr_nwaiting;

  Sync           *pd_head;
  Strand         *pd_next;
  Rope           *pd_rope;
  _CORBA_Boolean  pd_dying;
  _CORBA_Boolean  pd_heapAllocated;
  int		  pd_refcount;
  _CORBA_ULong     pd_seqNumber;

  // Make the default constructor private. This traps at compile time
  // any attempt to allocate an array of objects using the new operator.
  Strand();
  Strand(const Strand&);
  Strand &operator=(const Strand&);
};

typedef Strand::Sync Strand_Sync;

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

  virtual ~Endpoint() {
    delete [] pd_protocolname;
    return;
  }

  _CORBA_Boolean is_protocol(_CORBA_Char *name) {
    if (strcmp((char *)name,(char *)pd_protocolname) == 0) {
      return 1;
    }
    else {
      return 0;
    }
  }

private:
  _CORBA_Char * pd_protocolname;
  Endpoint();
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

private:
  const Rope   *pd_rope;
  _CORBA_Boolean pd_leave_mutex;
  Strand *pd_s;
  Strand_iterator();
};

class Rope_iterator;

class Anchor {
public:
  Anchor();
  ~Anchor();

  static Anchor incomingAnchor;
  static Anchor outgoingAnchor;

private:
  friend class Rope;
  friend class Rope_iterator;

  omni_mutex   pd_lock;
  Rope        *pd_head;
};



class Rope {
public:
  Rope(Anchor *a,
       unsigned int maxStrands,
       _CORBA_Boolean heapAllocated = 0);
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

  virtual void iopProfile(const _CORBA_Octet *objkey,const size_t objkeysize,
			  IOP::TaggedProfile &p) = 0;
  // Use the supplied object key and the endpoint of this rope, produce
  // a heap allocated IOP tagged profile.
  // Concurrency Control:
  //      None required
			  

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

  _CORBA_Boolean is_idle(_CORBA_Boolean held_anchor_mutex = 0);
  // Concurrency Control:
  //      MUTEX = pd_anchor->pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX> on enter if held_anchor_mutex == FALSE
  //      Hold <MUTEX> on enter if held_anchor_mutex == TRUE              
  // Post-condition:
  //      Restore <MUTEX> to the same state as indicated by held_anchor_mutex


  friend class Strand;
  friend class Strand_iterator;
  friend class Rope_iterator;

#ifndef __DECCXX
  // DEC C++ compiler (as of version 5.4) fails to recognise class Strand::Sync
  // is a friend and allows access to the following protected members.

  friend class Strand::Sync;

protected:

#endif

  omni_mutex pd_lock;
  virtual Strand *getStrand();
  // Concurrency Control:
  //     MUTEX = pd_lock
  // Pre-condition:
  //     Must hold <MUTEX> on entry
  // Post-condition:
  //     Must hold <MUTEX> on exit, even if an exception is raised
  //
  // getStrand() returns a ptr to an unused Strand, i.e. there is no
  // Strand::Sync object associated with it. If none is available and
  // the number of strands has not exceeded pd_maxStrands, call newStrand()
  // to create a new one.
  // A thread may be blocked in this function until a free strand is available.
  //

  virtual Strand *newStrand() = 0;
  // Concurrency Control:
  //     MUTEX = pd_lock
  // Pre-condition:
  //     Must hold <MUTEX> on entry
  // Post-condition:
  //     Must hold <MUTEX> on exit, even if an exception is raised

private:


  unsigned int    pd_maxStrands;

  _CORBA_Boolean  pd_heapAllocated;

  Strand         *pd_head;
  Rope           *pd_next;
  Anchor          *pd_anchor;
  int              pd_refcount;

  Rope();
};

class Rope_iterator {
public:
  Rope_iterator(const Anchor *a);
  // Concurrency Control:
  //     MUTEX = a->pd_lock
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
  const Anchor *pd_anchor;
  Rope *pd_r;
  Rope_iterator();
};

#endif // __ROPE_H__
