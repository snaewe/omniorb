// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_vartypes.h           Created on: 16/7/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//

/*
 $Log$
 Revision 1.3  2000/07/13 15:26:05  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.1.2.3  2000/06/27 16:15:08  sll
 New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
 _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
 sequence of string and a sequence of object reference.

 Revision 1.1.2.2  1999/10/13 12:44:35  djr
 Added definition of TypeCode_out.

 Revision 1.1.2.1  1999/09/24 09:51:39  djr
 Moved from omniORB2 + some new files.

*/

//////////////////////////////////////////////////////////////////////
///////////////////////////// Object_var /////////////////////////////
//////////////////////////////////////////////////////////////////////

class Object_Member;
class Object_Element;
class Object_INOUT_arg;
class Object_OUT_arg;
class Object_tcDesc_arg;

class Object_var {
public:
  typedef Object        T;
  typedef Object_ptr    T_ptr;
  typedef Object_var    T_var;
  typedef Object_Member T_member;
  typedef Object_Element T_element;

  inline Object_var() : pd_ref(T::_nil()) {}
  inline Object_var(T_ptr p) { pd_ref = p; }
  inline Object_var(const T_var& p) : pd_ref(T::_duplicate(p.pd_ref)) {}
  inline Object_var(const T_member& p);
  inline Object_var(const T_element& p);
  inline ~Object_var() { release(pd_ref); }

  inline T_var& operator= (T_ptr p) {
    release(pd_ref);
    pd_ref = p;
    return *this;
  }
  inline T_var& operator= (const T_var& p) {
    if( pd_ref != p.pd_ref ) {
      release(pd_ref);
      pd_ref = T::_duplicate(p.pd_ref);
    }
    return *this;
  }
  inline T_var& operator= (const T_member& p);

  inline T_var& operator= (const T_element& p);

  inline T_ptr operator->() const { return pd_ref; }
  inline operator T_ptr() const   { return pd_ref; }

  inline T_ptr in() const { return pd_ref; }
  inline T_ptr& inout()   { return pd_ref; }
  inline T_ptr& out() {
    if( !is_nil(pd_ref) ) {
      release(pd_ref);
      pd_ref = T::_nil();
    }
    return pd_ref;
  }
  inline T_ptr _retn() {
    T_ptr tmp = pd_ref;
    pd_ref = T::_nil();
    return tmp;
  }

  friend class Object_Member;
  friend class Object_INOUT_arg;
  friend class Object_OUT_arg;
  friend class Object_tcDesc_arg;

private:
  T_ptr pd_ref;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// Object_Member ///////////////////////////
//////////////////////////////////////////////////////////////////////

class Object_Member {
public:
  typedef Object        T;
  typedef Object_ptr    T_ptr;
  typedef Object_var    T_var;
  typedef Object_Member T_member;
  typedef Object_Element T_element;

  inline Object_Member() : _ptr(T::_nil()) {}
  inline Object_Member(const T_member& p) {
    _ptr = T::_duplicate(p._ptr);
  }
  inline ~Object_Member() {
    CORBA::release(_ptr);
  }

  inline T_member& operator= (T_ptr p) {
    CORBA::release(_ptr);
    _ptr = p;
    return *this;
  }
  inline T_member& operator= (const T_member& p) {
    if( &p != this ) {
      CORBA::release(_ptr);
      _ptr = T::_duplicate(p._ptr);
    }
    return *this;
  }

  inline T_member& operator= (const T_var& p) {
    CORBA::release(_ptr);
    _ptr = T::_duplicate(p);
    return *this;
  }

  inline T_member& operator= (const T_element& p);

