// -*- Mode: C++; -*-
//                            Package   : omniORB
// corbaOrb.cc                Created on: 6/2/96
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
//      Implementation of the ORB interface
//

/*
  $Log$
  Revision 1.29.6.1  1999/09/22 14:26:45  djr
  Major rewrite of orbcore to support POA.

  Revision 1.28  1999/08/30 16:53:04  sll
  Added new options -ORBclientCallTimeOutPeriod, -ORBserverCallTimeOutPeriod
  -ORBscanOutgoingPeriod, -ORBscanIncomingPeriod and -ORBhelp.

  Revision 1.25  1999/06/26 18:05:36  sll
  New options -ORBabortOnInternalError, -ORBverifyObjectExistsAndType.

  Revision 1.24  1999/05/25 17:20:44  sll
  Added check for invalid arguments in static member functions.
  Default to throw system exception in the DII interface.
  resolve_initial_reference now deals with InterfaceRepository.

  Revision 1.23  1999/05/22 17:40:57  sll
  Added #ifdef __CIAO__ so that CCia would not complain about gnu/linux
  particulars.

  Revision 1.22  1999/03/19 15:18:00  djr
  New option acceptMisalignedIndirections

  Revision 1.21  1999/03/11 16:25:52  djr
  Updated copyright notice

  Revision 1.20  1999/02/18 15:15:12  djr
  omniORB::strictIIOP if now true by default.
  New runtime option -ORBlcdMode  (omniORB::enableLcdMode())

  Revision 1.19  1999/02/01 15:16:14  djr
  Replace copy-initialisation of _var types with direct initialisation.

  Revision 1.18  1999/01/07 15:33:34  djr
  New configuration variable omniORB::diiThrowsSysExceptions
  New command line options -ORBdiiThrowsSysExceptions
                           -ORBinConScanPeriod
                           -ORBoutConScanPeriod

  Revision 1.17  1998/08/26 21:52:29  sll
  Added configuration variable omniORB::maxTcpConnectionPerServer.

  Revision 1.16  1998/08/21 19:13:32  sll
  Use omniInitialReferences::singleton() to manage initial object references.
  New command line options: -ORBInitialHost, -ORBInitialPort.

  Revision 1.15  1998/08/14 13:44:57  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.14  1998/04/18 10:09:52  sll
  Make the definition of omniORB::serverName consistent with omniORB.h

  Revision 1.13  1998/04/08 16:06:25  sll
  Added support for Reliant UNIX 5.43.

  Revision 1.12  1998/04/07 19:32:38  sll
  Replace cerr with omniORB::log.
  Use namespace and bool type if available.

// Revision 1.11  1998/03/04  15:21:39  ewc
// giopServerThreadWrapper singelton initialised
//
// Revision 1.10  1998/02/26  10:41:18  sll
// Parse_ORB_args once again handles the case when argv is a NULL pointer
// properly.
//
// Revision 1.9  1998/01/27  15:32:14  ewc
// Added -ORBtcAliasExpand flag
//
  Revision 1.8  1997/12/12 18:42:24  sll
  New command line option to set omniORB::serverName.

  Revision 1.7  1997/12/09 18:29:46  sll
  Merged in code from orb.cc
  Updated to use the new rope factory interface.
  Old code to deal with server side thread management removed.

// Revision 1.6  1997/05/06  15:11:03  sll
// Public release.
//
 */

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <corbaOrb.h>
#include <initFile.h>
#include <scavenger.h>
#include <bootstrap_i.h>
#include <omniORB3/omniObjRef.h>
#include <poaimpl.h>
#include <ropeFactory.h>
#ifndef __atmos__
#include <tcpSocket.h>
#define _tcpOutgoingFactory tcpSocketMToutgoingFactory
#else
#include <tcpATMos.h>
#define _tcpOutgoingFactory tcpATMosMToutgoingFactory
#endif
#ifdef _HAS_SIGNAL
#include <dynamicLib.h>
#include <signal.h>
#include <errno.h>
#endif
#include <stdio.h>


