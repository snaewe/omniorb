// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopStream12.cc            Created on: 11/2/99
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
  Revision 1.1.4.1  2000/09/27 17:30:29  sll
  *** empty log message ***

  Revision 1.1.2.7  2000/03/27 17:37:50  sll
  Changed to use the new giopStreamImpl interface.
  Added support to allow call multiplexing on the client side.

  Revision 1.1.2.6  2000/02/14 18:06:45  sll
  Support GIOP 1.2 fragment interleaving. This requires minor access control
  changes to the relevant classes.

  Revision 1.1.2.5  1999/11/10 14:02:45  sll
  When omniORB::strictIIOP is not set, do not throw marshal exception even if
  an incoming non-fragmented request message with no message body (because
  the operation takes no argument) has been padded with extra bytes at the
  end to make the end of the request message aligned on multiple of 8.

  Revision 1.1.2.4  1999/11/04 20:20:20  sll
  GIOP engines can now do callback to the higher layer to calculate total
  message size if necessary.
  Where applicable, changed to use the new server side descriptor-based stub.

  Revision 1.1.2.3  1999/10/05 20:35:34  sll
  Added support to GIOP 1.2 to recognise all TargetAddress mode.
  Now handles NEEDS_ADDRESSING_MODE and LOC_NEEDS_ADDRESSING_MODE.

  Revision 1.1.2.2  1999/10/02 18:24:32  sll
  Reformatted trace messages.

  Revision 1.1.2.1  1999/09/15 20:37:29  sll
  *** empty log message ***

*/

#include <omniORB4/CORBA.h>
#include <giopStreamImpl.h>
#include <ropeFactory.h>
#include <tcpSocket.h>
#include <initialiser.h>

#define PARANOID

static GIOP::Version   giop_1_2_version = { 1, 2};
static giop_1_2_Impl*  giop_1_2_singleton = 0;
static giop_1_2_buffered_Impl* giop_1_2_buffered_singleton = 0;

static const char requestHeader[8] = { 
  'G','I','O','P',1,2,_OMNIORB_HOST_BYTE_ORDER_,GIOP::Request };

static const char replyHeader[8] = {
   'G','I','O','P',1,2,_OMNIORB_HOST_BYTE_ORDER_,GIOP::Reply };

static const char cancelRequestHeader[8] = {
   'G','I','O','P',1,2,_OMNIORB_HOST_BYTE_ORDER_,GIOP::CancelRequest };

static const char locateRequestHeader[8] = {
   'G','I','O','P',1,2,_OMNIORB_HOST_BYTE_ORDER_,GIOP::LocateRequest };

static const char locateReplyHeader[8] = {
   'G','I','O','P',1,2,_OMNIORB_HOST_BYTE_ORDER_,GIOP::LocateReply };

static const char closeConnectionHeader[8] = {
   'G','I','O','P',1,2,_OMNIORB_HOST_BYTE_ORDER_,GIOP::CloseConnection };

static const char messageErrorHeader[8] = {
   'G','I','O','P',1,2,_OMNIORB_HOST_BYTE_ORDER_,GIOP::MessageError };

static const char fragmentHeader[8] = {
   'G','I','O','P',1,2,_OMNIORB_HOST_BYTE_ORDER_,GIOP::Fragment };


#define LOGMESSAGE(level,prefix,message) do {\
   if (omniORB::trace(level)) {\
     omniORB::logger log;\
	log << " giop 1.2 " ## prefix ## ": " message ## "\n";\
   }\
} while (0)

#define PTRACE(prefix,message) LOGMESSAGE(25,prefix,message)

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// giop_1_2_Impl implements the giopStreamImpl interface for GIOP         // 
// revision 1.2 See also the description in giopStreamImpl.h and          //
// giopStream.h                                                           //
////////////////////////////////////////////////////////////////////////////
class giop_1_2_Impl : public giopStreamImpl {

public:

  //////////////////////////////////////////////////////////////////
  giop_1_2_Impl() : giopStreamImpl(giop_1_2_version) {}


  //////////////////////////////////////////////////////////////////
  ~giop_1_2_Impl() {}


private:

  //////////////////////////////////////////////////////////////////
  class marshalRequestHeader : public giopMarshaller {
  public:
    marshalRequestHeader(omniIOR* ior,
			 const char* op,size_t opsz,CORBA::Boolean w,
			 CORBA::Boolean resp, CORBA::ULong req) :
      end_of_data(0),
      pd_ior(ior), pd_op(op), pd_opsize(opsz), pd_oneway(w),
      pd_response_expected(resp),
      pd_request_id(req) {}

    void marshal(cdrStream&);

    void* end_of_data;

  private:
    omniIOR*         pd_ior;
    const char*      pd_op;
    size_t           pd_opsize;
    CORBA::Boolean   pd_oneway;
    CORBA::Boolean   pd_response_expected;
    CORBA::ULong     pd_request_id;

    marshalRequestHeader();
    marshalRequestHeader(const marshalRequestHeader&);
    marshalRequestHeader& operator=(const marshalRequestHeader&);
  };

  friend class marshalRequestHeader;

  //////////////////////////////////////////////////////////////////
  class marshalLocateRequestHeader : public giopMarshaller {
  public:
    marshalLocateRequestHeader(omniIOR* ior,CORBA::ULong req) :
      pd_ior(ior), pd_request_id(req) {}

    void marshal(cdrStream&);

  private:
    omniIOR*        pd_ior;
    CORBA::ULong    pd_request_id;

    marshalLocateRequestHeader();
    marshalLocateRequestHeader(const marshalLocateRequestHeader&);
    marshalLocateRequestHeader& operator=(const marshalLocateRequestHeader&);
  };

  //////////////////////////////////////////////////////////////////
  class marshalReplyHeader : public giopMarshaller {
  public:
    marshalReplyHeader(giopStream::requestInfo& r,
		       GIOP::ReplyStatusType t) :
      end_of_data(0), pd_r(r), pd_t(t) {}

    void marshal(cdrStream&);

    void* end_of_data;

  private:
    giopStream::requestInfo& pd_r;
    GIOP::ReplyStatusType    pd_t;

    marshalReplyHeader();
    marshalReplyHeader(const marshalReplyHeader&);
    marshalReplyHeader& operator=(const marshalReplyHeader&);
  };

  friend class marshalReplyHeader;

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
    // XXX Call interceptor(s) 

    g->pd_output_at_most_once = oneway;

    g->pd_request_id = g->pd_strand->sequenceNumber();
    marshalRequestHeader marshaller(ior,opname,opnamesize,oneway,
				    response_expected,
				    g->pd_request_id);
    outputMessageBegin(g,requestHeader,&marshaller);

    if (!g->pd_output_msgsent_size) {
      // The whole request header is still in the buffer.
      // The pd_outb_mkr has been aligned to 8 bytes boundary in
      // preparation for the data in the request body to be marshalled.
      // However, it is also possible that the reply body is empty.
      // If that is the case, we have to back off the paddings in the
      // request header so that the message ends where the request header
      // ends.
      // In order to do that, we have to record where the request header
      // actually ends. This is given in the marshalRequestHeader::end_of_data
      // field.
      g->pd_output_hdr_end = marshaller.end_of_data;
    }
    
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

