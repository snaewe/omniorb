// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaBoa.cc                Created on: 6/2/96
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
//      Implementation of the BOA interface
//	

/*
  $Log$
  Revision 1.9  1999/05/25 17:17:41  sll
  Added check for invalid argument in static member functions.

  Revision 1.8  1999/03/11 16:25:51  djr
  Updated copyright notice

  Revision 1.7  1998/08/21 19:15:07  sll
  Added new command line option: -BOAno_bootstrap_agent. If this option
  is specified, do not initialise the special object that can respond to
  incoming request for initial object references.

  Revision 1.6  1998/08/14 13:43:39  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.5  1998/04/07 19:31:46  sll
  Replace cerr with omniORB::log.

  Revision 1.4  1997/12/09 18:19:09  sll
  New members BOA::impl_shutdown and BOA::destroy
  Merged code from orb.cc.
  Updated to use the new rope factory interfaces.

// Revision 1.3  1997/05/06  15:09:04  sll
// Public release.
//
 */

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <stdio.h>
#include <stdlib.h>

#include <bootstrap_i.h>
#include <ropeFactory.h>
#include <objectManager.h>
#ifndef __atmos__
#include <tcpSocket.h>
#define _tcpIncomingFactory tcpSocketMTincomingFactory
#define _tcpIncomingRope    tcpSocketIncomingRope
#define _tcpEndpoint        tcpSocketEndpoint
#else
#include <tcpATMos.h>
#define _tcpIncomingFactory tcpATMosMTincomingFactory
#define _tcpIncomingRope    tcpATMosIncomingRope
#define _tcpEndpoint        tcpATMosEndpoint
#endif
#include <scavenger.h>

#ifndef OMNIORB_USEHOSTNAME_VAR
#define OMNIORB_USEHOSTNAME_VAR "OMNIORB_USEHOSTNAME"
#endif

static CORBA::BOA_ptr     boa = 0;
static const char*        myBOAId = "omniORB2_BOA";
static omni_mutex         internalLock;
static omni_condition     internalCond(&internalLock);
static int                internalBlockingFlag = 0;
static omniObjectManager* rootObjectManager = 0;
static CORBA::Boolean     noBootStrapAgent = 0;

omniObjectManager*
omniObjectManager::root(CORBA::Boolean no_exception) throw (CORBA::OBJ_ADAPTER)
{
  if (!rootObjectManager) {
    if (!no_exception)
      throw CORBA::OBJ_ADAPTER(0,CORBA::COMPLETED_NO);
    else
      return 0;
  }
  return rootObjectManager;
}


class BOAobjectManager : public omniObjectManager {
public:
  ropeFactoryList* incomingRopeFactories() { return &pd_factories; }
  Rope* defaultLoopBack();

  BOAobjectManager();
  virtual ~BOAobjectManager() {}
private:
  ropeFactoryList  pd_factories;
  Rope*            pd_loopback;
};

BOAobjectManager::BOAobjectManager() : pd_loopback(0) {
  pd_factories.insert(new _tcpIncomingFactory);
}

Rope*
BOAobjectManager::defaultLoopBack()
{
  omni_mutex_lock sync(internalLock);
  if (!pd_loopback) {
    Endpoint* myaddr = 0;

    // Locate the incoming tcpSocket Rope, read its address and
    // use this address to create a new outgoing tcpSocket Rope.
    {
      ropeFactory_iterator iter(pd_factories);
      incomingRopeFactory* factory;
      while ((factory = (incomingRopeFactory*) iter())) {
	if (factory->getType()->is_protocol( _tcpEndpoint ::protocol_name))
	  {
	    Rope_iterator riter(factory);
	    _tcpIncomingRope * r = ( _tcpIncomingRope *) riter();
	    if (r) {
	      r->this_is(myaddr);
	    }
	    else {
	      // This is tough!!! Haven't got a loop back!
	      // May be the BOA has been destroyed!!!
	      throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
	    }
	  }
      }
    }

    {
      ropeFactory_iterator iter(globalOutgoingRopeFactories);
      outgoingRopeFactory* factory;
      while ((factory = (outgoingRopeFactory*) iter())) {
	if ((pd_loopback = factory->findOrCreateOutgoing((Endpoint*)myaddr))) {
	  break;
	}
      }
    }
    delete myaddr;
  }
  return pd_loopback;
}