  inline size_t _NP_alignedSize(size_t initialoffset) const {
    return CORBA::Object::_NP_alignedSize(_ptr,initialoffset);
  }
  inline void operator>>= (NetBufferedStream& s) const {
    CORBA::Object::_marshalObjRef(_ptr,s);
  }
  inline void operator<<= (NetBufferedStream& s) {
    Object_ptr _result = CORBA::Object::_unmarshalObjRef(s);
    CORBA::release(_ptr);
    _ptr = _result;
  }
  inline void operator>>= (MemBufferedStream& s) const {
    CORBA::Object::_marshalObjRef(_ptr,s);
  }
  inline void operator<<= (MemBufferedStream& s) {
    Object_ptr _result = CORBA::Object::_unmarshalObjRef(s);
    CORBA::release(_ptr);
    _ptr = _result;
  }

  inline Object_ptr operator->() const { return _ptr; }
  inline operator Object_ptr () const  { return _ptr; }

  inline Object_ptr  in() const { return _ptr; }
  inline Object_ptr& inout()    { return _ptr; }
  inline Object_ptr& out() {
    CORBA::release(_ptr);
    _ptr = CORBA::Object::_nil();
    return _ptr;
  }
  inline Object_ptr _retn() {
    Object_ptr tmp;
    tmp = _ptr;
    _ptr = CORBA::Object::_nil();
    return tmp;
  }

  Object_ptr _ptr;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// Object_Element //////////////////////////
//////////////////////////////////////////////////////////////////////

class Object_Element {
public:
  typedef Object        T;
  typedef Object_ptr    T_ptr;
  typedef Object_var    T_var;
  typedef Object_Member T_member;
  typedef Object_Element T_element;

  inline Object_Element(T_ptr& p, _CORBA_Boolean rel)
    : pd_rel(rel), pd_data(p) {}

  inline Object_Element(const T_element& p)
    : pd_rel(p.pd_rel), pd_data(p.pd_data) {}

  inline ~Object_Element() {
  // intentionally does nothing.
  }

  inline T_element& operator= (T_ptr p) {
    if( pd_rel )  CORBA::release(pd_data);
    pd_data = p;
    return *this;
  }

  inline T_element& operator= (const T_element& p) {
    if( pd_rel ) {
      CORBA::release(pd_data);
      T::_duplicate(p.pd_data);
      pd_data = p.pd_data;
    }
    else
      pd_data = p.pd_data;
    return *this;
  }

  inline T_element& operator= (const T_member& p) {
    if( pd_rel ) {
      CORBA::release(pd_data);
      T::_duplicate(p);
    }
    pd_data = (T_ptr) p;
    return *this;
  }

  inline T_element& operator= (const T_var& p) {
    if( pd_rel ) {
      CORBA::release(pd_data);
      T::_duplicate(p);
    }
    pd_data = (T_ptr)p;
    return *this;
  }

  inline T_ptr operator->() const { return pd_data; }
  inline operator T_ptr () const  { return pd_data; }

  inline T_ptr in() const { return pd_data; }
  inline T_ptr& inout()         { return pd_data; }
  inline T_ptr& out() {
    if (pd_rel) {
      CORBA::release(pd_data);
    }
    pd_data = T::_nil();
    return pd_data;
  }

  inline T_ptr _retn() {
    T_ptr tmp = pd_data;
    if (!pd_rel) { T::_duplicate(pd_data); }
    pd_data = T::_nil();
    return tmp;
  }

  inline T_ptr& _NP_ref() const {return pd_data;}
  inline _CORBA_Boolean _NP_release() const {return pd_rel;}

  _CORBA_Boolean pd_rel;
  T_ptr& pd_data;
};

//////////////////////////////////////////////////////////////////////
////////////////////////// Object_INOUT_arg //////////////////////////
//////////////////////////////////////////////////////////////////////

class Object_INOUT_arg {
public:
  inline Object_INOUT_arg(Object_ptr& p) : _data(p) {}
  inline Object_INOUT_arg(Object_var& p) : _data(p.pd_ref) {}
  inline Object_INOUT_arg(Object_Member& p) : _data(p._ptr) {}
  inline Object_INOUT_arg(Object_Element& p) : _data(p._NP_ref()) {
    // If the Object_Element has pd_rel == 0,
    // the ObjRef is not owned by the sequence and should not
    // be freed. Since this is an inout argument and the callee may call
    // release, we duplicate the ObjRef and pass it to the callee. This will
    // result in a memory leak! This only occurs when there is a programming
    // error and cannot be trapped by the compiler.
    if( !p._NP_release() )
      Object::_duplicate(p._NP_ref());
  }
  inline ~Object_INOUT_arg() {}

