// echo_dsiimpl.cc
//
//               This example demonstrates the Dynamic Skeleton Interface.
//               It is a server for the eg2_clt client in the
//               src/examples/echo directory. Alternatively you can use the
//               DII echo client in src/examples/dii.
//
// Usage: echo_dsiimpl
//
//        On startup, the object reference is printed to cerr as a
//        stringified IOR. This string should be used as the argument to 
//        eg2_clt or echo_diiclt.
//

#include <iostream.h>
#include <omniORB4/CORBA.h>


CORBA::ORB_var orb;


class MyDynImpl : public PortableServer::DynamicImplementation,
		  public PortableServer::RefCountServantBase
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
    a.replace(CORBA::_tc_string, 0);
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
    orb = CORBA::ORB_init(argc, argv, "omniORB4");

    {
      CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

      MyDynImpl* myimpl = new MyDynImpl;

#if 0
      PortableServer::ObjectId_var myimplid = poa->activate_object(myimpl);

      // NB. PortableServer::DynamicImplementation::_this() can
      // only be used in the context of an invocation, so we cannot
      // use it to get a reference here.
      obj = poa->servant_to_reference(myimpl);
#else
      // Although servant_to_reference(myimpl) above will suceed, it
      // will return a typeless reference.  When the client attempts
      // to narrow this, it will contact the object and ask it if it
      // is really an Echo object.  This is not currently implemented
      // for DSI servants, since it requires the support of the
      // PortableServer::Current interface.
      //  We get round it here by specifying the interface that we
      // want for the reference, then using the object id encapsulated
      // by the reference to incarnate the object.

      obj = poa->create_reference("IDL:Echo:1.0");
      PortableServer::ObjectId_var myimplid = poa->reference_to_id(obj);
      poa->activate_object_with_id(myimplid, myimpl);
#endif

      CORBA::String_var sior(orb->object_to_string(obj));
      cerr << "'" << (char*)sior << "'" << endl;

      myimpl->_remove_ref();

      PortableServer::POAManager_var pman = poa->the_POAManager();
      pman->activate();

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
