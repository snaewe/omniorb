
#include <iostream.h>
#include <omniORB4/CORBA.h>

#include "value.hh"
#include "valimpl.h"

int main(int argc, char** argv)
{
  CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

  if( argc != 2 ) {
    cerr << "usage:  vclient <object reference>" << endl;
    return 1;
  }

  CORBA::Object_var obj = orb->string_to_object(argv[1]);

  ValueTest::Test_var test = ValueTest::Test::_narrow(obj);

  One_i* one = new One_i("hello", 12345);

  ValueTest::One* ret = test->op1(one);
  
  cout << "String: " << ret->s() << ", long: " << ret->l() << endl;

  CORBA::remove_ref(one);
  CORBA::remove_ref(ret);

  orb->destroy();

  return 0;
}
