//				Package : threadtests
// prio.cc			Created : 8/95 tjr
//
// Copyright (C) AT&T Laboratories Cambridge 1995
//
// This program is designed to test out thread priorities.  It creates six
// threads, two at each of high, normal and low priority (the main thread
// becomes the second low-priority thread).  Each thread does a compute-bound
// loop, tries yielding, then does a second compute-bound loop, followed by a 2
// second sleep before repeating the whole thing.  The program takes a single
// argument, the number of iterations of the compute-bound loop.  When the
// number of loop iterations is small all the threads get to run and so the
// different priorities have little effect.  When the number of loop iterations
// gets large (i.e. when the loop takes more than about a second) the high
// priority threads should run nearly all the time and the lower priority
// threads get little if any cpu time.
//
// Note that this program can take a lot of CPU so don't leave it
// running for a long time if anyone else is using the same machine.
//

#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif
#include <omnithread.h>

#if defined(__arm__) && defined(__atmos__)
#define flush ""
#endif

static void func(void*);

static int loop_iterations;

static omni_mutex print_mutex;
#define PRINTMSG(x) print_mutex.lock(); x << flush; print_mutex.unlock()

int main(int argc, char** argv)
{
    unsigned long s1, s2, n1, n2, sum = 0;
    char buf[20];

    if ((argc != 2) || (argv[1][0] == '-')) {
	cerr << "usage: " << argv[0] << " loop_iterations\n";
	exit(1);
    }

    loop_iterations = atoi(argv[1]);

    omni_thread::get_time(&s1,&n1);

    for (int i = 0; i < loop_iterations; i++) {
        sum += (1 - 2 * (i & 1)) * i * i;
    }
    omni_thread::get_time(&s2,&n2);

    if (n2 >= n1) {
	n2 -= n1;
	s2 -= s1;
    } else {
	n2 = n2 + 1000000000 - n1;
	s2 = s2 - 1 - s1;
    }

    sprintf(buf,"%d.%03lu", (unsigned long) s2, (unsigned long)n2/1000000ul);

    cout << argv[0] << ": doing " << loop_iterations
	 << " loop iterations (approx " << buf << " seconds per loop)\n";

    PRINTMSG(cout << "main: creating h1\n");
    omni_thread::create(func,(void*)"h1",omni_thread::PRIORITY_HIGH);

    PRINTMSG(cout << "main: creating m1\n");
    omni_thread::create(func,(void*)"m1",omni_thread::PRIORITY_NORMAL);

    PRINTMSG(cout << "main: creating l1\n");
    omni_thread::create(func,(void*)"l1",omni_thread::PRIORITY_LOW);

    PRINTMSG(cout << "main: creating h2\n");
    omni_thread::create(func,(void*)"h2",omni_thread::PRIORITY_HIGH);

    PRINTMSG(cout << "main: creating m2\n");
    omni_thread::create(func,(void*)"m2",omni_thread::PRIORITY_NORMAL);

    PRINTMSG(cout << "main: creating l2\n");
    omni_thread::self()->set_priority(omni_thread::PRIORITY_LOW);
    func((void*)"l2");

    return 0;
}

static void func(void* arg)
{
    char *name = (char*)arg;

    while (1) {
	PRINTMSG(cout << name << ": entering 1st compute-bound loop\n");

        int i; long sum = 0;
	for (i = 0; i < loop_iterations; i++) {
	    sum += (1 - 2 * (i & 1)) * i * i;
	}

	PRINTMSG(cout << name << ": left compute-bound loop; yielding\n");

	omni_thread::yield();

	PRINTMSG(cout << name << ": entering 2nd compute-bound loop\n");

	sum = 0;
	for (i = 0; i < loop_iterations; i++) {
	    sum += (1 - 2 * (i & 1)) * i * i;
	}

	PRINTMSG(cout << name
		 << ": left compute-bound loop; sleeping for 2 seconds\n");

	omni_thread::sleep(2);
    }
}
