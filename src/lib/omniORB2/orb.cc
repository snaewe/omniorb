// -*- Mode: C++; -*-
//                            Package   : omniORB2
// orb.cc                     Created on: 20/5/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//      
 
/*
  $Log$
  Revision 1.8  1997/04/22 17:31:45  sll
  - Use getRopeAndKey instead of _rope() and objkey().
  - Added -ORBstrictIIOP command line option.

// Revision 1.7  1997/04/21  10:24:00  ewc
// Minor cosmetic change.
//
// Revision 1.6  1997/03/10  12:10:20  sll
// - Support for ORB and BOA initialisation arguments.
// -  Add runtime configurable trace messages.
// - omni::init will only throws CORBA::INITIALIZE exception if anything goes
//   wrong.
// - Minor cleanup to accomodate the creation of a public API for omniORB2.
//
// Revision 1.5  1997/02/19  11:03:29  ewc
// Added support for NT
//
  Revision 1.4  1997/01/23 16:53:35  sll
  Changed to use the static member variables defined in the class omniORB etc.
  Previously they were local static variables.

// Revision 1.3  1997/01/21  14:25:35  ewc
// Added support for initial reference interface.
//
// Revision 1.2  1997/01/08  18:21:06  ewc
// Corrected bug in omni::iopProfilesToRope (code assumed that profile
// tag was IOP::TAG_INTERNET_IOP)
//
// Revision 1.1  1996/10/10  14:37:53  sll
// Initial revision
//
  */

#include <omniORB2/CORBA.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <string.h>

#ifdef _HAS_SIGNAL
#include <signal.h>
#include <errno.h>
#endif

#if defined(UnixArchitecture)
#include "tcpSocket_UNIX.h"
#elif defined(ATMosArchitecture)
#include "tcpSocket_ATMos.h"
#elif defined(NTArchitecture)
#include "tcpSocket_NT.h"

#else
#error "No tcpSocket.h header for this architecture."
#endif

#if defined(UnixArchitecture)
#include <sys/time.h>
#include <unistd.h>
#elif defined(NTArchitecture)
#include <sys/types.h>
#include <sys/timeb.h>
#endif

#ifdef __atmos__
#include <kernel.h>
#include <timelib.h>
#include <sys/time.h>
#endif

// XXX Some work needs to be done to either remove the explicit reference
//     (and hence dependency) on tcp socket transport or rewrite to incorporate
//     an interface to allow new transports be registered and instantiated.

#if defined(__sunos__) && defined(__sparc__) && defined(SVR4)
#include <unistd.h>
#include <signal.h>
extern "C" void abort()
{
  kill (getpid(),SIGABRT);
  while (1) {
    sleep(1000000);
  }
}

typedef void (*PFV)();
extern PFV set_terminate(PFV);

#endif

class tcpsock_rendezvouser : public omni_thread {
public:
  tcpsock_rendezvouser(tcpSocketRope *r);
  virtual ~tcpsock_rendezvouser();
  virtual void run(void *arg);


private:
  tcpSocketRope *pd_r;
  tcpsock_rendezvouser();
};

class strand_server : public omni_thread {
public:
  strand_server(Strand *s);
  virtual ~strand_server() {}
  virtual void run(void *arg);
private:
  Strand *pd_s;
  Strand::Sync    pd_sync;
};

// XXX Testing code
class killer : public omni_thread {
public:
  killer() { start(); }
  virtual ~killer() {}
  virtual void run(void *arg);
};

static
CORBA::Boolean 
parse_ORB_args(int &argc,char **argv,const char *orb_identifier);

static
CORBA::Boolean
parse_BOA_args(int &argc,char **argv,const char *orb_identifier);

