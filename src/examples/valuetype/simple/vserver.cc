
#include <iostream.h>
#include <omniORB4/CORBA.h>

#include "value.hh"
#include "valimpl.h"


class Test_i : public virtual POA_ValueTest::Test,
	       public virtual PortableServer::RefCountServantBase
{
public:
  virtual ~Test_i() {}
  
  ValueTest::One* op1(ValueTest::One* a, ValueTest::One* b);
  ValueTest::Two* op2(const ValueTest::Two& a);
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

ValueTest::Two*
Test_i::op2(const ValueTest::Two& a)
{
  return 0;
}



int main(int argc, char** argv)
{
  CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

  OneFactory* onef = new OneFactory();
  orb->register_value_factory("IDL:ValueTest/One:1.0", onef);

  CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

  Test_i* servant = new Test_i();

  obj = servant->_this();
  CORBA::String_var sior(orb->object_to_string(obj));
  cerr << "'" << (char*)sior << "'" << endl;

  servant->_remove_ref();

  PortableServer::POAManager_var pman = poa->the_POAManager();
  pman->activate();

  orb->run();

  return 0;
}
