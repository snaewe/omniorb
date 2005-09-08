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
  Revision 1.36.2.4  2005/09/08 14:49:40  dgrisby
  Merge -ORBconfigFile argument.

  Revision 1.36.2.3  2005/03/30 23:36:10  dgrisby
  Another merge from omni4_0_develop.

  Revision 1.36.2.2  2005/01/06 23:10:12  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.36.2.1  2003/03/23 21:02:22  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.33.2.41  2002/09/09 00:24:40  dgrisby
  Make sure various modules initialise. Needed by MacOSX.

  Revision 1.33.2.40  2002/09/08 21:58:54  dgrisby
  Support for MSVC 7. (Untested.)

  Revision 1.33.2.39  2002/03/18 15:13:07  dpg1
  Fix bug with old-style ORBInitRef in config file; look for
  -ORBtraceLevel arg before anything else; update Windows registry
  key. Correct error message.

  Revision 1.33.2.38  2002/03/13 16:05:38  dpg1
  Transport shutdown fixes. Reference count SocketCollections to avoid
  connections using them after they are deleted. Properly close
  connections when in thread pool mode.

  Revision 1.33.2.37  2002/02/21 15:57:46  dpg1
  Remove dead code.

  Revision 1.33.2.36  2002/02/11 15:15:50  dpg1
  Things for ETS kernel.

  Revision 1.33.2.35  2002/01/16 11:31:58  dpg1
  Race condition in use of registerNilCorbaObject/registerTrackedObject.
  (Reported by Teemu Torma).

  Revision 1.33.2.34  2002/01/15 16:38:12  dpg1
  On the road to autoconf. Dependencies refactored, configure.ac
  written. No makefiles yet.

  Revision 1.33.2.33  2002/01/09 11:35:22  dpg1
  Remove separate omniAsyncInvoker library to save library overhead.

  Revision 1.33.2.32  2001/11/13 14:11:45  dpg1
  Tweaks for CORBA 2.5 compliance.

  Revision 1.33.2.31  2001/11/06 15:41:38  dpg1
  Reimplement Context. Remove CORBA::Status. Tidying up.

  Revision 1.33.2.30  2001/10/19 11:06:45  dpg1
  Principal support for GIOP 1.0. Correct some spelling mistakes.

  Revision 1.33.2.29  2001/09/20 14:18:12  dpg1
  Make hooked initialiser suitable for ORB restart.

  Revision 1.33.2.28  2001/09/20 13:26:13  dpg1
  Allow ORB_init() after orb->destroy().

  Revision 1.33.2.27  2001/09/19 17:26:48  dpg1
  Full clean-up after orb->destroy().

  Revision 1.33.2.26  2001/08/24 16:44:59  sll
  Switch to use Winsock 2. Replaced wsock32.lib with ws2_32.lib and mswsock.lib

  Revision 1.33.2.25  2001/08/23 16:01:43  sll
  Added initialisers for transportRules and giopEndpoint.

  Revision 1.33.2.24  2001/08/21 11:02:13  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.33.2.23  2001/08/20 08:19:22  sll
  Read the new ORB configuration file format. Can still read old format.
  Can also set configuration parameters from environment variables.

  Revision 1.33.2.22  2001/08/17 17:12:35  sll
  Modularise ORB configuration parameters.

  Revision 1.33.2.21  2001/08/08 15:57:11  sll
  New options unixTransportDirectory & unixTransportPermission.

  Revision 1.33.2.20  2001/08/03 17:41:19  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.33.2.19  2001/08/01 10:08:21  dpg1
  Main thread policy.

  Revision 1.33.2.18  2001/07/31 16:28:00  sll
  Added GIOP BiDir support.

  Revision 1.33.2.17  2001/07/13 15:23:09  sll
  New ORB initialisation options.

  Revision 1.33.2.16  2001/06/18 16:28:49  dpg1
  Print out distribution date at ORB_init()

  Revision 1.33.2.15  2001/06/18 16:18:49  dpg1
  Print out distribution date at ORB_init()

  Revision 1.33.2.14  2001/06/11 17:57:13  sll
   External libraries can now hook-up to the orb initialiser list irrespective
   of whether the global variables in the external libraries are initialised
   before the core library.

  Revision 1.33.2.13  2001/06/07 16:24:09  dpg1
  PortableServer::Current support.

  Revision 1.33.2.12  2001/05/31 16:18:12  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.33.2.11  2001/05/09 20:03:14  sll
  ORB::shutdown() now works as expected.
  Moved calling detach() on all the initializers from shutdown() to destroy()

  Revision 1.33.2.10  2001/04/19 11:16:39  sll
  Missing -ORBhelp listing for -ORBmaxGIOPVersion

  Revision 1.33.2.9  2001/04/18 18:18:09  sll
  Big checkin with the brand new internal APIs.

  Revision 1.33.2.8  2000/11/20 11:59:44  dpg1
  API to configure code sets.

  Revision 1.33.2.7  2000/11/15 17:19:07  sll
  Added initialiser for cdrStream.

  Revision 1.33.2.6  2000/11/09 12:27:56  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.33.2.5  2000/10/27 15:42:07  dpg1
  Initial code set conversion support. Not yet enabled or fully tested.

  Revision 1.33.2.4  2000/10/04 16:54:41  sll
  Added omni_omniIOR_initialiser_.

  Revision 1.33.2.3  2000/10/03 17:37:42  sll
  New initialiser for omniIOR.

  Revision 1.33.2.2  2000/09/27 17:54:29  sll
  Updated to identify the ORB as omniORB4. Added initialiser calls to the new
  code.

  Revision 1.33.2.1  2000/07/17 10:35:52  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.34  2000/07/13 15:25:58  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.29.6.20  2000/07/10 10:56:43  sll
  Minor rearrangement to keep DEC C++ 5.6 happy.

  Revision 1.29.6.19  2000/06/22 10:40:14  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.29.6.18  2000/06/19 14:18:33  dpg1
  Explicit cast to (const char*) when using String_var with logger.

  Revision 1.29.6.17  2000/06/02 14:17:05  dpg1
  Add static install() method to initialiser so extra initialisers /
  deinitialisers can be added at run time (needed by omniORBpy).

  Revision 1.29.6.16  2000/05/24 17:19:41  dpg1
  Add documented but non-existent -ORBobjectTableSize and
  -ORBno_bootstrap_agent arguments

  Revision 1.29.6.15  2000/04/27 10:46:30  dpg1
  Interoperable Naming Service

  Add initialisers for URI and initRefs modules.
  object_to_string() and string_to_object() use omniURI:: functions.
  resolve_initial_references() replaced with omniInitialReferences::resolve().
  Add -ORBInitRef and -ORBDefaultInitRef command-line options.

  Revision 1.29.6.14  2000/02/04 18:11:01  djr
  Minor mods for IRIX (casting pointers to ulong instead of int).

  Revision 1.29.6.13  2000/01/27 10:55:45  djr
  Mods needed for powerpc_aix.  New macro OMNIORB_BASE_CTOR to provide
  fqname for base class constructor for some compilers.

  Revision 1.29.6.12  2000/01/22 16:46:04  djr
  Added -ORBtraceInvocations to option help summary.

  Revision 1.29.6.11  2000/01/20 11:51:34  djr
  (Most) Pseudo objects now used omni::poRcLock for ref counting.
  New assertion check OMNI_USER_CHECK.

  Revision 1.29.6.10  2000/01/07 14:51:13  djr
  Call timeouts are now disabled by default.

  Revision 1.29.6.9  2000/01/05 17:59:45  djr
  Added check for reinitialisation in ORB_init.

  Revision 1.29.6.8  1999/11/02 17:47:01  djr
  Removed obsolete references to rope factories.

  Revision 1.29.6.7  1999/10/29 13:18:17  djr
  Changes to ensure mutexes are constructed when accessed.

  Revision 1.29.6.6  1999/10/14 16:22:06  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.29.6.5  1999/10/04 17:08:32  djr
  Some more fixes/MSVC work-arounds.

  Revision 1.29.6.4  1999/09/24 17:11:11  djr
  New option -ORBtraceInvocations and omniORB::traceInvocations.

  Revision 1.29.6.3  1999/09/24 15:01:33  djr
  Added module initialisers, and sll's new scavenger implementation.

  Revision 1.29.6.2  1999/09/24 10:27:30  djr
  Improvements to ORB and BOA options.

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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <corbaOrb.h>
#include <initRefs.h>
#include <omniORB4/omniObjRef.h>
#include <poaimpl.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/objTracker.h>
#include <giopStreamImpl.h>
#include <invoker.h>
#include <omniCurrent.h>
#include <omniORB4/distdate.hh>
#include <orbOptions.h>
#include <orbParameters.h>
#include <omniIdentity.h>
#include <SocketCollection.h>

