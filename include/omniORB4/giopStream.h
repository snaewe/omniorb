// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopStream.h               Created on: 11/1/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1999 AT&T Laboratories, Cambridge
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
  Revision 1.1.2.3  2000/11/15 17:07:47  sll
  Added giopStreamInfo. This structure contains all the state variables
  related to the giop setup on the strand.

  Revision 1.1.2.2  2000/11/03 18:49:16  sll
  Separate out the marshalling of byte, octet and char into 3 set of distinct
  marshalling functions.
  Renamed put_char_array and get_char_array to put_octet_array and
  get_octet_array.
  New string marshal member functions.

  Revision 1.1.2.1  2000/09/27 16:54:08  sll
  *** empty log message ***

*/

#ifndef __GIOPSTREAM_H__
#define __GIOPSTREAM_H__

class giopMarshaller;
class giopStreamInfo;
class giopStreamImpl;
class giop_1_0_Impl;
class giop_1_1_Impl;
class giop_1_2_Impl;
class giop_1_2_buffered_Impl;

class giopStream : public cdrStream {
public:

  class requestInfo;

  static giopStream* acquireClient(Rope* r, GIOP::Version v);
  // Acquire a giopStream for use as a GIOP client.
  // The protocol version to be used is specified in v.
  // When finished, the giopStream must be returned with release.
  // This function is atomic and thread safe.

  static giopStream* acquireServer(Strand* s);
  // Acquire a giopStream for use as a GIOP server.
  // The protocol version to be used is determined by the incoming
  // message.
  // When finished, the giopStream must be returned with release.
  // This function is atomic and thread safe.

  void release();
  // Release the giopStream obtained by acquireClient or acquireServer.
  // This function resets the giopStream to an idle state. Any
  // exclusive lock on the underlying strand would be released.
  // This function is atomic and thread safe.

  _CORBA_Long outputRequestMessageBegin(omniIOR* ior,
					const char* opname,
					size_t opnamesize,
					_CORBA_Boolean oneway,
					_CORBA_Boolean reponse_expected);
  // Begin a GIOP message and write a Request Header using the info
  // provided in the function's arguments.
  // The return value is the request ID used on the wire to identify the
  // reply to this message.
  // XXX For the moment, the following invariant must be true:
  //         <oneway> == !<response_expected>
  //     This will be relax in future to allow for optional use of
  //     the response_flags feature in GIOP 1.2 or above.


  _CORBA_Long outputLocateMessageBegin(omniIOR* ior);
  // Begin a GIOP message and write a LocateRequest Header using the info
  // provided in the function's arguments.
  // The return value is the request ID used on the wire to identify the
  // reply to this message.

  void outputReplyMessageBegin(requestInfo& f,GIOP::ReplyStatusType status);
  // Begin a GIOP message and write a Reply Header using the info
  // provided in the function's arguments.

  void outputLocateReplyMessageBegin(requestInfo& f,GIOP::LocateStatusType status);
  // Begin a GIOP message and write a Reply Header using the info
  // provided in the function's arguments.

  void outputMessageEnd();
  // Signifies the end of the GIOP message currently being written.
  //
  // Since an earlier call to output*MessageBegin()s, the stub
  // code have marshalled arguments into the stream.

  void SendMsgErrorMessage();
  // Write a GIOP MessageError message.
  // Pre-condition:
  //     The caller should not have called output*MessageBegin()s.

  void outputMessageBodyMarshaller(giopMarshaller& m);
  // Register the marshaller object that will marshal the message
  // body. If the giopStream implementation requires to calculate
  // the message body size, it will call m.dataSize().
  // Notice that registering the marshaller does not mean that the
  // giopStream implementation will call its m.marshalData() method
  // automatically. On the contrary, the caller should call
  // m.marshalData() after it has called one of the output*MessageBegin()
  // function.
  // A subsequent call to outputMessageEnd() or release() automatically
  // unregister the marshaller.
  //
  // Typical usage is as follows:
  //     giopStream& s; 
  //     giopMarshaller& m; /* m contains info to marshal data into s */
  //      ...
  //     s.outputMessageBodyMarshaller(m);
  //     s.outputReplyMessageBegin(f,GIOP::NO_EXCEPTION);
  //     m.marshalData();  /* This member marshal data into s */
  //     s.outputMessageEnd();
  //
  // Pre-condition:
  //     The caller should not have called output*MessageBegin()s.

  class requestInfo {
  public:
# define GIOPSTREAM_INLINE_BUF_SIZE 32

