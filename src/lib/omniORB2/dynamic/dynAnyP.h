// -*- Mode: C++; -*-
//                            Package   : omniORB2
// DynAnyP.h                  Created on: 12/02/98
//                            Author    : Sai-Lai Lo (sll)
//
//
//    Copyright (C) 1998 Olivetti & Oracle Research Laboratory
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
//      Private class DynAnyP for the implementation of the DynAny interface
//

#ifndef __DYNANYP_H__
#define __DYNANYP_H__

class dynAnyP;

class dynAnyP {
public:
  static omni_mutex lock;
  static omni_mutex refCountLock;

  dynAnyP(CORBA::TypeCode_ptr tc);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock if parent != 0
  //
  // Ctor.
  //
  // If the typeCode argument is a tk_alias, the alias is expanded to its
  // real typecode. The expanded typecode is returned by tc().

  ~dynAnyP();

  void fromAny(const CORBA::Any& v);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Use the Any value contained in the argument v to (re)initialise this node.
  // Through DynAny::Invalid() if the argument contain invalid data, such
  // as the wrong typecode.

  void toAny(CORBA::Any& v);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Return the Any value of this node.

  void incrRefCount();
  // Concurrency Control:
  //   dynAnyP::refCountLock
  //
  // Caller *must not* hold mutex dynAnyP::refCountlock
  //
  // Increment the reference count of this node.


  void decrRefCount(CORBA::DynAny_ptr);
  // Concurrency Control:
  //   dynAnyP::refCountLock
  //
  //   Caller *must not* hold mutex dynAnyP::refCountlock
  //
  // If pd_refcount > 0
  //    pd_refcount--;
  // If pd_refcount  == 0 && pd_is_root != 0
  //    This is the root node and there is no more reference to this
  //    node. Call the dispose() method.
  //    The dispose() method call the delete operator on this instance 
  //    as well as on the DynAny_ptr argument.
  //    The DynAny_ptr argument should be the DynAny instance that contains
  //    this instance.
  // 
  // NOTE: If pd_is_root == 0 
  //          This is not the root node. dispose() is not called. This
  //          node will be deleted when dispose() is called on the root node
  //          which contains this node as part of its tree.
  //
  // NOTE: This should be the only method that to cause a DynAny instance to 
  //       be deleted. Never call the delete operator directly.
  //

  inline CORBA::TypeCode_ptr tc() const { return pd_tc; }
  // Return the TypeCode_ptr of this node.
  // The storage of the reference still belongs to this node.
  // In other words, the caller should not call _release() on the
  // reference.

  // Each node has a current component ptr.
  // When this node is first initialised. The current component ptr
  // points to the 0th component.
  //
  // The nth component for
  //   - a DnyAny is the only element (n is always 0)
  //   - a DynEnum is the only element (n is always 0) 
  //   - a DynSequence is the nth element of the sequence
  //   - a DynArray is the nth element of the array
  //   - a DynStruct is the nth member of the structure
  //   - a DynUnion is the discriminant if n==0 and
  //                   the member if n = 1
  //
  // The following member functions manipulate the current component
  // ptr and the component it points to.

  inline CORBA::ULong totalComponents() const
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Return the current size of the component list.
  {
    return (CORBA::ULong) pd_total_components;
  }

  void totalComponents(CORBA::ULong v);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Set the size of the component list to <v>. For DynAny types that
  // contain fixed size component list, calling this function would
  // result in a omniORB::fatalException.

  CORBA::ULong MaxComponents() const;
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Return the maximum size of the component list.
  // If this is an unbounded sequence, return 0.

  inline int currentComponentIndex()
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Return the index value of the current component ptr. If the current
  // component ptr is not valid (it has been moved beyond the end of the
  // list), -1 is returned.
  {
    return pd_curr_index;
  }

  CORBA::Boolean nextComponent();
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Advance the current component ptr to the next component
  // Return 0 if the end of the component list has been reached.
  //             the current component ptr becomes invalid.
  //             Any subsequent call to currentComponent(),
  //             currentComponentTC() would
  //             result in a omniORB::fatalException if the ptr
  //             is not reset by nthComponent().

