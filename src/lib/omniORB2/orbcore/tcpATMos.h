// -*- Mode: C++; -*-
//                            Package   : omniORB2
// tcpATMos.h                 Created on: 30/9/97
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
 Revision 1.3  1998/11/09 10:57:33  sll
 Removed the use of the reserved C++ keyword "export".

 Revision 1.2  1998/03/20 12:28:49  sll
 Added delay connect to tcpATMosStrand

 Revision 1.1  1997/12/09 18:43:20  sll
 Initial revision

*/

#ifndef __TCPATMos_H__
#define __TCPATMos_H__

// The following defines all the derived classes to implement the transport
// over TCP/IP ATMos blue interface
//
//                                      Abstract class
//    tcpATMosFactoryType                ropeFactoryType
//    tcpATMosMTincomingFactory          incomingRopeFactory
//    tcpATMosMToutgoingFactory          outgoingRopeFactory
//    tcpATMosStrand                     Strand
//    tcpATMosRope                       Rope
//    tcpATMosEndpoint                   Endpoint
//
//    ropeFactory.h and rope.h contains the definition of these classes and
//    a description of each methods.
//

#include <relStream.h>

typedef FILE*  tcpATMosHandle_t;

class tcpATMosEndpoint;
class tcpATMosOutgoingRope;
class tcpATMosIncomingRope;
class tcpATMosStrand;
class tcpATMosRendezvouser;

class nobody;  // dummy class defined to silent gcc warning about
               // a private destructor without any friend 

class tcpATMosFactoryType : public ropeFactoryType {
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

  static tcpATMosFactoryType* singleton;
  static void init();

  friend class nobody;

private:
  tcpATMosFactoryType();
  ~tcpATMosFactoryType() {}  // Cannot delete a factory type instance
};

class tcpATMosMTincomingFactory : public incomingRopeFactory {
public:
  tcpATMosMTincomingFactory();

  const ropeFactoryType* getType() const { 
    return tcpATMosFactoryType::singleton;
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

private:
  enum { IDLE, ACTIVE, ZOMBIE } pd_state;

  ~tcpATMosMTincomingFactory() {} // Cannot delete a factory instance

};


class tcpATMosMToutgoingFactory : public outgoingRopeFactory {
public:
  tcpATMosMToutgoingFactory();

  const ropeFactoryType* getType() const { 
    return tcpATMosFactoryType::singleton;
  }

  CORBA::Boolean isOutgoing(Endpoint* addr) const;
  Rope*  findOrCreateOutgoing(Endpoint* addr);

  friend class nobody;

private:
  ~tcpATMosMToutgoingFactory() {} // Cannot delete a factory instance
};

class tcpATMosStrand : public reliableStreamStrand
{
public:

  static const unsigned int buffer_size;

  tcpATMosStrand(tcpATMosOutgoingRope *r,
		  tcpATMosEndpoint *remote,
		  _CORBA_Boolean heapAllocated = 0);
  // Concurrency Control:
  //    MUTEX = r->pd_lock
  // Pre-condition:
  //	  Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

  tcpATMosStrand(tcpATMosIncomingRope *r,
		  tcpATMosHandle_t sock,
		  _CORBA_Boolean heapAllocated = 0);
  // Concurrency Control:
  //    MUTEX = r->pd_lock
  // Pre-condition:
  //	  Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

  virtual ~tcpATMosStrand();
  // MUTEX:
  //    pd_rope->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit

  void shutdown();
  void ll_send(void* buf,size_t sz);
  size_t ll_recv(void* buf,size_t sz);

  inline tcpATMosHandle_t handle() const {
    return pd_filehandle;
  }

private:
  tcpATMosHandle_t pd_filehandle;
  CORBA::Boolean    pd_send_giop_closeConnection;
  tcpATMosEndpoint* pd_delay_connect;
};


class tcpATMosIncomingRope : public Rope {
public:
  virtual ~tcpATMosIncomingRope();
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
  friend class tcpATMosMTincomingFactory;
  friend class tcpATMosRendezvouser;

private:

  tcpATMosIncomingRope(tcpATMosMTincomingFactory* f,
			unsigned int maxStrands,
			tcpATMosEndpoint *e,
			CORBA::Boolean exportflag);
  // Create a tcpATMos incoming Rope.
  //   If e->port() == 0,
  //        Create a file handle with LPORT=0 to get the ip module
  //        to assign an unused port number. Store the file handle
  //        in pd_rendezvous. This file handle will not be used but
  //        will be kept open until 
  //             tcpATMosMTincomingFactory::removeIncoming() is called.
  //        This is to keep the port reserved by having at lease one
  //        file handle opened on that port.
  //   else      
  //        Record the port number. Do not create a file handle, i.e.
  //        pd_rendezvous = 0.
  //
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
  // cancelled and all the file handles created by the rendezvouser
  // thread would be disconnected.
  // The worker threads that are serving the sockets would notice the shutdown
  // and exit eventually. However, when will the worker threads see the
  // the shutdown is completely asynchronous to this call.
  // Concurrency Control:
  //    MUTEX = f->anchor().pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised


  tcpATMosEndpoint *me;
  tcpATMosHandle_t  pd_rendezvous;
  CORBA::Boolean     pd_export;
  enum { ACTIVE, SHUTDOWN, NO_THREAD } pd_shutdown;
  tcpATMosRendezvouser *rendezvouser;
};

class tcpATMosOutgoingRope : public Rope {
public:

  virtual ~tcpATMosOutgoingRope();
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

  friend class tcpATMosMToutgoingFactory;

private:

  tcpATMosOutgoingRope(tcpATMosMToutgoingFactory* f,
			unsigned int maxStrands,
			tcpATMosEndpoint *e);
  // Create a tcpATMos outgoing Rope.
  //      The endpoint <e> is the remote endpoint and should contain the
  //      host name in FQDN form or as a dot separeted numeric string.
  // Concurrency Control:
  //    MUTEX = a->pd_lock
  // Pre-condition:
  //    Must hold <MUTEX> on entry
  // Post-condition:
  //    Still hold <MUTEX> on exit, even if an exception is raised

  tcpATMosEndpoint *remote;
};


class tcpATMosEndpoint : public Endpoint {
public:
  tcpATMosEndpoint(CORBA::Char *h,CORBA::UShort p);
  tcpATMosEndpoint(const tcpATMosEndpoint *e);
  tcpATMosEndpoint &operator=(const tcpATMosEndpoint &e);
  CORBA::Boolean operator==(const tcpATMosEndpoint *e);
  virtual ~tcpATMosEndpoint();
  CORBA::Char * host() const;
  void host(const CORBA::Char *p);
  CORBA::UShort port() const;
  void port(const CORBA::UShort p);
  static tcpATMosEndpoint *castup(const Endpoint* e);

  static const char* protocol_name;

private:
  CORBA::Char  *pd_host;
  CORBA::UShort pd_port;
  
  tcpATMosEndpoint();
};

#endif // __TCPATMos_H__