    if (!g->pd_output_msgsent_size) {
      // The whole reply header is still in the buffer.
      // The pd_outb_mkr has been aligned to 8 bytes boundary in
      // preparation for the data in the reply body to be marshalled.
      // However, it is also possible that the reply body is empty.
      // If that is the case, we have to back off the paddings in the
      // reply header so that the message ends where the reply header
      // ends.
      // In order to do that, we have to record where the reply header
      // actually ends. This is given in the marshalReplyHeader::end_of_data
      // field.
      g->pd_output_hdr_end = marshaller.end_of_data;
    }
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
    if (g->pd_output_msgfrag_size) {
      // This means that the header of the fragment has already gone out.
      // It has the fragment bit set to 1.
      // We flush what is left in the buffer and starts a new fragment.

      g->pd_strand->giveback_reserved((omni::ptr_arith_t)g->pd_outb_end -
				      (omni::ptr_arith_t)g->pd_outb_mkr,1,1);
      g->pd_outb_begin = g->pd_outb_end = g->pd_outb_mkr;

      newFragment(g,fragmentHeader);
      ::operator>>=(g->pd_request_id,(cdrStream&)*g);
    }
    else if (!g->pd_output_msgsent_size && 
	     (g->pd_state == giopStream::OutputRequest || 
	      g->pd_state == giopStream::OutputReply))     {
      
      // The whole request or reply message is still in the body.
      // We have to check if the request or reply body is empty.
      // If that is the case, we have to back off the paddings in the
      // request or reply header so that the message ends where the
      // header ends.
      omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)
					    g->pd_output_hdr_end,
					    omni::ALIGN_8);
      if ((void*)p1 == g->pd_outb_mkr) {
	// Bingo.
	g->pd_outb_mkr = g->pd_output_hdr_end;
      }
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

    if (marshalhdr) marshalhdr->marshal(*g);

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
      ::operator>>=(g->pd_request_id,(cdrStream&)*g);
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
	cdrCountingStream s(12);
	g->pd_output_header_marshaller->marshal(s);
	g->pd_output_msgfrag_size  = s.total();
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
      ::operator>>=(g->pd_request_id,(cdrStream&)*g);
    }
    else {
      int align = giopStreamImpl::address_alignment(g->pd_outb_mkr);
      size_t sz = g->pd_strand->max_reserve_buffer_size();
      sz = (sz >> 3) << 3;
      if (sz > outputRemaining(g)) sz = outputRemaining(g);
      Strand::sbuf b = g->pd_strand->reserve(sz,1,align);
      g->pd_outb_begin = g->pd_outb_mkr = b.buffer;
      g->pd_outb_end = (char*)g->pd_outb_begin + b.size;
    }

    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)g->pd_outb_mkr,
					  align);
    omni::ptr_arith_t p2 = p1 + reqsize;
    if( (void*)p2 > g->pd_outb_end ) {
      PTRACE("getReserveSpace","MARSHAL exception(reserve space fail)");
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }
  }

  //////////////////////////////////////////////////////////////////
  // Invariant:
  //    size >= 7.
  //    This invariant is necessary to make sure that we can flush
  //    the data in the buffer out as a fragment and make sure that
  //    the fragment size is in multiples of 8.
  //
  void copyOutputData(giopStream* g,void* buf, size_t size,
		      omni::alignment_t alignment)
  {
    OMNIORB_ASSERT(size >= 7);

    size_t spillover = 0;

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
      CORBA::ULong fragsz;

      if (g->pd_output_header_marshaller) {
	// Now we are given a callback object to work out the size of the
	// message. We call this method to determine the size.
	cdrCountingStream s(12);
	g->pd_output_header_marshaller->marshal(s);
	g->pd_output_msgfrag_size = fragsz = s.total();
      }
      else {
	fragsz = (omni::ptr_arith_t)g->pd_outb_mkr -
	  (omni::ptr_arith_t)g->pd_outb_begin + padding + size;
	// Now we have to make sure that the size of this fragment is
	// multiple of 8. If not, compute the no. of bytes that has to
	// be sent in the next fragment in order to satisfy this invariant.
	
	spillover = fragsz & 7;

	size -= spillover;

	fragsz -= spillover + 12;
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

    if (g->pd_output_msgfrag_size == 0) {

      s->reserve_and_copy(b,1,1);

      // create a new fragment
      newFragment(g,fragmentHeader);
      ::operator>>=(g->pd_request_id,(cdrStream&)*g);

      if (spillover) {
	memcpy((void*)g->pd_outb_mkr,
	       (void*)((omni::ptr_arith_t)g->pd_outb_mkr + size),
	       spillover);
	g->pd_outb_mkr = (void*)((omni::ptr_arith_t)g->pd_outb_mkr+spillover);
      }

    }
    else {

      s->reserve_and_copy(b);

      g->pd_outb_mkr = (void*)((omni::ptr_arith_t)g->pd_outb_mkr + 
			       padding + size);
      align = giopStreamImpl::address_alignment(g->pd_outb_mkr);

      // the upcoming data is part of the fragment which we have just sent
      size_t sz = s->max_reserve_buffer_size();
      sz = (sz >> 3) << 3;
      b = s->reserve(sz,1,align);
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
    if (sz > s->max_reserve_buffer_size()) {
      sz = s->max_reserve_buffer_size();
      // adjust the size to multiple of 8. It is a requirement of GIOP 1.2
      // that fragment must be multiple of 8 in size.
      sz = (sz >> 3) << 3;
    }

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
  // Invariant:
  //    If fragsz != 0, then ((fragsz+12) % 8 == 0 || last == TRUE)
  void endFragment(giopStream* g, CORBA::ULong fragsz,CORBA::Boolean last)
  {
    CORBA::Boolean endMTU = 0;

    if (!fragsz) {
      fragsz = (omni::ptr_arith_t)g->pd_outb_mkr -
	(omni::ptr_arith_t)g->pd_outb_begin;

      if (!last) {
	CORBA::ULong padding = 8 - (fragsz & 7);
	if (padding != 8) {
	  g->pd_outb_mkr = (void*)((omni::ptr_arith_t)g->pd_outb_mkr +padding);
	  fragsz += padding;
	}
      }
      fragsz -= 12;
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
      OMNIORB_ASSERT(((fragsz + 12) & 7) == 0);
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
    // maximum space that can be provided by the strand rounded to multiple
    // of 8. We have to make sure that fragment size is in multiple of 8.
    // Subtract the GIOP header size. Unless the limit on the overall GIOP
    // message limit has been exceeded, this value is what getReserveSpace
    // can support.
    size_t total = g->pd_strand->max_reserve_buffer_size();
    total = (total >> 3) << 3;
    size_t overhead = omni::align_to((omni::ptr_arith_t)16,align);
    return (total - overhead);
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
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,
					  align);

    size_t padding = p1 - (omni::ptr_arith_t)g->pd_inb_mkr;

    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
      (omni::ptr_arith_t) g->pd_inb_mkr;

    if (avail >= (padding+reqsize)) return;

    skipInputData(g,padding);

    if (!omniORB::strictIIOP && reqsize == 0) return;
    // There is no reason for the upper level to call this function, even
    // when reqsize == 0, *unless* it is expecting more data to come.
    // If we do not return here, we just let the code below to do a
    // check on whether there are more data to come. And if not, throws
    // a marshal exception. 
    // Now that we return here if reqsize == 0, we are letting sloppy
    // implementation to slip through undetected!
    // Only do so if strictIIOP is not set!

  again:
    if (g->pd_input_msgfrag_to_come) {

      if (g->pd_input_msgfrag_to_come < reqsize ||
	  g->pd_strand->max_receive_buffer_size() < reqsize) {

	// The request size is too large. 
	g->setTerminalError();
	PTRACE("getInputData","MARSHAL exception(request size too large)");
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
      
      Strand::sbuf b;
      size_t sz = g->pd_strand->max_receive_buffer_size();
      if (sz > g->pd_input_msgfrag_to_come) sz = g->pd_input_msgfrag_to_come;
      b = g->pd_strand->receive(sz,0,
			   giopStreamImpl::address_alignment(g->pd_inb_mkr));
      if (b.size < reqsize) {
	g->pd_strand->giveback_received(b.size);
	b = g->pd_strand->receive(reqsize,1,
			giopStreamImpl::address_alignment(g->pd_inb_mkr));
      }
      g->pd_inb_begin = g->pd_inb_mkr = b.buffer;
      g->pd_inb_end = (void*) ((omni::ptr_arith_t)g->pd_inb_mkr + b.size);

      // XXX
      OMNIORB_ASSERT(g->pd_input_msgfrag_to_come >= b.size);
      g->pd_input_msgfrag_to_come -= b.size;
    }
    else if (g->pd_input_fragmented) {

      GIOP::MsgType t = inputFragmentBegin(g,1,0,1,0);
      goto again;
    }
    else {
      // This is already the last fragment.
      PTRACE("getInputData","MARSHAL exception(reached last fragment)");
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }
  }

  //////////////////////////////////////////////////////////////////
  size_t max_input_buffer_size(giopStream* g, omni::alignment_t align)
  {
  again:
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,
					  align);

    if (p1 <= ((omni::ptr_arith_t) g->pd_inb_end)) {
	g->pd_inb_mkr = (void*) p1;
    }
    else {
      size_t padding = p1 - (omni::ptr_arith_t)g->pd_inb_mkr;
      skipInputData(g,padding);
    }

    size_t avail = ((omni::ptr_arith_t) g->pd_inb_end - (omni::ptr_arith_t) 
		    g->pd_inb_mkr) + g->pd_input_msgfrag_to_come;

    if (!avail && g->pd_input_fragmented) {

      g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
				       (omni::ptr_arith_t)g->pd_inb_begin);
      (void) inputFragmentBegin(g,1,0,1,0);
      goto again;
    }

    size_t rz = g->pd_strand->max_receive_buffer_size();

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

      (void) inputFragmentBegin(g,1,0,1,0);
      goto again;
    }

    if (size > g->pd_input_msgfrag_to_come) {
      PTRACE("skipInputData","MessageError(request size too large)");
      g->SendMsgErrorMessage();
      g->setTerminalError();
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }

    g->pd_strand->skip(size);

    g->pd_input_msgfrag_to_come -= size;

    g->pd_inb_begin = g->pd_inb_end = g->pd_inb_mkr = (void*)((omni::ptr_arith_t) 
					    g->pd_inb_mkr + size);
    g->pd_input_msgbody_received += size;
  }

  //////////////////////////////////////////////////////////////////
  void copyInputData(giopStream* g,void* b, size_t size,
		     omni::alignment_t alignment)
  {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,
					  alignment);

    size_t padding = p1 - (omni::ptr_arith_t)g->pd_inb_mkr;

    skipInputData(g,padding);

    char* bp = (char*)b;

    while (size) {
    
      if (g->pd_input_msgfrag_to_come) {

	omni::ptr_arith_t p = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,
					     alignment);

	size_t padding = p - (omni::ptr_arith_t) g->pd_inb_mkr; 

	OMNIORB_ASSERT(padding == 0);
      
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
	g->pd_inb_begin = g->pd_inb_mkr = g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_inb_mkr + 
					 reqsize);
	size -= reqsize;
	bp += reqsize;
      }
      else if (g->pd_input_fragmented) {

	GIOP::MsgType t = inputFragmentBegin(g,1,0,1,0);
	continue;
      }
      else {
	// This is already the last fragment.
	PTRACE("copyInputData","MARSHAL exception(reached last fragment)");
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

  CORBA::Boolean saveInterleaveFragment(giopStream* g, 
					CORBA::ULong reqid,
					GIOP::MsgType msgtype);

protected:
  void getHeader12(giopStream*g,CORBA::Boolean headerInBuffer,
		   unsigned char*& hdr,CORBA::Boolean& bswap)
  {
    if (!headerInBuffer) {
      OMNIORB_ASSERT(g->pd_inb_end == g->pd_inb_mkr);

      Strand::sbuf b;
      b = g->pd_strand->receive(12,1,omni::max_alignment,1);
      g->pd_inb_begin = g->pd_inb_mkr = b.buffer;
      g->pd_inb_end = (void *) ((omni::ptr_arith_t)g->pd_inb_mkr + b.size);
    }
    hdr = (unsigned char*) g->pd_inb_mkr;
    bswap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_) ? 0 : 1 );
  }

  void getHeader16(giopStream*g,CORBA::Boolean headerInBuffer,
		   unsigned char*& hdr,CORBA::Boolean& bswap,
		   CORBA::ULong& reqid)
  {
    if (headerInBuffer) {
      g->pd_strand->giveback_received(12);
    }
    else {
      OMNIORB_ASSERT(g->pd_inb_end == g->pd_inb_mkr);
    }
    Strand::sbuf b;
    b = g->pd_strand->receive(16,1,omni::max_alignment,1);
    g->pd_inb_begin = g->pd_inb_mkr = b.buffer;
    g->pd_inb_end = (void *) ((omni::ptr_arith_t)g->pd_inb_mkr + b.size);
    hdr = (unsigned char*) g->pd_inb_mkr;
    bswap = (((hdr[6] & 0x1) == _OMNIORB_HOST_BYTE_ORDER_) ? 0 : 1 );
    reqid = *(CORBA::ULong*)(hdr + 12);
    if (bswap) {
      CORBA::ULong t = reqid;
      reqid = ((((t) & 0xff000000) >> 24) |
	       (((t) & 0x00ff0000) >> 8)  |
	       (((t) & 0x0000ff00) << 8)  |
	       (((t) & 0x000000ff) << 24));
    }	
  }

  void inputCheckFragmentHeader(giopStream* g,
				CORBA::Boolean giop12only,
				CORBA::Boolean headerInBuffer,
				CORBA::Boolean expectFragment,
				CORBA::Boolean expectReply)
  {
  again:
    unsigned char* hdr;
    CORBA::Boolean bswap;
    CORBA::ULong reqid;

    getHeader12(g,headerInBuffer,hdr,bswap);

    if (hdr[0] != 'G' || hdr[1] != 'I' || hdr[2] != 'O' || hdr[3] != 'P' ||
	hdr[4] != 1   || hdr[5] > 2 || (hdr[5] != 2 && giop12only)) {

      // Notice that if giop12only is TRUE, we reject any GIOP 1.0 or 1.1
      // message right here.
      // While the spec. say that a GIOP 1.2 message can interleave with
      // another 1.2 message. It does not say if a GIOP 1.0 or 1.1 message
      // can interleave with a GIOP 1.2 message. Our interpretation is to
      // disallow this.
      // XXX If this is allowed, we have to check the expectFragment flag here,
      // if the flag is TRUE, we do not bail_out.
      PTRACE("inputCheckFragmentHeader","MessageError (invalid GIOP Header)");
      goto bail_out;
    }

    if (hdr[7] > (unsigned char) GIOP::Fragment) goto bail_out;

    switch ((GIOP::MsgType)hdr[7]) {
    case GIOP::Fragment:
    case GIOP::CancelRequest:
      {	
	getHeader16(g,1,hdr,bswap,reqid);

	if (expectFragment) {
	  if (reqid == g->pd_request_id) {
	    if (hdr[7] == (unsigned char) GIOP::CancelRequest) {
	      // We are in the process of unmarshalling an incoming request &
	      // there are still more fragments to come. The request id
	      // in this cancelrequest message match the current request id,
	      // we terminate the processing of the current request.
	      g->pd_inb_mkr = (void*)((omni::ptr_arith_t)g->pd_inb_mkr + 16);
	      throw GIOP_S::terminateProcessing();
	    }
	    if (g->pd_unmarshal_byte_swap != bswap) {
	      // Fragments of the same message must be the same byte order.
	      // This is a protocol violation.
	      PTRACE("inputCheckFragmentHeader","MessageError (different byte order among fragments)");
	      goto bail_out;
	    }
	    return;
	  }
	}
	else if (hdr[7] == (unsigned char) GIOP::CancelRequest)
	  return;

	// Reach here for one of the following reason:
	//  1. This fragment is part of another request or
	//  2. This cancel message is targeting another request.
	goto demux;
	break;     // dummy statement
      }
    case GIOP::MessageError:
    case GIOP::CloseConnection:
      {
	PTRACE("inputCheckFragmentHeader","MessageError (Unexpected message type)");
	goto bail_out;
	break;     // dummy statement
      }
    case GIOP::Reply:
    case GIOP::LocateReply:
      {
	getHeader16(g,1,hdr,bswap,reqid);

	if (expectReply && g->pd_request_id == reqid)
	  return;
	else
	  goto demux;
	break; 	   // dummy statement
      }
    default:
      {
	if (expectFragment) {
	  // We expect a fragment and this isn't.
	  getHeader16(g,1,hdr,bswap,reqid);
	  goto demux;
	}
      }
    }
    return;

  demux:
    // This is the demultiplexing point.
    // Reach here only because we have in the buffer the header of a
    // giop message/fragment that does not belong to this stream object.
    // We either create a new buffered giopStream object or insert
    // this fragment into an existing buffered giopStream object.
    // After this is done, we go back to the beginning to look at the
    // header of the next fragment. This process repeats until we've
    // encountered the fragment we expect.

    // Reach here:
    //  reqid = Request ID of this fragment
    //  hdr   = points to the start of the header
    //  bswap = TRUE(1) if byte swap is necessary

    if (saveInterleaveFragment(g,reqid,(GIOP::MsgType)hdr[7])) {
      headerInBuffer = 0;
      goto again;
    }
    else
      goto bail_out;

  bail_out:
    if (hdr[7] != (unsigned char) GIOP::MessageError &&
	hdr[7] != (unsigned char) GIOP::CloseConnection) {
      g->SendMsgErrorMessage();
    }
    g->setTerminalError();
    g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
  }


  //////////////////////////////////////////////////////////////////
  // Post-condition:
  //    If expectFragment == TRUE, then on return g->pd_inb_mkr has moved
  //    beyond the fragment header.
  //    Otherwise, the g->pd_inb_mkr stays pointing to the beginning of
  //    the fragment.
  virtual GIOP::MsgType inputFragmentBegin(giopStream* g,
					   CORBA::Boolean giop12only,
					   CORBA::Boolean headerInBuffer,
					   CORBA::Boolean expectFragment,
					   CORBA::Boolean expectReply)
  {
    inputCheckFragmentHeader(g,giop12only,headerInBuffer,
			     expectFragment,expectReply);
    // If expectFragment == 1
    //    returns here only if the incoming contains a fragment that is part 
    //    of the current message or a CancelRequest message direct at
    //    this request.
    // else
    //    returns here if this is a new GIOP message and is not a fragment.

    unsigned char* hdr = (unsigned char*) g->pd_inb_mkr;

    if (!expectFragment) {
      g->pd_unmarshal_byte_swap = (((hdr[6] & 0x1)==_OMNIORB_HOST_BYTE_ORDER_)
				   ? 0 : 1 );
    }

    g->pd_input_msgfrag_to_come = *(CORBA::ULong*)(hdr + 8);
    if (g->pd_unmarshal_byte_swap) {
      CORBA::ULong fragsz = g->pd_input_msgfrag_to_come;
      g->pd_input_msgfrag_to_come = ((((fragsz) & 0xff000000) >> 24) |
				     (((fragsz) & 0x00ff0000) >> 8)  |
				     (((fragsz) & 0x0000ff00) << 8)  |
				     (((fragsz) & 0x000000ff) << 24));
    }

    if (hdr[5] >= 1) {
      g->pd_input_fragmented = ((hdr[6] & 0x2) ? 1 : 0);
      if (g->pd_input_fragmented) {
	// Check the size of this fragment is in multiple of 8 bytes.
	// If not, it is protocol violation.
	if (((g->pd_input_msgfrag_to_come + 12) & 7)) {
	  PTRACE("inputFragmentBegin","MessageError (fragment size is not multiple of 8)");
	  goto bail_out;
	}
      }
    }

    if (expectFragment) {
      g->pd_inb_mkr = (void*)((omni::ptr_arith_t)g->pd_inb_mkr + 16);
      g->pd_input_msgfrag_to_come -= 4;
    }
    else {
      // subtract from the count the bytes in the body that has already
      // been read from the strand.
      g->pd_input_msgfrag_to_come -= ((omni::ptr_arith_t)g->pd_inb_end -
				      (omni::ptr_arith_t)g->pd_inb_mkr - 12);
    }

    if ((g->pd_input_msgbody_received + g->pd_input_msgfrag_to_come) > 
        GIOP_Basetypes::max_giop_message_size) {

      // We may choose not to send MessageError but  instead just
      // throw MARSHAL exception.

      PTRACE("inputFragmentBegin","MessageError(message size exceeded limit)");
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

public:
  //////////////////////////////////////////////////////////////////
  GIOP::MsgType inputRequestMessageBegin(giopStream* g,
					 giopStream::requestInfo& r,
					 CORBA::Boolean headerInBuffer)
  {
    g->pd_input_msgbody_received = 0;

    GIOP::MsgType t = inputFragmentBegin(g,0,headerInBuffer,0,0);
  
    unsigned char* hdr = (unsigned char*) g->pd_inb_mkr;

    if (hdr[5] < 2) {
      // This is either a GIOP 1.0 or a GIOP 1.1 message, switch to the 
      // implementation of giop 1.0/1.1 and dispatch again.
      GIOP::Version v;
      v.major = 1;
      v.minor = (CORBA::Char) hdr[5];
      g->pd_impl = giopStreamImpl::matchVersion(v);
      OMNIORB_ASSERT(g->pd_impl);
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
	PTRACE("inputRequestMessageBegin","MessageError (unexpected message type");
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
    g->pd_input_msgbody_received = 0;

    GIOP::MsgType t =  inputFragmentBegin(g,1,0,0,1);

    g->pd_inb_mkr = (void*) ((omni::ptr_arith_t)g->pd_inb_mkr + 12);

    if (t != GIOP::Reply)
      {
	g->setTerminalError();
	g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
	// never reach here.
      }

    CORBA::ULong replyid;
    GIOP::ReplyStatusType rc;

    unmarshalReplyHeader(g,replyid,rc);
    if (g->pd_request_id != replyid) {
      PTRACE("inputReplyMessageBegin","MessageError (request ID mismatch)");
      g->SendMsgErrorMessage();
      g->setTerminalError();
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }
    g->pd_reply_status.replyStatus = rc;
    // XXX Call interceptor(s);
    return 1;
  }

  //////////////////////////////////////////////////////////////////
  CORBA::Boolean inputLocateReplyMessageBegin(giopStream* g)
  {
    g->pd_input_msgbody_received = 0;

    GIOP::MsgType t =  inputFragmentBegin(g,1,0,0,1);

    g->pd_inb_mkr = (void*) ((omni::ptr_arith_t)g->pd_inb_mkr + 12);

    if (t != GIOP::LocateReply)
      {
	g->setTerminalError();
	g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
	// never reach here.
      }

    CORBA::ULong replyid;
    GIOP::LocateStatusType rc;

    unmarshalLocateReplyHeader(g,replyid,rc);
    if (g->pd_request_id != replyid) {
      PTRACE("inputLocateReplyMessageBegin","MessageError (request ID mismatch)");
      g->SendMsgErrorMessage();
      g->setTerminalError();
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }
    g->pd_reply_status.locateReplyStatus = rc;
    return 1;
  }

  //////////////////////////////////////////////////////////////////
  void inputMessageEnd(giopStream* g, CORBA::Boolean disgard,
		       CORBA::Boolean error)
  {
    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
      (omni::ptr_arith_t) g->pd_inb_mkr;
    if (avail) {
      if (!g->terminalError()) g->pd_strand->giveback_received(avail);
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
      (void) inputFragmentBegin(g,1,0,1,0);
    }

    if (error || (g->pd_input_fragmented && ! disgard)) {
      // Either the caller have identified a comm error or
      // There are more fragments to come and the message does not
      // end here.

      if (error) 
	PTRACE("inputMessageEnd","MessageError (detected by stub)");
      else
	PTRACE("inputMessageEnd","MessageError (unexpected fragments left)");
      SendMsgErrorMessage(g);
      g->setTerminalError();
      g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
      // never reach here.
    }
    
    if (!disgard && g->pd_input_msgfrag_to_come) {
      // XXX worry about straight conformance here.
      PTRACE("inputMessageEnd","garbage left at the end of message");
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

	  GIOP::MsgType t = inputFragmentBegin(g,1,0,1,0);
	}
      }
    }
  }

  struct internalBuffer {
    CORBA::Boolean         bodybegin;
    size_t                 size;
    struct internalBuffer* next;
    char                   data[24];  // 16 bytes header + 8 bytes padding
    // more data  to follow
  };


  ///////////////////////////////////////////////////////////////////
  //
  //          Member functions to perform fast copy
  //
  ///////////////////////////////////////////////////////////////////
  CORBA::Boolean startSavingInputMessageBody(giopStream* g);

  CORBA::Boolean prepareCopyMessageBodyFrom(giopStream* g, giopStream& s)
  {
    GIOP::Version v = s.pd_impl->version();
    if (v.major != 1 || v.minor != 2) return 0;
    // This output stream will marshal in the same endian as the source.
    g->pd_marshal_byte_swap = s.pd_unmarshal_byte_swap;
    return 1;
  }

  void copyMessageBodyFrom(giopStream*g, giopStream& s);

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

    CORBA::ULong   vl;
    CORBA::Octet   vb;

    // request id
    vl <<= s;
    r.requestID(vl);
    g->pd_request_id = vl;

    // response flag
    vb <<= s;
    r.response_expected((vb & 0x1));
    r.result_expected((vb & 0x2) ? 1 : 0);

    // padding
    vb <<= s;
    vb <<= s;
    vb <<= s;

    // object key
    r.unmarshalIORAddressingInfo(s);

    // operation
    vl <<= s;
    if (!vl || !s.checkInputOverrun(1,vl)) {
      PTRACE("unmarshalRequestHeader","MARSHAL exception (corrupted header");
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
    r.set_operation_size(vl);
    char* op = r.operation();
    s.get_char_array((CORBA::Char*)op,vl);
    op[vl-1] = '\0';

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

    if (giop_1_2_singleton->inputRemaining(g))
      s.alignInput(omni::ALIGN_8);
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
    r.unmarshalIORAddressingInfo(s);
  }

  //////////////////////////////////////////////////////////////////
  void unmarshalReplyHeader(giopStream* g,CORBA::ULong& reqid,
			    GIOP::ReplyStatusType& rc)
  {
    cdrStream& s = *g;

    reqid <<= s;
    g->pd_request_id = reqid;
  
    CORBA::ULong v;
    v <<= s;

    switch (v) {
    case GIOP::SYSTEM_EXCEPTION:
    case GIOP::NO_EXCEPTION:
    case GIOP::USER_EXCEPTION:
    case GIOP::LOCATION_FORWARD:
    case GIOP::LOCATION_FORWARD_PERM:
    case GIOP::NEEDS_ADDRESSING_MODE:
      break;
    default:
      // Should never receive anything other that the above
      // Same treatment as wrong header
      PTRACE("unmarshalReplyHeader","Error (unrecognised reply. Corrupted header?");
      g->setTerminalError();
      g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
      // never reach here.
    }
    rc = (GIOP::ReplyStatusType) v;

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

    if (giop_1_2_singleton->inputRemaining(g))
      s.alignInput(omni::ALIGN_8);
  }

  //////////////////////////////////////////////////////////////////
  void unmarshalLocateReplyHeader(giopStream* g,CORBA::ULong& reqid,
				  GIOP::LocateStatusType& rc)
  {
    cdrStream& s = *g;

    reqid <<= s;
    g->pd_request_id = reqid;

    CORBA::ULong v;
    v <<= s;

    switch (v) {
    case GIOP::UNKNOWN_OBJECT:
    case GIOP::OBJECT_HERE:
    case GIOP::OBJECT_FORWARD:
    case GIOP::OBJECT_FORWARD_PERM:
    case GIOP::LOC_SYSTEM_EXCEPTION:
    case GIOP::LOC_NEEDS_ADDRESSING_MODE:
      break;
    default:
      // Should never receive anything other that the above
      // Same treatment as wrong header
      g->setTerminalError();
      PTRACE("unmarshalLocateReplyHeader","Error (unrecognised reply. Corrupted header?");
      g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
      // never reach here.
    }
    rc = (GIOP::LocateStatusType) v;
  }

  giop_1_2_Impl(const giop_1_2_Impl&);
  giop_1_2_Impl& operator=(const giop_1_2_Impl&);

  ////////////////////////////////////////////////////////////////
  //        End of class giop_1_2_Impl.
};


