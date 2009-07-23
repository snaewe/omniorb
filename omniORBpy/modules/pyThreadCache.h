// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyThreadCache.h            Created on: 2000/05/26
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
// Revision 1.1.2.1  2000/10/13 13:55:27  dpg1
// Initial support for omniORB 4.
//

#if defined(__VMS)
#include <pythread.h>
#else
#include PYTHON_THREAD_INC
#endif

// Python 2.3 introduced functions to get and released the Python
// interpreter lock, and create thread state as necessary.
// Unfortunately, they are too inefficient since we would end up
// creating and destroying thread states (and Python threading.Thread
// objects) on every call. Even more unfortunately, we can't ignore
// the new functions and use our own scheme, because there are (debug)
// assertions in Python to check that the thread state is what is
// expected. So, we have to jump through all sorts of hoops to play
// nice, and it's still slower than the equivalent code in Python <=
// 2.2...


class omnipyThreadCache {
public:

  static omni_mutex* guard;

  static void init();
  static void shutdown();

  struct CacheNode {
    long             id;
    PyThreadState*   threadState;
    PyObject*        workerThread;

    CORBA::Boolean   used;
    CORBA::Boolean   can_scavenge;
    CORBA::Boolean   reused_state;
    int              active;

#if PY_VERSION_HEX >= 0x02030000
    PyGILState_STATE gilstate;
#endif

    CacheNode*       next;
    CacheNode**      back;
  };

  // Fixed-size open hash table of cacheNodes
  static const unsigned int tableSize;
  static CacheNode**        table;

  // Time in seconds between runs of the node scavenger
  static unsigned int       scanPeriod;

  // Class lock acquires the Python interpreter lock when it is
  // created, and releases it again when it is deleted.
  class lock {
  public:
    inline lock() {
#if PY_VERSION_HEX >= 0x02030000
      PyThreadState* tstate = PyGILState_GetThisThreadState();
      if (tstate) {
	cacheNode_ = 0;
	PyEval_AcquireLock();
	PyThreadState_Swap(tstate);
      }
      else
#endif
      {
	long id    = PyThread_get_thread_ident();
	cacheNode_ = acquireNode(id);
	PyEval_AcquireLock();
	PyThreadState_Swap(cacheNode_->threadState);
      }
    }

    inline ~lock() {
      PyThreadState_Swap(0);
      PyEval_ReleaseLock();
      if (cacheNode_)
	releaseNode(cacheNode_);
    }
  private:
    CacheNode* cacheNode_;
  };


  static inline CacheNode* acquireNode(long id) {
    unsigned int hash = id % tableSize; 
    CacheNode* cn;
    {
      omni_mutex_lock _l(*guard);
      OMNIORB_ASSERT(table);

      cn = table[hash];
      while (cn && cn->id != id) cn = cn->next;
      if (cn) {
	cn->used = 1;
	cn->active++;
	return cn;
      }
    }
    return addNewNode(id, hash);
  }

  static inline void releaseNode(CacheNode* cn) {
    omni_mutex_lock _l(*guard);
    cn->used = 1;
    cn->active--;
  }

  static CacheNode* addNewNode(long id, unsigned int hash);

  static void threadExit(CacheNode* cn);
};
