// -*- Mode: C++; -*-
//                            Package   : omniORB2
// giopStream11.cc            Created on: 11/2/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1999 AT&T Laboratories Cambridge
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
  Revision 1.1.2.3  2000/03/27 17:37:50  sll
  Changed to use the new giopStreamImpl interface.
  Added support to allow call multiplexing on the client side.

  Revision 1.1.2.2  1999/11/04 20:20:20  sll
  GIOP engines can now do callback to the higher layer to calculate total
  message size if necessary.
  Where applicable, changed to use the new server side descriptor-based stub.

  Revision 1.1.2.1  1999/09/15 20:37:28  sll
  *** empty log message ***

*/

#include <omniORB2/CORBA.h>
#include <giopObjectInfo.h>
#include <giopStreamImpl.h>


static GIOP::Version   giop_1_1_version = { 1, 1};
static giop_1_1_Impl*  giop_1_1_singleton = 0;

static const char requestHeader[8] = { 
  'G','I','O','P',1,1,_OMNIORB_HOST_BYTE_ORDER_,GIOP::Request };

static const char replyHeader[8] = {
   'G','I','O','P',1,1,_OMNIORB_HOST_BYTE_ORDER_,GIOP::Reply };

static const char cancelRequestHeader[8] = {
   'G','I','O','P',1,1,_OMNIORB_HOST_BYTE_ORDER_,GIOP::CancelRequest };

static const char locateRequestHeader[8] = {
   'G','I','O','P',1,1,_OMNIORB_HOST_BYTE_ORDER_,GIOP::LocateRequest };

static const char locateReplyHeader[8] = {
   'G','I','O','P',1,1,_OMNIORB_HOST_BYTE_ORDER_,GIOP::LocateReply };

static const char closeConnectionHeader[8] = {
   'G','I','O','P',1,1,_OMNIORB_HOST_BYTE_ORDER_,GIOP::CloseConnection };

static const char messageErrorHeader[8] = {
   'G','I','O','P',1,1,_OMNIORB_HOST_BYTE_ORDER_,GIOP::MessageError };

static const char fragmentHeader[8] = {
   'G','I','O','P',1,1,_OMNIORB_HOST_BYTE_ORDER_,GIOP::Fragment };

#define LOGMESSAGE(level,prefix,message) do {\
   if (omniORB::trace(level)) {\
     omniORB::logger log;\
	log << " giop 1.1 " ## prefix ## ": " message ## "\n";\
   }\
} while (0)

#define PTRACE(prefix,message) LOGMESSAGE(25,prefix,message)

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// giop_1_1_Impl implements the giopStreamImpl interface for GIOP         // 
// revision 1.1 See also the description in giopStreamImpl.h and          //
// giopStream.h                                                           //
////////////////////////////////////////////////////////////////////////////
class giop_1_1_Impl : public giopStreamImpl {

public:

  //////////////////////////////////////////////////////////////////
  giop_1_1_Impl() : giopStreamImpl(giop_1_1_version) {}


  //////////////////////////////////////////////////////////////////
  ~giop_1_1_Impl() {}


private:


  //////////////////////////////////////////////////////////////////
  class marshalRequestHeader : public giopMarshaller {
  public:
    marshalRequestHeader(giopStream* s,GIOPObjectInfo* i,
			 const char* op,size_t opsz,_CORBA_Boolean w,
			 CORBA::ULong req) :
      pd_s(s), pd_i(i), pd_op(op), pd_opsize(opsz), pd_response_expected(w),
      pd_request_id(req) {}

    void marshalData();
    size_t dataSize(size_t initialoffset);

  private:
    giopStream*      pd_s;
    GIOPObjectInfo*  pd_i;
    const char*      pd_op;
    size_t           pd_opsize;
    CORBA::Boolean   pd_response_expected;
    CORBA::ULong     pd_request_id;

    marshalRequestHeader();
    marshalRequestHeader(const marshalRequestHeader&);
    marshalRequestHeader& operator=(const marshalRequestHeader&);
  };

  //////////////////////////////////////////////////////////////////
  class marshalLocateRequestHeader : public giopMarshaller {
  public:
    marshalLocateRequestHeader(giopStream* s,GIOPObjectInfo* i,
			       CORBA::ULong req) :
      pd_s(s), pd_i(i), pd_request_id(req) {}

    void marshalData();
    size_t dataSize(size_t initialoffset);

  private:
    giopStream*     pd_s;
    GIOPObjectInfo* pd_i;
    CORBA::ULong    pd_request_id;

    marshalLocateRequestHeader();
    marshalLocateRequestHeader(const marshalLocateRequestHeader&);
    marshalLocateRequestHeader& operator=(const marshalLocateRequestHeader&);
  };

  //////////////////////////////////////////////////////////////////
  class marshalReplyHeader : public giopMarshaller {
  public:
    marshalReplyHeader(giopStream* s,giopStream::requestInfo& r,
		       GIOP::ReplyStatusType t) :
      pd_s(s), pd_r(r), pd_t(t) {}

    void marshalData();
    size_t dataSize(size_t initialoffset);

  private:
    giopStream*              pd_s;
    giopStream::requestInfo& pd_r;
    GIOP::ReplyStatusType    pd_t;

    marshalReplyHeader();
    marshalReplyHeader(const marshalReplyHeader&);
    marshalReplyHeader& operator=(const marshalReplyHeader&);
  };

  //////////////////////////////////////////////////////////////////
  class marshalLocateReplyHeader : public giopMarshaller {
  public:
    marshalLocateReplyHeader(giopStream* s,giopStream::requestInfo& r,
			     GIOP::LocateStatusType t) :
      pd_s(s), pd_r(r), pd_t(t) {}

    void marshalData();
    size_t dataSize(size_t initialoffset);

  private:
    giopStream*                pd_s;
    giopStream::requestInfo&   pd_r;
    GIOP::LocateStatusType     pd_t;