//////////////////////////////////////////////////////////////////
void giop_1_2_Impl::marshalRequestHeader::marshal(cdrStream& s)
{
  pd_request_id >>= s;

  // response_flags
  CORBA::Octet v = ((pd_oneway ? 0 : 1) << 1) + 
                   (pd_response_expected ? 1 : 0);
  if (v == 0x02) v = 0x00;    // Can't be. reset to 0x00.
  v >>= s;

  // reserved[3]
  v = 0;
  v >>= s;
  v >>= s;
  v >>= s;

  // Target address
  pd_ior->marshalIORAddressingInfo(s);

    // operation
  operator>>= ((CORBA::ULong) pd_opsize, s);
  s.put_char_array((CORBA::Char*) pd_op, pd_opsize);

    // Service context
  ::operator>>=((CORBA::ULong)0,s);

  end_of_data = s.PR_get_outb_mkr();

  s.alignOutput(omni::ALIGN_8);
}

//////////////////////////////////////////////////////////////////
void giop_1_2_Impl::marshalLocateRequestHeader::marshal(cdrStream& s)
{
  pd_request_id >>= s;

  // Target address
  pd_ior->marshalIORAddressingInfo(s);
}

//////////////////////////////////////////////////////////////////
void giop_1_2_Impl::marshalReplyHeader::marshal(cdrStream& s)
{
  pd_r.requestID() >>= s;

  CORBA::ULong v = (CORBA::ULong) pd_t;
  v >>= s;

  // Service context
  operator>>= ((CORBA::ULong)0,s);

  end_of_data = s.PR_get_outb_mkr();

  s.alignOutput(omni::ALIGN_8);
}

