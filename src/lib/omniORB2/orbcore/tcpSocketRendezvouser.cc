
////////////////////////////////////////////////////////////////////////////
// Implementation of the abstract base class tcpSocketRendezvouser
tcpSocketRendezvouser::tcpSocketRendezvouser(tcpSocketIncomingRope *r,
					     tcpSocketMTincomingFactory *f):
  omni_thread(r), pd_factory(f){
  cerr << "tcpSocketRendezvouser::pd_factory = " << pd_factory << endl;
  // the actual thread should be started last, by the most derived 
  // constructor
}

void*
tcpSocketRendezvouser::run_undetached(void *arg)
{
  tcpSocketIncomingRope* r = (tcpSocketIncomingRope*) arg;

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x420
  set_terminate(abort);
#endif
#endif
  PTRACE("Rendezvouser","start.");

  CORBA::Boolean die = 0;

  while (r->pd_shutdown == tcpSocketIncomingRope::ACTIVE  && !die) {
    try {
      waitForEvents(r);
    }
    catch(CORBA::COMM_FAILURE&) {
      // XXX accepts failed. The probable cause is that the number of
      //     file descriptors opened has exceeded the limit.
      //     On unix, the value of this limit can be set and get using the
      //              ulimit command.
      //     On NT, if this is part of a DLL, the limit is 256(?)
      //            else the limit is 16(?)
      // The following is a temporary fix, this thread just wait for a while
      // and tries again. Hopfully, some connections might be freed by then.
      PTRACE("Rendezvouser","accept fails. Too many file descriptors opened?");
      omni_thread::sleep(1,0);
      continue;
    }
    catch(omniORB::fatalException& ex) {
      if (omniORB::trace(0)) {
	omniORB::logger l;
	l << "You have caught an omniORB bug, details are as follows:\n" <<
	  " file: " << ex.file() << "\n"
	  " line: " << ex.line() << "\n"
	  " mesg: " << ex.errmsg() << "\n"
	  " tcpSocketMT Rendezvouser thread will not accept new connection.\n";
      }
      die = 1;
    }
    catch(...) {
      omniORB::logs(0,
       "Unexpected exception caught by tcpSocketMT Rendezvouser\n"
       " tcpSocketMT Rendezvouser thread will not accept new connection.");
      die = 1;
    }
  }
  if (die) {
    // Something very wrong has happened, before we quit, we have to
    // make sure that a future call by another thread to
    // tcpSocketIncomingRope::cancelThreads() would not wait indefinitely
    // on this thread to response to its connect() request to the
    // rendezous socket. 
    // Shutdown (not close) the rendezvous socket to make sure that the
    // connect() in cancelThreads() would fail.
    SHUTDOWNSOCKET(r->pd_rendezvous);
  }

  while (r->pd_shutdown != tcpSocketIncomingRope::NO_THREAD) {

    // We keep on accepting connection requests but close the new sockets
    // immediately until the state of pd_shutdown changes to NO_THREAD.
    // This is to make sure that the thread calling cancelThreads()
    // will be unblocked from the connect() call.

    {
      fd_set rdfds;
#     ifndef __CIAO__
      FD_ZERO(&rdfds);
      FD_SET(r->pd_rendezvous,&rdfds);
#     endif
      struct timeval t = { 1,0};
      int rc;
      if ((rc = select(r->pd_rendezvous+1,&rdfds,0,0,&t)) <= 0) {
#ifndef __WIN32__
	if (rc < 0 && errno != EINTR) {
	  die = 1;
	}
#else
 	if (rc < 0 && ::WSAGetLastError() != WSAEINTR) {
	  die = 1;
	}
#endif
	PTRACE("Rendezvouser","waiting on shutdown state to change to NO_THREAD.");
	continue;
      }
    }

    tcpSocketHandle_t new_sock;
    struct sockaddr_in raddr;
#if (defined(__GLIBC__) && __GLIBC__ >= 2)
    // GNU C library uses socklen_t * instead of int* in accept ().
    // This is suppose to be compatible with the upcoming POSIX standard.
    socklen_t l;
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
    size_t l;
#else
    int l;
#endif

    l = sizeof(struct sockaddr_in);
    if ((new_sock = ::accept(r->pd_rendezvous,(struct sockaddr *)&raddr,&l)) 
	      == RC_INVALID_SOCKET) 
      {
	die = 1;
	continue;
      }
    CLOSESOCKET(new_sock);
  }

  PTRACE("Rendezvouser","exit.");
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// Implementation of the class OneToOneRendezvouser
void OneToOneRendezvouser::waitForEvents(tcpSocketIncomingRope *r){
  tcpSocketHandle_t new_sock;
  struct sockaddr_in raddr;
  tcpSocketStrand *newSt = 0;

#if (defined(__GLIBC__) && __GLIBC__ >= 2)
  // GNU C library uses socklen_t * instead of int* in accept ().
  // This is suppose to be compatible with the upcoming POSIX standard.
  socklen_t l;
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
  size_t l;
#else
  int l;
#endif

  l = sizeof(struct sockaddr_in);

  PTRACE("Rendezvouser","block on accept()");

  if ((new_sock = ::accept(r->pd_rendezvous,(struct sockaddr *)&raddr,&l)) 
      == RC_INVALID_SOCKET) {
#ifndef __WIN32__
    OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
    OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
  }
  PTRACE("Rendezvouser","unblock from accept()");

  {
    omni_mutex_lock sync(r->pd_lock);

    if (r->pd_shutdown != tcpSocketIncomingRope::ACTIVE) {
      // It has been indicated that this thread should stop
      // accepting connection request.
      CLOSESOCKET(new_sock);
      return;
    }
    
    newSt = new tcpSocketStrand(r,new_sock);
    newSt->incrRefCount(1);
  }

  newConnectionAttempted(newSt);
}

void OneToOneRendezvouser::newConnectionAttempted(tcpSocketStrand *newSt){
  tcpStrandWorker *newthr = 0;

  PTRACE("Rendezvouser","accept new strand.");
  omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
  if (pd_factory->pd_shutdown_nthreads >= 0) {
    pd_factory->pd_shutdown_nthreads++;
  }
  else {
    pd_factory->pd_shutdown_nthreads--;
  }

  try {
    newthr = new OneToOneWorker(newSt,pd_factory);
  }
  catch(...) {
    newthr = 0;
  }
  if (!newthr) {
    // Cannot create a new thread to serve the strand
    // We have no choice but to shutdown the strand.
    // The long term solutions are:  start multiplexing the new strand
    // and the rendezvous; close down idle connections; reasign
    // threads to strands; etc.
    newSt->decrRefCount();
    newSt->real_shutdown();

    omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
    OMNIORB_ASSERT(pd_factory->pd_shutdown_nthreads != 0);
    if (pd_factory->pd_shutdown_nthreads > 0) {
      pd_factory->pd_shutdown_nthreads--;
    }
    else {
      pd_factory->pd_shutdown_nthreads++;
      pd_factory->pd_shutdown_cond.signal();
    }

  }
}

int maxstrands = getdtablesize();

PoolRendezvouser::PoolRendezvouser(tcpSocketIncomingRope *r,
				   tcpSocketMTincomingFactory *f):
  tcpSocketRendezvouser(r, f), pd_factory(f){
    //cerr << "PoolRendezvouser::pd_factory = " << pd_factory << endl;
    //cerr << "maxstrands set to " << maxstrands << endl;

  { 
    // FIXME: is this data potentially shared?

    // Build the fd -> tcpSocketStrand* table
    fdstrandmap = new tcpSocketStrand*[maxstrands];
    // Clean table entries
    for (int i=0;i<maxstrands;i++) fdstrandmap[i] = NULL;
    // Initialise the queue if hasn't been done already
    if (!queue)
      queue = new FixedQueue<tcpSocketStrand*>(queueLength);

  }

#ifdef THREADPOOL
  start_undetached();
#endif
}

void PoolRendezvouser::buildPool(tcpSocketMTincomingFactory *f){
  PTRACE("PoolRendezvouser", "buildPool");
  // build the thread pool
  for (unsigned int i=0;i<poolSize;i++) new PoolWorker(this, f);
}

void *PoolRendezvouser::run_undetached(void *arg){
  PTRACE("PoolRendezvouser", "::run_undetached");
  tcpSocketIncomingRope* r = (tcpSocketIncomingRope*) arg;

  buildPool(pd_factory);

#ifdef THREADPOOL
  rendezvous = this;
#endif 

  poller.add(r->pd_rendezvous);

  return tcpSocketRendezvouser::run_undetached(arg);
}

void PoolRendezvouser::watchStrand(tcpSocketStrand *s){
  poller.add(s->handle());
}

void PoolRendezvouser::waitForEvents(tcpSocketIncomingRope *r){
  tcpSocketHandle_t new_sock;
  struct sockaddr_in raddr;

  PTRACE("PoolRendezvouser","::waitForEvents");
#if (defined(__GLIBC__) && __GLIBC__ >= 2)
  // GNU C library uses socklen_t * instead of int* in accept ().
  // This is suppose to be compatible with the upcoming POSIX standard.
  socklen_t l;
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
  size_t l;
#else
  int l;
#endif

  l = sizeof(struct sockaddr_in);

#ifdef EXTRADEBUG
  {
    PollSet_Active_Iterator set = poller.wait();
    socket_t fd;
    while ((fd = set()) != SOCKET_UNDEFINED){
      cerr << "active fd = " << fd << endl;
    }
  }
#endif

  PollSet_Active_Iterator set = poller.wait();
  socket_t fd;
  while ((fd = set()) != SOCKET_UNDEFINED){
#ifdef EXTRADEBUG
    fprintf(stderr, "fd = %d\n", fd);
#endif
    if (fd == r->pd_rendezvous){
      PTRACE("PoolRendezvouser", "new connection attempted");
#ifdef EXTRADEBUG
      fflush(stderr);
#endif
      if ((new_sock = ::accept(r->pd_rendezvous,(struct sockaddr *)&raddr,&l)) 
	  == RC_INVALID_SOCKET) {
#ifndef __WIN32__
	OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
	OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
      }
#ifdef EXTRADEBUG
      cerr << "accepted fd = " << new_sock << endl;
      fflush(stderr);
#endif
      PTRACE("PoolRendezvouser","unblock from select(), accept() successful");
      {
	omni_mutex_lock sync(r->pd_lock);
	
	if (r->pd_shutdown != tcpSocketIncomingRope::ACTIVE) {
	  // It has been indicated that this thread should stop
	  // accepting connection request.
	  CLOSESOCKET(new_sock);
	  return;
	}
	
	tcpSocketStrand *newSt = new tcpSocketStrand(r,new_sock);
	newSt->incrRefCount(1);

	newConnectionAttempted(newSt);
	
      }
      // data available on an existing strand

      
      PTRACE("PoolRendezvouser","accept new strand.");

      OMNIORB_ASSERT(pd_factory != NULL);
      omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
      if (pd_factory->pd_shutdown_nthreads >= 0) {
	pd_factory->pd_shutdown_nthreads++;
      } else {
	pd_factory->pd_shutdown_nthreads--;
      }

      continue;
    }
    // which strands were the requests on?
    tcpSocketStrand *s = fdstrandmap[fd];
    OMNIORB_ASSERT(s != NULL);
    strandIsActive(s);
  }      
}

void PoolRendezvouser::strandIsActive(tcpSocketStrand *s){
  PTRACE("PoolRendezvouser", "strandIsActive");

  // mark this strand as uninteresting
  poller.remove(s->handle());

  // add this strand to the Queue of things to process
  queue->add(s);
}

void PoolRendezvouser::newConnectionAttempted(tcpSocketStrand *s){
  PTRACE("PoolRendezvouser", "newConnectionAttempted");
  int new_sock = s->handle();

  // mark this strand as interesting
  poller.add(new_sock);

  // add the strand to the lookup table
  OMNIORB_ASSERT(new_sock < maxstrands);
  fdstrandmap[new_sock] = s;
}

#if 0
void SelectSignalRendezvouser::newConnectionAttempted(tcpSocketStrand *s){
  PTRACE("SelectSignalRendezvouser", "newConnectionAttempted");
  int new_sock = s->handle();

  sem_wait(FD_semaphore);
  {
    FD_SET(new_sock, &interestingFDs);
  }
  sem_post(FD_semaphore);

  if (new_sock > highestFD) highestFD = new_sock;
  // is it important to reset that after removing a FD?

  // add the strand to the lookup table
  OMNIORB_ASSERT(new_sock < maxstrands);
  fdstrandmap[new_sock] = s;
}

void SelectSignalRendezvouser::strandIsActive(tcpSocketStrand *s){
  PTRACE("SelectSignalRendezvouser", "strandIsActive");
  sem_wait(FD_semaphore);
  {
    // mark this strand as uninteresting
    FD_CLR(s->handle(), &interestingFDs);
  }
  sem_post(FD_semaphore);
  // add this strand to the Queue of things to process
  queue->add(s);
}

// Updates the state of the rendezvouser thread to make it watch
// a strand again for more data. Send a signal to the thread to
// force it to rescan its filedescriptor set.
void SelectSignalRendezvouser::watchStrand(tcpSocketStrand *s){
  PTRACE("SelectSignalRendezvouser", "watchStrand called");
  // mark the strand as interesting again
#ifdef EXTRADEBUG
  cerr << "Watching strand fd = " << s->handle() << " again" << endl;
  fflush(stderr);
#endif
  // this handle is actually a strand isn't it?
  OMNIORB_ASSERT(s->handle() < maxstrands);
  OMNIORB_ASSERT(rendezvous->fdstrandmap[s->handle()]);

  PTRACE("Rendezvouser", " adding FD to interestingFDs [acquiring sem]");
  sem_wait(rendezvous->FD_semaphore);
  {
    FD_SET(s->handle(), &rendezvous->interestingFDs);
    cerr << "watchStrand interestingFDs (" << (&interestingFDs) << ") = ";
    dump_fdset(&interestingFDs, highestFD);
    fflush(stderr);
  }
  sem_post(rendezvous->FD_semaphore);
  PTRACE("Rendezvouser", " added FD                    [released sem]");

  PTRACE("Rendezvouser", "about to signal [aquiring lock]");
  rendezvous->allowed_to_signal.lock();
  {
    pthread_kill(rendez_thread, wake_signal);
  }
  rendezvous->allowed_to_signal.unlock();

  sem_wait(rendezvous->FD_semaphore);
  cerr << "watchStrand interestingFDs (" << (&interestingFDs) << ") = ";
  dump_fdset(&interestingFDs, highestFD);
  fflush(stderr);
  sem_post(rendezvous->FD_semaphore);

  PTRACE("Rendezvouser", "signal sent     [released lock]");
}

void dump_fdset(fd_set *f, int highestFD){
  cerr << "highest = " << highestFD << " [";
  for (int i=0;i<=highestFD;i++)
    if (FD_ISSET(i, f)) cerr << i << "  ";
  cerr << "]" << endl;
}

void*
SelectSignalRendezvouser::run_undetached(void *arg)
{
  tcpSocketIncomingRope* r = (tcpSocketIncomingRope*) arg;

  PTRACE("SelectSignalRendezvouser","start.");

  // HACK - get the pthreads thread ID
  rendez_thread = pthread_self();
  rendezvous = this;
  struct sigaction handler = {
    &handle_signal,
    NULL, /* alternative handler */
    0,    /* only block this signal */
    0,    /* no special flags */
    0
  };
  sigaction(wake_signal, &handler, NULL); 
  //signal(wake_signal, &handle_signal);
  // HACK
  
  // build the interesting bitmap of file descriptors
  FD_ZERO(&interestingFDs);
  // we're always interested in new connections
  FD_SET(r->pd_rendezvous, &interestingFDs);
  highestFD = r->pd_rendezvous;

  // noone is allowed to signal us unless we know it is safe
  allowed_to_signal.lock();

  return tcpSocketRendezvouser::run_undetached(arg);
}

void SelectSignalRendezvouser::waitForEvents(tcpSocketIncomingRope *r){
  tcpSocketHandle_t new_sock;
  struct sockaddr_in raddr;

  // select() modifies the bitmap in place so we need to copy it
  // Worker threads are also able to modify the interestingFD set
  // while we aren't looking and then cause us to take a fresh new
  // local copy.
  
  // readFDs are shared- semaphores are the only async-signal safe 
  sem_wait(FD_semaphore);
  {
#ifdef EXTRADEBUG
    cerr << "waitForEvents interestingFDs (" << (&interestingFDs) << " = "; 
    dump_fdset(&interestingFDs, highestFD);
#endif EXTRADEBUG 
    memcpy(&readFDs, &interestingFDs, sizeof(fd_set));
  }
  sem_post(FD_semaphore);

#if (defined(__GLIBC__) && __GLIBC__ >= 2)
  // GNU C library uses socklen_t * instead of int* in accept ().
  // This is suppose to be compatible with the upcoming POSIX standard.
  socklen_t l;
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
  size_t l;
#else
  int l;
#endif

  l = sizeof(struct sockaddr_in);

  int signal_result;
      
  // ----------------------------------------------------------
  // A worker thread can signal us safely while we are in here
  // (safe == does not screw up important other syscalls) 
  allowed_to_signal.unlock();
  {
    PTRACE("Rendezvouser","blocking on select()");
#ifdef EXTRADEBUG
    cerr << "Blocking with Read FDs = ";
    dump_fdset(&readFDs, highestFD);
#endif
    signal_result = ::select(highestFD + 1, &readFDs, NULL, NULL, NULL);
  } 
  allowed_to_signal.lock();
  // ----------------------------------------------------------

  // dodgy hack- in case readFDs got clobbered after select
  sem_wait(FD_semaphore);
  {
    memcpy(&readFDs, &interestingFDs, sizeof(fd_set));
  }
  sem_post(FD_semaphore);
      
  signal_result = ::select(highestFD + 1, &readFDs, NULL, NULL, NULL);
	
#ifdef EXTRADEBUG
  cerr << "signal_result == " << signal_result << endl;
  cerr << "select resultFDs == ";
  sem_wait(FD_semaphore);
  dump_fdset(&readFDs, highestFD);
  sem_post(FD_semaphore);
  fflush(stderr);
#endif

  if (signal_result == -1){
    // Received a signal while blocked in select()
    PTRACE("Rendezvouser", "SIGNAL unblocked select");
    // This means either that:
    //  * A worker thread has finished reading a request on
    //    a strand and wishes that strand to be monitored for
    //    new incoming requests again
    //  * A signal in some other context indicating something
    //    really bad has happened
    // Assuming that noone else will send this particular signal
    // type, assume the former.
    
    // Just go round the loop again refreshing the bitmap and
    // reblocking
  } else {

    // is it a new connection?
    if (FD_ISSET(r->pd_rendezvous, &readFDs)){
      PTRACE("Rendezvouser", "new connection attempted");
#ifdef EXTRADEBUG
      cerr << "accept?" << endl;
      fflush(stderr);
#endif
      if ((new_sock = ::accept(r->pd_rendezvous,(struct sockaddr *)&raddr,&l)) 
	  == RC_INVALID_SOCKET) {
#ifndef __WIN32__
	OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
	OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
      }
#ifdef EXTRADEBUG
      cerr << "accepted." << endl;
      fflush(stderr);
#endif
      PTRACE("Rendezvouser","unblock from select(), accept() successful");

      {
	omni_mutex_lock sync(r->pd_lock);
	
	if (r->pd_shutdown != tcpSocketIncomingRope::ACTIVE) {
	  // It has been indicated that this thread should stop
	  // accepting connection request.
	  CLOSESOCKET(new_sock);
	  return;
	}
	
	tcpSocketStrand *newSt = new tcpSocketStrand(r,new_sock);
	newSt->incrRefCount(1);

	newConnectionAttempted(newSt);

      }

      PTRACE("Rendezvouser","accept new strand.");

      OMNIORB_ASSERT(pd_factory != NULL);
      omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
      if (pd_factory->pd_shutdown_nthreads >= 0) {
	pd_factory->pd_shutdown_nthreads++;
      } else {
	pd_factory->pd_shutdown_nthreads--;
      }
	
    } else {
      // (code is only ever reached iff the rendezvouser is looking
      // for incoming GIOP data)

      // which strands were the requests on?
      int nactive = signal_result;
      int fd = 0;
      while (nactive){
	OMNIORB_ASSERT(fd <= highestFD);
	if (FD_ISSET(fd, &readFDs)){
	  OMNIORB_ASSERT(fd < maxstrands);
	  tcpSocketStrand *s = fdstrandmap[fd];
	  OMNIORB_ASSERT(s != NULL);
	  strandIsActive(s);

	  // need to make this thread higher priority than any activated
	  // worker because it is still holding the signal mutex?
	  nactive--;
	}
	fd++;
      }
    }
  }
}
#endif
