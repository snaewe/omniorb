// -*- Mode: C++; -*-
//                            Package   : omniORB
// relStream.cc               Created on: 18/3/96
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
//	

/*
  $Log$
  Revision 1.3.8.2  1999/09/24 15:01:35  djr
  Added module initialisers, and sll's new scavenger implementation.

  Revision 1.3.8.1  1999/09/22 14:27:05  djr
  Major rewrite of orbcore to support POA.

  Revision 1.2  1998/08/14 13:50:58  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.1  1997/12/09 18:43:13  sll
  Initial revision

  */

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <relStream.h>
#include <limits.h>

#define  DO_NOT_AVOID_MISALIGNMENT    

reliableStreamStrand::reliableStreamStrand(size_t buffer_size,
					   Rope* r, CORBA::Boolean h)
  : Strand(r,h), pd_buffer_size(buffer_size)
{
  pd_tx_buffer = (void *) new char[pd_buffer_size];
  pd_tx_begin  = pd_tx_end = pd_tx_reserved_end = pd_tx_buffer;
  pd_rx_buffer = (void *) new char[pd_buffer_size];
  pd_rx_begin = pd_rx_end = pd_rx_received_end = pd_rx_buffer;
  // Get rid of purify warning?
  memset(pd_tx_buffer, 0, pd_buffer_size);
}


reliableStreamStrand::~reliableStreamStrand()
{
  if (pd_tx_buffer) {
    delete [] (char *)pd_tx_buffer;
    pd_tx_buffer = 0;
  }
  if (pd_rx_buffer) {
    delete [] (char *)pd_rx_buffer;
    pd_rx_buffer = 0;
  }
}


size_t 
reliableStreamStrand::MaxMTU() const {
  // No limit
  return UINT_MAX;
}


Strand::sbuf
reliableStreamStrand::receive(size_t size,
		CORBA::Boolean exactly,
		int align,
		CORBA::Boolean) 
{
  giveback_received(0);

  size_t bsz = ((omni::ptr_arith_t) pd_rx_end - 
    (omni::ptr_arith_t) pd_rx_begin);

  int current_alignment;
  omni::ptr_arith_t new_align_ptr;

  if (!bsz) {
    // No data left in receive buffer, fetch() and try again
    // rewind the buffer pointers to the beginning of the buffer and
    // at the same alignment as it is requested in <align>
    new_align_ptr = omni::align_to((omni::ptr_arith_t) pd_rx_buffer,
				   omni::max_alignment) + align;
    if (new_align_ptr >= ((omni::ptr_arith_t)pd_rx_buffer + 
			  (int)omni::max_alignment)) {
      new_align_ptr -= (int) omni::max_alignment;
    }
    pd_rx_begin = pd_rx_received_end = pd_rx_end = (void *)new_align_ptr;

#ifndef DO_NOT_AVOID_MISALIGNMENT    
    fetch(size);
#else
    fetch();
#endif
    return receive(size,exactly,align);
  }

  if (align > (int)omni::max_alignment) {
    throw CORBA::INTERNAL(0,CORBA::COMPLETED_MAYBE);
  }

  current_alignment = (omni::ptr_arith_t) pd_rx_begin &
    ((int)omni::max_alignment - 1);
  if (current_alignment == 0) {
    current_alignment = (int) omni::max_alignment;
  }

  if (current_alignment != align) {
    // alignment is not right, move the data to the correct alignment
    new_align_ptr = omni::align_to((omni::ptr_arith_t) pd_rx_buffer,
				      omni::max_alignment) + align;
    if (new_align_ptr >= ((omni::ptr_arith_t)pd_rx_buffer + 
			  (int)omni::max_alignment)) {
      new_align_ptr -= (int) omni::max_alignment;
    }
    memmove((void *)new_align_ptr,(void *)pd_rx_begin,bsz);
    pd_rx_begin = pd_rx_received_end = (void *)new_align_ptr;
    pd_rx_end = (void *)(new_align_ptr + bsz);
  }

  if (bsz < size) {
    if (exactly) {
      if (size > max_receive_buffer_size()) {
	throw CORBA::INTERNAL(0,CORBA::COMPLETED_MAYBE);
      }
      // Not enough data to satisfy the request, fetch() and try again
      // Check if there is enough empty space for fetch() to satisfy this
      // request. If necessary, make some space by moving existing data to
      // the beginning of the buffer. Always keep the buffer pointers at
      // the same alignment as they were previously

      
      size_t avail = pd_buffer_size - 
	                ((omni::ptr_arith_t) pd_rx_end - 
			 (omni::ptr_arith_t) pd_rx_buffer) + bsz;
      if (avail < size) {
	// Not enough empty space, got to move existing data
	current_alignment = (omni::ptr_arith_t) pd_rx_begin &
	  ((int)omni::max_alignment - 1);
	if (current_alignment == 0) {
	  current_alignment = (int) omni::max_alignment;
	}
	new_align_ptr = omni::align_to((omni::ptr_arith_t) pd_rx_buffer,
					  omni::max_alignment) + 
	                 current_alignment;
	if (new_align_ptr >= ((omni::ptr_arith_t)pd_rx_buffer + 
			      (int)omni::max_alignment)) {
	  new_align_ptr -= (int) omni::max_alignment;
	}
	memmove((void *)new_align_ptr,pd_rx_begin,bsz);
	pd_rx_begin = pd_rx_received_end = (void *)new_align_ptr;
	pd_rx_end = (void *)(new_align_ptr + bsz);
      }
#ifndef DO_NOT_AVOID_MISALIGNMENT
      fetch(size-bsz);
#else
      fetch();
#endif
      return receive(size,exactly,align);
    }
    else {
      size = bsz;
    }
  }
  pd_rx_received_end = (void *)((omni::ptr_arith_t)pd_rx_begin + size);
  Strand::sbuf result;
  result.buffer = pd_rx_begin;
  result.size   = size;
  return result;
}


