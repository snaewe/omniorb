// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpSocket.h                Created on: 30/9/97
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
 Revision 1.7  2000/07/04 15:22:49  dpg1
 Merge from omni3_develop.

 Revision 1.5.6.3  2000/01/28 15:57:10  djr
 Removed superflouous ref counting in Strand_iterator.
 Removed flags to indicate that Ropes and Strands are heap allocated.
 Improved allocation of client requests to strands.

 Revision 1.5.6.2  1999/09/24 15:01:39  djr
 Added module initialisers, and sll's new scavenger implementation.

 Revision 1.5.2.1  1999/09/21 20:37:18  sll
 -Simplified the scavenger code and the mechanism in which connections
  are shutdown. Now only one scavenger thread scans both incoming
  and outgoing connections. A separate thread do the actual shutdown.
 -omniORB::scanGranularity() now takes only one argument as there is
  only one scan period parameter instead of 2.
 -Trace messages in various modules have been updated to use the logger
  class.
 -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                -ORBscanIncomingPeriod.

 Revision 1.5  1999/07/09 21:04:29  sll
 Added private data member in tcpSocketMTincomingFactory.

 Revision 1.4  1999/03/11 16:25:57  djr
 Updated copyright notice

 Revision 1.3  1998/11/09 10:56:17  sll
 Removed the use of the reserved keyword "export".

 Revision 1.2  1998/03/19 19:52:03  sll
 Added delay connect to tcpSocketStrand.

 Revision 1.1  1997/12/09 18:43:15  sll
 Initial revision

*/

#ifndef __TCPSOCKET_H__
#define __TCPSOCKET_H__

// The following defines all the derived classes to implement the transport
// over TCP/IP using a BSD socket style interface.
//
//                                      Abstract class
//    tcpSocketFactoryType                ropeFactoryType
//    tcpSocketMTincomingFactory          incomingRopeFactory
//    tcpSocketMToutgoingFactory          outgoingRopeFactory
//    tcpSocketStrand                     Strand
//    tcpSocketRope                       Rope
//    tcpSocketEndpoint                   Endpoint
//
//    ropeFactory.h and rope.h contains the definition of these classes and
//    a description of each methods.
//

#include <relStream.h>

#if defined(__WIN32__)
typedef SOCKET tcpSocketHandle_t;
#elif defined(__atmos__)
typedef FILE*  tcpSocketHandle_t;
#else
typedef int    tcpSocketHandle_t;
#endif

class tcpSocketEndpoint;
class tcpSocketOutgoingRope;
class tcpSocketIncomingRope;
class tcpSocketStrand;
class tcpSocketRendezvouser;

class nobody;  // dummy class defined to silent gcc warning about
               // a private destructor without any friend 

class tcpSocketFactoryType : public ropeFactoryType {
public:
  // This is a singleton class.

  CORBA::Boolean is_IOPprofileId(IOP::ProfileId tag) const;
  CORBA::Boolean is_protocol(const char* name) const;
  CORBA::Boolean decodeIOPprofile(const IOP::TaggedProfile& profile,
					  // return values:
					  Endpoint*&     addr,
					  CORBA::Octet*& objkey,
					  size_t&        objkeysize) const;
  void encodeIOPprofile(const Endpoint* addr,
			const CORBA::Octet* objkey,
			const size_t objkeysize,
			IOP::TaggedProfile& profile) const;

  static tcpSocketFactoryType* singleton;
  static void init();

  friend class nobody;

private:
  tcpSocketFactoryType();
  ~tcpSocketFactoryType() {}  // Cannot delete a factory type instance
};

class tcpSocketMTincomingFactory : public incomingRopeFactory {
public:
  tcpSocketMTincomingFactory();

  const ropeFactoryType* getType() const { 
    return tcpSocketFactoryType::singleton;
  }

  CORBA::Boolean isIncoming(Endpoint* addr) const;
  void instantiateIncoming(Endpoint* addr,CORBA::Boolean exportflag);
  void startIncoming();
  void stopIncoming();
  void removeIncoming();
  Rope* findIncoming(Endpoint* addr) const;
  void getIncomingIOPprofiles(const CORBA::Octet*     objkey,
			      const size_t            objkeysize,
			      IOP::TaggedProfileList& profilelist) const;

  friend class nobody;
  friend class tcpSocketRendezvouser;
  friend class tcpSocketWorker;

private:
  enum { IDLE, ACTIVE, ZOMBIE } pd_state;

  omni_mutex                    pd_shutdown_lock;
  omni_condition                pd_shutdown_cond;
  int                           pd_shutdown_nthreads;

  virtual ~tcpSocketMTincomingFactory();
};


class tcpSocketMToutgoingFactory : public outgoingRopeFactory {
public:
  tcpSocketMToutgoingFactory();

  const ropeFactoryType* getType() const { 
    return tcpSocketFactoryType::singleton;
  }

