//////////////////////////////////////////////////////////////////////////////
// Filename:	 vxWorks.cc
// Author:		Tihomir Sokcevic
//					Acterna, Eningen.
// Description: vxWorks adaptation of the omnithread wrapper classes
// Notes:		 Munching strategy is imperative
//////////////////////////////////////////////////////////////////////////////
// $Log$
// Revision 1.1.4.4  2005/07/08 17:04:56  dgrisby
// Merge from omni4_0_develop again.
//
// Revision 1.1.4.3  2005/04/25 18:24:23  dgrisby
// Always release per thread data in the thread it belongs to.
//
// Revision 1.1.4.2  2005/01/06 23:11:01  dgrisby
// Big merge from omni4_0_develop.
//
// Revision 1.1.4.1  2003/03/23 21:01:54  dgrisby
// Start of omniORB 4.1.x development branch.
//
// Revision 1.1.2.1  2003/02/17 02:03:11  dgrisby
// vxWorks port. (Thanks Michael Sturm / Acterna Eningen GmbH).
//
// Revision 1.1.1.1  2002/11/19 14:58:04  sokcevti
// OmniOrb4.0.0 VxWorks port
//
// Revision 1.4  2002/10/15 07:54:09  kuttlest
// change semaphore from SEM_FIFO to SEM_PRIO
// ---
//
// Revision 1.3  2002/07/05 07:38:52  engeln
// made priority redefinable on load time by defining int variables
// 	omni_thread_prio_low = 220;
// 	omni_thread_prio_normal = 110;
// 	omni_thread_prio_high = 55;
// the default priority is prio_normal.
// The normal priority default has been increased from 200 to 110 and the
//     high priority from 100 to 55.
// ---
//
// Revision 1.2  2002/06/14 12:44:57  engeln
// replaced possibly unsafe wakeup procedure in broadcast.
// ---
//
// Revision 1.1.1.1  2002/04/02 10:09:34  sokcevti
// omniORB4 initial realease
//
// Revision 1.0	2001/10/23 14:22:45	sokcevti
// Initial Version 4.00
// ---
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Include files
//////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <omnithread.h>
#include <sysLib.h>

#include <assert.h>		// assert
#include <intLib.h>		// intContext


//////////////////////////////////////////////////////////////////////////////
// Local defines
//////////////////////////////////////////////////////////////////////////////
#define ERRNO(x) (((x) != 0) ? (errno) : 0)
#define THROW_ERRORS(x) { if((x) != OK) throw omni_thread_fatal(errno); }
#define OMNI_THREAD_ID	0x7F7155AAl
#define OMNI_STACK_SIZE 60000l

#ifdef _DEBUG
	#include <fstream>
	#define DBG_TRACE(X) X
#else // _DEBUG
	#define DBG_TRACE(X)
#endif // _DEBUG

#define DBG_ASSERT(X)

#define DBG_THROW(X) X

int omni_thread_prio_low = 220;
int omni_thread_prio_normal = 110;
int omni_thread_prio_high = 55;
///////////////////////////////////////////////////////////////////////////
//
// Mutex
//
///////////////////////////////////////////////////////////////////////////
omni_mutex::omni_mutex(void):m_bConstructed(false)
{
	mutexID = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);

	DBG_ASSERT(assert(mutexID != 0));

	if(mutexID==0)
	{
		DBG_TRACE(cout<<"Exception: omni_mutex::omni_mutex()  tid: "<<(int)taskIdSelf()<<endl);
		DBG_THROW(throw omni_thread_fatal(-1));
	}

	m_bConstructed = true;
}

omni_mutex::~omni_mutex(void)
{
	m_bConstructed = false;

	STATUS status = semDelete(mutexID);

	DBG_ASSERT(assert(status == OK));

	if(status != OK)
	{
		DBG_TRACE(cout<<"Exception: omni_mutex::~omni_mutex()  mutexID: "<<(int)mutexID<<" tid: "<<(int)taskIdSelf()<<endl);
		DBG_THROW(throw omni_thread_fatal(errno));
	}
}

