// echo_diiclt.cc
//
//              This example demonstrates the Dynamic Invocation Interface.
//              It is a client for the eg3_impl server in the
//              src/examples/echo directory. Alternatively you can use the
//              DSI echo server in src/examples/dsi.
//
// Usage: echo_diiclt
//
//
//        On startup, the client lookup the object reference from the
//        COS naming service.
//
//        The name which the object is bound to is as follows:
//              root  [context]
//               |
//              text  [context] kind [my_context]
//               |
//              Echo  [object]  kind [Object]
//

#include <iostream.h>
#include <omniORB2/CORBA.h>


static CORBA::Object_ptr getObjectReference(CORBA::ORB_ptr orb);


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

  CORBA::String_var ret;
  req->return_value() >>= ret.out();

  cout << "I said, \"" << (char*)arg << "\"."
       << " The Object said, \"" << (char*)ret <<"\"" << endl;
}


static void hello_deferred(CORBA::Object_ptr obj)
{
  CORBA::String_var arg = (const char*) "Hello!";
  CORBA::Request_var req = obj->_request("echoString");
  req->add_in_arg() <<= arg;
  req->set_return_type(CORBA::_tc_string);

  req->send_deferred();
  cerr << "Sending deferred request: ";
  while( !req->poll_response() )
    cerr << '#';
  cerr << endl << "Response received." << endl;

  if( req->env()->exception() ) {
    cout << "echo_diiclt: An exception was thrown!" << endl;
    return;
  }

  CORBA::String_var ret;
  req->return_value() >>= ret.out();

  cout << "I said, \"" << (char*)arg << "\"."
       << " The Object said, \"" << (char*)ret <<"\"" << endl;
}


int
main(int argc, char* argv[])
{
  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB2");
  CORBA::BOA_ptr boa = orb->BOA_init(argc,argv,"omniORB2_BOA");

  try {
    CORBA::Object_var obj = getObjectReference(orb);
    hello(obj);
    hello_deferred(obj);
  }
  catch(CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE, unable to contact the "
         << "object." << endl;
  }
  catch(omniORB::fatalException& ex) {
    cerr << "Caught omniORB2 fatalException. This indicates a bug is caught "
         << "within omniORB2.\nPlease send a bug report.\n"
         << "The exception was thrown in file: " << ex.file() << "\n"
         << "                            line: " << ex.line() << "\n"
         << "The error message is: " << ex.errmsg() << endl;
  }
  catch(...) {
    cerr << "Caught a system exception." << endl;
  }

  return 0;
}


static 
CORBA::Object_ptr
getObjectReference(CORBA::ORB_ptr orb)
{
  CosNaming::NamingContext_var rootContext;
  
  try {
    // Obtain a reference to the root context of the Name service:
    CORBA::Object_var initServ;
    initServ = orb->resolve_initial_references("NameService");

    // Narrow the object returned by resolve_initial_references()
    // to a CosNaming::NamingContext object:
    rootContext = CosNaming::NamingContext::_narrow(initServ);
    if (CORBA::is_nil(rootContext)) 
      {
        cerr << "Failed to narrow naming context." << endl;
        return CORBA::Object::_nil();
      }
  }
  catch(CORBA::ORB::InvalidName& ex) {
    cerr << "Service required is invalid [does not exist]." << endl;
    return CORBA::Object::_nil();
  }


  // Create a name object, containing the name test/context:
  CosNaming::Name name;
  name.length(2);

  name[0].id   = (const char*) "test";       // string copied
  name[0].kind = (const char*) "my_context"; // string copied
  name[1].id   = (const char*) "Echo";
  name[1].kind = (const char*) "Object";
  // Note on kind: The kind field is used to indicate the type
  // of the object. This is to avoid conventions such as that used
  // by files (name.type -- e.g. test.ps = postscript etc.)

  
  CORBA::Object_ptr obj;
  try {
    // Resolve the name to an object reference, and assign the reference 
    // returned to a CORBA::Object:
    obj = rootContext->resolve(name);
  }
  catch(CosNaming::NamingContext::NotFound& ex)
    {
      // This exception is thrown if any of the components of the
      // path [contexts or the object] aren't found:
      cerr << "Context not found." << endl;
      return CORBA::Object::_nil();
    }
  catch (CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE, unable to contact the "
         << "naming service." << endl;
    return CORBA::Object::_nil();
  }
  catch(omniORB::fatalException& ex) {
    throw;
  }
  catch (...) {
    cerr << "Caught a system exception while using the naming service."<< endl;
    return CORBA::Object::_nil();
  }
  return obj;
}
