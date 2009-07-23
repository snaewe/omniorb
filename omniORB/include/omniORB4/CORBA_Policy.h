// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Policy.h             Created on: 2001/08/17
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
//    CORBA::Policy
//

/*
  $Log$
  Revision 1.1.4.2  2005/01/06 23:08:07  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:04:23  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.4  2003/01/14 11:48:15  dgrisby
  Remove warnings from gcc -Wshadow. Thanks Pablo Mejia.

  Revision 1.1.2.3  2002/01/02 18:13:43  dpg1
  Platform fixes/additions.

  Revision 1.1.2.2  2001/11/12 13:47:09  dpg1
  Minor fixes.

  Revision 1.1.2.1  2001/08/17 13:39:47  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////////////// Policy ///////////////////////////////
//////////////////////////////////////////////////////////////////////

typedef ULong PolicyType;

class Policy;
typedef Policy* Policy_ptr;
typedef Policy_ptr PolicyRef;

class Policy : public virtual Object {
public:
  virtual ~Policy();

  virtual PolicyType policy_type();
  virtual Policy_ptr copy();
  // This at least needs to be overridden in subclasses.
  virtual void destroy();
  // May throw NO_PERMISSION.

  static Policy_ptr _duplicate(Policy_ptr);
  static Policy_ptr _narrow(Object_ptr);
  static Policy_ptr _nil();

  // omniORB internal.

  Policy(PolicyType type);
  Policy(); // nil policy

  virtual void* _ptrToObjRef(const char* repoId);
  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Override Object.

  static _core_attr const char* _PD_repoId;

private:
  Policy(const Policy&);
  Policy& operator = (const Policy&);

  int pd_refCount;
  PolicyType pd_type;
};

typedef _CORBA_PseudoObj_Var<Policy> Policy_var;
typedef _CORBA_PseudoObj_Member<Policy, Policy_var> Policy_member;

typedef _CORBA_Pseudo_Unbounded_Sequence<Policy, Policy_member> PolicyList;

typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_ULong,4,4> PolicyTypeSeq;

class PolicyList_out;

class PolicyList_var {
public:
  inline PolicyList_var() : _pd_seq(0) {}
  inline PolicyList_var(PolicyList* _s) : _pd_seq(_s) {}
  inline PolicyList_var(const PolicyList_var& _s) {
    if( _s._pd_seq )  _pd_seq = new PolicyList(*_s._pd_seq);
    else              _pd_seq = 0;
  }
  inline ~PolicyList_var() { if( _pd_seq )  delete _pd_seq; }
    
  inline PolicyList_var& operator = (PolicyList* _s) {
    if( _pd_seq )  delete _pd_seq;
    _pd_seq = _s;
    return *this;
  }
  inline PolicyList_var& operator = (const PolicyList_var& _s) {
    if( _s._pd_seq ) {
      if( !_pd_seq )  _pd_seq = new PolicyList;
      *_pd_seq = *_s._pd_seq;
    } else if( _pd_seq ) {
      delete _pd_seq;
      _pd_seq = 0;
    }
    return *this;
  }
  inline Policy_member& operator [] (_CORBA_ULong _s) {
    return (*_pd_seq)[_s];
  }
  inline PolicyList* operator -> () { return _pd_seq; }
#if defined(__GNUG__)
  inline operator PolicyList& () const { return *_pd_seq; }
#else
  inline operator const PolicyList& () const { return *_pd_seq; }
  inline operator PolicyList& () { return *_pd_seq; }
#endif
    
  inline const PolicyList& in() const { return *_pd_seq; }
  inline PolicyList&       inout()    { return *_pd_seq; }
  inline PolicyList*&      out() {
    if( _pd_seq ) { delete _pd_seq; _pd_seq = 0; }
    return _pd_seq;
  }
  inline PolicyList* _retn() { PolicyList* tmp = _pd_seq; _pd_seq = 0; return tmp; }
    
  friend class PolicyList_out;
  
private:
  PolicyList* _pd_seq;
};

class PolicyList_out {
public:
  inline PolicyList_out(PolicyList*& _s) : _data(_s) { _data = 0; }
  inline PolicyList_out(PolicyList_var& _s)
    : _data(_s._pd_seq) { _s = (PolicyList*) 0; }
  inline PolicyList_out(const PolicyList_out& _s) : _data(_s._data) {}
  inline PolicyList_out& operator = (const PolicyList_out& _s) {
    _data = _s._data;
    return *this;
  }
  inline PolicyList_out& operator = (PolicyList* _s) {
    _data = _s;
    return *this;
  }
  inline operator PolicyList*&()  { return _data; }
  inline PolicyList*& ptr()       { return _data; }
  inline PolicyList* operator->() { return _data; }

  inline Policy* operator [] (_CORBA_ULong _i) {
    return (*_data)[_i];
  }

  PolicyList*& _data;

private:
  PolicyList_out();
  PolicyList_out& operator=(const PolicyList_var&);
};
  



#ifdef OMNIORB_DECLARE_POLICY_OBJECT
#error OMNIORB_DECLARE_POLICY_OBJECT is already defined!
#endif
#define OMNIORB_DECLARE_POLICY_OBJECT(name, type)  \
  class name;  \
  typedef name* name##_ptr;  \
  typedef name##_ptr name##Ref;  \
  \
  class name : public CORBA::Policy  \
  {  \
  public:  \
    inline name(name##Value value_) :  \
      CORBA::Policy(type), pd_value(value_) {}  \
    inline name() {}  \
    virtual ~name();  \
    \
    virtual CORBA::Policy_ptr copy();  \
    virtual name##Value value() { return pd_value; }  \
    \
    virtual void* _ptrToObjRef(const char* repoId);  \
    \
    static name##_ptr _duplicate(name##_ptr p);  \
    static name##_ptr _narrow(CORBA::Object_ptr p);  \
    static name##_ptr _nil();  \
    \
    static _core_attr const char* _PD_repoId;  \
    \
  private:  \
    name##Value pd_value;  \
  }; \
  \
  typedef _CORBA_PseudoObj_Var<name> name##_var;

#ifdef OMNIORB_DECLARE_POLICY_OBJECT_OPERATORS
#error OMNIORB_DECLARE_POLICY_OBJECT_OPERATORS is already defined!
#endif

#define OMNIORB_POLICY_VALUE(policy) policy##Value

#define OMNIORB_DECLARE_POLICY_OBJECT_OPERATORS(name) \
void operator<<=(CORBA::Any &, name); \
CORBA::Boolean operator>>=(const CORBA::Any&, name& );