static
CORBA::Boolean
parse_BOA_args(int &argc,char **argv,const char *orb_identifier);

CORBA::
BOA::BOA()
{
  pd_magic = CORBA::BOA::PR_magic;
}

CORBA::
BOA::~BOA()
{
  pd_magic = 0;
}

CORBA::BOA_ptr
CORBA::
ORB::BOA_init(int &argc, char **argv, const char *boa_identifier)
{
  omni_mutex_lock sync(internalLock);
  if (boa)
    return CORBA::BOA::_duplicate(boa);

  try {
    rootObjectManager = new BOAobjectManager;
    if (!parse_BOA_args(argc,argv,boa_identifier)) {
      throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
    }

    ropeFactory_iterator iter(*rootObjectManager->incomingRopeFactories());
    incomingRopeFactory* factory;

    while ((factory = (incomingRopeFactory*)iter())) {
      if (factory->getType()->is_protocol( _tcpEndpoint ::protocol_name)) {
	CORBA::Boolean install_iiop_port = 1;
	{
	  // Do not call factory->instantiateIncoming() while the
	  // Rope_iterator is still in scope. Otherwise deadlock will occur
	  Rope_iterator iter(factory);
	  if (iter() != 0) {
	    // iiop port has already been specified by -BOAiiop_port
	    install_iiop_port = 0;
	  }
	}
	if (install_iiop_port) {
	  CORBA::Char* hostname;
	  if ((hostname=(CORBA::Char*)getenv(OMNIORB_USEHOSTNAME_VAR))==NULL){
	    hostname = (CORBA::Char*)"";
	  }
	  _tcpEndpoint e (hostname,0);
	  // instantiate a rope. Let the OS pick a port number.
	  factory->instantiateIncoming(&e,1);
	}
      }
    }
    boa = new CORBA::BOA;
    if (!noBootStrapAgent) {
      	omniInitialReferences::singleton()->initialise_bootstrap_agentImpl();
    }
  }
  catch (...) {
    if (rootObjectManager) {
      delete rootObjectManager;
      rootObjectManager = 0;
    }
    if (boa) {
      delete boa;
      boa = 0;
    }
    throw;
  }
  return boa;
}

CORBA::BOA_ptr
CORBA::
BOA::getBOA()
{
  if (!boa) {
    throw CORBA::OBJ_ADAPTER(0,CORBA::COMPLETED_NO);
  }
  return CORBA::BOA::_duplicate(boa);
}

void
CORBA::
BOA::impl_is_ready(CORBA::ImplementationDef_ptr p,CORBA::Boolean NonBlocking)
{
  omni_mutex_lock sync(internalLock);
  internalBlockingFlag++;
  if (internalBlockingFlag == 1) {

    {
      // Beware of a possible deadlock where the scavenger thread and this
      // thread both try to grep the mutex in factory->anchor().
      // To prevent this from happening, put this block of code in a separate
      // scope.
      ropeFactory_iterator iter(*rootObjectManager->incomingRopeFactories());
      incomingRopeFactory* factory;
      while ((factory = (incomingRopeFactory*)iter())) {
	factory->startIncoming();
      }
    }
    StrandScavenger::initInScavenger();
  }
  if (!NonBlocking) {
    while (internalBlockingFlag > 0) {
      internalCond.wait();	// block here until impl_shutdown()
    }
  }
  else {
    if (internalBlockingFlag != 1) {
      internalBlockingFlag--;
    }
  }
  // If impl_is_ready() has been called, internalBlockingFlag is >=1.
  // If internalBlockFlag > 1, its value n indicates that n or n-1 threads
  // are blocking inside impl_is_ready().
  return;
}

