// lcremove.cc -- LifeCycle example remove client
//
// See the LifeCycle tutorial for explanation.

#include <stdlib.h>
#include <iostream.h>
#include "echolag.hh"

int
main(int argc, char **argv) {
  CORBA::ORB_ptr orb = CORBA::ORB_init(argc, argv, "omniORB2");

  if (argc < 2) {
    cout << "usage: " << argv[0] << " <EchoLag IOR>" << endl;
    exit(1);
  }

  EchoLag_var elag;

  try {
    CORBA::Object_var obj = orb->string_to_object(argv[1]);

    elag = EchoLag::_narrow(obj);

    elag->remove();
    cout << endl << "EchoLag removed." << endl;
  }
  catch(CORBA::COMM_FAILURE &ex) {
    cerr << "Caught COMM_FAILURE" << endl;
  }
  catch(CORBA::OBJECT_NOT_EXIST &ex) {
    cerr << "Caught OBJECT_NOT_EXIST" << endl;
  }
  catch(CORBA::TRANSIENT &ex) {
    cerr << "Caught TRANSIENT" << endl;
  }

  orb->NP_destroy();

  return 0;
}
