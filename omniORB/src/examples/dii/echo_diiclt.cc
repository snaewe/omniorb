// echo_diiclt.cc
//
//              This example demonstrates the Dynamic Invocation Interface.
//              It is a client for the eg2_impl server in the
//              src/examples/echo directory. Alternatively you can use the
//              DSI echo server in src/examples/dsi.
//
// Usage: eg2_clt <object reference>
//

#include <omniORB4/CORBA.h>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif


static void hello(CORBA::Object_ptr obj)
{
  CORBA::String_var arg = (const char*) "Hello!";
  CORBA::Request_var req = obj->_request("echoString");
  req->add_in_arg() <<= arg;
  req->set_return_type(CORBA::_tc_string);

  req->invoke();

  if( req->env()->exception() ) {
    cout << "echo_diiclt: An exception was thrown!" << endl;
    return;
  }

  const char* ret;
  req->return_value() >>= ret;

  cout << "I said, \"" << (char*)arg << "\"." << endl
       << "The Echo object replied, \"" << ret <<"\"." << endl;
}


static void hello_deferred(CORBA::Object_ptr obj)
{
  CORBA::String_var arg = (const char*) "Hello!";
  CORBA::Request_var req = obj->_request("echoString");
  req->add_in_arg() <<= arg;
  req->set_return_type(CORBA::_tc_string);

  req->send_deferred();
  cout << "Sending deferred request: ";
  while( !req->poll_response() )
    cout << '#';
  cout << endl << "Response received." << endl;

  if( req->env()->exception() ) {
    cout << "echo_diiclt: An exception was thrown!" << endl;
    return;
  }

  const char* ret;
  req->return_value() >>= ret;

  cout << "I said, \"" << (char*)arg << "\"." << endl
       << "The Echo object replied, \"" << ret <<"\"." << endl;
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "omniORB4");

    if( argc != 2 ) {
      cerr << "usage:  eg2_clt <object reference>" << endl;
      return 1;
    }

    {
      CORBA::Object_var obj = orb->string_to_object(argv[1]);
      hello(obj);
      hello_deferred(obj);
    }

    orb->destroy();
  }
  catch(CORBA::TRANSIENT&) {
    cerr << "Caught system exception TRANSIENT -- unable to contact the "
         << "server." << endl;
  }
  catch(CORBA::SystemException& ex) {
    cerr << "Caught a CORBA::" << ex._name() << endl;
  }
  catch(CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }
  catch(omniORB::fatalException& fe) {
    cerr << "Caught omniORB::fatalException:" << endl;
    cerr << "  file: " << fe.file() << endl;
    cerr << "  line: " << fe.line() << endl;
    cerr << "  mesg: " << fe.errmsg() << endl;
  }
  return 0;
}
