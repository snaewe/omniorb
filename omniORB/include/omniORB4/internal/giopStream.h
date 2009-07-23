// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopStream.h                   Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2006 Apasphere Ltd
//    Copyright (C) 2001      AT&T Laboratories Cambridge
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
  Revision 1.1.6.3  2006/09/20 13:36:31  dgrisby
  Descriptive logging for connection and GIOP errors.

  Revision 1.1.6.2  2005/11/17 17:03:26  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.6.1  2003/03/23 21:03:46  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.7  2001/10/17 16:44:05  dpg1
  Update DynAny to CORBA 2.5 spec, const Any exception extraction.

  Revision 1.1.4.6  2001/09/04 14:38:09  sll
  Added the boolean argument to notifyCommFailure to indicate if
  omniTransportLock is held by the caller.

  Revision 1.1.4.5  2001/09/03 16:50:43  sll
  Added the deadline parameter and access functions. All member functions
  that previously had deadline arguments now use the per-object deadline
  implicitly.

  Revision 1.1.4.4  2001/08/03 17:43:19  sll
  Make sure dll import spec for win32 is properly done.

  Revision 1.1.4.3  2001/07/31 16:20:30  sll
  New primitives to acquire read lock on a connection.

  Revision 1.1.4.2  2001/05/01 16:07:32  sll
  All GIOP implementations should now work with fragmentation and abitrary
  sizes non-copy transfer.

  Revision 1.1.4.1  2001/04/18 17:18:59  sll
  Big checkin with the brand new internal APIs.

  Revision 1.1.2.1  2001/02/23 16:47:04  sll
  Added new files.

  */


#ifndef __GIOPSTREAM_H__
#define __GIOPSTREAM_H__

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

class giopStrand;
class giopStreamImpl;
class giopImpl10;
class giopImpl11;
class giopImpl12;

struct giopStream_Buffer {
  CORBA::ULong             start;   // offset to the beginning of data
  CORBA::ULong             end;     // offset to the end of buffer
  CORBA::ULong             last;    // offset to the last data byte
  CORBA::ULong             size;    // GIOP message size.
  giopStream_Buffer*       next;    // next Buffer in a chain
  // buffer data to follows.
  void alignStart(omni::alignment_t);
  static void deleteBuffer(giopStream_Buffer*);
  static giopStream_Buffer* newBuffer(CORBA::ULong sz=0);
};

class giopStream : public cdrStream {
public:

  giopStream(giopStrand*);
  // No thread safety precondition

  ~giopStream();
  // No thread safety precondition

  virtual void* ptrToClass(int* cptr);
  static inline giopStream* downcast(cdrStream* s) {
    return (giopStream*)s->ptrToClass(&_classid);
  }
  static _core_attr int _classid;

  void reset();

  GIOP::Version version();
  // No thread safety precondition

  operator giopStrand& () { return *pd_strand; }
  // No thread safety precondition

  giopStreamImpl* impl() const { return pd_impl; }
  // No thread safety precondition

  void impl(giopStreamImpl* impl) { pd_impl = impl; }
  // No thread safety precondition

  inline void getDeadline(unsigned long& secs, unsigned long& nanosecs) const {
    secs = pd_deadline_secs;
    nanosecs = pd_deadline_nanosecs;
  }
  // No thread safety precondition

  inline void setDeadline(unsigned long secs, unsigned long nanosecs) {
    pd_deadline_secs = secs;
    pd_deadline_nanosecs = nanosecs;
  }
  // No thread safety precondition
  
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  // Thread Safety preconditions:
  //   Caller of these strand locking functions must hold the
  //   omniTransportLock before calling.

  virtual void rdLock();
  // Acquire read lock on the strand.
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Error conditions. If the deadline has expired, this call raises the
  // CommFailure exception.

  virtual void wrLock();
  // Acquire write lock on the strand.
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Error conditions. If the deadline has expired, this call raises the
  // CommFailure exception.

  virtual void rdUnLock();// Release the read lock if this instance holds one
  virtual void wrUnLock();// Release the write lock if this instance holds one.

  virtual _CORBA_Boolean rdLockNonBlocking();
  // Acquire read lock but do not block if another thread is already
  // holding one. Return True if read lock is acquired.

