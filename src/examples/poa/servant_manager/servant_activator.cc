//
// servant_activator.cc
//
// Usage: servant_activator
//
//        On startup, the object reference is printed to cout as a
//        stringified IOR. This string should be used as the argument to 
//        eg2_clt (from the <omniORB>/src/examples/echo/ directory).
//
//        A message of 'shutdown' causes the server (you guessed it) to
//        shutdown.
//

#include <iostream.h>
#include <echo.hh>


static CORBA::ORB_var orb;

//////////////////////////////////////////////////////////////////////

class Echo_i : public POA_Echo,
	       public PortableServer::RefCountServantBase
{
public:
  virtual ~Echo_i() { cout << "Echo_i::~Echo_i()" << endl; }

  char* echoString(const char* mesg)
  {
    cout << "Echo_i::echoString(\"" << mesg << "\")" << endl;
    if( !strcmp(mesg, "shutdown") )  orb->shutdown(0);
    return CORBA::string_dup(mesg);
  }
};

//////////////////////////////////////////////////////////////////////

class MyActivator_i : public POA_PortableServer::ServantActivator,
		      public PortableServer::RefCountServantBase
{
public:
  virtual ~MyActivator_i()
  { cout << "MyActivator_i::~MyActivator_i()" << endl; }

  PortableServer::Servant
  incarnate(const PortableServer::ObjectId& oid,
	    PortableServer::POA_ptr adapter)
  {
    cout << "MyActivator_i::incarnate()" << endl;
    return new Echo_i;
  }

  void
  etherealize(const PortableServer::ObjectId& oid,
	      PortableServer::POA_ptr adapter,
	      PortableServer::Servant the_servant,
	      CORBA::Boolean cleanup_in_progress,
	      CORBA::Boolean remaining_activations)
  {
    cout << "MyActivator_i::etherealize()" << endl;
    the_servant->_remove_ref();
  }
};

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    orb = CORBA::ORB_init(argc, argv, "omniORB3");

    {
      // Get hold of the root poa, and activate it.
      CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj);
      PortableServer::POAManager_var poa_man = root_poa->the_POAManager();
      poa_man->activate();

      // Create a new poa to hold our echo objects.
      CORBA::PolicyList pl;
      pl.length(1);
      pl[0] = root_poa->create_request_processing_policy(
							 PortableServer::USE_SERVANT_MANAGER);

      PortableServer::POA_var poa = root_poa->create_POA("mypoa", poa_man, pl);

      // Activate a servant activator in the root poa.
      MyActivator_i* sa = new MyActivator_i;
      PortableServer::ObjectId_var id = root_poa->activate_object(sa);
      PortableServer::ServantActivator_var saref = sa->_this();
      sa->_remove_ref();

      // Register the servant activator with our new poa.
      poa->set_servant_manager(saref);

      // Print out a reference to an object.
      CORBA::Object_var ref = poa->create_reference("IDL:Echo:1.0");
      CORBA::String_var sior(orb->object_to_string(ref));
      cout << "'" << (char*) sior << "'" << endl;

      orb->run();
    }
    orb->destroy();
  }
  catch(CORBA::SystemException&) {
    cerr << "Caught CORBA::SystemException." << endl;
  }
  catch(CORBA::Exception&) {
    cerr << "Caught CORBA::Exception." << endl;
  }
  catch(omniORB::fatalException& fe) {
    cerr << "Caught omniORB::fatalException:" << endl;
    cerr << "  file: " << fe.file() << endl;
    cerr << "  line: " << fe.line() << endl;
    cerr << "  mesg: " << fe.errmsg() << endl;
  }
  catch(...) {
    cerr << "Caught unknown exception." << endl;
  }

  return 0;
}
