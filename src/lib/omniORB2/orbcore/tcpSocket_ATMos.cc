// -*- Mode: C++; -*-
//                            Package   : omniORB2
// tcpSocket_ATMOS.cc         Created on: 4/11/96
//                            Author  : Eoin Carroll (ewc)
//
// Copyright (C) Olivetti - Oracle Research Limited, 1996
//
// Description:
//	Implementation of the Strand using TCP/IP and socket interface
//	

/*
  $Log$
  Revision 1.1  1997/01/08 18:35:17  ewc
  Initial revision

  */

/*********************************************************************/
/*                                                                   */
/*                        ATMOS version                              */
/*                                                                   */
/*********************************************************************/

// #define TRACE_RECV
// #define TRACE_SEND

#include <omniORB2/CORBA.h>
#include "tcpSocket_ATMos.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <kernel.h>
#include <pthread.h>
#include <timelib.h>
#include <netdb.h>
#include <ip.h>
#include <bluelib.h>


#include "libcWrapper.h"


#define NO_DNS    // Define this while ATMOS doesn't have a proper DNS
                  // implementation

#define RETRY 5   // Number of times to try to open an outgoing TCP 
                  // connection before giving up.



// Size of transmit and receive buffers
const 
unsigned int 
tcpSocketStrand::buffer_size = 8192 + (int)omniORB::max_alignment;

tcpSocketStrand::tcpSocketStrand(tcpSocketRope *rope,
				 tcpSocketEndpoint   *r,
				 CORBA::Boolean heapAllocated)
  : Strand(rope,heapAllocated)
{
  const int ipfstrlen = 64;  
          // Max length of string used to create ip file handle.
          // [Excluding length of host ip address]

  int retry = RETRY;   // Number of times to retry opening connection 
                       // to (remote) object.


  // Get remote IP address:
  char ipaddr[16];
  
  if (! LibcWrapper::isipaddr((char*) r->host()))
    {
      char* remote_host;

#ifdef NO_DNS
      // ATMOS doesn't have a proper implementation of DNS functions.
      // Strip away any domain from the hostname before calling gethostbyname()
    
      char* remstr;
      if ((remstr = strchr((char*) r->host(), '.')) == NULL)
	{
	  remote_host = new char[strlen((char*) r->host())+1];
	  strcpy(remote_host,(char*) r->host());
	}
      else
	{
	  int rlen = (remstr - (char*) r->host());
	  remote_host = new char[rlen+1];
	  strncpy(remote_host, (char*) r->host(), rlen);
	  *(remote_host+rlen) = 0;
	}
#else
      remote_host = r->host();
#endif

      

      LibcWrapper::hostent_var h;
      int  rc;
      if (LibcWrapper::gethostbyname(remote_host,h,rc) < 0) {
	// XXX look at rc to decide what to do or if to give up what errno
	// XXX to return
	// XXX For the moment, just return EINVAL
#ifdef NO_DNS
	delete[] remote_host;
#endif

	throw CORBA::COMM_FAILURE(EINVAL,CORBA::COMPLETED_NO);
      }

#ifdef NO_DNS
      delete[] remote_host;
#endif

      unsigned long int ip_p;
      memcpy((void*) &ip_p,(void*)h.hostent()->h_addr_list[0],sizeof(long));
      sprintf(ipaddr,"%d.%d.%d.%d",
	      (int)(ip_p & 0x000000ff),
	      (int)((ip_p & 0x0000ff00) >> 8),
	      (int)((ip_p & 0x00ff0000) >> 16),
	      (int)((ip_p & 0xff000000) >> 24));
    }
else
    {
      strncpy(ipaddr,(char*) r->host(),sizeof(ipaddr));
    }

    // Set up file string (used to open TCP connection to remote host):
    char* ipfstr = new char[ipfstrlen+strlen(ipaddr)];
    sprintf(ipfstr,"//ip/TYPE=TCP/RHOST=%s/RPORT=%d",ipaddr,r->port());


    // Open out-going connection:

    pd_ipfilep = NULL;
    do
      {
	kprintf("Connect attempt: %d.\n",retry);
	pd_ipfilep = fopen(ipfstr,"wb+");

      }
    while(!pd_ipfilep && retry--);


    if (!pd_ipfilep)
      {
	delete[] ipfstr;
	throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
      }

   delete[] ipfstr;
   
   pd_tx_buffer = (void *) new char[tcpSocketStrand::buffer_size];
   pd_tx_begin  = pd_tx_end = pd_tx_reserved_end = pd_tx_buffer;
   pd_rx_buffer = (void *) new char[tcpSocketStrand::buffer_size];
   pd_rx_begin = pd_rx_end = pd_rx_received_end = pd_rx_buffer;
   return;
}

