// mainthread.cc -- Demonstrate the main thread policy

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif
#include <echo.hh>

CORBA::ORB_ptr orb;


class Echo_i : public POA_Echo,
	       public PortableServer::RefCountServantBase
{
public:
  inline Echo_i() {}
  virtual ~Echo_i() {}
  virtual char* echoString(const char* mesg);
};


char* Echo_i::echoString(const char* mesg)
{
  cerr << "Upcall " << mesg << endl;
  if (!strcmp(mesg, "shutdown"))
    orb->shutdown(0);

  return CORBA::string_dup(mesg);
}

int main(int argc, char** argv)
{
  try {
    orb = CORBA::ORB_init(argc, argv);

    {
      CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj);
      PortableServer::POAManager_var poa_man = root_poa->the_POAManager();
      poa_man->activate();

      // Create a new POA with the MAIN_THREAD_MODEL policy
      CORBA::PolicyList pl;
      pl.length(1);
      pl[0] = root_poa->create_thread_policy(PortableServer::MAIN_THREAD_MODEL);
      PortableServer::POA_var poa = root_poa->create_POA("mypoa", poa_man, pl);

      // Create and activate servant
      Echo_i* myecho = new Echo_i();
      
      PortableServer::ObjectId_var myechoid = poa->activate_object(myecho);

      // Obtain a reference to the object, and print it out as a
      // stringified IOR.
      obj = myecho->_this();
      CORBA::String_var sior(orb->object_to_string(obj));
      cerr << "'" << (char*)sior << "'" << endl;

      myecho->_remove_ref();

#if 0
      orb->run();
#else
      while(1) {
	omni_thread::sleep(1);
	if (orb->work_pending()) {
	  cout << "work pending" << endl;
	  orb->perform_work();
	}
	else {
	  cout << "no work pending" << endl;
	}
      }
#endif
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
