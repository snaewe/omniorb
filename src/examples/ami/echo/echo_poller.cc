// echo.cc - Creates a ReplyHandler servant and sends an async request to
//           an echo object.
//
// Usage:
//   echo_ami <IOR of echo server>
//

#include <iostream.h>
#include <echo_ami.hh>


CORBA::ORB_var orb;

unsigned long int nInvocations = 0;



int main(int argc, char** argv)
{
  try {
    orb = CORBA::ORB_init(argc, argv, "omniORB3");
    
    if (argc != 3){
      cerr << "usage:  " << argv[0] << " <nInvocations> <echo IOR>" << endl;
      return 1;
    }
    // Grab the number of Invocations
    sscanf(argv[1], "%lu", &nInvocations);

    // Grab the Echo server IOR
    CORBA::Object_var obj = orb->string_to_object(argv[2]);
    Echo_var echoref = Echo::_narrow(obj);
    if( CORBA::is_nil(echoref) ) {
      cerr << "Can't narrow reference to type Echo (or it was nil)." << endl;
      return 1;
    }
    
    // Initiate the ReplyHandler servant
    CORBA::Object_var poa_obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(poa_obj);

    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();


    AMI_EchoPoller **pollers = new AMI_EchoPoller*[nInvocations];;

    // Make the async request
    CORBA::String_var message = (const char*)"Hello there";
    for (unsigned long int x = 0; x < nInvocations; x++){
      pollers[x] = echoref->sendp_echoString(message);
    }
    cout << "I sent, \"" << message << "\"";
    if (nInvocations > 1)
      cout << " " << nInvocations << " times";
    cout << "." << endl;

    cout << "Polling for replies..." << endl;
    try{
      for (unsigned long int x = 0; x < nInvocations; x++){
	CORBA::String_var result;
	CORBA::Boolean done = 0;
	while (!done){
	  try{
	    pollers[x]->echoString(100 /* ms */, result);
	    done = 1;
	  }catch(CORBA::TIMEOUT &e){
	    cerr << "Caught CORBA::TIMEOUT-- Retrying" << endl;
	  }
	}
	if (strcmp(message, result) != 0){
	  cout << "Received a different message!" << endl;
	  abort();
	}
	pollers[x]->_remove_ref();
      }
      cout << "Received all replies." << endl;

    }catch(CORBA::TIMEOUT &e){
      cerr << "Caught CORBA::TIMEOUT." << endl;
    }catch(CORBA::PollableSet::NoPossiblePollable &e){
      cerr << "Caught CORBA::PollableSet::NoPossiblePollable." << endl;
    }catch(CORBA::PollableSet::UnknownPollable &e){
      cerr << "Caught CORBA::PollableSet::UnknownPollable." << endl;
    }catch(CORBA::WRONG_TRANSACTION &e){
      cerr << "Caught CORBA::WrongTransaction." << endl;
    }

    cout << "Shutting down..." << endl;
    orb->shutdown(1);

    sleep(20000);

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