  inline operator Object_ptr&()  { return _data; }

  Object_ptr& _data;

private:
  Object_INOUT_arg();
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// Object_OUT_arg ///////////////////////////
//////////////////////////////////////////////////////////////////////

class Object_OUT_arg {
public:
  typedef Object        T;
  typedef Object_ptr    T_ptr;
  typedef Object_var    T_var;
  typedef Object_Member T_member;
  typedef Object_Element T_element;
  typedef Object_OUT_arg T_out;

  inline Object_OUT_arg(T_ptr& p) : _data(p) { _data = T::_nil(); }
  inline Object_OUT_arg(T_var& p) : _data(p.pd_ref) {
    p = T::_nil();
  }
  inline Object_OUT_arg(T_member& p) : _data(p._ptr) {
    p = T::_nil();
  }
  inline Object_OUT_arg(T_element& p) : _data(p._NP_ref()) {
    p = T::_nil();
  }
  inline Object_OUT_arg(const T_out& p) : _data(p._data) {}
  inline ~Object_OUT_arg() {}
 
  inline Object_OUT_arg& operator=(const T_out& p) { 
    _data = p._data; return *this;  }
  inline Object_OUT_arg& operator=(T_ptr p) { _data = p; return *this; }

  inline operator T_ptr&()  { return _data; }
  inline T_ptr& ptr()       { return _data; }
  inline T_ptr operator->() { return _data; }

  T_ptr& _data;

private:
  Object_OUT_arg();
  T_out& operator=(const T_member& p);
  T_out& operator=(const T_element& p);
  T_out& operator=(const T_var& p);
  // CORBA 2.3 p23-26 says that T_var assignment should be disallowed.
};


typedef Object_OUT_arg Object_out;

//////////////////////////////////////////////////////////////////////
/////////////////////// Object_tcDesc_arg        /////////////////////
//////////////////////////////////////////////////////////////////////

class Object_tcDesc_arg {
public:
  // The sole purpose of this class is for passing as an argument
  // to the tc build descriptor function of an interface.
  // It provides the necessary conversions from T_var, T_member,
  // T_element so that only one tc build description function is needed.
  // The alternative is to have 3 overloaded build description functions
  // which makes the stub code even more bloated.
  typedef Object_Member T_member;
  typedef Object_Element T_element;
  typedef Object_var     T_var;
  inline Object_tcDesc_arg(T_var& p) : _data(p.pd_ref), _rel(1) {}
  inline Object_tcDesc_arg(T_member& p) : _data(p._ptr), _rel(1) {}
  inline Object_tcDesc_arg(T_element p) 
    : _data(p._NP_ref()), _rel(p._NP_release()) {}
  inline ~Object_tcDesc_arg() {}

  Object_ptr& _data;
  _CORBA_Boolean _rel;

private:
  Object_tcDesc_arg();
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// TypeCode_var ////////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_INOUT_arg;
class TypeCode_OUT_arg;

class TypeCode_var {
public:
  typedef TypeCode_ptr T_ptr;
  typedef TypeCode_var T_var;
  typedef TypeCode_member T_member;

  inline TypeCode_var() : pd_ref(TypeCode::_nil()) {}
  inline TypeCode_var(T_ptr p) : pd_ref(p) {}
  inline TypeCode_var(const T_var& p) { 
    pd_ref = TypeCode::_duplicate(p.pd_ref);
  }
  TypeCode_var(const TypeCode_member& p) {
    pd_ref = TypeCode::_duplicate(p._ptr);
  }
  ~TypeCode_var() { release(pd_ref); }

  inline T_var& operator= (T_ptr p) {
    release(pd_ref);
    pd_ref = p;
    return *this;
  }

