// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_ValueBase.h          Created on: 2003/08/11
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2003-2006 Apasphere Ltd.
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
//    ValueType definitions
//

/*
  $Log$
  Revision 1.1.2.8  2006/10/23 15:08:31  dgrisby
  Suppress GCC warnings about missing base class constructor calls.
  Thanks Tamas Kerecsen. Somehow, this patch works on VC++ 6, where
  before a similar change failed.

  Revision 1.1.2.7  2006/01/19 17:21:59  dgrisby
  Avoid member name conflict in DefaultValueRefCountBase.

  Revision 1.1.2.6  2005/08/16 13:51:21  dgrisby
  Problems with valuetype / abstract interface C++ mapping.

  Revision 1.1.2.5  2005/01/17 18:08:41  dgrisby
  Small tweaks to compile with Sun CC.

  Revision 1.1.2.4  2005/01/06 16:39:23  dgrisby
  DynValue and DynValueBox implementations; misc small fixes.

  Revision 1.1.2.3  2004/07/04 23:53:35  dgrisby
  More ValueType TypeCode and Any support.

  Revision 1.1.2.2  2003/11/06 11:56:55  dgrisby
  Yet more valuetype. Plain valuetype and abstract valuetype are now working.

  Revision 1.1.2.1  2003/09/26 16:12:53  dgrisby
  Start of valuetype support.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////// ValueBase ///////////////////////////////
//////////////////////////////////////////////////////////////////////

class ValueBase_var;

class ValueBase {
public:

  ////////////////////////
  // Standard interface //
  ////////////////////////

  typedef ValueBase_var _var_type;

  virtual void       _add_ref()        = 0;
  virtual void       _remove_ref()     = 0;
  virtual ValueBase* _copy_value()     = 0;
  virtual ULong      _refcount_value() = 0;

  static  ValueBase* _downcast(ValueBase*);


  //////////////////////
  // omniORB internal //
  //////////////////////

  virtual const char* _NP_repositoryId() const            = 0;
  virtual const char* _NP_repositoryId(ULong& hash) const = 0;
  // Most derived repoId of value, and hash value for it.

  virtual const _omni_ValueIds* _NP_truncatableIds() const = 0;
  // If the value is truncatable, return a pointer to the truncatable
  // id struct. If not truncatable, return 0.

  virtual _CORBA_Boolean _NP_custom() const = 0;
  // Return true if value is custom marshalled, false otherwise.

  virtual _CORBA_Boolean _NP_box() const;
  // Return true if the value is a valuebox; false otherwise.

  static inline _CORBA_Boolean _PR_is_valid(ValueBase* p) {
    return p ? (p->_pd_magic == _PR_magic) : 1;
  }

  virtual void* _ptrToValue(const char* repoId) = 0;

  static _dyn_attr const char* _PD_repoId;
  static _dyn_attr const ULong _PR_magic;

  static void _NP_marshal(ValueBase* v, cdrStream& s);
  // Marshal value specified in IDL as ValueBase. Marshals the type
  // information then calls _PR_marshal_state().

  static ValueBase* _NP_unmarshal(cdrStream& s);
  // Unmarshal a base value in the situation that the IDL specifies
  // the type as ValueBase. Unmarshals the type information, creates a
  // suitable value instance, calls the instance's _PR_unmarshal_state()
  // member function, then truncates any remaining data if necessary.

  virtual void _PR_marshal_state(cdrStream& s) const = 0;
  // Marshal the state members.

  virtual void _PR_unmarshal_state(cdrStream& s) = 0;
  // Unmarshal the state members.

protected:
  ValueBase();
  ValueBase(const ValueBase&);
  virtual ~ValueBase();

private:
  void operator=(const ValueBase&);
  // Not implemented.

  ULong _pd_magic;
};

class ValueBase_Helper {
public:
  static inline void add_ref(ValueBase* v) {
    if (v) v->_add_ref();
  }
  static inline void remove_ref(ValueBase* v) {
    if (v) v->_remove_ref();
  }
  static inline void marshal(ValueBase* v, cdrStream& s) {
    ValueBase::_NP_marshal(v, s);
  }
  static inline ValueBase* unmarshal(cdrStream& s) {
    return ValueBase::_NP_unmarshal(s);
  }
};

_CORBA_MODULE_FN inline void add_ref(ValueBase* vb) {
  if (vb != 0) vb->_add_ref();
}

_CORBA_MODULE_FN inline void remove_ref(ValueBase* vb) {
  if (vb != 0) vb->_remove_ref();
}


//////////////////////////////////////////////////////////////////////
///////////////////// DefaultValueRefCountBase ///////////////////////
//////////////////////////////////////////////////////////////////////

class DefaultValueRefCountBase : public virtual ValueBase {
public:
  virtual void  _add_ref();
  virtual void  _remove_ref();
  virtual ULong _refcount_value();

protected:
  inline DefaultValueRefCountBase() : _pd__refCount(1) {}
  virtual ~DefaultValueRefCountBase();

  inline DefaultValueRefCountBase(const DefaultValueRefCountBase& _v)
    : ValueBase(_v), _pd__refCount(1) {}

private:
  void operator=(const DefaultValueRefCountBase&);
  // Not implemented.

  ULong _pd__refCount;
};


//////////////////////////////////////////////////////////////////////
///////////////////// ValueFactoryBase ///////////////////////////////
//////////////////////////////////////////////////////////////////////

class ValueFactoryBase;
typedef ValueFactoryBase* ValueFactory;

class ValueFactoryBase {
public:
  virtual ~ValueFactoryBase();
  virtual void _add_ref();
  virtual void _remove_ref();
  static ValueFactory _downcast(ValueFactory vf);

protected:
  ValueFactoryBase();

private:
  virtual ValueBase* create_for_unmarshal() = 0;
  ULong _pd_refCount;

  //////////////////////
  // omniORB internal //
  //////////////////////

public:
  friend class ::_omni_ValueFactoryManager;

  virtual void* _ptrToFactory(const char* repoId);
};