#define MY_ORB_ID           "omniORB3"
#define OLD_ORB_ID          "omniORB2"


static omniOrbORB*          the_orb              = 0;
static omni_tracedmutex     orb_lock;
static omni_tracedcondition orb_signal(&orb_lock);
static volatile int         orb_n_blocked_in_run = 0;

static char*             bootstrapAgentHostname  = 0;
static CORBA::UShort     bootstrapAgentPort      = 900;


#ifdef __SINIX__
// Why haven't we got this signature from signal.h? - sll
//
extern "C" int sigaction(int, const struct sigaction *, struct sigaction *);
#endif


static CORBA::Boolean
parse_ORB_args(int& argc, char** argv, const char* orb_identifier);

//////////////////////////////////////////////////////////////////////
///////////////////////////// CORBA::ORB /////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::ORB::~ORB()  {}


CORBA::ORB_ptr
CORBA::ORB::_duplicate(CORBA::ORB_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}


CORBA::ORB_ptr
CORBA::ORB::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) || !obj->_NP_is_pseudo() )  return _nil();

  ORB_ptr p = (ORB_ptr) obj->_ptrToObjRef(_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}


static omniOrbORB the_nil_orb(1);


CORBA::ORB_ptr
CORBA::ORB::_nil()
{
  return &the_nil_orb;
}


const char*
CORBA::ORB::_PD_repoId = "IDL:omg.org/CORBA/ORB:1.0";


CORBA::ORB_ptr
CORBA::ORB_init(int& argc, char** argv, const char* orb_identifier)
{
  omni_tracedmutex_lock sync(orb_lock);

  if( !parse_ORB_args(argc,argv,orb_identifier) ) {
    throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
  }
  if( the_orb ) {
    the_orb->incrRefCount();
    return the_orb;
  }

  if( (char*) omniORB::serverName == 0 )
    omniORB::serverName = CORBA::string_dup("unknown");

  try {
    // initialise object tables
    omni::globalInit();

    if( !globalOutgoingRopeFactories ) {
      // Now initialise all the rope factories that will be used to
      // create outgoing ropes.
      globalOutgoingRopeFactories = new ropeFactoryList;
      globalOutgoingRopeFactories->insert(new _tcpOutgoingFactory);
    }

    // Add rope factories for other transports here.

    // get configuration information:
    {
      initFile configFile;
      configFile.initialize();
    }

    if( bootstrapAgentHostname ) {
      // The command-line option -ORBInitialHost has been specified.
      // Override any previous NamesService object reference
      // that may have been read from the configuration file.
      omniInitialReferences::set("NameService", CORBA::Object::_nil());
      omniInitialReferences::initialise_bootstrap_agent(bootstrapAgentHostname,
							bootstrapAgentPort);
    }

    // myPrincipalID, to be used in the principal field of IIOP calls
    CORBA::ULong l = strlen("nobody") + 1;
    CORBA::Octet *p = (CORBA::Octet *) "nobody";
    omni::myPrincipalID.length(l);
    unsigned int i;
    for (i=0; i < l; i++)
      omni::myPrincipalID[i] = p[i];

#ifdef _HAS_SIGNAL
#ifndef _USE_MACH_SIGNAL
#  ifndef __SINIX__
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = SIG_IGN;
    act.sa_flags = 0;
    if (sigaction(SIGPIPE,&act,0) < 0) {
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "WARNING -- ORB_init() cannot install the\n"
	  " SIG_IGN handler for signal SIGPIPE. (errno = " << errno << ")\n";
      }
    }
#  else
    // SINUX
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = (void (*)())0;
    act.sa_flags = 0;
    if (sigaction(SIGPIPE,&act,0) < 0) {
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "WARNING -- ORB_init() cannot install the\n"
	  " SIG_IGN handler for signal SIGPIPE. (errno = " << errno << ")\n";
      }
    }
