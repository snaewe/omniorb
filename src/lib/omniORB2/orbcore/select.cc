
#ifdef __WIN32__
#include <winsock2.h>
#endif

#include "select.h"
#include <omniORB3/CORBA.h>
#include <omniORB3/omniORB.h>
#include <exception.h>

#define PTRACE(prefix,message)  \
  omniORB::logs(15, "select.cc " prefix ": " message)

PollSet::PollSet(const int max): maxfds(max){
	//cerr << "PollSet::PollSet" << endl;
#ifdef __WIN32__
	next = 0;
	handles = (HANDLE*)malloc(sizeof(HANDLE) * maxfds);
	fds = (socket_t*)malloc(sizeof(socket_t) * maxfds);
#else
  FD_ZERO(&rfds);
#endif
  maxFD = 0;
}


PollSet::PollSet(const PollSet &p): maxfds(p.maxfds), maxFD(p.maxFD){
  //cerr << "copy constructor copying [" << &p << "] ( " << p << " )" << endl;
#ifdef __WIN32__
	memcpy(handles,  p.handles, sizeof(HANDLE) * maxfds);
	memcpy(fds, p.fds, sizeof(socket_t) * maxfds);
	next = p.next;
#else
  memcpy(&rfds, &p.rfds, sizeof(struct fd_set));
#endif
  //cerr << "   *this =  [" << this << "] ( " << *this << endl;
}


PollSet::~PollSet(){
}

void PollSet::add(const socket_t fd){
  omni_mutex_lock sync(guard);

  //cerr << "PollSet::add(" << fd << ")" << endl;
#ifdef __WIN32__
	OMNIORB_ASSERT(next < maxfds);
	// create an event object and associate it with activity on this socket
	HANDLE eventObject = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (WSAEventSelect(fd, eventObject, FD_READ | FD_CLOSE | FD_ACCEPT) == SOCKET_ERROR){
		cerr << "WSAEventSelect failed" << endl;
		exit(1);
	}
	*(handles + next) = eventObject;
	*(fds + next) = fd;
	next++;
#else
  FD_SET(fd, &rfds);
#endif
  if (fd > maxFD) maxFD = fd;
}

void PollSet::remove(const socket_t fd){
  omni_mutex_lock sync(guard);

  //cerr << "PollSet::remove(" << fd << ")" << endl;
#ifdef __WIN32__
	int foundIt = 0;
	for (unsigned int i=0;i<next;i++)
		if (*(fds + i) == fd){
			CloseHandle(*(handles + i));
			for (unsigned int j=i+1;j<next;j++){
				*(handles + j - 1) = *(handles + j);
				*(fds + j - 1) = *(fds + j);
			}
			foundIt = 1;
			next --;
			break;
		}
	OMNIORB_ASSERT(foundIt == 1);
#else
  assert (FD_ISSET(fd, &rfds));
  FD_CLR(fd, &rfds);
#endif
  // ought to reset maxFD, but that requires a scan through
  // whole bit set
}

ostream &operator << (ostream& s, const PollSet& set){
  omni_mutex_lock sync(set.guard);

  s << "PollSet containing bits [";
#ifdef __WIN32__
	for (int i=0;i<set.next;i++)
		s << *(set.fds + i) << "[" << *(set.handles + i) << "] ";
#else
  for (int i=0;i<=set.maxFD;i++)
    if (FD_ISSET(i, &set.rfds)) s << i << " ";
#endif
  s << "]";

  return s;  
}

PollSet& PollSet::operator = (const PollSet &set){
  maxFD = set.maxFD;
#ifdef __WIN32__
	memcpy(handles, set.handles, sizeof(HANDLE) * maxfds);
	memcpy(fds, set.fds, sizeof(socket_t) * maxfds);
	next = set.next;
#else
  memcpy(&rfds, &set.rfds, sizeof(struct fd_set));
#endif
  return *this;
}

