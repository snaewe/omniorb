// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
  Revision 1.29.2.5  2001/06/06 10:36:41  sll
  Backported workaround for Sun C++ 5.0 or Forte WS C++ 6.0.

  Revision 1.29.2.4  2000/01/07 15:44:56  djr
  Call timeouts are now disabled by default.

  Revision 1.29.2.3  2000/01/05 17:19:03  djr
  Added check for reinitialisation in ORB_init and BOA_init.

  Revision 1.29.2.2  1999/09/22 18:43:58  sll
  Updated documentation

  Revision 1.29.2.1  1999/09/21 20:37:16  sll
  -Simplified the scavenger code and the mechanism in which connections
   are shutdown. Now only one scavenger thread scans both incoming
   and outgoing connections. A separate thread do the actual shutdown.
  -omniORB::scanGranularity() now takes only one argument as there is
   only one scan period parameter instead of 2.
  -Trace messages in various modules have been updated to use the logger
   class.
  -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                 -ORBscanIncomingPeriod.

  Revision 1.29  1999/09/01 13:17:11  djr
  Update to use new logging support.

  Revision 1.28  1999/08/30 16:53:04  sll
  Added new options -ORBclientCallTimeOutPeriod, -ORBserverCallTimeOutPeriod
  -ORBscanOutgoingPeriod, -ORBscanIncomingPeriod and -ORBhelp.

// Revision 1.27  1999/08/16  20:27:26  sll
// *** empty log message ***
//
// Revision 1.26  1999/08/16  19:21:38  sll
// Use per-compilation unit initialiser object to perform initialisation
// and cleanup.
// New member function CORBA::ORB::NP_destory().
//
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

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <stdio.h>
#include <stdlib.h>
#include <bootstrap_i.h>
#ifdef _HAS_SIGNAL
#include <signal.h>
#include <errno.h>
#endif


// Globals defined in class omniORB
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the variables external linkage otherwise. Its a bug.
namespace omniORB {

int                      traceLevel = 1;
CORBA::Boolean           strictIIOP = 1;
char*                    serverName = 0;
CORBA::Boolean           tcAliasExpand = 0;
unsigned int             maxTcpConnectionPerServer = 5;
CORBA::Boolean           diiThrowsSysExceptions = 1;
CORBA::Boolean           useTypeCodeIndirections = 1;
CORBA::Boolean           acceptMisalignedTcIndirections = 0;
CORBA::Boolean           verifyObjectExistsAndType = 1;
CORBA::Boolean           abortOnInternalError = 0;
}

#else
int                             omniORB::traceLevel = 1;
CORBA::Boolean                  omniORB::strictIIOP = 1;
#if defined(HAS_Cplusplus_Namespace)
char*                           omniORB::serverName = 0;
#else
CORBA::String_var		omniORB::serverName((const char*) "unknown");
#endif
CORBA::Boolean                  omniORB::tcAliasExpand = 0;
unsigned int                    omniORB::maxTcpConnectionPerServer = 5;
CORBA::Boolean                  omniORB::diiThrowsSysExceptions = 1;
CORBA::Boolean                  omniORB::useTypeCodeIndirections = 1;
CORBA::Boolean                  omniORB::acceptMisalignedTcIndirections = 0;
CORBA::Boolean                  omniORB::verifyObjectExistsAndType = 1;
CORBA::Boolean                  omniORB::abortOnInternalError = 0;
#endif

_CORBA_Unbounded_Sequence_Octet omni::myPrincipalID;



static const char*       myORBId          = "omniORB2";
static CORBA::ORB_ptr    orb              = 0;
static int               orb_destroyed    = 0;
static omni_mutex        internalLock;

static const char*       bootstrapAgentHostname = 0;
static CORBA::UShort     bootstrapAgentPort     = 900;


#ifdef __SINIX__
// Why haven't we got this signature from signal.h? - sll
//
extern "C" int sigaction(int, const struct sigaction *, struct sigaction *);
#endif


///////////////////////////////////////////////////////////////////////
//          Per module initialisers.
//
extern omniInitialiser& omni_corbaOrb_initialiser_;
extern omniInitialiser& omni_ropeFactory_initialiser_;
extern omniInitialiser& omni_objectRef_initialiser_;
extern omniInitialiser& omni_initFile_initialiser_;
extern omniInitialiser& omni_bootstrap_i_initialiser_;
extern omniInitialiser& omni_strand_initialiser_;
extern omniInitialiser& omni_scavenger_initialiser_;

static
CORBA::Boolean
parse_ORB_args(int &argc,char **argv,const char *orb_identifier);


CORBA::
ORB::ORB()
{
  pd_magic = CORBA::ORB::PR_magic;
}