  virtual void sleepOnRdLock();
  // Block until the read lock is available.
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Error conditions. If the deadline has expired, this call raises the
  // CommFailure exception.

  virtual void sleepOnRdLockAlways();
  // Irrespective of the state of the read lock. Block the thread
  // on the read lock condition variable until it is signalled by another
  // thread or the deadline in real time has expired.
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Error conditions. If the deadline has expired, this call raises the
  // CommFailure exception.


  static void wakeUpRdLock(giopStrand*);
  // Wakeup all those threads blocking to acquire a read lock.

  static _CORBA_Boolean noLockWaiting(giopStrand*);
  // Returns True (1) if no thread is waiting to acquire a read or write lock

  static _CORBA_Boolean rdLockNonBlocking(giopStrand*);
  // Acquire read lock but do not block if another thread is already
  // holding one. Return True if read lock is acquired. A variant of
  // the member func with the same name except that the caller do not
  // have a giopStream at hand.

  static void sleepOnRdLock(giopStrand*);
  // Block until the read lock is available. A variant of
  // the member func with the same name except that the caller do not
  // have a giopStream at hand.

  void markRdLock();
  // Caller must have called rdLockNonBlocking(giopStrand*) and succeeded.
  // Mark this instance as the one that have acquired the lock.

  static _CORBA_Boolean RdLockIsHeld(giopStrand*);
  // Return True (1) if the read lock is held by some giopStream.

  ////////////////////////////////////////////////////////////////////////
  virtual void notifyCommFailure(CORBA::Boolean heldlock,
				 CORBA::ULong& minor,
				 CORBA::Boolean& retry);
  // When the giopStream detects an error in sending or receiving data,
  // it raises the giopStream::CommFailure exception.
  // But before it does so, it calls this function to determine the 
  // the minor code of the exception. The function set the value.
  // Notice that this is a virtual function. The intention is for the
  // derived classes GIOP_S and GIOP_C to override this function in order
  // to intercept the callback. The override function can then look at its
  // internal state, e.g. the progress of a call, to decide what the
  // minor code should be.
  //
  // The function also set retry to TRUE(1) if the invocation should be
  // relaunched.
  //
  // Unless this function is overridden, the function returns minor = 0
  // retry = 0.
  //
  // Thread Safety preconditions:
  //    Internally, omniTransportLock is used for synchronisation, if
  //    <heldlock> is TRUE(1), the caller already hold the lock.

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  // CommFailure is the exception raised by giopStream when it details an
  // error in sending or receiving data in any of member functions derived
  // from cdrStream. (see below)
  class CommFailure {
  public:
    inline CommFailure(CORBA::ULong minor,
		       CORBA::CompletionStatus status,
		       CORBA::Boolean retry) :
      pd_minor(minor), pd_status(status), pd_retry(retry) {}

    // minor      - error no.
    // status     - completion status
    // retry      - TRUE(1) the invocation SHOULD BE relaunched
    //              FALSE(0) don't bother
    // filename   - source file
    // linenumber - line number where the exception is raised.
    // message    - reason for the exception
    // strand     - strand the exception occurred with

    inline ~CommFailure() {}

    inline CORBA::ULong minor() const { return pd_minor; }
    inline CORBA::CompletionStatus completed() const { return pd_status; }
    inline CORBA::Boolean retry() const { return pd_retry; }

    inline CommFailure(const CommFailure& e) :
      pd_minor(e.pd_minor), pd_status(e.pd_status), pd_retry(e.pd_retry) {}

    inline CommFailure& operator=(const CommFailure& e) {
      pd_minor = e.pd_minor;
      pd_status = e.pd_status;
      pd_retry = e.pd_retry;
      return *this;
    }

    static void _raise(CORBA::ULong minor,
		       CORBA::CompletionStatus status,
		       CORBA::Boolean retry,
		       const char* filename,
		       CORBA::ULong linenumber,
		       const char* message,
		       giopStrand* strand);