#ifdef __WIN32__
PollSet_Iterator::PollSet_Iterator(socket_t a): active(a) { }
#else
PollSet_Iterator::PollSet_Iterator(PollSet &ps): set(ps){
  //set = ps;
  index = 0;
}
#endif

socket_t PollSet_Iterator::operator() (){
  //cerr << "set.maxFD = " << set.maxFD << endl;
#ifdef __WIN32__
	if (active){
		socket_t h = active;
		active = 0;
		return h;
	}
	return SOCKET_UNDEFINED;
#else
  do{
    if (FD_ISSET(index, &set.rfds))
      return index++;
    index++;
  }while (index <= set.maxFD);

  return SOCKET_UNDEFINED;
#endif
}

ostream &operator <<(ostream& s, const Poller& ps){
  return s << ps.ps;
}

Poller::Poller(const int maxFDs): ps(maxFDs), active_ps(maxFDs),
		  state(awake), nthreads_waiting(0),
                  wakeUP_cond(&state_guard) {
	//cerr << "Poller::Poller" << endl;
#ifdef __WIN32__
	wakeUpEvent = CreateEvent( NULL,   // no security attributes means
																	   // no inheritance by subprocesses
														 FALSE,  // make it auto-reset
														 FALSE,  // initially unsignalled
														 NULL ); // no name
	if (wakeUpEvent == NULL){
		PTRACE("Poller", "Failed to create event object");
		OMNIORB_THROW(COMM_FAILURE, ::WSAGetLastError(), CORBA::COMPLETED_NO);
	}
	// special (non-socket) event
	*(ps.handles + ps.next) = wakeUpEvent;
	*(ps.fds + ps.next) = 0;
	ps.next++;
#else
  pipe_fd = new socket_t[2];
	pipe_fd[0] = 0; pipe_fd[1] = 0; 
  if (PIPE(pipe_fd) != 0){
    perror("Unable to open internal pipe");
    exit(1);
  }
  ps.add(pipe_fd[0]);
#endif
}

void Poller::wakeUp(){
#ifdef __WIN32__
	if (!SetEvent(wakeUpEvent)){
		PTRACE("Poller", "SetEvent failed to set wakeup event");
		OMNIORB_THROW(COMM_FAILURE, ::WSAGetLastError(), CORBA::COMPLETED_NO);
	}
#else
  char mesg = '!';
  write(pipe_fd[1], &mesg, sizeof(char));
#endif
}

void Poller::add(socket_t x){
  omni_mutex_lock s(state_guard);
  if (state == awake){
    // safe to modify PollSet
    ps.add(x);
    return;
  }
  // wait until nthreads_waiting >=0
  while (nthreads_waiting < 0) wakeUP_cond.wait();
    
  wakeUp();
  nthreads_waiting ++;
  // control thread will flip the sign when its ready
  while (nthreads_waiting > 0) wakeUP_cond.wait();
  
  ps.add(x);
  nthreads_waiting ++;
  wakeUP_cond.broadcast();
  
  // this ones done
}

void Poller::remove(socket_t x){
  omni_mutex_lock s(state_guard);
  if (state == awake){
    // safe to modify PollSet
    ps.remove(x);
    return;
  }
  // wait until nthreads_waiting >=0
  while (nthreads_waiting < 0) wakeUP_cond.wait();
  
  wakeUp();
  nthreads_waiting ++;
  // control thread will flip the sign when its ready
  while (nthreads_waiting > 0) wakeUP_cond.wait();
  
  ps.remove(x);
  nthreads_waiting ++;
  wakeUP_cond.broadcast();
  
  // this ones done
}

