//				Package : omnithread
// omnithread/solaris.h		Created : 7/94 tjr
//
// Copyright (C) Olivetti Research Ltd 1994, 1995
//
// OMNI thread implementation classes for solaris threads.
//

#ifndef __omnithread_solaris_h_
#define __omnithread_solaris_h_

#include <thread.h>

#define OMNI_MUTEX_IMPLEMENTATION			\
    mutex_t sol_mutex;

#define OMNI_CONDITION_IMPLEMENTATION			\
    cond_t sol_cond;

#define OMNI_SEMAPHORE_IMPLEMENTATION			\
    sema_t sol_sem;

#define OMNI_THREAD_IMPLEMENTATION			\
    thread_t sol_thread;				\
    static void* wrapper(void*);			\
    static int sol_priority(priority_t);

#endif