#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#  include <errno.h>
#endif
#include <stdio.h>
#include <stdlib.h>

OMNI_USING_NAMESPACE(omni)

#define ORB_ID_STRING "omniORB4"

static const char* orb_ids[] = { ORB_ID_STRING,
				 "omniORB3",
				 "omniORB2",
				 0 };

static omniOrbORB*          the_orb                   = 0;
static int                  orb_count                 = 0;
static omni_tracedmutex     orb_lock;
static omni_tracedcondition orb_signal(&orb_lock);
static volatile int         orb_n_blocked_in_run      = 0;

// For the main thread AsyncInvoker:
static volatile int         invoker_threads           = 0;
static omni_tracedcondition invoker_signal(&orb_lock);
static omniTaskLink         invoker_dedicated_tq;
static CORBA::Boolean       invoker_shutting_down     = 0;


#ifdef __SINIX__
// Why haven't we got this signature from signal.h? - sll
//
extern "C" int sigaction(int, const struct sigaction *, struct sigaction *);
#endif

// Config file
#if defined(NTArchitecture) && !defined(__ETS_KERNEL__)
static const char* config_fname = 0;
#else
static const char* config_fname = CONFIG_DEFAULT_LOCATION;
#endif



///////////////////////////////////////////////////////////////////////
//          Per module initialisers.
//
OMNI_NAMESPACE_BEGIN(omni)