#  endif
#else
    struct sigvec act;
    act.sv_mask = 0;
    act.sv_handler = SIG_IGN;
    act.sv_flags = 0;
    if (sigvec(SIGPIPE,&act,0) < 0) {
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "WARNING -- ORB_init() cannot install the\n"
	  " SIG_IGN handler for signal SIGPIPE. (errno = " << errno << ")\n";
      }
    }
#endif
#endif // _HAS_SIGNAL

#ifdef __WIN32__

    // Initialize WinSock:

    WORD versionReq;
    WSADATA wData;
    versionReq = MAKEWORD(1, 1);  // Nothing specific to releases > 1.1 used

    int rc = WSAStartup(versionReq, &wData);

    if (rc != 0)
      // Couldn't find a usable DLL.
      throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);

    // Confirm that the returned Windows Sockets DLL supports 1.1

    if( LOBYTE( wData.wVersion ) != 1 ||
	HIBYTE( wData.wVersion ) != 1 ) {
      // Couldn't find a usable DLL
      WSACleanup();
      throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
    }

#endif

    // Initialise a giopServerThreadWrapper singelton
    omniORB::giopServerThreadWrapper::setGiopServerThreadWrapper(
					 new omniORB::giopServerThreadWrapper);

    // Start the thread to cleanup idle outgoing strands.
    StrandScavenger::initOutScavenger();

    // Attach the dynamic library hook, and initialise.
    if( omniDynamicLib::hook )
      omniDynamicLib::ops = omniDynamicLib::hook;
    omniDynamicLib::ops->init();
  }
  catch (CORBA::INITIALIZE &ex) {
    throw;
  }
  catch (...) {
    throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
  }

  the_orb = new omniOrbORB(0);
  the_orb->incrRefCount();
  return the_orb;
}

//////////////////////////////////////////////////////////////////////
///////////////////////////// omniOrbORB /////////////////////////////
//////////////////////////////////////////////////////////////////////

#define CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED()  \
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();  \
  if( pd_destroyed )  throw CORBA::OBJECT_NOT_EXIST(0, CORBA::COMPLETED_NO);  \
  if( pd_shutdown  )  throw CORBA::BAD_INV_ORDER(CORBA::OMGVMCID | 4,  \
						 CORBA::COMPLETED_NO);  \


omniOrbORB::~omniOrbORB()  {}


omniOrbORB::omniOrbORB(int nil)
  : ORB(nil),
    pd_refCount(1),
    pd_destroyed(0),
    pd_shutdown(0),
    pd_shutdown_in_progress(0)
{
}


char*
omniOrbORB::object_to_string(CORBA::Object_ptr obj)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  if( obj && obj->_NP_is_pseudo() )
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_NO);

  return omni::objectToString(obj ? obj->_PR_getobj() : 0);
}


CORBA::Object_ptr
omniOrbORB::string_to_object(const char* sior)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();
  if( !sior )  throw CORBA::BAD_PARAM(0, CORBA::COMPLETED_NO);

  omniObjRef* objref;

  if( !omni::stringToObject(objref, sior) )
    throw CORBA::INV_OBJREF(0, CORBA::COMPLETED_NO);

  return objref ?
    (CORBA::Object_ptr) objref->_ptrToObjRef(CORBA::Object::_PD_repoId)
    : CORBA::Object::_nil();
}


CORBA::ORB::ObjectIdList*
omniOrbORB::list_initial_services()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  CORBA::ORB::ObjectIdList* ids = omniInitialReferences::list();
  CORBA::ORB::ObjectIdList& idl = *ids;

  CORBA::ULong len = idl.length();
  idl.length(len + 2);
  idl[len++] = CORBA::string_dup("RootPOA");
  idl[len++] = CORBA::string_dup("POACurrent");
  
  return ids;
}


