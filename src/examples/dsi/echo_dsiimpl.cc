// echo_dsiimpl.cc
//
//               This example demonstrates the Dynamic Skeleton Interface.
//               It is a server for the eg3_clt client in the
//               src/examples/echo directory. Alternatively you can use the
//               DII echo client in src/examples/dii.
//
// Usage: echo_dsiimpl
//
//        On startup, the object reference is registered with the 
//        COS naming service. The client uses the naming service to
//        locate this object.
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


static CORBA::Boolean bindObjectToName(CORBA::ORB_ptr,CORBA::Object_ptr);

CORBA::ORB_ptr orb = 0;


class MyDynImpl : public CORBA::BOA::DynamicImplementation {
public:
  virtual void invoke(CORBA::ServerRequest_ptr request,
		      CORBA::Environment& env) throw();
};


void
MyDynImpl::invoke(CORBA::ServerRequest_ptr request, CORBA::Environment& env)
  throw()
{
  try {
    if( strcmp(request->op_name(), "echoString") )
      throw CORBA::BAD_OPERATION(0, CORBA::COMPLETED_NO);

    CORBA::NVList_ptr args;
    orb->create_list(0, args);
    CORBA::Any a;
    a.replace(CORBA::_tc_string, 0);
    args->add_value("", a, CORBA::ARG_IN);

    request->params(args);

    CORBA::String_var mesg;
    *(args->item(0)->value()) >>= mesg.out();

    CORBA::Any* result = new CORBA::Any();
    *result <<= CORBA::Any::from_string(mesg._retn(), 0);
    request->result(result);
  }
  catch(CORBA::SystemException& ex){
    env.exception(CORBA::Exception::_duplicate(&ex));
  }
  catch(...){
    cout << "echo_dsiimpl: MyDynImpl::invoke - caught an unknown exception."
	 << endl;
    env.exception(new CORBA::UNKNOWN(0, CORBA::COMPLETED_NO));
  }
}


int
main(int argc, char **argv)
{
  orb = CORBA::ORB_init(argc,argv,"omniORB2");
  CORBA::BOA_ptr boa = orb->BOA_init(argc,argv,"omniORB2_BOA");

  CORBA::Object_ptr obj = boa->create_dynamic_object(new MyDynImpl,
						     "IDL:Echo:1.0");
  boa->obj_is_ready(obj);

  if( !bindObjectToName(orb, obj) )
    return 1;

  boa->impl_is_ready();
  // Tell the BOA we are ready. The BOA's default behaviour is to block
  // on this call indefinitely.

  return 0;
}


static
CORBA::Boolean
bindObjectToName(CORBA::ORB_ptr orb, CORBA::Object_ptr obj)
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
        return 0;
      }
  }
  catch(CORBA::ORB::InvalidName& ex) {
    cerr << "Service required is invalid [does not exist]." << endl;
    return 0;
  }


  try {
    // Bind a context called "test" to the root context:

    CosNaming::Name contextName;
    contextName.length(1);
    contextName[0].id   = (const char*) "test";    // string copied
    contextName[0].kind = (const char*) "my_context"; // string copied    
    // Note on kind: The kind field is used to indicate the type
    // of the object. This is to avoid conventions such as that used
    // by files (name.type -- e.g. test.ps = postscript etc.)

    CosNaming::NamingContext_var testContext;
    try {
      // Bind the context to root, and assign testContext to it:
      testContext = rootContext->bind_new_context(contextName);
    }
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      // If the context already exists, this exception will be raised.
      // In this case, just resolve the name and assign testContext
      // to the object returned:
      CORBA::Object_var tmpobj;
      tmpobj = rootContext->resolve(contextName);
      testContext = CosNaming::NamingContext::_narrow(tmpobj);
      if (CORBA::is_nil(testContext)) {
        cerr << "Failed to narrow naming context." << endl;
        return 0;
      }
    } 

    // Bind the object (obj) to testContext, naming it Echo:
    CosNaming::Name objectName;
    objectName.length(1);
    objectName[0].id   = (const char*) "Echo";   // string copied
    objectName[0].kind = (const char*) "Object"; // string copied


    // Bind obj with name Echo to the testContext:
    try {
      testContext->bind(objectName,obj);
    }
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      testContext->rebind(objectName,obj);
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
  catch (CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE, unable to contact the "
         << "naming service." << endl;
    return 0;
  }
  catch (omniORB::fatalException& ex) {
    throw;
  }
  catch (...) {
    cerr << "Caught a system exception while using the naming service."<< endl;
    return 0;
  }
  return 1;
}