ORBAsyncInvoker* orbAsyncInvoker = 0;

extern omniInitialiser& omni_omniIOR_initialiser_;
extern omniInitialiser& omni_corbaOrb_initialiser_;
extern omniInitialiser& omni_omniInternal_initialiser_;
extern omniInitialiser& omni_initRefs_initialiser_;
extern omniInitialiser& omni_hooked_initialiser_;
extern omniInitialiser& omni_interceptor_initialiser_;
extern omniInitialiser& omni_ior_initialiser_;
extern omniInitialiser& omni_codeSet_initialiser_;
extern omniInitialiser& omni_cdrStream_initialiser_;
extern omniInitialiser& omni_giopStrand_initialiser_;
extern omniInitialiser& omni_giopStreamImpl_initialiser_;
extern omniInitialiser& omni_giopRope_initialiser_;
extern omniInitialiser& omni_giopserver_initialiser_;
extern omniInitialiser& omni_giopbidir_initialiser_;
extern omniInitialiser& omni_omniTransport_initialiser_;
extern omniInitialiser& omni_omniCurrent_initialiser_;
extern omniInitialiser& omni_dynamiclib_initialiser_;
extern omniInitialiser& omni_objadpt_initialiser_;
extern omniInitialiser& omni_giopEndpoint_initialiser_;
extern omniInitialiser& omni_transportRules_initialiser_;
extern omniInitialiser& omni_ObjRef_initialiser_;
extern omniInitialiser& omni_orbOptions_initialiser_;
extern omniInitialiser& omni_poa_initialiser_;
extern omniInitialiser& omni_uri_initialiser_;

OMNI_NAMESPACE_END(omni)

static void
shutdownAsyncInvoker();


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


