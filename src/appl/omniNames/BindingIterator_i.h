#ifndef _BindingIterator_i_h_
#define _BindingIterator_i_h_

#include "NamingContext_i.h"

class BindingIterator_i : public virtual CosNaming::_sk_BindingIterator {

public:

  BindingIterator_i(CORBA::BOA_ptr boa, CosNaming::BindingList* l)
    : list(l)
  {
    _obj_is_ready(boa);
  }

  CORBA::Boolean next_one(CosNaming::Binding*& b) {

    CosNaming::BindingList* bl;
    CORBA::Boolean ret = next_n(1, bl);
    b = new CosNaming::Binding;
    if (ret)
      *b = (*bl)[0];
    delete bl;
    return ret;
  }

  CORBA::Boolean next_n(CORBA::ULong how_many, CosNaming::BindingList*& bl) {

    //
    // What we do here is return the current list to the caller, shortening
    // it to the required length.  Before this however, we create a new list
    // and copy the rest of the bindings into the new list.
    //

    if (list->length() < how_many) {
      how_many = list->length();
    }
    bl = list;
    list = new CosNaming::BindingList(bl->length() - how_many);
    list->length(bl->length() - how_many);
    for (unsigned int i = 0; i < list->length(); i++) {
      (*list)[i] = (*bl)[i + how_many];
    }

    bl->length(how_many);

    if (how_many == 0)
      return 0;
    else
      return 1;
  }

  void destroy(void) {
    // remember the destructor for an object should never be called explicitly.
    _dispose();
  }

private:

  CosNaming::BindingList* list;

  // remember the destructor for an object should never be called explicitly.
  ~BindingIterator_i() {
    delete list;
  }
};

#endif
