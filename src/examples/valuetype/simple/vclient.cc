#include <omniORB4/CORBA.h>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif

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

  ValueTest::One_var ret;

  ret = test->op1(one, two);
  
  if (ret.in()) {
    cout << "String: " << ret->s() << ", long: " << ret->l() << endl;
  }
  else {
    cout << "nil" << endl;
  }

  ret = test->op1(one, one);
  ret = test->op1(one, 0);
  ret = test->op1(0, one);
  ret = test->op1(0, 0);

  CORBA::remove_ref(one);
  CORBA::remove_ref(two);
  onef->_remove_ref();

  orb->destroy();

  return 0;
}
