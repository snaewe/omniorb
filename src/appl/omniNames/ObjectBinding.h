#ifndef _ObjectBinding_h_
#define _ObjectBinding_h_

#include "NamingContext_i.h"

class ObjectBinding {

public:

  CosNaming::Binding binding;
  CORBA::Object_ptr object;

  NamingContext_i* nc;
  ObjectBinding* prev;
  ObjectBinding* next;

  ObjectBinding(const CosNaming::Name& n, CosNaming::BindingType t,
		CORBA::Object_ptr o, NamingContext_i* nct,
		ObjectBinding* nx = 0)
    : object(o), nc(nct), next(nx)
  {
    binding.binding_name = n;
    binding.binding_type = t;
    if (next) {
      prev = next->prev;
      next->prev = this;
    } else {
      prev = nc->tailBinding;
      nc->tailBinding = this;
    }
    if (prev) {
      prev->next = this;
    } else {
      nc->headBinding = this;
    }
    nc->size++;
  }

  ~ObjectBinding()
  {
    if (prev) {
      prev->next = next;
    } else {
      nc->headBinding = next;
    }
    if (next) {
      next->prev = prev;
    } else {
      nc->tailBinding = prev;
    }
    nc->size--;
  }

};

#endif
