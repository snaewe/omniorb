//				Package : threadtests
// thrspecdata.cc		Created : 6/95 tjr
//
// Copyright (C) AT&T Laboratories Cambridge 1995
//
// Demonstrates the use of thread-specific data by having a class which
// inherits from omni_thread.  Also demonstrates joining with a thread
// which returns a value.
//

#include <iostream.h>
#include <omnithread.h>

class thread_with_data : public omni_thread {
    int my_thread_id_plus_two;

    void* run_undetached(void* ptr) {
	int arg = *(int*)ptr;
	delete (int*)ptr;
	cerr << "thread: run invoked with arg " << arg << endl;
	cerr << "thread: my id is " << id() << endl;
	cerr << "thread: my private data (id plus 2) is "
	     << my_thread_id_plus_two << endl;
	int* rv = new int(my_thread_id_plus_two + 1);
	cerr << "thread: returning " << *rv << endl;
	return (void*)rv;
    }

    // the destructor of a class that inherits from omni_thread should never be
    // public (otherwise the thread object can be destroyed while the
    // underlying thread is still running).

    ~thread_with_data(void) {}

    static void* make_arg(int i) { return (void*) new int(i); }

public:

    thread_with_data(void) : omni_thread(make_arg(5)) {
	my_thread_id_plus_two = id() + 2;
	start_undetached();
    }
};


int main(int argc, char** argv)
{
    thread_with_data* t = new thread_with_data;

    cerr << "main: joining\n";

    int* rv;
    t->join((void**)&rv);

    cerr << "main: joined - got return value " << *rv << endl;

    delete rv;

    return 0;
}