void
reliableStreamStrand::receive_and_copy(Strand::sbuf b,CORBA::Boolean)
{
  giveback_received(0);

  size_t sz = b.size;
  char  *p = (char *)b.buffer;

  size_t bsz = ((omni::ptr_arith_t)pd_rx_end - 
		(omni::ptr_arith_t)pd_rx_begin);
  if (bsz) {
    if (bsz > sz) {
      bsz = sz;
    }
    memcpy((void *)p,pd_rx_begin,bsz);
    pd_rx_begin = (void *)((omni::ptr_arith_t) pd_rx_begin + bsz);
    pd_rx_received_end = pd_rx_begin;
    sz -= bsz;
    p += bsz;
  }
  while (sz) {
    int rx;
    rx = ll_recv(p,sz);
    sz -= rx;
    p += rx;
  }
}




void 
reliableStreamStrand::giveback_received(size_t leftover) 
{
  size_t total = (omni::ptr_arith_t)pd_rx_received_end -
    (omni::ptr_arith_t)pd_rx_begin;
  if (total < leftover) {
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  }
  total -= leftover;
  pd_rx_begin = (void *)((omni::ptr_arith_t)pd_rx_begin + total);
  pd_rx_received_end = pd_rx_begin;
  return;
}


void
reliableStreamStrand::fetch(CORBA::ULong max)
{
  size_t bsz = pd_buffer_size -
    ((omni::ptr_arith_t) pd_rx_end - (omni::ptr_arith_t) pd_rx_buffer);

  bsz = (max != 0 && bsz > max) ? max : bsz;

  if (!bsz) return;

  size_t rx = ll_recv(pd_rx_end,bsz);
  pd_rx_end = (void *)((omni::ptr_arith_t) pd_rx_end + rx);
  return;
}

void 
reliableStreamStrand::reserve_and_copy(Strand::sbuf b,
				  CORBA::Boolean transmit,CORBA::Boolean)
{
  // transmit anything that is left in the transmit buffer
  giveback_reserved(0,1);
  
  // Transmit directly from the supplied buffer
  if (b.size) ll_send(b.buffer,b.size);
}

void
reliableStreamStrand::transmit() 
{
  size_t sz = (omni::ptr_arith_t)pd_tx_end - 
              (omni::ptr_arith_t)pd_tx_begin;
  if (sz) ll_send(pd_tx_begin,sz);
  pd_tx_begin = pd_tx_end = pd_tx_reserved_end = pd_tx_buffer;
  return;
}