    marshalLocateReplyHeader();
    marshalLocateReplyHeader(const marshalLocateReplyHeader&);
    marshalLocateReplyHeader& operator=(const marshalLocateReplyHeader&);
  };


public:

   /////////////////////////////////////////////////////////////////
   //
   //          Member functions to output GIOP messages
   //
   /////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////
  CORBA::Long outputRequestMessageBegin(giopStream* g,
					GIOPObjectInfo* f,
					const char* opname,
					size_t opnamesize,
					CORBA::Boolean oneway,
					CORBA::Boolean response_expected)
  {
    // XXX Call interceptor(s) 

    g->pd_output_at_most_once = oneway;

    g->pd_request_id = g->pd_strand->sequenceNumber();
    marshalRequestHeader marshaller(g,f,opname,opnamesize,
				    (oneway?0:response_expected),
				    g->pd_request_id);
    outputMessageBegin(g,requestHeader,&marshaller);
    
    return g->pd_request_id;
  }

  //////////////////////////////////////////////////////////////////
  CORBA::Long outputLocateMessageBegin(giopStream* g, GIOPObjectInfo* f)
  {
    g->pd_output_at_most_once = 0;

    g->pd_request_id = g->pd_strand->sequenceNumber();
    marshalLocateRequestHeader marshaller(g,f,g->pd_request_id);
    outputMessageBegin(g,locateRequestHeader,&marshaller);

    return g->pd_request_id;
  }

  //////////////////////////////////////////////////////////////////
  void outputReplyMessageBegin(giopStream* g,
			       giopStream::requestInfo& f,
			       GIOP::ReplyStatusType status)
  {
    // XXX Call interceptor(s) 

    g->pd_output_at_most_once = 0;

    f.requestID(g->pd_request_id);

    marshalReplyHeader marshaller(g,f,status);
    outputMessageBegin(g,replyHeader,&marshaller);
  }

  //////////////////////////////////////////////////////////////////
  void outputLocateReplyMessageBegin(giopStream* g,
				     giopStream::requestInfo& f,
				     GIOP::LocateStatusType status)
  {
    g->pd_output_at_most_once = 0;

    f.requestID(g->pd_request_id);
    marshalLocateReplyHeader marshaller(g,f,status);
    outputMessageBegin(g,locateReplyHeader,&marshaller);
  }

  //////////////////////////////////////////////////////////////////
  void outputMessageEnd(giopStream* g)
  {
    if (g->pd_output_msgfrag_size) {
      // This means that the header of the fragment has already gone out.
      // It has the fragment bit set to 1.
      // We flush what is left in the buffer and starts a new fragment.

      g->pd_strand->giveback_reserved((omni::ptr_arith_t)g->pd_outb_end -
				      (omni::ptr_arith_t)g->pd_outb_mkr,1,1);
      g->pd_outb_begin = g->pd_outb_end = g->pd_outb_mkr;

      newFragment(g,fragmentHeader);
    }

    endFragment(g,0,1);
  }

private:

  //////////////////////////////////////////////////////////////////
  void outputMessageBegin(giopStream* g,const char* hdr,
			  giopMarshaller* marshalhdr)
  {
    newFragment(g,hdr);

    g->pd_output_msgsent_size = 0;

    g->pd_output_header_marshaller = marshalhdr;

    if (marshalhdr) marshalhdr->marshalData();

    if (g->pd_output_msgfrag_size) {
      // This can only mean that the header is too large to fit into the
      // maximum buffer space provided by the strand.
      // Part of the header have already been flushed to the network and part
      // of it is still in the strand's buffer.
      // We flush what is left in the buffer and starts a new fragment
      // before we marshal the request body.
      size_t leftover = (omni::ptr_arith_t)g->pd_outb_end - 
	                (omni::ptr_arith_t)g->pd_outb_mkr;
      g->pd_strand->giveback_reserved(leftover,1,1);
      g->pd_outb_begin = g->pd_outb_end = g->pd_outb_mkr;

      newFragment(g,fragmentHeader);
    }

    g->pd_output_header_marshaller = 0;
  }

public:

  //////////////////////////////////////////////////////////////////
  void getReserveSpace(giopStream* g,omni::alignment_t align, size_t reqsize)
  {
    if (omni::align_to((omni::ptr_arith_t)g->pd_outb_mkr,align)+
	(omni::ptr_arith_t)reqsize <= (omni::ptr_arith_t) g->pd_outb_end)
      return;

    g->pd_output_msgsent_size += (omni::ptr_arith_t)g->pd_outb_mkr -
      (omni::ptr_arith_t)g->pd_outb_begin;

    if (g->pd_output_msgfrag_size == 0) {

      g->pd_output_msgsent_size -= 12; // subtract the header

      if (g->pd_output_header_marshaller) {
	// We are given a callback object to work out the size of the
	// message. We call this method to determine the size.
	g->pd_output_msgfrag_size  = g->pd_output_header_marshaller->dataSize(12);
      }
      endFragment(g,g->pd_output_msgfrag_size,0);
    }
    else {
      // This fragment is actually larger than can be accomodated by the
      // maximum buffer space returned by the strand. The header of the
      // fragment has already gone out to the network. No need to write
      // the fragment header.

      g->pd_strand->giveback_reserved((omni::ptr_arith_t)g->pd_outb_end -
				      (omni::ptr_arith_t)g->pd_outb_mkr,0,0);
      g->pd_outb_begin = g->pd_outb_end = g->pd_outb_mkr;
    }

    if (g->pd_output_msgfrag_size == 0) {
      newFragment(g,fragmentHeader);
    }
    else {
      int align = giopStreamImpl::address_alignment(g->pd_outb_mkr);
      size_t sz = g->pd_strand->max_reserve_buffer_size();
      if (sz > outputRemaining(g)) sz = outputRemaining(g);

      Strand::sbuf b = g->pd_strand->reserve(sz,1,align);
      g->pd_outb_begin = g->pd_outb_mkr = b.buffer;
      g->pd_outb_end = (char*)g->pd_outb_begin + b.size;
    }

    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)g->pd_outb_mkr,
					  align);
    omni::ptr_arith_t p2 = p1 + reqsize;
    if( (void*)p2 > g->pd_outb_end ) {
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }
  }

  //////////////////////////////////////////////////////////////////
  void copyOutputData(giopStream* g,void* buf, size_t size,
		      omni::alignment_t alignment)
  {
    int align = giopStreamImpl::address_alignment(g->pd_outb_mkr);
    size_t padding = omni::align_to((omni::ptr_arith_t)align, 
				    alignment) - align;

    size_t avail = (omni::ptr_arith_t)g->pd_outb_end -
      (omni::ptr_arith_t)g->pd_outb_mkr;

    if (padding && avail >= padding) {
      g->pd_outb_mkr = (void*)((omni::ptr_arith_t)g->pd_outb_mkr + padding);
      avail -= padding;
      padding = 0;
    }

    g->pd_output_msgsent_size += (omni::ptr_arith_t)g->pd_outb_mkr -
      (omni::ptr_arith_t)g->pd_outb_begin;

    if (g->pd_output_msgfrag_size == 0) {

      g->pd_output_msgsent_size -= 12;

      // Work out the fragment size and write the value to the header
      CORBA::ULong fragsz = (omni::ptr_arith_t)g->pd_outb_mkr -
	(omni::ptr_arith_t)g->pd_outb_begin - 12
	+ padding + size;

      if (g->pd_output_header_marshaller) {
	// Now we are given a callback object to work out the size of the
	// message. We call this method to determine the size.
	g->pd_output_msgfrag_size = fragsz = g->pd_output_header_marshaller->dataSize(12);
      }

      endFragment(g,fragsz,0);

    }
    else {
      g->pd_strand->giveback_reserved(avail);
      g->pd_outb_begin = g->pd_outb_end = g->pd_outb_mkr;
    }
    

    if (outputRemaining(g) >= (padding + size)) {
      g->pd_output_msgsent_size += (padding + size);
    }
    else
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);

    Strand* s = g->pd_strand;
    Strand::sbuf b;
    b.buffer = (void*) buf;
    b.size = size;

    if (padding) {
      s->reserve(padding,1,align);
    }

    if (g->pd_output_msgfrag_size == 0) {

      s->reserve_and_copy(b,1,1);

      // create a new fragment
      newFragment(g,fragmentHeader);
    }
    else {

      s->reserve_and_copy(b);

      g->pd_outb_mkr = (void*)((omni::ptr_arith_t)g->pd_outb_mkr + 
			       padding + size);
      align = giopStreamImpl::address_alignment(g->pd_outb_mkr);

      // the upcoming data is part of the fragment which we have just sent
      b = s->reserve(s->max_reserve_buffer_size(),1,align);
      g->pd_outb_begin = g->pd_outb_mkr = b.buffer;
      g->pd_outb_end = (char*)g->pd_outb_begin + b.size;
    }
 
  }

  //////////////////////////////////////////////////////////////////
  CORBA::ULong currentOutputPtr(const giopStream* g) const
  {
    return (GIOP_Basetypes::max_giop_message_size - outputRemaining(g));
  }

private:

  //////////////////////////////////////////////////////////////////
  void newFragment(giopStream* g,const char* hdr)
  {
    Strand::sbuf b;
    Strand* s = g->pd_strand;
    size_t sz = outputRemaining(g) + 12;
    if (sz > s->max_reserve_buffer_size()) sz = s->max_reserve_buffer_size();

    b = s->reserve_and_startMTU(sz,1,omni::max_alignment,
				0,g->pd_output_at_most_once);
    g->pd_outb_begin = b.buffer;
    g->pd_outb_end = (char*)g->pd_outb_begin + b.size;
    g->pd_output_msgfrag_size = 0;

    memcpy(g->pd_outb_begin,(void*)hdr,8);
    if (g->pd_marshal_byte_swap) {
      // toggle the byte endian flag
      *((char*)g->pd_outb_begin + 6) = _OMNIORB_HOST_BYTE_ORDER_ ^ 1;
    }
    g->pd_outb_mkr = (char*)g->pd_outb_begin + 12;
  }

  //////////////////////////////////////////////////////////////////
  void endFragment(giopStream* g, CORBA::ULong fragsz,CORBA::Boolean last)
  {
    CORBA::Boolean endMTU = 0;

    if (!fragsz) {
      fragsz = (omni::ptr_arith_t)g->pd_outb_mkr -
	(omni::ptr_arith_t)g->pd_outb_begin - 12;
      endMTU = 1;
    }

    CORBA::ULong* hdr = (CORBA::ULong*)((char*)g->pd_outb_begin + 8);
    if (!g->pd_marshal_byte_swap) {
      *hdr = fragsz;
    }
    else {
      *hdr = ((((fragsz) & 0xff000000) >> 24) |
	      (((fragsz) & 0x00ff0000) >> 8)  |
	      (((fragsz) & 0x0000ff00) << 8)  |
	      (((fragsz) & 0x000000ff) << 24));
    }

    if (!last) {
      // Set fragment bit to true to indicate there are more fragments to come
      char* fragbitp = (char*)g->pd_outb_begin + 6;
      *fragbitp |= 0x2;
    }

    g->pd_strand->giveback_reserved((omni::ptr_arith_t)g->pd_outb_end -
				    (omni::ptr_arith_t)g->pd_outb_mkr,1,endMTU);
    g->pd_outb_begin = g->pd_outb_end = g->pd_outb_mkr;
    // Invariant: the alignment info in g->pd_outb_mkr must be preserved.
  }

