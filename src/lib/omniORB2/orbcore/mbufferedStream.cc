// -*- Mode: C++; -*-
//                            Package   : omniORB2
// mbufferedStream.cc         Created on: 6/3/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.3  1997/04/23 11:12:34  sll
  New member skip().

// Revision 1.2  1997/03/10  11:50:57  sll
// Minor changes to accomodate the creation of a public API for omniORB2.
//
  Revision 1.1  1997/01/08 17:26:01  sll
  Initial revision

  */

#include <omniORB2/CORBA.h>

const int MemBufferedStream::pd_inline_buf_size = MEMBUFFEREDSTREAM_INLINE_BUF_SIZE;



MemBufferedStream::MemBufferedStream(size_t initialBufsize) {
  pd_bufp = NULL;     // default is to use the in-line buffer
  pd_bufend = (void *) (pd_buffer + pd_inline_buf_size);
  rewind_inout_mkr();
  if (initialBufsize > size()) {
    initialBufsize -= size();
    grow(initialBufsize);
  }
  pd_byte_order = omni::myByteOrder;
  return;
}


MemBufferedStream::MemBufferedStream(const MemBufferedStream& m) {
  size_t bsize = (omni::ptr_arith_t)m.pd_out_mkr - (omni::ptr_arith_t)m.startofstream();

  pd_bufp = NULL;     // default is to use the in-line buffer
  pd_bufend = (void *) (pd_buffer + pd_inline_buf_size);
  rewind_inout_mkr();
  if (bsize > size()) {
    bsize -= size();
    grow(bsize);
  }
  copy(m);
  pd_byte_order = m.byteOrder();
  return;
}

MemBufferedStream &
MemBufferedStream::operator= (const MemBufferedStream & m) {
  // Determine whether we have sufficent buffer space to store the original
  // buffer stream.
  size_t bsize = (omni::ptr_arith_t)m.pd_out_mkr - (omni::ptr_arith_t)m.startofstream();
  rewind_inout_mkr();
  if (bsize > size()) {
    bsize -= size();
    grow(bsize);
  }
  copy(m);
  pd_byte_order = m.byteOrder();
  return *this;
}


MemBufferedStream::~MemBufferedStream() {
  if (pd_bufp) {
    delete [] (char *)pd_bufp;
  }
  return;
}

void
MemBufferedStream::grow(size_t minimum) {
  size_t newsize = size() + minimum + (size_t) omni::ALIGN_8;
  if (newsize < 1024) {
    // Pick the closest 2^N bytes
    size_t v = (1 << 9);  // start from 2 ^ 9 = 512
    while (newsize < v) {
      v = (v >> 1);
    }
    newsize = (v << 1);
  }
  else {
    // Pick the closest N Kbytes
    newsize = (newsize + 1024 - 1) & ~(1024 - 1);
  }
  void * oldbufp = pd_bufp;
  void * oldstartofstream = startofstream();
  size_t copysize = (omni::ptr_arith_t)pd_out_mkr - (omni::ptr_arith_t)startofstream();
  void * old_in_mkr = pd_in_mkr;
  void * old_out_mkr = pd_out_mkr;
  pd_bufp = (void *)(new char [newsize]);
  pd_bufend = (void *)((omni::ptr_arith_t) pd_bufp + newsize);
  rewind_inout_mkr();
  if (copysize) {
    memcpy(startofstream(),oldstartofstream,copysize);
  }
  pd_in_mkr  = (void *) ((omni::ptr_arith_t) pd_in_mkr + 
			 ((omni::ptr_arith_t) old_in_mkr - 
			  (omni::ptr_arith_t) oldstartofstream));
  pd_out_mkr = (void *) ((omni::ptr_arith_t) pd_out_mkr + 
			 ((omni::ptr_arith_t) old_out_mkr - 
			  (omni::ptr_arith_t) oldstartofstream));
  if (oldbufp) {
    delete [] (char *)oldbufp;
  }
}

void
MemBufferedStream::rewind_inout_mkr() {
  rewind_in_mkr();
  pd_out_mkr = pd_in_mkr;
  return;
}

void
MemBufferedStream::rewind_in_mkr() {
  pd_in_mkr = startofstream();
  return;
}

void
MemBufferedStream::put_char_array(const CORBA::Char * src,int sz) {
  void *dst = align_and_put_bytes(omni::ALIGN_1,sz);
  memcpy(dst,src,sz);
  return;
}


void
MemBufferedStream::get_char_array(CORBA::Char * dst,int sz) {
  void *src = align_and_get_bytes(omni::ALIGN_1,sz);
  memcpy(dst,src,sz);
  return;
}

void *
MemBufferedStream::startofstream() const {
  omni::ptr_arith_t p;
  // The start of the buffer stream is 8 bytes aligned.
  p = (pd_bufp) ? (omni::ptr_arith_t) pd_bufp : (omni::ptr_arith_t) pd_buffer;
  return (void *)omni::align_to(p,omni::ALIGN_8);
}

size_t
MemBufferedStream::size() {
  return (omni::ptr_arith_t) pd_bufend - (omni::ptr_arith_t) startofstream();
}

void
MemBufferedStream::copy(const MemBufferedStream &m) {
  // The new buffer may be on a different alignment. Hence we don't
  // just copy from the beginning of the buffer. Copy from the beginning
  // of the buffer stream instead.
  rewind_inout_mkr();
  memcpy(startofstream(),m.startofstream(),
	 (omni::ptr_arith_t)m.pd_out_mkr - (omni::ptr_arith_t)m.startofstream());
  pd_in_mkr = (void *) ((omni::ptr_arith_t) pd_in_mkr + 
			((omni::ptr_arith_t) m.pd_in_mkr - 
			 (omni::ptr_arith_t) m.startofstream()));
  pd_out_mkr = (void *) ((omni::ptr_arith_t) pd_out_mkr + 
			 ((omni::ptr_arith_t) m.pd_out_mkr - 
			  (omni::ptr_arith_t) m.startofstream()));
  return;
}

void
MemBufferedStream::skip(CORBA::ULong size)
{
  align_and_get_bytes(omni::ALIGN_1,size);
  return;
}

void *
MemBufferedStream::overrun_error() {
  throw omniORB::fatalException(__FILE__,__LINE__,
     "MemBufferedStream::overrun_error()");
  // never reach here
  return 0;
}
