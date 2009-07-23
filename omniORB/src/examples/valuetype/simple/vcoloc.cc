#include <omniORB4/CORBA.h>

#ifdef HAVE_STD
#  include <iostream>
   using namespace std;
#else
#  include <iostream.h>
#endif

#include "value.hh"
#include "valimpl.h"


class Test_i : public virtual POA_ValueTest::Test
{
public:
  virtual ~Test_i() {}
  
  ValueTest::One* op1(ValueTest::One* a, ValueTest::One* b);
};

ValueTest::One*
Test_i::op1(ValueTest::One* a, ValueTest::One* b)
{
  if (a)
    cout << "op1: a " << a->s() << ", " << a->l() << endl;
  else
    cout << "op1: a nil" << endl;

  if (b)
    cout << "op1: b " << b->s() << ", " << b->l() << endl;
  else
    cout << "op1: b nil" << endl;

  if (a == b)
    cout << "op1: a==b" << endl;
  else
    cout << "op1: a!=b" << endl;

  CORBA::add_ref(a);
  return a;
}


int main(int argc, char** argv)
{
  CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

  OneFactory* onef = new OneFactory();
  orb->register_value_factory("IDL:ValueTest/One:1.0", onef);

  CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

  Test_i* servant = new Test_i();

  ValueTest::Test_var test = servant->_this();

  servant->_remove_ref();

  PortableServer::POAManager_var pman = poa->the_POAManager();
  pman->activate();

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
