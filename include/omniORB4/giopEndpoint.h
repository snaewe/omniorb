// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopEndpoint.h               Created on: 20 Dec 2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
  Revision 1.1.4.7  2002/03/19 15:42:03  dpg1
  Use list of IP addresses to pick a non-loopback interface if there is one.

  Revision 1.1.4.6  2001/12/03 13:39:54  dpg1
  Explicit socket shutdown flag for Windows.

  Revision 1.1.4.5  2001/08/23 16:00:34  sll
  Added method in giopTransportImpl to return the addresses of the host
  interfaces.

  Revision 1.1.4.4  2001/07/31 16:16:27  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.4.3  2001/07/13 15:15:04  sll
  Replaced giopEndpoint::Accept with giopEndpoint::AcceptAndMonitor.
  giopConnection is now reference counted.
  Added setSelectable, clearSelectable and Peek to giopConnection.

  Revision 1.1.4.2  2001/06/20 18:35:18  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.4.1  2001/04/18 17:26:29  sll
  Big checkin with the brand new internal APIs.

*/

#ifndef __GIOPENDPOINT_H__
#define __GIOPENDPOINT_H__

#include <omniORB4/omniutilities.h>

OMNI_NAMESPACE_BEGIN(omni)

class giopActiveConnection;   // Active in the sense that it is created with 
                              // Connect()
class giopConnection;
class giopEndpoint;
class giopActiveCollection;   // Singleton to act on a bunch of 
                              // giopActiveConnection.
class giopServer;

class giopConnection {
public:
  typedef void (*notifyReadable_t)(void* cookie,giopConnection* conn);

  // None of the members raise an exception.

  virtual int Send(void* buf, size_t sz,
		   unsigned long deadline_secs = 0,
		   unsigned long deadline_nanosecs = 0) = 0;
  virtual int Recv(void* buf, size_t sz,
		   unsigned long deadline_secs = 0,
		   unsigned long deadline_nanosecs = 0) = 0;
  virtual void Shutdown() = 0;

  virtual const char* myaddress() = 0;
  virtual const char* peeraddress() = 0;


  virtual void setSelectable(_CORBA_Boolean now = 0,
			     _CORBA_Boolean data_in_buffer = 0) = 0;
  // Indicates that this connection should be watched by a select()
  // so that any new data arriving on the connection will be noted.
  // If now == 1, immediately make this connection part of the select
  // set.
  // If data_in_buffer == 1, treat this connection as if there are
  // data available from the connection already.


  virtual void clearSelectable() = 0;
  // Indicates that this connection need not be watched any more.

  virtual void Peek(notifyReadable_t func,void* cookie) = 0;
  // Do nothing and returns immediately if the connection has not been
  // set to be watched by a previous setSelectable().
  // Otherwise, monitor the connection's status for a short time.
  // If data have arrived at the connection, call the callback function
  // <func> with the <cookie> and the pointer to the connection as the
  // arguments. This function is similar to giopEndpoint::AcceptAndMonitor
  // except that it only monitor one connection.

  giopConnection() : pd_refcount(1), pd_dying(0), 
		     pd_has_dedicated_thread(0), 
		     pd_dedicated_thread_in_upcall(0),
                     pd_n_workers(0),
                     pd_has_hit_n_workers_limit(0) {}

  int decrRefCount(_CORBA_Boolean forced=0);
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock unless forced == 1.


  void incrRefCount();
  // Thread Safety preconditions:
  //    Caller must hold omniTransportLock.

  friend class giopServer;

protected:
  virtual ~giopConnection() {}

private:

  int            pd_refcount;

  _CORBA_Boolean pd_dying;
  // Initialised to 0. Read and write by giopServer exclusively.

  _CORBA_Boolean pd_has_dedicated_thread;
  // Initialised to 0. Read and write by giopServer exclusively.

  _CORBA_Boolean pd_dedicated_thread_in_upcall;
  // Initialised to 0. Read and write by giopServer exclusively.

