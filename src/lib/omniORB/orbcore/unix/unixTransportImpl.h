// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixTransportImpl.h        Created on: 6 Aug 2001
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

  Revision 1.1.2.1  2001/08/06 15:47:45  sll
  Added support to use the unix domain socket as the local transport.

*/

#ifndef __UNIXTRANSPORTIMPL_H__
#define __UNIXTRANSPORTIMPL_H__

OMNI_NAMESPACE_BEGIN(omni)

class unixTransportImpl : public giopTransportImpl {
 public:

  unixTransportImpl();
  ~unixTransportImpl();
  
  giopEndpoint*  toEndpoint(const char* param);
  giopAddress*   toAddress(const char* param);
  CORBA::Boolean isValid(const char* param);
  CORBA::Boolean addToIOR(const char* param);
  const omnivector<const char*>* getInterfaceAddress();

 private:
  unixTransportImpl(const unixTransportImpl&);
  unixTransportImpl& operator=(const unixTransportImpl&);
};

OMNI_NAMESPACE_END(omni)

#endif // __UNIXTRANSPORTIMPL_H__