/*
void omni_mutex::lock(void)
{
	DBG_ASSERT(assert(!intContext()));		// not in ISR context
	DBG_ASSERT(assert(m_bConstructed));

	STATUS status = semTake(mutexID, WAIT_FOREVER);

	DBG_ASSERT(assert(status == OK));

	if(status != OK)
	{
		DBG_TRACE(cout<<"Exception: omni_mutex::lock()  mutexID: "<<(int)mutexID<<" tid: "<<(int)taskIdSelf()<<endl);
		DBG_THROW(throw omni_thread_fatal(errno));
	}
}

void omni_mutex::unlock(void)
{
	DBG_ASSERT(assert(m_bConstructed));

	STATUS status = semGive(mutexID);

	DBG_ASSERT(assert(status == OK));

	if(status != OK)
	{
		DBG_TRACE(cout<<"Exception: omni_mutex::unlock()  mutexID: "<<(int)mutexID<<" tid: "<<(int)taskIdSelf()<<endl);
		DBG_THROW(throw omni_thread_fatal(errno));
	}
}
*/

///////////////////////////////////////////////////////////////////////////
//
// Condition variable
//
///////////////////////////////////////////////////////////////////////////
omni_condition::omni_condition(omni_mutex* m) : mutex(m)
{
	DBG_TRACE(cout<<"omni_condition::omni_condition  mutexID: "<<(int)mutex->mutexID<<" tid:"<<(int)taskIdSelf()<<endl);

	sema_ = semCCreate(SEM_Q_PRIORITY, 0);
	if(sema_ == 0)
	{
		DBG_TRACE(cout<<"Exception: omni_condition::omni_condition()  tid: "<<(int)taskIdSelf()<<endl);
		DBG_THROW(throw omni_thread_fatal(errno));
	}
}

omni_condition::~omni_condition(void)
{
	STATUS status = semDelete(sema_);

	DBG_ASSERT(assert(status == OK));

	if(status != OK)
	{
		DBG_TRACE(cout<<"Exception: omni_condition::~omni_condition"<<endl);
		DBG_THROW(throw omni_thread_fatal(errno));
	}
}

void omni_condition::wait(void)
{
	STATUS status = OK;

	taskLock();	// disable task switch until semTake
				// else a broadcast may not wakeup
				// this waiting thread
	mutex->unlock();
		// release mutex
	status = semTake(sema_, WAIT_FOREVER);
		// wait at semaphore for a signal or broadcast
	taskUnlock();		// reenable task switch

	DBG_ASSERT(assert(status == OK));

	if(status != OK)
	{
		DBG_TRACE(cout<<"Exception: omni_condition::wait"<<endl);
		DBG_THROW(throw omni_thread_fatal(errno));
	}

	mutex->lock();
		// get owner of mutex before return
}


// The time given is absolute. Return 0 is timeout
int omni_condition::timedwait(unsigned long secs, unsigned long nanosecs)
{
	STATUS result = OK;
	timespec now;
	unsigned long timeout;
	int ticks;

	clock_gettime(CLOCK_REALTIME, &now);

	if(((unsigned long)secs <= (unsigned long)now.tv_sec) &&
		(((unsigned long)secs < (unsigned long)now.tv_sec) ||
		(nanosecs < (unsigned long)now.tv_nsec)))
		timeout = 0;
	else
		timeout = (secs-now.tv_sec) * 1000 + (nanosecs-now.tv_nsec) / 1000000l;

	// disable task lock to have an atomic unlock+semTake
	taskLock();

	// We keep the lock held just long enough to increment the count
	// of waiters by one.
	mutex->unlock();

	// Wait to be awakened by a signal() or broadcast().
	ticks = (timeout * sysClkRateGet()) / 1000L;
	result = semTake(sema_, ticks);

	// reenable task rescheduling
	taskUnlock();

	// We must always regain the <external_mutex>, even when errors
	// occur because that's the guarantee that we give to our callers.
	mutex->lock();

	if(result!= OK) // timeout
		return 0;

	return 1;
}

void omni_condition::signal(void)
{
	DBG_TRACE(cout<<"omni_condition::signal          mutexID: "<<(int)mutex->mutexID<<" tid:"<<(int)taskIdSelf()<<endl);

	STATUS status = semGive(sema_);

	DBG_ASSERT(assert(status == OK));

	if(status != OK)
	{
		DBG_TRACE(cout<<"Exception: omni_condition::signal"<<endl);
		DBG_THROW(throw omni_thread_fatal(errno));
	}
    
    // take unread message if no thread was waiting
    semTake(sema_, NO_WAIT);
}

