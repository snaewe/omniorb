//				Package : omnithread
// omnithread/nt.h		Created : 6/95 tjr
//
// Copyright (C) Olivetti Research Ltd 1995
//
// OMNI thread implementation classes for NT threads.
//

#ifndef __omnithread_nt_h_
#define __omnithread_nt_h_

#include <windows.h>

#define OMNI_MUTEX_IMPLEMENTATION			\
    CRITICAL_SECTION crit;

#define OMNI_CONDITION_IMPLEMENTATION			\
    CRITICAL_SECTION crit;				\
    omni_thread* waiting_head;				\
    omni_thread* waiting_tail;

#define OMNI_SEMAPHORE_IMPLEMENTATION			\
    HANDLE nt_sem;

#define OMNI_THREAD_IMPLEMENTATION			\
    HANDLE handle;					\
    DWORD nt_id;					\
    void* return_val;					\
    HANDLE cond_semaphore;				\
    omni_thread* cond_next;				\
    omni_thread* cond_prev;				\
    BOOL cond_waiting;					\
    static DWORD wrapper(LPVOID);			\
    static int nt_priority(priority_t);			\
    friend class omni_condition;

#define ETIMEDOUT 0x20000000

#endif