    inline _CORBA_ULong  requestID() const { return pd_request_id; }
    inline void requestID(_CORBA_ULong v) { pd_request_id = v; }

    inline _CORBA_Boolean response_expected() const { 
      return pd_response_expected; 
    }
    inline void response_expected(_CORBA_Boolean v) { 
      pd_response_expected = v;
    }
    inline void result_expected(_CORBA_Boolean v) { 
      pd_result_expected = v;
    }
    // If result_expected returns 0, then one can skip the
    // marshalling of the results completely. It is somewhat
    // redundent information for us because we can make that decision
    // purely on the value of response_expected.
    inline _CORBA_Boolean result_expected() const {
      return pd_result_expected;
    }

    inline char* operation() const { return pd_operation; }

    inline void  set_operation_size(_CORBA_ULong sz) {
      if (pd_operation != pd_op_buffer) {
	delete [] pd_operation;
	pd_operation = pd_op_buffer;
      }
      if (sz > GIOPSTREAM_INLINE_BUF_SIZE) {
	pd_operation = new char[sz];
      }
    }

    inline _CORBA_Octet* principal() const { return pd_principal; }
    inline _CORBA_ULong principal_size() const { return pd_principal_len; }
    
    inline void set_principal_size(_CORBA_ULong sz) {
      if (pd_principal != pd_pr_buffer) {
	delete [] pd_principal;
	pd_principal = pd_pr_buffer;
      }
      if (sz > GIOPSTREAM_INLINE_BUF_SIZE) {
	pd_principal = new _CORBA_Octet[sz];
      }
    }

    int keysize() const {
      return pd_key.size();
    }

    _CORBA_Octet* key() const {
      return (_CORBA_Octet*)pd_key.key();
    }

    void keysize(_CORBA_ULong sz) {
      pd_key.set_size((int)sz);
    }

    void resetKey() {
      pd_key.set_size(0);
    }


    IOP::ServiceContextList& service_contexts() { return pd_service_contexts; }

    inline GIOP::IORAddressingInfo& targetAddress() { 
      return pd_target_address;
    }

    void unmarshalIORAddressingInfo(cdrStream& s);

    inline void set_user_exceptions(const char*const* ues, int n) {
      pd_user_exns = ues;  pd_n_user_exns = n;
    }

    inline const char*const* user_exceptions() { return pd_user_exns; }
    inline int n_user_exceptions() { return pd_n_user_exns; }

    inline requestInfo() : pd_request_id(0), 
                           pd_response_expected(0),
                           pd_result_expected(0),
		           pd_operation((char*)pd_op_buffer),
		           pd_principal(pd_pr_buffer),
                           pd_principal_len(0),
                           pd_user_exns(0),
                           pd_n_user_exns(0) {}

    inline ~requestInfo() {
      if (pd_operation != (char*)pd_op_buffer) delete [] pd_operation;
      if (pd_principal != pd_pr_buffer) delete [] pd_principal;
    }

  private:
    _CORBA_ULong            pd_request_id;
    _CORBA_Boolean          pd_response_expected;
    _CORBA_Boolean          pd_result_expected;
    omniObjKey              pd_key;
    char*                   pd_operation;
    char                    pd_op_buffer[GIOPSTREAM_INLINE_BUF_SIZE];
    _CORBA_Octet*           pd_principal;
    _CORBA_Octet            pd_pr_buffer[GIOPSTREAM_INLINE_BUF_SIZE];
    _CORBA_ULong            pd_principal_len;
    IOP::ServiceContextList pd_service_contexts;

    // If pd_objkeysize < 0, pd_target_address contains the
    // full IOR that the client has sent. Only used in GIOP 1.2.
    GIOP::IORAddressingInfo   pd_target_address;

    const char*const* pd_user_exns;
    int               pd_n_user_exns;
    // A list of the user exceptions that current operation may raise
  };

  GIOP::MsgType inputRequestMessageBegin(requestInfo& f);
  // Begin reading a GIOP message expected on the server side. Only the 
  // GIOP header is read from the stread at the end of this call.
  // The returned message type can be any of the following:
  //         Request
  //         LocateRequest
  //         CancelRequest
  //         MessageError
  //         CloseConnection
  //
  // Return the message type value.
  // If the message type is Request or LocateRequest, the RequestHeader
  // or LocateRequestHeader info are unmarshalled into the requestInfo 
  // argument.
  // 

  GIOP::ReplyStatusType inputReplyMessageBegin();
  // Begin reading a GIOP Reply message. The Reply header is read from the
  // stream at the end of this call.
  // Return the status field of the Reply header.

