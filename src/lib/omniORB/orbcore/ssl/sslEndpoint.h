// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslEndpoint.h              Created on: 29 May 2001
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
  Revision 1.1.2.1  2001/06/11 18:11:06  sll
  *** empty log message ***


*/

#ifndef __SSLENDPOINT_H__
#define __SSLENDPOINT_H__

OMNI_NAMESPACE_BEGIN(omni)

class sslEndpoint : public giopEndpoint {
public:

  sslEndpoint(const IIOP::Address& address, sslContext* ctx);
  sslEndpoint(const char* address, sslContext* ctx);
  const char* type() const;
  const char* address() const;
  CORBA::Boolean bind();
  giopConnection* accept();
  void poke();
  void shutdown();

  ~sslEndpoint();

 private:
  tcpSocketHandle_t  pd_socket;
  IIOP::Address      pd_address;
  CORBA::String_var  pd_address_string;
  sslContext*        pd_ctx;

  sslEndpoint();
  sslEndpoint(const sslEndpoint&);
};

OMNI_NAMESPACE_END(omni)

#endif // __SSLENDPOINT_H__