CORBA::ORB_ptr
CORBA::ORB::_nil()
{
  static omniOrbORB* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniOrbORB(1 /* is nil */);
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


const char*
CORBA::ORB::_PD_repoId = "IDL:omg.org/CORBA/ORB:1.0";

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x500

#include <exception.h>
static void omni_abort()
{
  abort();
}

#endif
#endif

static void enableLcdMode() {
  orbParameters::strictIIOP = 0;
  orbParameters::tcAliasExpand = 1;
  orbParameters::scanGranularity = 0;
  orbParameters::outConScanPeriod = 0;
  orbParameters::inConScanPeriod = 0;
  orbParameters::useTypeCodeIndirections = 0;
  orbParameters::verifyObjectExistsAndType = 0;
  orbParameters::acceptMisalignedTcIndirections = 1;
}

CORBA::ORB_ptr
CORBA::ORB_init(int& argc, char** argv, const char* orb_identifier,
		const char* options[][2])
{
  omni_tracedmutex_lock sync(orb_lock);

  if( the_orb ) {
    the_orb->_NP_incrRefCount();
    return the_orb;
  }

  const char* option_src_1  = "configuration file";
  const char* option_src_2  = "environment variable";
  const char* option_src_3  = "argument";
  const char* option_src_4  = "option list";
  const char* option_src_5  = "-ORB arguments";
  const char* option_source = 0;
  try {

    orbOptions::singleton().reset();

    // Look for -ORBtraceLevel arg first
    option_source = option_src_5;
    orbOptions::singleton().getTraceLevel(argc,argv);

    {
      const char* f = getenv(CONFIG_ENV);
      if (f) config_fname = f;
    }
    // Configuration file name can be overriden by command line.
    config_fname = orbOptions::singleton().getConfigFileName(argc, argv,
							     config_fname);

    // Parse configuration file
    option_source = option_src_1;

    if (config_fname) {
      orbOptions::singleton().importFromFile(config_fname);
    }
#if defined(NTArchitecture) && !defined(__ETS_KERNEL__)
    else {
      // Parse configuration from registry on NT if no configuration
      // file is specified.
      if (!orbOptions::singleton().importFromRegistry()) {
	// Failed to read from the registry. Try the default file location.
	config_fname = CONFIG_DEFAULT_LOCATION;
	orbOptions::singleton().importFromFile(config_fname);
      }
    }
#endif

    // Parse configuration from environment variables
    option_source = option_src_2;
    orbOptions::singleton().importFromEnv();


    if ( orb_identifier && strlen(orb_identifier) ) {
      option_source = option_src_3;
      orbOptions::singleton().addOption("id",orb_identifier);
    }

    // Parse configuration from argument <options>
    if (options) {
      option_source = option_src_4;
      orbOptions::singleton().addOptions(options);
    }

    // Parse configurations from argv
    option_source = option_src_5;
    orbOptions::singleton().extractInitOptions(argc,argv);

  }
  catch (const orbOptions::Unknown& ex) {
    if ( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "ORB_init failed: unknown option ("
	<< ex.key << ") in " << option_source << "\n";
    }
    OMNIORB_THROW(INITIALIZE,INITIALIZE_InvalidORBInitArgs,
		  CORBA::COMPLETED_NO);
  }
  catch (const orbOptions::BadParam& ex) {
    if ( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "ORB_init failed: Bad parameter (" << ex.value
	<< ") for option "
	<< ((option_source == option_src_5) ? "-ORB" : "")
	<< ex.key << " in " <<  option_source << ", reason: "
	<< ex.why << "\n";
    }
    OMNIORB_THROW(INITIALIZE,INITIALIZE_InvalidORBInitArgs,
		  CORBA::COMPLETED_NO);
  }

  try {
    orbOptions::singleton().visit();
  }
  catch (const orbOptions::BadParam& ex) {
    if ( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "ORB_init failed: Bad parameter (" << ex.value
	<< ") for ORB configuration option " << ex.key
	<< ", reason: " << ex.why << "\n";
    }
    OMNIORB_THROW(INITIALIZE,INITIALIZE_InvalidORBInitArgs,
		  CORBA::COMPLETED_NO);
  }


  if (omniORB::trace(2)) {
    omniORB::logger l;
    l << "Distribution date: " OMNIORB_DIST_DATE "\n";
  }

  try {
    // Call attach method of each initialiser object.
    // The order of these calls must take into account of the dependency
    // among the modules.
    omni_giopEndpoint_initialiser_.attach();
    omni_transportRules_initialiser_.attach();
    omni_omniInternal_initialiser_.attach();
    omni_corbaOrb_initialiser_.attach();
    omni_objadpt_initialiser_.attach();
    omni_interceptor_initialiser_.attach();
    omni_giopStreamImpl_initialiser_.attach();
    omni_omniIOR_initialiser_.attach();
    omni_ior_initialiser_.attach();
    omni_codeSet_initialiser_.attach();
    omni_cdrStream_initialiser_.attach();
    omni_omniTransport_initialiser_.attach();
    omni_giopRope_initialiser_.attach();
    omni_giopserver_initialiser_.attach();
    omni_giopbidir_initialiser_.attach();
    omni_giopStrand_initialiser_.attach();
    omni_omniCurrent_initialiser_.attach();
    omni_dynamiclib_initialiser_.attach();
    omni_ObjRef_initialiser_.attach();
    omni_initRefs_initialiser_.attach();
    omni_orbOptions_initialiser_.attach();
    omni_poa_initialiser_.attach();
    omni_uri_initialiser_.attach();
    omni_hooked_initialiser_.attach();

    if (orbParameters::lcdMode) {
      enableLcdMode();
    }

    if (omniORB::trace(20) || orbParameters::dumpConfiguration) {
      orbOptions::sequenceString_var currentSet;
      currentSet = orbOptions::singleton().dumpCurrentSet();
      omniORB::logger l;
      l << "Current configuration is as follows:\n";
      for (CORBA::ULong i = 0; i < currentSet->length(); i++)
	l << "omniORB:   " << (const char*)currentSet[i] << "\n";
    }
  }
  catch (CORBA::INITIALIZE &ex) {
    throw;
  }
  catch (...) {
    OMNIORB_THROW(INITIALIZE,INITIALIZE_FailedORBInit,CORBA::COMPLETED_NO);
  }

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x500
  // Sun C++ 5.0 or Forte C++ 6.0 generated code will segv occasionally
  // when concurrent threads throw an exception. The stack trace points
  // to a problem in the exception unwinding. The workaround seems to be
  // to install explicitly an uncaught exception handler, which is what
  // we do here.
  set_terminate(omni_abort);
#endif
#endif

  the_orb = new omniOrbORB(0);
  the_orb->_NP_incrRefCount();
  orb_count++;
  return the_orb;
}

//////////////////////////////////////////////////////////////////////
///////////////////////////// omniOrbORB /////////////////////////////
//////////////////////////////////////////////////////////////////////

#define CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED()  \
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();  \
  if( pd_destroyed )  OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_NoMatch, CORBA::COMPLETED_NO);  \
  if( pd_shutdown  )  OMNIORB_THROW(BAD_INV_ORDER, \
                                    BAD_INV_ORDER_ORBHasShutdown, \
                                    CORBA::COMPLETED_NO);  \

