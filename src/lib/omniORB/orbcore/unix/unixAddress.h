// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixAddress.h              Created on: 19 Mar 2001
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
  Revision 1.1.4.3  2006/06/22 13:53:49  dgrisby
  Add flags to strand.

  Revision 1.1.4.2  2006/04/28 18:40:46  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:01:57  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.1  2001/08/06 15:47:44  sll
  Added support to use the unix domain socket as the local transport.

*/

#ifndef __UNIXADDRESS_H__
#define __UNIXADDRESS_H__

OMNI_NAMESPACE_BEGIN(omni)

class unixAddress : public giopAddress {
 public:

  unixAddress(const char* filename);
  const char* type() const;
  const char* address() const;
  giopAddress* duplicate() const;
  giopActiveConnection* Connect(unsigned long deadline_secs = 0,
				unsigned long deadline_nanosecs = 0,
				CORBA::ULong  strand_flags = 0) const;
  CORBA::Boolean Poke() const;
  ~unixAddress() {}

 private:
  CORBA::String_var  pd_address_string;
  CORBA::String_var  pd_filename;

  unixAddress();
  unixAddress(const unixAddress&);
};

OMNI_NAMESPACE_END(omni)

#endif // __UNIXADDRESS_H__