void omni_condition::broadcast(void)
{
	DBG_TRACE(cout<<"omni_condition::broadcast       mutexID: "<<(int)mutex->mutexID<<" tid:"<<(int)taskIdSelf()<<endl);

	// Wake up all the waiters.
	STATUS status = semFlush(sema_);

	DBG_ASSERT(assert(status == OK));

	if(status != OK)
	{
		DBG_TRACE(cout<<"omni_condition::broadcast1! - thread:"<<omni_thread::self()->id()<<" SemID:"<<(int)sema_<<" errno:"<<errno<<endl);
		DBG_THROW(throw omni_thread_fatal(errno));
	}
}


///////////////////////////////////////////////////////////////////////////
//
// Counting semaphore
//
///////////////////////////////////////////////////////////////////////////
omni_semaphore::omni_semaphore(unsigned int initial)
{

	DBG_ASSERT(assert(0 <= (int)initial));		// POSIX expects only unsigned init values

	semID = semCCreate(SEM_Q_PRIORITY, (int)initial);

	DBG_ASSERT(assert(semID!=0));

	if(semID==0)
	{
		DBG_TRACE(cout<<"Exception: omni_semaphore::omni_semaphore"<<endl);
		DBG_THROW(throw omni_thread_fatal(-1));
	}
}

omni_semaphore::~omni_semaphore(void)
{
	STATUS status = semDelete(semID);

	DBG_ASSERT(assert(status == OK));

	if(status != OK)
	{
		DBG_TRACE(cout<<"Exception: omni_semaphore::~omni_semaphore"<<endl);
		DBG_THROW(throw omni_thread_fatal(errno));
	}
}

void omni_semaphore::wait(void)
{
	DBG_ASSERT(assert(!intContext()));		// no wait in ISR

	STATUS status = semTake(semID, WAIT_FOREVER);

	DBG_ASSERT(assert(status == OK));

	if(status != OK)
	{
		DBG_TRACE(cout<<"Exception: omni_semaphore::wait"<<endl);
		DBG_THROW(throw omni_thread_fatal(errno));
	}
}

int omni_semaphore::trywait(void)
{
	STATUS status = semTake(semID, NO_WAIT);

	DBG_ASSERT(assert(status == OK));

	if(status != OK)
	{
		if(errno == S_objLib_OBJ_UNAVAILABLE)
		{
			return 0;
		}
		else
		{
			DBG_ASSERT(assert(false));

			DBG_TRACE(cout<<"Exception: omni_semaphore::trywait"<<endl);
			DBG_THROW(throw omni_thread_fatal(errno));
		}
	}

	return 1;
}

void omni_semaphore::post(void)
{
	STATUS status = semGive(semID);

	DBG_ASSERT(assert(status == OK));

	if(status != OK)
	{
		DBG_TRACE(cout<<"Exception: omni_semaphore::post"<<endl);
		DBG_THROW(throw omni_thread_fatal(errno));
	}
}



///////////////////////////////////////////////////////////////////////////
//
// Thread
//
///////////////////////////////////////////////////////////////////////////


//
// static variables
//
omni_mutex* omni_thread::next_id_mutex = 0;
int omni_thread::next_id = 0;

// omniORB requires a larger stack size than the default (21120) on OSF/1
static size_t stack_size = OMNI_STACK_SIZE;


//
// Initialisation function (gets called before any user code).
//

static int& count() {
  static int the_count = 0;
  return the_count;
}

omni_thread::init_t::init_t(void)
{
	// Only do it once however many objects get created.
	if(count()++ != 0)
		return;

	attach();
}

omni_thread::init_t::~init_t(void)
{
    if (--count() != 0) return;

    omni_thread* self = omni_thread::self();
    if (!self) return;

    taskTcb(taskIdSelf())->spare1 = 0;
    delete self;

    delete next_id_mutex;
}


//
// Wrapper for thread creation.
//
extern "C" void omni_thread_wrapper(void* ptr)
{
	omni_thread* me = (omni_thread*)ptr;

	DBG_TRACE(cout<<"omni_thread_wrapper: thread "<<me->id()<<" started\n");

	//
	// We can now tweaked the task info since the tcb exist now
	//
	me->mutex.lock();	// To ensure that start has had time to finish
	taskTcb(me->tid)->spare1 = OMNI_THREAD_ID;
	taskTcb(me->tid)->spare2 = (int)ptr;
	me->mutex.unlock();

	//
	// Now invoke the thread function with the given argument.
	//
	if(me->fn_void != 0)
	{
		(*me->fn_void)(me->thread_arg);
		omni_thread::exit();
	}

	if(me->fn_ret != 0)
	{
		void* return_value = (*me->fn_ret)(me->thread_arg);
		omni_thread::exit(return_value);
	}

	if(me->detached)
	{
		me->run(me->thread_arg);
		omni_thread::exit();
	}
	else
	{
		void* return_value = me->run_undetached(me->thread_arg);
		omni_thread::exit(return_value);
	}
}