CORBA::Boolean
omniOrbORB::all_destroyed()
{
  return orb_count == 0;
}


omniOrbORB::~omniOrbORB()  {}


omniOrbORB::omniOrbORB(int nil)
  : OMNIORB_BASE_CTOR(CORBA::)ORB(nil),
    pd_refCount(1),
    pd_destroyed(0),
    pd_shutdown(0),
    pd_shutdown_in_progress(0)
{
}

char*
omniOrbORB::id()
{
  return CORBA::string_dup("");
}


char*
omniOrbORB::object_to_string(CORBA::Object_ptr obj)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();
  return omniURI::objectToString(obj);
}


CORBA::Object_ptr
omniOrbORB::string_to_object(const char* uri)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();
  return omniURI::stringToObject(uri);
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
  return omniInitialReferences::resolve(id);
}

CORBA::Boolean
omniOrbORB::work_pending()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  omni_thread* self = omni_thread::self();

  if (self && self->id() == omni::mainThreadId)
    return orbAsyncInvoker->work_pending();

  return 0;
}


void
omniOrbORB::perform_work()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  omni_thread* self = omni_thread::self();

  if (self && self->id() == omni::mainThreadId) {
    unsigned long s, ns;
    omni_thread::get_time(&s, &ns);
    orbAsyncInvoker->perform(s, ns);
  }
}


void
omniOrbORB::run()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  omni_thread* self = omni_thread::self();

  if (self && self->id() == omni::mainThreadId) {
    orbAsyncInvoker->perform();
  }
  else {
    orb_lock.lock();

    orb_n_blocked_in_run++;
    while( !pd_shutdown )  orb_signal.wait();
    orb_n_blocked_in_run--;

    orb_lock.unlock();
  }
}

CORBA::Boolean
omniOrbORB::run_timeout(unsigned long secs, unsigned long nanosecs)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  omni_thread* self = omni_thread::self();

  if (self && self->id() == omni::mainThreadId) {
    orbAsyncInvoker->perform(secs, nanosecs);
  }
  else {
    orb_lock.lock();

    orb_n_blocked_in_run++;
    if( !pd_shutdown )  orb_signal.timedwait(secs, nanosecs);
    orb_n_blocked_in_run--;

    orb_lock.unlock();
  }
  return pd_shutdown;
}



void
omniOrbORB::shutdown(CORBA::Boolean wait_for_completion)
{
  omni_tracedmutex_lock sync(orb_lock);

  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  if( wait_for_completion ) {
    // Complain if in the context of an operation invocation
    omniCurrent* current = omniCurrent::get();
    if (current && current->callDescriptor()) {
      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_WouldDeadLock,
		    CORBA::COMPLETED_NO);
    }
  }
  do_shutdown(wait_for_completion);
}


void
omniOrbORB::destroy()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();

  omniOrbORB* orb;
  {
    omni_tracedmutex_lock sync(orb_lock);

    if( pd_destroyed )  OMNIORB_THROW(BAD_INV_ORDER,
				      BAD_INV_ORDER_ORBHasShutdown,
				      CORBA::COMPLETED_NO);

    // Complain if in the context of an operation invocation
    omniCurrent* current = omniCurrent::get();
    if (current && current->callDescriptor()) {
      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_WouldDeadLock,
		    CORBA::COMPLETED_NO);
    }

    if( !pd_shutdown )  do_shutdown(1);

    if( pd_destroyed ) {
      omniORB::logs(15, "ORB destroyed by another thread.");
      return;
    }

    // Call detach method of the initialisers in reverse order.
    omni_hooked_initialiser_.detach();
    omni_uri_initialiser_.detach();
    omni_poa_initialiser_.detach();
    omni_orbOptions_initialiser_.detach();
    omni_initRefs_initialiser_.detach();
    omni_ObjRef_initialiser_.detach();
    omni_dynamiclib_initialiser_.detach();
    omni_omniCurrent_initialiser_.detach();
    omni_giopStrand_initialiser_.detach();
    omni_giopbidir_initialiser_.detach();
    omni_giopserver_initialiser_.detach();
    omni_giopRope_initialiser_.detach();
    omni_omniTransport_initialiser_.detach();
    omni_cdrStream_initialiser_.detach();
    omni_codeSet_initialiser_.detach();
    omni_ior_initialiser_.detach();
    omni_omniIOR_initialiser_.detach();
    omni_giopStreamImpl_initialiser_.detach();
    omni_interceptor_initialiser_.detach();
    omni_objadpt_initialiser_.detach();
    omni_corbaOrb_initialiser_.detach();
    omni_omniInternal_initialiser_.detach();
    omni_transportRules_initialiser_.detach();
    omni_giopEndpoint_initialiser_.detach();

    pd_destroyed = 1;
    orb = the_orb;
    the_orb = 0;
    orb_count--;
  }
  CORBA::release(orb);
}