size_t 
reliableStreamStrand::max_receive_buffer_size() 
{
  return pd_buffer_size - (int)omni::max_alignment;
}


void
reliableStreamStrand::skip(size_t size,CORBA::Boolean)
{
  giveback_received(0);

  while (size) {
    size_t sz;
    sz = max_receive_buffer_size();
    if (sz > size) {
      sz = size;
    }
    int current_alignment = (omni::ptr_arith_t) pd_rx_begin &
      ((int)omni::max_alignment - 1);
    if (current_alignment == 0) {
      current_alignment = (int) omni::max_alignment;
    }
    Strand::sbuf sb = receive(sz,0,current_alignment);
    size -= sb.size;
  }
  return;
}

Strand::sbuf 
reliableStreamStrand::reserve_and_startMTU(size_t size,
				      CORBA::Boolean exactly,
				      int align,
				      CORBA::Boolean tx,
				      CORBA::Boolean) 
{
  return reserve(size, exactly, align, tx, 0);
}

Strand::sbuf 
reliableStreamStrand::reserve(size_t size,
		CORBA::Boolean exactly,
		int align,
		CORBA::Boolean tx,
		CORBA::Boolean) 
{

  giveback_reserved(0,tx);
  
  size_t bsz = pd_buffer_size -
    ((omni::ptr_arith_t) pd_tx_end - (omni::ptr_arith_t) pd_tx_buffer);
  
  if (!bsz) {
    // No space left, transmit and try again
    transmit();
    return reserve(size,exactly,align,tx);
  }

  if (align > (int)omni::max_alignment) {
    throw CORBA::INTERNAL(0,CORBA::COMPLETED_MAYBE);
  }

  int current_alignment = (omni::ptr_arith_t) pd_tx_end & 
    ((int)omni::max_alignment - 1);
  if (current_alignment == 0) {
    current_alignment = (int)omni::max_alignment;
  }

  if (current_alignment != align) {
    // alignment is not right
    if (pd_tx_end == pd_tx_begin) {
      // There is nothing in the buffer, we could adjust
      // pd_tx_begin and pd_tx_end to the required alignment
      omni::ptr_arith_t new_align_ptr;

      new_align_ptr = omni::align_to((omni::ptr_arith_t) pd_tx_buffer,
					omni::max_alignment) + align;
      if (new_align_ptr >= ((omni::ptr_arith_t)pd_tx_buffer + 
			   (int)omni::max_alignment)) {
	new_align_ptr -= (int) omni::max_alignment;
      }
      pd_tx_begin = pd_tx_end = pd_tx_reserved_end = (void *)new_align_ptr;
      bsz = pd_buffer_size - ((omni::ptr_arith_t) pd_tx_end 
		    - (omni::ptr_arith_t) pd_tx_buffer);
    }
    else {
      // transmit what is left and try again
      transmit();
      return reserve(size,exactly,align,tx);
    }
  }

  if (bsz < size) {
    if (exactly) {
      if (size > max_reserve_buffer_size()) {
	throw CORBA::INTERNAL(0,CORBA::COMPLETED_MAYBE);
      }
      // Not enough space to satisfy the request, transmit what is
      // left and try again
      transmit();
      return reserve(size,exactly,align,tx);
    }
    else {
      size = bsz;
    }
  }
  pd_tx_reserved_end = (void *)((omni::ptr_arith_t)pd_tx_end + size);
  Strand::sbuf result;
  result.buffer = pd_tx_end;
  result.size   = size;
  return result;
}


void
reliableStreamStrand::giveback_reserved(size_t leftover,
				   CORBA::Boolean tx,CORBA::Boolean) 
{
  size_t total = (omni::ptr_arith_t)pd_tx_reserved_end -
    (omni::ptr_arith_t)pd_tx_end;
  if (total < leftover) {
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  }
  total -= leftover;
  pd_tx_end = (void *)((omni::ptr_arith_t)pd_tx_end + total);
  pd_tx_reserved_end = pd_tx_end;
  if (tx) {
    transmit();
  }
  return;
}

size_t
reliableStreamStrand::max_reserve_buffer_size() 
{
  return pd_buffer_size - (int)omni::max_alignment;
}