CORBA::Object_ptr
omniOrbORB::resolve_initial_references(const char* id)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  if( !id )  throw CORBA::ORB::InvalidName();

  if( !strcmp(id, "POACurrent") ) {
    throw CORBA::NO_IMPLEMENT(0, CORBA::COMPLETED_NO);
  }
  else if( !strcmp(id, "RootPOA") ) {
    // Instantiate the root POA on demand.
    // NB. No race condition problem here - this fn is thread safe.
    return omniOrbPOA::rootPOA();

    // We cannot insert the reference into the initial references
    // map, since holding a reference there would prevent the poa
    // from being released properly when it has been destroyed.
  }
  else {
    CORBA::Object_ptr obj = omniInitialReferences::get(id);

    if( !CORBA::is_nil(obj) )  return obj;

    if( !strcmp(id, "InterfaceRepository") ||
	!strcmp(id, "NameService") ||
	!strcmp(id, "TradingService") ||
	!strcmp(id, "SecurityCurrent") ||
	!strcmp(id, "TransactionCurrent") )
      // Resource not found.
      throw CORBA::NO_RESOURCES(0,CORBA::COMPLETED_NO);

    // The identifier is not defined.
    throw CORBA::ORB::InvalidName();
  }

  // Never get here...
  return 0;
}


CORBA::Boolean
omniOrbORB::work_pending()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  return 0;
}


void
omniOrbORB::perform_work()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  // No-op.
}


void
omniOrbORB::run()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  // It is possible for there to be multiple threads stuck in
  // here -- so we need to be sure that shutdown wakes 'em all up!

  omni_tracedmutex_lock sync(orb_lock);

  orb_n_blocked_in_run++;
  while( !pd_shutdown )  orb_signal.wait();
  orb_n_blocked_in_run--;
}


void
omniOrbORB::shutdown(CORBA::Boolean wait_for_completion)
{
  omni_tracedmutex_lock sync(orb_lock);

  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  if( wait_for_completion && 0 /*?? in context of invocation */ )
    throw CORBA::BAD_INV_ORDER(CORBA::OMGVMCID | 3, CORBA::COMPLETED_NO);

  do_shutdown(wait_for_completion);
}


void
omniOrbORB::destroy()
{
  omniOrbORB* orb;
  {
    omni_tracedmutex_lock sync(orb_lock);

    if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();
    if( pd_destroyed )  throw CORBA::OBJECT_NOT_EXIST(0, CORBA::COMPLETED_NO);

    if( 0 /*?? in context of invocation */ )
      throw CORBA::BAD_INV_ORDER(CORBA::OMGVMCID | 3, CORBA::COMPLETED_NO);

    if( !pd_shutdown )  do_shutdown(1);

    pd_destroyed = 1;
    orb = the_orb;
    the_orb = 0;
  }
  CORBA::release(orb);
}


CORBA::Boolean
omniOrbORB::_is_a(const char* repoId)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  return (repoId && _ptrToObjRef(repoId)) ? 1 : 0;
}


CORBA::Boolean
omniOrbORB::_non_existent()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  {
    omni_tracedmutex_lock sync(orb_lock);
    return pd_destroyed ? 1 : 0;
  }
}


CORBA::Boolean
omniOrbORB::_is_equivalent(CORBA::Object_ptr other_object)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  return other_object == (CORBA::Object_ptr) this;
}


CORBA::ULong
omniOrbORB::_hash(CORBA::ULong max)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  return CORBA::ULong(this) % max;
}


