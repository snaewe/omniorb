// -*- Mode: C++; -*-
//                            Package   : omniORB2
// nbufferedStream.cc         Created on: 14/3/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.3  1997/03/10 11:52:38  sll
  Minor changes to accomodate the creation of a public API for omniORB2.

// Revision 1.2  1997/01/08  18:15:21  ewc
// Added work-around for Windows NT / Visual C++ 4.2 nested class bug
//
// Revision 1.1  1996/10/10  14:37:53  sll
// Initial revision
//
  */

#include <omniORB2/CORBA.h>


NetBufferedStream::NetBufferedStream(Strand *s,
				     CORBA::Boolean Rdlock,
				     CORBA::Boolean Wrlock,
				     size_t Bufsize) 
  : Strand_Sync(s,Rdlock,Wrlock)
{
  if (Bufsize) {
    if (Bufsize > s->max_receive_buffer_size() ||
	Bufsize > s->max_reserve_buffer_size()) 
      {	
	if (Rdlock) RdUnlock();
	if (Wrlock) WrUnlock();
	throw omniORB::fatalException(__FILE__,__LINE__,
	      "NetBufferedStream::NetbufferedStream() cannot allocate the requested buffer size.");
      }
  }
  pd_ideal_buf_size = Bufsize;
  pd_strand = s;
  pd_RdLock = Rdlock;
  pd_WrLock = Wrlock;
  rewind_inb_mkr((int)omni::max_alignment);
  rewind_outb_mkr((int)omni::max_alignment);
  pd_rdmsg_size = pd_wrmsg_size = pd_read = pd_written = 0;
  return;
}


NetBufferedStream::NetBufferedStream(Rope *r,
				     CORBA::Boolean Rdlock,
				     CORBA::Boolean Wrlock,
				     size_t Bufsize) 
  : Strand_Sync(r,Rdlock,Wrlock)
{
  pd_strand = Strand_Sync::get_strand();
  if (Bufsize) 
    {
      if (Bufsize > pd_strand->max_receive_buffer_size() ||
	  Bufsize > pd_strand->max_reserve_buffer_size()) 
	{
	  if (Rdlock) RdUnlock();
	  if (Wrlock) WrUnlock();
	  throw omniORB::fatalException(__FILE__,__LINE__,
	      "NetBufferedStream::NetbufferedStream() cannot allocate the requested buffer size.");
	}
    }
  pd_ideal_buf_size = Bufsize;
  pd_RdLock = Rdlock;
  pd_WrLock = Wrlock;
  rewind_inb_mkr((int)omni::max_alignment);
  rewind_outb_mkr((int)omni::max_alignment);
  pd_rdmsg_size = pd_wrmsg_size = pd_read = pd_written = 0;
  return;
}


NetBufferedStream::~NetBufferedStream() {
  RdUnlock();
  WrUnlock();
  return;
}

void

NetBufferedStream::get_char_array(CORBA::Char *b,int size) {
  Strand::sbuf s;
  if (!size) return;
  if (size > 512) {
    ensure_rdlocked();
    giveback_received();
    if (RdMessageSize()) {
      if (size > (int)RdMessageUnRead()) {
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      }
    }

    s.buffer = (void *)b;
    s.size   = size;
    pd_strand->receive_and_copy(s);
    pd_read += size;
    int newalignment = current_inb_alignment() + size;
    newalignment = newalignment &((int)omni::max_alignment - 1);
    rewind_inb_mkr((newalignment)?newalignment:(int)omni::max_alignment);
  }
  else {
    char * p = (char *)align_and_get_bytes(omni::ALIGN_1,size);
    memcpy((void *)b,p,size);
  }
  return;
}

void

NetBufferedStream::put_char_array(const CORBA::Char *b,int size) {
  Strand::sbuf s;
  if (!size) return;
  if (size > 512) {
    ensure_wrlocked();
    giveback_reserved();
    if (WrMessageSize()) {
      if (size > (int)WrMessageSpaceLeft()) {
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      }
    }
    s.buffer = (void *) b;
    s.size = size;
    pd_strand->reserve_and_copy(s);
    pd_written += size;
    int newalignment = current_outb_alignment() + size;
    newalignment = newalignment &((int)omni::max_alignment - 1);
    rewind_outb_mkr((newalignment)?newalignment:(int)omni::max_alignment);
  }
  else {
    void *p = align_and_put_bytes(omni::ALIGN_1,size);
    memcpy(p,(void *)b,size);
  }
  return;
}