tcpSocketStrand::tcpSocketStrand(tcpSocketRope *r,
				 tcpSocketHandle_t connfilep,
				 CORBA::Boolean heapAllocated)
  : Strand(r,heapAllocated)
{
  pd_ipfilep = connfilep;
  pd_tx_buffer = (void *) new char[tcpSocketStrand::buffer_size];
  pd_tx_begin  = pd_tx_end = pd_tx_reserved_end = pd_tx_buffer;
  pd_rx_buffer = (void *) new char[tcpSocketStrand::buffer_size];
  pd_rx_begin = pd_rx_end = pd_rx_received_end = pd_rx_buffer;
  return;
}


tcpSocketStrand::~tcpSocketStrand() 
{
  fclose(pd_ipfilep);
  pd_ipfilep = NULL;
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
tcpSocketStrand::MaxMTU() const {
  // No limit
  return UINT_MAX;
}


Strand::sbuf
tcpSocketStrand::receive(size_t size,
		CORBA::Boolean exactly,
		int align) 
{
  giveback_received(0);

  size_t bsz = ((omniORB::ptr_arith_t) pd_rx_end - 
    (omniORB::ptr_arith_t) pd_rx_begin);

  int current_alignment;
  omniORB::ptr_arith_t new_align_ptr;

  if (!bsz) {
    // No data left in receive buffer, fetch() and try again
    // rewind the buffer pointers to the beginning of the buffer and
    // at the same alignment as they were previously
    current_alignment = (omniORB::ptr_arith_t) pd_rx_begin &
      ((int)omniORB::max_alignment - 1);
    if (current_alignment == 0) {
      current_alignment = (int) omniORB::max_alignment;
    }
    new_align_ptr = omniORB::align_to((omniORB::ptr_arith_t) pd_rx_buffer,
				      omniORB::max_alignment) + 
                    current_alignment;
    if (new_align_ptr >= ((omniORB::ptr_arith_t)pd_rx_buffer + 
			  (int)omniORB::max_alignment)) {
      new_align_ptr -= (int) omniORB::max_alignment;
    }
    pd_rx_begin = pd_rx_received_end = pd_rx_end = (void *)new_align_ptr;

    fetch();
    return receive(size,exactly,align);
  }

  if (align > (int)omniORB::max_alignment) {
    throw CORBA::INTERNAL(0,CORBA::COMPLETED_MAYBE);
  }

  current_alignment = (omniORB::ptr_arith_t) pd_rx_begin &
    ((int)omniORB::max_alignment - 1);
  if (current_alignment == 0) {
    current_alignment = (int) omniORB::max_alignment;
  }

  if (current_alignment != align) {
    // alignment is not right, move the data to the correct alignment
    new_align_ptr = omniORB::align_to((omniORB::ptr_arith_t) pd_rx_buffer,
				      omniORB::max_alignment) + align;
    if (new_align_ptr >= ((omniORB::ptr_arith_t)pd_rx_buffer + 
			  (int)omniORB::max_alignment)) {
      new_align_ptr -= (int) omniORB::max_alignment;
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

      
      size_t avail = tcpSocketStrand::buffer_size - 
	                ((omniORB::ptr_arith_t) pd_rx_end - 
			 (omniORB::ptr_arith_t) pd_rx_buffer) + bsz;
      if (avail < size) {
	// Not enough empty space, got to move existing data
	current_alignment = (omniORB::ptr_arith_t) pd_rx_begin &
	  ((int)omniORB::max_alignment - 1);
	if (current_alignment == 0) {
	  current_alignment = (int) omniORB::max_alignment;
	}
	new_align_ptr = omniORB::align_to((omniORB::ptr_arith_t) pd_rx_buffer,
					  omniORB::max_alignment) + 
	                 current_alignment;
	if (new_align_ptr >= ((omniORB::ptr_arith_t)pd_rx_buffer + 
			      (int)omniORB::max_alignment)) {
	  new_align_ptr -= (int) omniORB::max_alignment;
	}
	memmove((void *)new_align_ptr,pd_rx_begin,bsz);
	pd_rx_begin = pd_rx_received_end = (void *)new_align_ptr;
	pd_rx_end = (void *)(new_align_ptr + bsz);
      }
	      
      fetch();
      return receive(size,exactly,align);
    }
    else {
      size = bsz;
    }
  }
  pd_rx_received_end = (void *)((omniORB::ptr_arith_t)pd_rx_begin + size);
  Strand::sbuf result;
  result.buffer = pd_rx_begin;
  result.size   = size;
  return result;
}

void 
tcpSocketStrand::giveback_received(size_t leftover) 
{
  size_t total = (omniORB::ptr_arith_t)pd_rx_received_end -
    (omniORB::ptr_arith_t)pd_rx_begin;
  if (total < leftover) {
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  }
  total -= leftover;
  pd_rx_begin = (void *)((omniORB::ptr_arith_t)pd_rx_begin + total);
  pd_rx_received_end = pd_rx_begin;
  return;
}

size_t 
tcpSocketStrand::max_receive_buffer_size() 
{
  return tcpSocketStrand::buffer_size - (int)omniORB::max_alignment;
}

void
tcpSocketStrand::receive_and_copy(Strand::sbuf b)
{
  giveback_received(0);

  size_t sz = b.size;
  char  *p = (char *)b.buffer;

  size_t bsz = ((omniORB::ptr_arith_t)pd_rx_end - 
		(omniORB::ptr_arith_t)pd_rx_begin);
  if (bsz) {
    if (bsz > sz) {
      bsz = sz;
    }
    memcpy((void *)p,pd_rx_begin,bsz);
    pd_rx_begin = (void *)((omniORB::ptr_arith_t) pd_rx_begin + bsz);
    pd_rx_received_end = pd_rx_begin;
    sz -= bsz;
    p += bsz;
  }
  while (sz) {
    int rx;
#ifdef TRACE_RECV
    kprintf("tcpSocketStrand::receive_and_copy--- fread(). size = %d\n",
	      (int) sz);
#endif

    rx = fread((void*) p,1,sz,pd_ipfilep);

    if (rx < 0)
      {
	// error condition raised
          setStrandIsDying();
	  throw CORBA::COMM_FAILURE(errno,CORBA::COMPLETED_MAYBE);
      }
    else if (rx == 0)
      {
	// no bytes read
	  setStrandIsDying();
	  throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
      }

#ifdef TRACE_RECV
    kprintf("tcpSocketStrand::receive_and_copy-- %d bytes\n",rx);
#endif
    sz -= rx;
    p += rx;
  }
}

void
tcpSocketStrand::skip(size_t size)
{
  giveback_received(0);

  while (size) {
    size_t sz;
    sz = max_receive_buffer_size();
    if (sz > size) {
      sz = size;
    }
    int current_alignment = (omniORB::ptr_arith_t) pd_rx_begin &
      ((int)omniORB::max_alignment - 1);
    if (current_alignment == 0) {
      current_alignment = (int) omniORB::max_alignment;
    }
    Strand::sbuf sb = receive(sz,0,current_alignment);
    size -= sb.size;
  }
  return;
}

void
tcpSocketStrand::fetch()
{
  size_t bsz = tcpSocketStrand::buffer_size -
    ((omniORB::ptr_arith_t) pd_rx_end - (omniORB::ptr_arith_t) pd_rx_buffer);

  if (!bsz) return;

  int rx;

#ifdef TRACE_RECV
  kprintf("tcpSocketStrand::fetch bsz = %d\n",(int) bsz);
#endif
 
    if (net_receive(pd_ipfilep,(BYTE*) pd_rx_end,(int) bsz,&rx) != 0) 
      {
          // net_receive returned error
	  setStrandIsDying();
	  throw CORBA::COMM_FAILURE(errno,CORBA::COMPLETED_MAYBE);
      }

   if (rx <= 0)
      {
	// no bytes read by net_receive
	setStrandIsDying();
	throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
      }

#ifdef TRACE_RECV
  kprintf("tcpSocketStrand::fetched -- %d bytes.\n",rx);
#endif

  pd_rx_end = (void *)((omniORB::ptr_arith_t) pd_rx_end + rx);
  return;
}


Strand::sbuf 
tcpSocketStrand::reserve(size_t size,
		CORBA::Boolean exactly,
		int align,
		CORBA::Boolean tx) 
{
  giveback_reserved(0,tx);
  
  size_t bsz = tcpSocketStrand::buffer_size -
    ((omniORB::ptr_arith_t) pd_tx_end - (omniORB::ptr_arith_t) pd_tx_buffer);
  
  if (!bsz) {
    // No space left, transmit and try again
    transmit();
    return reserve(size,exactly,align,tx);
  }

  if (align > (int)omniORB::max_alignment) {
    throw CORBA::INTERNAL(0,CORBA::COMPLETED_MAYBE);
  }

  int current_alignment = (omniORB::ptr_arith_t) pd_tx_end & 
    ((int)omniORB::max_alignment - 1);
  if (current_alignment == 0) {
    current_alignment = (int)omniORB::max_alignment;
  }

  if (current_alignment != align) {
    // alignment is not right
    if (pd_tx_end == pd_tx_begin) {
      // There is nothing in the buffer, we could adjust
      // pd_tx_begin and pd_tx_end to the required alignment
      omniORB::ptr_arith_t new_align_ptr;

      new_align_ptr = omniORB::align_to((omniORB::ptr_arith_t) pd_tx_buffer,
					omniORB::max_alignment) + align;
      if (new_align_ptr >= ((omniORB::ptr_arith_t)pd_tx_buffer + 
			   (int)omniORB::max_alignment)) {
	new_align_ptr -= (int) omniORB::max_alignment;
      }
      pd_tx_begin = pd_tx_end = pd_tx_reserved_end = (void *)new_align_ptr;
      bsz = tcpSocketStrand::buffer_size - ((omniORB::ptr_arith_t) pd_tx_end 
		    - (omniORB::ptr_arith_t) pd_tx_buffer);
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
  pd_tx_reserved_end = (void *)((omniORB::ptr_arith_t)pd_tx_end + size);
  Strand::sbuf result;
  result.buffer = pd_tx_end;
  result.size   = size;
  return result;
}

void
tcpSocketStrand::giveback_reserved(size_t leftover,
			  CORBA::Boolean tx) 
{
  size_t total = (omniORB::ptr_arith_t)pd_tx_reserved_end -
    (omniORB::ptr_arith_t)pd_tx_end;
  if (total < leftover) {
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  }
  total -= leftover;
  pd_tx_end = (void *)((omniORB::ptr_arith_t)pd_tx_end + total);
  pd_tx_reserved_end = pd_tx_end;
  if (tx) {
    transmit();
  }
  return;
}

void 
tcpSocketStrand::reserve_and_copy(Strand::sbuf b,
			 CORBA::Boolean transmit)
{
  // transmit anything that is left in the transmit buffer
  giveback_reserved(0,1);
  
  // Transmit directly from the supplied buffer
  int tx;
  size_t sz = b.size;
  char *p = (char *)b.buffer;
  while (sz) {
#ifdef TRACE_SEND
    kprintf("tcpSocketStrand::reserve_and_copy-- send %d bytes\n",sz);
#endif
    tx = fwrite(p,1,sz,pd_ipfilep);
    fflush(pd_ipfilep);

    if (tx < 0)
      {
	// error condition
	setStrandIsDying();
	throw CORBA::COMM_FAILURE(errno,CORBA::COMPLETED_MAYBE);
      }
    else if (tx == 0) 
      {
	// no bytes written
	setStrandIsDying();
	throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
      }

    
    sz -= tx;
    p += tx;
  }
}

size_t
tcpSocketStrand::max_reserve_buffer_size() 
{
  return tcpSocketStrand::buffer_size - (int)omniORB::max_alignment;
}

void
tcpSocketStrand::transmit() 
{
  size_t sz = (omniORB::ptr_arith_t)pd_tx_end - 
              (omniORB::ptr_arith_t)pd_tx_begin;
  int tx;
  char *p = (char *)pd_tx_begin;
  while (sz) {
#ifdef TRACE_SEND
    kprintf("tcpSocketStrand::transmit-- send %d bytes\n", sz);

#endif
    tx = fwrite(p,1,sz,pd_ipfilep);
    fflush(pd_ipfilep);

    if (tx < 0) 
      {
	// error condition raised
	  setStrandIsDying();
	  throw CORBA::COMM_FAILURE(errno,CORBA::COMPLETED_MAYBE);
      }
    else if (tx == 0)
      {
	// no bytes sent
	  setStrandIsDying();
	  throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
      }

    sz -= tx;
    p += tx;
  }
  pd_tx_begin = pd_tx_end = pd_tx_reserved_end = pd_tx_buffer;
  return;
}

void
tcpSocketStrand::shutdown()
{
  setStrandIsDying();

  net_disconnect(pd_ipfilep,0);
  pd_ipfilep = NULL;

  return;
}

tcpSocketRendezvous::tcpSocketRendezvous(tcpSocketRope *r,tcpSocketEndpoint *me) 
{
  const int hostStrLen = 64;   // Length of buffer containing hostname.
  const int lportStrLen = 16;  // Length of buffer containing port

  // Open a connection and obtain a (local) port:  

  pd_ipfilep = NULL;

  // r+ = listen/accept, b = binary mode
  // Note that setting LPORT to zero automatically assigns a port *without* 
  // blocking. This port is used in tcpSocketRendezvous::accept() to
  // accept incoming calls [and is also the port placed in the IIOP 
  //                        reference]

  if (!(pd_ipfilep = fopen("//ip/TYPE=TCP/LPORT=0","rb+")))
    {
      throw CORBA::COMM_FAILURE(errno,CORBA::COMPLETED_NO);
    }
  

  // Obtain the allocated port:

  char lportBuf[lportStrLen];

  if(fgetattr1(pd_ipfilep,"LPORT",lportBuf,64) != 0)
    {
      fclose(pd_ipfilep);
      pd_ipfilep = NULL;
      throw CORBA::COMM_FAILURE(errno,CORBA::COMPLETED_NO);
    }

  short lport = (short) atoi(lportBuf);


  // Find out local ip address:

  char self[hostStrLen];
  gethostname(&self[0],hostStrLen);


  LibcWrapper::hostent_var h;
  int rc;
  if (LibcWrapper::gethostbyname(self,h,rc) < 0) 
    {
	 throw omniORB::fatalException(__FILE__,__LINE__,
				    "Cannot get the address of this host");
    } 
      
   unsigned long int ip_p;
   memcpy((void*) &ip_p,(void*)h.hostent()->h_addr_list[0],sizeof(long));

   char ipaddr[16];
   sprintf(ipaddr,"%d.%d.%d.%d",
	      (int)(ip_p & 0x000000ff),
	      (int)((ip_p & 0x0000ff00) >> 8),
	      (int)((ip_p & 0x00ff0000) >> 16),
	      (int)((ip_p & 0xff000000) >> 24));

  // Set hostname and port in Endpoint:

  me->host((const CORBA::Char*) ipaddr);
  me->port(lport);

  pd_rope = r;
  return;
}

tcpSocketRendezvous::tcpSocketRendezvous(tcpSocketRope *r,tcpSocketHandle_t 
					                    connfilep) 
{
  pd_ipfilep = connfilep;
  pd_rope = r;
  return;
}

tcpSocketRendezvous::~tcpSocketRendezvous() 
{
  if (pd_ipfilep != NULL) {
    fclose(pd_ipfilep);
    pd_ipfilep = NULL;
  }
  return;
}


tcpSocketStrand *
tcpSocketRendezvous::accept() 
{
  const int portStrLen = 16;  // Maximum number of digits in a port
  char lportstr[portStrLen];

  
  //  Listen on the port 
  // [previously allocated when tcpSocketRendezvous constructed].

  // Get the local port:

  Endpoint* e = NULL;  
  pd_rope->this_is(e);
  tcpSocketEndpoint* me = tcpSocketEndpoint::castup(e);
  

  //   Open a new file connection:
  tcpSocketHandle_t new_connfilep = NULL;

  if(!(new_connfilep = fopen("//ip/TYPE=TCP","rb+")))
    {
      throw CORBA::COMM_FAILURE(errno,CORBA::COMPLETED_NO);
    }

  sprintf(lportstr,"%d",me->port());  

  // listen for connections, using new file connection, and port allocated
  // during construction:

  if (net_listen(new_connfilep,NULL,lportstr,NULL,NULL) != 0)
    {
      fclose(new_connfilep);
      new_connfilep = NULL;
      throw CORBA::COMM_FAILURE(errno,CORBA::COMPLETED_NO);
    }
   

  // Listen has succeeded

  tcpSocketStrand *ns;
  pd_rope->pd_lock.lock();

  try
    {
      ns = new tcpSocketStrand(pd_rope,new_connfilep,1);
      if (!ns)
	{
	  pd_rope->pd_lock.unlock();
	  throw(CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO));
	}

      ns->incrRefCount(1);
    }
  catch(...)
    {
      pd_rope->pd_lock.unlock();
      throw;
    }

  pd_rope->pd_lock.unlock();
  return ns;
}


tcpSocketRope::tcpSocketRope(Anchor *a,
			     unsigned int maxStrands,
			     Endpoint *e,
			     CORBA::Boolean passive,
			     CORBA::Boolean heapAllocated)
  : Rope(a,maxStrands,heapAllocated)
{
  tcpSocketEndpoint *te = tcpSocketEndpoint::castup(e);
  if (!te) {
    throw CORBA::INTERNAL(0,CORBA::COMPLETED_NO);
  }
  pd_is_passive = passive;
  if (!passive) {
    pd_endpoint.remote = new tcpSocketEndpoint(te);
  }
  else {
    pd_endpoint.me = new tcpSocketEndpoint(te);
    pd_rendezvous = new tcpSocketRendezvous(this,pd_endpoint.me);
  }
  return;
}

tcpSocketRope::~tcpSocketRope()
{
  if (is_passive()) {
    if (pd_rendezvous) {
      delete pd_rendezvous;
      pd_rendezvous = 0;
    }
    if (pd_endpoint.me) {
      delete pd_endpoint.me;
      pd_endpoint.me = 0;
    }
  }
  else {
    if (pd_endpoint.remote) {
      delete pd_endpoint.remote;
      pd_endpoint.remote = 0;
    }
  }
  return;
}

CORBA::Boolean
tcpSocketRope::remote_is(Endpoint *&e)
{
  if (is_passive())
    return 0;

  if (e) {
    tcpSocketEndpoint *te = tcpSocketEndpoint::castup(e);
    if (!te)
      return 0;
    if (te == pd_endpoint.remote)
      return 1;
    else
      return 0;
  }
  else {
    e = new tcpSocketEndpoint(pd_endpoint.remote);
    if (!e)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    return 1;
  }
}

CORBA::Boolean
tcpSocketRope::this_is(Endpoint *&e)
{
  if (!is_passive())
    return 0;

  if (e) {
    tcpSocketEndpoint *te = tcpSocketEndpoint::castup(e);
    if (!te)
      return 0;
    if (*te == pd_endpoint.me)
      return 1;
    else
      return 0;
  }
  else {
    e = new tcpSocketEndpoint(pd_endpoint.me);
    if (!e)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    return 1;
  }
}

void
tcpSocketRope::iopProfile(const _CORBA_Octet *objkey,const size_t objkeysize,
			  IOP::TaggedProfile &p)
{
  p.tag = IOP::TAG_INTERNET_IOP;

  IIOP::ProfileBody b;
  b.iiop_version.major = IIOP::current_major;
  b.iiop_version.minor = IIOP::current_minor;
  if (is_passive()) {
    b.host = pd_endpoint.me->host();
    b.port = pd_endpoint.me->port();
  }
  else {
    b.host = pd_endpoint.remote->host();
    b.port = pd_endpoint.remote->port();
  }
  b.object_key.length((CORBA::ULong)objkeysize);
  memcpy((void *)&b.object_key[0],(void *)objkey,objkeysize);
  IIOP::profileToEncapStream(b,p.profile_data);
  return;
}


Strand *
tcpSocketRope::newStrand()
{
  if (is_passive()) 
  {
    throw CORBA::INTERNAL(0,CORBA::COMPLETED_NO);
  }
  return new tcpSocketStrand(this,pd_endpoint.remote,1);
}