void
omni::init(int &argc,char **argv,const char *orb_identifier)
{
  extern void objectRef_init();

  if (!parse_ORB_args(argc,argv,orb_identifier)) {
    throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
  }

  try {
    objectRef_init();

#ifdef __NT__

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

    // Get configuration information:

    configFile = new initFile;
    configFile->initialize();
  
#else

    // Get configuration information:
    configFile = new initFile;
    configFile->initialize();

#endif

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
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = SIG_IGN;
    act.sa_flags = 0;
    if (sigaction(SIGPIPE,&act,0) < 0) {
      if (omniORB::traceLevel > 0) {
	cerr << "Warning: omni::init() cannot install the SIG_IGN handler for signal SIGPIPE. (errno = " << errno << ")" << endl;
      }
    }
#endif // _HAS_SIGNAL
  }
  catch (const CORBA::INITIALIZE &ex) {
    throw;
  }
  catch (...) {
    throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
  }
  return;
}

void
omni::boaInit(int &argc,char **argv,const char *orb_identifier)
{
  if (!parse_BOA_args(argc,argv,orb_identifier)) {
    throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
  }

  try {
    Rope *r;
    Rope_iterator next(&Anchor::incomingAnchor);
    if (next() == 0) {
      // No port number is specified on the command line using
      // the -BOAiiop_port <port> option
      tcpSocketEndpoint e ((CORBA::Char *)"",0);
      // let the ctor to initialise the host name and port number
      r = new tcpSocketRope(&Anchor::incomingAnchor,0,&e,1,1);
      if (omniORB::traceLevel >= 2) {
	cerr << "Accept IIOP calls on port " << e.port() << endl;
      }
    }
  }
  catch(...) {
    throw  CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
  }      
  return;
}

omniObject* omni::resolveInitRef(const char* identifier)
{
  // Resolve initial references:

  if (strcmp(identifier,"InterfaceRepository") == 0)
    {
      // No Interface Repository
      throw CORBA::INTF_REPOS(10426,CORBA::COMPLETED_NO);
      return NULL;
    }
  else if (strcmp(identifier,"NameService") == 0)
    {
      if (omni::configFile->NameService == NULL)
	{
	  // Failed to get a reference to the Naming Service during ORB 
	  // initialization

	  throw CORBA::NO_RESOURCES(0,CORBA::COMPLETED_NO);
	  return NULL;
	}
      else 
	{
	  return omni::configFile->NameService;
	}
    }
  else
    {
      // No further ObjectIds are defined
      
      throw CORBA::ORB::InvalidName();
      return NULL;
    }
}


unsigned long
omni::listInitServices(char**& servicelist)
{
  // List known initial CORBA Services for which ORB can return a reference
  
  int number_services = 1;  // Number of possible known services

  servicelist = new char*[number_services];

  if (omni::configFile->NameService != NULL)
    {
      servicelist[0] = new char[12];
      strcpy(servicelist[0],"NameService");
    }
  else number_services--;
  
  if (number_services == 0) delete[] servicelist;

  return number_services;
}