public:

  //////////////////////////////////////////////////////////////////
  void SendMsgErrorMessage(giopStream* g)
  {
    Strand::sbuf b;

    b = g->pd_strand->reserve_and_startMTU(12,1,omni::max_alignment,0);
    memcpy(b.buffer,(void*)messageErrorHeader,8);
    CORBA::ULong* hdr = (CORBA::ULong*)((char*)b.buffer + 8);
    *hdr = 0;
    g->pd_strand->giveback_reserved(0,1,1);
    g->pd_outb_begin = g->pd_outb_end = g->pd_outb_mkr = 0;
  }

  //////////////////////////////////////////////////////////////////
  size_t outputRemaining(const giopStream* g) const
  {
    return (GIOP_Basetypes::max_giop_message_size - 
	    g->pd_output_msgsent_size -
	    ((omni::ptr_arith_t)g->pd_outb_mkr -
	     (omni::ptr_arith_t)g->pd_outb_begin));
  }

  //////////////////////////////////////////////////////////////////
  size_t max_reserve_buffer_size(const giopStream* g,omni::alignment_t align)
  {
    // maximum space that can be provided by the strand minus the GIOP header
    // size. Unless the limit on the overall GIOP message limit has been 
    // exceeded, this value is what getReserveSpace can support.
    size_t overhead = omni::align_to((omni::ptr_arith_t)12,align);
    return (g->pd_strand->max_reserve_buffer_size() - overhead);
  }


  ///////////////////////////////////////////////////////////////////
  //
  //          Member functions to input GIOP messages
  //
  ///////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////
  size_t inputRemaining(giopStream* g)
  {
    if (!g->pd_input_fragmented) {
      return ((omni::ptr_arith_t)g->pd_inb_end - 
	      (omni::ptr_arith_t)g->pd_inb_mkr)
	+ g->pd_input_msgfrag_to_come;
    }
    else {
      return GIOP_Basetypes::max_giop_message_size - 
             g->pd_input_msgbody_received -
	     ((omni::ptr_arith_t)g->pd_inb_mkr - 
	      (omni::ptr_arith_t)g->pd_inb_begin);
    }
  }


  //////////////////////////////////////////////////////////////////
  void getInputData(giopStream* g,omni::alignment_t align, size_t reqsize)
  {
    size_t padding = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,align) -
      (omni::ptr_arith_t)g->pd_inb_mkr;

    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
      (omni::ptr_arith_t) g->pd_inb_mkr;

    if (avail >= (padding+reqsize)) return;

    if (!g->pd_input_msgfrag_to_come && !g->pd_input_fragmented) {
      // This is already the last fragment.
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }

    g->pd_strand->giveback_received(avail);
    g->pd_input_msgfrag_to_come += avail;
    g->pd_inb_end = g->pd_inb_mkr;
    g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
				     (omni::ptr_arith_t)g->pd_inb_begin);

  again:
    if (g->pd_input_msgfrag_to_come) {

      if (g->pd_input_msgfrag_to_come < (reqsize+padding) ||
	  g->pd_strand->max_receive_buffer_size() < (reqsize+padding)) {

	// The request size is too large. 
	g->setTerminalError();
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }

      Strand::sbuf b;
      b = g->pd_strand->receive(g->pd_strand->max_receive_buffer_size(),0,
				giopStreamImpl::address_alignment(g->pd_inb_mkr));
      if (b.size < (reqsize+padding)) {
	g->pd_strand->giveback_received(b.size);
	b = g->pd_strand->receive(reqsize+padding,1,
				  giopStreamImpl::address_alignment(g->pd_inb_mkr));
      }
      g->pd_inb_begin = g->pd_inb_mkr = b.buffer;
      g->pd_inb_end = (void*) ((omni::ptr_arith_t)g->pd_inb_mkr + b.size);
      g->pd_input_msgfrag_to_come -= b.size;
    }
    else if (g->pd_input_fragmented) {

      GIOP::MsgType t = inputFragmentBegin(g,1,0,1);
      padding = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,align) -
	(omni::ptr_arith_t)g->pd_inb_mkr;
      goto again;
    }
    else {
      // This is already the last fragment.
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }
  }

  //////////////////////////////////////////////////////////////////
  size_t max_input_buffer_size(giopStream* g, omni::alignment_t align)
  {
  again:
    size_t avail = ((omni::ptr_arith_t) g->pd_inb_end - (omni::ptr_arith_t) 
		    g->pd_inb_mkr) + g->pd_input_msgfrag_to_come;

    if (!avail && g->pd_input_fragmented) {

      g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
				       (omni::ptr_arith_t)g->pd_inb_begin);
      (void) inputFragmentBegin(g,1,0,1);
      goto again;
    }
    size_t padding = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,align) -
      (omni::ptr_arith_t)g->pd_inb_mkr;

    if (avail < padding) {
      // Error. Taking into acount of the alignment requirement, the padding
      // take us to beyond the end of the fragment. This is most likely to
      // be a problem with the incoming stream. We return 0 bytes here.
      avail = 0;
    }
    else {
      avail -= padding;
    }

    size_t rz = g->pd_strand->max_receive_buffer_size() - padding;

    if (rz > avail)
      return avail;
    else
      return rz;
  }

  //////////////////////////////////////////////////////////////////
  void skipInputData(giopStream* g,size_t size)
  {
    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
      (omni::ptr_arith_t) g->pd_inb_mkr;
    g->pd_strand->giveback_received(avail);
    g->pd_input_msgfrag_to_come += avail;
    g->pd_inb_end = g->pd_inb_mkr;
    g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
				     (omni::ptr_arith_t)g->pd_inb_begin);

  again:
    if (!g->pd_input_msgfrag_to_come && g->pd_input_fragmented) {

      (void) inputFragmentBegin(g,1,0,1);
      goto again;
    }

    if (size > g->pd_input_msgfrag_to_come) {
      g->SendMsgErrorMessage();
      g->setTerminalError();
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }

    g->pd_strand->skip(size);

    g->pd_input_msgfrag_to_come -= size;

    g->pd_inb_end = g->pd_inb_mkr = (void*)((omni::ptr_arith_t) 
					    g->pd_inb_mkr + size);
    g->pd_input_msgbody_received += size;
  }

  //////////////////////////////////////////////////////////////////
  void copyInputData(giopStream* g,void* b, size_t size,
		     omni::alignment_t alignment)
  {
    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
      (omni::ptr_arith_t) g->pd_inb_mkr;
    g->pd_strand->giveback_received(avail);
    g->pd_input_msgfrag_to_come += avail;
    g->pd_inb_end = g->pd_inb_mkr;
    g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
				     (omni::ptr_arith_t)g->pd_inb_begin);

    char* bp = (char*)b;

    while (size) {
    
      if (g->pd_input_msgfrag_to_come) {

	omni::ptr_arith_t p = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,
					     alignment);

	size_t padding = p - (omni::ptr_arith_t) g->pd_inb_mkr; 

	if (padding) {

	  if (padding > g->pd_input_msgfrag_to_come) {
	    // Protocol violation
	    g->SendMsgErrorMessage();
	    g->setTerminalError();
	    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
	  }
	  g->pd_input_msgfrag_to_come -= padding;
	  g->pd_strand->skip(padding);

	  g->pd_inb_mkr = g->pd_inb_end = (void*) p;
	  g->pd_input_msgbody_received += padding;
	}
      
	size_t reqsize = size;
	if (reqsize > g->pd_input_msgfrag_to_come) {

	  reqsize = g->pd_input_msgfrag_to_come;

	  if (reqsize % (int)alignment) {
	    // Protocol violation. The data should be integral multiple 
	    g->SendMsgErrorMessage();
	    g->setTerminalError();
	    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
	  }
	}

	Strand::sbuf bf;
	bf.buffer = bp;
	bf.size = reqsize;
	g->pd_strand->receive_and_copy(bf);

	g->pd_input_msgfrag_to_come -= reqsize;
	g->pd_input_msgbody_received += reqsize;
	size -= reqsize;
	bp += reqsize;
      }
      else if (g->pd_input_fragmented) {

	GIOP::MsgType t = inputFragmentBegin(g,1,0,1);
	continue;
      }
      else {
	// This is already the last fragment.
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }

    }
  }

  //////////////////////////////////////////////////////////////////
  CORBA::ULong currentInputPtr(const giopStream* g) const
  {
    return (g->pd_input_msgbody_received + 
	    ((omni::ptr_arith_t) g->pd_inb_mkr - 
	     (omni::ptr_arith_t) g->pd_inb_begin));
  }