//
// Special functions for VxWorks only
//
void omni_thread::attach(void)
{
	DBG_TRACE(cout<<"omni_thread_attach: VxWorks mapping thread initialising\n");

	int _tid = taskIdSelf();

	// Check the task is not already attached
	if(taskTcb(_tid)->spare1 == OMNI_THREAD_ID)
		return;

	// Create the mutex required to lock the threads debugging id (create before the thread!!!)
	if(next_id_mutex == 0)
		next_id_mutex = new omni_mutex;

	// Create a thread object for THIS running process
	omni_thread* t = new omni_thread;

	// Lock its mutex straigh away!
	omni_mutex_lock l(t->mutex);

	// Adjust data members of this instance
	t->_state = STATE_RUNNING;
	t->tid = taskIdSelf();

	// Set the thread values so it can be recongnised as a omni_thread
	// Set the id last can possibly prevent race condition
	taskTcb(t->tid)->spare2 = (int)t;
	taskTcb(t->tid)->spare1 = OMNI_THREAD_ID;

	// Create the running_mutex at this stage, but leave it empty. We are not running
	//	in the task context HERE, so taking it would be disastrous.
	t->running_cond = new omni_condition(&t->mutex);
}


void omni_thread::detach(void)
{
	DBG_TRACE(cout<<"omni_thread_detach: VxWorks detaching thread mapping\n");

	int _tid = taskIdSelf();

	// Check the task has a OMNI_THREAD attached
	if(taskTcb(_tid)->spare1 != OMNI_THREAD_ID)
		return;

	// Invalidate the id NOW !
	taskTcb(_tid)->spare1 = 0;

	// Even if 0, it is safe to delete the thread
	omni_thread* t = (omni_thread*)taskTcb(_tid)->spare2;
	// Fininsh cleaning the tcb structure
	taskTcb(_tid)->spare2 = 0;

	delete t;
}


//
// Constructors for omni_thread - set up the thread object but don't
// start it running.
//

// construct a detached thread running a given function.
omni_thread::omni_thread(void (*fn)(void*), void* arg, priority_t pri)
{
	common_constructor(arg, pri, 1);
	fn_void = fn;
	fn_ret = 0;
}

// construct an undetached thread running a given function.
omni_thread::omni_thread(void* (*fn)(void*), void* arg, priority_t pri)
{
	common_constructor(arg, pri, 0);
	fn_void = 0;
	fn_ret = fn;
}

// construct a thread which will run either run() or run_undetached().

omni_thread::omni_thread(void* arg, priority_t pri)
{
	common_constructor(arg, pri, 1);
	fn_void = 0;
	fn_ret = 0;
}

// common part of all constructors.
void omni_thread::common_constructor(void* arg, priority_t pri, int det)
{
	_state = STATE_NEW;
	_priority = pri;

	// Set the debugging id
	next_id_mutex->lock();
	_id = next_id++;
	next_id_mutex->unlock();

	// Note : tid can only be setup when the task is up and running
	tid = 0;

	thread_arg = arg;
	detached = det;		// may be altered in start_undetached()

    _dummy       = 0;
    _values      = 0;
    _value_alloc = 0;
}

//
// Destructor for omni_thread.
//
omni_thread::~omni_thread(void)
{
    DBG_TRACE(cout<<"omni_thread::~omni_thread for thread "<<id()<<endl);

    if (_values) {
        for (key_t i=0; i < _value_alloc; i++) {
          if (_values[i]) {
              delete _values[i];
          }
        }
      delete [] _values;
    }

    // glblock -- added this to prevent problem with unitialized running_cond
    if(running_cond)
      {
        delete running_cond;
      }
}