Rope *
omni::iopProfilesToRope(const IOP::TaggedProfileList *profiles,
			   _CORBA_Octet *&objkey,
			   size_t &keysize)
{
  CORBA::ULong i;
  for (i=0; i< profiles->length(); i++) {
    // skip any unsupported profile.
    // If there is more than one profile that can be used, choose
    // an optimal one. e.g. use same machine IPC instead of IIOP.
    // For the moment, only IIOP is supported.
    if ((*profiles)[i].tag == IOP::TAG_INTERNET_IOP) {
      break;
    }
    // Add tests here for any other transports that may be supported in future.
  }

  if (i == profiles->length()) {
    // no supported profile is found
    throw CORBA::INV_OBJREF(0,CORBA::COMPLETED_NO);
  }
    
  if ((*profiles)[i].tag == IOP::TAG_INTERNET_IOP) {
    Rope *r;

    IIOP::ProfileBody p;
    
    IIOP::EncapStreamToProfile((*profiles)[i].profile_data,p);

    tcpSocketEndpoint addr(p.host,p.port);

    {
      Rope_iterator next(&Anchor::incomingAnchor);
      while ((r= next())) {
	Endpoint *addrp = &addr;
	if (r->this_is(addrp)) {
	  // found it
	  break;
	}
      }
    }
    if (r) {
      // This is a local object
      if (p.object_key.length() != sizeof(omniObjectKey)) {
	throw CORBA::INV_OBJREF(0,CORBA::COMPLETED_NO);
      }
      objkey = (CORBA::Octet *) (new omniObjectKey);
      if (!objkey)
	throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
      memcpy((void *)objkey,(void *)p.object_key.NP_data(),
	     sizeof(omniObjectKey));
      keysize = sizeof(omniObjectKey);
      return 0;
    }
    {
      // This must be a remote object
      Rope_iterator next(&Anchor::outgoingAnchor);
      while ((r=next())) {
	// Do we have a rope to the same address yet?
	Endpoint *addrp = &addr;
	if (r->remote_is(addrp)) {
	  break;
	}
      }
      if (r) {
	r->incrRefCount(1);
      }
      else {
	// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
	r = new tcpSocketRope(&Anchor::outgoingAnchor,5,&addr,0,1);
	// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
	if (!r)
	  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
	r->incrRefCount(1);
      }
    }
    objkey = new CORBA::Octet[p.object_key.length()];
    if (!objkey)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    memcpy((void *)objkey,(void *)p.object_key.NP_data(),
	   (size_t)p.object_key.length());
    keysize = p.object_key.length();
    return r;
  }
else
  {
    // Invalid object reference
    throw CORBA::INV_OBJREF(0,CORBA::COMPLETED_NO);
    return 0;  // For NT VC++ 4.2
  }

}


IOP::TaggedProfileList *
omni::objectToIopProfiles(omniObject *obj)
{
  omniRopeAndKey rak;
  obj->getRopeAndKey(rak);

  if (obj->is_proxy()) {
    IOP::TaggedProfileList * p = new IOP::TaggedProfileList(1);
    if (!p)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    try {
      obj->pd_rope->iopProfile((CORBA::Char *)rak.key(),
			       rak.keysize(),
			       ((IOP::TaggedProfileList &) *p)[0]);
    }	
    catch (...) {
      delete p;
      throw;
    }
    return p;
  }
  else {
    IOP::TaggedProfileList *p = new IOP::TaggedProfileList;
    if (!p)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    try {
      Rope_iterator next(&Anchor::incomingAnchor);
      Rope *r;
      while ((r = next())) {
	p->length(p->length()+1);
	r->iopProfile((CORBA::Char *)rak.key(),
		      rak.keysize(),
		      ((IOP::TaggedProfileList &)*p)[p->length()-1]);
      }
    }
    catch (...) {
      delete p;
      throw;
    }
    return p;
  }
}


void
omni::orbIsReady()
{
  omni::initLock.lock();
  if (!tcpSocketRendezvous::has_spawned_rendevous_threads) 
    {
      Rope_iterator next(&Anchor::incomingAnchor);
      Rope *r;
      Endpoint *e = 0;
      while ((r = next())) {
	if (r->this_is(e)) {
	  // Spawn rendevous thread for tcpSocketRope
	  // &&&&&&&&&&&&&&&&&&&&&&&&&&&&
	  tcpSocketEndpoint *te = tcpSocketEndpoint::castup(e);
	  if (te) {
	    tcpSocketRope *tr = (tcpSocketRope *)r;
	    if (tr->is_passive()) {
	      tr->incrRefCount(1);
	      tcpsock_rendezvouser *newthr = new tcpsock_rendezvouser(tr);
	      if (!newthr) {
		throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
	      }
	    }
	    // &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
	  }
	  // Test for other transports here, none at the moment
	  e = 0;
	}
      }
      
      //{
      // XXX Testing code
      //killer *thr = new killer();
      //}

      tcpSocketRendezvous::has_spawned_rendevous_threads = 1;
    }
  omni::initLock.unlock();
  return;
}


