// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpAddress.h               Created on: 19 Mar 2001
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
  Revision 1.1.2.1  2001/04/18 18:10:44  sll
  Big checkin with the brand new internal APIs.

*/

#ifndef __TCPADDRESS_H__
#define __TCPADDRESS_H__

OMNI_NAMESPACE_BEGIN(omni)

class tcpAddress : public giopAddress {
 public:

  tcpAddress(const IIOP::Address& address);
  tcpAddress(const char* address);
  const char* type() const;
  const char* address() const;
  giopAddress* duplicate() const;
  giopConnection* connect(unsigned long deadline_secs = 0,
			  unsigned long deadline_nanosecs = 0) const;

  ~tcpAddress() {}

 private:
  IIOP::Address      pd_address;
  CORBA::String_var  pd_address_string;

  tcpAddress();
  tcpAddress(const tcpAddress&);
};

OMNI_NAMESPACE_END(omni)

#endif // __TCPADDRESS_H__
