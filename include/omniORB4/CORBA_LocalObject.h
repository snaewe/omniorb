// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_LocalObject.h        Created on: 2005/09/19
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2005 Apasphere Ltd.
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//    02111-1307, USA
//
//
// Description:
//    CORBA::LocalObject
//

/*
  $Log$
  Revision 1.1.2.1  2005/11/09 12:22:18  dgrisby
  Local interfaces support.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////////// LocalObject ////////////////////////////
//////////////////////////////////////////////////////////////////////

class LocalObject;
class LocalObject_var;
typedef LocalObject* LocalObject_ptr;

class LocalObject_Helper {
public:
  typedef LocalObject_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static void marshalObjRef(_ptr_type, cdrStream&);
  static _ptr_type unmarshalObjRef(cdrStream&);
};

class LocalObject : public virtual Object {
public:
  typedef LocalObject_ptr _ptr_type;
  typedef LocalObject_var _var_type;

  virtual void _add_ref();
  virtual void _remove_ref();
  // These functions do nothing; may be overridden by application code.

  virtual Boolean _non_existent();
  // Always returns false

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(Object_ptr o);
  static _ptr_type _unchecked_narrow(Object_ptr o);
  static _ptr_type _nil();

  //////////////////////
  // omniORB internal //
  //////////////////////

  virtual ~LocalObject();
  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  virtual void* _ptrToObjRef(const char* repoId);

  static void _marshalObjRef(_ptr_type, cdrStream&);
  static _ptr_type _unmarshalObjRef(cdrStream&);

  static _core_attr const char* _PD_repoId;

protected:
  inline LocalObject() { }

private:
  LocalObject(const LocalObject&);
  LocalObject& operator = (const LocalObject&);
  // not implemented
};
