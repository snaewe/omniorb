// lcclient.cc -- LifeCycle example client
//
// See the LifeCycle tutorial for explanation.

#include <stdlib.h>

#include "echolag.hh"
#include <iostream.h>

int
main(int argc, char **argv) {
  CORBA::ORB_ptr orb = CORBA::ORB_init(argc, argv, "omniORB2");
  CORBA::BOA_ptr boa = orb->BOA_init(argc, argv, "omniORB2_BOA");

  if (argc < 3) {
    cout << "usage: " << argv[0]
	 << " <string> <EchoLag IOR> [EchoLagFactory IOR]" << endl;
    exit(1);
  }

  EchoLag_var elag;

  try {
    CORBA::Object_var obj = orb->string_to_object(argv[2]);

    elag = EchoLag::_narrow(obj);

    if (argc >= 4) {
      // Tell the EchoLag to move to a new location:
      obj = orb->string_to_object(argv[3]);
      EchoLagFactory_var factory = EchoLagFactory::_narrow(obj);

      elag->move(factory);
      cout << endl << "EchoLag moved." << endl;
    }

    cout << endl << "EchoLag::echoStringLag() returned: "
	 << elag->echoStringLag(argv[1]) << endl;
  }
  catch(CORBA::COMM_FAILURE &ex) {
    cerr << "Caught COMM_FAILURE" << endl;
  }
  catch(CORBA::OBJECT_NOT_EXIST &ex) {
    cerr << "Caught OBJECT_NOT_EXIST" << endl;
  }
  catch(CORBA::TRANSIENT &ex) {
    cout << "Caught TRANSIENT!  Where's the TRANSIENT handler gone?" << endl;
    cout << " Retrying..." << endl;
    try {
      cout << endl << "EchoLag::echoStringLag() returned: "
	   << elag->echoStringLag(argv[1]) << endl;
    }
    catch(...) {
      cerr << "Caught another exception." << endl;
    }
  }

  boa->destroy();

  orb->NP_destroy();

  return 0;
}
