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
  Revision 1.1.2.3  1999/11/04 20:20:15  sll
  GIOP engines can now do callback to the higher layer to calculate total
  message size if necessary.
  Where applicable, changed to use the new server side descriptor-based stub.

  Revision 1.1.2.2  1999/10/05 20:35:32  sll
  Added support to GIOP 1.2 to recognise all TargetAddress mode.
  Now handles NEEDS_ADDRESSING_MODE and LOC_NEEDS_ADDRESSING_MODE.

  Revision 1.1.2.1  1999/09/15 20:35:44  sll
  *** empty log message ***


*/

#ifndef __GIOPSTREAM_H__
#define __GIOPSTREAM_H__

class giopMarshaller;
class giopStreamImpl;
class giop_1_0_Impl;
class giop_1_1_Impl;
class giop_1_2_Impl;
class giop_1_2_buffered_Impl;

class giopStream : public cdrStream, Strand_Sync {
public:

  class requestInfo;

  static giopStream* acquire(Rope* r, GIOP::Version v);
  // Acquire a giopStream for use as a GIOP client.
  // The protocol version to be used is specified in v.
  // When finished, the giopStream must be returned with release.
  // This function is atomic and thread safe.

  static giopStream* acquire(Strand* s);
  // Acquire a giopStream for use as a GIOP server.
  // The protocol version to be used is determined by the incoming
  // message.
  // When finished, the giopStream must be returned with release.
  // This function is atomic and thread safe.

  void release();
  // Release the giopStream.
  // This function resets the giopStream to an idle state. Any
  // exclusive lock on the underlying strand would be released.
  // This function is atomic and thread safe.

  _CORBA_Long outputRequestMessageBegin(GIOPObjectInfo* f,
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


  _CORBA_Long outputLocateMessageBegin(GIOPObjectInfo* f);
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

    inline int keysize() const { return pd_objkeysize; }

    inline _CORBA_Octet* key() const {
      return ((pd_objkeysize <= (int)sizeof(omniObjectKey)) ? 
	      (_CORBA_Octet*) (&pd_objkey) : foreign);
    }

    inline void keysize(_CORBA_ULong sz) {
      if (pd_objkeysize > (int)sizeof(omniObjectKey)) {
	delete [] foreign;
	pd_objkeysize = (int) sizeof(omniObjectKey);
      }
      if (sz > sizeof(omniObjectKey)) {
	foreign = new _CORBA_Octet[sz];
      }
      pd_objkeysize = (int)sz;
    }

    inline void resetKey() { 
      if (pd_objkeysize > (int)sizeof(omniObjectKey)) delete [] foreign;
      pd_objkeysize = -1; 
    }

    inline GIOP::IORAddressingInfo& targetAddress() { 
      return pd_target_address;
    }

    void unmarshalIORAddressingInfo(cdrStream& s);

    inline requestInfo() : pd_request_id(0), 
                           pd_response_expected(0),
                           pd_result_expected(0),
                           pd_objkeysize(-1),
		           pd_operation((char*)pd_op_buffer),
		           pd_principal(pd_pr_buffer),
                           pd_principal_len(0) {}

    inline ~requestInfo() {
      if (pd_objkeysize > (int)sizeof(omniObjectKey)) delete [] foreign;
      if (pd_operation != (char*)pd_op_buffer) delete [] pd_operation;
      if (pd_principal != pd_pr_buffer) delete [] pd_principal;
    }

  private:
    _CORBA_ULong    pd_request_id;
    _CORBA_Boolean  pd_response_expected;
    _CORBA_Boolean  pd_result_expected;

    union {
      _CORBA_Octet*  foreign;
      omniObjectKey  pd_objkey;
    };
    int             pd_objkeysize;
  
    char*           pd_operation;
    char            pd_op_buffer[GIOPSTREAM_INLINE_BUF_SIZE];
    _CORBA_Octet*   pd_principal;
    _CORBA_Octet    pd_pr_buffer[GIOPSTREAM_INLINE_BUF_SIZE];
    _CORBA_ULong    pd_principal_len;

    // If pd_objkeysize < 0, pd_target_address contains the
    // full IOR that the client has sent. Only used in GIOP 1.2.
    GIOP::IORAddressingInfo   pd_target_address;

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

  GIOP::ReplyStatusType inputReplyMessageBegin(_CORBA_ULong reqid);
  // Begin reading a GIOP Reply message. The Reply header is read from the
  // stream at the end of this call.
  // The value of the request id field in the reply message header should 
  // be the same as the argument <reqid>.
  // Return the status field of the Reply header.

  GIOP::LocateStatusType inputLocateReplyMessageBegin(_CORBA_ULong reqid);
  // Begin reading a GIOP Locate Reply message. The Reply header is read
  // from the stream at the end of this call.  The value of the request id
  // field in the reply message header should be the same as the argument
  // <reqid>.  Return the status field of the Reply header.

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

  giopStreamImpl* pd_impl;
  _CORBA_Boolean  pd_rdlocked;
  _CORBA_Boolean  pd_wrlocked;

  omni_condition  pd_cond;
  int             pd_nwaiting;

  int             pd_clicks;

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

  _CORBA_ULong      pd_request_id;

  enum { UnUsed,
	 OutputIdle, 
	 OutputRequest, 
	 OutputLocateRequest,
	 OutputReply, 
	 OutputLocateReply,
	 OutputRequestCompleted,
	 InputIdle,
	 InputRequest,
	 InputReply,
	 InputLocateReply,
         InputRequestCompleted } pd_state;
	       
public:
  // The following implement the abstract functions defined in cdrStream
  void put_char_array(const _CORBA_Char*, int,
		      omni::alignment_t align=omni::ALIGN_1);
  void get_char_array(_CORBA_Char*,int,
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

  // Overload the following virtual functions in Sync class
  _CORBA_Boolean garbageCollect();
  _CORBA_Boolean is_unused();
};

// The giopMarshaller interface is implemented by the stub
// marshalling code or the header marshalling code. An instance of such
// a class is attached to the giopStream. Its internal state contains all
// the information necessary to marshal a set of data items to the stream.
// Two methods are provided:
//      call marshalData() to marshal the data items
//      call dataSize() to calculate the total length of the data items.
// 
// When the giopStream is about to flush its internal buffer to the
// network, it may have to find out the total length of the message before
// all the elements are marshalled into its internal buffer. The giopStream
// calls the dataSize() method of this callback object to obtain
// the length of the message.
class giopMarshaller {
public:
  virtual void marshalData() = 0;
  virtual size_t dataSize(size_t initialoffset) = 0;
};

#endif /* __CDRSTREAM_H__ */