void
CORBA::
BOA::impl_shutdown()
{
  omni_mutex_lock sync(internalLock);
  if (internalBlockingFlag > 0) {
    {
      // Beware of a possible deadlock where the scavenger thread and this
      // thread both try to grep the mutex in factory->anchor().
      // To prevent this from happening, put this block of code in a separate
      // scope.
      ropeFactory_iterator iter(*rootObjectManager->incomingRopeFactories());
      incomingRopeFactory* factory;
      while ((factory = (incomingRopeFactory*)iter())) {
	factory->stopIncoming();
      }
    }
    StrandScavenger::killInScavenger();
    while (internalBlockingFlag) {
      internalCond.signal();
      internalBlockingFlag--;
    }
  }
}

void
CORBA::
BOA::destroy()
{
  omni_mutex_lock sync(internalLock);
  {
    // Beware of a possible deadlock where the scavenger thread and this
    // thread both try to grep the mutex in factory->anchor().
    // To prevent this from happening, put this block of code in a separate
    // scope.
    ropeFactory_iterator iter(*rootObjectManager->incomingRopeFactories());
    incomingRopeFactory* factory;
    while ((factory = (incomingRopeFactory*)iter())) {
      if (internalBlockingFlag > 0) {
	factory->stopIncoming();
      }
      factory->removeIncoming();
    }
  }
  if (internalBlockingFlag > 0) {
    StrandScavenger::killInScavenger();
    while (internalBlockingFlag) {
      internalCond.signal();
      internalBlockingFlag--;
    }
  }
}


void
CORBA::
BOA::dispose(CORBA::Object_ptr p)
{
  omni::disposeObject(p->PR_getobj());
  return;
}

void 
CORBA::
BOA::obj_is_ready(Object_ptr op, ImplementationDef_ptr ip /* ignored */)
{
  omniObject *obj = op->PR_getobj();
  omni::objectIsReady(obj);
  return;
}


CORBA::BOA_ptr 
CORBA::
BOA::_duplicate(CORBA::BOA_ptr p)
{
  if (!PR_is_valid(p)) throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
  return p;
}

CORBA::BOA_ptr
CORBA::
BOA::_nil()
{
  return 0;
}

CORBA::Boolean
CORBA::is_nil(CORBA::BOA_ptr p)
{
  if (!CORBA::BOA::PR_is_valid(p))
    return 0;
  else
    return ((p==0) ? 1 : 0);
}

void
CORBA::release(CORBA::BOA_ptr p)
{
  return;
}

CORBA::Object_ptr
CORBA::
BOA::create(const CORBA::ReferenceData& ref,
	    CORBA::InterfaceDef_ptr intf,
	    CORBA::ImplementationDef_ptr impl)
{
  // XXX not implemented yet
  return 0;
}

CORBA::ReferenceData *
CORBA::
BOA::get_id(CORBA::Object_ptr obj)
{
  // XXX not implemented yet
  return 0;
}

void
CORBA::
BOA::change_implementation(CORBA::Object_ptr obj,
			   CORBA::ImplementationDef_ptr impl)
{
  // XXX not implemented yet
  return;
}

CORBA::Principal_ptr
CORBA::
BOA::get_principal(CORBA::Object_ptr obj, CORBA::Environment_ptr env)
{
  // XXX not implemented yet
  return 0;
}

void
CORBA::
BOA::deactivate_impl(CORBA::ImplementationDef_ptr impl)
{
  // XXX not implemented yet
  return;
}

void
CORBA::
BOA::deactivate_obj(CORBA::Object_ptr obj)
{
  // XXX not implemented yet
  return;
}

static
void
move_args(int& argc,char **argv,int idx,int nargs)
{
  if ((idx+nargs) <= argc)
    {
      for (int i=idx+nargs; i < argc; i++) {
	argv[i-nargs] = argv[i];
      }
      argc -= nargs;
    }
}