CORBA::
ORB::~ORB()
{
  pd_magic = 0;
}

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x500

#include <exception.h>
static void omni_abort()
{
  abort();
}

#endif
#endif

CORBA::ORB_ptr
CORBA::ORB_init(int &argc,char **argv,const char *orb_identifier)
{
  omni_mutex_lock sync(internalLock);

  if( orb_destroyed ) {
    omniORB::logs(1, "The ORB cannot be re-initialised!");
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);
  }

  if (!parse_ORB_args(argc,argv,orb_identifier)) {
    throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
  }
  if (orb)
    return CORBA::ORB::_duplicate(orb);

  if ((char*)omniORB::serverName == 0) {
    omniORB::serverName = CORBA::string_dup("unknown");
  }

  omniORB::seed.hi = omniORB::seed.med = 0;

  try {

    // Call attach method of each initialiser object.
    // The order of these calls must take into account of the dependency
    // amount the modules.
    omni_corbaOrb_initialiser_.attach();
    omni_strand_initialiser_.attach();
    omni_scavenger_initialiser_.attach();
    omni_ropeFactory_initialiser_.attach();
    omni_objectRef_initialiser_.attach();
    omni_initFile_initialiser_.attach();
    omni_bootstrap_i_initialiser_.attach();

    if (bootstrapAgentHostname) {
      // The command-line option -ORBInitialHost has been specified.
      // Override any previous NamesService object reference
      // that may have been read from the configuration file.
      omniInitialReferences::singleton()->set("NameService",
					      CORBA::Object::_nil());
      omniInitialReferences::singleton()->set("InterfaceRepository",
					      CORBA::Object::_nil());
      omniInitialReferences::singleton()
	->initialise_bootstrap_agent(bootstrapAgentHostname,
				     bootstrapAgentPort);
    }
  }
  catch (const CORBA::INITIALIZE &ex) {
    throw;
  }
  catch (...) {
    throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
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

  orb = new CORBA::ORB;
  return orb;
}

CORBA::Object_ptr
CORBA::
ORB::resolve_initial_references(const char* identifier)
{
  if (!identifier) throw CORBA::ORB::InvalidName();

  CORBA::Object_ptr obj = omniInitialReferences::singleton()->get(identifier);

  if (!CORBA::is_nil(obj)) {
    return obj;
  }
  else {
    // Resource not found. 
    if (strcmp(identifier,"InterfaceRepository") == 0) {
      // No Interface Repository
      throw CORBA::INTF_REPOS(0,CORBA::COMPLETED_NO);
    }
    else if (strcmp(identifier,"NameService") == 0) {
      throw CORBA::NO_RESOURCES(0,CORBA::COMPLETED_NO);
    }
    else {
      // No further ObjectIds are defined
      throw CORBA::ORB::InvalidName();
    }
  }
#ifdef NEED_DUMMY_RETURN
  return CORBA::Object::_nil(); // dummy return to keep some compilers happy
#endif
}


CORBA::ORB::ObjectIdList*
CORBA::
ORB::list_initial_services()
{
  CORBA_InitialReferences::ObjIdList_var v;
  v = omniInitialReferences::singleton()->list();
  CORBA::ORB::ObjectIdList* result = new CORBA::ORB::ObjectIdList;
  result->length(v->length());
  for (CORBA::ULong index = 0; index < v->length(); index++) {
    (*result)[index] = v[index];
  }
  return result;
}


char*
CORBA::
ORB::object_to_string(CORBA::Object_ptr p)
{
  if (!CORBA::Object::PR_is_valid(p))
    // throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
    throw CORBA::OBJECT_NOT_EXIST(0,CORBA::COMPLETED_NO);

  if (CORBA::is_nil(p))
    return omni::objectToString(0);
  else 
    return omni::objectToString(p->PR_getobj());
}


CORBA::Object_ptr
CORBA::
ORB::string_to_object(const char *m)
{
  if (!m || strlen(m) == 0) 
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
    
  try {
    omniObject *objptr = omni::stringToObject(m);
    if (objptr)
      return (CORBA::Object_ptr)(objptr->_widenFromTheMostDerivedIntf(0));
    else
      return CORBA::Object::_nil();
  }
  catch(...) {
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
  }
}


CORBA::ORB_ptr
CORBA::
ORB::_duplicate(CORBA::ORB_ptr p)
{
  if (!PR_is_valid(p)) throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
  return p;
}


CORBA::ORB_ptr
CORBA::
ORB::_nil()
{
  return 0;
}


void
CORBA::release(ORB_ptr p)
{
  return;
}


