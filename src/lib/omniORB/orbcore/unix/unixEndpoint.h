// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixEndpoint.h             Created on: 6 Aug 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
  Revision 1.1.4.1  2003/03/23 21:01:57  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.2  2002/08/21 06:23:16  dgrisby
  Properly clean up bidir connections and ropes. Other small tweaks.

  Revision 1.1.2.1  2001/08/06 15:47:45  sll
  Added support to use the unix domain socket as the local transport.

*/

#ifndef __UNIXENDPOINT_H__
#define __UNIXENDPOINT_H__

OMNI_NAMESPACE_BEGIN(omni)

class unixConnection;

class unixEndpoint : public giopEndpoint, public SocketCollection {
public:

  unixEndpoint(const char* filename);

  // The following implement giopEndpoint abstract functions
  const char* type() const;
  const char* address() const;
  CORBA::Boolean Bind();
  giopConnection* AcceptAndMonitor(giopConnection::notifyReadable_t,void*);
  void Poke();
  void Shutdown();

  ~unixEndpoint();

protected:
  CORBA::Boolean notifyReadable(SocketHandle_t);
  // implement SocketCollection::notifyReadable
  

private:
  SocketHandle_t       pd_socket;
  CORBA::String_var    pd_filename;
  CORBA::String_var    pd_address_string;

  SocketHandle_t                   pd_new_conn_socket;
  giopConnection::notifyReadable_t pd_callback_func;
  void*                            pd_callback_cookie;

  unixEndpoint();
  unixEndpoint(const unixEndpoint&);
  unixEndpoint& operator=(const unixEndpoint&);
};


class unixActiveConnection;

class unixActiveCollection : public giopActiveCollection, 
			     public SocketCollection {
public:
  const char* type() const;
  // implement giopActiveCollection::type

  void Monitor(giopConnection::notifyReadable_t func, void* cookie);
  // implement giopActiveCollection::Monitor

  CORBA::Boolean isEmpty() const;
  // implement giopActiveCollection::isEmpty

  void deactivate();
  // implement giopActiveCollection::deactivate

  unixActiveCollection();
  ~unixActiveCollection();

  friend class unixActiveConnection;

protected:
  CORBA::Boolean notifyReadable(SocketHandle_t);
  // implement SocketCollection::notifyReadable

  void addMonitor(SocketHandle_t);
  void removeMonitor(SocketHandle_t);

private:
  CORBA::ULong      pd_n_sockets;
  CORBA::Boolean    pd_shutdown;
  omni_tracedmutex  pd_lock;

  giopConnection::notifyReadable_t pd_callback_func;
  void*                            pd_callback_cookie;

  unixActiveCollection(const unixActiveCollection&);
  unixActiveCollection& operator=(const unixActiveCollection&);
};

OMNI_NAMESPACE_END(omni)

#endif // __UNIXENDPOINT_H__
