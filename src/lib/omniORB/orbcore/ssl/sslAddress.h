// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslAddress.h               Created on: 29 May 2001
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
  Revision 1.1.2.1  2001/06/11 18:11:07  sll
  *** empty log message ***

*/

#ifndef __SSLADDRESS_H__
#define __SSLADDRESS_H__

OMNI_NAMESPACE_BEGIN(omni)

class sslAddress : public giopAddress {
 public:

  sslAddress(const IIOP::Address& address, sslContext* ctx);
  sslAddress(const char* address, sslContext* ctx);
  const char* type() const;
  const char* address() const;
  giopAddress* duplicate() const;
  giopConnection* connect(unsigned long deadline_secs = 0,
			  unsigned long deadline_nanosecs = 0) const;

  ~sslAddress() {}

 private:
  IIOP::Address      pd_address;
  CORBA::String_var  pd_address_string;
  sslContext*         pd_ctx;

  sslAddress();
  sslAddress(const sslAddress&);
};

OMNI_NAMESPACE_END(omni)

#endif // __SSLADDRESS_H__
