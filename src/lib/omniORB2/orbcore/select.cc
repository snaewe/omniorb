
#include "select.h"

PollSet::PollSet(const int max): maxfds(max){
  FD_ZERO(&rfds);
  maxFD = 0;
}


PollSet::PollSet(const PollSet &p): maxfds(p.maxfds), maxFD(p.maxFD){
  //cerr << "copy constructor copying [" << &p << "] ( " << p << " )" << endl;
  memcpy(&rfds, &p.rfds, sizeof(struct fd_set));
  //cerr << "   *this =  [" << this << "] ( " << *this << endl;
}


PollSet::~PollSet(){
}

void PollSet::add(const socket_t fd){
  omni_mutex_lock sync(guard);

  //cerr << "add " << fd << endl;

  FD_SET(fd, &rfds);
  if (fd > maxFD) maxFD = fd;
}

void PollSet::remove(const socket_t fd){
  omni_mutex_lock sync(guard);

  //cerr << "remove " << fd << endl;

  assert (FD_ISSET(fd, &rfds));
  FD_CLR(fd, &rfds);
  // ought to reset maxFD, but that requires a scan through
  // whole bit set
}

ostream &operator << (ostream& s, const PollSet& set){
  omni_mutex_lock sync(set.guard);

  s << "PollSet containing bits [";
  for (int i=0;i<=set.maxFD;i++)
    if (FD_ISSET(i, &set.rfds)) s << i << " ";
  s << "]";

  return s;  
}

PollSet& PollSet::operator = (const PollSet &set){
  maxFD = set.maxFD;

  memcpy(&rfds, &set.rfds, sizeof(struct fd_set));
  return *this;
}


PollSet_Iterator::PollSet_Iterator(PollSet &ps): set(ps){
  //set = ps;
  index = 0;
}

socket_t PollSet_Iterator::operator() (){
  //cerr << "set.maxFD = " << set.maxFD << endl;
  do{
    if (FD_ISSET(index, &set.rfds))
      return index++;
    index++;
  }while (index <= set.maxFD);

  return SOCKET_UNDEFINED;
}

ostream &operator <<(ostream& s, const Poller& ps){
  return s << ps.ps;
}

Poller::Poller(): ps(MAXFDS), active_ps(MAXFDS),
		  state(awake), nthreads_waiting(0),
                  wakeUP_cond(&state_guard) {
  pipe_fd = new socket_t[2];
  if (pipe(pipe_fd) != 0){
    perror("Unable to open internal pipe");
    exit(1);
  }
  ps.add(pipe_fd[0]);
  //cerr << "Should have added: " << pipe_fd[0] << endl;
}

void Poller::wakeUp(){
  char mesg = '!';
  write(pipe_fd[1], &mesg, sizeof(char));
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
  //cerr << "Poller::wait()" << endl;
  //cerr << "set = " << ps << endl;


  while(1){
    // about to block...
    {
      omni_mutex_lock s(state_guard);
      state = asleep;
    }

    while (1){
      // select is destructive
      //cerr << "-- about to sleep --" << endl;
      //cerr << "ps [" << &ps << "] = " << ps << endl;
      active_ps = ps;
      //cerr << "active_ps [" << &active_ps << "] = " << active_ps << endl;
      int result = select(active_ps.maxFD + 1, 
			  &active_ps.rfds, NULL, NULL, NULL);
      //cerr << "selectset = " << active_ps << endl;
      if (result == -1){
	if (errno == EINTR) continue;
	perror("select() returned an error");
	exit(1);
      }
      if (result == 0){
	perror("select() timed out?!");
	exit(1);
      }
      if (result > 0) break;
    }

    {
      omni_mutex_lock s(state_guard);
      state = awake;

      if (FD_ISSET(pipe_fd[0], &active_ps.rfds)){
	char temp;
	if (read(pipe_fd[0], &temp, sizeof(char)) != 1){
	  perror("Error reading from control pipe");
	  exit(1);
	}
	// someone signalled us.. they're now blocked on
	// the condition variable
	nthreads_waiting =- nthreads_waiting;
	wakeUP_cond.broadcast();
	// it's safe for them to do things now... wait 
	// for them all to finish
	while (nthreads_waiting < 0) wakeUP_cond.wait();
	// they've all finished updating my state
	active_ps.remove(pipe_fd[0]);
	// round the loop again
	continue;
      }
    }
    return PollSet_Active_Iterator(active_ps);
  }
}