void

NetBufferedStream::flush() {
  giveback_reserved(1);
  return;
}


void

NetBufferedStream::reserve(size_t minimum) {
  Strand::sbuf b;

  ensure_wrlocked();
  giveback_reserved();

  // determine what buffer size to request
  size_t bufsize = WrMessageSpaceLeft();

  if (WrMessageSize())
    {
      if (bufsize < minimum)
	// Error, try to reserve more bytes than the message size limit
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      else 
	if (bufsize > pd_strand->max_reserve_buffer_size())
	  bufsize = pd_strand->max_reserve_buffer_size();
    }
  else
    {
      // Unlimited message size
      bufsize = ((pd_ideal_buf_size) ? pd_ideal_buf_size :
		   pd_strand->max_reserve_buffer_size());
    }

  b = pd_strand->reserve(bufsize,0,current_outb_alignment());
  pd_outb_mkr = b.buffer;
  pd_outb_end = (void *) ((omni::ptr_arith_t)pd_outb_mkr + b.size);
  pd_written += b.size;

  if (b.size < minimum) {
    // The buffer is too small, try again and ask for exactly <minimum>
    // bytes.
    giveback_reserved();
    b = pd_strand->reserve(minimum,1,current_outb_alignment());
    pd_outb_mkr = b.buffer;
    pd_outb_end = (void *) ((omni::ptr_arith_t)pd_outb_mkr + b.size);
    pd_written += b.size;
  }
  return;
}

void

NetBufferedStream::giveback_reserved(CORBA::Boolean transmit) {
  // Tell the strand we have finished with the previous buffer with
  // so much to spare
  ensure_wrlocked();
  int oldalignment = current_outb_alignment();

  pd_strand->giveback_reserved((omni::ptr_arith_t)pd_outb_end - 
			       (omni::ptr_arith_t)pd_outb_mkr, transmit);
  pd_written = WrMessageAlreadyWritten();
  rewind_outb_mkr(oldalignment);
  return;
}

void

NetBufferedStream::receive(size_t minimum) {
  Strand::sbuf b;

  ensure_rdlocked();
  giveback_received();

  // determine what buffer size to request
  size_t bufsize = RdMessageUnRead();

  if (RdMessageSize())
    {
      if (bufsize < minimum) {
	// Error, try to reserve more bytes than the message size limit
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      }
      else 
	if (bufsize > pd_strand->max_receive_buffer_size())
	  bufsize = pd_strand->max_receive_buffer_size();
    }
  else
    {
      // Unlimited message size
      // Be conservative, just ask for the minimum
      bufsize = ((pd_ideal_buf_size) ? pd_ideal_buf_size :
		 pd_strand->max_receive_buffer_size());
      if (bufsize > minimum)
	bufsize = minimum;
    }

  b = pd_strand->receive(bufsize,0,current_inb_alignment());
  pd_inb_mkr = b.buffer;
  pd_inb_end = (void *) ((omni::ptr_arith_t)pd_inb_mkr + b.size);
  pd_read += b.size;

  if (b.size < minimum) {
    // What has been received is too small, try again and ask for
    // exactly <minimum> bytes
    giveback_received();
    b = pd_strand->receive(minimum,1,current_inb_alignment());
    pd_inb_mkr = b.buffer;
    pd_inb_end = (void *) ((omni::ptr_arith_t)pd_inb_mkr + b.size);
    pd_read += b.size;
  }
  return;
}

void

NetBufferedStream::giveback_received() {
  // Return any leftovers.

  ensure_rdlocked();
  int oldalignment = current_inb_alignment();

  pd_strand->giveback_received((omni::ptr_arith_t)pd_inb_end -
			       (omni::ptr_arith_t)pd_inb_mkr);
  pd_read = RdMessageAlreadyRead();
  rewind_inb_mkr(oldalignment);
  return;
}

