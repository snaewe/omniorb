// -*- Mode: C++; -*-
//                            Package   : omniORB2
// dynAny.h                   Created on: 11/1998
//                            Author    : David Riddoch (djr)
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
//   Implementation of CORBA::DynAny.
//

/*
  $Log$
  Revision 1.2.6.3  1999/10/26 20:02:50  sll
  DynAny no longer do alias expansion on the typecode. In other words, all
  aliases in the typecode are preserved.

  Revision 1.2.6.2  1999/09/25 17:00:17  sll
  Merged changes from omni2_8_develop branch.

  Revision 1.2.4.1  1999/09/22 16:38:26  djr
  Removed MT locking for 'DynAny's.
  New methods DynUnionImpl::NP_disc_value() and NP_disc_index().

  Revision 1.2.6.1  1999/09/15 20:18:24  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

*/

#ifndef __DYNANY_H__
#define __DYNANY_H__

#include <typecode.h>
#include <omniutilities.h>


// Node type constants.
#define dt_any        0
#define dt_enum       1
#define dt_fixed      2
#define dt_struct     3
#define dt_union      4
#define dt_seq        5
#define dt_array      6
#define dt_disc       7
#define dt_enumdisc   8


// Forward declarations.
class DynAnyImplBase;
class DynAnyImpl;
class DynEnumImpl;
class DynFixedImpl;
class DynStructImpl;
class DynUnionImpl;
class DynSequenceImpl;
class DynArrayImpl;


inline DynAnyImpl* ToDynAnyImpl(CORBA::DynAny_ptr p);
inline DynAnyImpl* ToDynAnyImpl(DynAnyImplBase* p);


// Values used to indicate whether a DynAny is the child of
// another DynAny, or the root.
#define DYNANY_CHILD  0
#define DYNANY_ROOT   1


//////////////////////////////////////////////////////////////////////
/////////////////////////// DynAnyImplBase ///////////////////////////
//////////////////////////////////////////////////////////////////////

//: Functionnality common to all DynAny implementations.

class DynAnyImplBase : public virtual CORBA::DynAny
{
public:
  DynAnyImplBase(TypeCode_base* tc, int nodetype, CORBA::Boolean is_root)
    : pd_tc(tc), pd_refcount(1), pd_is_root(is_root) {}
  // <tc> must be an alias-expanded TypeCode, and is consumed.

  virtual ~DynAnyImplBase();

  /*******************
  * public interface *
  *******************/
  virtual CORBA::TypeCode_ptr type() const;
  virtual void destroy();
  virtual void from_any(const CORBA::Any& value);
  virtual CORBA::Any* to_any();

  /****************************
  * exposed private interface *
  ****************************/
  virtual CORBA::Boolean NP_is_nil() const;

//protected:

  /***********
  * internal *
  ***********/

  virtual int copy_to(cdrMemoryStream& mbs) = 0;
  // Copies our value into the given stream. Does not flush <mbs>,
  // but does rewind our buffer first. Returns 0 if we are not
  // properly initialised.
  //  Concurrency: hold DynAnyImplBase::lock

  virtual int copy_from(cdrMemoryStream& mbs) = 0;
  // Copies the value from the given stream into this DynAny. Does
  // not rewind the input stream, but does flush our buffer first.
  // Returns 0 if there was a problem reading the value out of the
  // stream.
  //  Concurrency: hold DynAnyImplBase::lock

  virtual void onDispose();
  // Called when the reference count goes to zero. This gives the
  // node a chance to detach() any children before it is destroyed.
  // Should be overriden by descendants (which should call their
  // base-class's implementation).
  //  Concurrency: hold DynAnyImplBase::lock

  void detach() { pd_is_root = 1; }
  // Detach this one from its parent - ie. make it a root, so that
  // when it's reference count goes to zero it will be cleaned up.

  void incrRefCount();
  void decrRefCount();
  // Must not hold DynAnyImplBase::refCountLock.

  TypeCode_base* tc() const    { return pd_tc;            }

  TypeCode_base* actualTc() const { 
    return (TypeCode_base*)TypeCode_base::NP_expand(pd_tc);
  }
  // Return the typecode. If the typecode is an alias, return the content
  // type.

  CORBA::TCKind tckind() const { return actualTc()->NP_kind();}
  // Return the TCKind. If the typecode is an alias, return the TCKind of
  // the content type.


