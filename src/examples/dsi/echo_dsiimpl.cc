// echo_dsiimpl.cc
//
//               This example demonstrates the Dynamic Skeleton Interface.
//               It is a server for the eg2_clt client in the
//               src/examples/echo directory. Alternatively you can use the
//               DII echo client in src/examples/dii.
//
// Usage: echo_dsiimpl
//
//        On startup, the object reference is printed to cout as a
//        stringified IOR. This string should be used as the argument to 
//        eg2_clt or echo_diiclt.
//

#include <omniORB4/CORBA.h>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif


CORBA::ORB_var orb;


class MyDynImpl : public PortableServer::DynamicImplementation
{
public:
  virtual void invoke(CORBA::ServerRequest_ptr);
  virtual char* _primary_interface(const PortableServer::ObjectId&,
				   PortableServer::POA_ptr);
};


void
MyDynImpl::invoke(CORBA::ServerRequest_ptr request)
{
  try {
    if( strcmp(request->operation(), "echoString") )
      throw CORBA::BAD_OPERATION(0, CORBA::COMPLETED_NO);

    CORBA::NVList_ptr args;
    orb->create_list(0, args);
    CORBA::Any a;
    a <<= "";
    args->add_value("", a, CORBA::ARG_IN);

    request->arguments(args);

    const char* mesg;
    *(args->item(0)->value()) >>= mesg;

    CORBA::Any* result = new CORBA::Any();
    *result <<= CORBA::Any::from_string(mesg, 0);
    request->set_result(*result);
  }
  catch(CORBA::SystemException& ex){
    CORBA::Any a;
    a <<= ex;
    request->set_exception(a);
  }
  catch(...){
    cout << "echo_dsiimpl: MyDynImpl::invoke - caught an unknown exception."
	 << endl;
    CORBA::Any a;
    a <<= CORBA::UNKNOWN(0, CORBA::COMPLETED_NO);
    request->set_exception(a);
  }
}


char*
MyDynImpl::_primary_interface(const PortableServer::ObjectId&,
			      PortableServer::POA_ptr)
{
  return CORBA::string_dup("IDL:Echo:1.0");
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    orb = CORBA::ORB_init(argc, argv);

    {
      CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

      MyDynImpl* myimpl = new MyDynImpl;

      PortableServer::ObjectId_var myimplid = poa->activate_object(myimpl);

      // If we just use servant_to_reference() or id_to_reference() to
      // get an object reference, it will contain an empty repository
      // id, since the ORB has no way of knowing what the DSI
      // servant's interface is. Instead, we use
      // create_reference_with_id() to create a reference for the
      // object with the correct id. It would work with an empty id,
      // but it's polite to our clients to give them the full
      // information.

      obj = poa->create_reference_with_id(myimplid, "IDL:Echo:1.0");

      CORBA::String_var sior(orb->object_to_string(obj));
      cout << (char*)sior << endl;

      myimpl->_remove_ref();

      PortableServer::POAManager_var pman = poa->the_POAManager();
      pman->activate();

      orb->run();
    }
    orb->destroy();
  }
  catch(CORBA::SystemException& ex) {
    cerr << "Caught CORBA::" << ex._name() << endl;
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