  private:
    CORBA::ULong            pd_minor;
    CORBA::CompletionStatus pd_status;
    CORBA::Boolean          pd_retry;
  };
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  static void dumpbuf(unsigned char* buf, size_t sz);
  // Dump the buffer to the log. Logs just the first 128 bytes unless
  // omniORB::traceLevel >= 40.


protected:
  giopStrand*                pd_strand;
  CORBA::Boolean             pd_rdlocked;
  CORBA::Boolean             pd_wrlocked;
  giopStreamImpl*            pd_impl;
  unsigned long              pd_deadline_secs;
  unsigned long              pd_deadline_nanosecs;

private:
  giopStream();
  giopStream(const giopStream&);
  giopStream& operator=(const giopStream&);

public:
  // These tunable parameters are used to determine at what size an
  // octet array will be sent/received directly to/from the network
  // connection without staging through the internal buffer.
  static _core_attr CORBA::ULong directSendCutOff;
  static _core_attr CORBA::ULong directReceiveCutOff;

  static _core_attr CORBA::ULong bufferSize;
  // Allocate this number of bytes for each giopStream_Buffer.

public:
  // The following implement the abstract functions defined in cdrStream
  //
  // In addition to the CORBA SystemException, the member functions will
  // raise the giopStream::CommFailure instead of CORBA::COMM_FAILURE exception
  // to indicate an error in sending or receiving data. 

  void put_octet_array(const _CORBA_Octet* b, int size,
		       omni::alignment_t align=omni::ALIGN_1);
  void get_octet_array(_CORBA_Octet* b,int size,
		       omni::alignment_t align=omni::ALIGN_1);
  void skipInput(_CORBA_ULong);
  _CORBA_Boolean checkInputOverrun(_CORBA_ULong, _CORBA_ULong,
				   omni::alignment_t align=omni::ALIGN_1);
  _CORBA_Boolean checkOutputOverrun(_CORBA_ULong,_CORBA_ULong,
				    omni::alignment_t align=omni::ALIGN_1);
  void fetchInputData(omni::alignment_t,size_t);
  _CORBA_Boolean reserveOutputSpaceForPrimitiveType(omni::alignment_t,size_t);
  _CORBA_Boolean maybeReserveOutputSpace(omni::alignment_t,size_t);

  _CORBA_Boolean is_giopStream();

  _CORBA_ULong currentInputPtr() const;
  _CORBA_ULong currentOutputPtr() const;


  friend class giopImpl10;
  friend class giopImpl11;
  friend class giopImpl12;


  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  // Receiving GIOP messages: 
  // GIOP messages are read from the strand via the member function
  // inputMessage() and inputChunk(). Both functions return a
  // giopStream_Buffer. The buffer should be returned to the strand using
  // the releaseInputBuffer() function.
  //
  // struct giopStream_Buffer {
  //   CORBA::ULong        start;   /* offset to the beginning of data */
  //   CORBA::ULong        end;     /* offset to the end of buffer */
  //   CORBA::ULong        last;    /* offset to the last data byte */
  //   CORBA::ULong        size;    /* GIOP message size. */
  //   inputBuffer*        next;    /* next inputBuffer in a chain */
  //   buffer data to follows.
  //   static void deleteBuffer(inputBuffer*);
  // };
  giopStream_Buffer* newInputBuffer(CORBA::ULong sz=0);
  // Return a giopStream_Buffer. Should be deleted by calling
private:

  // The following variables must be initialised to 0 by reset().
  giopStream_Buffer*         pd_currentInputBuffer;
  giopStream_Buffer*         pd_input;
  CORBA::Boolean             pd_inputFullyBuffered;
  CORBA::Boolean             pd_inputMatchedId;

  // The following variables can be left uninitialised and will be
  // written with a sensible value when used.
  CORBA::Boolean             pd_inputExpectAnotherFragment;
  CORBA::ULong               pd_inputFragmentToCome;
  CORBA::ULong               pd_inputMessageSize;

  inline CORBA::Boolean inputFullyBuffered() const { 
    return pd_inputFullyBuffered;
  }

  inline void inputFullyBuffered(CORBA::Boolean b) {
    pd_inputFullyBuffered = b;
  }

  inline CORBA::Boolean inputExpectAnotherFragment() const {
    return pd_inputExpectAnotherFragment;
  }

  inline void inputExpectAnotherFragment(CORBA::Boolean yes ) {
    pd_inputExpectAnotherFragment = yes;
  }

