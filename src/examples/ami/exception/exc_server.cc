//
// Example code for implementing IDL interfaces in file exc.idl
//

#include <iostream.h>
#include <exc.hh>

//
// Example class implementing IDL interface I
//
class I_i: public POA_I,
                public PortableServer::RefCountServantBase {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  virtual ~I_i() {}
public:
  // standard constructor
  I_i() {}

  // methods corresponding to defined IDL attributes and operations
  void op(const char* arg);
  
};

//
// Example implementational code for IDL interface I
//
void I_i::op(const char* arg){
  StringException e(CORBA::string_dup(arg));
  throw e;
}

// End of example implementational code

int main(int argc, char** argv)
{
  try {
    // Initialise the ORB.
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "omniORB3");

    // Obtain a reference to the root POA.
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

    // We allocate the objects on the heap.  Since these are reference
    // counted objects, they will be deleted by the POA when they are no
    // longer needed.
    I_i* myI_i = new I_i();
    
    // Activate the objects.  This tells the POA that the objects are
    // ready to accept requests.
    PortableServer::ObjectId_var myI_iid = poa->activate_object(myI_i);
    
    // Obtain a reference to each object and output the stringified
    // IOR to stdout
    {
      // IDL interface: I
      CORBA::Object_var ref = myI_i->_this();
      CORBA::String_var sior(orb->object_to_string(ref));
      cout << "IDL object I IOR = '" << (char*)sior << "'" << endl;
    }
    
    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    orb->run();
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