//////////////////////////////////////////////////////////////////
void giop_1_2_Impl::marshalLocateReplyHeader::marshal(cdrStream& s)
{
  pd_r.requestID() >>= s;

  CORBA::ULong rc = (CORBA::ULong) pd_t;
  rc >>= s;
}

////////////////////////////////////////////////////////////////////////////
// giop_1_2_buffered_Impl is a specialisation of giop_1_2_impl which      //
// buffer the whole input message in its internal buffer.                 //
////////////////////////////////////////////////////////////////////////////
class giop_1_2_buffered_Impl : public giop_1_2_Impl {
public:

  //////////////////////////////////////////////////////////////////
  giop_1_2_buffered_Impl() {}

  //////////////////////////////////////////////////////////////////
  ~giop_1_2_buffered_Impl() {}


  void release(giopStream* g)
  {
    size_t leftover = (omni::ptr_arith_t)g->pd_outb_end - 
                      (omni::ptr_arith_t)g->pd_outb_mkr;
    if (g->pd_wrlocked && leftover && !g->terminalError()) {
      // XXX This should not be done if this is a buffered giopStream
      g->pd_strand->giveback_reserved(leftover);
    }
    g->pd_outb_end = g->pd_outb_mkr;

    internalBuffer* bp = (internalBuffer*) g->pd_input_buffers;
    while (bp) {
      char* p = (char*) bp;
      bp = bp->next;
      delete [] p;
    }
    g->pd_input_buffers = g->pd_input_current_buffer = g->pd_input_hdr_end= 0; 
    g->pd_inb_end = g->pd_inb_mkr;
  }

