// eg2_impl.cc - This example is similar to that found in:
//                  <top>/src/examples/echo/eg2_impl.cc
//               except that the object reference is the same each time.
//
// Usage: eg2_impl -ORBpoa_iiop_port <portnum>
//
//        On startup, the object reference is printed to stderr as a
//        stringified IOR. This string should be used as the argument to 
//        eg2_clt.
//
//        If the same port number is given at each invocation, then the same
//        object reference is issued each time.  It is also possible to
//        specify the port number from an application, by inserting the
//        relevant option into argc,argv before passing to CORBA::ORB_init().
//        See <top>/src/appl/omniNames/omniNames.cc for an example.
//

#include <iostream.h>
#include <stdlib.h>
#include <echo.hh>


class Echo_i : public POA_Echo,
	       public PortableServer::RefCountServantBase
{
public:
  inline Echo_i() {}
  virtual ~Echo_i() {}
  virtual char* echoString(const char* mesg);
};


char* Echo_i::echoString(const char* mesg)
{
  return CORBA::string_dup(mesg);
}

//////////////////////////////////////////////////////////////////////

void usage()
{
  cerr << "usage:  eg2_impl -ORBpoa_iiop_port <portnum>" << endl;
  exit(-1);
}


int main(int argc, char** argv)
{
  try {
    
    if( argc < 3 )  usage();

    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    if( argc != 1 )  usage();

    {
      CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj);
      PortableServer::POAManager_var pman = root_poa->the_POAManager();
      pman->activate();

      // Create a new POA with the persistent lifespan policy.
      CORBA::PolicyList pl;
      pl.length(2);
      pl[0] = root_poa->create_lifespan_policy(PortableServer::PERSISTENT);
      pl[1] = root_poa->create_id_assignment_policy(PortableServer::USER_ID);

      PortableServer::POA_var poa = root_poa->create_POA("my poa", pman, pl);

      // Always use the same object id.
      PortableServer::ObjectId_var oid =
	PortableServer::string_to_ObjectId("my echo object");

      // Activate the echo object...
      Echo_i* myecho = new Echo_i();
      poa->activate_object_with_id(oid, myecho);

      obj = myecho->_this();
      CORBA::String_var sior(orb->object_to_string(obj));
      cerr << "'" << (char*)sior << "'" << endl;

      myecho->_remove_ref();

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