static
CORBA::Boolean
parse_BOA_args(int &argc,char **argv,const char *orb_identifier)
{
  CORBA::Boolean orbId_match = 0;
  if (orb_identifier && strcmp(orb_identifier,myBOAId) != 0)
    {
      if (omniORB::traceLevel > 0) {
	omniORB::log << "BOA_init failed: the BOAid ("
		     << orb_identifier << ") is not " <<  myBOAId << "\n";
	omniORB::log.flush();
      }
      return 0;
    }

  int idx = 1;
  while (argc > idx)
    {
      // -BOAxxxxxxxx ??
      if (strlen(argv[idx]) < 4 ||
	  !(argv[idx][0] == '-' && argv[idx][1] == 'B' &&
	    argv[idx][2] == 'O' && argv[idx][3] == 'A'))
	{
	  idx++;
	  continue;
	}
	  
      // -BOAid <id>
      if (strcmp(argv[idx],"-BOAid") == 0) {
	if ((idx+1) >= argc) {
	  if (omniORB::traceLevel > 0) {
	    omniORB::log << "BOA_init failed: missing -BOAid parameter.\n";
	    omniORB::log.flush();
	  }
	  return 0;
	}
	if (strcmp(argv[idx+1],myBOAId) != 0)
	  {
	    if (omniORB::traceLevel > 0) {
	      omniORB::log << "BOA_init failed: the BOAid ("
			   << argv[idx+1] << ") is not " << myBOAId << "\n";
	      omniORB::log.flush();
	    }
	    return 0;
	  }
	orbId_match = 1;
	move_args(argc,argv,idx,2);
	continue;
      }

      // -BOAiiop_port <port number>[,<port number>]*
      if (strcmp(argv[idx],"-BOAiiop_port") == 0) {
	if ((idx+1) >= argc) {
	  if (omniORB::traceLevel > 0) {
	    omniORB::log << "BOA_init failed: missing -BOAiiop_port parameter.\n";
	    omniORB::log.flush();
	  }
	  return 0;
	}
        unsigned int port;
	if (sscanf(argv[idx+1],"%u",&port) != 1 ||
            (port == 0 || port >= 65536)) {
	  if (omniORB::traceLevel > 0) {
	    omniORB::log << "BOA_init failed: invalid -BOAiiop_port parameter.\n";
	    omniORB::log.flush();
	  }
	  return 0;
	}

	CORBA::Char* hostname;
	if ((hostname = (CORBA::Char*)getenv(OMNIORB_USEHOSTNAME_VAR))==NULL) {
	  hostname = (CORBA::Char*)"";
	}
	try {
	  _tcpEndpoint e (hostname,(CORBA::UShort)port);
	  ropeFactory_iterator iter(*rootObjectManager->incomingRopeFactories());
	  incomingRopeFactory* factory;
	  while ((factory = (incomingRopeFactory*)iter())) {
	    if (factory->getType()->is_protocol( _tcpEndpoint ::protocol_name)) {
	      if (!factory->isIncoming(&e)) {
		// This port has not been instantiated
		factory->instantiateIncoming(&e,1);
		if (omniORB::traceLevel >= 2) {
		  omniORB::log << "Accept IIOP calls on port " << e.port()
			       << "\n";
		  omniORB::log.flush();
		}
	      }
	      break;
	    }
	  }
	}
	catch (...) {
	  if (omniORB::traceLevel > 0) {
	    omniORB::log << "BOA_init falied: cannot use port " << port
			 << " to accept incoming IIOP calls.\n";
	    omniORB::log.flush();
	  }
	  return 0;
	}
	move_args(argc,argv,idx,2);
	continue;
      }

      // -BOAno_bootstrap_agent
      if (strcmp(argv[idx],"-BOAno_bootstrap_agent") == 0) {
	noBootStrapAgent = 1;
	move_args(argc,argv,idx,1);
	continue;
      }

      // Reach here only if the argument in this form: -BOAxxxxx
      // is not recognised.
      if (omniORB::traceLevel > 0) {
	omniORB::log << "BOA_init failed: unknown BOA argument ("
		     << argv[idx] << ")\n";
	omniORB::log.flush();
      }
      return 0;
    }

  if (!orb_identifier && !orbId_match) {
    if (omniORB::traceLevel > 0) {
      omniORB::log << "BOA_init failed: BOAid is not specified.\n";
      omniORB::log.flush();
    }
    return 0;
  }
  return 1;
}