void
omniOrbORB::register_initial_reference(const char* id, CORBA::Object_ptr obj)
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  omniInitialReferences::setFromORB(id, obj);
}



CORBA::Boolean
omniOrbORB::_non_existent()
{
  CHECK_NOT_NIL_SHUTDOWN_OR_DESTROYED();

  orb_lock.lock();
  CORBA::Boolean ret = pd_destroyed ? 1 : 0;
  orb_lock.unlock();

  return ret;
}


void*
omniOrbORB::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( omni::ptrStrMatch(repoId, CORBA::ORB::_PD_repoId) )
    return (CORBA::ORB_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


void
omniOrbORB::_NP_incrRefCount()
{
  omni::poRcLock->lock();
  pd_refCount++;
  omni::poRcLock->unlock();
}


void
omniOrbORB::_NP_decrRefCount()
{
  omni::poRcLock->lock();
  int done = --pd_refCount > 0;
  omni::poRcLock->unlock();
  if( done )  return;

  OMNIORB_USER_CHECK(pd_destroyed);
  OMNIORB_USER_CHECK(pd_refCount == 0);
  // If either of these fails then the application has released the
  // ORB reference too many times.

  omniORB::logs(15, "No more references to the ORB -- deleted.");

  delete this;
}


void
omniOrbORB::actual_shutdown()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(orb_lock, 1);
  OMNIORB_ASSERT(pd_shutdown_in_progress);

  //?? Is is safe to unlock orb_lock here?
  orb_lock.unlock();

  // Shutdown object adapters.  When this returns all
  // outstanding requests have completed.
  omniOrbPOA::shutdown();

  // Shutdown incoming connections.
  omniObjAdapter::shutdown();

  // Disable object references
  omniObjRef::_shutdown();

  // Wait for all client requests to complete
  omniIdentity::waitForLastIdentity();

  omniORB::logs(10, "ORB shutdown is complete.");

  orb_lock.lock();
  pd_shutdown = 1;

  // Wake up threads stuck in run().
  orb_signal.broadcast();

  // Wake up main thread if there is one running
  shutdownAsyncInvoker();
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
      omniORB::logs(15, "ORB shutdown complete -- finished waiting.");
    }
    else {
      omniORB::logs(15, "ORB shutdown already in progress -- nothing to do.");
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
////////////////////////  ORBAsyncInvoker  ///////////////////////////
//////////////////////////////////////////////////////////////////////

OMNI_NAMESPACE_BEGIN(omni)

ORBAsyncInvoker::~ORBAsyncInvoker()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(orb_lock, 1);
  OMNIORB_ASSERT(omniTaskLink::is_empty(invoker_dedicated_tq));
  OMNIORB_ASSERT(invoker_threads == 0);
}

int
ORBAsyncInvoker::work_pending()
{
  omni_tracedmutex_lock sync(orb_lock);

  return !omniTaskLink::is_empty(invoker_dedicated_tq);
}

void
ORBAsyncInvoker::perform(unsigned long secs, unsigned long nanosecs)
{
  orb_lock.lock();
  invoker_threads++;

  while (!invoker_shutting_down) {

    while (!invoker_shutting_down &&
	   omniTaskLink::is_empty(invoker_dedicated_tq)) {

      // Wait for a task to arrive
      if (secs || nanosecs) {
	if (invoker_signal.timedwait(secs, nanosecs) == 0) {
	  // timeout
	  invoker_threads--;
	  if (invoker_shutting_down) invoker_signal.signal();
	  orb_lock.unlock();
	  return;
	}
      }
      else {
	invoker_signal.wait();
      }
    }
    while (!omniTaskLink::is_empty(invoker_dedicated_tq)) {
      // Run all queued tasks
      omniTask* t = (omniTask*)invoker_dedicated_tq.next;
      t->deq();
      orb_lock.unlock();
      try {
	t->execute();
      }
      catch (...) {
	omniORB::logs(1, "ORBAsyncInvoker: Warning - unexpected "
		      "exception caught while executing a task.");
      }
      orb_lock.lock();
    }
  }
  OMNIORB_ASSERT(omniTaskLink::is_empty(invoker_dedicated_tq));

  invoker_threads--;
  if (invoker_shutting_down) invoker_signal.signal();
  orb_lock.unlock();
}

int
ORBAsyncInvoker::insert_dedicated(omniTask* t)
{
  OMNIORB_ASSERT(t->category() == omniTask::DedicatedThread);

  omni_tracedmutex_lock sync(orb_lock);
  t->enq(invoker_dedicated_tq);
  invoker_signal.signal();
  return 1;
}