  inline CORBA::Boolean inputMatchedId() const {
    return pd_inputMatchedId;
  }

  inline void inputMatchedId(CORBA::Boolean yes ) {
    pd_inputMatchedId = yes;
  }

  inline CORBA::ULong inputMessageSize() const {
    return pd_inputMessageSize;
  }

  inline void inputMessageSize(CORBA::ULong msgsz) {
    pd_inputMessageSize = msgsz;
  }

  inline CORBA::ULong inputFragmentToCome() const {
    return pd_inputFragmentToCome;
  }

  inline void inputFragmentToCome(CORBA::ULong fsz) {
    pd_inputFragmentToCome = fsz;
  }

  giopStream_Buffer* inputMessage();
  // Read the next GIOP message from the connection. This function does the
  // basic sanity check on the message header. 
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Error conditions. If the data do not look like a GIOP message or
  // the connection reports an error in reading data or the deadline has
  // expired, this call raises the CommFailure exception. The strand
  // will be marked as DYING.
  // 
  // The size of the GIOP message is reported in <size>.
  // The returned buffer may or may not contain the complete message, i.e.
  // <last> - <start> may or may not equal to <size>. If it is the latter,
  // the caller should use inputChunk() to get the rest of the message.
  //
  // Thread Safety preconditions:
  //   Caller must have acquired the read lock on the strand.
  
  giopStream_Buffer* inputChunk(CORBA::ULong maxsize);
  // Same as inputMessage except that no sanity check is done on the
  // data received. The buffer returned contains up to maxsize bytes of
  // data. The <size> field is not applicable and is set to 0.
  //
  // Thread Safety preconditions:
  //   Caller must have acquired the read lock on the strand.

  void inputCopyChunk(void* dest,CORBA::ULong size);
  // Same as inputChunk except that data is copied directly into the
  // destination buffer.
  //
  // Thread Safety preconditions:
  //   Caller must have acquired the read lock on the strand.

  void releaseInputBuffer(giopStream_Buffer*);
  // Return the buffer given out by inputMessage and inputChunk.
  //
  // Thread Safety preconditions:
  //   None.

  void errorOnReceive(int,const char*,CORBA::ULong,giopStream_Buffer*,
		      CORBA::Boolean,const char*);
  // internal helper function, do not use outside this class

  CORBA::ULong ensureSaneHeader(const char*,CORBA::ULong,
				giopStream_Buffer*,CORBA::ULong);
  // internal helper function, do not use outside this class


  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  // Sending GIOP messages:

  // The following variables must be initialised to 0 at ctor.
  giopStream_Buffer*         pd_currentOutputBuffer;

  // The following variables can be left uninitialised and will be
  // written with a sensible value when used.
  CORBA::ULong               pd_outputFragmentSize;
  CORBA::ULong               pd_outputMessageSize;
  CORBA::ULong               pd_request_id;

  inline CORBA::ULong outputMessageSize() const {
    return pd_outputMessageSize;
  }

  inline void outputMessageSize(CORBA::ULong msgsz) {
    pd_outputMessageSize = msgsz;
  }

  inline CORBA::ULong outputFragmentSize() const {
    return pd_outputFragmentSize;
  }

  inline void outputFragmentSize(CORBA::ULong fsz) {
    pd_outputFragmentSize = fsz;
  }

  // GIOP message are sent via these member functions

  void sendChunk(giopStream_Buffer*);
  // Send the buffer to the strand.
  //
  // The function honours the deadline set on the the object. If the deadline
  // is reached, the function should give up waiting.
  //
  // Thread Safety preconditions:
  //   Caller must have acquired the write lock on the strand.

  void sendCopyChunk(void*,CORBA::ULong size);
  // Same as sendChunk() except that the data is copied directly from
  // the application buffer.
  //
  // Thread Safety preconditions:
  //   Caller must have acquired the write lock on the strand.

  void errorOnSend(int,const char*,CORBA::ULong,CORBA::Boolean,const char*);
  // internal helper function, do not use outside this class

protected:
  //////////////////////////////////////////////////////////////////
  inline CORBA::ULong  requestId() const { return pd_request_id; }
  inline void requestId(CORBA::ULong v) { pd_request_id = v; }

};


OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __GIOPSTREAM_H__
