// greeting.cc - This source code demonstrates the use of an object
//               reference by a client to perform an operation on an 
//               object. It is part of the three examples used
//               in Chapter 2 "The Basics" of the omniORB2 user guide.
//
#include <iostream.h>
#include "echo.hh"

void
hello(CORBA::Object_ptr obj)
{
  Echo_var e = Echo::_narrow(obj);

  if (CORBA::is_nil(e)) {
    cerr << "hello: cannot invoke on a nil object reference.\n" << endl;
    return;
  }

  CORBA::String_var src = (const char*) "Hello!"; // String literals are not
                                                  // const char*. Must do
                                                  // explicit casting to 
                                                  // force the use of the copy
                                                  // operator=(). 
  CORBA::String_var dest;

  dest = e->echoString(src);

  cerr << "I said,\"" << (char*)src << "\"."
       << " The Object said,\"" << (char*)dest <<"\"" << endl;
}