  cdrMemoryStream pd_buf;
  // The value held by the DynAny. Basic DynAny values are
  // always stored in the buffer. For complex types it is stored
  // in the buffer when convenient - otherwise in sub-components.

private:
  // TypeCode of the value held.
  TypeCode_base* pd_tc;

  // Reference counting and child management.
  static omni_mutex refCountLock;
  int               pd_refcount;
  CORBA::Boolean    pd_is_root;
};

//////////////////////////////////////////////////////////////////////
///////////////////////////// DynAnyImpl /////////////////////////////
//////////////////////////////////////////////////////////////////////

//: Implementation of DynAny for basic types.

class DynAnyImpl : public DynAnyImplBase
{
public:
  DynAnyImpl(TypeCode_base* tc, int nodetype, CORBA::Boolean is_root=1);
  virtual ~DynAnyImpl();

  /*******************
  * public interface *
  *******************/
  virtual void assign(CORBA::DynAny_ptr dyn_any);
  virtual CORBA::DynAny_ptr copy();
  virtual void insert_boolean(CORBA::Boolean value);
  virtual void insert_octet(CORBA::Octet value);
  virtual void insert_char(CORBA::Char value);
  virtual void insert_short(CORBA::Short value);
  virtual void insert_ushort(CORBA::UShort value);
  virtual void insert_long(CORBA::Long value);
  virtual void insert_ulong(CORBA::ULong value);
#ifndef NO_FLOAT
  virtual void insert_float(CORBA::Float value);
  virtual void insert_double(CORBA::Double value);
#endif
  virtual void insert_string(const char* value);
  virtual void insert_reference(CORBA::Object_ptr value);
  virtual void insert_typecode(CORBA::TypeCode_ptr value);
  virtual void insert_any(const CORBA::Any& value);
  virtual CORBA::Boolean get_boolean();
  virtual CORBA::Octet get_octet();
  virtual CORBA::Char get_char();
  virtual CORBA::Short get_short();
  virtual CORBA::UShort get_ushort();
  virtual CORBA::Long get_long();
  virtual CORBA::ULong get_ulong();
#ifndef NO_FLOAT
  virtual CORBA::Float get_float();
  virtual CORBA::Double get_double();
#endif
  virtual char* get_string();
  virtual CORBA::Object_ptr get_reference();
  virtual CORBA::TypeCode_ptr get_typecode();
  virtual CORBA::Any* get_any();
  virtual CORBA::DynAny_ptr current_component();
  virtual CORBA::Boolean next();
  virtual CORBA::Boolean seek(CORBA::Long index);
  virtual void rewind();

  /****************************
  * exposed private interface *
  ****************************/
  virtual void* NP_narrow();
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual int copy_to(cdrMemoryStream& mbs);
  virtual int copy_from(cdrMemoryStream& mbs);

  CORBA::Boolean isValid() const { return pd_isValid; }
  // If true it indicates that the value in the internal
  // buffer is valid.
  //  Must hold DynAnyImplBase::lock.

  void setValid()                { pd_isValid = 1; }
  void setInvalid()              { pd_isValid = 0; }
  // Must hold DynAnyImplBase::lock.

protected:
  cdrMemoryStream& doWrite(CORBA::TCKind kind) {
    if( tckind() != kind )  throw CORBA::DynAny::InvalidValue();
    pd_buf.rewindPtrs();
    setValid();
    return pd_buf;
  }

  cdrMemoryStream& doRead(CORBA::TCKind kind) {
    if( tckind() != kind || !isValid())  throw CORBA::DynAny::TypeMismatch();
    pd_buf.rewindInputPtr();
    return pd_buf;
  }

private:
  CORBA::Boolean    pd_isValid;
};

//////////////////////////////////////////////////////////////////////
///////////////////////////// DynEnumImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