size_t
omniORB::MaxMessageSize()
{
  return GIOP_Basetypes::max_giop_message_size;
}


tcpsock_rendezvouser::tcpsock_rendezvouser(tcpSocketRope *r)
{
  pd_r = r;
  start();
  return;
}

tcpsock_rendezvouser::~tcpsock_rendezvouser()
{
  if (pd_r) {
    pd_r->decrRefCount();
  }
  pd_r = 0;
  return;
}

void
tcpsock_rendezvouser::run(void *arg)
{
#if defined(__sunos__) && defined(__sparc__) && defined(SVR4)
  set_terminate(abort);
#endif
  if (omniORB::traceLevel >= 5) {
    cerr << "tcpsock_rendezvouser thread: starts." << endl;
  }

  tcpSocketRendezvous *r = pd_r->getRendezvous();
  tcpSocketStrand *newSt = 0;
  strand_server *newthr = 0;
  CORBA::Boolean die = 0;
  while (!die) {
    try {
      newSt = r->accept();
      if (omniORB::traceLevel >= 5) {
	cerr << "tcpsock_rendezvouser thread: accept new strand." << endl;
      }
      newthr = new strand_server(newSt);
      if (!newthr) {
	throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
      }
    }
    catch(omniORB::fatalException &ex) {
      if (omniORB::traceLevel > 0) {
	cerr << "#### You have caught an omniORB2 bug, details are as follows:" << endl;
	cerr << ex.file() << " " << ex.line() << ":" << ex.errmsg() << endl; 
	cerr << "Server rendezvouser thread exited." << endl;
      }
      die = 1;
    }
    catch(...) {
      die = 1;
    }
    if (die && newSt) {
      newSt->decrRefCount();
      newSt->shutdown();
      if (!newthr) {
	if (omniORB::traceLevel >= 5) {
	  cerr << "tcpsock_rendezvouser thread cannot spawn a new server thread."
	       << endl;
	}
      }
    }
  }
  if (omniORB::traceLevel >= 5) {
    cerr << "tcpsock_rendezvouser thread: exits." << endl;
  }
}

strand_server::strand_server(Strand *s) : pd_sync(s,0,0)
{
  pd_s = s;
  pd_s->decrRefCount();
  start();
  return;
}

void
strand_server::run(void *arg)
{
#if defined(__sunos__) && defined(__sparc__) && defined(SVR4)
  set_terminate(abort);
#endif
  
  if (omniORB::traceLevel >= 5) {
    cerr << "strand_server thread: starts." << endl;
  }
  while (1) {
    try {
      GIOP_S::dispatcher(pd_s);
    }
    catch (CORBA::COMM_FAILURE &ex) {
      if (omniORB::traceLevel >= 5) {
	cerr << "#### Commnication failure (minor = " << ex.minor() 
	     << "). Connection closed." << endl;
      }
      break;
    }
    catch(omniORB::fatalException &ex) {
      if (omniORB::traceLevel > 0) {
	cerr << "#### You have caught an omniORB2 bug, details are as follows:" << endl;
	cerr << ex.file() << " " << ex.line() << ":" << ex.errmsg() << endl; 
      }
      break;
    }
#if defined(__sunos__) && defined(__sparc__) && defined(SVR4)
    // let the runtime to detect the uncatched exception and generate
    // a core dump.
#else
    catch (...) {
      if (omniORB::traceLevel > 0) {
	cerr << "#### A system exception has occured and was caught by strand_server thread." << endl;
      }
      abort();  // never returns
    }
#endif
  }
  if (omniORB::traceLevel >= 5) {
    cerr << "strand_server thread: exits." << endl;
  }
}