  CORBA::Boolean isOutgoing(Endpoint* addr) const;
  Rope*  findOrCreateOutgoing(Endpoint* addr);

  friend class nobody;

private:
  virtual ~tcpSocketMToutgoingFactory();
};

class tcpSocketStrand : public reliableStreamStrand
{
public:

  static const unsigned int buffer_size;

  tcpSocketStrand(tcpSocketOutgoingRope *r,
		  tcpSocketEndpoint *remote);
  // Concurrency Control:
  //    MUTEX = r->pd_lock
  // Pre-condition:
  //	  Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

  tcpSocketStrand(tcpSocketIncomingRope *r,
		  tcpSocketHandle_t sock);
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

  void real_shutdown();
  void ll_send(void* buf,size_t sz);
  size_t ll_recv(void* buf,size_t sz);

  inline tcpSocketHandle_t handle() const {
    return pd_socket;
  }

private:
  tcpSocketHandle_t pd_socket;
  CORBA::Boolean    pd_send_giop_closeConnection;
  tcpSocketEndpoint* pd_delay_connect;
};


class tcpSocketIncomingRope : public Rope {
public:
  virtual ~tcpSocketIncomingRope();
  // Concurrency Control:
  //    MUTEX = pd_anchor->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit

  CORBA::Boolean is_incoming() const { return 1; }
  CORBA::Boolean is_outgoing() const { return 0; }
  CORBA::Boolean remote_is(Endpoint *&e) { return 0; }
  CORBA::Boolean this_is(Endpoint *&e);

  Strand *newStrand();
  friend class tcpSocketMTincomingFactory;
  friend class tcpSocketRendezvouser;

private:

  tcpSocketIncomingRope(tcpSocketMTincomingFactory* f,
			unsigned int maxStrands,
			tcpSocketEndpoint *e,
			CORBA::Boolean exportflag);
  // Create a tcpSocket incoming Rope.
  //      Creates a passive socket. e->port() specifies the port number to
  //      bind to or 0 if an arbitary port number can be assigned). 
  //      e->host() can either be the host's fully qualified domain name 
  //      (FQDN) or a 0 length string. If it is the latter, the constructor
  //      will initialise the host field with the host's IP address.
  //      This is a dot separated numeric string of the form "xxx.xxx.xxx.xxx".
  // Concurrency Control:
  //    MUTEX = f->anchor().pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

  void cancelThreads();
  // When this call returns, the rendezvouser thread would have been
  // cancelled and all the sockets instantiated by the accept() call on
  // the pd_rendezvous socket would have been shutdown.
  // The worker threads that are serving the sockets would notice the shutdown
  // and exit eventually. However, when will the worker threads see the
  // the shutdown is completely asynchronous to this call.
  // Concurrency Control:
  //    MUTEX = f->anchor().pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised


  tcpSocketEndpoint *me;
  tcpSocketHandle_t  pd_rendezvous;
  CORBA::Boolean     pd_export;
  enum { ACTIVE, SHUTDOWN, NO_THREAD } pd_shutdown;
  tcpSocketRendezvouser *rendezvouser;
};

class tcpSocketOutgoingRope : public Rope {
public:

  virtual ~tcpSocketOutgoingRope();
  // Concurrency Control:
  //    MUTEX = pd_anchor->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit

  CORBA::Boolean is_incoming() const { return 0; }
  CORBA::Boolean is_outgoing() const { return 1; }
  CORBA::Boolean remote_is(Endpoint *&e);
  CORBA::Boolean this_is(Endpoint *&e) { return 0; }

  Strand *newStrand();

  friend class tcpSocketMToutgoingFactory;

private:

  tcpSocketOutgoingRope(tcpSocketMToutgoingFactory* f,
			unsigned int maxStrands,
			tcpSocketEndpoint *e);
  // Create a tcpSocket outgoing Rope.
  //      The endpoint <e> is the remote endpoint and should contain the
  //      host name in FQDN form or as a dot separeted numeric string.
  // Concurrency Control:
  //    MUTEX = a->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

  tcpSocketEndpoint *remote;
};


class tcpSocketEndpoint : public Endpoint {
public:
  tcpSocketEndpoint(const CORBA::Char* h,CORBA::UShort p);
  tcpSocketEndpoint(const tcpSocketEndpoint* e);
  tcpSocketEndpoint &operator=(const tcpSocketEndpoint &e);
  CORBA::Boolean operator==(const tcpSocketEndpoint *e);
  virtual ~tcpSocketEndpoint();
  CORBA::Char * host() const;
  void host(const CORBA::Char *p);
  CORBA::UShort port() const;
  void port(const CORBA::UShort p);
  static tcpSocketEndpoint *castup(const Endpoint* e);

  static const char* protocol_name;

private:
  CORBA::Char  *pd_host;
  CORBA::UShort pd_port;
  
  tcpSocketEndpoint();
};

#endif // __TCPSOCKET_H__
