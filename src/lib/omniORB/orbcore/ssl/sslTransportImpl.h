// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslTransportImpl.h         Created on: 29 May 2001
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
  Revision 1.1.2.2  2001/08/23 16:02:58  sll
  Implement getInterfaceAddress().

  Revision 1.1.2.1  2001/06/11 18:11:05  sll
  *** empty log message ***

*/

#ifndef __SSLTRANSPORTIMPL_H__
#define __SSLTRANSPORTIMPL_H__

class sslContext;

OMNI_NAMESPACE_BEGIN(omni)

class sslTransportImpl : public giopTransportImpl {
 public:

  giopEndpoint*  toEndpoint(const char* param);
  giopAddress*   toAddress(const char* param);
  CORBA::Boolean isValid(const char* param);
  CORBA::Boolean addToIOR(const char* param);
  sslContext*    getContext() const { return pd_ctx; }
  const omnivector<const char*>* getInterfaceAddress();

  sslTransportImpl(sslContext* ctx);
  ~sslTransportImpl();

 private:

  sslContext*  pd_ctx;


  sslTransportImpl();
  sslTransportImpl(const sslTransportImpl&);
  sslTransportImpl& operator=(const sslTransportImpl&);
};

OMNI_NAMESPACE_END(omni)

#endif // __SSLTRANSPORTIMPL_H__