void
killer::run(void *arg)
{
  while (1)
    {
      (void) omni_thread::sleep(10,0);

      cerr << "Testing code: killer thread wakes up." << endl;
      Rope_iterator next(&Anchor::incomingAnchor);
      Rope *r;
      Endpoint *e = 0;
      while ((r = next())) {
	if (r->this_is(e)) {
	  tcpSocketEndpoint *te = tcpSocketEndpoint::castup(e);
	  if (te) {
	    tcpSocketRope *tr = (tcpSocketRope *)r;
	    if (tr->is_passive()) {
	      Strand_iterator next_strand(r);
	      Strand *s;
	      while ((s = next_strand())) {
		cerr << "Testing code: killer shutdown strand." << endl;
		s->shutdown();
	      }
	    }
	    delete e;
	    e = 0;
	  }
	}
      }
      cerr << "Testing code: killer thread sleeps again." << endl;
    }
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
parse_ORB_args(int &argc,char **argv,const char *orb_identifier)
{
  CORBA::Boolean orbId_match = 0;

  if (orb_identifier && strcmp(orb_identifier,omni::myORBId)!=0)
    {
      if (omniORB::traceLevel > 0) {
	cerr << "CORBA::ORB_init failed: the ORBid ("
	     << orb_identifier << ") is not " << omni::myORBId << endl;
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
	if (strcmp(argv[idx+1],omni::myORBId) != 0)
	  {
	    if (omniORB::traceLevel > 0) {
	      cerr << "CORBA::ORB_init failed: the ORBid ("
		   << argv[idx+1] << ") is not " << omni::myORBId << endl;
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
	if (sscanf(argv[idx+1],"%u",&omniORB::traceLevel) != 1) {
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


static
CORBA::Boolean
parse_BOA_args(int &argc,char **argv,const char *orb_identifier)
{
  CORBA::Boolean orbId_match = 0;
  if (orb_identifier && strcmp(orb_identifier,omni::myBOAId) != 0)
    {
      if (omniORB::traceLevel > 0) {
	cerr << "BOA_init failed: the BOAid ("
	     << orb_identifier << ") is not " <<  omni::myBOAId << endl;
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
	    cerr << "BOA_init failed: missing -BOAid parameter." << endl;
	  }
	  return 0;
	}
	if (strcmp(argv[idx+1],omni::myBOAId) != 0)
	  {
	    if (omniORB::traceLevel > 0) {
	      cerr << "BOA_init failed: the BOAid ("
		   << argv[idx+1] << ") is not " << omni::myBOAId << endl;
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
	    cerr << "BOA_init failed: missing -BOAiiop_port parameter." << endl;
	  }
	  return 0;
	}
	CORBA::ULong port;
	if (sscanf(argv[idx+1],"%u",&port) != 1 ||
            (port == 0 || port >= 65536)) {
	  if (omniORB::traceLevel > 0) {
	    cerr << "BOA_init failed: invalid -BOAiiop_port parameter." << endl;
	  }
	  return 0;
	}
	try {
	  Rope *r;
	  {
	    Rope_iterator anchorLocked(&Anchor::incomingAnchor);
	    tcpSocketEndpoint e ((CORBA::Char *)"",(CORBA::UShort)port);
	    // let the ctor to initialise the host name
	    r = new tcpSocketRope(&Anchor::incomingAnchor,0,&e,1,1);
	    if (omniORB::traceLevel >= 2) {
	      cerr << "Accept IIOP calls on port " << e.port() << endl;
	    }
	  }
	  if (!r)
	    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
	}
	catch (...) {
	  if (omniORB::traceLevel > 0) {
	    cerr << "BOA_init falied: cannot use port " << port
		 << " to accept incoming IIOP calls." << endl;
	  }
	  return 0;
	}
	move_args(argc,argv,idx,2);
	continue;
      }

      // Reach here only if the argument in this form: -ORBxxxxx
      // is not recognised.
      if (omniORB::traceLevel > 0) {
	cerr << "BOA_init failed: unknown BOA argument ("
	     << argv[idx] << ")" << endl;
      }
      return 0;
    }

  if (!orb_identifier && !orbId_match) {
    if (omniORB::traceLevel > 0) {
      cerr << "BOA_init failed: BOAid is not specified." << endl;
    }
    return 0;
  }
  return 1;
}