  GIOP::LocateStatusType inputLocateReplyMessageBegin();
  // Begin reading a GIOP Locate Reply message. The Reply header is read
  // from the stream at the end of this call. 
  // Return the status field of the Reply header.

  void inputMessageEnd(_CORBA_Boolean disgard=0,
		       _CORBA_Boolean error=0);
  // GIOP message header processing
  //
  // Call this function to signify the end of a GIOP message.
  // In between an earlier call to inputMessageBegin() and this call, the
  // stub code may have unmarshalled data from the input stream.
  // This function checks if there is any data left (as indicated by the
  // message size in the header) and process accordingly.
  // If <disgard> is TRUE(1), skip the input data till the end of the 
  // current message.
  // If <error> is TRUE(1), serious error has been detected and no data
  // can be accepted any further. Cleanup appropriately and then throw a
  // COMM_FAILURE exception.

  _CORBA_Boolean terminalError() const;
  // Returns TRUE(1) if there is any serious error with the input or output
  // stream. No data can be sent or received if this funcion returns TRUE.

  void setTerminalError();
  // Call this function to signify that the upper layer has detected a
  // serious error in the input or output stream.

  virtual ~giopStream();

  friend class giopStreamImpl;
  friend class giop_1_0_Impl;
  friend class giop_1_1_Impl;
  friend class giop_1_2_Impl;
  friend class giop_1_2_buffered_Impl;

  // The following member functions:
  //     void startSavingInputMessageBody();
  //     void prepareCopyMessageBodyFrom(giopStream& s);
  //     void copyMessageBodyFrom(giopStream& s);
  //
  // are provided to perform fast copy of an input message from one
  // giopStream to an output message of another giopStream. The task
  // of unmarshalling the input and re-marshalling the data to the
  // output is eliminated.
  // The way to do this is as follows:
  //    giopStream* input;
  //    giopStream* ouput;
  //     ...   /* Unmarshal the message header from input */
  //    (void) input->startSavingInputMessageBody();
  //     ...   /* May be partially unmarshal the input message here. */
  //    (void) output->prepareCopyMessageBodyFrom(input);
  //    ...
  //    output->outputRequestMessageBegin(....);
  //    output->copyMessageBodyFrom(*input);
  //    output->outputMessageEnd();
  //
  // The input and the output giopStreams may be encoded in different
  // GIOP versions. Even if they are of the same GIOP version, other
  // states of the streams, such as the CodeSet parameters may be different.
  // As a result of these variations, the fast copy may not be used.
  //
  // To ascertain if the fast copy can proceed, the caller should check
  // the return value of prepareCopyMessageBodyFrom(). If it returns TRUE(1),
  // the fast copy can go ahead, otherwise fast copy should not be used.
  // In the latter case, calling copyMessageBodyFrom subsequently is illegal.
  //
  _CORBA_Boolean startSavingInputMessageBody();
  // Start saving the input message. The saved message body is available
  // for copying into an output message of another giopStream using
  // the giopStream::copyMessageBodyFrom member function.
  //
  // Return TRUE(1) if this operation is supported, otherwise return
  // FALSE(0) and the message body is not saved.
  //
  // The saved message body remains until inputMessageEnd is called.
  // With the first call to startSavingInputMessageBody since a call
  // to input{Request|Reply}MessageBegin, the giopStream stores the current
  // buffer pointer and start saving the message body
  // Another call to startSavingInputMessageBody resets the buffer pointer
  // to the stored value. After this call, the full message body can
  // be unmarshalled from the stream again.

  _CORBA_Boolean prepareCopyMessageBodyFrom(giopStream& s);
  // Prepare this stream to output a message that contains the
  // body from the giopStream <s>.
  //
  // Return TRUE(1) if the fast copy can proceed, i.e. copyMessageBodyFrom
  // can be called. Otherwise returns FALSE(0). In this case,
  // copyMessageBodyFrom should not be called and the state of the
  // giopStream remains unchanged.

  void copyMessageBodyFrom(giopStream& s);
  // Copy the message body from the giopStream <s> to the output message


private:
  giopStream(Strand*);

  giopStream();
  giopStream(const giopStream&);
  giopStream& operator=(const giopStream&);

  omni_mutex& getSyncMutex();

