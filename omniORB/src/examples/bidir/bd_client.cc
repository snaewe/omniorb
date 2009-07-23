#include <echo_callback.hh>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif

#include <stdlib.h>


// Implementation of cb::CallBack.

class cb_i : public virtual POA_cb::CallBack
{
public:
  inline cb_i() {}
  virtual ~cb_i() {}

  virtual void call_back(const char* mesg) {
    cout << "cb_client: call_back(\"" << mesg << "\")" << endl;
  }
};

//////////////////////////////////////////////////////////////////////

static void do_single(cb::Server_ptr server, cb::CallBack_ptr cb)
{
  if( CORBA::is_nil(server) ) {
    cerr << "cb_client: The server reference is nil!" << endl;
    return;
  }

  cout << "cb_client: server->one_time(call_back, \"Hello!\")" << endl;
  server->one_time(cb, "Hello!");
  cout << "cb_client: Returned." << endl;
}


static void do_register(cb::Server_ptr server, cb::CallBack_ptr cb,
			int period, int time_to_shutdown)
{
  if( CORBA::is_nil(server) ) {
    cerr << "cb_client: The server reference is nil!" << endl;
    return;
  }

  cout << "cb_client: server->register(call_back, \"Hello!\", "
       << period << ")" << endl;
  server->_cxx_register(cb, "Hello!", period);
  cout << "cb_client: Returned." << endl;

  omni_thread::sleep(time_to_shutdown);
  cout << "cb_client: Finished." << endl;
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {

    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    if( argc != 2 && argc != 4 ) {
      cerr << "usage:  bd_client <object reference> [<call-back period>"
	" <time to shutdown>]" << endl;
      return 1;
    }


    {
      CORBA::Object_var obj;

      // Initialise the POA.
      obj = orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var rootpoa = PortableServer::POA::_narrow(obj);
      PortableServer::POAManager_var pman = rootpoa->the_POAManager();
      pman->activate();

      // Create a POA with the Bidirectional policy
      CORBA::PolicyList pl;
      pl.length(1);
      CORBA::Any a;
      a <<= BiDirPolicy::BOTH;
      pl[0] = orb->create_policy(BiDirPolicy::BIDIRECTIONAL_POLICY_TYPE, a);

      PortableServer::POA_var poa = rootpoa->create_POA("bidir", pman, pl);

      // Get the reference the server.
      obj = orb->string_to_object(argv[1]);
      cb::Server_var server = cb::Server::_narrow(obj);

      // Register a CallBack object in this process.
      cb_i* mycallback = new cb_i();
      PortableServer::ObjectId_var oid = poa->activate_object(mycallback);
      cb::CallBack_var callback = mycallback->_this();
      mycallback->_remove_ref();

      if( argc == 2 )  do_single(server, callback);
      else             do_register(server, callback, atoi(argv[2]),
				   atoi(argv[3]));

    }
    // Clean-up.  This also destroys the call-back object.
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
