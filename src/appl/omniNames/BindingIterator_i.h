// -*- Mode: C++; -*-
//                          Package   : omniNames
// BindingIterator_i.h      Author    : Tristan Richardson (tjr)
//
//    Copyright (C) 1997 Olivetti & Oracle Research Laboratory
//
//  This file is part of omniNames.
//
//  omniNames is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//

#ifndef _BindingIterator_i_h_
#define _BindingIterator_i_h_

#include <NamingContext_i.h>

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