  inline T_var& operator= (const T_var& p) {
    if( pd_ref != p.pd_ref ) {
      release(pd_ref);
      pd_ref = TypeCode::_duplicate(p.pd_ref);
    }
    return *this;
  }

  inline T_var& operator=(const TypeCode_member& p) {
    release(pd_ref);
    pd_ref = TypeCode::_duplicate(p._ptr);
    return *this;
  }

  T_ptr operator->() const { return pd_ref; }
  operator T_ptr () const  { return pd_ref; }

  T_ptr  in() const   { return pd_ref; }
  T_ptr& inout()      { return pd_ref; }
  T_ptr& out() {
    release(pd_ref);
    pd_ref = TypeCode::_nil();
    return pd_ref;
  }
  T_ptr _retn() {
    T_ptr tmp = pd_ref;
    pd_ref = TypeCode::_nil();
    return tmp;
  }

  friend class TypeCode_member;
  friend class TypeCode_INOUT_arg;
  friend class TypeCode_OUT_arg;

private:
  T_ptr pd_ref;
};

//////////////////////////////////////////////////////////////////////
///////////////////////// TypeCode_INOUT_arg /////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_INOUT_arg {
public:
  inline TypeCode_INOUT_arg(TypeCode_ptr& p) : _data(p) { }
  inline TypeCode_INOUT_arg(TypeCode_var& p) : _data(p.pd_ref) { }
  inline TypeCode_INOUT_arg(TypeCode_member& p) : _data(p._ptr) { }
  inline ~TypeCode_INOUT_arg() {}

  inline operator TypeCode_ptr&()  { return _data; }

  TypeCode_ptr& _data;

private:
  TypeCode_INOUT_arg();
};

//////////////////////////////////////////////////////////////////////
////////////////////////// TypeCode_OUT_arg //////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_OUT_arg {
public:
  typedef TypeCode        T;
  typedef TypeCode_ptr    T_ptr;
  typedef TypeCode_var    T_var;
  typedef TypeCode_member T_member;

  TypeCode_OUT_arg(T_ptr& p) : _data(p) { _data = T::_nil(); }
  TypeCode_OUT_arg(T_var& p) : _data(p.pd_ref) {
    p = T::_nil();
  }
  inline TypeCode_OUT_arg(T_member& p) : _data(p._ptr) {
    p = T::_nil();
  }
  inline ~TypeCode_OUT_arg() {}

  inline TypeCode_OUT_arg& operator=(T_ptr p) { _data = p; return *this; }
  inline TypeCode_OUT_arg& operator=(const T_member& p) {
    _data = T::_duplicate(p); return *this;
  }
  inline operator T_ptr&()  { return _data; }
  inline T_ptr& ptr()       { return _data; }
  inline T_ptr operator->() { return _data; }

  T_ptr& _data;

private:
  TypeCode_OUT_arg();
};


typedef TypeCode_OUT_arg TypeCode_out;

//////////////////////////////////////////////////////////////////////
/////////////////////////////// ORB_var //////////////////////////////
//////////////////////////////////////////////////////////////////////

class ORB_var {
public:
  typedef ORB     T;
  typedef ORB_ptr T_ptr;
  typedef ORB_var T_var;

  inline ORB_var() : pd_ref(T::_nil()) {}
  inline ORB_var(T_ptr p) : pd_ref(p) {}
  inline ORB_var(const T_var& p) : pd_ref(T::_duplicate(p.pd_ref)) {}
  inline ~ORB_var() { release(pd_ref); }

  inline T_var& operator= (T_ptr p) {
    release(pd_ref);
    pd_ref = p;
    return *this;
  }
  inline T_var& operator= (const T_var& p) {
    if( pd_ref != p.pd_ref ) {
      release(pd_ref);
      pd_ref = T::_duplicate(p.pd_ref);
    }
    return *this;
  }

  inline T_ptr operator->() const { return pd_ref; }
  inline operator T_ptr() const   { return pd_ref; }