  CORBA::Boolean nthComponent(CORBA::ULong index);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Reset the current component to the <index>th component
  // Return 0 if the index is not within the valid range and
  //             the current component ptr is not changed.

  CORBA::DynAny_ptr     currentComponent();
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Return a DynAny_ptr for the current component.
  // When the caller finishes with the returned value, _release() should
  // be called on the reference.
  // return a nil DynAny if the current component ptr is not valid.

  void currentComponentToAny(CORBA::Any&);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Return the current component as an any

  void currentComponentFromAny(CORBA::Any&);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Set the current component to the value of the any argument

  inline CORBA::TypeCode_ptr   currentComponentTC()
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Return the TypeCode_ptr of the current component.
  // The storage of the TypeCode_ptr still belongs to this node.
  // In other words, the caller should not call _release() on the reference.
  // Return nil typecode if the current component ptr is not valid.
  {
    return nthComponentTC(currentComponentIndex());
  }

  inline CORBA::TCKind currentComponentTCKind()
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Return the TCKind of the current component.
  // Return CORBA::tk_null if the current component pointer is not valid.
  {
    CORBA::TypeCode_ptr tc = nthComponentTC(currentComponentIndex());
    if (CORBA::is_nil(tc))
      return CORBA::tk_null;
    else
      return tc->kind();
  }

  union Bval {
    CORBA::Boolean      bv;
    CORBA::Octet        ov;
    CORBA::Char         cv;
    CORBA::Short        sv;
    CORBA::UShort       usv;
    CORBA::Long         lv;
    CORBA::ULong        ulv;
#ifndef NO_FLOAT
#ifndef USING_PROXY_FLOAT
    CORBA::Float        fv;
    CORBA::Double       dv;
#else
    float		fv;
    double		dv;
#endif
#endif
    const char*         stiv;
    char*               stov;
    CORBA::Object_ptr   objv;
    CORBA::TypeCode_ptr tcv;
    CORBA::Any*         anyv;
  };


  void currentComponentFromBasetype(CORBA::TCKind,Bval&);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Insert the basetype argument into the current component.
  // If the type is tk_enum, the value is passed as Bval.ulv.
  // Throw CORBA::DynAny::InvalidValue if the argument does not
  // match the component type.

  void currentComponentToBasetype(CORBA::TCKind,Bval&);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Extract the value of the current component into the argument.
  // If the type is tk_enum, the value is passed as Bval.ulv.
  // Throw CORBA::DynAny::TypeMismatch() if the argument does not
  // match the component type.
  // For string, object reference, typecode and any, this method
  // heap allocate the return value.

  void createComponent(CORBA::ULong index,CORBA::Boolean replace=0,
		       CORBA::TypeCode_ptr newtc=0);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Create a DynAny for the <index>th component in pd_components.
  // Its value is copied into the newly created component.
  //
  // If <index>th component already exists and is non-nil, then
  //    If <replace> == 0, just return.
  //    If <replace> == 1, detach the current component and create a new
  //                       one.
  //
  // If <replace>==1 && the node is a tk_union && <index> == 1,
  //    <newtc> contains the typecode of the new member. This node now owns
  //    the storage of <newtc>.
  // Otherwise, the argument <newtc> is ignored.
  //
  // If there is a value for the component in the buffer, copy it into
  // the newly created DynAny node.

private:

  void dispose(CORBA::DynAny_ptr dp);
  // Concurrency Control:
  //    dynAnyP::lock
  //
  //    Caller *must not* hold mutex dynAnyP::lock.
  //
  // 1. Iterate through all the children in pd_components
  //       If pd_refcount of a node == 0
  //          There is no external reference to this node. Call dispose().
  //       else
  //          There are external reference to this node, change pd_flags.root
  //          of this node to 1 so that it become a root node.
  // 2. Call the delete operator on this node
  // 3. Call the delete operator on <dp> 


