// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopStreamImpl.h           Created on: 11/2/99
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
  Revision 1.1.2.1  1999/09/15 20:37:29  sll
  *** empty log message ***


*/

#ifndef __GIOPSTREAMIMPL_H__
#define __GIOPSTREAMIMPL_H__

// giopStreamImpl defines an abstract interface for which each giop protocol
// revision has its own implementation.
//
// The purpose of giopStreamImpl is to provide support functions to
// giopStream. The support functions insulate giopStream from revision
// specific processing. For the moment, the three supported GIOP revisions
// are 1.0, 1.1 and 1.2.
// 
// The functions can be classified into 2 categories:
//     1. Message Header processing
//        - New message type (FRAGMENT) is added to GIOP 1.1. Also the
//          content of the headers differ slightly between different versions.
//          However, the changes are upward compatiable.
//
//     2. Message Body processing
//        - from GIOP 1.1 onwards, a new message type FRAGMENT is introduced.
//          A GIOP request/reply can now be split into multiple GIOP FRAGMENT
//          messages. The support functions in giopStreamImpl hides away
//          the processing of FRAGMENTs and present giopStream with a
//          continuous byte stream as it is the case with GIOP 1.0
//
// Points to note:
// 1. Each protocol revision provides a **singleton** to implement 
//    giopStreamImpl.
// 2. Because the implementation class is a singleton, no private data should 
//    be added to the implementation classes. Should there be any need for 
//    private data members, these should be added to giopStream.
// 3. Because of 2, all implementation classes have to be declared 'friend'
//    of giopStream.


class giopStreamImpl {
public:

  static giopStreamImpl* matchVersion(const GIOP::Version&);
  // Return the singleton that implements the GIOP version specified.

  static giopStreamImpl* maxVersion();
  // Return the singleton that implements the highest GIOP version number

  inline GIOP::Version version() { return pd_version; }

  virtual void release(giopStream* g);

  //////////////////////////////////////////////////////////////////////////
  //       Support functions for message body processing
  //

  virtual size_t inputRemaining(giopStream* g) = 0;
    // Support function for message body processing.
    // return no. of bytes remaining in the input message. 

  virtual void getInputData(giopStream* g,
			    omni::alignment_t align,
			    size_t reqsize) = 0;
  // Support function for message body processing.
  //
  // Ask the strand for a buffer containing input data of <reqsize> bytes.
  // The buffer starts at alignment <align>. Some padding bytes may be
  // skipped from the input stream.
  //
  // This function may throw CORBA::MARSHAL if the data left in the GIOP
  // message is less than requested.
  //
  // The request size must be less than the value return by
  // max_input_buffer_size();
  //
  // Pre-condition:
  //
  // Post-condition:
  //   g->pd_inb_mkr and g->pd_inb_end are updated to point to the input
  //   data buffer.

  virtual size_t max_input_buffer_size(giopStream* g,
				       omni::alignment_t align) = 0;
  // Support function for message body processing.
  // The buffer starts at alignment <align>
  //
  // Return the maximum request data size the getInputData is prepared
  // to accept.

  virtual void skipInputData(giopStream* g,size_t size) = 0;
  // Support function for message body processing.
  //
  // Ask the strand to skip <size> bytes from the input stream.
  // fragments.
  //
  // This function may throw CORBA::MARSHAL if the data left in the GIOP
  // message is less than requested.
  //
  // The request size must be less than the value return by
  // max_input_buffer_size();
  //
  // Pre-condition:
  //
  // Post-condition:
  //   g->pd_inb_mkr and g->pd_inb_end are updated accordingly

  virtual void copyInputData(giopStream* g,void* b, size_t size,
			     omni::alignment_t alignment) = 0;
  // Support function for message body processing.
  //
  // Ask the strand to copy input data into the buffer points to by <b>.
  // If GIOP FRAGMENT is supported, the data may straddle two consecutive
  // fragments. The data should be copied from the input stream at the
  // <alignment> specified.
  //
  // This function may throw CORBA::MARSHAL if the data left in the GIOP
  // message is less than requested.
  //
  // Pre-condition:
  //
  // Post-condition:
  //   g->pd_inb_mkr and g->pd_inb_end are updated accordingly.


  virtual size_t outputRemaining(const giopStream* g) const = 0;
  // Support function for message body processing.
  //
  // return no. of bytes that can be written in the output message. 
  // ULONG_MAX means unlimited.

