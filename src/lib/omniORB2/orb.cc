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
  Revision 1.1  1997/01/08 17:26:01  sll
  Initial revision

  */

#include <omniORB2/CORBA.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _HAS_SIGNAL
#include <signal.h>
#include <errno.h>
#endif

#include "tcpSocket.h"

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

_CORBA_Unbounded_Sequence_w_FixSizeElement<CORBA::Octet,1,1> omniORB::myPrincipalID;

static omni_mutex initLock;
static CORBA::Boolean orb_initialised = 0;
static CORBA::Boolean boa_initialised = 0;

static Anchor incomingAnchor;
static Anchor outgoingAnchor;

static size_t          max_giop_message_size = 256 * 1024;

static CORBA::Boolean has_spawned_rendevous_threads = 0;
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

void
omniORB::init(int &argc,char **argv,const char *orb_identifier)
{
  extern void objectRef_init();
  initLock.lock();
  if (orb_initialised) {
    initLock.unlock();
    return;
  }

  objectRef_init();

  CORBA::ULong l = strlen("nobody")+1;
  CORBA::Octet *p = (CORBA::Octet *) "nobody";
  omniORB::myPrincipalID.length(l);
  unsigned int i;
  for (i=0; i < l; i++) {
    omniORB::myPrincipalID[i] = p[i];
  }

  orb_initialised = 1;

#ifdef _HAS_SIGNAL
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_handler = SIG_IGN;
  act.sa_flags = 0;
  if (sigaction(SIGPIPE,&act,0) < 0) {
    cerr << "Warning: omniORB::init() cannot install the SIG_IGN handler for signal SIGPIPE. (errno = " << errno << ")" << endl;
  }
#endif // _HAS_SIGNAL

  initLock.unlock();
  return;
}

void
omniORB::boaInit(int &argc,char **argv,const char *orb_identifier)
{
  initLock.lock();
  if (boa_initialised) {
    initLock.unlock();
    return;
  }

  Rope *r;
  {
    Rope_iterator anchorLocked(&incomingAnchor);
    tcpSocketEndpoint e ((CORBA::Char *)"",0);
    // let the ctor to initialise the host name and port number
    r = new tcpSocketRope(&incomingAnchor,0,&e,1,1);
  }
  initLock.unlock();
  boa_initialised = 1;
  if (!r)
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  return;
}

Rope *
omniORB::iopProfilesToRope(const IOP::TaggedProfileList *profiles,
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
      Rope_iterator next(&incomingAnchor);
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
      Rope_iterator next(&outgoingAnchor);
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
	r = new tcpSocketRope(&outgoingAnchor,5,&addr,0,1);
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
}


IOP::TaggedProfileList *
omniORB::objectToIopProfiles(omniObject *obj)
{
  if (obj->is_proxy()) {
    IOP::TaggedProfileList * p = new IOP::TaggedProfileList(1);
    if (!p)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    try {
      obj->pd_rope->iopProfile((CORBA::Char *)obj->objkey(),
			       obj->objkeysize(),
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
      Rope_iterator next(&incomingAnchor);
      Rope *r;
      while ((r = next())) {
	p->length(p->length()+1);
	r->iopProfile((CORBA::Char *)obj->objkey(),
		      obj->objkeysize(),
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
omniORB::orbIsReady()
{
  initLock.lock();
  if (!has_spawned_rendevous_threads) 
    {
      Rope_iterator next(&incomingAnchor);
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

      has_spawned_rendevous_threads = 1;
    }
  initLock.unlock();
  return;
}

static int tmp_seed = 1;

void
omniObjectKey::generateNewKey(omniObjectKey &k)
{
  // XXX This is just a temporary hack.
  // Eventually, this function should initialise the object key to
  // a unique ID that is guarantteed not to repeat on the same machine
  // ever.
  initLock.lock();
  k.lo = tmp_seed++;
  initLock.unlock();
  k.med = k.hi = 0;
  return;
}

size_t
omniORB::MaxMessageSize()
{
  return max_giop_message_size;
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
  cerr << "tcpsock_rendezvouser thread: starts." << endl;

  tcpSocketRendezvous *r = pd_r->getRendezvous();
  tcpSocketStrand *newSt = 0;
  strand_server *newthr = 0;
  CORBA::Boolean die = 0;
  while (!die) {
    try {
      newSt = r->accept();
      cerr << "tcpsock_rendezvouser thread: accept new strand." << endl;
      newthr = new strand_server(newSt);
      if (!newthr) {
	throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
      }
    }
    catch(omniORB::fatalException &ex) {
      cerr << "#### You have caught an omniORB2 bug, details are as follows:" << endl;
      cerr << ex.file() << " " << ex.line() << ":" << ex.errmsg() << endl; 
      cerr << "Server rendezvouser thread exited." << endl;
      die = 1;
    }
    catch(...) {
      die = 1;
    }
    if (die && newSt) {
      newSt->decrRefCount();
      newSt->shutdown();
      if (!newthr) {
	cerr << "tcpsock_rendezvouser thread cannot spawn a new server thread." 	<< endl;
      }
    }
  }
  cerr << "tcpsock_rendezvouser thread: exits." << endl;
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

  cerr << "strand_server thread: starts." << endl;
  while (1) {
    try {
      GIOP_S::dispatcher(pd_s);
    }
    catch (CORBA::COMM_FAILURE &ex) {
      cerr << "#### Commnication failure (minor = " << ex.minor() 
	   << "). Connection closed." << endl;
      break;
    }
    catch(omniORB::fatalException &ex) {
      cerr << "#### You have caught an omniORB2 bug, details are as follows:" << endl;
      cerr << ex.file() << " " << ex.line() << ":" << ex.errmsg() << endl; 
      break;
    }
#if defined(__sunos__) && defined(__sparc__) && defined(SVR4)
    // let the runtime to detect the uncatched exception and generate
    // a core dump.
#else
    catch (...) {
      cerr << "#### A system exception has occured and was caught by strand_server thread." << endl;
      abort();  // never returns
    }
#endif
  }
  cerr << "strand_server thread: exits." << endl;
}


void
killer::run(void *arg)
{
  while (1)
    {
      (void) omni_thread::sleep(10,0);

      cerr << "Testing code: killer thread wakes up." << endl;
      Rope_iterator next(&incomingAnchor);
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

#if 0
ropeFactory::ropeFactory()
{
  pd_next = ropeFactories;
  ropeFactories = this;
  return;
}

ropeFactory::~ropeFactory()
{
}

ropeFactory_iterator::ropeFactory_iterator()
{
  pd_f = ropeFactories;
}

ropeFactory *
ropeFactory_iterator::operator() ()
{
  ropeFactory *p = pd_f;
  if (pd_f)
    pd_f = pd_f->pd_next;
  return p;
}
#endif