  CORBA::TypeCode_ptr                             pd_tc;
  _CORBA_Unbounded_Sequence<CORBA::DynAny_member> pd_components;
  MemBufferedStream                               pd_buf;
  int	                                          pd_refcount;
  CORBA::Boolean                                  pd_is_root;
  // pd_is_root = 1 if this is the root node. Only when dispose() is called 
  //                on a root node would cause the node and its children to 
  //                be deleted.


  // TypeCode of component(s).
  //   For predefined types, enum, sequence and array the typecode is
  //   stored in val. Otherwise the typecode is storied in seq indexed by
  //   the position of the member.
  union {
    CORBA::TypeCode_ptr                                val;
    _CORBA_Unbounded_Sequence<CORBA::TypeCode_member>* seq;
  } pd_componentTC;


  int                                            pd_total_components;
  int                                            pd_curr_startpos;
  int                                            pd_curr_index;
  int                                            pd_last_index;


  enum { Invalid, 
	 Ready, 
	 ReadInProgress, 
	 ReadCompleted,
	 WriteInProgress,
	 WriteCompleted } pd_state;

  // beginReadComponent(), endReadComponent()
  // beginWriteComponent(), endWriteComponent()
  //
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //

  CORBA::Boolean      beginReadComponent();
  // Call this function to make sure that the buffer is properly adjusted
  // so that the value of the current component can be unmarshalled from
  // the buffer.
  //
  // Return 1 if the current component can be read from the buffer.
  // Return 0 if otherwise. In this case the only other place to read
  // the value is from pd_components.
  //
  // If the return value is 1, the value can be unmarshalled from the
  // buffer.  If the value is only partially read from the buffer when an
  // exception is raised during data unmarshalling, the next call to
  // beginReadComponent(), beginWriteComponent(), nextComponent() would
  // restore the buffer to a consistent state.

  void                endReadComponent();
  // Call this function to indicate that the value of the current
  // component has been unmarshalled from the buffer.
  //
  // This function should only be called in pair with beginReadComponent().

  CORBA::Boolean      beginWriteComponent();
  // Call this function to make sure that the buffer is properly adjusted
  // so that the value of the current component can be marshalled into
  // the buffer.
  //
  // Return 1 if the current component can be written into the buffer.
  // Return 0 if otherwise. In this case the only other place to write
  // the value is into pd_components.
  //
  // Data can only be appended to the end of the buffer. In other words,
  // this function only returns 1 if the current component is the one that
  // follows the last component in the buffer. If the return value is 1,
  // the component value can be marshalled into the buffer. If the value
  // is only partially written when an exception is raised during data
  // marshalling, the next call to beginReadComponent(), beginWriteComponent()
  // and nextComponent() would restore the buffer to a consistent state.
  //

  void                endWriteComponent();
  // Call this function to indicate that the value of the current component
  // has been marshalled into the buffer.
  //
  // This function should only be called in pair with endWriteComponent().

  inline CORBA::ULong nComponentInBuffer() 
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Return the values of how many components are stored in the buffer
  { return pd_last_index + 1; }


  CORBA::TypeCode_ptr   nthComponentTC(CORBA::ULong index);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Return the typcode of the <index>th component.
  // Return nil typecode if the index is not within the valid range.
  // The storage of the TypeCode_ptr still belongs to this node.
  // In other words, the caller should not call _release() on the reference.
  //
  // The returned typecode would not be a tk_alias. Any tk_alias typecode
  // is expanded internally.

  void toStream(MemBufferedStream& v);
  // Concurrency Control:
  //   Caller must hold mutex dynAnyP::lock
  //
  // Marshal the value of this node into v. 


  dynAnyP();
  dynAnyP(const dynAnyP&);
  dynAnyP& operator=(const dynAnyP&);
};


