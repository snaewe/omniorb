
#include <iostream.h>
#include <omniORB4/CORBA.h>

#include "value.hh"
#include "valimpl.h"

int main(int argc, char** argv)
{
  CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

  OneFactory* onef = new OneFactory();
  orb->register_value_factory("IDL:ValueTest/One:1.0", onef);

  if( argc != 2 ) {
    cerr << "usage:  vclient <object reference>" << endl;
    return 1;
  }

  CORBA::Object_var obj = orb->string_to_object(argv[1]);

  ValueTest::Test_var test = ValueTest::Test::_narrow(obj);

  One_i* one = new One_i("hello", 12345);
  One_i* two = new One_i("hello again", 88888);

  ValueTest::One* ret = test->op1(one, two);
  
  if (ret) {
    cout << "String: " << ret->s() << ", long: " << ret->l() << endl;
  }
  else {
    cout << "nil" << endl;
  }

  CORBA::remove_ref(one);
  CORBA::remove_ref(two);
  CORBA::remove_ref(ret);
  onef->_remove_ref();

  orb->destroy();

  return 0;
}