void*
omniOrbORB::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( !strcmp(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;
  if( !strcmp(repoId, CORBA::ORB::_PD_repoId) )
    return (CORBA::ORB_ptr) this;

  return 0;
}


void
omniOrbORB::_NP_incrRefCount()
{
  orb_lock.lock();
  pd_refCount++;
  orb_lock.unlock();
}


void
omniOrbORB::_NP_decrRefCount()
{
  orb_lock.lock();
  int done = --pd_refCount > 0;
  orb_lock.unlock();
  if( done )  return;

  omniORB::logs(15, "No more references to the ORB -- deleted.");

  delete this;
}


void
omniOrbORB::actual_shutdown()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(orb_lock, 1);
  OMNIORB_ASSERT(pd_shutdown_in_progress);

  // Deinitialise the dynamic library.
  omniDynamicLib::ops->deinit();

  // Shutdown object adapters.  When this returns all
  // outstanding requests have completed.
  omniOrbPOA::shutdown();

  // Shutdown incoming connections.
  omniObjAdapter::shutdown();

  // Shutdown outgoing scavenger.
  StrandScavenger::killOutScavenger();

  proxyObjectFactory::shutdown();

  omniORB::logs(10, "ORB shutdown is complete.");

  pd_shutdown = 1;

  // Wake up everyone stuck in run().
  orb_signal.broadcast();
}


static void
shutdown_thread_fn(void* arg)
{
  OMNIORB_ASSERT(arg);

  omniORB::logs(15, "ORB shutdown thread started.");

  omni_tracedmutex_lock sync(orb_lock);
  ((omniOrbORB*) arg)->actual_shutdown();
}


void
omniOrbORB::do_shutdown(CORBA::Boolean wait_for_completion)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(orb_lock, 1);

  if( pd_shutdown )  return;

  if( pd_shutdown_in_progress ) {
    if( wait_for_completion ) {
      omniORB::logs(15, "ORB shutdown already in progress -- waiting.");
      orb_n_blocked_in_run++;
      while( !pd_shutdown )  orb_signal.wait();
      orb_n_blocked_in_run--;
    }
    return;
  }

  omniORB::logs(10, "Preparing to shutdown ORB.");

  pd_shutdown_in_progress = 1;

  if( wait_for_completion ) {
    actual_shutdown();
  }
  else {
    // If wait_for_completion is zero we need to pass this to another
    // thread. This is needed to support shutting down the orb from
    // a method invocation -- otherwise we would deadlock waiting for
    // the method invocation to complete.

    omniORB::logs(15, "Starting an ORB shutdown thread.");

    (new omni_thread(shutdown_thread_fn, (omniOrbORB*) this))->start();
  }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

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
parse_ORB_args(int &argc,char **argv,const char *orb_identifier)
{
  CORBA::Boolean orbId_match = 0;

  if( orb_identifier && strcmp(orb_identifier, MY_ORB_ID)
                     && strcmp(orb_identifier, OLD_ORB_ID) ) {
    if( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "CORBA::ORB_init failed -- the ORBid (" << orb_identifier << ")"
	" is not " << MY_ORB_ID << "\n";
    }
    return 0;
  }
  if( omniORB::trace(1) && !strcmp(orb_identifier, OLD_ORB_ID) )
    omniORB::logs(1, "WARNING -- using ORBid omniORB2 with omniORB3.");

  if (argc > 0) {
    // Using argv[0] as the serverName implicitly assumes that the
    // argv array follows the unix tradition of passing the command name
    // as the first argument. This may not be the case on other platforms
    // and the application may choose to create its own argv array instead
    // of passing the command line arguments to ORB_init in verbatim form.
    //
    // XXX Should we trim this to a leafname?
#ifdef HAS_Cplusplus_Namespace
    if (omniORB::serverName) omni::freeString(omniORB::serverName);
#endif
    omniORB::serverName = CORBA::string_dup(argv[0]);
  }

  int idx = 1;
  while (argc > idx)
    {
      // -ORBxxxxxxx ?
      if (strlen(argv[idx]) < 4 ||
	  !(argv[idx][0] == '-' && argv[idx][1] == 'O' &&
	    argv[idx][2] == 'R' && argv[idx][3] == 'B'))
	{
	  idx++;
	  continue;
	}


      // -ORBid <id>
      if (strcmp(argv[idx],"-ORBid") == 0) {
	if ((idx+1) >= argc) {
	  omniORB::logs(1,"CORBA::ORB_init failed: missing -ORBid parameter.");
	  return 0;
	}
	if( strcmp(argv[idx+1], MY_ORB_ID) && strcmp(argv[idx+1], OLD_ORB_ID) )
	  {
	    if( omniORB::trace(1) ) {
	      omniORB::logger l;
	      l << "CORBA::ORB_init failed -- the ORBid (" <<
		argv[idx+1] << ") is not " << MY_ORB_ID << "\n";
	    }
	    return 0;
	  }
	orbId_match = 1;
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBtraceLevel
      if (strcmp(argv[idx],"-ORBtraceLevel") == 0) {
	if((idx+1) >= argc) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing -ORBtraceLevel"
			" parameter.");
	  return 0;
	}
	if( sscanf(argv[idx+1],"%d", &omniORB::traceLevel) != 1
	    || omniORB::traceLevel < 0 ) {
	  omniORB::logf("CORBA::ORB_init failed: invalid -ORBtraceLevel"
			" parameter.");
	  return 0;
	}
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBstrictIIOP
      if (strcmp(argv[idx],"-ORBstrictIIOP") == 0) {
	if((idx+1) >= argc) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing -ORBstrictIIOP"
			" parameter (0 or 1).");
	  return 0;
	}
	unsigned int v;
	if (sscanf(argv[idx+1],"%u",&v) != 1) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid -ORBstrictIIOP"
			" parameter.");
	  return 0;
	}
	omniORB::strictIIOP = ((v)?1:0);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBtcAliasExpand
      if (strcmp(argv[idx],"-ORBtcAliasExpand") == 0) {
	if((idx+1) >= argc) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing"
			" -ORBtcAliasExpand parameter (0 or 1).");
	  return 0;
	}
	unsigned int v;
	if (sscanf(argv[idx+1],"%u",&v) != 1) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid "
			"-ORBtcAliasExpand parameter.");
	  return 0;
	}
	omniORB::tcAliasExpand = ((v)?1:0);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBgiopMaxMsgSize
      if (strcmp(argv[idx],"-ORBgiopMaxMsgSize") == 0) {
	if((idx+1) >= argc) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing "
			"-ORBgiopMaxMsgSize parameter.");
	  return 0;
	}
	CORBA::ULong sz;
