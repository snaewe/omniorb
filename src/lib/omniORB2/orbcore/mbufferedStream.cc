// -*- Mode: C++; -*-
//                            Package   : omniORB2
// mbufferedStream.cc         Created on: 6/3/96
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
  Revision 1.6  1998/08/26 11:11:55  sll
  Minor update to remove warnings when compiled with standard C++ compiler.

  Revision 1.5  1998/01/27 15:34:47  ewc
  Added support for TypeCode and type any

// Revision 1.4  1997/05/06  15:24:09  sll
// Public release.
//
  */

#include <omniORB2/CORBA.h>

const int MemBufferedStream::pd_inline_buf_size = MEMBUFFEREDSTREAM_INLINE_BUF_SIZE;



MemBufferedStream::MemBufferedStream(size_t initialBufsize) {
  pd_bufp = NULL;     // default is to use the in-line buffer
  pd_dupl = 0;
  pd_noboundcheck = 0;
  pd_bufend = (void *) (pd_buffer + pd_inline_buf_size);
  rewind_inout_mkr();
  if (initialBufsize > size()) {
    initialBufsize -= size();
    grow(initialBufsize);
  }
  pd_byte_order = omni::myByteOrder;
  return;
}


MemBufferedStream::MemBufferedStream(const MemBufferedStream& m, 
				        _CORBA_Boolean dupl) {
  size_t bsize = (omni::ptr_arith_t)m.pd_out_mkr - (omni::ptr_arith_t)m.startofstream();
  
  if (dupl == 0) {
    pd_bufp = NULL;     // default is to use the in-line buffer
    pd_noboundcheck = 0;
    pd_dupl = 0;
    pd_bufend = (void *) (pd_buffer + pd_inline_buf_size);
    rewind_inout_mkr();
    if (bsize > size()) {
      bsize -= size();
      grow(bsize);
    }
    copy(m);
    pd_byte_order = m.byteOrder();
  }
  else {
    pd_bufp = m.pd_bufp;
    if (!pd_bufp) pd_bufp = m.startofstream();

    pd_bufend = m.pd_bufend;
    pd_out_mkr = m.pd_out_mkr;
    
    pd_in_mkr = startofstream();
    pd_byte_order = m.pd_byte_order;
    pd_noboundcheck = 0;
    pd_dupl = 1;
  }
}


MemBufferedStream::MemBufferedStream(_CORBA_Char* data) {
  pd_bufp = NULL;
  pd_bufend = (void*) (pd_buffer + pd_inline_buf_size);
  rewind_inout_mkr();

  
  pd_bufp = data;
  pd_bufend = 0;
  pd_out_mkr = 0;

  pd_in_mkr = pd_bufp;
  pd_byte_order = omni::myByteOrder;
  pd_noboundcheck = 1;
  pd_dupl = 1;
}


MemBufferedStream &
MemBufferedStream::operator= (const MemBufferedStream & m) {
  // Determine whether we have sufficent buffer space to store the original
  // buffer stream.
  if (pd_dupl || pd_noboundcheck) 
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "MemBufferedStream::operator=()");

  size_t bsize = (omni::ptr_arith_t)m.pd_out_mkr - (omni::ptr_arith_t)m.startofstream();
  pd_noboundcheck = 0;
  pd_dupl = 0;
  rewind_inout_mkr();
  if (bsize > size()) {
    bsize -= size();
    grow(bsize);
  }
  copy(m);
  pd_byte_order = m.byteOrder();
  return *this;
}


void MemBufferedStream::shallowCopy(const MemBufferedStream& m) {
  if (pd_noboundcheck)
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "MemBufferedStream::shallowCOpy");
  if (!pd_dupl) reset();

  pd_bufp = m.pd_bufp;  
  if (!pd_bufp) pd_bufp = m.startofstream();
  pd_bufend = m.pd_bufend;
  pd_out_mkr = m.pd_out_mkr;
  
  pd_in_mkr = startofstream();
  pd_byte_order = m.pd_byte_order;
  pd_noboundcheck = 0;
  pd_dupl = 1;
}


MemBufferedStream::~MemBufferedStream() {
  if (pd_bufp && !pd_dupl && !pd_noboundcheck) {
    delete [] (char *)pd_bufp;
  }
  return;
}


void
MemBufferedStream::grow(size_t minimum) {
  if (pd_dupl || pd_noboundcheck) 
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "MemBufferedStream::grow()");

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
MemBufferedStream::size() const {
  return (omni::ptr_arith_t) pd_bufend - (omni::ptr_arith_t) startofstream();
}

void
MemBufferedStream::copy(const MemBufferedStream &m) {
  // The new buffer may be on a different alignment. Hence we don't
  // just copy from the beginning of the buffer. Copy from the beginning
  // of the buffer stream instead.
  if (pd_dupl || pd_noboundcheck) 
    throw omniORB::fatalException(__FILE__,__LINE__, 
				  "MemBufferedStream::copy()");
  
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
MemBufferedStream::reset() {
  if (pd_dupl || pd_noboundcheck) 
    throw omniORB::fatalException(__FILE__,__LINE__, 
				    "MemBufferedStream::reset()");
  if (pd_bufp) {
    delete [] (char *)pd_bufp;
    pd_bufp = NULL;
  }

  pd_bufend = (void *) (pd_buffer + pd_inline_buf_size);
  rewind_inout_mkr();

  pd_byte_order = omni::myByteOrder;
}

void
MemBufferedStream::skip(CORBA::ULong size,omni::alignment_t align)
{
  align_and_get_bytes(align,size);
  return;
}

void *
MemBufferedStream::overrun_error() {
  throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}
