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
  Revision 1.1.4.3  2006/04/28 18:40:46  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.2  2006/03/25 18:54:03  dgrisby
  Initial IPv6 support.

  Revision 1.1.4.1  2003/03/23 21:01:59  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.3  2001/07/31 16:16:23  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.2  2001/06/20 18:35:16  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.1  2001/06/11 18:11:07  sll
  *** empty log message ***

*/

#ifndef __SSLADDRESS_H__
#define __SSLADDRESS_H__

OMNI_NAMESPACE_BEGIN(omni)

class sslAddress : public giopAddress {
 public:

  sslAddress(const IIOP::Address& address, sslContext* ctx);
  const char* type() const;
  const char* address() const;
  giopAddress* duplicate() const;
  giopActiveConnection* Connect(unsigned long deadline_secs = 0,
				unsigned long deadline_nanosecs = 0,
				CORBA::ULong  strand_flags = 0) const;
  CORBA::Boolean Poke() const;
  ~sslAddress() {}

 private:
  IIOP::Address      pd_address;
  CORBA::String_var  pd_address_string;
  sslContext*        pd_ctx;

  sslAddress();
  sslAddress(const sslAddress&);
};

OMNI_NAMESPACE_END(omni)

#endif // __SSLADDRESS_H__
