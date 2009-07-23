// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyThreadCache.cc           Created on: 2000/05/26
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2005 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambridge
//
//    This file is part of the omniORBpy library
//
//    The omniORBpy library is free software; you can redistribute it
//    and/or modify it under the terms of the GNU Lesser General
//    Public License as published by the Free Software Foundation;
//    either version 2.1 of the License, or (at your option) any later
//    version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
//    MA 02111-1307, USA
//
//
// Description:
//    Cached mapping from threads to PyThreadState and
//    threading.Thread objects

// $Id$

// $Log$
// Revision 1.1.4.6  2005/07/22 17:41:07  dgrisby
// Update from omnipy2_develop.
//
// Revision 1.1.4.5  2005/04/25 18:28:04  dgrisby
// Minor log output changes.
//
// Revision 1.1.4.4  2005/03/02 13:39:16  dgrisby
// Another merge from omnipy2_develop.
//
// Revision 1.1.4.3  2005/01/25 11:45:48  dgrisby
// Merge from omnipy2_develop; set RPM version.
//
// Revision 1.1.4.2  2005/01/07 00:22:33  dgrisby
// Big merge from omnipy2_develop.
//
// Revision 1.1.4.1  2003/03/23 21:51:57  dgrisby
// New omnipy3_develop branch.
//
// Revision 1.1.2.4  2001/09/20 14:51:25  dpg1
// Allow ORB reinitialisation after destroy(). Clean up use of omni namespace.
//
// Revision 1.1.2.3  2001/09/20 10:13:03  dpg1
// Avoid deadlock on exit due to new ORB core cleanup.
//
// Revision 1.1.2.2  2001/08/01 10:12:36  dpg1
// Main thread policy.
//
// Revision 1.1.2.1  2000/10/13 13:55:27  dpg1
// Initial support for omniORB 4.
//


#include <omnipy.h>
#include "pyThreadCache.h"

static int static_cleanup = 0;
// Set true when static data is being destroyed. Used to make sure
// Python things aren't used after they have gone away.

omni_mutex*                    omnipyThreadCache::guard      = 0;
const unsigned int             omnipyThreadCache::tableSize  = 67;
omnipyThreadCache::CacheNode** omnipyThreadCache::table      = 0;
unsigned int                   omnipyThreadCache::scanPeriod = 30;

class omnipyThreadScavenger : public omni_thread {
public:
  omnipyThreadScavenger() : dying_(0), cond_(omnipyThreadCache::guard) {
    start_undetached();
  }
  ~omnipyThreadScavenger() { }

  void kill() {
    {
      omni_mutex_lock l(*omnipyThreadCache::guard);
      dying_ = 1;
      cond_.signal();
    }
    join(0);
  }

  void* run_undetached(void*);
private:
  CORBA::Boolean dying_;
  omni_condition cond_;
  PyThreadState* threadState_;
  PyObject*      workerThread_;
};

class omnipyThreadData : public omni_thread::value_t {
public:
  omnipyThreadData(omnipyThreadCache::CacheNode* cn)
    : cn_(cn) {}

  ~omnipyThreadData() {
    omnipyThreadCache::threadExit(cn_);
  }

private:
  omnipyThreadCache::CacheNode* cn_;
};


static omnipyThreadScavenger* the_scavenger = 0;
static omni_thread::key_t     omnithread_key;


void
omnipyThreadCache::
init()
{
  omnithread_key = omni_thread::allocate_key();
  guard          = new omni_mutex();
  table          = new CacheNode*[tableSize];
  for (unsigned int i=0; i < tableSize; i++) table[i] = 0;

  the_scavenger  = new omnipyThreadScavenger();
}


void
omnipyThreadCache::
shutdown()
{
  if (the_scavenger) the_scavenger->kill();
  the_scavenger = 0;

  if (guard) delete guard;
  table = 0;
  guard = 0;
}


