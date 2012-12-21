// ziop_clt.cc
//
// This is a variant of the eg2_clt from src/examples/echo that
// enables ZIOP, and sends large enough messages to trigger
// compression.
//
// Usage:
//   ziop_clt -ORBclientTransportRule "* unix,ssl,tcp,ziop" <object reference>
//

#include <echo.hh>
#include <omniORB4/omniZIOP.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

static void hello(Echo_ptr e)
{
  stringstream buf;
  ifstream stream(__FILE__);

  buf << stream.rdbuf();

  string input = buf.str();
  CORBA::String_var result = e->echoString(input.c_str());

  cout << "I sent " << input.size() << " characters, and received "
       << strlen((const char*)result) << "." << endl;
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    CORBA::PolicyList pl;
    omniZIOP::setGlobalPolicies(pl);

    if (argc != 2) {
      cerr << "usage: ziop_clt -ORBclientTransportRule \"* unix,ssl,tcp,ziop\""
           << " <ior>" << endl;
      return 1;
    }

    CORBA::Object_var obj = orb->string_to_object(argv[1]);

    Echo_var echoref = Echo::_narrow(obj);
    if( CORBA::is_nil(echoref) ) {
      cerr << "Can't narrow reference to type Echo (or it was nil)." << endl;
      return 1;
    }

    hello(echoref);

    orb->destroy();
  }
  catch(CORBA::SystemException& ex) {
    cerr << "Caught a CORBA::" << ex._name() << endl;
  }
  catch(CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }
  return 0;
}