  CORBA::Boolean isBuffered(giopStream*) { return 1; }

  ///////////////////////////////////////////////////////////////////
  //
  //          Member functions to input GIOP messages
  //
  ///////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////
  void getInputData(giopStream* g,omni::alignment_t align, size_t reqsize)
  {
  again:
    size_t padding = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,align) -
      (omni::ptr_arith_t)g->pd_inb_mkr;

    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
                   (omni::ptr_arith_t) g->pd_inb_mkr;

    if (avail >= (padding+reqsize)) return;

    skipInputData(g,padding);

    if (!omniORB::strictIIOP && reqsize == 0) return;
    // There is no reason for the upper level to call this function, even
    // when reqsize == 0, *unless* it is expecting more data to come.
    // If we do not return here, we just let the code below to do a
    // check on whether there are more data to come. And if not, throws
    // a marshal exception. 
    // Now that we return here if reqsize == 0, we are letting sloppy
    // implementation to slip through undetected!
    // Only do so if strictIIOP is not set!

    avail = (omni::ptr_arith_t) g->pd_inb_end - 
            (omni::ptr_arith_t) g->pd_inb_mkr;

    if (avail) {

      if (avail < reqsize) {
	// The request size is too large. 
	g->setTerminalError();
	PTRACE("getInputData(m)","MARSHAL exception(request size too large)");
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
    }
    else if (g->pd_input_fragmented) {
      g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
				       (omni::ptr_arith_t)g->pd_inb_begin);
      (void) inputFragmentBegin(g,1,0,1,0);
      goto again;
    }
    else {
      // This is already the last fragment.
      PTRACE("getInputData(m)","MARSHAL exception(reached last fragment)");
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }
  }

  //////////////////////////////////////////////////////////////////
  void skipInputData(giopStream* g,size_t size)
  {
  again:
    if ((g->pd_inb_mkr == g->pd_inb_end) && g->pd_input_fragmented) {
      g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
				       (omni::ptr_arith_t)g->pd_inb_begin);
      (void) inputFragmentBegin(g,1,0,1,0);
      goto again;
    }

    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
      (omni::ptr_arith_t) g->pd_inb_mkr;

    if (size > avail) {
      PTRACE("skipInputData(m)","MessageError(request size too large)");
      g->SendMsgErrorMessage();
      g->setTerminalError();
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
    }

    g->pd_inb_mkr = (void*) ((omni::ptr_arith_t) g->pd_inb_mkr + size);
  }

  //////////////////////////////////////////////////////////////////
  void copyInputData(giopStream* g,void* b, size_t size,
		     omni::alignment_t alignment)
  {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,
					  alignment);

    size_t padding = p1 - (omni::ptr_arith_t)g->pd_inb_mkr;

    skipInputData(g,padding);

    char* bp = (char*) b;

    while (size) {

      size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
	             (omni::ptr_arith_t) g->pd_inb_mkr;

      if (avail) {

	omni::ptr_arith_t p = omni::align_to((omni::ptr_arith_t)g->pd_inb_mkr,
					     alignment);

	size_t padding = p - (omni::ptr_arith_t) g->pd_inb_mkr; 
	
	OMNIORB_ASSERT(padding == 0);

	size_t reqsize = size;
	if (reqsize > avail) {

	  reqsize = avail;

	  if (reqsize % (int)alignment) {
	    // Protocol violation. The data should be integral multiple 
	    PTRACE("copyInputData(m)","MessageError(protocol violation. Data is not integral multiple of requested element size)");
	    g->SendMsgErrorMessage();
	    g->setTerminalError();
	    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
	  }
	}

	memcpy((void*)bp,g->pd_inb_mkr,reqsize);
	size -= reqsize;
	bp += reqsize;
	g->pd_inb_mkr = (void*)((omni::ptr_arith_t) g->pd_inb_mkr + reqsize);

      }
      else if (g->pd_input_fragmented) {
	g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
					 (omni::ptr_arith_t)g->pd_inb_begin);
	(void) inputFragmentBegin(g,1,0,1,0);
	continue;
      }
      else {
	// This is already the last fragment.
	PTRACE("copyInputData(m)","MARSHAL exception(reached last fragment)");
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      }
    }

  }

  void fetchFragment(giopStream* g) {
    // The following invariants must be true:
    //      1. No data cached from strand i.e. g->pd_inb_end == g->pd_inb_mkr
    //      2. The giopStream <g> is not currently used by another thread
    //         other than the calling thread.
    //         (This is true as long as we do not dispatch a server thread
    //          to service a giopStream managed by giop_1_2_buffered_Impl
    //          until all the fragments of the request has been received.)
    //      3. A valid giop 1.2 header is present in the input stream.
    //      4. If this is not the 1st fragment, the byte order must
    //         match that of the first fragment.

    unsigned char* hdr;
    CORBA::Boolean bswap;

    getHeader12(g,0,hdr,bswap);

    if (hdr[7] != (unsigned char) GIOP::Fragment)
      g->pd_unmarshal_byte_swap = bswap;


    CORBA::ULong fragsz = *(CORBA::ULong*)(hdr + 8);
    if (g->pd_unmarshal_byte_swap) {
      CORBA::ULong fsz = fragsz;
      fragsz = ((((fsz) & 0xff000000) >> 24) |
		(((fsz) & 0x00ff0000) >> 8)  |
		(((fsz) & 0x0000ff00) << 8)  |
		(((fsz) & 0x000000ff) << 24));
    }

    if (hdr[5] >= 1) {
      g->pd_input_fragmented = ((hdr[6] & 0x2) ? 1 : 0);
    }

#ifdef PARANOID
    // Check the invariants if we are paraniod
    OMNIORB_ASSERT(hdr[0] == 'G' && hdr[1] == 'I' && hdr[2] == 'O' &&
	   hdr[3] == 'P' && hdr[4] == 1   && hdr[5] == 2);
    OMNIORB_ASSERT(hdr[7] <= (unsigned char) GIOP::Fragment);
    OMNIORB_ASSERT(g->pd_unmarshal_byte_swap == bswap);
#endif


    // XXX Here we should check if the sum of all fragments has
    //     exceeded the input message size limit. If that is the
    //     case, we do not buffer the data. Instead, we just
    //     allocate an internalBuffer to store the header and
    //     skip the data from the strand. When this stream is
    //     unmarshalled, it would throw a MARSHAL exception when
    //     it reach this internalBuffer.
    //
    //     For the moment, we allocate space to store the data.
    //     This is correct semantics but open up the ORB to abuse.
    //

    internalBuffer* bp = (internalBuffer*)
                           (new char[sizeof(internalBuffer)+fragsz]);

    bp->bodybegin   = 0;
    bp->size        = fragsz + 12;
    bp->next        = 0;

    internalBuffer** bpp = (internalBuffer**)&g->pd_input_buffers;
    while (*bpp) bpp = &((*bpp)->next);
    *bpp = bp;

    void* p = (void*)omni::align_to((omni::ptr_arith_t)&(bp->data),
				    omni::ALIGN_8);
    memcpy(p,(void*)hdr,12);
    g->pd_inb_mkr = g->pd_inb_end;
    {
      Strand::sbuf bb;
      bb.buffer = (void*)((omni::ptr_arith_t)p + 12);
      bb.size = fragsz;
      g->pd_strand->receive_and_copy(bb);
    }
  }

