// anyExample_clt.cc -  This is the source code of the example used in 
//                      Chapter 9 "Type Any and TypeCode" of the omniORB
//                      users guide.
//
//                      This is the client.
//
// Usage: anyExample_clt <object reference>
//

#include <anyExample.hh>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif


static void invokeOp(anyExample_ptr& tobj, const CORBA::Any& a)
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


static void hello(anyExample_ptr tobj)
{
  CORBA::Any a;

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

//////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  try {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    if( argc != 2 ) {
      cerr << "usage:  anyExample_clt <object reference>" << endl;
      return 1;
    }

    {
      CORBA::Object_var obj = orb->string_to_object(argv[1]);
      anyExample_var ref = anyExample::_narrow(obj);
      if( CORBA::is_nil(ref) ) {
	cerr << "Can't narrow reference to type anyExample (or it was nil)."
	     << endl;
	return 1;
      }
      hello(ref);
    }
    orb->destroy();
  }
  catch(CORBA::COMM_FAILURE& ex) {
    cerr << "Caught system exception COMM_FAILURE -- unable to contact the "
         << "object." << endl;
  }
  catch(CORBA::SystemException&) {
    cerr << "Caught a CORBA::SystemException." << endl;
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
