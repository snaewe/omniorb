// -*- Mode: C++; -*-
//                            Package   : omniORB2
// unshared.cc                Created on: 16/01/97
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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

// This module contains all the static data that cannot be shared by
// multiple processes. When this runtime is made into a DLL on NT or a
// shared library on ATMos, this module should not be linked in. Instead,
// it should be statically linked with each process.

/*
  $Log$
  Revision 1.8  1998/08/14 13:55:20  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.7  1997/09/20 17:00:34  dpg1
  Added LifeCycle support hash table.

// Revision 1.6  1997/08/26  15:29:25  sll
// Added initFile.h include.
//
  Revision 1.5  1997/05/06 15:32:58  sll
  Public release.

 */

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <initFile.h>
#include "libcWrapper.h"
#if defined(UnixArchitecture)
#include "tcpSocket_UNIX.h"
#elif defined(ATMosArchitecture)
#include "tcpSocket_ATMos.h"
#elif defined(NTArchitecture)
#include "tcpSocket_NT.h"
#else
#error "No tcpSocket.h header for this architecture."
#endif


CORBA::ORB          CORBA::ORB::orb;

CORBA::BOA          CORBA::BOA::boa;

CORBA::Object       CORBA::Object::CORBA_Object_nil;

omni_mutex          omni::initLock;
CORBA::Boolean      omni::orb_initialised = 0;
CORBA::Boolean      omni::boa_initialised = 0;

CORBA::ULong        omniORB::traceLevel = 1;

CORBA::Boolean      omniORB::strictIIOP = 0;

initFile*	    omni::configFile = 0;

omni_mutex          omniObject::objectTableLock;
omniObject*         omniObject::proxyObjectTable = 0;
omniObject**        omniObject::localObjectTable = 0;

omni_mutex          omniObject::wrappedObjectTableLock;
_wrap_proxy**        omniObject::wrappedObjectTable = 0;

proxyObjectFactory* proxyObjectFactory::proxyStubs = 0;

Anchor              Anchor::incomingAnchor;
Anchor              Anchor::outgoingAnchor;

CORBA::Boolean      tcpSocketRendezvous::has_spawned_rendevous_threads = 0;

size_t              GIOP_Basetypes::max_giop_message_size = 256 * 1024;

omni_mutex          LibcWrapper::non_reentrant;

_CORBA_Unbounded_Sequence_Octet omni::myPrincipalID;

omniORB::objectKey       omniORB::seed;

static const CosNaming::NamingContext_proxyObjectFactory CosNaming_NamingContext_proxyObjectFactory1; // To ensure that Naming Stubs are linked.