#if SIZEOF_LONG == 4
	if (sscanf(argv[idx+1],"%lu",&sz) != 1) {
#else
	if (sscanf(argv[idx+1],"%u",&sz) != 1) {
#endif
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid"
			" -ORBgiopMaxMsgSize parameter.");
	  return 0;
	}
	omniORB::MaxMessageSize(sz);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBserverName
      if (strcmp(argv[idx],"-ORBserverName") == 0) {
	if((idx+1) >= argc) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing -ORBserverName"
			" parameter.");
	  return 0;
	}
#ifdef HAS_Cplusplus_Namespace
	if (omniORB::serverName) omni::freeString(omniORB::serverName);
#endif
	omniORB::serverName = CORBA::string_dup(argv[idx+1]);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBInitialHost
      if (strcmp(argv[idx],"-ORBInitialHost") == 0) {
	if((idx+1) >= argc) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing -ORBInitialHost"
			" parameter.");
	  return 0;
	}
	bootstrapAgentHostname = CORBA::string_dup(argv[idx+1]);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBInitialPort
      if (strcmp(argv[idx],"-ORBInitialPort") == 0) {
	if((idx+1) >= argc) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing -ORBInitialPort"
			" parameter.");
	  return 0;
	}
        unsigned int port;
	if (sscanf(argv[idx+1],"%u",&port) != 1 ||
            (port == 0 || port >= 65536)) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid -ORBInitialPort"
			" parameter.");
	  return 0;
	}
	bootstrapAgentPort = (CORBA::UShort)port;
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBdiiThrowsSysExceptions
      if( strcmp(argv[idx],"-ORBdiiThrowsSysExceptions") == 0 ) {
	if( idx + 1 >= argc ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing"
			" -ORBdiiThrowsSysExceptions parameter (0 or 1).");
	  return 0;
	}
	unsigned int v;
	if( sscanf(argv[idx+1],"%u",&v) != 1 ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid"
			" -ORBdiiThrowsSysExceptions parameter.");
	  return 0;
	}
	omniORB::diiThrowsSysExceptions = v ? 1 : 0;
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBabortOnInternalError
      if( strcmp(argv[idx],"-ORBabortOnInternalError") == 0 ) {
	if( idx + 1 >= argc ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing"
			" -ORBabortOnInternalError parameter (0 or 1).");
	  return 0;
	}
	unsigned int v;
	if( sscanf(argv[idx+1],"%u",&v) != 1 ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid"
			" -ORBabortOnInternalError parameter.");
	  return 0;
	}
	omniORB::abortOnInternalError = v ? 1 : 0;
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBverifyObjectExistsAndType
      if( strcmp(argv[idx],"-ORBverifyObjectExistsAndType") == 0 ) {
	if( idx + 1 >= argc ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing"
			" -ORBverifyObjectExistsAndType parameter (0 or 1).");
	  return 0;
	}
	unsigned int v;
	if( sscanf(argv[idx+1],"%u",&v) != 1 ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid"
			" -ORBverifyObjectExistsAndType parameter.");
	  return 0;
	}
	omniORB::verifyObjectExistsAndType = v ? 1 : 0;
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBinConScanPeriod
      if( strcmp(argv[idx],"-ORBinConScanPeriod") == 0 ) {
	if( idx + 1 >= argc ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing"
			" -ORBinConScanPeriod parameter.");
	  return 0;
	}
	unsigned int v;
	if( sscanf(argv[idx+1], "%u", &v) != 1 ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid"
			" -ORBinConScanPeriod parameter.");
	  return 0;
	}
	omniORB::idleConnectionScanPeriod(omniORB::idleIncoming, v);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBoutConScanPeriod
      if( strcmp(argv[idx],"-ORBoutConScanPeriod") == 0 ) {
	if( idx + 1 >= argc ) {
	  omniORB::logs(1, "CORBA::ORB_init failed -- missing"
			" -ORBoutConScanPeriod parameter.");
	  return 0;
	}
	unsigned int v;
	if( sscanf(argv[idx+1], "%u", &v) != 1 ) {
	  omniORB::logs(1, "CORBA::ORB_init failed -- invalid"
			" -ORBoutConScanPeriod parameter.");
	  return 0;
	}
	omniORB::idleConnectionScanPeriod(omniORB::idleOutgoing, v);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBclientCallTimeOutPeriod
      if( strcmp(argv[idx],"-ORBclientCallTimeOutPeriod") == 0 ) {
	if( idx + 1 >= argc ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing"
			" -ORBclientCallTimeOutPeriod parameter.");
	  return 0;
	}
	unsigned int v;
	if( sscanf(argv[idx+1], "%u", &v) != 1 ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid"
			" -ORBinConScanPeriod parameter.");
	  return 0;
	}
	omniORB::callTimeOutPeriod(omniORB::clientSide, v);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBserverCallTimeOutPeriod
      if( strcmp(argv[idx],"-ORBserverCallTimeOutPeriod") == 0 ) {
	if( idx + 1 >= argc ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing"
			" -ORBserverCallTimeOutPeriod parameter.");
	  return 0;
	}
	unsigned int v;
	if( sscanf(argv[idx+1], "%u", &v) != 1 ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid"
			" -ORBserverCallTimeOutPeriod parameter.");
	  return 0;
	}
	omniORB::callTimeOutPeriod(omniORB::serverSide, v);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBscanOutgoingPeriod
      if( strcmp(argv[idx],"-ORBscanOutgoingPeriod") == 0 ) {
	if( idx + 1 >= argc ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing"
			" -ORBscanOutgoingPeriod parameter.");
	  return 0;
	}
	unsigned int v;
	if( sscanf(argv[idx+1], "%u", &v) != 1 ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid"
			" -ORBscanOutgoingPeriod parameter.");
	  return 0;
	}
	omniORB::scanGranularity(omniORB::scanOutgoing, v);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBscanIncomingPeriod
      if( strcmp(argv[idx],"-ORBscanIncomingPeriod") == 0 ) {
	if( idx + 1 >= argc ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing"
			" -ORBscanIncomingPeriod parameter.");
	  return 0;
	}
	unsigned int v;
	if( sscanf(argv[idx+1], "%u", &v) != 1 ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid"
			" -ORBscanIncomingPeriod parameter.");
	  return 0;
	}
	omniORB::scanGranularity(omniORB::scanIncoming, v);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBlcdMode
      if( strcmp(argv[idx],"-ORBlcdMode") == 0 ) {
	omniORB::enableLcdMode();
	move_args(argc,argv,idx,1);
	continue;
      }

      // -ORBhelp
      if( strcmp(argv[idx],"-ORBhelp") == 0 ) {
	omniORB::logger l;
	l <<
	  "Valid -ORB<options> are:\n"
	  "    -ORBid omniORB2\n"
	  "    -ORBtraceLevel <n>\n"
	  "    -ORBstrictIIOP <0|1>\n"
	  "    -ORBtcAliasExpand <0|1>\n"
	  "    -ORBgiopMaxMsgSize <n bytes>\n"
	  "    -ORBserverName <name>\n"
	  "    -ORBInitialHost <name>\n"
	  "    -ORBInitialPort <1-65535>\n"
	  "    -ORBdiiThrowsSysExceptions <0|1>\n"
	  "    -ORBabortOnInternalError <0|1>\n"
	  "    -ORBverifyObjectExistsAndType <0|1>\n"
	  "    -ORBinConScanPeriod <n seconds>\n"
	  "    -ORBoutConScanPeriod <n seconds>\n"
	  "    -ORBclientCallTimeOutPeriod <n seconds>\n"
	  "    -ORBserverCallTimeOutPeriod <n seconds>\n"
	  "    -ORBscanOutgoingPeriod <n seconds>\n"
	  "    -ORBscanIncomingPeriod <n seconds>\n"
	  "    -ORBlcdMode <0|1>\n";
	move_args(argc,argv,idx,1);
	continue;
      }


      // -ORBlcdMode
      if( strcmp(argv[idx],"-ORBlcdMode") == 0 ) {
	omniORB::enableLcdMode();
	move_args(argc,argv,idx,1);
	continue;
      }


      // -ORBpoa_iiop_port  -- I think this wants a better name.
      if( strcmp(argv[idx],"-ORBpoa_iiop_port") == 0 ) {
	if( (idx+1) >= argc ) {
	  omniORB::logs(1, "CORBA::ORB_init failed -- missing "
			"-ORBpoa_iiop_port parameter.");
	  return 0;
	}
        unsigned port;
	if( sscanf(argv[idx+1], "%u", &port) != 1 ||
            (port == 0 || port >= 65536) ) {
	  omniORB::logs(1, "CORBA::ORB_init failed -- invalid "
			"-ORBpoa_iiop_port parameter.");
	  return 0;
	}

	omniObjAdapter::options.
	  incomingPorts.push_back(omniObjAdapter::ListenPort("", port));

	move_args(argc, argv, idx, 2);
	continue;
      }


      // Reach here only if the argument in this form: -ORBxxxxx
      // is not recognised.
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "CORBA::ORB_init failed: unknown ORB argument ("
	  << argv[idx] << ")\n";
      }
      return 0;
    }

  if (!orb_identifier && !orbId_match) {
    omniORB::logs(1, "CORBA::ORB_init failed: ORBid is not specified.");
    return 0;
  }
  return 1;
}

