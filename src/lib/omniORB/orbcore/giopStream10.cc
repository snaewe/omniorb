// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopStream10.cc            Created on: 11/2/99
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
  Revision 1.1.4.3  2000/11/15 17:23:16  sll
  Added call interceptors. Added  char, wchar codeset convertor support.

  Revision 1.1.4.2  2000/11/03 19:12:06  sll
  Use new marshalling functions for byte, octet and char. Use get_octet_array
  instead of get_char_array and put_octet_array instead of put_char_array.

  Revision 1.1.4.1  2000/09/27 17:30:29  sll
  *** empty log message ***

  Revision 1.1.2.3  2000/03/27 17:37:00  sll
  Changed to use the new giopStreamImpl interface.
  Added support to allow call multiplexing on the client side.

  Revision 1.1.2.2  1999/11/04 20:20:20  sll
  GIOP engines can now do callback to the higher layer to calculate total
  message size if necessary.
  Where applicable, changed to use the new server side descriptor-based stub.

  Revision 1.1.2.1  1999/09/15 20:37:28  sll
  *** empty log message ***

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/omniInterceptors.h>
#include <giopStreamImpl.h>
#include <initialiser.h>

#define PARANOID

// giop_1_0_Impl implements the giopStreamImpl interface for GIOP revision 1.0
// See also the description in giopStreamImpl.h and giopStream.h


static GIOP::Version   giop_1_0_version = { 1, 0};
static giop_1_0_Impl*  giop_1_0_singleton = 0;

static const char requestHeader[8] = { 
  'G','I','O','P',1,0,_OMNIORB_HOST_BYTE_ORDER_,GIOP::Request };

static const char replyHeader[8] = {
   'G','I','O','P',1,0,_OMNIORB_HOST_BYTE_ORDER_,GIOP::Reply };

static const char cancelRequestHeader[8] = {
   'G','I','O','P',1,0,_OMNIORB_HOST_BYTE_ORDER_,GIOP::CancelRequest };

static const char locateRequestHeader[8] = {
   'G','I','O','P',1,0,_OMNIORB_HOST_BYTE_ORDER_,GIOP::LocateRequest };

static const char locateReplyHeader[8] = {
   'G','I','O','P',1,0,_OMNIORB_HOST_BYTE_ORDER_,GIOP::LocateReply };

static const char closeConnectionHeader[8] = {
   'G','I','O','P',1,0,_OMNIORB_HOST_BYTE_ORDER_,GIOP::CloseConnection };

static const char messageErrorHeader[8] = {
   'G','I','O','P',1,0,_OMNIORB_HOST_BYTE_ORDER_,GIOP::MessageError };

#define LOGMESSAGE(level,prefix,message) do {\
   if (omniORB::trace(level)) {\
     omniORB::logger log;\
	log << " giop 1.0 " ## prefix ## ": " message ## "\n";\
   }\
} while (0)

#define PTRACE(prefix,message) LOGMESSAGE(25,prefix,message)


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// giop_1_0_Impl implements the giopStreamImpl interface for GIOP         // 
// revision 1.0 See also the description in giopStreamImpl.h and          //
// giopStream.h                                                           //
////////////////////////////////////////////////////////////////////////////
class giop_1_0_Impl : public giopStreamImpl {

public:

  //////////////////////////////////////////////////////////////////
  giop_1_0_Impl() : giopStreamImpl(giop_1_0_version) {}

  //////////////////////////////////////////////////////////////////
  ~giop_1_0_Impl() {}

private:

  //////////////////////////////////////////////////////////////////
  class marshalRequestHeader : public giopMarshaller {
  public:
    marshalRequestHeader(omniIOR* ior,
			 const char* op,size_t opsz,_CORBA_Boolean w,
			 CORBA::ULong req,
			 const IOP::ServiceContextList& svc) :
      pd_ior(ior), pd_op(op), pd_opsize(opsz), 
      pd_response_expected(w), pd_request_id(req), 
      pd_service_context(svc) {}

    void marshal(cdrStream&);