CORBA::Boolean
CORBA::is_nil(ORB_ptr p)
{
  if (!CORBA::ORB::PR_is_valid(p))
    return 0;
  else
    return ((p == 0) ? 1 : 0);
}

void
CORBA::
ORB::NP_destroy()
{
  assert(this == orb);

  omni_mutex_lock sync(internalLock);

  // Call detach method of the initialisers in reverse order.
  omni_bootstrap_i_initialiser_.detach();
  omni_initFile_initialiser_.detach();
  omni_objectRef_initialiser_.detach();
  omni_ropeFactory_initialiser_.detach();
  omni_scavenger_initialiser_.detach();
  omni_strand_initialiser_.detach();
  omni_corbaOrb_initialiser_.detach();

  delete orb;
  orb = 0;
  orb_destroyed = 1;
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
parse_ORB_args(int &argc,char **argv,const char *orb_identifier)
{
  CORBA::Boolean orbId_match = 0;

  if (orb_identifier && strcmp(orb_identifier,myORBId)!=0) {
    if( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "CORBA::ORB_init failed -- the ORBid (" << orb_identifier << ")\n"
	" is not " << myORBId << "\n";
    }
    return 0;
  }

  if (argc > 0) {
    // Using argv[0] as the serverName implicitly assumes that the
    // argv array follows the unix tradition of passing the command name
    // as the first argument. This may not be the case on other platforms
    // and the application may choose to create its own argv array instead
    // of passing the command line arguments to ORB_init in verbatim form.
    //
    // XXX Should we trim this to a leafname?
#ifdef HAS_Cplusplus_Namespace
    if (omniORB::serverName) CORBA::string_free(omniORB::serverName);
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
	  omniORB::logs(1,
			"CORBA::ORB_init failed: missing -ORBid parameter.");
	  return 0;
	}
	if (strcmp(argv[idx+1],myORBId) != 0)
	  {
	    if( omniORB::trace(1) ) {
	      omniORB::logger l;
	      l << "CORBA::ORB_init failed -- the ORBid (" <<
		argv[idx+1] << ") is not " << myORBId << "\n";
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
	if (omniORB::serverName) CORBA::string_free(omniORB::serverName);
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
			" -ORBclientCallTimeOutPeriod parameter.");
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

      // -ORBscanGranularity
      if( strcmp(argv[idx],"-ORBscanGranularity") == 0 ) {
	if( idx + 1 >= argc ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: missing"
			" -ORBscanGranularity parameter.");
	  return 0;
	}
	unsigned int v;
	if( sscanf(argv[idx+1], "%u", &v) != 1 ) {
	  omniORB::logs(1, "CORBA::ORB_init failed: invalid"
			" -ORBscanGranularity parameter.");
	  return 0;
	}
	omniORB::scanGranularity(v);
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
	  "    -ORBscanGranularity <n seconds>\n"
	  "    -ORBlcdMode\n";
	move_args(argc,argv,idx,1);
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


void
omniORB::enableLcdMode()
{
  omniORB::strictIIOP = 0;
  omniORB::tcAliasExpand = 1;
  omniORB::idleConnectionScanPeriod(omniORB::idleIncoming, 0);
  omniORB::idleConnectionScanPeriod(omniORB::idleOutgoing, 0);
  omniORB::useTypeCodeIndirections = 0;
  omniORB::acceptMisalignedTcIndirections = 1;
}

omniORB::
fatalException::fatalException(const char *file,int line,const char *errmsg)
  : pd_file(file), pd_line(line), pd_errmsg(errmsg)
{
  if (omniORB::abortOnInternalError) abort();
}

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_corbaOrb_initialiser : public omniInitialiser {
public:

  void attach() {

    // myPrincipalID, to be used in the principal field of IIOP calls
    CORBA::ULong l = strlen("nobody")+1;
    CORBA::Octet *p = (CORBA::Octet *) "nobody";
    omni::myPrincipalID.length(l);
    unsigned int i;
    for (i=0; i < l; i++) {
      omni::myPrincipalID[i] = p[i];
    }


#if defined(_HAS_SIGNAL) && !defined(__CIAO__)
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
      {
	// Couldn't find a usable DLL.
	throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
      }

    // Confirm that the returned Windows Sockets DLL supports 1.1

    if ( LOBYTE( wData.wVersion ) != 1 ||
	 HIBYTE( wData.wVersion ) != 1 )
      {
	// Couldn't find a usable DLL
	WSACleanup();
	throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
      }

#endif
  }

  void detach() {
#ifdef __WIN32__
    (void) WSACleanup();
#endif
  }
};

static omni_corbaOrb_initialiser initialiser;

omniInitialiser& omni_corbaOrb_initialiser_ = initialiser;