  void rdLock();
  void wrLock();
  void rdUnlock();
  void wrUnlock();
  static _CORBA_Boolean rdLockNonBlocking(Strand*);
  // Try to acquire the read lock. Returns TRUE if the lock is acquired
  // without blocking. Returns FALSE if the lock has already been taken.
  //
  static void sleepOnRdLock(Strand*);
  // Block until a read lock is released or no thread currently holds the
  // read lock.
  //

  static void wakeUpRdLock(Strand*);
  // Wake up all threads currently block on getting a read lock.

    // IMPORTANT: to avoid deadlock, the following protocol MUST BE obeyed.
    //            1. Acquire Read lock before Write Lock.
    //            2. Never acquire a Read lock while holding a Write lock.
    //               Must release the Read lock first.
    // Concurrency Control:
    // 	  MUTEX = pd_strand->pd_rope->pd_lock
    // Pre-condition:
    //    Must Hold <MUTEX> on enter.
    // Post-condition:
    //    Still hold <MUTEX> on exit.
    //

  static giopStream* findOnly(Strand* s,_CORBA_ULong reqid);
  // Search the giopStream instances attached to the strand <s>.
  // Return the instance with the same reqid in pd_request_id.
  // If none is found, return 0.
  // Concurrency Control:
  // 	  MUTEX = s->pd_rope->pd_lock
  // Pre-condition:
  //    MUST hold <MUTEX> on enter.
  // Post-condition:
  //    Still hold <MUTEX> on exit.
  //
  static giopStream* findOrCreateBuffered(Strand* s,_CORBA_ULong reqid,
					  _CORBA_Boolean& new1);
  // Search the giopStream instances attached to the strand <s>.
  // Return the instance with the same reqid in pd_request_id and set
  // new1 to FALSE(0).
  // If none is find, create and return a new instance, set its pd_state to
  // InputPartiallyBuffered and set new1 to TRUE(1).
  //
  // Concurrency Control:
  // 	  MUTEX = s->pd_rope->pd_lock
  // Pre-condition:
  //    Does not hold <MUTEX> on enter.
  // Post-condition:
  //    Does not hold <MUTEX> on exit.
  //


  void changeToFullyBuffered();
  //
  // Concurrency Control:
  // 	  MUTEX = pd_strand->pd_rope->pd_lock
  // Pre-condition:
  //     Does not hold <MUTEX> on enter.
  //     pd_state == InputPartiallyBuffered.
  // Post-condition:
  //     pd_state == InputFullyBuffered
  //     Does not hold <MUTEX> on exit.
  // 
  // Call this function if this instance fully buffered a giop message.

  void transferReplyState(giopStream* dest);
  // Helper function for giopStream implementation classes.
  // Swap the input state of this instance with that of <dest>
  //
  // Concurrency Control:
  // 	  MUTEX = pd_strand->pd_rope->pd_lock
  // Pre-condition:
  //    MUST hold <MUTEX> on enter.
  //    pd_state == InputReplyHeader
  //    (dest->pd_state == OutputRequestCompleted ||
  //     dest->pd_state == OutputRequest ||
  //     dest->pd_state == OutputLocateRequest)
  //    Held read lock on strand (pd_rdlocked == 1)
  //    
  // Post-condition:
  //    Still hold <MUTEX> on exit.
  //    pd_state == OutputRequestCompleted
  //    dest->pd_state == InputReply
  //    Does not hold read lock on strand.
  //    The read lock now belongs to dest (dest->pd_rdlocked == 1)

  void deleteThis();
  // Delete this giopStream instances.
  // Caller must ensure that no thread is holding a reference to this instance.
  // On return, the instance is deleted.
  // 
  // Concurrency Control:
  // 	  MUTEX = s->pd_rope->pd_lock
  // Pre-condition:
  //    Do not Hold <MUTEX> on enter.
  // Post-condition:
  //    Do not hold <MUTEX> on exit.

public:
  static void deleteAll(Strand* s);
  // Delete all giopStream instances associated with this strand.
  // Caller must ensure that no thread is holding a reference to any
  // of the instances.
  // 
  // Concurrency Control:
  // 	  MUTEX = s->pd_rope->pd_lock
  // Pre-condition:
  //    Must Hold <MUTEX> on enter.
  // Post-condition:
  //    Still hold <MUTEX> on exit.

  inline giopStreamInfo* streamInfo() {
    return pd_strand->pd_giop_info;
  }

  GIOP::Version version();

private:
  giopStreamImpl* pd_impl;
  _CORBA_Boolean  pd_rdlocked;
  _CORBA_Boolean  pd_wrlocked;

  giopStream*     pd_next;
  Strand*         pd_strand;