private:

  //////////////////////////////////////////////////////////////////
  // Post-condition:
  //    If expectFragment == TRUE, then on return g->pd_inb_mkr has moved
  //    beyond the fragment header.
  //    Otherwise, the g->pd_inb_mkr stays pointing to the beginning of
  //    the fragment.
  GIOP::MsgType inputFragmentBegin(giopStream* g,CORBA::Boolean giop11only,
				   CORBA::Boolean headerInBuffer,
				   CORBA::Boolean expectFragment)
  {
    if (!headerInBuffer) {
      assert(g->pd_inb_end == g->pd_inb_mkr);

      Strand::sbuf b;
      b = g->pd_strand->receive(12,1,omni::max_alignment,1);
      g->pd_inb_begin = g->pd_inb_mkr = b.buffer;
      g->pd_inb_end = (void *) ((omni::ptr_arith_t)g->pd_inb_mkr + b.size);
    }

    unsigned char* hdr = (unsigned char*) g->pd_inb_mkr;
    if (hdr[0] != 'G' || hdr[1] != 'I' || hdr[2] != 'O' || hdr[3] != 'P' ||
	hdr[4] != 1   || hdr[5] > 1 || (hdr[5] == 0 && giop11only))
      {
	goto bail_out;
      }
  
    if (hdr[5] >= 1) {
      g->pd_input_fragmented = ((hdr[6] & 0x2) ? 1 : 0);
    }

    {
      CORBA::Boolean bswap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_)
			      ? 0 : 1 );
      if (!expectFragment) {
	g->pd_unmarshal_byte_swap = bswap;
      }
      else {
	// The byte order of the message is set in the first fragment.
	// The byte order of this fragment must match that of the first
	// fragment.
	if (g->pd_unmarshal_byte_swap != bswap)
	  goto bail_out;
	g->pd_inb_mkr = (void*)((omni::ptr_arith_t)g->pd_inb_mkr + 12);
      }
    }

    g->pd_input_msgfrag_to_come = *(CORBA::ULong*)(hdr + 8);

    if (g->pd_unmarshal_byte_swap) {
      CORBA::ULong fragsz = g->pd_input_msgfrag_to_come;
      g->pd_input_msgfrag_to_come = ((((fragsz) & 0xff000000) >> 24) |
				     (((fragsz) & 0x00ff0000) >> 8)  |
				     (((fragsz) & 0x0000ff00) << 8)  |
				     (((fragsz) & 0x000000ff) << 24));
    }

    if (hdr[7] > (unsigned char) GIOP::Fragment) {

      goto bail_out;
    }

    if (expectFragment && hdr[7] != (unsigned char) GIOP::Fragment) {
      if (hdr[7] == (unsigned char) GIOP::CancelRequest &&
	  g->pd_state == giopStream::InputRequest) {
	// We are in the process of unmarshalling an incoming request &
	// there are still more fragments to come. If the request id
	// in this cancelrequest message match the current request id,
	// we terminate the processing of the current request.
	Strand::sbuf b;
	b = g->pd_strand->receive(4,1,omni::ALIGN_4);
	CORBA::ULong reqid = *(CORBA::ULong*)b.buffer;
	if (g->pd_unmarshal_byte_swap) {
	  CORBA::ULong t = reqid;
	  reqid = ((((t) & 0xff000000) >> 24) |
		   (((t) & 0x00ff0000) >> 8)  |
		   (((t) & 0x0000ff00) << 8)  |
		   (((t) & 0x000000ff) << 24));
	}
	if (reqid == g->pd_request_id) {
	  g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_inb_end + 4);
	  g->pd_inb_mkr = g->pd_inb_end;
	  // bingo! We throw terminateProcessing to cancel the processing
	  // of this request.
	  throw GIOP_S::terminateProcessing();
	}
      }
      else {
	goto bail_out;
      }
    }

    if ((g->pd_input_msgbody_received + g->pd_input_msgfrag_to_come) > 
        GIOP_Basetypes::max_giop_message_size) {

      // We may choose not to send MessageError but  instead just
      // throw MARSHAL exception.
      g->SendMsgErrorMessage();
      g->setTerminalError();
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }

    return (GIOP::MsgType) hdr[7];

  bail_out:
    if (hdr[7] != (unsigned char) GIOP::MessageError &&
	hdr[7] != (unsigned char) GIOP::CloseConnection) {
      g->SendMsgErrorMessage();
    }
    g->setTerminalError();
    g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
    return (GIOP::MsgType) 0; // Dummy return
  }

  //////////////////////////////////////////////////////////////////
  void transferInputContext(giopStream* g, giopStream* dest)
  {
    // NOTE: this function transfer the state setup by inputFragmentBegin()
    //       Any state changes made in inputFragmentBegin() must be transfer
    //       here.
    size_t leftover = (omni::ptr_arith_t)g->pd_inb_end - 
                      (omni::ptr_arith_t)g->pd_inb_mkr;
    if (leftover) {
      g->pd_strand->giveback_received(leftover);
      g->pd_input_msgfrag_to_come += leftover;
      g->pd_inb_end = g->pd_inb_mkr;
    }
    dest->pd_inb_mkr = g->pd_inb_mkr;
    dest->pd_inb_end = g->pd_inb_end;
    dest->pd_input_fragmented = g->pd_input_fragmented;
    dest->pd_unmarshal_byte_swap = g->pd_unmarshal_byte_swap;
    dest->pd_input_msgfrag_to_come = g->pd_input_msgfrag_to_come;
    dest->pd_input_msgbody_received = g->pd_input_msgbody_received;
  }

