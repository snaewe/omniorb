// eg3_tieimpl.cc - This example is similar to eg3_impl.cc except that
//                  the tie implementation skeleton is used.
//
//               This is the object implementation.
//
// Usage: eg3_tieimpl
//
//        On startup, the object reference is registered with the 
//        COS naming service. The client uses the naming service to
//        locate this object.
//
//        The name which the object is bound to is as follows:
//              root  [context]
//               |
//              test  [context] kind [my_context]
//               |
//              Echo  [object]  kind [Object]
//

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif
#include <echo.hh>


static CORBA::Boolean bindObjectToName(CORBA::ORB_ptr,CORBA::Object_ptr);


// This is the object implementation.  Notice that it does not
// inherit from any stub class.

class Echo_i {
public:
  inline Echo_i() {}
  inline ~Echo_i() {}
  virtual char* echoString(const char* mesg);
};


char* Echo_i::echoString(const char* mesg)
{
  return CORBA::string_dup(mesg);
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

    // Note that the <myecho> object is constructed on the stack here.
    // This is because tie implementations do not inherit from the
    // PortableServer::RefCountServantBase mixin class -- and so are
    // not automatically deleted by the POA.
    // However, it will delete its implementation (myimpl) when it
    // it itself destroyed (when it goes out of scope).  It is
    // essential however to ensure that such objects are not deleted
    // whilst still activated.
    Echo_i* myimpl = new Echo_i();
    POA_Echo_tie<Echo_i> myecho(myimpl);

    PortableServer::ObjectId_var myechoid = poa->activate_object(&myecho);

    // Obtain a reference to the object, and register it in
    // the naming service.
    obj = myecho._this();
    if( !bindObjectToName(orb, obj) )
      return 1;

    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    orb->run();
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

//////////////////////////////////////////////////////////////////////

static CORBA::Boolean
bindObjectToName(CORBA::ORB_ptr orb, CORBA::Object_ptr objref)
{
  CosNaming::NamingContext_var rootContext;

  try {
    // Obtain a reference to the root context of the Name service:
    CORBA::Object_var obj;
    obj = orb->resolve_initial_references("NameService");

    // Narrow the reference returned.
    rootContext = CosNaming::NamingContext::_narrow(obj);
    if( CORBA::is_nil(rootContext) ) {
      cerr << "Failed to narrow the root naming context." << endl;
      return 0;
    }
  }
  catch(CORBA::ORB::InvalidName& ex) {
    // This should not happen!
    cerr << "Service required is invalid [does not exist]." << endl;
    return 0;
  }

  try {
    // Bind a context called "test" to the root context:

    CosNaming::Name contextName;
    contextName.length(1);
    contextName[0].id   = (const char*) "test";       // string copied
    contextName[0].kind = (const char*) "my_context"; // string copied
    // Note on kind: The kind field is used to indicate the type
    // of the object. This is to avoid conventions such as that used
    // by files (name.type -- e.g. test.ps = postscript etc.)

    CosNaming::NamingContext_var testContext;
    try {
      // Bind the context to root.
      testContext = rootContext->bind_new_context(contextName);
    }
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      // If the context already exists, this exception will be raised.
      // In this case, just resolve the name and assign testContext
      // to the object returned:
      CORBA::Object_var obj;
      obj = rootContext->resolve(contextName);
      testContext = CosNaming::NamingContext::_narrow(obj);
      if( CORBA::is_nil(testContext) ) {
        cerr << "Failed to narrow naming context." << endl;
        return 0;
      }
    }

    // Bind objref with name Echo to the testContext:
    CosNaming::Name objectName;
    objectName.length(1);
    objectName[0].id   = (const char*) "Echo";   // string copied
    objectName[0].kind = (const char*) "Object"; // string copied

    try {
      testContext->bind(objectName, objref);
    }
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      testContext->rebind(objectName, objref);
    }
    // Note: Using rebind() will overwrite any Object previously bound
    //       to /test/Echo with obj.
    //       Alternatively, bind() can be used, which will raise a
    //       CosNaming::NamingContext::AlreadyBound exception if the name
    //       supplied is already bound to an object.

    // Amendment: When using OrbixNames, it is necessary to first try bind
    // and then rebind, as rebind on it's own will throw a NotFoundexception if
    // the Name has not already been bound. [This is incorrect behaviour -
    // it should just bind].
  }
  catch(CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
         << "naming service." << endl;
    return 0;
  }
  catch(CORBA::SystemException&) {
    cerr << "Caught a CORBA::SystemException while using the naming service."
	 << endl;
    return 0;
  }

  return 1;
}