  _CORBA_ULong    pd_request_id;

  enum { UnUsed,
	 OutputIdle, 
	 OutputRequest, 
	 OutputLocateRequest,
	 OutputReply, 
	 OutputLocateReply,
	 OutputRequestCompleted,
	 InputIdle,
	 InputRequest,
	 InputReplyHeader,
	 InputReply,
	 InputLocateReplyHeader,
	 InputLocateReply,
         InputRequestCompleted,
         InputPartiallyBuffered,
         InputFullyBuffered } pd_state;

  union {
    GIOP::ReplyStatusType replyStatus;
    GIOP::LocateStatusType locateReplyStatus;
  } pd_reply_status;


  //----------------------------------------------------------
  // The following data members are **strictly private** to
  // *pd_impl and should not be read or written by any of
  // the member functions of this class or its friends.
  //
  void*           pd_inb_begin;
  void*           pd_outb_begin;
  size_t          pd_input_msgbody_received;
  // no. of bytes in the message body that has been received so far.
  size_t          pd_input_msgfrag_to_come;
  // no. of bytes in the current fragment that is still to be read from 
  // the strand. For GIOP 1.1 or higher
  _CORBA_Boolean  pd_input_fragmented;
  // TRUE(1) if the current message is divided into multiple fragments.

  void*           pd_input_buffers;
  void*           pd_input_current_buffer;
  void*           pd_input_hdr_end;

  size_t          pd_output_msgfrag_size;
  // Size of the current fragment to send. In case of GIOP 1.0, there is
  // only 1 fragment per GIOP message. Higher protocol versions may have
  // 1 or more fragments.
  // If this value is 0, the size field in the GIOP message header has not
  // been written. Otherwise this is the value already written into the
  // header. It is only in the latter state that the message header is
  // flushed to the network.
  
  size_t           pd_output_msgsent_size;
  // no. of bytes in the message body that has been sent so far.
  // Notice that the bytes may have been sent in separate GIOP fragments.

  void*            pd_output_hdr_end;

  _CORBA_Boolean   pd_output_at_most_once;

  giopMarshaller*  pd_output_header_marshaller;
  // Call back object to calculate the message header size

  giopMarshaller*  pd_output_body_marshaller;
  // Call back object to calculate the message body size


	       
public:
  // The following implement the abstract functions defined in cdrStream
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
  size_t maxFetchInputData(omni::alignment_t) const;
  _CORBA_Boolean reserveOutputSpace(omni::alignment_t,size_t);
  size_t maxReserveOutputSpace(omni::alignment_t) const;
  _CORBA_ULong currentInputPtr() const;
  _CORBA_ULong currentOutputPtr() const;
};

// The giopMarshaller interface is implemented by the stub
// marshalling code or the header marshalling code. An instance of such
// a class is attached to the giopStream. Its internal state contains all
// the information necessary to marshal a set of data items to the stream.
//
// The giopStream calls the only member function marshal() to cause the 
// data to be marshalled into the argument cdrStream. Calling the function
// with the cdrCountingStream can be used to calculate exactly how much space
// is need to store the data.
class giopMarshaller {
public:
  virtual void marshal(cdrStream&) = 0;
};


// The giopStreamInfo contains the GIOP related state associated with a 
// strand. Each strand instance is allocated one of this.
//
class giopStreamInfo {
public:

  giopStream*         head;
  // head  - a strand may have more than one giopStream instances associated
  // 	     with it. Mostly this is because from GIOP 1.2 onwards, requests
  //         can be interleaved on a connection. Each of these request is
  //         represented by a giopStream insteance. They are linked together
  //         from <head>

  _CORBA_Boolean      tcs_selected;
  omniCodeSet::TCS_C* tcs_c;
  omniCodeSet::TCS_W* tcs_w;
  GIOP::Version       version;
  // The transmission codesets for char, wchar, string and wstring are selected
  // by the client based on the codeset info in the IOR. The client informs
  // the server of its selection using a codeset service context. This is
  // done only once per lifetime of a connection (strand). 
  // If <tcs_selected> == 1,
  //    <tcs_c>, <tcs_w> and <version> records the chosen code set convertors
  //    and the GIOP version for which the convertors apply.

  _CORBA_Boolean      biDir;
  // Indicate if the strand is used for bidirectional GIOP.

  giopStreamInfo() : head(0), tcs_selected(0), tcs_c(0), tcs_w(0), biDir(0) {
    version.major = version.minor = 0;
  }
};

#endif /* __CDRSTREAM_H__ */
