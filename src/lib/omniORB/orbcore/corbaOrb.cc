// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaOrb.cc                Created on: 6/2/96
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
//      Implementation of the ORB interface
//	

/*
  $Log$
  Revision 1.7  1997/12/09 18:29:46  sll
  Merged in code from orb.cc
  Updated to use the new rope factory interface.
  Old code to deal with server side thread management removed.

// Revision 1.6  1997/05/06  15:11:03  sll
// Public release.
//
 */

#include <omniORB2/CORBA.h>
#include <stdio.h>
#include <initFile.h>
#include <scavenger.h>
#include <ropeFactory.h>
#ifndef __atmos__
#include <tcpSocket.h>
#define _tcpOutgoingFactory tcpSocketMToutgoingFactory
#else
#include <tcpATMos.h>
#define _tcpOutgoingFactory tcpATMosMToutgoingFactory
#endif
#ifdef _HAS_SIGNAL
#include <signal.h>
#include <errno.h>
#endif


// Globals defined in class omniORB
CORBA::ULong                    omniORB::traceLevel = 1;
CORBA::Boolean                  omniORB::strictIIOP = 0;
_CORBA_Unbounded_Sequence_Octet omni::myPrincipalID;

static const char*       myORBId          = "omniORB2";
static CORBA::ORB_ptr    orb              = 0;
static CORBA::Object_ptr NameServiceRef   = 0; 
static omni_mutex        internalLock;


// constants

static
CORBA::Boolean 
parse_ORB_args(int &argc,char **argv,const char *orb_identifier);


CORBA::
ORB::ORB()
{
  return;
}

CORBA::
ORB::~ORB()
{
  return;
}


