// -*- Mode: C++; -*-
//                            Package   : omniORB2
// gatekeeper.cc              Created on: 12/12/1997
//                            Author    : Sai-Lai Lo (sll)
//
//    Copyright (C) 1997 Olivetti & Oracle Research Laboratory
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
//	A dummy gatekeeper implementation. Accept all connections.
//

#include <omniORB2/CORBA.h>
#include <gatekeeper.h>

const char*
gateKeeper::version()
{
  return (const char*)"not installed. All incoming are accepted.";
}


CORBA::Boolean 
gateKeeper::checkConnect(tcpSocketStrand *s)
{
  return 1;
}

char *hosts_deny_table = "";
char *hosts_allow_table = "";

char *&gateKeeper::denyFile = hosts_deny_table;
char *&gateKeeper::allowFile = hosts_allow_table;
