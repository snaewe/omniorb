// -*- Mode: C++; -*-
//                            Package   : omniORB
// mbufferedStream.cc         Created on: 6/3/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
  Revision 1.12  2001/02/21 14:12:12  dpg1
  Merge from omni3_develop for 3.0.3 release.

  Revision 1.9.6.3  2000/06/22 10:40:15  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.9.6.2  1999/10/14 16:22:12  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.9.6.1  1999/09/22 14:26:54  djr
  Major rewrite of orbcore to support POA.

  Revision 1.9  1999/05/10 16:36:04  djr
  Fixed bug in constructors for read-only streams.

  Revision 1.8  1999/03/11 16:25:54  djr
  Updated copyright notice

  Revision 1.7  1999/01/07 16:04:39  djr
  Interface changed slightly to agree more closely with NetBufferedStream.
  Support for read-only streams with external buffer improved.

  Revision 1.6  1998/08/26 11:11:55  sll
  Minor update to remove warnings when compiled with standard C++ compiler.

  Revision 1.5  1998/01/27 15:34:47  ewc
  Added support for TypeCode and type any

// Revision 1.4  1997/05/06  15:24:09  sll
// Public release.
//
  */

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <exceptiondefs.h>
#include <limits.h>


const int MemBufferedStream::pd_inline_buf_size =
                                MEMBUFFEREDSTREAM_INLINE_BUF_SIZE;


MemBufferedStream::MemBufferedStream(size_t initialBufsize) {
  pd_external_buffer = 0;
  pd_bufp = NULL;     // default is to use the in-line buffer
  pd_bufend = (void *) (pd_buffer + pd_inline_buf_size);
  rewind_inout_mkr();
  if (initialBufsize > size()) {
    initialBufsize -= size();
    grow(initialBufsize);
  }
  pd_byte_order = omni::myByteOrder;
}


MemBufferedStream::MemBufferedStream(const MemBufferedStream& m)
{
  size_t bsize = (omni::ptr_arith_t)m.pd_out_mkr -
                 (omni::ptr_arith_t)m.startofstream();

  if( m.pd_external_buffer ){
    // For an external buffer the storage is managed elsewhere. We assume
    // that it will continue to exist for the lifetime of this buffered
    // stream also - so just copy the pointer.
    pd_bufp = m.pd_bufp;
    pd_bufend = m.pd_bufend;
    pd_external_buffer = 1;
    rewind_inout_mkr();
  }else{
    pd_bufp = NULL;     // default is to use the in-line buffer
    pd_bufend = (void *) (pd_buffer + pd_inline_buf_size);
    pd_external_buffer = 0;

    rewind_inout_mkr();
    if (bsize > size()) {
      bsize -= size();
      grow(bsize);
    }
    copy(m);
  }

  pd_byte_order = m.byteOrder();
}


MemBufferedStream&
MemBufferedStream::operator= (const MemBufferedStream& m)
{
  if (m.pd_external_buffer){
    // For an external buffer the storage is managed elsewhere. We assume
    // that it will continue to exist for the lifetime of this buffered
    // stream also - so just copy the pointer.

    // Delete the old data buffer, if necessary
    if( pd_bufp && !pd_external_buffer )
      delete[] (char*)pd_bufp;

    pd_bufp = m.pd_bufp;
    pd_bufend = m.pd_bufend;
    pd_external_buffer = 1;
    rewind_inout_mkr();
  }else{
    if( pd_external_buffer ){
      pd_bufp = NULL;     // default is to use the in-line buffer
      pd_bufend = (void*)(pd_buffer + pd_inline_buf_size);
      pd_external_buffer = 0;
    }
    // Determine whether we have sufficent buffer space to store the original
    // without further allocation.
    size_t bsize = (omni::ptr_arith_t)m.pd_out_mkr -
      (omni::ptr_arith_t)m.startofstream();

    // Now rewind the in & out pointers & copy across the data
    rewind_inout_mkr();
    if (bsize > size()) {
      bsize -= size();
      grow(bsize);
    }
    copy(m);
  }

  pd_byte_order = m.byteOrder();
  return *this;
}


MemBufferedStream::~MemBufferedStream() {
  if( pd_bufp && !pd_external_buffer )
    delete[] (char*)pd_bufp;
}


MemBufferedStream::MemBufferedStream(void* databuffer) {
  // Create a read-only MemBufferedStream, which reads from an
  // externally-managed buffer
  pd_external_buffer = 1;
  pd_byte_order = omni::myByteOrder;
  pd_bufp = databuffer;
#if (SIZEOF_LONG == SIZEOF_PTR)
  pd_bufend = (void *) ULONG_MAX;
#elif (SIZEOF_INT == SIZEOF_PTR)
  pd_bufend = (void *) UINT_MAX;
#else
#error "No suitable integer type available to calculate maximum" \
  " pointer value from"
#endif
  pd_in_mkr = startofstream();
  pd_out_mkr = pd_bufend;
}


