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
  Revision 1.1.2.1  2001/02/23 16:47:03  sll
  Added new files.

*/

#ifndef __GIOPSTREAMIMPL_H__
#define __GIOPSTREAMIMPL_H__

OMNI_NAMESPACE_BEGIN(omni)

class giopStream;

class giopStreamImpl {
public:
  
  static void registerImpl(giopStreamImpl*);

  static giopStreamImpl* matchVersion(const GIOP::Version&);
  // Return the singleton that implements the GIOP version specified.

  static giopStreamImpl* maxVersion();
  // Return the singleton that implements the highest GIOP version number

  inline GIOP::Version version() { return pd_version; }


  //////////////////////////////////////////////////////////////////////////
  // Shared by the client and server side
  // Message Header Processing functions
  void (*outputMessageBegin)(giopStream*,
			     void (*marshalHeader)(giopStream*));
  // acquire wrLock
  //
  void (*outputMessageEnd)(giopStream*);
  // release wrLock
  //
  void (*inputMessageBegin)(giopStream*,
			    void (*unmarshalHeader)(giopStream*));
  // make sure rdLock is acquired
  //
  void (*inputMessageEnd)(giopStream*,
			  CORBA::Boolean disgard = 0);
  // release rdlock
  //
  void (*sendMsgErrorMessage)(giopStream*);
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  // Client side
  // Message Header Processing functions
  void (*marshalRequestHeader)(giopStream*);
  void (*marshalLocateRequest)(giopStream*);
  void (*unmarshalReplyHeader)(giopStream*);
  void (*unmarshalLocateReply)(giopStream*);
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  // Server side
  // Message Header Processing functions
  void (*unmarshalWildCardRequestHeader)(giopStream*);
  void (*unmarshalRequestHeader)(giopStream*);
  void (*unmarshalLocateRequest)(giopStream*);
  void (*marshalReplyHeader)(giopStream*);
  void (*sendSystemException)(giopStream*,const CORBA::SystemException&);
  void (*sendUserException)(giopStream*,const CORBA::UserException&);
  void (*sendLocationForwardReply)(giopStream*,CORBA::Object_ptr);
  void (*sendLocateReply)(giopStream*,GIOP::LocateStatusType,
			 CORBA::Object_ptr,CORBA::SystemException* p = 0);
  //////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////
  // Shared by the client and the server side
  // Message Body Processing functions
  size_t (*inputRemaining)(giopStream* g);
    // Support function for message body processing.
    // return no. of bytes remaining in the input message. 

  void (*getInputData)(giopStream* g,
		       omni::alignment_t align,
		       size_t reqsize);
  // Support function for message body processing.
  //
  // On return, the bufer contains at least <reqsize> bytes of input data.
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

  size_t (*max_input_buffer_size)(giopStream* g,
				  omni::alignment_t align);
  // Support function for message body processing.
  // The buffer starts at alignment <align>
  //
  // Return the maximum request data size the getInputData is prepared
  // to accept.

  void (*skipInputData)(giopStream* g,size_t size);
  // Support function for message body processing.
  //
  // Skip pass <size> bytes in the input data buffer.
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

  void (*copyInputData)(giopStream* g,void* b, size_t size,
			omni::alignment_t alignment);
  // Support function for message body processing.
  //
  // Copy input data into the buffer points to by <b>.
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


  size_t (*outputRemaining)(const giopStream* g);
  // Support function for message body processing.
  //
  // return no. of bytes that can be written in the output message. 
  // ULONG_MAX means unlimited.

  void (*getReserveSpace)(giopStream* g,
			  omni::alignment_t align,
			  size_t reqsize);
  // Support function for message body processing.
  //
  // Prepare a buffer of <reqsize> bytes to write data into.
  // The buffer should start at alignment <align>. 
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

  void (*copyOutputData)(giopStream* g,void* b, size_t size,
			 omni::alignment_t alignment);
  // Support function for message body processing.
  //
  // Copy data from <b> to the output stream. The data
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

  size_t (*max_reserve_buffer_size)(const giopStream* g,
				    omni::alignment_t align);
  // Support function for message body processing.
  //
  // Return the maximum request data size the getReserveSpace is prepared
  // to accept. The buffer starts at alignment <align>

  CORBA::ULong (*currentInputPtr)(const giopStream* g);
  CORBA::ULong (*currentOutputPtr)(const giopStream* g);

  giopStreamImpl(const GIOP::Version&);
  ~giopStreamImpl();

private:
  GIOP::Version   pd_version;
  giopStreamImpl* pd_next;

  giopStreamImpl();
  giopStreamImpl(const giopStreamImpl&);
  giopStreamImpl& operator=(const giopStreamImpl&);
};

OMNI_NAMESPACE_END(omni)

#endif // __GIOPSTREAMIMPL_H__