omnipyThreadCache::CacheNode*
omnipyThreadCache::
addNewNode(long id, unsigned int hash)
{
  CacheNode* cn = new CacheNode;
  cn->id = id;

  omni_thread* ot = omni_thread::self();

  if (ot) {
    if (omniORB::trace(20)) {
      omniORB::logger l;
      l << "Creating new Python state for thread id " << id << "\n";
    }

#if PY_VERSION_HEX >= 0x02030000
    cn->gilstate     = PyGILState_Ensure();
    cn->threadState  = PyThreadState_Get();
    cn->can_scavenge = 0;
#else
    PyEval_AcquireLock();
    cn->threadState  = PyThreadState_New(omniPy::pyInterpreter);
    cn->can_scavenge = 0;
    PyThreadState_Swap(cn->threadState);
#endif
    omni_thread::value_t* tv = ot->set_value(omnithread_key,
					     new omnipyThreadData(cn));
    OMNIORB_ASSERT(tv);
  }
  else {
    if (omniORB::trace(20)) {
      omniORB::logger l;
      l << "Creating new Python state for non-omni thread id " << id << "\n";
    }

    PyEval_AcquireLock();
    cn->threadState  = PyThreadState_New(omniPy::pyInterpreter);
    cn->reused_state = 0;
    cn->can_scavenge = 1;
    PyThreadState_Swap(cn->threadState);
  }    

  cn->used         = 1;
  cn->active       = 1;
  cn->workerThread = 0;

  // Insert into hash table
  {
    omni_mutex_lock _l(*guard);
    CacheNode* he = table[hash];
    cn->next = he;
    cn->back = &(table[hash]);
    if (he) he->back = &(cn->next);
    table[hash] = cn;
  }

  // Create omniORB worker thread threading state.
  //
  // Note that this happens after inserting the CacheNode into the
  // hash table, and outside any locks. This is because there is a
  // remote possibility that while executing the Python code to
  // create the worker thread object, the thread will end up calling
  // back through the thread cache (either because of a registered
  // thread hook, or because the cyclic garbage collector runs). In
  // that case, the re-entry will get a valid Python thread state,
  // albeit without a threading.Thread object.

  cn->workerThread = PyEval_CallObject(omniPy::pyWorkerThreadClass,
				       omniPy::pyEmptyTuple);
  if (!cn->workerThread) {
    if (omniORB::trace(1)) {
      {
	omniORB::logger l;
	l << "Exception trying to create worker thread.\n";
      }
      PyErr_Print();
    }
    else {
      PyErr_Clear();
    }
  }
  PyThreadState_Swap(0);
  PyEval_ReleaseLock();

  return cn;
}


void
omnipyThreadCache::
threadExit(CacheNode* cn)
{
  OMNIORB_ASSERT(!cn->active);

  if (static_cleanup) {
    // Too late to do anything now
    return;
  }

  if (omniORB::trace(20)) {
    omniORB::logger l;
    l << "Deleting Python state for thread id " << cn->id
      << " (thread exit)\n";
  }

  if (table) {
    // Remove the CacheNode from the table
    omni_mutex_lock _l(*guard);

    if (cn->back) {
      CacheNode* cnn = cn->next;
      *(cn->back) = cnn;
      if (cnn) cnn->back = cn->back;
    }
  }

  // Acquire Python thread lock and remove Python-world things
  PyEval_AcquireLock();
  PyThreadState_Swap(cn->threadState);
  if (cn->workerThread) {
    PyObject* argtuple = PyTuple_New(1);
    PyTuple_SET_ITEM(argtuple, 0, cn->workerThread);

    PyObject* tmp = PyEval_CallObject(omniPy::pyWorkerThreadDel,
				      argtuple);
    if (!tmp) {
      if (omniORB::trace(10)) {
	{
	  omniORB::logger l;
	  l << "Exception trying to delete worker thread.\n";
	}
	PyErr_Print();
      }
      else {
	PyErr_Clear();
      }
    }
    Py_XDECREF(tmp);
    Py_DECREF(argtuple);
  }

#if PY_VERSION_HEX >= 0x02030000
  PyGILState_Release(cn->gilstate);
#else
  PyThreadState_Swap(0);
  PyThreadState_Clear(cn->threadState);
  PyThreadState_Delete(cn->threadState);
  PyEval_ReleaseLock();
#endif

  delete cn;
}