  int            pd_n_workers;
  // Initialised to 0. Read and write by giopServer exclusively.

  _CORBA_Boolean pd_has_hit_n_workers_limit;
  // Initialised to 0. Read and write by giopServer exclusively.

  giopConnection(const giopConnection&);
  giopConnection& operator=(const giopConnection&);
};


class giopAddress {
public:
  // Each giopAddress must register via decodeIOR interceptor if it
  // wants to decode its own IOR component.

  // None of the members raise an exception.

  static giopAddress* str2Address(const char* address);
  // Given a string, returns an instance that can be used to connect to
  // the address.
  // The format of the address string is as follows:
  //     giop:<transport name>:[<transport specific fields]+
  //     ^^^^^^^^^^^^^^^^^^^^^
  //        transport identifier
  //
  // The format of the following transports are defined (but may not be
  // implemented yet):
  //
  //   giop:tcp:<hostname>:<port no.>
  //   giop:ssl:<hostname>:<port no.>
  //   giop:unix:<filename>
  //   giop:fd:<file no.>
  //
  // Returns 0 if no suitable endpoint can be created.

  virtual const char* type() const = 0;
  // return the transport identifier, e.g. "giop:tcp","giop:ssl", etc.

  virtual const char* address() const = 0;
  // return the string that describe this remote address.
  // The string format is described in str2Address().

  virtual giopActiveConnection* Connect(unsigned long deadline_secs = 0,
				        unsigned long deadline_nanosecs = 0)const =0;
  // Connect to the remote address.
  // Return 0 if no connection can be established.

  virtual giopAddress* duplicate() const = 0;
  // Return an identical instance.

  giopAddress() {}
  virtual ~giopAddress() {}

  static giopAddress* fromTcpAddress(const IIOP::Address& addr);
  static giopAddress* fromSslAddress(const IIOP::Address& addr);

private:
  giopAddress(const giopAddress&);
  giopAddress& operator= (const giopAddress&);

};

typedef omnivector<giopAddress*>  giopAddressList;


class giopEndpoint {
public:
  // None of the members raise an exception.

  static giopEndpoint* str2Endpoint(const char* endpoint);
  // Given a string, returns an instance that represent the endpoint
  // The format of an endpoint string is as follows:
  //     giop:<transport name>:[<transport specific fields]+
  //     ^^^^^^^^^^^^^^^^^^^^^
  //        transport identifier
  //
  // The format of the following transports are defined (but may not be
  // implemented yet):
  //
  //   giop:tcp:<hostname>:<port no.>      note 1
  //   giop:ssl:<hostname>:<port no.>      note 1
  //   giop:unix:<filename>
  //   giop:fd:<file no.>
  //
  // Note 1: if <hostname> is empty, the IP address of one of the host
  //         network interface will be used.
  //         if <port no.> is not present, a port number is chosen by
  //         the operation system.
  //
  // Returns 0 if no suitable endpoint can be created.

  static _CORBA_Boolean strIsValidEndpoint(const char* endpoint);
  // Return true if endpoint is syntactically correct as described
  // in str2Endpoint(). None of the fields are optional.

  static _CORBA_Boolean addToIOR(const char* endpoint);
  // Return true if the endpoint has been sucessfully registered so that
  // all IORs generated by the ORB will include this endpoint.

  virtual const char* type() const = 0;
  // return the transport identifier

  virtual const char* address() const = 0;
  // return the string that describe this endpoint.
  // The string format is described in str2Endpoint().

  virtual _CORBA_Boolean Bind() = 0;
  // Establish a binding to the this address.
  // Return TRUE(1) if the binding has been established successfully,
  // otherwise returns FALSE(0).

