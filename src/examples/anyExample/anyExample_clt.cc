// anyExample_clt.cc -  This is the source code of the example used in 
//                      Chapter 9 "Type Any and TypeCode" of the omniORB2 
//                      users guide.
//
//              This is the client. It uses the COSS naming service
//              to obtain the object reference.
//
// Usage: anyExample_clt
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
//              anyExample  [object]  kind [Object]
//

#include <iostream.h>
#include "anyExample.hh"

static CORBA::Object_ptr getObjectReference(CORBA::ORB_ptr orb);
static void invokeOp(anyExample_ptr& tobj, const CORBA::Any& a);

int
main (int argc, char **argv) 
{
  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB2");
  CORBA::Object_var obj;
  
  try {
    obj = getObjectReference(orb);
  }
  catch(CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE, unable to contact the "
         << "object." << endl;
    return -1;
  }
  catch(omniORB::fatalException& ex) {
    cerr << "Caught omniORB2 fatalException. This indicates a bug is caught "
         << "within omniORB2.\nPlease send a bug report.\n"
         << "The exception was thrown in file: " << ex.file() << "\n"
         << "                            line: " << ex.line() << "\n"
         << "The error message is: " << ex.errmsg() << endl;
    return -1;
  }
  catch(...) {
    cerr << "Caught a system exception." << endl;
    return -1;
  }


  anyExample_ptr tobj = anyExample::_narrow(obj);
  
  if (CORBA::is_nil(tobj)) {
    cerr << "Can't narrow object reference to type anyExample." << endl;
    return -1;
  }


  CORBA::Any a;

  try {
    // Sending Long
    CORBA::Long l = 100;
    a <<= l;
    cout << "Sending Any containing Long: " << l << endl; 
    invokeOp(tobj,a);
    
    // Sending Double
#ifndef NO_FLOAT
    CORBA::Double d = 1.2345;
    a <<= d;
    cout << "Sending Any containing Double: " << d << endl; 
    invokeOp(tobj,a);
#endif
  
    // Sending String
    const char* str = "Hello";
    a <<= str;
    cout << "Sending Any containing String: " << str << endl;
    invokeOp(tobj,a);
    
    // Sending testStruct  [Struct defined in IDL]
    testStruct t;
    t.l = 456;
    t.s = 8;
    a <<= t;
    cout << "Sending Any containing testStruct: l: " << t.l << endl;
    cout << "                                   s: " << t.s << endl;
    invokeOp(tobj,a);
  }
  catch(CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE, unable to contact the "
         << "object." << endl;
    return -1;
  }
  catch(omniORB::fatalException& ex) {
    cerr << "Caught omniORB2 fatalException. This indicates a bug is caught "
         << "within omniORB2.\nPlease send a bug report.\n"
         << "The exception was thrown in file: " << ex.file() << "\n"
         << "                            line: " << ex.line() << "\n"
         << "The error message is: " << ex.errmsg() << endl;
    return -1;
  }
  catch(...) {
    cerr << "Caught a system exception." << endl;
    return -1;
  }

  orb->NP_destroy();
	
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
  name[1].id   = (const char*) "anyExample";
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


static void 
invokeOp(anyExample_ptr& tobj, const CORBA::Any& a)
{
  CORBA::Any_var bp;

  cout << "Invoking operation." << endl;
  bp = tobj->testOp(a);

  cout << "Operation completed. Returned Any: ";
  CORBA::ULong ul;

  if (bp >>= ul) {
    cout << "ULong: " << ul << "\n" << endl;
  }
  else {
    cout << "Unknown value." << "\n" << endl;
  }
}