protected:

  GIOP::MsgType inputFragmentBegin(giopStream* g,CORBA::Boolean giop12only,
				   CORBA::Boolean headerInBuffer,
				   CORBA::Boolean expectFragment,
				   CORBA::Boolean expectReply)
  {
  again:
    internalBuffer* p = (internalBuffer*) g->pd_input_current_buffer;
    unsigned char* hdr;
    size_t avail;

    if (!p)
      p = (internalBuffer*) g->pd_input_buffers;
    else {
      p = p->next;
    }

    if (p) {
      OMNIORB_ASSERT(headerInBuffer == 0);
      g->pd_input_current_buffer = p;
      g->pd_inb_begin = (void*)omni::align_to((omni::ptr_arith_t)&(p->data),
					    omni::ALIGN_8);
      g->pd_inb_mkr = g->pd_inb_begin;
      g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_inb_mkr + p->size);
      hdr = (unsigned char*) g->pd_inb_mkr;

      if (!expectFragment) {
	g->pd_unmarshal_byte_swap = (((hdr[6]&0x1)==_OMNIORB_HOST_BYTE_ORDER_)
				     ? 0 : 1 );
      }
      else {
	g->pd_inb_mkr = (void*)((omni::ptr_arith_t)g->pd_inb_mkr + 16);
	if (hdr[7] == (unsigned char) GIOP::CancelRequest) 
	  throw GIOP_S::terminateProcessing();
      }

      g->pd_input_msgfrag_to_come = 0;
      
      if (hdr[5] >= 1) {
	g->pd_input_fragmented = ((hdr[6] & 0x2) ? 1 : 0);
	if (g->pd_input_fragmented) {
	  // Check the size of this fragment is in multiple of 8 bytes.
	  // If not, it is protocol violation.
	  if ((p->size & 7)) {
	    PTRACE("inputFragmentBegin(m)","MessageError (fragment size is not multiple of 8)");
	    goto bail_out;
	  }
	}
      }

      if ((g->pd_input_msgbody_received + p->size) > 
	  GIOP_Basetypes::max_giop_message_size) {

	// We may choose not to send MessageError but  instead just
	// throw MARSHAL exception.
	PTRACE("inputFragmentBegin(m)","MessageError(message size exceeded limit)");
	g->SendMsgErrorMessage();
	g->setTerminalError();
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      }

      return (GIOP::MsgType) hdr[7];
    }

    // Reach here if we have to fetch a new fragment from the strand.
    inputCheckFragmentHeader(g,giop12only,headerInBuffer,expectFragment,
			     expectReply);

    // Returns here only if there is a fragment that is part of
    // the current message.

    avail = (omni::ptr_arith_t) g->pd_inb_end -
            (omni::ptr_arith_t) g->pd_inb_mkr;
    g->pd_strand->giveback_received(avail);
    g->pd_inb_end = g->pd_inb_mkr;

    fetchFragment(g);

    goto again;

  bail_out:
    if (hdr[7] != (unsigned char) GIOP::MessageError &&
	hdr[7] != (unsigned char) GIOP::CloseConnection) {
      g->SendMsgErrorMessage();
    }
    g->setTerminalError();
    g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
    return (GIOP::MsgType) 0; // Dummy return
  }

