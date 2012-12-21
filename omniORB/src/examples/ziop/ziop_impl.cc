// ziop_impl.cc
//
// This is a variant of eg2_impl from src/examples/echo that enables
// ZIOP.
//
// Usage:
//   ziop_impl -ORBserverTransportRule "* unix,ssl,tcp,ziop"
//


#include <echo.hh>
#include <omniORB4/omniZIOP.h>

#include <iostream>
#include <stdlib.h>

using namespace std;


class Echo_i : public POA_Echo
{
public:
  inline Echo_i() {}
  virtual ~Echo_i() {}
  virtual char* echoString(const char* mesg);
};


char* Echo_i::echoString(const char* mesg)
{
  return CORBA::string_dup(mesg);
}

//////////////////////////////////////////////////////////////////////

void usage()
{
  cerr << "usage: ziop_impl -ORBserverTransportRule \"* unix,ssl,tcp,ziop\""
       << endl;
  exit(1);
}


int main(int argc, char** argv)
{
  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    if (argc != 1) {
      cerr << "usage: ziop_impl -ORBserverTransportRule \"* unix,ssl,tcp,ziop\""
           << endl;
      exit(1);
    }

    {
      CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
      PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj);

      PortableServer::POAManager_var pman = root_poa->the_POAManager();
      pman->activate();

      // Create a new POA with ZIOP policies
      Compression::CompressorIdLevelList ids;
      ids.length(1);
      ids[0].compressor_id     = Compression::COMPRESSORID_ZLIB;
      ids[0].compression_level = 6;

      CORBA::PolicyList pl;
      pl.length(2);
      pl[0] = omniZIOP::create_compression_enabling_policy(1);
      pl[1] = omniZIOP::create_compression_id_level_list_policy(ids);

      PortableServer::POA_var poa = root_poa->create_POA("my poa", pman, pl);
      
      // Activate the echo object...
      Echo_i* myecho = new Echo_i();
      PortableServer::ObjectId_var oid = poa->activate_object(myecho);

      obj = myecho->_this();
      CORBA::String_var sior(orb->object_to_string(obj));
      cout << (char*)sior << endl;

      myecho->_remove_ref();

      orb->run();
    }
    orb->destroy();
  }
  catch(PortableServer::POA::InvalidPolicy& ex) {
    cerr << "InvalidPolicy: " << ex.index << endl;
  }
  catch(CORBA::SystemException& ex) {
    cerr << "Caught CORBA::" << ex._name() << endl;
  }
  catch(CORBA::Exception& ex) {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
  }
  return 0;
}