void*
omnipyThreadScavenger::
run_undetached(void*)
{
  unsigned long abs_sec, abs_nsec;
  unsigned int  i;
  omnipyThreadCache::CacheNode *cn, *cnn, *to_remove;

  omniORB::logs(15, "Python thread state scavenger start.");

  // Create a thread state for the scavenger thread itself
  PyEval_AcquireLock();
  threadState_  = PyThreadState_New(omniPy::pyInterpreter);
  PyThreadState_Swap(threadState_);
  workerThread_ = PyEval_CallObject(omniPy::pyWorkerThreadClass,
				    omniPy::pyEmptyTuple);
  PyThreadState_Swap(0);
  PyEval_ReleaseLock();

  // Main loop
  while (!dying_) {

    to_remove = 0;

    {
      omni_mutex_lock _l(*omnipyThreadCache::guard);

      omni_thread::get_time(&abs_sec,&abs_nsec);
      abs_sec += omnipyThreadCache::scanPeriod;
      cond_.timedwait(abs_sec, abs_nsec);

      if (dying_) break;

      omniORB::logs(15, "Scanning Python thread states.");
    
      for (i=0; i < omnipyThreadCache::tableSize; i++) {
	cn = omnipyThreadCache::table[i];

	while (cn) {
	  cnn = cn->next;
	  if (cn->can_scavenge && !cn->active) {

	    if (cn->used) {
	      cn->used = 0;
	    }
	    else {
	      // Unlink from hash table
	      *(cn->back) = cnn;
	      if (cnn) cnn->back = cn->back;
	      
	      if (omniORB::trace(20)) {
		omniORB::logger l;
		l << "Will delete Python state for thread id "
		  << cn->id << " (scavenged)\n";
	      }
	      cn->next = to_remove;
	      to_remove = cn;
	    }
	  }
	  cn = cnn;
	}
      }
    }

    for (cn = to_remove; cn; cn=cnn) {
      cnn = cn->next;

      if (omniORB::trace(20)) {
	omniORB::logger l;
	l << "Delete Python state for thread id " 
	  << cn->id << " (scavenged)\n";
      }

      // Acquire Python thread lock and remove Python-world things
      PyEval_AcquireLock();
      PyThreadState_Swap(threadState_);
      if (cn->workerThread) {
	PyObject* argtuple = PyTuple_New(1);
	PyTuple_SET_ITEM(argtuple, 0, cn->workerThread);

	PyObject* tmp = PyEval_CallObject(omniPy::pyWorkerThreadDel,
					  argtuple);
	if (!tmp) {
	  if (omniORB::trace(1)) {
	    {
	      omniORB::logger l;
	      l << "Exception trying to delete worker thread.\n";
	    }
	    PyErr_Print();
	  }
	  else {
	    PyErr_Clear();
	  }
	}
	Py_XDECREF(tmp);
	Py_DECREF(argtuple);
      }
      PyThreadState_Clear(cn->threadState);
      PyThreadState_Delete(cn->threadState);
      PyThreadState_Swap(0);
      PyEval_ReleaseLock();

      delete cn;
    }
  }

  omnipyThreadCache::CacheNode** table;
  {
    omni_mutex_lock _l(*omnipyThreadCache::guard);
    table = omnipyThreadCache::table;
    omnipyThreadCache::table = 0;
  }

  // Delete all table entries
  PyEval_AcquireLock();
  PyThreadState_Swap(threadState_);

  for (i=0; i < omnipyThreadCache::tableSize; i++) {
    cn = table[i];

    while (cn) {
      if (cn->can_scavenge) {
	if (omniORB::trace(20)) {
	  omniORB::logger l;
	  l << "Deleting Python state for thread id "
	    << cn->id << " (shutdown)\n";
	}

	if (cn->workerThread) {
	  PyObject* argtuple = PyTuple_New(1);
	  PyTuple_SET_ITEM(argtuple, 0, cn->workerThread);

	  PyObject* tmp = PyEval_CallObject(omniPy::pyWorkerThreadDel,
					    argtuple);
	  Py_XDECREF(tmp);
	  Py_DECREF(argtuple);
	}
	PyThreadState_Clear(cn->threadState);
	PyThreadState_Delete(cn->threadState);

	// Remove the CacheNode
	cnn = cn->next;
	delete cn;
	cn = cnn;
      }
      else {
	if (omniORB::trace(20)) {
	  omniORB::logger l;
	  l << "Remove Python state for thread id "
	    << cn->id << " from cache (shutdown)\n";
	}
	cnn = cn->next;
	cn->next = 0;
	cn->back = 0;
	cn = cnn;
      }
    }
  }

  delete [] table;

  // Remove this thread's Python state
  if (workerThread_) {
    PyObject* argtuple = PyTuple_New(1);
    PyTuple_SET_ITEM(argtuple, 0, workerThread_);

    PyObject* tmp = PyEval_CallObject(omniPy::pyWorkerThreadDel, argtuple);
    Py_XDECREF(tmp);
    Py_DECREF(argtuple);
  }
  PyThreadState_Swap(0);
  PyThreadState_Clear(threadState_);
  PyThreadState_Delete(threadState_);
  PyEval_ReleaseLock();

  omniORB::logs(15, "Python thread state scavenger exit.");

  return 0;
}


class _omnipy_cleapup_detector {
public:
  inline ~_omnipy_cleapup_detector() { static_cleanup = 1; }
};

static _omnipy_cleapup_detector _the_omnipy_cleapup_detector;
