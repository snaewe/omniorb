//				Package : omnithread
// omnithread/os2_impl.h	Created : 6/95 tjr
//                              Ported  : 6/01 DLS
//
//    Copyright (C) 1995, 1996, 1997 Olivetti & Oracle Research Laboratory
//
//    This file is part of the omnithread library
//
//    The omnithread library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//    02111-1307, USA
//
//

// Implementation of OMNI thread abstraction for OS/2 threads - compile
// using gcc/pgcc/emx.  This was shamelessly stolen and translated from
// the NT (and a little of the posix) implementation.

#ifndef __omnithread_os2_impl_h_
#define __omnithread_os2_impl_h_

#include <os2.h>

#define OMNI_THREAD_WRAPPER \
    void APIENTRY omni_thread_wrapper(void *ptr);

extern "C" OMNI_THREAD_WRAPPER;

#define OMNI_MUTEX_IMPLEMENTATION			\
    HMTX crit;

#define OMNI_CONDITION_IMPLEMENTATION			\
    HMTX crit;				\
    omni_thread* waiting_head;				\
    omni_thread* waiting_tail;

#define OMNI_SEMAPHORE_IMPLEMENTATION			\
    omni_mutex m;					\
    omni_condition c;					\
    int value;

#define OMNI_THREAD_IMPLEMENTATION			\
    TID nt_id;						\
    void* return_val;					\
    HEV cond_semaphore;					\
    omni_thread* cond_next;				\
    omni_thread* cond_prev;				\
    BOOL cond_waiting;					\
    static int nt_priority(priority_t);			\
    friend class omni_condition;			\
    friend OMNI_THREAD_WRAPPER

#endif
