
#include <iostream.h>
#include <echo_ami.hh>


CORBA::ORB_var orb;
//
// Example class implementing IDL interface AMI_EchoHandler
//
class AMI_EchoHandler_i: public POA_AMI_EchoHandler,
			 public PortableServer::RefCountServantBase {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  virtual ~AMI_EchoHandler_i() {}
public:
  AMI_EchoHandler_i() {}

  // methods corresponding to defined IDL attributes and operations
  void echoString(const char* ami_return_val){
    cout << "The server said, \"" << ami_return_val << "\"." << endl;
    //orb->shutdown(0);
  }
  void echoString_excep(const struct AMI_EchoExceptionHolder &excep_holder){
    cout << "Received ExceptionHolder from server" << endl;
    try{
      AMI_EchoExceptionHolder _e = excep_holder;
      _e.raise_echoString();

    } catch (CORBA::UserException &e){
      cout << "Caught a UserException" << endl;
    } catch (CORBA::SystemException &e){
      cout << "Caught a SystemException" << endl;
    } catch (CORBA::Exception &e){
      cout << "Caught an Exception" << endl;
    } catch (...){
      cout << "Caught an unknown exception type" << endl;
    }
    //orb->shutdown(0);
  }
};

int main(int argc, char** argv)
{
  try {
    orb = CORBA::ORB_init(argc, argv, "omniORB3");
    
    if (argc != 2){
      cerr << "usage:  " << argv[0] << " <echo IOR>" << endl;
      return 1;
    }
    
    // Grab the Echo server IOR
    CORBA::Object_var obj = orb->string_to_object(argv[1]);
    Echo_var echoref = Echo::_narrow(obj);
    if( CORBA::is_nil(echoref) ) {
      cerr << "Can't narrow reference to type Echo (or it was nil)." << endl;
      return 1;
    }
    
    // Initiate the ReplyHandler servant
    CORBA::Object_var poa_obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(poa_obj);

    AMI_EchoHandler_i* handler_servant = new AMI_EchoHandler_i();
    PortableServer::ObjectId_var handler_id =
      poa->activate_object(handler_servant);
    AMI_EchoHandler_var handler_ref = handler_servant->_this();

    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    // Make the async request
    CORBA::String_var message = (const char*)"Hello there";
    echoref->sendc_echoString(handler_ref, message);
    cout << "I said, \"" << message << "\"." << endl;

    orb->run();
  }
  catch(CORBA::SystemException&) {
    cerr << "Caught CORBA::SystemException." << endl;
		throw;
  }
  catch(CORBA::Exception&) {
    cerr << "Caught CORBA::Exception." << endl;
		throw;
  }
  catch(omniORB::fatalException& fe) {
    cerr << "Caught omniORB::fatalException:" << endl;
    cerr << "  file: " << fe.file() << endl;
    cerr << "  line: " << fe.line() << endl;
    cerr << "  mesg: " << fe.errmsg() << endl;
		throw;
  }
  catch(...) {
    cerr << "Caught unknown exception." << endl;
		abort();
		throw;
  }
  return 0;
}