int
ORBAsyncInvoker::cancel_dedicated(omniTask* t)
{
  OMNIORB_ASSERT(t->category() == omniTask::DedicatedThread);

  omni_tracedmutex_lock sync(orb_lock);

  omniTaskLink* l;

  for (l = invoker_dedicated_tq.next; l != &invoker_dedicated_tq; l= l->next) {
    if ((omniTask*)l == t) {
      l->deq();
      return 1;
    }
  }
  return 0;
}


OMNI_NAMESPACE_END(omni)


static void
shutdownAsyncInvoker()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(orb_lock, 1);
  invoker_shutting_down = 1;
  invoker_signal.broadcast();
}



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static
CORBA::Boolean
isValidId(const char* id) {
  const char** p = orb_ids;
  while (*p) {
    if (strcmp(*p,id) == 0) return 1;
    p++;
  }
  return 0;
}

static
const char*
myOrbId() {
  return orb_ids[0];
}

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////////
//            Hooked initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

static omnivector<omniInitialiser*>*& the_hooked_list()
{
  static omnivector<omniInitialiser*>* the_list = 0;
  if (!the_list) the_list = new omnivector<omniInitialiser*>;
  return the_list;
}

class omni_hooked_initialiser : public omniInitialiser {
public:
  void attach() {
    omnivector<omniInitialiser*>::iterator i    = the_hooked_list()->begin();
    omnivector<omniInitialiser*>::iterator last = the_hooked_list()->end();

    for (; i != last; i++) {
      (*i)->attach();
    }
  }

  void detach() {
    omnivector<omniInitialiser*>::iterator i    = the_hooked_list()->begin();
    omnivector<omniInitialiser*>::iterator last = the_hooked_list()->end();

    for (; i != last; i++) {
      (*i)->detach();
    }
  }

  virtual ~omni_hooked_initialiser() {
    omnivector<omniInitialiser*>*& the_list = the_hooked_list();
    delete the_list;
    the_list = 0;
  }
};

static omni_hooked_initialiser hinitialiser;
omniInitialiser& omni_hooked_initialiser_ = hinitialiser;

void
omniInitialiser::
install(omniInitialiser* init) {
  the_hooked_list()->push_back(init);
}


////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
CORBA::Boolean   orbParameters::dumpConfiguration = 0;
//  Set to 1 to cause the ORB to dump the current set of configuration
//  parameters.
//
//  Valid values = 0 or 1

CORBA::Boolean   orbParameters::lcdMode = 0;
//  Set to 1 to enable 'Lowest Common Denominator' Mode.
//  This will disable various features of IIOP and GIOP which are
//  poorly supported by some ORBs, and disable warnings/errors when
//  certain types of erroneous message are received on the wire.
//
//  Valid values = 0 or 1


/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class helpHandler : public orbOptions::Handler {
public:

  helpHandler() :
    orbOptions::Handler("help",
			0,
			1,
			"-ORBhelp ",
			1) {}


  void visit(const char*,orbOptions::Source) throw (orbOptions::BadParam) {

    orbOptions::sequenceString_var usage;
    usage = orbOptions::singleton().usageArgv();

    omniORB::logger l;
    l << "Valid -ORB<options> are:\n";
    for (CORBA::ULong i = 0; i < usage->length(); i++)
      l << "  " << usage[i] << "\n";

  }

  void dump(orbOptions::sequenceString& result) {
    return;
  }
};

static helpHandler helpHandler_;

/////////////////////////////////////////////////////////////////////////////
class idHandler : public orbOptions::Handler {
public:

  idHandler() :
    orbOptions::Handler("id",
			"id = " ORB_ID_STRING,
			1,
			"-ORBid "ORB_ID_STRING" (standard option)") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    if (!isValidId(value)) {
      throw orbOptions::BadParam(key(),value,"id is not "ORB_ID_STRING);
    }
    if( strcmp(value, myOrbId()) ) {
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "WARNING -- using ORBid " << value
	  << " (should be " << ORB_ID_STRING << ")." << "\n";
      }
    }
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVString(key(),ORB_ID_STRING,result);
  }
};

static idHandler idHandler_;

/////////////////////////////////////////////////////////////////////////////
class dumpConfigurationHandler : public orbOptions::Handler {
public:

  dumpConfigurationHandler() :
    orbOptions::Handler("dumpConfiguration",
			"dumpConfiguration = 0 or 1",
			1,
			"-ORBdumpConfiguration < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::dumpConfiguration = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::dumpConfiguration,
			     result);
  }
};

static dumpConfigurationHandler dumpConfigurationHandler_;

/////////////////////////////////////////////////////////////////////////////
class lcdModeHandler : public orbOptions::Handler {
public:

  lcdModeHandler() :
    orbOptions::Handler("lcdMode",
			"lcdMode = 0 or 1",
			1,
			"-ORBlcdMode < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::lcdMode = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::lcdMode,
			     result);
  }
};

static lcdModeHandler lcdModeHandler_;


