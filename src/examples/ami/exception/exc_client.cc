// exc_client.cc - Creates a ReplyHandler servant and sends an async request to
//                 an object which sends back an exception.
//
// Usage:
//   exc_client <IOR of server>
//

#include <iostream.h>
#include <stdio.h>
#include <exc.hh>

#define STRING

CORBA::ORB_var orb;
unsigned long int nInvocations;

//
// Example class implementing IDL interface I
//
class AMI_IHandler_i: public POA_AMI_IHandler,
		      public PortableServer::RefCountServantBase {
private:
  // Make sure all instances are built on the heap by making the
  // destructor non-public
  virtual ~AMI_IHandler_i() {}
  unsigned long int nReceived;
  omni_mutex state_lock;
public:
  AMI_IHandler_i(): nReceived(0) {}

  // methods corresponding to defined IDL attributes and operations
  void op(){
    cout << "Server replied without an exception." << endl;
    orb->shutdown(0);
  }
  void op_excep(const struct AMI_IExceptionHolder &excep_holder){
    try{
      //AMI_IExceptionHolder _e = excep_holder;
      //_e.raise_op();

    } catch (CustomException &e){
      omni_mutex_lock lock(state_lock);
      nReceived ++;
      if (nReceived == nInvocations){
	cout << "Caught a String Exception containing \"" 
	     << e.message << "\"";
	if (nInvocations > 1)
	  cout << " " << nInvocations << " times";
	cout << "." << endl;
	orb->shutdown(0);
      }
    } catch (CORBA::UserException &e){
      cout << "Caught a UserException" << endl;
    } catch (CORBA::SystemException &e){
      cout << "Caught a SystemException" << endl;
    } catch (CORBA::Exception &e){
      cout << "Caught an Exception" << endl;
    } catch (...){
      cout << "Caught an unknown exception type" << endl;
    }
  }
};

int main(int argc, char** argv)
{
  try {
    orb = CORBA::ORB_init(argc, argv, "omniORB3");
    
    if (argc != 3){
      cerr << "usage:  " << argv[0] << " <nInvocations> <server IOR>" << endl;
      return 1;
    }
    
    sscanf(argv[1], "%lu", &nInvocations);
    if (nInvocations == 0) return 0;

    // Grab the Echo server IOR
    CORBA::Object_var obj = orb->string_to_object(argv[2]);
    I_var Iref = I::_narrow(obj);
    if( CORBA::is_nil(Iref) ) {
      cerr << "Can't narrow reference to type I (or it was nil)." << endl;
      return 1;
    }
    
    // Initiate the ReplyHandler servant
    CORBA::Object_var poa_obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(poa_obj);

    AMI_IHandler_i* handler_servant = new AMI_IHandler_i();
    PortableServer::ObjectId_var handler_id =
      poa->activate_object(handler_servant);
    AMI_IHandler_var handler_ref = handler_servant->_this();

    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    // Make the async request
#ifdef STRING
    CORBA::String_var message = (const char*)"Hello there";
#else
    CORBA::ULong message = 4;
#endif
    for (unsigned int i = 0 ; i < nInvocations; i++)
      Iref->sendc_op(handler_ref, message);
    cout << "I said";
    if (nInvocations > 1)
      cout << " " << nInvocations << " times";
    cout << ", \"" << message << "\"." << endl;

    orb->run();
    cout << "Returned from orb->run();" << endl;
    orb->destroy();
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
    throw;
  }
  return 0;
}

