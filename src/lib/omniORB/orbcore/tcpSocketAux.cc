// -*- Mode: C++; -*-
//                            Package   : omniORB2
// tcpSocketAux.cc            Created on: 28/9/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//	

/* $Log$
/* Revision 1.1.4.1  2000/09/27 17:30:31  sll
/* *** empty log message ***
/*
   Revision 1.1.2.2  1999/11/04 20:20:23  sll
   GIOP engines can now do callback to the higher layer to calculate total
   message size if necessary.
   Where applicable, changed to use the new server side descriptor-based stub.

   Revision 1.1.2.1  1999/10/02 18:21:31  sll
   Added support to decode optional tagged components in the IIOP profile.
   Added support to negogiate with a firewall proxy- GIOPProxy to invoke
   remote objects inside a firewall.
   Added tagged component TAG_ORB_TYPE to identify omniORB IORs.
*/

#include <omniORB4/CORBA.h>
#include <ropeFactory.h>
#include <tcpSocket.h>

//#include <firewallProxy.h>
//#include <sslMTFactory.h>

#define LOGMESSAGE(level,prefix,message) do {\
   if (omniORB::trace(level)) {\
     omniORB::logger log;\
	log << " " ## prefix ## ": " ## message ## "\n";\
   }\
} while (0)


Rope*
tcpSocketMToutgoingFactory::auxillaryTransportLookup(omniIOR* ior)
{
  LOGMESSAGE(15,"auxillaryTransportLookup","");

#if 0  
  Rope* r;

  r = firewallProxyRopeFactory::singleton()->findOrCreateOutgoing(ior);
  if (r) return r;

#endif

#ifdef ENABLE_SSL_TRANSPORT
  r = sslOutgoingRopeFactory::singleton()->findOrCreateOutgoing(ior);
  if (r) return r;
#endif

  return 0;
}