  virtual giopConnection* 
          AcceptAndMonitor(giopConnection::notifyReadable_t func,
			   void* cookie) = 0;
  // Accept a new connection. Returns 0 if no connection can be accepted.
  // In addition, for all the connections of this endpoint that has been
  // marked, monitors their status.  If data have arrived at a connection,
  // call the callback function <func> with the <cookie> and the pointer to
  // the connection as the arguments.

  virtual void Poke() = 0;
  // Call to unblock any thread blocking in accept().

  virtual void Shutdown() = 0;
  // Remove the binding.

  giopEndpoint() {}
  virtual ~giopEndpoint() {}

private:
  giopEndpoint(const giopEndpoint&);
  giopEndpoint& operator=(const giopEndpoint&);
};

typedef omnivector<giopEndpoint*>  giopEndpointList;


class giopActiveConnection {
public:
  // 'Active' in the sense that this is created with Connect().
  virtual giopActiveCollection* registerMonitor() = 0;

  virtual giopConnection& getConnection() = 0;

  // This class could have been written to inherit from giopConnection and
  // with just one abstract function added. The trouble is if we do so,
  // giopConnection must be a public virtual base class. If that is
  // the case, then we cannot simply cast a giopConnection* back to its
  // implementation class. Instead dynamic_cast has to be used. This is not
  // what we want to do.

  giopActiveConnection() {}
  virtual ~giopActiveConnection() {}

private:
  giopActiveConnection(const giopActiveConnection&);
  giopActiveConnection& operator=(const giopActiveConnection&);
};


class giopActiveCollection {
public:
  // A singleton to act on a bunch of giopActiveConnection of the same
  // transport type.

  virtual const char* type() const = 0;

  virtual void Monitor(giopConnection::notifyReadable_t func,void* cookie) = 0;
  // For all the connections that are associated with this singleton and
  // have previously been registered via registerMonitor(), watches their
  // status. If data have arrived at a connection, call the callback
  // function <func> with the <cookie> and the pointer to the connection as
  // the arguments.  This function will only returns when there is no
  // connection to monitor, i.e. all the connections that were registered
  // have been deleted.

  virtual _CORBA_Boolean isEmpty() const = 0;
  // Returns TRUE(1) if no connections have been added via registerMonitor().

  giopActiveCollection() {}
  virtual ~giopActiveCollection() {}

private:
  giopActiveCollection(const giopActiveCollection&);
  giopActiveCollection& operator=(const giopActiveCollection&);
};


class giopTransportImpl {
public:

  virtual giopEndpoint* toEndpoint(const char* param) = 0;
  // Returns the endpoint object for this endpoint if it is recognised by
  // this transport.

  virtual giopAddress*   toAddress(const char* param) = 0;
  // Returns the address object for this address if it is recognised by
  // this transport.

  virtual _CORBA_Boolean isValid(const char* param) = 0;
  // Returns 1 if the address/endpoint is recognised by this transport

  virtual _CORBA_Boolean addToIOR(const char* param) = 0;
  // Make this endpoint part of the IORs created by this ORB.

  virtual const omnivector<const char*>* getInterfaceAddress() = 0;
  // Get the addresses of all the interfaces that can be used to talk to
  // this host using this transport.

  virtual void initialise();
  // Initialise the transport implementation. Called once the 1st time
  // ORB_init() is called.

  static const omnivector<const char*>* getInterfaceAddress(const char* type);
  // Get the addresses of all the interfaces that belongs to the transport
  // type <type>. These addresses can be used to talk to this host.
  // e.g. type == "giop:tcp" causes the tcp transport implementation to
  // returns the IP address of all the network interfaces of this host.
  // If <type> does not match returns 0.

  const char*        type;
  giopTransportImpl* next;

  giopTransportImpl(const char* t);
  virtual ~giopTransportImpl();

private:
  giopTransportImpl();
  giopTransportImpl(const giopTransportImpl&);
  giopTransportImpl& operator=(const giopTransportImpl&);
};


OMNI_NAMESPACE_END(omni)

#endif // __GIOPENDPOINT_H__