class DynEnumImpl : public DynAnyImpl,
		    public CORBA::DynEnum
{
public:
  DynEnumImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynEnumImpl();

  /*******************
  * public interface *
  *******************/
  virtual CORBA::DynAny_ptr copy();
  virtual char* value_as_string();
  virtual void value_as_string(const char* value);
  virtual CORBA::ULong value_as_ulong();
  virtual void value_as_ulong(CORBA::ULong value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual void* NP_narrow();
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  TypeCode_enum* actualTc() const {
    return (TypeCode_enum*) DynAnyImplBase::actualTc();
  }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// DynFixedImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

#if 0
class DynFixedImpl : public DynAnyImpl,
		     public CORBA::DynFixed
{
public:
  DynFixedImpl();
  virtual ~DynFixedImpl();

  /*******************
  * public interface *
  *******************/
  virtual OctetSeq* get_value();
  virtual void set_value(const OctetSeq& val);

  /****************************
  * exposed private interface *
  ****************************/
  virtual void* NP_narrow();
  virtual int NP_nodetype() const;

private:
};
#endif

//////////////////////////////////////////////////////////////////////
////////////////////////// DynAnyConstrBase //////////////////////////
//////////////////////////////////////////////////////////////////////

//: Base class for constructed type DynAny's.

class DynAnyConstrBase : public DynAnyImplBase
{
public:
  DynAnyConstrBase(TypeCode_base* tc, int nodetype, CORBA::Boolean is_root);
  ~DynAnyConstrBase();

  /*******************
  * public interface *
  *******************/
  virtual void assign(CORBA::DynAny_ptr dyn_any);
  virtual void insert_boolean(CORBA::Boolean value);
  virtual void insert_octet(CORBA::Octet value);
  virtual void insert_char(CORBA::Char value);
  virtual void insert_short(CORBA::Short value);
  virtual void insert_ushort(CORBA::UShort value);
  virtual void insert_long(CORBA::Long value);
  virtual void insert_ulong(CORBA::ULong value);
#ifndef NO_FLOAT
  virtual void insert_float(CORBA::Float value);
  virtual void insert_double(CORBA::Double value);
#endif
  virtual void insert_string(const char* value);
  virtual void insert_reference(CORBA::Object_ptr value);
  virtual void insert_typecode(CORBA::TypeCode_ptr value);
  virtual void insert_any(const CORBA::Any& value);
  virtual CORBA::Boolean get_boolean();
  virtual CORBA::Octet get_octet();
  virtual CORBA::Char get_char();
  virtual CORBA::Short get_short();
  virtual CORBA::UShort get_ushort();
  virtual CORBA::Long get_long();
  virtual CORBA::ULong get_ulong();
#ifndef NO_FLOAT
  virtual CORBA::Float get_float();
  virtual CORBA::Double get_double();
#endif
  virtual char* get_string();
  virtual CORBA::Object_ptr get_reference();
  virtual CORBA::TypeCode_ptr get_typecode();
  virtual CORBA::Any* get_any();
  virtual CORBA::DynAny_ptr current_component();
  virtual CORBA::Boolean next();
  virtual CORBA::Boolean seek(CORBA::Long index);
  virtual void rewind();

  /***********
  * internal *
  ***********/
  virtual int copy_to(cdrMemoryStream& mbs);
  virtual int copy_from(cdrMemoryStream& mbs);
  virtual void onDispose();

protected:
  void setNumComponents(unsigned n);
  // May be called by derived classes to set/change the number of
  // components.
  //  Concurrency: hold DynAnyImplBase::lock (unless called from
  //                                          a constructor)

  virtual TypeCode_base* nthComponentTC(unsigned n) = 0;
  // Returns the TypeCode of the n'th component. Overriden by descendants.
  // This does not return a new reference - so it should not be
  // released. <n> MUST be in the range [0 pd_n_components).
  //  Must hold DynAnyImplBase::lock.

  CORBA::TCKind currentKind() {
    return nthComponentTC(pd_curr_index)->NP_kind();
  }
  // Return the CORBA::TCKind of the current component. There
  // MUST be a valid current component.
  //  Must hold DynAnyImplBase::lock.

  CORBA::Boolean canAppendComponent(unsigned i) const {
    return i == pd_n_really_in_buf && i == pd_n_in_buf && i < pd_first_in_comp;
  }
  // True if the given component can be appended to <pd_buf>.
  // The result is only valid if <i> is in range.
  //  Must hold DynAnyImplBase::lock.

  cdrMemoryStream& writeCurrent(CORBA::TCKind kind) {
    if( pd_curr_index < 0 || currentKind() != kind )
      throw CORBA::DynAny::InvalidValue();
    if( canAppendComponent(pd_curr_index) ) {
      pd_n_in_buf++;
      pd_n_really_in_buf++;
      if( pd_curr_index + 1 < (int)pd_n_components )  pd_curr_index++;
      else                                            pd_curr_index = -1;
      return pd_buf;
    } else {
      DynAnyImpl* cc = ToDynAnyImpl(getCurrent());
      cc->pd_buf.rewindPtrs();
      cc->setValid();
      if( pd_curr_index + 1 < (int)pd_n_components )  pd_curr_index++;
      else                                            pd_curr_index = -1;
      return cc->pd_buf;
    }
  }
  // Helper function for writing primitive values into the current
  // component. It returns a reference to the buffer which the value
  // should be inserted into. If the value can be appended to <pd_buf>,
  // then <pd_buf> is returned, otherwise it is the buffer of the
  // child DynAny.
  //  Checks also the there is a current component, and that its type
  // is the same as the value being inserted.
  //  Must hold DynAnyImplBase::lock.

  cdrMemoryStream& readCurrent(CORBA::TCKind kind) {
    if( pd_curr_index < 0 || currentKind() != kind )
      throw CORBA::DynAny::TypeMismatch();
    if( pd_curr_index < (int)pd_n_in_buf ) {
      if( pd_read_index != pd_curr_index )  seekTo(pd_curr_index);
      pd_read_index++;
      if( pd_curr_index + 1 < (int)pd_n_components )  pd_curr_index++;
      else                                            pd_curr_index = -1;
      return pd_buf;
    }
    else if( pd_curr_index >= (int)pd_first_in_comp ) {
      DynAnyImpl* cc = ToDynAnyImpl(getCurrent());
      if( !cc->isValid() )  throw CORBA::DynAny::TypeMismatch();
      cc->pd_buf.rewindInputPtr();
      if( pd_curr_index + 1 < (int)pd_n_components )  pd_curr_index++;
      else                                            pd_curr_index = -1;
      return cc->pd_buf;
    }
    else throw CORBA::DynAny::TypeMismatch();
#ifdef NEED_DUMMY_RETURN
    return pd_buf;
#endif
  }
  // Helper function for reading a primitive value from the current
  // component. Returns a reference to the buffer from which the
  // value may be extracted. It checks that <pd_curr_index> is valid,
  // and the component is of the type expected.
  //  Must hold DynAnyImplBase::lock.

  DynAnyImplBase* getCurrent() {
    if( pd_curr_index < (int)pd_first_in_comp )
      createComponent(pd_curr_index);
    return pd_components[pd_curr_index];
  }
  // If not already there, puts the current component (and those
  // following it) into <pd_components>, and returns it. There
  // must be a current component.
  //  Must hold DynAnyImplBase::lock.

  void createComponent(unsigned n);
  // If it does not already exist, create a DynAny for the n'th
  // component. Ensures also that DynAny's are created for all
  // components following that one.
  //  Requires n < pd_n_components.
  //  Must hold DynAnyImplBase::lock.

  void seekTo(unsigned n);
  // Seek the internal buffer so as to read the i'th component.
  //  Does not throw any exceptions.
  //  Requires n < pd_n_in_buf.
  //  Must hold DynAnyImplBase::lock.

  int component_to_any(unsigned i, CORBA::Any& a);
  // Copy the i'th component into <a>. Returns 0 if that
  // component is not properly initialised.
  //  Does not throw any exceptions.
  //  Requires i < pd_n_components.
  //  Must hold DynAnyImplBase::lock.

  int component_from_any(unsigned i, const CORBA::Any& a);
  // Sets the value of the i'th component to that in <a>.
  // Returns 0 if the value in the Any is of the wrong
  // type, or the Any has not been initialised.
  //  Does not throw any exceptions.
  //  Requires i < pd_n_components.
  //  Must hold DynAnyImplBase::lock.

  omnivector<DynAnyImplBase*> pd_components;
  // Sequence of pointers to components that are not stored in <pd_buf>.
  // The length of this sequence is always equal to pd_n_components.

  unsigned pd_n_components;
  // The total number of components this value has.

  unsigned pd_n_in_buf;
  // Components in the range [0 pd_n_in_buf) are in <pd_buf>.
  // pd_n_in_buf <= pd_first_in_comp.

  unsigned pd_n_really_in_buf;
  // The number of components which have actually been written into
  // <pd_buf>. This value may be greater than <pd_n_components> or
  // <pd_first_in_comp>.

  unsigned pd_first_in_comp;
  // Components in the range [pd_first_in_comp pd_n_components)
  // are in pd_components. Thus those in the range
  // [pd_n_in_buf pd_first_in_comp) are not yet defined.

  int pd_curr_index;
  // The index of the 'current component'. If this is -1 then
  // there is no current component. If there are zero components
  // then this is always -1.

  int pd_read_index;
  // The index of the component that the buffer's read marker is
  // pointing at. So if pd_curr_index == pd_read_index, and the
  // component is in the buffer, then it can be read out.
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// DynStructImpl ///////////////////////////
//////////////////////////////////////////////////////////////////////

//: DynAny for structure and exception types.

class DynStructImpl : public DynAnyConstrBase,
		      public CORBA::DynStruct
{
public:
  DynStructImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynStructImpl();

  /*******************
  * public interface *
  *******************/
  virtual CORBA::DynAny_ptr copy();
  virtual char*  current_member_name();
  virtual CORBA::TCKind current_member_kind();
  virtual CORBA::NameValuePairSeq* get_members();
  virtual void set_members(const CORBA::NameValuePairSeq& NVSeqVal);

  /****************************
  * exposed private interface *
  ****************************/
  virtual void* NP_narrow();
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual TypeCode_base* nthComponentTC(unsigned n);
  // Overrides DynAnyConstrBase

private:
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// DynUnionDisc /////////////////////////////
//////////////////////////////////////////////////////////////////////

//: DynAny for the discriminator of a union.
// Behaves like a DynAnyImpl, but catches any updates, and informs
// the union (which this is the discriminator of) so that it may
// update its member appropriately.

class DynUnionDisc : public DynAnyImpl {
public:
  DynUnionDisc(TypeCode_base* tc, int nodetype, DynUnionImpl* un)
    : DynAnyImpl(tc, nodetype, DYNANY_CHILD),
      pd_union(un) {}
  virtual ~DynUnionDisc();

  /*******************
  * public interface *
  *******************/
  virtual void assign(CORBA::DynAny_ptr dyn_any);
  virtual CORBA::DynAny_ptr copy();
  virtual void insert_boolean(CORBA::Boolean value);
  virtual void insert_octet(CORBA::Octet value);
  virtual void insert_char(CORBA::Char value);
  virtual void insert_short(CORBA::Short value);
  virtual void insert_ushort(CORBA::UShort value);
  virtual void insert_long(CORBA::Long value);
  virtual void insert_ulong(CORBA::ULong value);
#ifndef NO_FLOAT
  virtual void insert_float(CORBA::Float value);
  virtual void insert_double(CORBA::Double value);
#endif
  virtual void insert_string(const char* value);
  virtual void insert_reference(CORBA::Object_ptr value);
  virtual void insert_typecode(CORBA::TypeCode_ptr value);
  virtual void insert_any(const CORBA::Any& value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual void* NP_narrow();
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  void detach() {
    pd_union = 0;
    DynAnyImpl::detach();
  }
  virtual void set_value(TypeCode_union::Discriminator v);
  // Must NOT hold DynAnyImplBase::lock.

protected:
  DynUnionImpl* pd_union;
  // Pointer to the parent union, or 0 if detached.
};

//////////////////////////////////////////////////////////////////////
///////////////////////// DynUnionEnumDisc ///////////////////////////
//////////////////////////////////////////////////////////////////////

//: DynAny for the (enum) discriminator of a union.

class DynUnionEnumDisc : public DynUnionDisc,
			 public CORBA::DynEnum
{
public:
  DynUnionEnumDisc(TypeCode_base* tc, DynUnionImpl* un);
  virtual ~DynUnionEnumDisc();

  /*******************
  * public interface *
  *******************/
  virtual CORBA::DynAny_ptr copy();
  virtual char* value_as_string();
  virtual void value_as_string(const char* value);
  virtual CORBA::ULong value_as_ulong();
  virtual void value_as_ulong(CORBA::ULong value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual void* NP_narrow();
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual void set_value(TypeCode_union::Discriminator v);
  // Must NOT hold DynAnyImplBase::lock.

  TypeCode_enum* actualTc() const {
    return (TypeCode_enum*) DynAnyImplBase::actualTc();
  }
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// DynUnionImpl /////////////////////////////
//////////////////////////////////////////////////////////////////////

class DynUnionImpl : public DynAnyImplBase,
		     public CORBA::DynUnion
{
public:
  DynUnionImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynUnionImpl();

  /*******************
  * public interface *
  *******************/
  virtual void assign(CORBA::DynAny_ptr dyn_any);
  virtual CORBA::DynAny_ptr copy();
  virtual void insert_boolean(CORBA::Boolean value);
  virtual void insert_octet(CORBA::Octet value);
  virtual void insert_char(CORBA::Char value);
  virtual void insert_short(CORBA::Short value);
  virtual void insert_ushort(CORBA::UShort value);
  virtual void insert_long(CORBA::Long value);
  virtual void insert_ulong(CORBA::ULong value);
#ifndef NO_FLOAT
  virtual void insert_float(CORBA::Float value);
  virtual void insert_double(CORBA::Double value);
#endif
  virtual void insert_string(const char* value);
  virtual void insert_reference(CORBA::Object_ptr value);
  virtual void insert_typecode(CORBA::TypeCode_ptr value);
  virtual void insert_any(const CORBA::Any& value);
  virtual CORBA::Boolean get_boolean();
  virtual CORBA::Octet get_octet();
  virtual CORBA::Char get_char();
  virtual CORBA::Short get_short();
  virtual CORBA::UShort get_ushort();
  virtual CORBA::Long get_long();
  virtual CORBA::ULong get_ulong();
#ifndef NO_FLOAT
  virtual CORBA::Float get_float();
  virtual CORBA::Double get_double();
#endif
  virtual char* get_string();
  virtual CORBA::Object_ptr get_reference();
  virtual CORBA::TypeCode_ptr get_typecode();
  virtual CORBA::Any* get_any();
  virtual CORBA::DynAny_ptr current_component();
  virtual CORBA::Boolean next();
  virtual CORBA::Boolean seek(CORBA::Long index);
  virtual void rewind();
  virtual CORBA::Boolean set_as_default();
  virtual void set_as_default(CORBA::Boolean value);
  virtual CORBA::DynAny_ptr discriminator();
  virtual CORBA::TCKind discriminator_kind();
  virtual CORBA::DynAny_ptr member();
  virtual char*  member_name();
  virtual void member_name(const char* value);
  virtual CORBA::TCKind member_kind();

  /****************************
  * exposed private interface *
  ****************************/
  virtual void* NP_narrow();
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual int copy_to(cdrMemoryStream& mbs);
  virtual int copy_from(cdrMemoryStream& mbs);
  virtual void onDispose();

  void discriminatorHasChanged();
  // If necassary detaches the old member, and creates a new member
  // of the appropriate type.

  inline TypeCode_union::Discriminator NP_disc_value() const {
    return pd_disc_value;
  }
  inline CORBA::Long NP_disc_index() const {
    return pd_member ? pd_disc_index : -1;
  }

private:
  TypeCode_union* actualTc() const {
    return (TypeCode_union*) DynAnyImplBase::actualTc();
  }

  cdrMemoryStream& writeCurrent(CORBA::TCKind kind) {
    switch( pd_curr_index ) {
    case 0:
      if( kind != pd_disc_kind )  throw CORBA::DynAny::InvalidValue();
      pd_disc->pd_buf.rewindPtrs();
      pd_disc->setValid();
      pd_curr_index = 1;
      return pd_disc->pd_buf;
    case 1:
      if( pd_member_kind != kind )  throw CORBA::DynAny::InvalidValue();
      pd_member->pd_buf.rewindPtrs();
      // Must be a DynAnyImpl ...
      ToDynAnyImpl(pd_member)->setValid();
      pd_curr_index = -1;
      return pd_member->pd_buf;
    default:
      throw CORBA::DynAny::InvalidValue();
    }
#ifdef NEED_DUMMY_RETURN
    return pd_buf;
#endif
  }

  cdrMemoryStream& readCurrent(CORBA::TCKind kind) {
    switch( pd_curr_index ) {
    case 0:
      if( kind != pd_disc_kind || !pd_disc->isValid() )
	throw CORBA::DynAny::TypeMismatch();
      pd_disc->pd_buf.rewindInputPtr();
      pd_curr_index = 1;
      return pd_disc->pd_buf;
    case 1:
      // If !pd_member, then pd_member_kind == tk_null.
      if( pd_member_kind != kind || !ToDynAnyImpl(pd_member)->isValid() )
	throw CORBA::DynAny::TypeMismatch();
      pd_member->pd_buf.rewindInputPtr();
      pd_curr_index = -1;
      return pd_member->pd_buf;
    default:
      throw CORBA::DynAny::TypeMismatch();
    }
#ifdef NEED_DUMMY_RETURN
    return pd_buf;
#endif
  }

  void detachMember() {
    if( pd_member ) {
      pd_member->detach();
      pd_member->decrRefCount();
      pd_member = 0;
      pd_member_kind = CORBA::tk_null;
    }
  }

  /*
  ** DynUnionImpl does not use an internal buffer to store the value.
  ** Instead an instance of a DynAny is created for each of the
  ** discriminator and the member value.
  */


  DynUnionDisc*                 pd_disc;
  TypeCode_base*                pd_disc_type;
  CORBA::TCKind                 pd_disc_kind;
  TypeCode_union::Discriminator pd_disc_value;
  CORBA::Long                   pd_disc_index;  // invalid if !pd_member

  DynAnyImplBase* pd_member;            // may be 0
  CORBA::TCKind   pd_member_kind;       // tk_null if !pd_member

  int pd_curr_index;                    // -1, 0 or 1
};

//////////////////////////////////////////////////////////////////////
///////////////////////// DynSequenceImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

class DynSequenceImpl : public DynAnyConstrBase,
			public CORBA::DynSequence
{
public:
  DynSequenceImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynSequenceImpl();

  /*******************
  * public interface *
  *******************/
  virtual CORBA::DynAny_ptr copy();
  virtual CORBA::ULong length();
  virtual void length (CORBA::ULong value);
  virtual CORBA::AnySeq* get_elements();
  virtual void set_elements(const CORBA::AnySeq& value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual void* NP_narrow();
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual int copy_to(cdrMemoryStream& mbs);
  virtual int copy_from(cdrMemoryStream& mbs);
  virtual TypeCode_base* nthComponentTC(unsigned n);
  // Overrides DynAnyConstrBase

private:
  CORBA::ULong pd_bound;  // 0 if unbounded sequence, bound otherwise
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// DynArrayImpl /////////////////////////////
//////////////////////////////////////////////////////////////////////

class DynArrayImpl : public DynAnyConstrBase,
		     public CORBA::DynArray
{
public:
  DynArrayImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynArrayImpl();

  /*******************
  * public interface *
  *******************/
  virtual CORBA::DynAny_ptr copy();
  virtual CORBA::AnySeq* get_elements();
  virtual void set_elements(const CORBA::AnySeq& value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual void* NP_narrow();
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual TypeCode_base* nthComponentTC(unsigned n);
  // Overrides DynAnyConstrBase

private:
};

//////////////////////////////////////////////////////////////////////
////////////////////////////// Narrowing /////////////////////////////
//////////////////////////////////////////////////////////////////////

// <p> must not be nil.
inline DynAnyImplBase* ToDynAnyImplBase(CORBA::DynAny_ptr p)
{
  void* da = p->NP_narrow();

  switch( p->NP_nodetype() ) {
  case dt_any:      return (DynAnyImpl*) da;
  case dt_enum:     return (DynEnumImpl*) da;
#if 0
  case dt_fixed:    return (DynFixedImpl*) da;
#endif
  case dt_struct:   return (DynStructImpl*) da;
  case dt_union:    return (DynUnionImpl*) da;
  case dt_seq:      return (DynSequenceImpl*) da;
  case dt_array:    return (DynArrayImpl*) da;
  case dt_disc:     return (DynUnionDisc*) da;
  case dt_enumdisc: return (DynUnionEnumDisc*) da;
  default:
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "ToDynAnyImplBase - oops");
  }
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

// <p> must not be nil.
inline DynAnyImpl* ToDynAnyImpl(CORBA::DynAny_ptr p)
{
  void* da = p->NP_narrow();

  switch( p->NP_nodetype() ) {
  case dt_any:      return (DynAnyImpl*) da;
  case dt_enum:     return (DynEnumImpl*) da;
  case dt_disc:     return (DynUnionDisc*) da;
  case dt_enumdisc: return (DynUnionEnumDisc*) da;
  default:
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "ToDynAnyImpl(DynAny_ptr) - was not a DynAnyImpl");
  }
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}

// <p> must not be nil.
inline DynAnyImpl* ToDynAnyImpl(DynAnyImplBase* p)
{
  switch( p->NP_nodetype() ) {
  case dt_any:
  case dt_enum:
  case dt_disc:
  case dt_enumdisc:
    return (DynAnyImpl*) p;
  default:
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "ToDynAnyImpl(DynAnyImplBase*) - was not a DynAnyImpl");
  }
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


#endif  // __DYNANY_H__
