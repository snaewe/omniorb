// lcserver.cc -- LifeCycle example server
//
// See the LifeCycle tutorial for explanation.

#include <iostream.h>

#include "echolag.hh"


class EchoLag_i :
  public virtual _lc_sk_EchoLag,
  public virtual omniLC::_threadControl
{
private:
  char *s;

public:
  EchoLag_i(const char *initial);
  virtual ~EchoLag_i();

  virtual char *echoStringLag(const char *str);
  virtual void move(EchoLagFactory_ptr there);
  virtual void remove();
};

EchoLag_i::EchoLag_i(const char *initial) {
  s = CORBA::string_dup(initial);
  cout << "EchoLag_i created." << endl;
}

EchoLag_i::~EchoLag_i() {
  CORBA::string_free(s);
  cout << "EchoLag_i deleted." << endl;
}

char *
EchoLag_i::echoStringLag(const char *str) {
  omniLC::ThreadOp zz(this);

  char *p = CORBA::string_dup(s);
  CORBA::string_free(s);
  s = CORBA::string_dup(str);
  cout << "echoStringLag() returning: " << p << endl;
  return p;
}

void
EchoLag_i::move(EchoLagFactory_ptr there) {
  omniLC::ThreadLC zz(this);

  // Create a copy of ourselves:
  char *p = CORBA::string_dup(s);
  EchoLag_var newEchoLag = there->copy_EchoLag(p, _get_lifecycle());

  // Tell the system to do the move:
  _move(newEchoLag);

  // Dispose of this object:
  CORBA::BOA::getBOA()->dispose(this);
}

void
EchoLag_i::remove() {
  omniLC::ThreadLC zz(this);

  _remove();

  CORBA::BOA::getBOA()->dispose(this);
}


class EchoLagFactory_i :
  public virtual _sk_EchoLagFactory
{
public:
  virtual EchoLag_ptr new_EchoLag();
  virtual EchoLag_ptr copy_EchoLag(const char *str,
				   omniLifeCycleInfo_ptr li);
};

EchoLag_ptr
EchoLagFactory_i::new_EchoLag() {
  EchoLag_i *elag = new EchoLag_i("First call");
  elag->_obj_is_ready(elag->_boa());

  return elag->_this();
}

EchoLag_ptr
EchoLagFactory_i::copy_EchoLag(const char *str,
			       omniLifeCycleInfo_ptr li) {
  EchoLag_i *elag = new EchoLag_i(str);

  elag->_set_lifecycle(li);

  elag->_obj_is_ready(elag->_boa());

  return EchoLag::_duplicate(elag);
}


int
main(int argc, char **argv) {
  CORBA::ORB_ptr orb = CORBA::ORB_init(argc, argv, "omniORB2");
  CORBA::BOA_ptr boa = orb->BOA_init(argc, argv, "omniORB2_BOA");

  // Declare that EchoLag objects can move into this address space:
  EchoLag_proxyObjectFactory::mayMoveLocal(1);

  // Create an EchoLagFactory:
  EchoLagFactory_i *factory = new EchoLagFactory_i();
  factory->_obj_is_ready(boa);

  cout << "EchoLagFactory reference:" << endl
       << (char *)orb->object_to_string(factory) << endl << endl;

  // Use the factory to create a new EchoLag:
  EchoLag_var elag = factory->new_EchoLag();

  cout << "EchoLag reference:" << endl
       << (char *)orb->object_to_string(elag) << endl << endl;

  // Start the server:
  boa->impl_is_ready(0);

  // Call boa->impl_shutdown() from another thread would unblock the
  // main thread from impl_is_ready().
  //
  // To properly shutdown the BOA and the ORB, add the following calls
  // after impl_is_ready() returns.
  //
  // boa->destroy();
  // orb->NP_destroy();

  return 0;
}