public:
  //////////////////////////////////////////////////////////////////
  GIOP::MsgType inputRequestMessageBegin(giopStream* g,
					 giopStream::requestInfo& r,
					 CORBA::Boolean headerInBuffer)
  {
    g->pd_input_msgbody_received = 0;

    GIOP::MsgType t = inputFragmentBegin(g,0,headerInBuffer,0);
  
    unsigned char* hdr = (unsigned char*) g->pd_inb_mkr;

    if (hdr[5] == 0) {
      // This is a GIOP 1.0 message, switch to the implementation of giop 1.0
      // and dispatch again.
      GIOP::Version v = { 1, 0 };
      g->pd_impl = giopStreamImpl::matchVersion(v);
      assert(g->pd_impl);
      return g->pd_impl->inputRequestMessageBegin(g,r,1);
    }

    g->pd_inb_mkr = (void*) ((omni::ptr_arith_t)g->pd_inb_mkr + 12);

    switch (t) {
    case GIOP::Request:
      {
	unmarshalRequestHeader(g,r);
	// XXX Call interceptor(s);
	break;
      }
    case GIOP::LocateRequest:
      {
	unmarshalLocateRequestHeader(g,r);
	break;
      }
    case GIOP::CancelRequest:
    case GIOP::CloseConnection:
      break;
    case GIOP::Reply:
    case GIOP::LocateReply:
    case GIOP::Fragment:
      {
	g->SendMsgErrorMessage();
	g->setTerminalError();
	g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
	// never reach here.
      }
    case GIOP::MessageError:
      {
	g->setTerminalError();
	g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
	// never reach here.
      }
    }
    return t;
  }

  //////////////////////////////////////////////////////////////////
  CORBA::Boolean inputReplyMessageBegin(giopStream* g)
  {
    CORBA::Boolean rc = inputAnyReplyBegin(g,GIOP::Reply);
    // XXX if (rc) Call interceptor(s);
    return rc;
  }

  //////////////////////////////////////////////////////////////////
  CORBA::Boolean inputLocateReplyMessageBegin(giopStream* g)
				    
  {
    return inputAnyReplyBegin(g,GIOP::LocateReply);
  }

private:
  CORBA::Boolean inputAnyReplyBegin(giopStream* g, GIOP::MsgType expect)
  {
    g->pd_input_msgbody_received = 0;

    GIOP::MsgType t = inputFragmentBegin(g,1,0,0);

    g->pd_inb_mkr = (void*) ((omni::ptr_arith_t)g->pd_inb_mkr + 12);

    CORBA::ULong replyid;

    if (t == GIOP::Reply) {
      unmarshalReplyHeader(g,replyid);
    }
    else if (t == GIOP::LocateReply) {
      unmarshalLocateReplyHeader(g,replyid);
    }
    else {
      g->setTerminalError();
      g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
      // never reach here.
    }

    if (g->pd_request_id == replyid) {
      if (t != expect) goto message_error;
    }
    else {
      // request ID mismatch.
      PTRACE("inputAnyReplyBegin","transfer reply message");

      omni_mutex_lock sync(g->getSyncMutex());
      giopStream* p = giopStream::findOnly(g->pd_strand,replyid);
      if (!p) {
	// This cannot happen because we do not issue Cancel Message
	goto message_error;
      }
      else {	
	transferInputContext(g,p);
	if (t == GIOP::Reply) {
	  transferReplyHeader(g,p);
	}
	else {
	  transferLocateReplyHeader(g,p);
	}
	g->transferReplyState(p);
	return 0;
      }
    }
    return 1;

  message_error:
    g->SendMsgErrorMessage();
    g->setTerminalError();
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  }