MemBufferedStream::MemBufferedStream(void* databuffer, size_t maxLen)
{
  // Create a read-only MemBufferedStream, which reads from an
  // externally-managed buffer and has a limited length
  pd_external_buffer = 1;
  pd_byte_order = omni::myByteOrder;
  pd_bufp = databuffer;
  pd_bufend = (void *)((omni::ptr_arith_t)pd_bufp + (omni::ptr_arith_t)maxLen);
  pd_in_mkr = startofstream();
  pd_out_mkr = pd_bufend;
}


void
MemBufferedStream::grow(size_t minimum)
{
  if (pd_external_buffer)
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "MemBufferedStream::grow() - called for read-only stream");

  size_t newsize = size() + minimum + (size_t) omni::ALIGN_8;
  if( newsize < 1024 ) {
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
  void* oldbufp = pd_bufp;
  void* oldstartofstream = startofstream();
  size_t copysize = (omni::ptr_arith_t)pd_out_mkr -
    (omni::ptr_arith_t)startofstream();
  void* old_in_mkr = pd_in_mkr;
  void* old_out_mkr = pd_out_mkr;
  pd_bufp = new char [newsize];
  pd_bufend = (void*)((omni::ptr_arith_t) pd_bufp + newsize);
  rewind_inout_mkr();
  if (copysize) {
    memcpy(startofstream(), oldstartofstream, copysize);
  }
  pd_in_mkr  = (void*) ((omni::ptr_arith_t) pd_in_mkr + 
			((omni::ptr_arith_t) old_in_mkr - 
			 (omni::ptr_arith_t) oldstartofstream));
  pd_out_mkr = (void*) ((omni::ptr_arith_t) pd_out_mkr + 
			((omni::ptr_arith_t) old_out_mkr - 
			 (omni::ptr_arith_t) oldstartofstream));
  if( oldbufp )
    delete[] (char*)oldbufp;
}


void
MemBufferedStream::put_char_array(const CORBA::Char* src, int size,
				  omni::alignment_t align)
{
  void *dst = align_and_put_bytes(align, size);
  memcpy(dst, src, size);
}


void
MemBufferedStream::get_char_array(CORBA::Char* dst,int size,
				  omni::alignment_t align)
{
  void *src = align_and_get_bytes(align, size);
  memcpy(dst, src, size);
}


void
MemBufferedStream::copy_from(MemBufferedStream& from, size_t size,
			     omni::alignment_t align)
{
  void* src = from.align_and_get_bytes(align, size);
  void* dst = align_and_put_bytes(align, size);
  memcpy(dst, src, size);
}


void
MemBufferedStream::copy_from(NetBufferedStream& from, size_t size,
			     omni::alignment_t align)
{
  void* dst = align_and_put_bytes(align, size);
  from.get_char_array((CORBA::Char*)dst, size, align);
}


void*
MemBufferedStream::startofstream() const {
  omni::ptr_arith_t p;
  // The start of the buffer stream is 8 bytes aligned.
  p = (pd_bufp) ? (omni::ptr_arith_t) pd_bufp : (omni::ptr_arith_t) pd_buffer;
  return (void *)omni::align_to(p,omni::ALIGN_8);
}


void
MemBufferedStream::copy(const MemBufferedStream &m) {

  // Copy should never be called on a read-only stream.
  if( pd_external_buffer )
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "MemBufferedStream::copy() - called for read-only stream");

  // The new buffer may be on a different alignment. Hence we don't
  // just copy from the beginning of the buffer. Copy from the beginning
  // of the buffer stream instead.
  rewind_inout_mkr();
  memcpy(startofstream(), m.startofstream(),
	 (omni::ptr_arith_t)m.pd_out_mkr -
	 (omni::ptr_arith_t)m.startofstream());
  pd_in_mkr = (void*) ((omni::ptr_arith_t) pd_in_mkr +
		       ((omni::ptr_arith_t) m.pd_in_mkr -
			(omni::ptr_arith_t) m.startofstream()));
  pd_out_mkr = (void*) ((omni::ptr_arith_t) pd_out_mkr +
			((omni::ptr_arith_t) m.pd_out_mkr -
			 (omni::ptr_arith_t) m.startofstream()));
}


void
MemBufferedStream::skip(CORBA::ULong size)
{
  align_and_get_bytes(omni::ALIGN_1, size);
}


void
MemBufferedStream::overrun_error()
{
  OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
}


void
MemBufferedStream::write_to_readonly_error(const char* file, int line)
{
  throw omniORB::fatalException(file, line,
     "Attempt to write to a readonly MemBufferedStream");
}