/////////////////////////////////////////////////////////////////////////////
class principalHandler : public orbOptions::Handler {
public:

  principalHandler() :
    orbOptions::Handler("principal",
			"principal = <GIOP 1.0 principal string>",
			1,
			"-ORBprincipal <GIOP 1.0 principal string>") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {
    CORBA::ULong l = strlen(value) + 1;
    omni::myPrincipalID.length(l);
    for (CORBA::ULong i = 0; i < l; i++)
      omni::myPrincipalID[i] = value[i];
  }

  void dump(orbOptions::sequenceString& result) {
    if (omni::myPrincipalID.length() == 0)
      orbOptions::addKVString(key(),"[Null]",result);
    else {
      CORBA::String_var s(CORBA::string_alloc(omni::myPrincipalID.length()+1));
      CORBA::ULong i;
      for (i=0; i<omni::myPrincipalID.length(); i++) {
	((char*)s)[i] = omni::myPrincipalID[i];
      }
      ((char*)s)[i] = '\0';
      orbOptions::addKVString(key(),s,result);
    }
  }
};

static principalHandler principalHandler_;

/////////////////////////////////////////////////////////////////////////////
class poa_iiop_portHandler : public orbOptions::Handler {
public:

#define POA_IIOP_IS_OBSOLETE "is now obsolete, use -ORBendpoint instead"

  poa_iiop_portHandler() :
    orbOptions::Handler("poa_iiop_port",0,1,0) {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    throw orbOptions::BadParam(key(),value,
			       "poa_iiop_port"POA_IIOP_IS_OBSOLETE);
  }

  void dump(orbOptions::sequenceString& result) {
    return;
  }
};

static poa_iiop_portHandler poa_iiop_portHandler_;

/////////////////////////////////////////////////////////////////////////////
class poa_iiop_name_portHandler : public orbOptions::Handler {
public:

  poa_iiop_name_portHandler() :
    orbOptions::Handler("poa_iiop_name_port",0,1,0) {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {
    throw orbOptions::BadParam(key(),value,
			       "poa_iiop_name_port"POA_IIOP_IS_OBSOLETE);
  }

  void dump(orbOptions::sequenceString& result) {
    return;
  }
};

static poa_iiop_name_portHandler poa_iiop_name_portHandler_;

/////////////////////////////////////////////////////////////////////////////
class configFileHandler : public orbOptions::Handler {
public:

  configFileHandler() :
    orbOptions::Handler("configFile",
			"configFile = <filename>",
			1,
			"-ORBconfigFile <filename>") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {
    // Do nothing -- already handled before normal arguments are processed
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVString(key(), config_fname ? config_fname : "[none]",
			    result);
  }
};

static configFileHandler configFileHandler_;


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_corbaOrb_initialiser : public omniInitialiser {
public:

  omni_corbaOrb_initialiser() {
    orbOptions::singleton().registerHandler(helpHandler_);
    orbOptions::singleton().registerHandler(idHandler_);
    orbOptions::singleton().registerHandler(dumpConfigurationHandler_);
    orbOptions::singleton().registerHandler(lcdModeHandler_);
    orbOptions::singleton().registerHandler(principalHandler_);
    orbOptions::singleton().registerHandler(poa_iiop_portHandler_);
    orbOptions::singleton().registerHandler(poa_iiop_name_portHandler_);
    orbOptions::singleton().registerHandler(configFileHandler_);
  }


  void attach() {

#if !defined(__CIAO__)
# if defined(HAVE_SIGACTION)

    struct sigaction act;
    sigemptyset(&act.sa_mask);
#  ifdef HAVE_SIG_IGN
    act.sa_handler = SIG_IGN;
#  else
    act.sa_handler = (void (*)())0;
#  endif
    act.sa_flags = 0;
    if (sigaction(SIGPIPE,&act,0) < 0) {
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "WARNING -- ORB_init() cannot install the\n"
	  " SIG_IGN handler for signal SIGPIPE. (errno = " << errno << ")\n";
      }
    }
# elif defined(HAVE_SIGVEC)
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
# endif // HAVE_SIGACTION
#endif // __CIAO__

    orbAsyncInvoker = new ORBAsyncInvoker(orbParameters::maxServerThreadPoolSize);
    invoker_shutting_down = 0;
  }

  void detach() {
    if (orbAsyncInvoker) {
      if (invoker_threads) {
	omniORB::logs(20, "Wait for ORB invoker threads to finish.");
	while (invoker_threads)
	  invoker_signal.wait();
	omniORB::logs(20, "All ORB invoker threads finished.");
      }
      delete orbAsyncInvoker;
      orbAsyncInvoker = 0;
      invoker_shutting_down = 0;
    }
#ifdef __WIN32__
    (void) WSACleanup();
#endif
  }
};

static omni_corbaOrb_initialiser initialiser;

omniInitialiser& omni_corbaOrb_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