CORBA::ORB_ptr
CORBA::ORB_init(int &argc,char **argv,const char *orb_identifier)
{
  omni_mutex_lock sync(internalLock);

  if (!parse_ORB_args(argc,argv,orb_identifier)) {
    throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
  }
  if (orb)
    return CORBA::ORB::_duplicate(orb);

  try {
    // initialise object tables
    omniObject::globalInit();

    // Now initialise all the rope factories that will be used to
    // create outgoing ropes.
    globalOutgoingRopeFactories.insert(new _tcpOutgoingFactory );
    // Add rope factories for other transports here.

    // get configuration information:
    {
      initFile configFile;
      configFile.initialize();
      NameServiceRef = configFile.NameService();
    }

    // myPrincipalID, to be used in the principal field of IIOP calls
    CORBA::ULong l = strlen("nobody")+1;
    CORBA::Octet *p = (CORBA::Octet *) "nobody";
    omni::myPrincipalID.length(l);
    unsigned int i;
    for (i=0; i < l; i++) {
      omni::myPrincipalID[i] = p[i];
    }
    
    omniORB::seed.hi = omniORB::seed.med = 0;

#ifdef _HAS_SIGNAL
#ifndef _USE_MACH_SIGNAL
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = SIG_IGN;
    act.sa_flags = 0;
    if (sigaction(SIGPIPE,&act,0) < 0) {
      if (omniORB::traceLevel > 0) {
	cerr << "Warning: omni::init() cannot install the SIG_IGN handler for signal SIGPIPE. (errno = " << errno << ")" << endl;
      }
    }
#else
    struct sigvec act;
    act.sv_mask = 0;
    act.sv_handler = SIG_IGN;
    act.sv_flags = 0;
    if (sigvec(SIGPIPE,&act,0) < 0) {
      if (omniORB::traceLevel > 0) {
	cerr << "Warning: omni::init() cannot install the SIG_IGN handler for signal SIGPIPE. (errno = " << errno << ")" << endl;
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


    // Start the thread to cleanup idle outgoing strands.
    StrandScavenger::initOutScavenger();
  }
  catch (const CORBA::INITIALIZE &ex) {
    throw;
  }
  catch (...) {
    throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
  }

  orb = new CORBA::ORB;
  return orb;
}

CORBA::Object_ptr 
CORBA::
ORB::resolve_initial_references(const char* identifier)
{
  // Resolve initial references:
  if (strcmp(identifier,"InterfaceRepository") == 0)
    {
      // No Interface Repository
      throw CORBA::INTF_REPOS(0,CORBA::COMPLETED_NO);
    }
  else if (strcmp(identifier,"NameService") == 0)
    {
      if (CORBA::is_nil(NameServiceRef))
	{
	  // Failed to get a reference to the Naming Service during ORB 
	  // initialization
	  throw CORBA::NO_RESOURCES(0,CORBA::COMPLETED_NO);
	}
      else 
	{
	  return CORBA::Object::_duplicate(NameServiceRef);
	}
    }
  else
    {
      // No further ObjectIds are defined
      throw CORBA::ORB::InvalidName();
    }
#ifdef NEED_DUMMY_RETURN
  return CORBA::Object::_nil(); // dummy return to keep some compilers happy
#endif
}


CORBA::ORB::ObjectIdList* 
CORBA::
ORB::list_initial_services()
{
  ObjectIdList* idlist = new ObjectIdList;
  idlist->length(0);

  if (NameServiceRef != 0)
    {
      idlist->length(1);
      (*idlist)[0] = (const char*) "NameService";
    }
  return idlist;
}


CORBA::
ORB::InvalidName::InvalidName(const ORB::InvalidName &_s)
{
}

CORBA::ORB::InvalidName&
CORBA::ORB::InvalidName::operator=(const ORB::InvalidName &_s)
{
  return *this;
}

size_t
CORBA::
ORB::InvalidName::NP_alignedSize(size_t _initialoffset)
{
  return _initialoffset;
}

void
CORBA::ORB::InvalidName::operator>>= (NetBufferedStream &_n)
{
}

void
CORBA::ORB::InvalidName::operator<<= (NetBufferedStream &_n)
{
}

void
CORBA::
ORB::InvalidName::operator>>= (MemBufferedStream &_n)
{
}

void
CORBA::
ORB::InvalidName::operator<<= (MemBufferedStream &_n)
{
}

char *
CORBA::
ORB::object_to_string(CORBA::Object_ptr p)
{
  if (CORBA::is_nil(p))
    return omni::objectToString(0);
  else 
    return omni::objectToString(p->PR_getobj());
}

CORBA::Object_ptr
CORBA::
ORB::string_to_object(const char *m)
{
  omniObject *objptr = omni::stringToObject(m);
  if (objptr)
    return (CORBA::Object_ptr)(objptr->_widenFromTheMostDerivedIntf(0));
  else
    return CORBA::Object::_nil();
}

CORBA::ORB_ptr
CORBA::
ORB::_duplicate(CORBA::ORB_ptr p)
{
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
  return (p == 0) ? 1 : 0;
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

  if (orb_identifier && strcmp(orb_identifier,myORBId)!=0)
    {
      if (omniORB::traceLevel > 0) {
	cerr << "CORBA::ORB_init failed: the ORBid ("
	     << orb_identifier << ") is not " << myORBId << endl;
      }
      return 0;
    }

  int idx = 1;
  while (argc > idx) 
    {
      // -ORBxxxxxxx ??
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
	  if (omniORB::traceLevel > 0) {
	    cerr << "CORBA::ORB_init failed: missing -ORBid parameter." << endl;
	  }
	  return 0;
	}
	if (strcmp(argv[idx+1],myORBId) != 0)
	  {
	    if (omniORB::traceLevel > 0) {
	      cerr << "CORBA::ORB_init failed: the ORBid ("
		   << argv[idx+1] << ") is not " << myORBId << endl;
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
	  if (omniORB::traceLevel > 0) {
	    cerr << "CORBA::ORB_init failed: missing -ORBtraceLevel parameter."
		 << endl;
	  }
	  return 0;
	}
#if SIZEOF_LONG == 4
	if (sscanf(argv[idx+1],"%lu",&omniORB::traceLevel) != 1) {
#else
	if (sscanf(argv[idx+1],"%u",&omniORB::traceLevel) != 1) {
#endif
	  if (omniORB::traceLevel > 0) {
	    cerr << "CORBA::ORB_init failed: invalid -ORBtraceLevel parameter."
		 << endl;
	  }
	  return 0;
	}
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBstrictIIOP
      if (strcmp(argv[idx],"-ORBstrictIIOP") == 0) {
	if((idx+1) >= argc) {
	  if (omniORB::traceLevel > 0) {
	    cerr << "CORBA::ORB_init failed: missing -ORBstrictIIOP parameter (0 or 1)."
		 << endl;
	  }
	  return 0;
	}
	unsigned int v;
	if (sscanf(argv[idx+1],"%u",&v) != 1) {
	  if (omniORB::traceLevel > 0) {
	    cerr << "CORBA::ORB_init failed: invalid -ORBstrictIIOP parameter."
		 << endl;
	  }
	  return 0;
	}
	omniORB::strictIIOP = ((v)?1:0);
	move_args(argc,argv,idx,2);
	continue;
      }

      // -ORBgiopMaxMsgSize
      if (strcmp(argv[idx],"-ORBgiopMaxMsgSize") == 0) {
	if((idx+1) >= argc) {
	  if (omniORB::traceLevel > 0) {
	    cerr << "CORBA::ORB_init failed: missing -ORBgiopMaxMsgSize parameter."
		 << endl;
	  }
	  return 0;
	}
	CORBA::ULong sz;
#if SIZEOF_LONG == 4
	if (sscanf(argv[idx+1],"%lu",&sz) != 1) {
#else
	if (sscanf(argv[idx+1],"%u",&sz) != 1) {
#endif
	  if (omniORB::traceLevel > 0) {
	    cerr << "CORBA::ORB_init failed: invalid -ORBgiopMaxMsgSize parameter."
		 << endl;
	  }
	  return 0;
	}
	omniORB::MaxMessageSize(sz);
	move_args(argc,argv,idx,2);
	continue;
      }

      
      // Reach here only if the argument in this form: -ORBxxxxx
      // is not recognised.
      if (omniORB::traceLevel > 0) {
	cerr << "CORBA::ORB_init failed: unknown ORB argument ("
	     << argv[idx] << ")" << endl;
      }
      return 0;
    }

  if (!orb_identifier && !orbId_match) {
    if (omniORB::traceLevel > 0) {
      cerr << "CORBA::ORB_init failed: ORBid is not specified." << endl;
    }
    return 0;
  }
  return 1;
}


