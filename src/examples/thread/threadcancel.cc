//  threadcancel.cc             Package   : threadtests
//                              Created on: 7/94 tjr
//
// This program tests the thread cancel feature of the omnithread library.
// It works like this:
//    The main thread creates 3 threads. It immediately cancel thread #2.
//    It then sleeps for a while and cancel all other threads before quitting.
//
// The expected output is:
//     $ ./threadcancel
//     thread 1 about to sleep
//     thread 3 about to sleep
//     thread 1 awoken
//     main returning
//
//

/*
 $Log$
 Revision 1.1  1995/01/25 11:54:53  sll
 Initial revision

 */
 
#include <iostream.h>
#include <stdlib.h>
#include "omnithread.h"

static void* thread_fn(void*);

int main(int argc, char** argv)
{
    omni_thread::set_max_priority(2);
    omni_thread::set_default_priority(1);
    omni_thread::self()->set_priority(1);

    omni_thread* t1 = omni_thread::create(thread_fn,0,(void*)1,2);
    omni_thread* t2 = omni_thread::create(thread_fn,0,(void*)3,0);
    omni_thread* t3 = omni_thread::create(thread_fn,0,(void*)4,2);

    t2->cancel();

    omni_thread::sleep(2,0);

    omni_thread::cancel_other_threads();

    cerr << "main returning\n";

    return 0;
}

static void* thread_fn(void* arg)
{
    cerr << "thread " << omni_thread::self()->id() << " about to sleep" << endl;

    omni_thread::sleep((int)arg,0);

    cerr << "thread " << omni_thread::self()->id() << " awoken\n";

    return (void*)3;
}