public:

  //////////////////////////////////////////////////////////////////
  void inputMessageEnd(giopStream* g, CORBA::Boolean disgard,
		       CORBA::Boolean error)
  {
    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
      (omni::ptr_arith_t) g->pd_inb_mkr;

    while (!avail && g->pd_input_fragmented && !error) {
      // There are more fragments to come. We fully expect the fragments to
      // come are empty. If not it is an error. We iterate here to read
      // all the empty fragments and stop either the last one is read or
      // a non-empty fragment is read.
      g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
				       (omni::ptr_arith_t)g->pd_inb_begin);
      (void) inputFragmentBegin(g,1,0,1,0);
      avail = (omni::ptr_arith_t) g->pd_inb_end - 
	      (omni::ptr_arith_t) g->pd_inb_mkr;
    }

    if (error || (g->pd_input_fragmented && ! disgard)) {
      // Either the caller have identified a comm error or
      // There are more fragments to come and the message does not
      // end here.
      if (error) 
	PTRACE("inputMessageEnd(m)","MessageError (detected by stub)");
      else
	PTRACE("inputMessageEnd(m)","MessageError (unexpected fragments left)");
      SendMsgErrorMessage(g);
      g->setTerminalError();
      g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
      // never reach here.
    }

    if (!disgard && avail) {
      // XXX worry about straight conformance here.
      PTRACE("inputMessageEnd(m)","garbage left at the end of message");
      if (omniORB::strictIIOP) {
	SendMsgErrorMessage(g);
	g->setTerminalError();
	g->pd_strand->raiseException(0,CORBA::COMPLETED_NO);
	// never reach here.
      }
    }
    else {
      while (g->pd_input_fragmented) {
	g->pd_input_msgbody_received += ((omni::ptr_arith_t)g->pd_inb_mkr -
					 (omni::ptr_arith_t)g->pd_inb_begin);
	(void) inputFragmentBegin(g,1,0,1,0);
      }
    }

    internalBuffer* bp = (internalBuffer*) g->pd_input_buffers;
    while (bp) {
      char* p = (char*) bp;
      bp = bp->next;
      delete [] p;
    }
    g->pd_input_buffers = g->pd_input_current_buffer = g->pd_input_hdr_end= 0; 
    g->pd_inb_end = g->pd_inb_mkr;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //          Member functions to perform fast copy
  //
  ///////////////////////////////////////////////////////////////////
  CORBA::Boolean startSavingInputMessageBody(giopStream* g)
  {
    if (!g->pd_input_hdr_end) {
      internalBuffer* bp = (internalBuffer*) g->pd_input_current_buffer;
      bp->bodybegin = 1;
      g->pd_input_hdr_end = g->pd_inb_mkr;
    }
    else {
      internalBuffer* bp = (internalBuffer*) g->pd_input_buffers;
      g->pd_input_msgbody_received = 0;
      while (bp && !bp->bodybegin) {
	g->pd_input_msgbody_received += bp->size;
	bp = bp->next;
      }
      OMNIORB_ASSERT(bp);
      g->pd_inb_begin = (void*)omni::align_to((omni::ptr_arith_t)&(bp->data),
					    omni::ALIGN_8);
      g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_inb_begin + bp->size);
      g->pd_inb_mkr = g->pd_input_hdr_end;
    }
    return 1;
  }

  void copyMessageBodyFrom(giopStream*g, giopStream& s)
  {
    if (!s.pd_input_buffers) {
#ifdef PARANOID
      OMNIORB_ASSERT(s.pd_input_fragmented == 0);
      OMNIORB_ASSERT(s.pd_input_hdr_end != 0);
#endif
      size_t size = ((omni::ptr_arith_t)s.pd_inb_end - 
		     (omni::ptr_arith_t)s.pd_input_hdr_end);
      g->put_char_array((CORBA::Char*)s.pd_input_hdr_end,
			(int)size,omni::ALIGN_8);
      s.pd_inb_mkr = s.pd_inb_end;
    }
    else {
#ifdef PARANOID
      OMNIORB_ASSERT(s.pd_input_hdr_end != 0);
#endif

      // Make sure that all the fragments of this message is received
      // before copying.
      {
	startSavingInputMessageBody(&s);
	while (s.pd_input_fragmented) (void) inputFragmentBegin(&s,1,0,1,0);
      }

      internalBuffer* bp = (internalBuffer*) s.pd_input_buffers;
      while (bp && !bp->bodybegin) bp = bp->next;
      OMNIORB_ASSERT(bp);

      s.pd_input_current_buffer = bp;
      s.pd_inb_begin = (void*)omni::align_to((omni::ptr_arith_t)&(bp->data),
					     omni::ALIGN_8);
      s.pd_inb_end = (void*)((omni::ptr_arith_t)s.pd_inb_begin + bp->size);
      s.pd_inb_mkr = s.pd_input_hdr_end;
      size_t size = ((omni::ptr_arith_t)s.pd_inb_end - 
		     (omni::ptr_arith_t)s.pd_inb_mkr);
      g->put_char_array((CORBA::Char*)s.pd_inb_mkr,
			(int)size,omni::ALIGN_8);
      s.pd_inb_mkr = s.pd_inb_end;
      while ((bp = bp->next)) {
	s.pd_input_current_buffer = bp;
	s.pd_inb_begin = (void*)omni::align_to((omni::ptr_arith_t)&(bp->data),
					       omni::ALIGN_8);
	s.pd_inb_end = (void*)((omni::ptr_arith_t)s.pd_inb_begin + bp->size);

	s.pd_inb_mkr = (void*)((omni::ptr_arith_t)s.pd_inb_begin + 16);
	size -= bp->size - 16;
	g->put_char_array((CORBA::Char*)s.pd_inb_mkr,(int)size,omni::ALIGN_8);
	s.pd_inb_mkr = s.pd_inb_end;
      }
      s.pd_input_fragmented = 0;
    }
  }

  void convertToBufferedInput(giopStream* g)
  {
    size_t avail = (omni::ptr_arith_t) g->pd_inb_end - 
      (omni::ptr_arith_t) g->pd_inb_mkr;
    g->pd_strand->giveback_received(avail);
    g->pd_input_msgfrag_to_come += avail;
    g->pd_inb_end = g->pd_inb_mkr;

    OMNIORB_ASSERT(((omni::ptr_arith_t)g->pd_inb_end & 0x7)==0);

    size_t fragsz = g->pd_input_msgfrag_to_come;
    g->pd_input_msgfrag_to_come = 0;
    internalBuffer* bp = (internalBuffer*)
                           (new char[sizeof(internalBuffer)+fragsz]);
    bp->bodybegin   = 0;
    bp->size        = fragsz;
    bp->next        = 0;
    g->pd_input_buffers = g->pd_input_current_buffer = (void*) bp;

    g->pd_impl = this;

    void* p = (void*)omni::align_to((omni::ptr_arith_t)&(bp->data),
				    omni::ALIGN_8);

    {
      Strand::sbuf bb;
      bb.buffer = p;
      bb.size = fragsz;
      g->pd_strand->receive_and_copy(bb);
    }

    g->pd_inb_begin = g->pd_inb_mkr = p;
    g->pd_inb_end = (void*)((omni::ptr_arith_t)g->pd_inb_mkr + fragsz);
  }
};

