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
typedef PolicyList* PolicyList_var;  //??

typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_ULong,4,4> PolicyTypeSeq;

  

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
    inline name(name##Value value) : CORBA::Policy(type), pd_value(value) {}  \
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
#define OMNIORB_DECLARE_POLICY_OBJECT_OPERATORS(name) \
void operator<<=(CORBA::Any&, name##Value); \
CORBA::Boolean operator>>=(const CORBA::Any&,name##Value& );
