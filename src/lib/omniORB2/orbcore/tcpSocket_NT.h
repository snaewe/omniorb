// -*- Mode: C++; -*-
//                            Package   : omniORB2
// TCPSocket_NT.h             Created on: 4/2/97
//                            Author    : Eoin Carroll (ewc)
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
//	Implementation of the Strand using TCP/IP and WinSock interface
//	

/*
  $Log$
  Revision 1.3  1997/05/06 15:29:54  sll
  Public release.

  */

#ifndef __TCPSOCKET_NT_H__
#define __TCPSOCKET_NT_H__

typedef SOCKET tcpSocketHandle_t;
class tcpSocketRope;
class tcpSocketStrand;

class tcpSocketEndpoint : public Endpoint {
public:
  tcpSocketEndpoint(CORBA::Char *h,CORBA::UShort p) 
    : Endpoint((CORBA::Char *)"TCPIP") 
  {
    pd_host = 0;
    pd_port = 0;
    host(h);
    port(p);
    return;
  }

  tcpSocketEndpoint(const tcpSocketEndpoint *e) 
    : Endpoint((CORBA::Char *)"TCPIP") 
  {
    pd_host = 0;
    pd_port = 0;
    host(e->host());
    port(e->port());
    return;
  }

  tcpSocketEndpoint &operator=(const tcpSocketEndpoint &e) 
  {
    host(e.host());
    port(e.port());
    return *this;
  }
  
  CORBA::Boolean operator==(const tcpSocketEndpoint *e)
  {
    if ((strcmp((const char *)pd_host,
		(const char *)e->host())==0) && (pd_port == e->port()))
      return 1;
    else
      return 0;
  }	

  virtual ~tcpSocketEndpoint() {
    if (pd_host) delete [] pd_host;
    return;
  }

  CORBA::Char * host() const { return pd_host; }

  void host(const CORBA::Char *p) {
    if (pd_host) delete [] pd_host;
    if (p) {
      pd_host = new CORBA::Char [strlen((char *)p) + 1];
      strcpy((char *)pd_host,(char *)p);
    }
    else {
      pd_host = new CORBA::Char [1];
      pd_host[0] = '\0';
    }
    return;
  }
  CORBA::UShort port() const { return pd_port; }
  void port(const CORBA::UShort p) { pd_port = p; }
  
  static tcpSocketEndpoint *castup(Endpoint *e) {
    if (e->is_protocol((CORBA::Char *)"TCPIP")) {
      return (tcpSocketEndpoint *)e;
    }
    else {
      return 0;
    }
  }
  
private:
  CORBA::Char  *pd_host;
  CORBA::UShort pd_port;
  
  tcpSocketEndpoint();
};


class tcpSocketRendezvous {
public:
  tcpSocketRendezvous(tcpSocketRope *r,tcpSocketEndpoint *me);
  tcpSocketRendezvous(tcpSocketRope *r,tcpSocketHandle_t sock);
  virtual ~tcpSocketRendezvous();

  tcpSocketStrand * accept();

  static CORBA::Boolean has_spawned_rendevous_threads;

private:
  tcpSocketRope    *pd_rope;
  tcpSocketHandle_t pd_socket;
};

class tcpSocketStrand : public Strand {
public:

  static const unsigned int buffer_size;

  tcpSocketStrand(tcpSocketRope *r,
		  tcpSocketEndpoint *remote,
		  _CORBA_Boolean heapAllocated = 0);
  // Concurrency Control:
  //    MUTEX = r->pd_lock
  // Pre-condition:
  //	  Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

  tcpSocketStrand(tcpSocketRope *r,
		  tcpSocketHandle_t sock,
		  _CORBA_Boolean heapAllocated = 0);
  // Concurrency Control:
  //    MUTEX = r->pd_lock
  // Pre-condition:
  //	  Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

  virtual ~tcpSocketStrand();
  // MUTEX:
  //    pd_rope->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit

  virtual size_t MaxMTU() const;

  virtual Strand::sbuf receive(size_t size,
			       CORBA::Boolean exactly,
			       int align);

  virtual void giveback_received(size_t leftover);
  virtual size_t max_receive_buffer_size();
  virtual void receive_and_copy(Strand::sbuf b);
  virtual void skip(size_t size);
  virtual Strand::sbuf reserve(size_t size,
			       CORBA::Boolean exactly,
			       int align,
			       CORBA::Boolean transmit=0);
  virtual void giveback_reserved(size_t leftover,
				 CORBA::Boolean transmit=0);
  virtual size_t max_reserve_buffer_size();
  virtual void reserve_and_copy(Strand::sbuf b,
				CORBA::Boolean transmit=0);
  virtual void shutdown();

private:

  void transmit();
  void fetch(CORBA::ULong max=0);
  // fetch data from the network to the internal buffer.
  // If <max>=0, fetch as much as possible, otherwise fetch at most <max>
  // bytes.

  tcpSocketHandle_t pd_socket;
  void    *pd_tx_buffer;
  void    *pd_tx_begin;
  void    *pd_tx_end;
  void    *pd_tx_reserved_end;

  void    *pd_rx_buffer;
  void    *pd_rx_begin;
  void    *pd_rx_end;
  void    *pd_rx_received_end;
    
};

class tcpSocketRope : public Rope {
public:
  tcpSocketRope(Anchor *a,
		unsigned int maxStrands,
		Endpoint *e, 
		_CORBA_Boolean passive = 0,
		_CORBA_Boolean heapAllocated = 0);
  // Create a tcpSocket Rope.
  // If passive == 1,
  //      create a passive socket. e->port() specifies the port number to
  //      bind to or 0 if an arbitary port number can be assigned). 
  //      e->host() can either be the host's fully qualified domain name 
  //      (FQDN) or a 0 length string. If it is the latter, the constructor
  //      will initialise the host field with the host's IP address.
  //      This is a dot separated numeric string of the form "xxx.xxx.xxx.xxx".
  // If passive == 0,
  //      The endpoint <e> is the remote endpoint and should contain the
  //      host name in FQDN form or as a dot separeted numeric string.
  // Concurrency Control:
  //    MUTEX = a->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

  virtual ~tcpSocketRope();
  // Concurrency Control:
  //    MUTEX = pd_anchor->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit

  virtual CORBA::Boolean remote_is(Endpoint *&e);
    
  virtual CORBA::Boolean this_is(Endpoint *&e);
  
  virtual void iopProfile(const _CORBA_Octet *objkey,const size_t objkeysize,
			  IOP::TaggedProfile &p);
  virtual Strand *newStrand();

  CORBA::Boolean is_passive() { return pd_is_passive; }
  tcpSocketRendezvous * getRendezvous() { return pd_rendezvous; }

  friend class tcpSocketRendezvous;
private:
  CORBA::Boolean pd_is_passive;
  union {
    tcpSocketEndpoint *remote;
    tcpSocketEndpoint *me;
  } pd_endpoint;
  tcpSocketRendezvous * pd_rendezvous;
};

#endif // __TCPSOCKET_NT_H__
