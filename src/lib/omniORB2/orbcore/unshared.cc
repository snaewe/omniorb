// -*- Mode: C++; -*-
//                            Package   : omniORB2
// unshared.cc                Created on: 16/01/97
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:

// This module contains all the static data that cannot be shared by
// multiple processes. When this runtime is made into a DLL on NT or a
// shared library on ATMos, this module should not be linked in. Instead,
// it should be statically linked with each process.

/*
  $Log$
  Revision 1.2  1997/02/19 11:07:58  ewc
  Added support for Windows NT. Ensures that Naming stubs are linked into
  library.

  Revision 1.1  1997/01/23 16:59:32  sll
  Initial revision

 */

#include <omniORB2/CORBA.h>
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

omni_mutex          omniORB::initLock;
CORBA::Boolean      omniORB::orb_initialised = 0;
CORBA::Boolean      omniORB::boa_initialised = 0;

initFile*	    omniORB::configFile = 0;

omni_mutex          omniObject::objectTableLock;
omniObject*         omniObject::proxyObjectTable = 0;
omniObject**        omniObject::localObjectTable = 0;

proxyObjectFactory* proxyObjectFactory::proxyStubs = 0;

Anchor              Anchor::incomingAnchor;
Anchor              Anchor::outgoingAnchor;

CORBA::Boolean      tcpSocketRendezvous::has_spawned_rendevous_threads = 0;

size_t              GIOP_Basetypes::max_giop_message_size = 256 * 1024;

omni_mutex          LibcWrapper::non_reentrant;

_CORBA_Unbounded_Sequence_Octet omniORB::myPrincipalID;

static const CosNaming::NamingContext_proxyObjectFactory CosNaming_NamingContext_proxyObjectFactory1; // To ensure that Naming Stubs are linked.