PollSet_Active_Iterator Poller::wait(){
	PTRACE("Poller", "wait() called");
 
  while(1){
    // about to block...
    {
      omni_mutex_lock s(state_guard);
      state = asleep;
    }
#ifdef __WIN32__
		DWORD activeIndex;
		HANDLE activeHandle;
		socket_t activeSocket;
#endif
    while (1){
      // select is destructive
			PTRACE("Poller", "About to block waiting for events");
#ifdef __WIN32__
# ifdef EXTRADEBUG
			cerr << "pollset = " << ps << endl;
# endif 
			OMNIORB_ASSERT(ps.next < MAXIMUM_WAIT_OBJECTS);
			activeIndex = WaitForMultipleObjects(ps.next, ps.handles, FALSE, INFINITE);
			OMNIORB_ASSERT( (WAIT_OBJECT_0 <= activeIndex) && (activeIndex < (WAIT_OBJECT_0 + ps.next)) );
# ifdef EXTRADEBUG
			cerr << "Event Index = " << activeIndex << endl;
# endif
			if (activeIndex == WAIT_FAILED){
				PTRACE("Poller", "Wait returned error code");
				OMNIORB_THROW(COMM_FAILURE, ::WSAGetLastError(), CORBA::COMPLETED_NO);
			}
			activeHandle = *(ps.handles + activeIndex - WAIT_OBJECT_0);
			activeSocket = *(ps.fds + activeIndex - WAIT_OBJECT_0);

			// grab the network events only if its not our custom event
			if (activeIndex != 0){
				WSANETWORKEVENTS events;
				if (WSAEnumNetworkEvents(activeSocket, activeHandle, &events) == SOCKET_ERROR){
					PTRACE("Poller", "WSAEnumNetworkEvents returned SOCKET_ERROR");
					OMNIORB_THROW(COMM_FAILURE, ::WSAGetLastError(), CORBA::COMPLETED_NO);
				}
# ifdef EXTRADEBUG
				if (events.lNetworkEvents & FD_READ)
					cerr << "FD_READ (error code " << events.iErrorCode[FD_READ_BIT] << ")" << endl;
				if (events.lNetworkEvents & FD_ACCEPT)
					cerr << "FD_ACCEPT (error code " << events.iErrorCode[FD_ACCEPT_BIT] << ")" << endl;
				if (events.lNetworkEvents & FD_CLOSE)
					cerr << "FD_CLOSE (error code " << events.iErrorCode[FD_CLOSE_BIT] << ")" << endl;
# endif
			}
			// The wakeup event is autoresetting. WSAEnumNetworkEvents should reset those
			// event objects too.
			break;
			
#else
      //cerr << "ps [" << &ps << "] = " << ps << endl;
      active_ps = ps;
      cerr << "active_ps [" << &active_ps << "] = " << active_ps << endl;
      int result = select(active_ps.maxFD + 1, 
			  &active_ps.rfds, NULL, NULL, NULL);
      //cerr << "selectset = " << active_ps << endl;
      if (result == -1){				
				OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
				//if (errno == EINTR) continue;
				//perror("select() returned an error");
				//exit(1);
      }
      if (result == 0){
				perror("select() timed out?!");
				exit(1);
      }
      if (result > 0) break;
#endif
    }
		
    {
      omni_mutex_lock s(state_guard);
      state = awake;
#ifdef __WIN32__
			if (activeHandle == wakeUpEvent){
#else
      if (FD_ISSET(pipe_fd[0], &active_ps.rfds)){
				char temp;
				if (read(pipe_fd[0], &temp, sizeof(char)) != 1){
					perror("Error reading from control pipe");
					exit(1);
				}
#endif
				PTRACE("Poller", "WakeUp event received");
				// someone signalled us.. they're now blocked on
				// the condition variable
				nthreads_waiting =- nthreads_waiting;
				wakeUP_cond.broadcast();
				// it's safe for them to do things now... wait 
				// for them all to finish
				while (nthreads_waiting < 0) wakeUP_cond.wait();
				// they've all finished updating my state
#ifndef __WIN32__
				active_ps.remove(pipe_fd[0]);
#endif
				// round the loop again
				continue;
      }
    }
#ifdef __WIN32__
		return PollSet_Active_Iterator(activeSocket);
#else
    return PollSet_Active_Iterator(active_ps);
#endif
  }
}



