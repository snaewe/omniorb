#include <omniORB2/CORBA.h>
#include <iostream.h>

extern void stripFireWallComponents(CORBA::Object_ptr obj);

int
main(int argc, char** argv) {

  if (argc != 2) {
    cerr << "usage: test_rewrite_ior <ior>" << endl;
    return 1;
  }

  try {
     CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB2");
     CORBA::Object_var obj = orb->string_to_object(argv[1]);

     stripFireWallComponents(obj);
     CORBA::String_var n = orb->object_to_string(obj);

     cerr << (const char*) n << endl;
  }
  catch(...) {
    cerr << "Unexpected exception. Terminate." << endl;
    return 1;
  }
  return 0;
}
