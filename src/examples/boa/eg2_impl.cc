// eg2_impl.cc - This is the source code of example 2 used in Chapter 2
//               "The Basics" of the omniORB2 user guide.
//
//               This is the object implementation.
//
// Usage: eg2_impl
//
//        On startup, the object reference is printed to cerr as a
//        stringified IOR. This string should be used as the argument to 
//        eg2_clt.
//

#include <iostream.h>
#include <becho.hh>


class Echo_i : public virtual _sk_Echo {
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

int main(int argc, char** argv)
{
  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
    CORBA::BOA_var boa = orb->BOA_init(argc, argv, "omniORB4_BOA");

    Echo_i* myecho = new Echo_i();
    myecho->_obj_is_ready(boa);

    Echo_var myechoref = myecho->_this();
    CORBA::String_var sior(orb->object_to_string(myechoref));
    cerr << "'" << (char*)sior << "'" << endl;

    boa->impl_is_ready();
    // Tell the BOA we are ready. The BOA's default behaviour is to block
    // on this call indefinitely.

    // Call boa->impl_shutdown() from another thread would unblock the
    // main thread from impl_is_ready().
    //
    // To properly shutdown the BOA and the ORB, add the following calls
    // after impl_is_ready() returns.
    //
    // boa->destroy();
    // orb->destroy();
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