public:
  //////////////////////////////////////////////////////////////////
  void inputMessageEnd(giopStream* g, CORBA::Boolean disgard,
		       CORBA::Boolean error)
  {
    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
      (omni::ptr_arith_t) g->pd_inb_mkr;
    if (avail) {
      g->pd_strand->giveback_received(avail);
      g->pd_input_msgfrag_to_come += avail;
      g->pd_inb_end = g->pd_inb_mkr;
      g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
				       (omni::ptr_arith_t)g->pd_inb_begin);
    }

    while (!g->pd_input_msgfrag_to_come && g->pd_input_fragmented && !error) {
      // There are more fragments to come. We fully expect the fragments to
      // come are empty. If not it is an error. We iterate here to read
      // all the empty fragments and stop either the last one is read or
      // a non-empty fragment is read.
      (void) inputFragmentBegin(g,1,0,1);
    }

    if (error || (g->pd_input_fragmented && ! disgard)) {
      // Either the caller have identified a comm error or
      // There are more fragments to come and the message does not
      // end here.
      SendMsgErrorMessage(g);
      g->setTerminalError();
      g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
      // never reach here.
    }
    
    if (!disgard && g->pd_input_msgfrag_to_come) {
      // XXX worry about straight conformance here.
      if (omniORB::traceLevel >= 15) {
	omniORB::log << "giop_1_1_Impl::inputMessageEnd: garbage left at the end of message.\n";
	omniORB::log.flush();
      }
      if (!omniORB::strictIIOP) {
	g->pd_strand->skip(g->pd_input_msgfrag_to_come);
	g->pd_input_msgfrag_to_come = 0;
      }
      else {
	SendMsgErrorMessage(g);
	g->setTerminalError();
	g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
	// never reach here.
      }

    }
    else {
      while (g->pd_input_fragmented || g->pd_input_msgfrag_to_come) {
	if (g->pd_input_msgfrag_to_come) {
	  g->pd_strand->skip(g->pd_input_msgfrag_to_come);
	  g->pd_input_msgfrag_to_come = 0;
	}
	else {

	  GIOP::MsgType t = inputFragmentBegin(g,1,0,1);
	}
      }
    }
  }

  ///////////////////////////////////////////////////////////////////
  //
  //          Member functions to perform fast copy
  //
  ///////////////////////////////////////////////////////////////////
  CORBA::Boolean startSavingInputMessageBody(giopStream* g)
  {
    return 0;
  }

  CORBA::Boolean prepareCopyMessageBodyFrom(giopStream* g, giopStream& s)
  {
    return 0;
  }

  void copyMessageBodyFrom(giopStream*g, giopStream& s)
  {
    throw omniORB::fatalException(__FILE__,__LINE__,
	  "giop_1_1_Impl::copyMessageBodyFrom() should not be called.");
  }