// Private classes to implement the DynAny_ptr given out as discriminator
// of DynUnion. Override all the insert methods so that any change in the
// discriminator value would be intercepted and if necessary the member
// DynAny_ptr updated.
//
class dynAnyUD1 : public CORBA::DynAny {
public:
  void NP_real_from_any(const CORBA::Any& value);
  void insert_boolean(CORBA::Boolean value);
  void insert_octet(CORBA::Octet value);
  void insert_char(CORBA::Char value);
  void insert_short(CORBA::Short value);
  void insert_ushort(CORBA::UShort value);
  void insert_long(CORBA::Long value);
  void insert_ulong(CORBA::ULong value);

  void set_member(int index,CORBA::Boolean update_value=1);
  // Concurrency Control:
  //    Caller must hold mutex dynAnyP::lock.
  //
  // select the member. Use the position of the member type in
  // the union typecode to identify the member.
  // If index == -1, the implicit default member is selected. The union
  //                 would compose solely of the discriminator value.
  // If update_value == 1, insert the label value for this member into
  //                       the discriminator.
  
  int get_member() { return pd_member_index; }
  // Concurrency Control:
  //    Caller must hold mutex dynAnyP::lock.
  //
  // get current selected member. Return the position of the member type
  // in the union typecode. Return -1 if the discriminator has not been
  // initialised or the implicit default member is selected.

  CORBA::Boolean implicit_default_member() const { 
    return pd_implicit_default;
  }
  // Concurrency Control:
  //    Caller must hold mutex dynAnyP::lock.
  //
  // Return 1 if the implicit default member has been selected.

  dynAnyUD1(void* p, dynAnyP* parent):CORBA::DynAny(p,CORBA::DynAny::dt_any),
            			      pd_parent(parent), pd_member_index(-1),
                                      pd_implicit_default(0)
  {}

  ~dynAnyUD1() {}

private:

  dynAnyP*      pd_parent;
  int           pd_member_index;
  CORBA::Boolean pd_implicit_default;

  dynAnyUD1();
  dynAnyUD1(const dynAnyUD1&);
  dynAnyUD1& operator=(const dynAnyUD1&);
};

class dynAnyUD2 : public CORBA::DynEnum {
public:
  void NP_real_from_any(const CORBA::Any& value);
#if !defined(_MSC_VER)
  char* value_as_string() {
    return CORBA::DynEnum::value_as_string();
  }
  CORBA::ULong value_as_ulong() { 
    return CORBA::DynEnum::value_as_ulong();
  }
#endif
  void value_as_string(const char* value);
  void value_as_ulong(CORBA::ULong value);

  void set_member(int index,CORBA::Boolean update_value=1);
  // Concurrency Control:
  //    Caller must hold mutex dynAnyP::lock.
  //
  // select the member. Use the position of the member type in
  // the union typecode to identify the member.
  // If index == -1, the implicit default member is selected. The union
  //                 would compose solely of the discriminator value.
  // If update_value == 1, insert the label value for this member into
  //                       the discriminator.
  
  int get_member() { return pd_member_index; }
  // Concurrency Control:
  //    Caller must hold mutex dynAnyP::lock.
  //
  // get current selected member. Return the position of the member type
  // in the union typecode. Return -1 if the discriminator has not been
  // initialised or the implicit default member is selected.

  CORBA::Boolean implicit_default_member() const {
    return pd_implicit_default;
  }
  // Concurrency Control:
  //    Caller must hold mutex dynAnyP::lock.
  //
  // Return 1 if the implicit default member has been selected.

  dynAnyUD2(void* p, dynAnyP* parent) : CORBA::DynEnum(p),
                                        pd_parent(parent),
                                        pd_member_index(-1),
                                        pd_implicit_default(0)
  {}
  ~dynAnyUD2() {}

private:

  dynAnyP* pd_parent;
  int pd_member_index;
  CORBA::Boolean pd_implicit_default;

  dynAnyUD2();
  dynAnyUD2(const dynAnyUD2&);
  dynAnyUD2& operator=(const dynAnyUD2&);
};

#endif // __DYNANYP_H__