void

NetBufferedStream::rewind_inb_mkr(int oldalignment) {
  pd_inb_end = pd_inb_mkr = (void *) ((omni::ptr_arith_t) oldalignment);
  return;
}

void

NetBufferedStream::rewind_outb_mkr(int oldalignment) {
  pd_outb_end = pd_outb_mkr = (void *)((omni::ptr_arith_t) oldalignment);
  return;
}

int

NetBufferedStream::current_outb_alignment() {
  int align = (omni::ptr_arith_t)pd_outb_mkr & ((int)omni::max_alignment - 1);
  return ((align) ? align : (int)omni::max_alignment);
}

int

NetBufferedStream::current_inb_alignment() {
  int align = (omni::ptr_arith_t)pd_inb_mkr & ((int)omni::max_alignment - 1);
  return ((align) ? align : (int)omni::max_alignment);
}


void

NetBufferedStream::ensure_rdlocked() {
  if (!pd_RdLock)
    throw omniORB::fatalException(__FILE__,__LINE__,
       "NetBufferedStream::ensure_rdlocked()- read lock is not held");
  return;
}

void

NetBufferedStream::ensure_wrlocked() {
  if (!pd_WrLock)
    throw omniORB::fatalException(__FILE__,__LINE__,
       "NetBufferedStream::ensure_wrlocked()- write lock is not held");

  return;
}


void

NetBufferedStream::RdLock() {
  if (!pd_RdLock) {
    Strand_Sync::RdLock();
    rewind_inb_mkr((int)omni::max_alignment);
    pd_RdLock = 1;
  }
  return;
}

void

NetBufferedStream::RdUnlock() {
  if (pd_RdLock) {
    giveback_received();
    Strand_Sync::RdUnlock();
    pd_RdLock = 0;
  }
  return;
}

void

NetBufferedStream::WrLock() {
  if (!pd_WrLock) {
    Strand_Sync::WrLock();
    rewind_outb_mkr((int)omni::max_alignment);
    pd_WrLock = 1;
  }
  return;
}

void

NetBufferedStream::WrUnlock() {
  if (pd_WrLock) {
    giveback_reserved();
    Strand_Sync::WrUnlock();
    pd_WrLock = 0;
  }
  return;
}

void

NetBufferedStream::skip(CORBA::ULong size)
{
  ensure_rdlocked();

  // Check that we do not skip beyond the current message boundary
  size_t m = RdMessageUnRead();
  if (m && m < size)
    // Error, try to skip more bytes than the message size limit
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  
  while (size)
    {
      size_t nbytes = size;
      if (nbytes > pd_strand->max_receive_buffer_size())
	nbytes = pd_strand->max_receive_buffer_size();
      (void) align_and_get_bytes(omni::ALIGN_1,nbytes);
      size -= nbytes;
    }
  return;
}

size_t

NetBufferedStream::WrMessageAlreadyWritten() const
{
  return (pd_written - ((omni::ptr_arith_t)pd_outb_end - (omni::ptr_arith_t)pd_outb_mkr));
}

size_t

NetBufferedStream::WrMessageSpaceLeft() const
{
  return ((WrMessageSize())?(WrMessageSize()-WrMessageAlreadyWritten()) : 0);
}

size_t

NetBufferedStream::RdMessageAlreadyRead() const
{
  return (pd_read - ((omni::ptr_arith_t)pd_inb_end - (omni::ptr_arith_t)pd_inb_mkr));
}

size_t

NetBufferedStream::RdMessageUnRead() const
{
  return ((RdMessageSize()) ? (RdMessageSize() - RdMessageAlreadyRead()) : 0);
}

void

NetBufferedStream::WrMessageSize(size_t msgsize)
{
  giveback_reserved();
  pd_written = 0;
  pd_wrmsg_size = msgsize;
  return;
}

void

NetBufferedStream::RdMessageSize(size_t msgsize,CORBA::Boolean byteorder)
{
  giveback_received();
  pd_read = 0;
  pd_rdmsg_size = msgsize;
  pd_rdmsg_byte_order = byteorder;
  return;
}

