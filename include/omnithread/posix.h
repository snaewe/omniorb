//				Package : omnithread
// omnithread/posix.h		Created : 7/94 tjr
//
// Copyright (C) Olivetti Research Ltd 1994, 1995
//
// OMNI thread implementation classes for posix threads
//

#ifndef __omnithread_posix_h_
#define __omnithread_posix_h_

#include <pthread.h>

#define OMNI_MUTEX_IMPLEMENTATION			\
    pthread_mutex_t posix_mutex;

#define OMNI_CONDITION_IMPLEMENTATION			\
    pthread_cond_t posix_cond;

#define OMNI_SEMAPHORE_IMPLEMENTATION			\
    omni_mutex m;					\
    omni_condition c;					\
    int value;


//
// The inclusion of cancel() here is a dirty hack to keep omniORB happy.
// It should disappear in the near future.
//

#define OMNI_THREAD_IMPLEMENTATION			\
    pthread_t posix_thread;				\
    static void* wrapper(void*);			\
    static int posix_priority(priority_t);		\
public:							\
    int cancel(void);

#endif