private:
   /////////////////////////////////////////////////////////////////
   //
   //       Marshalling functions for message headers   
   //
   /////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////
  void unmarshalRequestHeader(giopStream* g,giopStream::requestInfo& r)
  {
    cdrStream& s = *g;

    // Service context
    CORBA::ULong svcccount;
    CORBA::ULong svcctag;
    CORBA::ULong svcctxtsize;
    svcccount <<= s;
    while (svcccount-- > 0) {
      svcctag <<= s;
      svcctxtsize <<= s;
      s.skipInput(svcctxtsize);
    }

    CORBA::ULong   vl;
    CORBA::Boolean vb;

    // request id
    vl <<= s;
    r.requestID(vl);
    g->pd_request_id = vl;

    // response expected
    vb <<= s;
    r.response_expected(vb);
    r.result_expected(1);

    // skip 3 bytes reserved. 

    // object key
    vl <<= s;
    if (!s.checkInputOverrun(1,vl)) {
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
    r.keysize(vl);
    s.get_char_array((CORBA::Char*)r.key(),vl);

    // operation
    vl <<= s;
    if (!vl || !s.checkInputOverrun(1,vl)) {
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
    r.set_operation_size(vl);
    char* op = r.operation();
    s.get_char_array((CORBA::Char*)op,vl);
    op[vl-1] = '\0';

    // principal
    vl <<= s;
    if (!s.checkInputOverrun(1,vl)) {
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
    r.set_principal_size(vl);
    s.get_char_array((CORBA::Char*)r.principal(), vl);
  }

  //////////////////////////////////////////////////////////////////
  void unmarshalLocateRequestHeader(giopStream* g,giopStream::requestInfo& r)
  {
    cdrStream& s = *g;

    CORBA::ULong vl;

    // request ID
    vl <<= s;
    r.requestID(vl);
    g->pd_request_id = vl;

    // object key
    vl <<= s;
    if (!s.checkInputOverrun(1,vl)) {
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
    r.keysize(vl);
    s.get_char_array((CORBA::Char*)r.key(),vl);
  }

  //////////////////////////////////////////////////////////////////
  void unmarshalReplyHeader(giopStream* g,CORBA::ULong& reqid)
  {
    // Note: Any data extracted by this function should be transferred by
    //       transferReplyHeader()

    cdrStream& s = *g;

    // Service context
    CORBA::ULong svcccount;
    CORBA::ULong svcctag;
    CORBA::ULong svcctxtsize;
    svcccount <<= s;
    while (svcccount-- > 0) {
      svcctag <<= s;
      svcctxtsize <<= s;
      s.skipInput(svcctxtsize);
    }

    reqid <<= s;
  
    CORBA::ULong v;
    v <<= s;

    switch (v) {
    case GIOP::SYSTEM_EXCEPTION:
    case GIOP::NO_EXCEPTION:
    case GIOP::USER_EXCEPTION:
    case GIOP::LOCATION_FORWARD:
      break;
    default:
      // Should never receive anything other that the above
      // Same treatment as wrong header
      g->setTerminalError();
      g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
      // never reach here.
    }
    g->pd_reply_status.replyStatus = (GIOP::ReplyStatusType)v;
  }


  //////////////////////////////////////////////////////////////////
  void transferReplyHeader(giopStream* g,giopStream* dest)
  {
    // Note: this function transfer the data extracted by unmarshalReplyHeader.
    //       Any changes to unmarshalReplyHeader should be accompanied by
    //       corresponding changes here.

    // XXX Transfer the service context as well.
    dest->pd_reply_status.replyStatus = g->pd_reply_status.replyStatus;
  }

  //////////////////////////////////////////////////////////////////
  void unmarshalLocateReplyHeader(giopStream* g,CORBA::ULong& reqid)
  {
    cdrStream& s = *g;

    reqid <<= s;

    CORBA::ULong v;
    v <<= s;

    switch (v) {
    case GIOP::UNKNOWN_OBJECT:
    case GIOP::OBJECT_HERE:
    case GIOP::OBJECT_FORWARD:
      break;
    default:
      // Should never receive anything other that the above
      // Same treatment as wrong header
      g->setTerminalError();
      g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
      // never reach here.
    }
    g->pd_reply_status.locateReplyStatus = (GIOP::LocateStatusType)v;
  }

  //////////////////////////////////////////////////////////////////
  void transferLocateReplyHeader(giopStream* g,giopStream* dest)
  {
    // Note: this function transfer the data extracted by 
    //       unmarshalLocateReplyHeader. Any changes to 
    //       unmarshalLocateReplyHeader should be accompanied by
    //       corresponding changes here.

    dest->pd_reply_status.locateReplyStatus = g->pd_reply_status.locateReplyStatus;
  }

  giop_1_1_Impl(const giop_1_1_Impl&);
  giop_1_1_Impl& operator=(const giop_1_1_Impl&);

  ////////////////////////////////////////////////////////////////
  //        End of class giop_1_1_Impl.
};


//////////////////////////////////////////////////////////////////
void giop_1_1_Impl::marshalRequestHeader::marshalData()
{
  cdrStream& s = *pd_s;

    // Service context
  ::operator>>=((CORBA::ULong)0,s);

  pd_request_id >>= s;

  pd_response_expected >>= s;

  // padding
  CORBA::Octet v = 0;
  v >>= s;
  v >>= s;
  v >>= s;

  // object key
  pd_i->keysize() >>= s;
  s.put_char_array((CORBA::Char*) pd_i->key(), pd_i->keysize());

    // operation
  operator>>= ((CORBA::ULong) pd_opsize, s);
  s.put_char_array((CORBA::Char*) pd_op, pd_opsize);

    // principal
  operator>>= ((CORBA::ULong)0,s);
}

//////////////////////////////////////////////////////////////////
size_t giop_1_1_Impl::marshalRequestHeader::dataSize(size_t initialoffset)
{
  cdrCountingStream s(initialoffset);

  // Service context
  ::operator>>=((CORBA::ULong)0,s);

  pd_request_id >>= s;

  pd_response_expected >>= s;

  // object key
  pd_i->keysize() >>= s;
  s.put_char_array((CORBA::Char*) pd_i->key(), pd_i->keysize());

  // operation
  operator>>= ((CORBA::ULong) pd_opsize, s);
  s.put_char_array((CORBA::Char*) pd_op, pd_opsize);

  // principal
  operator>>= ((CORBA::ULong)0,s);

  return s.total();
}

//////////////////////////////////////////////////////////////////
void giop_1_1_Impl::marshalLocateRequestHeader::marshalData()
{
  cdrStream& s = *pd_s;

  pd_request_id >>= s;

  // object key
  pd_i->keysize() >>= s;
  s.put_char_array((CORBA::Char*) pd_i->key(), pd_i->keysize());
}

//////////////////////////////////////////////////////////////////
size_t giop_1_1_Impl::marshalLocateRequestHeader::dataSize(size_t initialoffset)
{
  cdrCountingStream s(initialoffset);

  pd_request_id >>= s;

  // object key
  pd_i->keysize() >>= s;
  s.put_char_array((CORBA::Char*) pd_i->key(), pd_i->keysize());
  
  return s.total();
}

//////////////////////////////////////////////////////////////////
void giop_1_1_Impl::marshalReplyHeader::marshalData()
{
  cdrStream& s = *pd_s;

  // Service context
  operator>>= ((CORBA::ULong)0,s);

  pd_r.requestID() >>= s;
  CORBA::ULong v = (CORBA::ULong) pd_t;
  v >>= s;
}

//////////////////////////////////////////////////////////////////
size_t giop_1_1_Impl::marshalReplyHeader::dataSize(size_t initialoffset)
{
  cdrCountingStream s(initialoffset);

  // Service context
  operator>>= ((CORBA::ULong)0,s);

  pd_r.requestID() >>= s;
  CORBA::ULong v = (CORBA::ULong) pd_t;
  v >>= s;

  return s.total();
}


//////////////////////////////////////////////////////////////////
void giop_1_1_Impl::marshalLocateReplyHeader::marshalData()
{
  cdrStream& s = *pd_s;
  
  pd_r.requestID() >>= s;

  CORBA::ULong rc = (CORBA::ULong) pd_t;
  rc >>= s;
}

//////////////////////////////////////////////////////////////////
size_t giop_1_1_Impl::marshalLocateReplyHeader::dataSize(size_t initialoffset)
{
  cdrCountingStream s(initialoffset);
  pd_r.requestID() >>= s;
  CORBA::ULong rc = (CORBA::ULong) pd_t;
  rc >>= s;
  return s.total();
}


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_giopStream11_initialiser : public omniInitialiser {
public:

  void attach() {
    if (!giop_1_1_singleton) {
      giop_1_1_singleton = new giop_1_1_Impl();
    }
  }

  void detach() { }
};

static omni_giopStream11_initialiser initialiser;

omniInitialiser& omni_giopStream11_initialiser_ = initialiser;