//
// Start the thread
//
void omni_thread::start(void)
{
	omni_mutex_lock l(mutex);

	DBG_ASSERT(assert(_state == STATE_NEW));

	if(_state != STATE_NEW)
		DBG_THROW(throw omni_thread_invalid());

	// Allocate memory for the task. (The returned id cannot be trusted by the task)
	tid = taskSpawn(
		0,				// Task name
		vxworks_priority(_priority),	// Priority
		VX_FP_TASK | VX_NO_STACK_FILL,	// Option
		stack_size,			// Stack size
		(FUNCPTR)omni_thread_wrapper,	// Entry point
		(int)this,			// First argument is this
		0,0,0,0,0,0,0,0,0		// Remaining unused args
		);

	DBG_ASSERT(assert(tid!=ERROR));

	if(tid==ERROR)
		DBG_THROW(throw omni_thread_invalid());

	_state = STATE_RUNNING;

	// Create the running_mutex at this stage, but leave it empty. We are not running
	//	in the task context HERE, so taking it would be disastrous.
	running_cond = new omni_condition(&mutex);
}


//
// Start a thread which will run the member function run_undetached().
//
void omni_thread::start_undetached(void)
{
	DBG_ASSERT(assert(!((fn_void != 0) || (fn_ret != 0))));

	if((fn_void != 0) || (fn_ret != 0))
		DBG_THROW(throw omni_thread_invalid());

	detached = 0;

	start();
}


//
// join - Wait for the task to complete before returning to the calling process
//
void omni_thread::join(void** status)
{
	mutex.lock();

	if((_state != STATE_RUNNING) && (_state != STATE_TERMINATED))
	{
		mutex.unlock();

		DBG_ASSERT(assert(false));

		DBG_THROW(throw omni_thread_invalid());
	}

	mutex.unlock();

	DBG_ASSERT(assert(this != self()));

	if(this == self())
		DBG_THROW(throw omni_thread_invalid());

	DBG_ASSERT(assert(!detached));

	if(detached)
		DBG_THROW(throw omni_thread_invalid());

	mutex.lock();
	running_cond->wait();
	mutex.unlock();

	if(status)
		*status = return_val;

	delete this;
}


//
// Change this thread's priority.
//
void omni_thread::set_priority(priority_t pri)
{
	omni_mutex_lock l(mutex);

	DBG_ASSERT(assert(_state == STATE_RUNNING));

	if(_state != STATE_RUNNING)
	{
		DBG_THROW(throw omni_thread_invalid());
	}

	_priority = pri;

	if(taskPrioritySet(tid, vxworks_priority(pri))==ERROR)
	{
		DBG_ASSERT(assert(false));

		DBG_THROW(throw omni_thread_fatal(errno));
	}
}


//
// create - construct a new thread object and start it running.	Returns thread
// object if successful, null pointer if not.
//

// detached version (the entry point is a void)
omni_thread* omni_thread::create(void (*fn)(void*), void* arg, priority_t pri)
{
	omni_thread* t = new omni_thread(fn, arg, pri);

	t->start();

	return t;
}

// undetached version (the entry point is a void*)
omni_thread* omni_thread::create(void* (*fn)(void*), void* arg, priority_t pri)
{
	omni_thread* t = new omni_thread(fn, arg, pri);

	t->start();

	return t;
}


//
// exit() _must_ lock the mutex even in the case of a detached thread.	This is
// because a thread may run to completion before the thread that created it has
// had a chance to get out of start().	By locking the mutex we ensure that the
// creating thread must have reached the end of start() before we delete the
// thread object.	Of course, once the call to start() returns, the user can
// still incorrectly refer to the thread object, but that's their problem.
//
void omni_thread::exit(void* return_value)
{
	omni_thread* me = self();

	if(me)
	{
		me->mutex.lock();

		me->return_val = return_value;
		me->_state = STATE_TERMINATED;
		me->running_cond->signal();

		me->mutex.unlock();

		DBG_TRACE(cout<<"omni_thread::exit: thread "<<me->id()<<" detached "<<me->detached<<" return value "<<(int)return_value<<endl);

		if (me->_values) {
		  for (key_t i=0; i < me->_value_alloc; i++) {
		    if (me->_values[i]) {
		      delete me->_values[i];
		    }
		  }
		  delete [] me->_values;
		  me->_values = 0;
		}

		if(me->detached)
			delete me;
	}
	else
		DBG_TRACE(cout<<"omni_thread::exit: called with a non-omnithread. Exit quietly."<<endl);

	taskDelete(taskIdSelf());
}


omni_thread* omni_thread::self(void)
{
	if(taskTcb(taskIdSelf())->spare1 != OMNI_THREAD_ID)
		return 0;

	return (omni_thread*)taskTcb(taskIdSelf())->spare2;
}