  virtual void getReserveSpace(giopStream* g,
			       omni::alignment_t align,
			       size_t reqsize) = 0;
  // Support function for message body processing.
  //
  // Ask the strand for a buffer of <reqsize> bytes to write data into.
  // The buffer should start at alignment <align>. 
  // The strand must return at least that amount of space and take into
  // account of any padding necessary to meet the alignment requirement.
  //
  // If GIOP FRAGMENT is supported, the implementations may have to
  // flush out the data written so far as a GIOP FRAGMENT before returning
  // a buffer in this call.
  //
  // This function may throw CORBA::MARSHAL if the space left in the GIOP
  // message is less than requested.
  //
  // Pre-condition:
  //
  // Post-condition:
  //   g->pd_outb_mkr and g->pd_outb_end are updated to point to the output
  //   buffer.

  virtual void copyOutputData(giopStream* g,void* b, size_t size,
			      omni::alignment_t alignment) = 0;
  // Support function for message body processing.
  //
  // Ask the strand to copy data from <b> to the output stream. The data
  // should be copied into the output stream at the <alignment> specified.
  //
  // This function may throw CORBA::MARSHAL if the space left in the GIOP
  // message is less than requested.
  //
  // Pre-condition:
  //
  //   <size> >= 7
  //   <size> is a multiple of <alignment>.
  //   For instance, if <alignment> == omni::ALIGN_8, then size % 8 == 0.
  //
  // Post-condition:
  //   g->pd_inb_mkr and g->pd_inb_end are updated accordingly.

  virtual size_t max_reserve_buffer_size(const giopStream* g,
					 omni::alignment_t align) = 0;
  // Support function for message body processing.
  //
  // Return the maximum request data size the getReserveSpace is prepared
  // to accept. The buffer starts at alignment <align>

  virtual CORBA::ULong currentInputPtr(const giopStream* g) const = 0;
  virtual CORBA::ULong currentOutputPtr(const giopStream* g) const = 0;

  static inline int address_alignment(void* ptr)
  // Returns an integer in the range of 1 to omni::max_alignment. This is the 
    // value of the least significant n bits of the pointer where 
    // 2^n = omni::max_alignment. 
    // If the n bits are all 0, return the value of omni::max_alignment.
    {
      int align = (omni::ptr_arith_t)ptr & ((int)omni::max_alignment - 1);
      return ((align) ? align : (int)omni::max_alignment);
    }

  //////////////////////////////////////////////////////////////////////////
  //       Real implementation of the giopStream header processing functions
  //       Same signature and semantics as the giopStream functions with
  //       the same name. See giopStream.h for semantic descriptions.
  //
  virtual _CORBA_Long outputRequestMessageBegin(giopStream* g,
						GIOPObjectInfo* f,
						const char* opname,
						size_t opnamesize,
						CORBA::Boolean oneway,
						CORBA::Boolean response) = 0;
  virtual _CORBA_Long outputLocateMessageBegin(giopStream* g,
					       GIOPObjectInfo* f) = 0;
  virtual void outputReplyMessageBegin(giopStream* g,
				       giopStream::requestInfo& f,
				       GIOP::ReplyStatusType status) = 0;
  virtual void outputLocateReplyMessageBegin(giopStream* g,
					     giopStream::requestInfo& f,
					     GIOP::LocateStatusType status)= 0;
  virtual void outputMessageEnd(giopStream* g) = 0;
  virtual void SendMsgErrorMessage(giopStream* g) = 0;
  virtual GIOP::MsgType inputRequestMessageBegin(giopStream*,
						 giopStream::requestInfo&,
						 CORBA::Boolean) = 0;
  virtual GIOP::ReplyStatusType inputReplyMessageBegin(giopStream*,
						       CORBA::ULong,
						       CORBA::Boolean) = 0;
  virtual GIOP::LocateStatusType inputLocateReplyMessageBegin(giopStream*,
							      CORBA::ULong,
							      CORBA::Boolean) = 0;
  virtual void inputMessageEnd(giopStream* g,
			       _CORBA_Boolean disgard=0,
			       _CORBA_Boolean error=0) = 0;
  virtual _CORBA_Boolean terminalError(const giopStream* g) const = 0;
  virtual void setTerminalError(giopStream* g) = 0;

  //////////////////////////////////////////////////////////////////////////
  //       Real implementation of the giopStream fast copy functions
  //       Same signature and semantics as the giopStream functions with
  //       the same name. See giopStream.h for semantic descriptions.
  //
  virtual CORBA::Boolean startSavingInputMessageBody(giopStream* g) = 0;
  virtual CORBA::Boolean prepareCopyMessageBodyFrom(giopStream* g, giopStream& s) = 0;
  virtual void copyMessageBodyFrom(giopStream*g, giopStream& s) = 0;


protected:
  giopStreamImpl(const GIOP::Version& v);
  virtual ~giopStreamImpl();

  GIOP::Version   pd_version;
  giopStreamImpl* pd_next;
  

private:
  giopStreamImpl();
  giopStreamImpl(const giopStreamImpl&);
  giopStreamImpl& operator=(const giopStreamImpl&);
};

#endif // __GIOPSTREAMIMPL_H__
