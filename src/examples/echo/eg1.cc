// eg1.cc - This is the source code of example 1 used in Chapter 2 
//          "The Basics" of the omniORB2 user guide.
//
//          In this example, both the object implementation and the 
//          client are in the same process.
//
// Usage: eg1
//
#include <iostream.h>
#include "echo.hh"

#include "echo_i.cc"
#include "greeting.cc"

int
main(int argc, char **argv)
{
  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB2");
  CORBA::BOA_ptr boa = orb->BOA_init(argc,argv,"omniORB2_BOA");

  Echo_i *myobj = new Echo_i();
  // Note: all implementation objects must be instantiated on the
  // heap using the new operator.

  myobj->_obj_is_ready(boa);
  // Tell the BOA the object is ready to serve.
  // This call is omniORB2 specific.
  //
  // This call is equivalent to the following call sequence:
  //     Echo_ptr myobjRef = myobj->_this();
  //     boa->obj_is_ready(myobjRef);
  //     CORBA::release(myobjRef);

  boa->impl_is_ready(0,1);
  // Tell the BOA we are ready and to return immediately once it has
  // done its stuff. It is omniORB2 specific to call impl_is_ready()
  // with the extra 2nd argument- CORBA::Boolean NonBlocking,
  // which is set to TRUE (1) in this case.

  Echo_ptr myobjRef = myobj->_this();
  // Obtain an object reference.
  // Note: always use _this() to obtain an object reference from the
  //       object implementation.

  hello(myobjRef);

  CORBA::release(myobjRef);
  // Dispose of the object reference.

  myobj->_dispose();
  // Dispose of the object implementation.
  // This call is omniORB2 specific.
  // Note: *never* call the delete operator or the dtor of the object
  //       directly because the BOA needs to be informed.
  //
  // This call is equivalent to the following call sequence:
  //     Echo_ptr myobjRef = myobj->_this();
  //     boa->dispose(myobjRef);
  //     CORBA::release(myobjRef);

  return 0;
}