void omni_thread::yield(void)
{
	taskDelay(NO_WAIT);
}


void omni_thread::sleep(unsigned long secs, unsigned long nanosecs)
{
	int tps = sysClkRateGet();

	// Convert to us to avoid overflow in the multiplication
	//	tps should always be less than 1000 !
	nanosecs /= 1000;

	taskDelay(secs*tps + (nanosecs*tps)/1000000l);
}


void omni_thread::get_time( unsigned long* abs_sec,
							unsigned long* abs_nsec,
							unsigned long rel_sec,
							unsigned long rel_nsec)
{
	timespec abs;
	clock_gettime(CLOCK_REALTIME, &abs);
	abs.tv_nsec += rel_nsec;
	abs.tv_sec += rel_sec + abs.tv_nsec / 1000000000;
	abs.tv_nsec = abs.tv_nsec % 1000000000;
	*abs_sec = abs.tv_sec;
	*abs_nsec = abs.tv_nsec;
}


int omni_thread::vxworks_priority(priority_t pri)
{
	switch (pri)
	{
	case PRIORITY_LOW:
		return omni_thread_prio_low;

	case PRIORITY_NORMAL:
		return omni_thread_prio_normal;

	case PRIORITY_HIGH:
		return omni_thread_prio_high;
	}

	DBG_ASSERT(assert(false));

	DBG_THROW(throw omni_thread_invalid());
}


void omni_thread::stacksize(unsigned long sz)
{
	stack_size = sz;
}


unsigned long omni_thread::stacksize()
{
	return stack_size;
}


void omni_thread::show(void)
{
	omni_thread *pThread;
	int s1, s2;
	int tid = taskIdSelf();

	printf("TaskId is %.8x\n", tid);

	s1 = taskTcb(tid)->spare1;

	if(s1 != OMNI_THREAD_ID)
	{
		printf("Spare 1 is %.8x, and not recongnized\n", s1);

		return;
	}
	else
	{
		printf("Spare 1 indicate an omni_thread.\n");
	}

	s2 = taskTcb(tid)->spare2;

	if(s2 == 0)
	{
		printf("Spare 2 is 0! - No thread object attached !!\n");

		return;
	}
	else
	{
		printf("Thread object at %.8x\n", s2);
	}

	pThread = (omni_thread *)s2;

	state_t status = pThread->_state;

	printf("	| Thread status is ");

	switch (status)
	{
	case STATE_NEW:
		printf("NEW\n");		break;
	case STATE_RUNNING:
		printf("STATE_RUNNING\n"); break;
	case STATE_TERMINATED:
		printf("TERMINATED\n");	break;
	default:
		printf("Illegal (=%.8x)\n", (unsigned int)status);

		return;
	}

	if(pThread->tid != tid)
	{
		printf("	| Task ID in thread object is different!! (=%.8x)\n", pThread->tid);

		return;
	}
	else
	{
		printf("	| Task ID in thread consistent\n");
	}

	printf("\n");
}


//
// Dummy thread
//

class omni_thread_dummy : public omni_thread {
public:
  inline omni_thread_dummy() : omni_thread()
  {
    // glblock -- added this to prevent problem with unitialized
    // running_cond the dummy thread never uses this and we dont want
    // the destructor to delete it.  vxWorks compiler seems to not set
    // unitialized vars to 0.
    running_cond = 0;

    _dummy = 1;
    _state = STATE_RUNNING;

    // Adjust data members of this instance
    tid = taskIdSelf();
    DBG_TRACE(cout<<"created dummy "<<(void*)tid<<endl);
    // Set the thread values so it can be recongnised as a omni_thread
    // Set the id last can possibly prevent race condition
    taskTcb(tid)->spare2 = (int)this;
    taskTcb(tid)->spare1 = OMNI_THREAD_ID;
  }
  inline ~omni_thread_dummy()
  {
    DBG_TRACE(cout<<"omni thread dummy destructor " <<endl);
    taskTcb(taskIdSelf())->spare1 = 0;
  }
};


omni_thread*
omni_thread::create_dummy()
{
  if (omni_thread::self())
    throw omni_thread_invalid();

  return new omni_thread_dummy;
}

void
omni_thread::release_dummy()
{
  omni_thread* self = omni_thread::self();
  if (!self || !self->_dummy)
    throw omni_thread_invalid();

  omni_thread_dummy* dummy = (omni_thread_dummy*)self;
  delete dummy;
}
