// -*- c++ -*-

#include <iostream.h>
#include "value.hh"


class One_i : public virtual OBV_ValueTest::One,
	      public virtual CORBA::DefaultValueRefCountBase
{
public:
  One_i(const char* s, long l)
    : OBV_ValueTest::One(s, l)
  {
    cout << "One_i created with values" << endl;
  }

  One_i()
    : OBV_ValueTest::One()
  {
    cout << "One_i created without values" << endl;
  }

  ~One_i() {
    cout << "One_i deleted" << endl;
  }
};

class OneFactory : public virtual CORBA::ValueFactoryBase
{
  CORBA::ValueBase* create_for_unmarshal()
  {
    cout << "create_for_unmarshal" << endl;
    return new One_i();
  }
};