  private:
    omniIOR*                       pd_ior;
    const char*                    pd_op;
    size_t                         pd_opsize;
    CORBA::Boolean                 pd_response_expected;
    CORBA::ULong                   pd_request_id;
    const IOP::ServiceContextList& pd_service_context;

    marshalRequestHeader();
    marshalRequestHeader(const marshalRequestHeader&);
    marshalRequestHeader& operator=(const marshalRequestHeader&);
  };

  //////////////////////////////////////////////////////////////////
  class marshalLocateRequestHeader : public giopMarshaller {
  public:
    marshalLocateRequestHeader(omniIOR* ior, CORBA::ULong req) :
      pd_ior(ior), pd_request_id(req) {}

    void marshal(cdrStream& s);

  private:
    omniIOR* pd_ior;
    CORBA::ULong    pd_request_id;

    marshalLocateRequestHeader();
    marshalLocateRequestHeader(const marshalLocateRequestHeader&);
    marshalLocateRequestHeader& operator=(const marshalLocateRequestHeader&);
  };

  //////////////////////////////////////////////////////////////////
  class marshalReplyHeader : public giopMarshaller {
  public:
    marshalReplyHeader(giopStream::requestInfo& r, GIOP::ReplyStatusType t) :
      pd_r(r), pd_t(t) {}

    void marshal(cdrStream&);

  private:
    giopStream::requestInfo& pd_r;
    GIOP::ReplyStatusType    pd_t;

    marshalReplyHeader();
    marshalReplyHeader(const marshalReplyHeader&);
    marshalReplyHeader& operator=(const marshalReplyHeader&);
  };

  //////////////////////////////////////////////////////////////////
  class marshalLocateReplyHeader : public giopMarshaller {
  public:
    marshalLocateReplyHeader(giopStream::requestInfo& r,
			     GIOP::LocateStatusType t) :
      pd_r(r), pd_t(t) {}

    void marshal(cdrStream&);

  private:
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
					omniIOR* ior,
					const char* opname,
					size_t opnamesize,
					CORBA::Boolean oneway,
					CORBA::Boolean response_expected)
  {	
    omniInterceptors::clientSendRequest_T::info_T info(*g,
						       *ior,
						       opname,
						       oneway,
						       response_expected);

    omniORB::getInterceptors()->clientSendRequest.visit(info);

    g->pd_output_at_most_once = oneway;

    g->pd_request_id = g->pd_strand->sequenceNumber();
    marshalRequestHeader marshaller(ior,opname,opnamesize,
				    (oneway?0:response_expected),
				    g->pd_request_id,
				    info.service_contexts);
    outputMessageBegin(g,requestHeader,&marshaller);

    return g->pd_request_id;
  }

  //////////////////////////////////////////////////////////////////
  CORBA::Long outputLocateMessageBegin(giopStream* g, omniIOR* ior)
  {
    g->pd_output_at_most_once = 0;

    g->pd_request_id = g->pd_strand->sequenceNumber();
    marshalLocateRequestHeader marshaller(ior,g->pd_request_id);
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

    marshalReplyHeader marshaller(f,status);
    outputMessageBegin(g,replyHeader,&marshaller);
  }

  //////////////////////////////////////////////////////////////////
  void outputLocateReplyMessageBegin(giopStream* g,
				     giopStream::requestInfo& f,
				     GIOP::LocateStatusType status)
  {
    g->pd_output_at_most_once = 0;

    f.requestID(g->pd_request_id);
    marshalLocateReplyHeader marshaller(f,status);
    outputMessageBegin(g,locateReplyHeader,&marshaller);
  }

  //////////////////////////////////////////////////////////////////
  void outputMessageEnd(giopStream* g)
  {
    if (!g->pd_output_msgfrag_size) {
      g->pd_output_msgfrag_size = (omni::ptr_arith_t)g->pd_outb_mkr -
	(omni::ptr_arith_t)g->pd_outb_begin - 12;
      outputMessageSize(g);
    }

    g->pd_strand->giveback_reserved((omni::ptr_arith_t)g->pd_outb_end -
				    (omni::ptr_arith_t)g->pd_outb_mkr,1,1);
    g->pd_outb_begin = g->pd_outb_end = g->pd_outb_mkr;
  }

private:

  //////////////////////////////////////////////////////////////////
  void outputMessageBegin(giopStream* g,
			  const char* hdr,
			  giopMarshaller* marshalhdr)
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

    g->pd_output_msgsent_size = 0;

    g->pd_output_header_marshaller = marshalhdr;

    g->pd_output_header_marshaller->marshal(*g);

    if (!g->pd_output_msgfrag_size) {
      g->pd_output_hdr_end = g->pd_outb_mkr;
    }

    g->pd_output_header_marshaller = 0;
  }

  //////////////////////////////////////////////////////////////////
  void outputMessageSize(giopStream* g)
  {
    CORBA::ULong* hdr = (CORBA::ULong*)((char*)g->pd_outb_begin + 8);
    if (!g->pd_marshal_byte_swap) {
      *hdr = g->pd_output_msgfrag_size;
    }
    else {
      CORBA::ULong fragsz = g->pd_output_msgfrag_size;
      *hdr = ((((fragsz) & 0xff000000) >> 24) |
	      (((fragsz) & 0x00ff0000) >> 8)  |
	      (((fragsz) & 0x0000ff00) << 8)  |
	      (((fragsz) & 0x000000ff) << 24));
    }
  }

public:

  //////////////////////////////////////////////////////////////////
  void getReserveSpace(giopStream* g,omni::alignment_t align,size_t reqsize)
  {
    if (omni::align_to((omni::ptr_arith_t)g->pd_outb_mkr,align)+
	(omni::ptr_arith_t)reqsize <= (omni::ptr_arith_t) g->pd_outb_end)
      return;

    g->pd_output_msgsent_size += (omni::ptr_arith_t)g->pd_outb_mkr -
      (omni::ptr_arith_t)g->pd_outb_begin;

    if (g->pd_output_msgfrag_size == 0) {

      g->pd_output_msgsent_size -= 12; // subtract the header

      // Call the  marshaller object to work out the size of the message. 
      PTRACE("getReserveSpace","buffer overflow, calculate body size");

      if (g->pd_output_header_marshaller) {
	cdrCountingStream s(g->TCS_C(),g->TCS_W(),12);
	g->pd_output_header_marshaller->marshal(s);
	g->pd_output_msgfrag_size  = s.total();
      }
      else {
	g->pd_output_msgfrag_size = ((omni::ptr_arith_t)g->pd_output_hdr_end -
				     (omni::ptr_arith_t)g->pd_outb_begin);
      }

      if (g->pd_output_body_marshaller) {
	cdrCountingStream s(g->TCS_C(),g->TCS_W(),g->pd_output_msgfrag_size);
	g->pd_output_body_marshaller->marshal(s);
	g->pd_output_msgfrag_size = s.total();
      }
      g->pd_output_msgfrag_size -= 12;

      outputMessageSize(g); 
    }
    else {
      // This message is actually larger than can be accomodated by the
      // maximum buffer space returned by the strand. The header of the
      // message has already gone out to the network.
    }
    g->pd_strand->giveback_reserved((omni::ptr_arith_t)g->pd_outb_end -
				    (omni::ptr_arith_t)g->pd_outb_mkr,0,0);
    g->pd_outb_begin = g->pd_outb_end = g->pd_outb_mkr;


    int oldalign = giopStreamImpl::address_alignment(g->pd_outb_mkr);
    size_t sz = g->pd_strand->max_reserve_buffer_size();
    if (sz > outputRemaining(g)) sz = outputRemaining(g);

    Strand::sbuf b = g->pd_strand->reserve(sz,1,oldalign);
    g->pd_outb_begin = g->pd_outb_mkr = b.buffer;
    g->pd_outb_end = (char*)g->pd_outb_begin + b.size;

    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)g->pd_outb_mkr,
					  align);
    omni::ptr_arith_t p2 = p1 + reqsize;
    if( (void*)p2 > g->pd_outb_end ) {
      PTRACE("getReserveSpace","MARSHAL exception(reserve space fail)");
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }
  }

  //////////////////////////////////////////////////////////////////
  void copyOutputData(giopStream* g,void* buf, size_t size,
		      omni::alignment_t alignment)
  {
    OMNIORB_ASSERT((size_t)((omni::ptr_arith_t) g->pd_outb_end - (omni::ptr_arith_t) g->pd_outb_begin) <= g->pd_strand->max_reserve_buffer_size());

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

      // Call the marshaller object to work out the size of the message.
      PTRACE("copyOutputData","calculate body size");

      if (g->pd_output_header_marshaller) {
	cdrCountingStream s(g->TCS_C(),g->TCS_W(),12);
	g->pd_output_header_marshaller->marshal(s);
	g->pd_output_msgfrag_size  = s.total();
      }
      else {
	g->pd_output_msgfrag_size = ((omni::ptr_arith_t)g->pd_output_hdr_end -
				     (omni::ptr_arith_t)g->pd_outb_begin);
      }
      if (g->pd_output_body_marshaller) {
	cdrCountingStream s(g->TCS_C(),g->TCS_W(),g->pd_output_msgfrag_size);
	g->pd_output_body_marshaller->marshal(s);
	g->pd_output_msgfrag_size = s.total();
      }
      g->pd_output_msgfrag_size -= 12;
      outputMessageSize(g);
    }

    g->pd_strand->giveback_reserved(avail);
    g->pd_outb_begin = g->pd_outb_end = g->pd_outb_mkr;

    if (outputRemaining(g) >= (padding + size)) {
      g->pd_output_msgsent_size += (padding + size);
    }
    else {
      PTRACE("copyOutputData","MARSHAL exception(not enough space)");
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }

    Strand* s = g->pd_strand;
    Strand::sbuf b;
    b.buffer = (void*) buf;
    b.size = size;

    if (padding) {
      s->reserve(padding,1,align);
    }

    s->reserve_and_copy(b);

    g->pd_outb_mkr = (void*)((omni::ptr_arith_t)g->pd_outb_mkr + 
			     padding + size);
    align = giopStreamImpl::address_alignment(g->pd_outb_mkr);

    // the upcoming data is part of the message which we have just sent
    b = s->reserve(s->max_reserve_buffer_size(),1,align);
    g->pd_outb_begin = g->pd_outb_mkr = b.buffer;
    g->pd_outb_end = (char*)g->pd_outb_begin + b.size;
  }

  //////////////////////////////////////////////////////////////////
  CORBA::ULong currentOutputPtr(const giopStream* g) const
  {
    return (GIOP_Basetypes::max_giop_message_size - outputRemaining(g));
  }


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

   /////////////////////////////////////////////////////////////////
   //
   //          Member functions to input GIOP messages
   //
   /////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////
  size_t inputRemaining(giopStream* g)
  {
    return ((omni::ptr_arith_t)g->pd_inb_end - 
	    (omni::ptr_arith_t)g->pd_inb_mkr)
            + g->pd_input_msgfrag_to_come;
  }

  //////////////////////////////////////////////////////////////////
  void getInputData(giopStream* g,omni::alignment_t align,size_t reqsize)
  {
    size_t padding = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,align) -
      (omni::ptr_arith_t)g->pd_inb_mkr;

    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
      (omni::ptr_arith_t) g->pd_inb_mkr;

    if (avail >= (padding+reqsize)) return;

    g->pd_strand->giveback_received(avail);
    g->pd_input_msgfrag_to_come += avail;
    g->pd_inb_end = g->pd_inb_mkr;
    g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
				     (omni::ptr_arith_t)g->pd_inb_begin);

    do {
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
      else {
	PTRACE("getInputData","MARSHAL exception(no more data from this message)");
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
    } while (0);
  }

  //////////////////////////////////////////////////////////////////
  size_t max_input_buffer_size(giopStream* g, omni::alignment_t align)
  {
    size_t avail = ((omni::ptr_arith_t) g->pd_inb_end - (omni::ptr_arith_t) 
		    g->pd_inb_mkr) + g->pd_input_msgfrag_to_come;

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

    if (size > g->pd_input_msgfrag_to_come) {
      PTRACE("skipInputData","MessageError(request size too large)");
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
	    PTRACE("copyInputData","MessageError(protocol violation. Data is not integral multiple of requested element size)");
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
      else {
	// This is already the last fragment.
	PTRACE("copyInputData","MARSHAL exception(reached end of message)");
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
  GIOP::MsgType inputMessageBegin(giopStream* g,CORBA::Boolean headerInBuffer)
  {
    // NOTE: any state setup by this member function should be transferred
    //       by transferInputContext()

    if (!headerInBuffer) {
      OMNIORB_ASSERT(g->pd_inb_end == g->pd_inb_mkr);

      Strand::sbuf b;
      b = g->pd_strand->receive(12,1,omni::max_alignment,1);
      g->pd_inb_begin = g->pd_inb_mkr = b.buffer;
      g->pd_inb_end = (void *) ((omni::ptr_arith_t)g->pd_inb_mkr + b.size);
    }

    unsigned char* hdr = (unsigned char*) g->pd_inb_mkr;
    if (hdr[0] != 'G' || hdr[1] != 'I' || hdr[2] != 'O' || hdr[3] != 'P' ||
	hdr[4] != 1   || hdr[5] != 0)
      {
	goto bail_out;
      }
  
    g->pd_unmarshal_byte_swap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_)
				 ? 0 : 1 );

    g->pd_input_msgfrag_to_come = *(CORBA::ULong*)(hdr + 8);

    if (g->pd_unmarshal_byte_swap) {
      CORBA::ULong fragsz = g->pd_input_msgfrag_to_come;
      g->pd_input_msgfrag_to_come = ((((fragsz) & 0xff000000) >> 24) |
				     (((fragsz) & 0x00ff0000) >> 8)  |
				     (((fragsz) & 0x0000ff00) << 8)  |
				     (((fragsz) & 0x000000ff) << 24));
    }

    if (hdr[7] > (unsigned char) GIOP::MessageError) {

      goto bail_out;
    }

    if (g->pd_input_msgfrag_to_come > GIOP_Basetypes::max_giop_message_size) {

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
    g->SendMsgErrorMessage();
    g->setTerminalError();
    g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
    // never reach here.
    return (GIOP::MsgType) 0; // Dummy return
  }

  //////////////////////////////////////////////////////////////////
  void transferInputContext(giopStream* g, giopStream* dest)
  {
    // NOTE: this function transfer the state setup by inputMessageBegin()
    //       Any state changes made in inputMessageBegin() must be transfer
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

    GIOP::MsgType t = inputMessageBegin(g,headerInBuffer);
  
    unsigned char* hdr = (unsigned char*) g->pd_inb_mkr;

    g->pd_inb_mkr = (void*) ((omni::ptr_arith_t)g->pd_inb_mkr + 12);

    switch (t) {
    case GIOP::Request:
      {
	unmarshalRequestHeader(g,r);
	g->pd_request_id = r.requestID();
	omniInterceptors::serverReceiveRequest_T::info_T info(*g,r);
	omniORB::getInterceptors()->serverReceiveRequest.visit(info);
	break;
      }
    case GIOP::LocateRequest:
      {
	unmarshalLocateRequestHeader(g,r);
	g->pd_request_id = r.requestID();
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

    GIOP::MsgType t = inputMessageBegin(g,0);

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
  void inputMessageEnd(giopStream* g,CORBA::Boolean disgard,
		       CORBA::Boolean error)
  {
    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
      (omni::ptr_arith_t) g->pd_inb_mkr;
    if (avail) {
      g->pd_strand->giveback_received(avail);
      g->pd_input_msgfrag_to_come += avail;
      g->pd_inb_end = g->pd_inb_mkr;
    }

    if (error) {
      // Either the caller have identified a comm error or
      // There are more fragments to come and the message does not
      // end here.
      SendMsgErrorMessage(g);
      g->setTerminalError();
      g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
      // never reach here.
    }

    if (!disgard && g->pd_input_msgfrag_to_come) {
      if (omniORB::traceLevel >= 15) {
	omniORB::log << "giop_1_0_Impl::inputMessageEnd: garbage left at the end of message.\n";
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
      while (g->pd_input_msgfrag_to_come) {
	g->pd_strand->skip(g->pd_input_msgfrag_to_come);
	g->pd_input_msgfrag_to_come = 0;
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
	  "giop_1_0_Impl::copyMessageBodyFrom() should not be called.");
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
    r.service_contexts() <<= s;

    CORBA::ULong   vl;
    CORBA::Boolean vb;

    // request id
    vl <<= s;
    r.requestID(vl);

    // response expected
    vb = s.unmarshalBoolean();
    r.response_expected(vb);
    r.result_expected(1);

    // object key
    vl <<= s;
    if (!s.checkInputOverrun(1,vl)) {
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
    r.keysize(vl);
    s.get_octet_array(r.key(),vl);

    // operation
    vl <<= s;
    if (!vl || !s.checkInputOverrun(1,vl)) {
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
    r.set_operation_size(vl);
    char* op = r.operation();
    s.get_octet_array((CORBA::Octet*)op,vl);
    op[vl-1] = '\0';

    // principal
    vl <<= s;
    if (!s.checkInputOverrun(1,vl)) {
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
    r.set_principal_size(vl);
    s.get_octet_array(r.principal(), vl);
  }

  //////////////////////////////////////////////////////////////////
  void unmarshalLocateRequestHeader(giopStream* g,
				    giopStream::requestInfo& r)
  {
    cdrStream& s = *g;

    CORBA::ULong vl;

    // request ID
    vl <<= s;
    r.requestID(vl);

    // object key
    vl <<= s;
    if (!s.checkInputOverrun(1,vl)) {
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
    r.keysize(vl);
    s.get_octet_array(r.key(),vl);
  }

  //////////////////////////////////////////////////////////////////
  void unmarshalReplyHeader(giopStream* g,
			    CORBA::ULong& reqid)
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
  void unmarshalLocateReplyHeader(giopStream* g,
				  CORBA::ULong& reqid)
  {
    // Note: Any data extracted by this function should be transferred by
    //       transferLocateReplyHeader()

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

  giop_1_0_Impl(const giop_1_0_Impl&);
  giop_1_0_Impl& operator=(const giop_1_0_Impl&);

  ////////////////////////////////////////////////////////////////
  //        End of class giop_1_0_Impl.
};


//////////////////////////////////////////////////////////////////
void giop_1_0_Impl::marshalRequestHeader::marshal(cdrStream& s)
{
  // Service context
  pd_service_context >>= s;

  pd_request_id >>= s;

  s.marshalBoolean(pd_response_expected);

  // object key
  pd_ior->iiop.object_key >>= s;

  // operation
  operator>>= ((CORBA::ULong) pd_opsize, s);
  s.put_octet_array((CORBA::Octet*) pd_op, pd_opsize);

  // principal
  operator>>= ((CORBA::ULong)0,s);
}

//////////////////////////////////////////////////////////////////
void giop_1_0_Impl::marshalLocateRequestHeader::marshal(cdrStream& s)
{
  pd_request_id >>= s;

  // object key
  pd_ior->iiop.object_key >>= s;
}

//////////////////////////////////////////////////////////////////
void giop_1_0_Impl::marshalReplyHeader::marshal(cdrStream& s)
{
  // Service context
  operator>>= ((CORBA::ULong)0,s);

  pd_r.requestID() >>= s;
  CORBA::ULong v = (CORBA::ULong) pd_t;
  v >>= s;
}

//////////////////////////////////////////////////////////////////
void giop_1_0_Impl::marshalLocateReplyHeader::marshal(cdrStream& s)
{
  pd_r.requestID() >>= s;

  CORBA::ULong rc = (CORBA::ULong) pd_t;
  rc >>= s;
}

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_giopStream10_initialiser : public omniInitialiser {
public:

  void attach() {
    if (!giop_1_0_singleton) {
      giop_1_0_singleton = new giop_1_0_Impl();
    }
  }

  void detach() { }
};

static omni_giopStream10_initialiser initialiser;

omniInitialiser& omni_giopStream10_initialiser_ = initialiser;
