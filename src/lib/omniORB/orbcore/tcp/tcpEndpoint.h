// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpEndpoint.h              Created on: 19 Mar 2001
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
  Revision 1.1.2.3  2001/07/13 15:34:24  sll
  Added the ability to monitor connections and callback to the giopServer when
  data has arrived at a connection.

  Revision 1.1.2.2  2001/06/20 18:35:15  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.1  2001/04/18 18:10:43  sll
  Big checkin with the brand new internal APIs.

*/

#ifndef __TCPENDPOINT_H__
#define __TCPENDPOINT_H__

#include <omniORB4/tracedthread.h>

OMNI_NAMESPACE_BEGIN(omni)

class tcpConnection;

class tcpEndpoint : public giopEndpoint {
public:

  tcpEndpoint(const IIOP::Address& address);
  tcpEndpoint(const char* address);
  const char* type() const;
  const char* address() const;
  CORBA::Boolean Bind();
  giopConnection* AcceptAndMonitor(notifyReadable_t,void*);
  void Poke();
  void Shutdown();

  ~tcpEndpoint();

  friend class tcpConnection;

  static unsigned long scan_interval_sec;
  static unsigned long scan_interval_nsec;
  static CORBA::ULong  hashsize;
  
 private:
  tcpSocketHandle_t    pd_socket;
  IIOP::Address        pd_address;
  CORBA::String_var    pd_address_string;
  tcpSocketHandleSet_t pd_fdset_1;
  tcpSocketHandleSet_t pd_fdset_2;
  tcpSocketHandleSet_t pd_fdset_dib; // data in buffer
  int                  pd_n_fdset_1;
  int                  pd_n_fdset_2;
  int                  pd_n_fdset_dib;
  omni_tracedmutex     pd_fdset_lock;
  unsigned long        pd_abs_sec;
  unsigned long        pd_abs_nsec;
  tcpConnection**      pd_hash_table;      

  tcpEndpoint();
  tcpEndpoint(const tcpEndpoint&);

  void notifyReadable(tcpSocketHandle_t,
		      giopEndpoint::notifyReadable_t func,
		      void*);

};


OMNI_NAMESPACE_END(omni)

#endif // __TCPENDPOINT_H__
