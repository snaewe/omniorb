// echo_i.cc - This source code demonstrates an implmentation of the
//             object interface Echo. It is part of the three examples
//             used in Chapter 2 "The Basics" of the omniORB2 user guide.
//
#include <string.h>
#include "echo.hh"

class Echo_i : public virtual _sk_Echo {
public:
  Echo_i() {}
  virtual ~Echo_i() {}
  virtual char * echoString(const char *mesg);
};

char *
Echo_i::echoString(const char *mesg) {
  char *p = CORBA::string_dup(mesg);
  return p;
}