  inline T_ptr in() const { return pd_ref; }
  inline T_ptr& inout()   { return pd_ref; }
  inline T_ptr& out() {
    if( !is_nil(pd_ref) ) {
      release(pd_ref);
      pd_ref = T::_nil();
    }
    return pd_ref;
  }
  inline T_ptr _retn() {
    T_ptr tmp = pd_ref;
    pd_ref = T::_nil();
    return tmp;
  }

private:
  T_ptr pd_ref;
};

//////////////////////////////////////////////////////////////////////
/////////////////////////////// BOA_var //////////////////////////////
//////////////////////////////////////////////////////////////////////

class BOA_var {
public:
  typedef BOA     T;
  typedef BOA_ptr T_ptr;
  typedef BOA_var T_var;

  inline BOA_var() : pd_ref(T::_nil()) {}
  inline BOA_var(T_ptr p) : pd_ref(p) {}
  inline BOA_var(const T_var& p) : pd_ref(T::_duplicate(p.pd_ref)) {}
  inline ~BOA_var() { release(pd_ref); }

  inline T_var& operator= (T_ptr p) {
    release(pd_ref);
    pd_ref = p;
    return *this;
  }
  inline T_var& operator= (const T_var& p) {
    if( pd_ref != p.pd_ref ) {
      release(pd_ref);
      pd_ref = T::_duplicate(p.pd_ref);
    }
    return *this;
  }

  inline T_ptr operator->() const { return pd_ref; }
  inline operator T_ptr() const   { return pd_ref; }

  inline T_ptr in() const { return pd_ref; }
  inline T_ptr& inout()   { return pd_ref; }
  inline T_ptr& out() {
    if( !is_nil(pd_ref) ) {
      release(pd_ref);
      pd_ref = T::_nil();
    }
    return pd_ref;
  }
  inline T_ptr _retn() {
    T_ptr tmp = pd_ref;
    pd_ref = T::_nil();
    return tmp;
  }

private:
  T_ptr pd_ref;
};

//////////////////////////////////////////////////////////////////////
/////////////////////////////// DynAny ///////////////////////////////
//////////////////////////////////////////////////////////////////////

// DynAny interface _var types.
typedef _CORBA_PseudoObj_Var<DynAny>      DynAny_var;
typedef _CORBA_PseudoObj_Var<DynEnum>     DynEnum_var;
typedef _CORBA_PseudoObj_Var<DynStruct>   DynStruct_var;
typedef _CORBA_PseudoObj_Var<DynUnion>    DynUnion_var;
typedef _CORBA_PseudoObj_Var<DynSequence> DynSequence_var;
typedef _CORBA_PseudoObj_Var<DynArray>    DynArray_var;

// DynAny interface _member types.
typedef _CORBA_PseudoObj_Member<DynAny,DynAny_var>       DynAny_member;
typedef _CORBA_PseudoObj_Member<DynEnum,DynEnum_var>     DynEnum_member;
typedef _CORBA_PseudoObj_Member<DynStruct,DynStruct_var> DynStruct_member;
typedef _CORBA_PseudoObj_Member<DynUnion,DynUnion_var>   DynUnion_member;
typedef _CORBA_PseudoObj_Member<DynSequence,DynSequence_var>
                                                         DynSequence_member;
typedef _CORBA_PseudoObj_Member<DynArray,DynArray_var>   DynArray_member;

// DynAny interface _out types.
typedef _CORBA_PseudoObj_Out<DynAny,DynAny_var>           DynAny_out;
typedef _CORBA_PseudoObj_Out<DynEnum,DynEnum_var>         DynEnum_out;
typedef _CORBA_PseudoObj_Out<DynStruct,DynStruct_var>     DynStruct_out;
typedef _CORBA_PseudoObj_Out<DynUnion,DynUnion_var>       DynUnion_out;
typedef _CORBA_PseudoObj_Out<DynSequence,DynSequence_var> DynSequence_out;
typedef _CORBA_PseudoObj_Out<DynArray,DynArray_var>       DynArray_out;
