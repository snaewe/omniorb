// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Object.h             Created on: 2001/08/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//    CORBA::Object
//

/*
  $Log$
  Revision 1.1.2.2  2001/11/06 15:41:34  dpg1
  Reimplement Context. Remove CORBA::Status. Tidying up.

  Revision 1.1.2.1  2001/08/17 13:39:47  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////////////// Object ///////////////////////////////
//////////////////////////////////////////////////////////////////////

class Object_Helper {
public:
  typedef Object_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static void marshalObjRef(_ptr_type, cdrStream&);
  static _ptr_type unmarshalObjRef(cdrStream&);
};

class Object {
public:
  typedef Object_ptr _ptr_type;
  typedef Object_var _var_type;

  virtual ~Object();

  void _create_request(Context_ptr ctx,
		       const char *operation,
		       NVList_ptr arg_list,
		       NamedValue_ptr result,
		       Request_out request,
		       Flags req_flags);

  void _create_request(Context_ptr ctx,
		       const char *operation,
		       NVList_ptr arg_list,
		       NamedValue_ptr result,
		       ExceptionList_ptr exceptions,
		       ContextList_ptr ctxlist,
		       Request_out request,
		       Flags req_flags);

  Request_ptr _request(const char* operation);

  ImplementationDef_ptr _get_implementation();
  _objref_InterfaceDef* _get_interface();
  // = InterfaceDef_ptr    _get_interface();

  Boolean         _is_a(const char* repoId);
  virtual Boolean _non_existent();
  Boolean         _is_equivalent(_ptr_type other_object);
  ULong           _hash(ULong maximum);

  static _ptr_type        _duplicate(_ptr_type);
  static inline _ptr_type _narrow(Object_ptr o) { return _duplicate(o); }
  static _ptr_type        _nil();

  //////////////////////
  // omniORB internal //
  //////////////////////

  inline Object() : pd_obj(0), pd_magic(_PR_magic) {}

  inline Boolean _NP_is_nil() const { return pd_obj == 0; }
  inline Boolean _NP_is_pseudo() const { return pd_obj == (omniObjRef*) 1; }
  inline omniObjRef* _PR_getobj() { return pd_obj; }
  inline void _PR_setobj(omniObjRef* o) { pd_obj = o; }
  static inline _CORBA_Boolean _PR_is_valid(Object_ptr p) {
    return p ? (p->pd_magic == _PR_magic) : 1;
  }

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  virtual void* _ptrToObjRef(const char* repoId);

  static void _marshalObjRef(Object_ptr, cdrStream&);
  static Object_ptr _unmarshalObjRef(cdrStream&);

  static _core_attr const char* _PD_repoId;
  static _core_attr const ULong _PR_magic;

private:
  omniObjRef* pd_obj;
  // <obj> is 0 for a nil reference, 1 for a pseudo reference.

  ULong       pd_magic;
};

_CORBA_MODULE_FN inline Boolean is_nil(Object_ptr o) {
  if( !Object::_PR_is_valid(o) )  return 0;
  if( o )                         return o->_NP_is_nil();
  else {
    // omniORB does not use a nil pointer to represent a nil object
    // reference. The program has passed in a pointer which has not
    // been initialised by CORBA::Object::_nil() or similar functions.
    // Some ORBs seems to be quite lax about this. We don't want to
    // break the applications that make this assumption. Just call
    // _CORBA_use_nil_ptr_as_nil_objref() to take note of this.
    return _CORBA_use_nil_ptr_as_nil_objref();
  }
}
_CORBA_MODULE_FN inline void release(Object_ptr o) {
  if( o && !o->_NP_is_nil() ) {
    if( o->_NP_is_pseudo() )  o->_NP_decrRefCount();
    else  omni::releaseObjRef(o->_PR_getobj());
  }
}