CORBA::Boolean
giop_1_2_Impl::startSavingInputMessageBody(giopStream* g)
{
  // XXX If the whole message can fit into the buffer returned by
  //     the strand, we can optimise away the creation of a
  //     buffered stream. However, giveback_received() should never
  //     be called in copyInputData(), skipInputData() and getInputData().
  //     As this is not the case at the moment, we always convert the
  //     current stream into a buffered one.

  giop_1_2_buffered_singleton->convertToBufferedInput(g);
  return g->startSavingInputMessageBody();
}

void
giop_1_2_Impl::copyMessageBodyFrom(giopStream*g, giopStream& s)
{
    // We use the giop_1_2_buffered_Impl to do the copy because
    // it can deal with both a buffered and non-buffered source.
    giop_1_2_buffered_singleton->copyMessageBodyFrom(g,s);
}


CORBA::Boolean 
giop_1_2_Impl::saveInterleaveFragment(giopStream* g, CORBA::ULong reqid,
				      GIOP::MsgType msgtype)
{
  {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << " giop 1.2 saveInterleaveFragment " 
	  << reqid << "\n";
    }
  }

  CORBA::Boolean isnew;

  giopStream* p = giopStream::findOrCreateBuffered(g->pd_strand,reqid,isnew);

  if (isnew) {
    p->pd_impl = giop_1_2_buffered_singleton;
  }
  else {
    if (p->pd_impl != giop_1_2_buffered_singleton) {
      p->pd_impl = giop_1_2_buffered_singleton;
    }
  }

  unsigned char* hdr = (unsigned char*) g->pd_inb_mkr;
  CORBA::Boolean delete_spurious = 0;

  switch ((GIOP::MsgType)hdr[7]) {
  case GIOP::Fragment:
    if (isnew) {
      // This fragment is part of fragmented request/reply message and we
      // have not got the 1st fragment. We can safely drop
      // the message.
      PTRACE("saveInterleaveFragment",
	     "Unexpected interleave fragment dropped");
      delete_spurious = 1;
    }
    else if (p->pd_state == giopStream::InputFullyBuffered) {
      // On the wire protocol error. We already have a complete request
      // message with the same request id in the buffer.
      PTRACE("saveInterleaveFragment",
	     "MessageError (unexpected interleave fragment)");
      return 0;
    }
    break;

  case GIOP::CancelRequest:
    break;

  case GIOP::Reply:
  case GIOP::LocateReply:
    {
      if (isnew) {
	// This is a reply and we should have a matching giopStream already
	// on the queue. Since a new giopStream has been created, this could
	// be a reply to an old request or the remote end is confused.
	// Either way, we can safely drop the message.
	PTRACE("saveInterleaveFragment",
	       "Unexpected reply dropped");
	delete_spurious = 1;
      }
      break;
    }

  default:

    if (!isnew) {
      // On the wire protocol error, this is the 1st message but
      // we already have another one with the same request id in the
      // buffer.
      PTRACE("saveInterleaveFragment",
	     "MessageError (message reuse the same request ID)");
      return 0;
    }
    break;
  }

  CORBA::Boolean fragmented = ((hdr[6] & 0x2) ? 1 : 0);
  
  g->pd_strand->giveback_received((omni::ptr_arith_t)g->pd_inb_end -
				  (omni::ptr_arith_t)g->pd_inb_mkr);
  g->pd_inb_end = g->pd_inb_mkr;

  giop_1_2_buffered_singleton->fetchFragment(p);

  if (!delete_spurious) {
    if (!fragmented) {
      PTRACE("saveInterleaveFragment",
	     "end of fragment received");
      p->changeToFullyBuffered();
    }
  }
  else {
    p->deleteThis();
  }
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_giopStream12_initialiser : public omniInitialiser {
public:

  void attach() {
    if (!giop_1_2_singleton) {
      giop_1_2_singleton = new giop_1_2_Impl();
    }
    if (!giop_1_2_buffered_singleton) {
      giop_1_2_buffered_singleton = new giop_1_2_buffered_Impl();
    }
  }

  void detach() { }
};

static omni_giopStream12_initialiser initialiser;

omniInitialiser& omni_giopStream12_initialiser_ = initialiser;
