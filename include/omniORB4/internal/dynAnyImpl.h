// -*- Mode: C++; -*-
//                            Package   : omniORB
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
 Revision 1.1.4.4  2005/01/06 16:39:23  dgrisby
 DynValue and DynValueBox implementations; misc small fixes.

 Revision 1.1.4.3  2004/07/23 10:29:57  dgrisby
 Completely new, much simpler Any implementation.

 Revision 1.1.4.2  2004/07/04 23:53:35  dgrisby
 More ValueType TypeCode and Any support.

 Revision 1.1.4.1  2003/03/23 21:03:51  dgrisby
 Start of omniORB 4.1.x development branch.

 Revision 1.1.2.3  2001/10/19 11:04:03  dpg1
 Avoid confusing (to gcc 2.95) inheritance of refcount functions.

 Revision 1.1.2.2  2001/10/17 18:51:51  dpg1
 Fix inevitable Windows problems.

 Revision 1.1.2.1  2001/10/17 16:44:05  dpg1
 Update DynAny to CORBA 2.5 spec, const Any exception extraction.

 Revision 1.1.2.1  2001/04/18 17:18:17  sll
 Big checkin with the brand new internal APIs.
 These files were relocated and scoped with the omni namespace.

 Revision 1.5.2.5  2000/11/17 19:09:37  dpg1
 Support codeset conversion in any.

 Revision 1.5.2.4  2000/11/09 12:27:53  dpg1
 Huge merge from omni3_develop, plus full long long from omni3_1_develop.

 Revision 1.5.2.3  2000/10/06 16:40:53  sll
 Changed to use cdrStream.

 Revision 1.5.2.2  2000/09/27 17:25:41  sll
 Changed include/omniORB3 to include/omniORB4.

 Revision 1.5.2.1  2000/07/17 10:35:41  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

 Revision 1.6  2000/07/13 15:26:02  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.2.8.3  1999/10/26 20:18:20  sll
 DynAny no longer do alias expansion on the typecode. In other words, all
 aliases in the typecode are preserved.

 Revision 1.2.8.2  1999/09/22 16:15:59  djr
 Removed MT locking.

*/

#ifndef __DYNANYIMPL_H__
#define __DYNANYIMPL_H__

#include <typecode.h>
#include <omniORB4/anyStream.h>

OMNI_NAMESPACE_BEGIN(omni)

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
#define dt_value      9
#define dt_value_box  10


// Forward declarations.
class DynAnyImplBase;
class DynAnyImpl;
class DynEnumImpl;
class DynFixedImpl;
class DynAnyConstrBase;
class DynStructImpl;
class DynUnionImpl;
class DynSequenceImpl;
class DynArrayImpl;
class DynValueImpl;
class DynValueBoxImpl;

inline DynAnyImpl*       ToDynAnyImpl      (DynamicAny::DynAny_ptr p);
inline DynAnyImplBase*   ToDynAnyImplBase  (DynamicAny::DynAny_ptr p);
inline DynAnyConstrBase* ToDynAnyConstrBase(DynamicAny::DynAny_ptr p);
inline DynUnionImpl*     ToDynUnionImpl    (DynamicAny::DynAny_ptr p);

// Values used to indicate whether a DynAny is the child of
// another DynAny, or the root.
#define DYNANY_CHILD  0
#define DYNANY_ROOT   1


//////////////////////////////////////////////////////////////////////
/////////////////////////// DynAnyImplBase ///////////////////////////
//////////////////////////////////////////////////////////////////////

//: Functionality common to all DynAny implementations.

class DynAnyImplBase : public virtual DynamicAny::DynAny
{
public:
  DynAnyImplBase(TypeCode_base* tc, int nodetype, CORBA::Boolean is_root)
    : OMNIORB_BASE_CTOR(DynamicAny::)DynAny(0),
      pd_tc(tc), pd_refcount(1), pd_is_root(is_root), pd_destroyed(0)
  {}
  // <tc> is consumed.

  virtual ~DynAnyImplBase();

  /*******************
  * public interface *
  *******************/
  virtual CORBA::TypeCode_ptr type() const;
  
  virtual void from_any(const CORBA::Any& value);
  virtual CORBA::Any* to_any();
  virtual void destroy();

  /***********
  * internal *
  ***********/

  virtual void set_to_initial_value() = 0;
  // Set the DynAny to the default initial value.

  virtual int copy_to(cdrAnyMemoryStream& mbs) = 0;
  // Copies our value into the given stream. Does not flush <mbs>,
  // but does rewind our buffer first. Returns 0 if we are not
  // properly initialised.
  //  Concurrency: hold DynAnyImplBase::lock

  virtual int copy_from(cdrAnyMemoryStream& mbs) = 0;
  // Copies the value from the given stream into this DynAny,
  // replacing the old value. Reads from the stream's current
  // position, and updates the stream's pointers. Returns 0 if there
  // was a problem reading the value out of the stream.
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

  void attach() { OMNIORB_ASSERT(pd_is_root); pd_is_root = 0; }
  // Attach to a new parent.

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Must not hold DynAnyImplBase::refCountLock.

  TypeCode_base* tc() const { return pd_tc; }

  TypeCode_base* actualTc() const { 
    return (TypeCode_base*)TypeCode_base::NP_expand(pd_tc);
  }
  // Return the typecode. If the typecode is an alias, return the content
  // type.

  CORBA::TCKind tckind() const { return actualTc()->NP_kind(); }
  // Return the TCKind. If the typecode is an alias, return the TCKind of
  // the content type.


  cdrAnyMemoryStream pd_buf;
  // The value held by the DynAny. Basic DynAny values are
  // always stored in the buffer. For complex types it is stored
  // in the buffer when convenient - otherwise in sub-components.

  inline CORBA::Boolean is_root()   const { return pd_is_root; }
  inline CORBA::Boolean destroyed() const { return pd_destroyed; }
  // Assume reading is atomic.

  virtual void* _ptrToObjRef(const char* repoId) = 0;

  static const char* _PD_repoId;
  // Fake repoId for use by _ptrToObjRef

private:
  // TypeCode of the value held.
  TypeCode_base* pd_tc;

  // Reference counting and child management.
  int            pd_refcount;
  CORBA::Boolean pd_is_root;
  CORBA::Boolean pd_destroyed;

  static omni_tracedmutex refCountLock;
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
  virtual void assign(DynamicAny::DynAny_ptr dyn_any);
  virtual DynamicAny::DynAny_ptr copy();
  virtual CORBA::Boolean equal(DynamicAny::DynAny_ptr dyn_any);

  virtual void insert_boolean   (CORBA::Boolean      	 value);
  virtual void insert_octet     (CORBA::Octet        	 value);
  virtual void insert_char      (CORBA::Char         	 value);
  virtual void insert_short     (CORBA::Short        	 value);
  virtual void insert_ushort    (CORBA::UShort       	 value);
  virtual void insert_long      (CORBA::Long         	 value);
  virtual void insert_ulong     (CORBA::ULong        	 value);
#ifndef NO_FLOAT				 	       
  virtual void insert_float     (CORBA::Float        	 value);
  virtual void insert_double    (CORBA::Double       	 value);
#endif						 	       
  virtual void insert_string    (const char*         	 value);
  virtual void insert_reference (CORBA::Object_ptr   	 value);
  virtual void insert_typecode  (CORBA::TypeCode_ptr 	 value);
#ifdef HAS_LongLong				 	       
  virtual void insert_longlong  (CORBA::LongLong     	 value);
  virtual void insert_ulonglong (CORBA::ULongLong    	 value);
#endif						 	       
#ifdef HAS_LongDouble				 	       
  virtual void insert_longdouble(CORBA::LongDouble   	 value);
#endif						 	       
  virtual void insert_wchar     (CORBA::WChar        	 value);
  virtual void insert_wstring   (const CORBA::WChar* 	 value);
  virtual void insert_any       (const CORBA::Any&   	 value);
  virtual void insert_dyn_any   (DynamicAny::DynAny_ptr  value);
  virtual void insert_val       (CORBA::ValueBase*   	 value);
  virtual void insert_abstract  (CORBA::AbstractBase_ptr value);

  virtual CORBA::Boolean      	  get_boolean();
  virtual CORBA::Octet        	  get_octet();
  virtual CORBA::Char         	  get_char();
  virtual CORBA::Short        	  get_short();
  virtual CORBA::UShort       	  get_ushort();
  virtual CORBA::Long         	  get_long();
  virtual CORBA::ULong        	  get_ulong();
#ifndef NO_FLOAT
  virtual CORBA::Float        	  get_float();
  virtual CORBA::Double       	  get_double();
#endif			 	    
  virtual char*               	  get_string();
  virtual CORBA::Object_ptr   	  get_reference();
  virtual CORBA::TypeCode_ptr 	  get_typecode();
#ifdef HAS_LongLong
  virtual CORBA::LongLong     	  get_longlong();
  virtual CORBA::ULongLong    	  get_ulonglong();
#endif			 	    
#ifdef HAS_LongDouble
  virtual CORBA::LongDouble   	  get_longdouble();
#endif
  virtual CORBA::WChar        	  get_wchar();
  virtual CORBA::WChar*       	  get_wstring();
  virtual CORBA::Any*         	  get_any();
  virtual DynamicAny::DynAny_ptr  get_dyn_any();
  virtual CORBA::ValueBase*       get_val();
  virtual CORBA::AbstractBase_ptr get_abstract();

  virtual void insert_boolean_seq   (CORBA::BooleanSeq&    value);
  virtual void insert_octet_seq     (CORBA::OctetSeq&      value);
  virtual void insert_char_seq      (CORBA::CharSeq&       value);
  virtual void insert_short_seq     (CORBA::ShortSeq&      value);
  virtual void insert_ushort_seq    (CORBA::UShortSeq&     value);
  virtual void insert_long_seq      (CORBA::LongSeq&       value);
  virtual void insert_ulong_seq     (CORBA::ULongSeq&      value);
#ifndef NO_FLOAT
  virtual void insert_float_seq     (CORBA::FloatSeq&      value);
  virtual void insert_double_seq    (CORBA::DoubleSeq&     value);
#endif
#ifdef HAS_LongLong
  virtual void insert_longlong_seq  (CORBA::LongLongSeq&   value);
  virtual void insert_ulonglong_seq (CORBA::ULongLongSeq&  value);
#endif
#ifdef HAS_LongDouble						 
  virtual void insert_longdouble_seq(CORBA::LongDoubleSeq& value);
#endif								 
  virtual void insert_wchar_seq     (CORBA::WCharSeq&      value);

  virtual CORBA::BooleanSeq*    get_boolean_seq();
  virtual CORBA::OctetSeq*      get_octet_seq();
  virtual CORBA::CharSeq*       get_char_seq();
  virtual CORBA::ShortSeq*      get_short_seq();
  virtual CORBA::UShortSeq*     get_ushort_seq();
  virtual CORBA::LongSeq*       get_long_seq();
  virtual CORBA::ULongSeq* 	get_ulong_seq();
#ifndef NO_FLOAT
  virtual CORBA::FloatSeq* 	get_float_seq();
  virtual CORBA::DoubleSeq*     get_double_seq();
#endif
#ifdef HAS_LongLong
  virtual CORBA::LongLongSeq*   get_longlong_seq();
  virtual CORBA::ULongLongSeq*  get_ulonglong_seq();
#endif
#ifdef HAS_LongDouble
  virtual CORBA::LongDoubleSeq* get_longdouble_seq();
#endif
  virtual CORBA::WCharSeq*      get_wchar_seq();

  virtual CORBA::Boolean seek(CORBA::Long index);
  virtual void rewind();
  virtual CORBA::Boolean next();
  virtual CORBA::ULong component_count();
  virtual DynamicAny::DynAny_ptr current_component();

  /****************************
  * exposed private interface *
  ****************************/
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual void set_to_initial_value();
  virtual int copy_to(cdrAnyMemoryStream& mbs);
  virtual int copy_from(cdrAnyMemoryStream& mbs);

  CORBA::Boolean isValid() const { return pd_isValid; }
  // If true it indicates that the value in the internal
  // buffer is valid.
  //  Must hold DynAnyImplBase::lock.

  void setValid()                { pd_isValid = 1; }
  void setInvalid()              { pd_isValid = 0; }
  // Must hold DynAnyImplBase::lock.

  virtual void* _ptrToObjRef(const char* repoId);

  static const char* _PD_repoId;
  // Fake repoId for use by _ptrToObjRef

protected:
  cdrAnyMemoryStream& doWrite(CORBA::TCKind kind) {
    if( tckind() != kind )  throw DynamicAny::DynAny::TypeMismatch();
    pd_buf.rewindPtrs();
    setValid();
    return pd_buf;
  }

  cdrAnyMemoryStream& doRead(CORBA::TCKind kind) {
    if( tckind() != kind || !isValid())
      throw DynamicAny::DynAny::TypeMismatch();
    pd_buf.rewindInputPtr();
    return pd_buf;
  }

private:
  CORBA::Boolean    pd_isValid;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// DynFixedImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

class DynFixedImpl : public DynAnyImpl,
		     public DynamicAny::DynFixed
{
public:
  DynFixedImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynFixedImpl();

  /*******************
  * public interface *
  *******************/
  virtual DynamicAny::DynAny_ptr copy();

  virtual char* get_value();
  virtual CORBA::Boolean set_value(const char* val);

  /****************************
  * exposed private interface *
  ****************************/
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual void set_to_initial_value();
  TypeCode_fixed* actualTc() const {
    return (TypeCode_fixed*) DynAnyImplBase::actualTc();
  }

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Must not hold DynAnyImplBase::refCountLock.

  virtual void* _ptrToObjRef(const char* repoId);

private:
};

//////////////////////////////////////////////////////////////////////
///////////////////////////// DynEnumImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

class DynEnumImpl : public DynAnyImpl,
		    public DynamicAny::DynEnum
{
public:
  DynEnumImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynEnumImpl();

  /*******************
  * public interface *
  *******************/
  virtual DynamicAny::DynAny_ptr copy();

  virtual char* get_as_string();
  virtual void set_as_string(const char* value);
  virtual CORBA::ULong get_as_ulong();
  virtual void set_as_ulong(CORBA::ULong value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual void set_to_initial_value();
  TypeCode_enum* actualTc() const {
    return (TypeCode_enum*) DynAnyImplBase::actualTc();
  }

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Must not hold DynAnyImplBase::refCountLock.

  virtual void* _ptrToObjRef(const char* repoId);
};


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
  virtual void assign(DynamicAny::DynAny_ptr dyn_any);
  virtual CORBA::Boolean equal(DynamicAny::DynAny_ptr dyn_any);

  virtual void insert_boolean   (CORBA::Boolean      	 value);
  virtual void insert_octet     (CORBA::Octet        	 value);
  virtual void insert_char      (CORBA::Char         	 value);
  virtual void insert_short     (CORBA::Short        	 value);
  virtual void insert_ushort    (CORBA::UShort       	 value);
  virtual void insert_long      (CORBA::Long         	 value);
  virtual void insert_ulong     (CORBA::ULong        	 value);
#ifndef NO_FLOAT				 	       
  virtual void insert_float     (CORBA::Float        	 value);
  virtual void insert_double    (CORBA::Double       	 value);
#endif						 	       
  virtual void insert_string    (const char*         	 value);
  virtual void insert_reference (CORBA::Object_ptr   	 value);
  virtual void insert_typecode  (CORBA::TypeCode_ptr 	 value);
#ifdef HAS_LongLong				 	       
  virtual void insert_longlong  (CORBA::LongLong     	 value);
  virtual void insert_ulonglong (CORBA::ULongLong    	 value);
#endif						 	       
#ifdef HAS_LongDouble				 	       
  virtual void insert_longdouble(CORBA::LongDouble   	 value);
#endif						 	       
  virtual void insert_wchar     (CORBA::WChar        	 value);
  virtual void insert_wstring   (const CORBA::WChar* 	 value);
  virtual void insert_any       (const CORBA::Any&   	 value);
  virtual void insert_dyn_any   (DynamicAny::DynAny_ptr  value);
  virtual void insert_val       (CORBA::ValueBase*   	 value);
  virtual void insert_abstract  (CORBA::AbstractBase_ptr value);

  virtual CORBA::Boolean      	  get_boolean();
  virtual CORBA::Octet        	  get_octet();
  virtual CORBA::Char         	  get_char();
  virtual CORBA::Short        	  get_short();
  virtual CORBA::UShort       	  get_ushort();
  virtual CORBA::Long         	  get_long();
  virtual CORBA::ULong        	  get_ulong();
#ifndef NO_FLOAT
  virtual CORBA::Float        	  get_float();
  virtual CORBA::Double       	  get_double();
#endif			 	    
  virtual char*               	  get_string();
  virtual CORBA::Object_ptr   	  get_reference();
  virtual CORBA::TypeCode_ptr 	  get_typecode();
#ifdef HAS_LongLong
  virtual CORBA::LongLong     	  get_longlong();
  virtual CORBA::ULongLong    	  get_ulonglong();
#endif			 	    
#ifdef HAS_LongDouble
  virtual CORBA::LongDouble   	  get_longdouble();
#endif
  virtual CORBA::WChar        	  get_wchar();
  virtual CORBA::WChar*       	  get_wstring();
  virtual CORBA::Any*         	  get_any();
  virtual DynamicAny::DynAny_ptr  get_dyn_any();
  virtual CORBA::ValueBase*       get_val();
  virtual CORBA::AbstractBase_ptr get_abstract();

  virtual void insert_boolean_seq   (CORBA::BooleanSeq&    value);
  virtual void insert_octet_seq     (CORBA::OctetSeq&      value);
  virtual void insert_char_seq      (CORBA::CharSeq&       value);
  virtual void insert_short_seq     (CORBA::ShortSeq&      value);
  virtual void insert_ushort_seq    (CORBA::UShortSeq&     value);
  virtual void insert_long_seq      (CORBA::LongSeq&       value);
  virtual void insert_ulong_seq     (CORBA::ULongSeq&      value);
#ifndef NO_FLOAT
  virtual void insert_float_seq     (CORBA::FloatSeq&      value);
  virtual void insert_double_seq    (CORBA::DoubleSeq&     value);
#endif
#ifdef HAS_LongLong
  virtual void insert_longlong_seq  (CORBA::LongLongSeq&   value);
  virtual void insert_ulonglong_seq (CORBA::ULongLongSeq&  value);
#endif
#ifdef HAS_LongDouble						 
  virtual void insert_longdouble_seq(CORBA::LongDoubleSeq& value);
#endif								 
  virtual void insert_wchar_seq     (CORBA::WCharSeq&      value);

  virtual CORBA::BooleanSeq*    get_boolean_seq();
  virtual CORBA::OctetSeq*      get_octet_seq();
  virtual CORBA::CharSeq*       get_char_seq();
  virtual CORBA::ShortSeq*      get_short_seq();
  virtual CORBA::UShortSeq*     get_ushort_seq();
  virtual CORBA::LongSeq*       get_long_seq();
  virtual CORBA::ULongSeq* 	get_ulong_seq();
#ifndef NO_FLOAT
  virtual CORBA::FloatSeq* 	get_float_seq();
  virtual CORBA::DoubleSeq*     get_double_seq();
#endif
#ifdef HAS_LongLong
  virtual CORBA::LongLongSeq*   get_longlong_seq();
  virtual CORBA::ULongLongSeq*  get_ulonglong_seq();
#endif
#ifdef HAS_LongDouble
  virtual CORBA::LongDoubleSeq* get_longdouble_seq();
#endif
  virtual CORBA::WCharSeq*      get_wchar_seq();

  virtual CORBA::Boolean seek(CORBA::Long index);
  virtual void rewind();
  virtual CORBA::Boolean next();
  virtual CORBA::ULong component_count();
  virtual DynamicAny::DynAny_ptr current_component();

  /***********
  * internal *
  ***********/
  virtual void set_to_initial_value();
  virtual int copy_to(cdrAnyMemoryStream& mbs);
  virtual int copy_from(cdrAnyMemoryStream& mbs);
  virtual void onDispose();

  static const char* _PD_repoId;
  // Fake repoId for use by _ptrToObjRef

protected:
  void setNumComponents(unsigned n);
  // May be called by derived classes to set/change the number of
  // components.
  //  Concurrency: hold DynAnyImplBase::lock (unless called from
  //                                          a constructor)

  virtual TypeCode_base* nthComponentTC(unsigned n) = 0;
  // Returns the TypeCode of the n'th component. Overriden by descendants.
  // This does not return a new reference - so it should not be
  // released. <n> MUST be in the range [0..pd_n_components).
  //  Must hold DynAnyImplBase::lock.

  CORBA::TCKind currentKind() {
    return TypeCode_base::NP_expand(nthComponentTC(pd_curr_index))->NP_kind();
  }
  // Return the alias expanded CORBA::TCKind of the current component.
  // There MUST be a valid current component.
  //  Must hold DynAnyImplBase::lock.

  CORBA::Boolean canAppendComponent(unsigned i) const {
    return i == pd_n_really_in_buf && i == pd_n_in_buf && i < pd_first_in_comp;
  }
  // True if the given component can be appended to <pd_buf>.
  // The result is only valid if <i> is in range.
  //  Must hold DynAnyImplBase::lock.

  cdrAnyMemoryStream& writeCurrent(CORBA::TCKind kind) {
    if( pd_curr_index < 0 )
      throw DynamicAny::DynAny::InvalidValue();
    if( currentKind() != kind )
      throw DynamicAny::DynAny::TypeMismatch();
    if( canAppendComponent(pd_curr_index) ) {
      pd_n_in_buf++;
      pd_n_really_in_buf++;
      return pd_buf;
    } else {
      DynAnyImpl* cc = ToDynAnyImpl(getCurrent());
      cc->pd_buf.rewindPtrs();
      cc->setValid();
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

  cdrAnyMemoryStream& readCurrent(CORBA::TCKind kind) {
    if( pd_curr_index < 0 )
      throw DynamicAny::DynAny::InvalidValue();
    if( currentKind() != kind )
      throw DynamicAny::DynAny::TypeMismatch();
    if( pd_curr_index < (int)pd_n_in_buf ) {
      if( pd_read_index != pd_curr_index )  seekTo(pd_curr_index);
      pd_read_index++;
      return pd_buf;
    }
    else if( pd_curr_index >= (int)pd_first_in_comp ) {
      DynAnyImpl* cc = ToDynAnyImpl(getCurrent());
      if( !cc->isValid() )  throw DynamicAny::DynAny::InvalidValue();
      cc->pd_buf.rewindInputPtr();
      return cc->pd_buf;
    }
    else throw DynamicAny::DynAny::InvalidValue();
#ifdef NEED_DUMMY_RETURN
    return pd_buf;
#endif
  }
  // Helper function for reading a primitive value from the current
  // component. Returns a reference to the buffer from which the
  // value may be extracted. It checks that <pd_curr_index> is valid,
  // and the component is of the type expected.
  //  Must hold DynAnyImplBase::lock.

public:
  enum SeqLocation { SEQ_HERE, SEQ_COMPONENT };

protected:
  virtual SeqLocation prepareSequenceWrite(CORBA::TCKind kind,
					   CORBA::ULong len) = 0;
  // Used by insert_..._seq functions. Check if a sequence with the
  // specified kind and length can be inserted, and prepare for the
  // write. Returns SEQ_HERE if it can be inserted in this DynAny's
  // <pd_buf>, SEQ_COMPONENT if it can be inserted in the current
  // component. Throws InvalidValue if no current component or wrong
  // length, TypeMismatch if the kind is wrong.

  virtual SeqLocation prepareSequenceRead(CORBA::TCKind kind) = 0;
  // Used by get_..._seq functions. Check if a sequence with the
  // specified kind can be read. Returns SEQ_HERE if this DynAny is a
  // sequence or array of the specified kind, SEQ_COMPONENT if the
  // current component is a sequence or array of the specified kind.
  // If there is a current component but it's not a sequence of the
  // right kind, throws TypeMismatch; if there is no current
  // component, throws InvalidValue.

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
  // Components in the range [0..pd_n_in_buf) are in <pd_buf>.
  // pd_n_in_buf <= pd_first_in_comp.

  unsigned pd_n_really_in_buf;
  // The number of components which have actually been written into
  // <pd_buf>. This value may be greater than <pd_n_components> or
  // <pd_first_in_comp>.

  unsigned pd_first_in_comp;
  // Components in the range [pd_first_in_comp..pd_n_components)
  // are in pd_components. Thus those in the range
  // [pd_n_in_buf..pd_first_in_comp) are not yet defined.

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
		      public DynamicAny::DynStruct
{
public:
  DynStructImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynStructImpl();

  /*******************
  * public interface *
  *******************/
  virtual DynamicAny::DynAny_ptr copy();

  virtual char* current_member_name();
  virtual CORBA::TCKind current_member_kind();
  virtual DynamicAny::NameValuePairSeq* get_members();
  virtual void set_members(const DynamicAny::NameValuePairSeq& value);
  virtual DynamicAny::NameDynAnyPairSeq* get_members_as_dyn_any();
  virtual void set_members_as_dyn_any(const DynamicAny::NameDynAnyPairSeq& value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual TypeCode_base* nthComponentTC(unsigned n);
  // Overrides DynAnyConstrBase

  virtual SeqLocation prepareSequenceWrite(CORBA::TCKind kind,
					   CORBA::ULong len);
  virtual SeqLocation prepareSequenceRead(CORBA::TCKind kind);

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Must not hold DynAnyImplBase::refCountLock.

  virtual void* _ptrToObjRef(const char* repoId);
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
  virtual void assign(DynamicAny::DynAny_ptr dyn_any);

  virtual void insert_boolean   (CORBA::Boolean      	value);
  virtual void insert_octet     (CORBA::Octet        	value);
  virtual void insert_char      (CORBA::Char         	value);
  virtual void insert_short     (CORBA::Short        	value);
  virtual void insert_ushort    (CORBA::UShort       	value);
  virtual void insert_long      (CORBA::Long         	value);
  virtual void insert_ulong     (CORBA::ULong        	value);
#ifndef NO_FLOAT				 	      
  virtual void insert_float     (CORBA::Float        	value);
  virtual void insert_double    (CORBA::Double       	value);
#endif						 	      
  virtual void insert_string    (const char*         	value);
  virtual void insert_reference (CORBA::Object_ptr   	value);
  virtual void insert_typecode  (CORBA::TypeCode_ptr 	value);
#ifdef HAS_LongLong				 	      
  virtual void insert_longlong  (CORBA::LongLong     	value);
  virtual void insert_ulonglong (CORBA::ULongLong    	value);
#endif						 	      
#ifdef HAS_LongDouble				 	      
  virtual void insert_longdouble(CORBA::LongDouble   	value);
#endif						 	      
  virtual void insert_wchar     (CORBA::WChar        	value);
  virtual void insert_wstring   (const CORBA::WChar* 	value);
  virtual void insert_any       (const CORBA::Any&   	value);
  virtual void insert_dyn_any   (DynamicAny::DynAny_ptr value);
  virtual void insert_val       (CORBA::ValueBase*   	 value);
  virtual void insert_abstract  (CORBA::AbstractBase_ptr value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  void set_to_initial_value();

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
			 public DynamicAny::DynEnum
{
public:
  DynUnionEnumDisc(TypeCode_base* tc, DynUnionImpl* un);
  virtual ~DynUnionEnumDisc();

  /*******************
  * public interface *
  *******************/
  virtual DynamicAny::DynAny_ptr copy();

  virtual char* get_as_string();
  virtual void set_as_string(const char* value);
  virtual CORBA::ULong get_as_ulong();
  virtual void set_as_ulong(CORBA::ULong value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  void set_to_initial_value();
  virtual void set_value(TypeCode_union::Discriminator v);
  // Must NOT hold DynAnyImplBase::lock.

  TypeCode_enum* actualTc() const {
    return (TypeCode_enum*) DynAnyImplBase::actualTc();
  }

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Must not hold DynAnyImplBase::refCountLock.

  virtual void* _ptrToObjRef(const char* repoId);
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// DynUnionImpl /////////////////////////////
//////////////////////////////////////////////////////////////////////

class DynUnionImpl : public DynAnyImplBase,
		     public DynamicAny::DynUnion
{
public:
  DynUnionImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynUnionImpl();

  /*******************
  * public interface *
  *******************/
  virtual void assign(DynamicAny::DynAny_ptr dyn_any);
  virtual DynamicAny::DynAny_ptr copy();
  virtual CORBA::Boolean equal(DynamicAny::DynAny_ptr dyn_any);

  virtual void insert_boolean   (CORBA::Boolean      	value);
  virtual void insert_octet     (CORBA::Octet        	value);
  virtual void insert_char      (CORBA::Char         	value);
  virtual void insert_short     (CORBA::Short        	value);
  virtual void insert_ushort    (CORBA::UShort       	value);
  virtual void insert_long      (CORBA::Long         	value);
  virtual void insert_ulong     (CORBA::ULong        	value);
#ifndef NO_FLOAT				 	      
  virtual void insert_float     (CORBA::Float        	value);
  virtual void insert_double    (CORBA::Double       	value);
#endif						 	      
  virtual void insert_string    (const char*         	value);
  virtual void insert_reference (CORBA::Object_ptr   	value);
  virtual void insert_typecode  (CORBA::TypeCode_ptr 	value);
#ifdef HAS_LongLong				 	      
  virtual void insert_longlong  (CORBA::LongLong     	value);
  virtual void insert_ulonglong (CORBA::ULongLong    	value);
#endif						 	      
#ifdef HAS_LongDouble				 	      
  virtual void insert_longdouble(CORBA::LongDouble   	value);
#endif						 	      
  virtual void insert_wchar     (CORBA::WChar        	value);
  virtual void insert_wstring   (const CORBA::WChar* 	value);
  virtual void insert_any       (const CORBA::Any&   	value);
  virtual void insert_dyn_any   (DynamicAny::DynAny_ptr value);
  virtual void insert_val       (CORBA::ValueBase*   	 value);
  virtual void insert_abstract  (CORBA::AbstractBase_ptr value);

  virtual CORBA::Boolean      	  get_boolean();
  virtual CORBA::Octet        	  get_octet();
  virtual CORBA::Char         	  get_char();
  virtual CORBA::Short        	  get_short();
  virtual CORBA::UShort       	  get_ushort();
  virtual CORBA::Long         	  get_long();
  virtual CORBA::ULong        	  get_ulong();
#ifndef NO_FLOAT
  virtual CORBA::Float        	  get_float();
  virtual CORBA::Double       	  get_double();
#endif			 	    
  virtual char*               	  get_string();
  virtual CORBA::Object_ptr   	  get_reference();
  virtual CORBA::TypeCode_ptr 	  get_typecode();
#ifdef HAS_LongLong
  virtual CORBA::LongLong     	  get_longlong();
  virtual CORBA::ULongLong    	  get_ulonglong();
#endif			 	    
#ifdef HAS_LongDouble
  virtual CORBA::LongDouble   	  get_longdouble();
#endif
  virtual CORBA::WChar        	  get_wchar();
  virtual CORBA::WChar*       	  get_wstring();
  virtual CORBA::Any*         	  get_any();
  virtual DynamicAny::DynAny_ptr  get_dyn_any();
  virtual CORBA::ValueBase*       get_val();
  virtual CORBA::AbstractBase_ptr get_abstract();

  virtual void insert_boolean_seq   (CORBA::BooleanSeq&    value);
  virtual void insert_octet_seq     (CORBA::OctetSeq&      value);
  virtual void insert_char_seq      (CORBA::CharSeq&       value);
  virtual void insert_short_seq     (CORBA::ShortSeq&      value);
  virtual void insert_ushort_seq    (CORBA::UShortSeq&     value);
  virtual void insert_long_seq      (CORBA::LongSeq&       value);
  virtual void insert_ulong_seq     (CORBA::ULongSeq&      value);
#ifndef NO_FLOAT
  virtual void insert_float_seq     (CORBA::FloatSeq&      value);
  virtual void insert_double_seq    (CORBA::DoubleSeq&     value);
#endif
#ifdef HAS_LongLong
  virtual void insert_longlong_seq  (CORBA::LongLongSeq&   value);
  virtual void insert_ulonglong_seq (CORBA::ULongLongSeq&  value);
#endif
#ifdef HAS_LongDouble						 
  virtual void insert_longdouble_seq(CORBA::LongDoubleSeq& value);
#endif								 
  virtual void insert_wchar_seq     (CORBA::WCharSeq&      value);

  virtual CORBA::BooleanSeq*    get_boolean_seq();
  virtual CORBA::OctetSeq*      get_octet_seq();
  virtual CORBA::CharSeq*       get_char_seq();
  virtual CORBA::ShortSeq*      get_short_seq();
  virtual CORBA::UShortSeq*     get_ushort_seq();
  virtual CORBA::LongSeq*       get_long_seq();
  virtual CORBA::ULongSeq* 	get_ulong_seq();
#ifndef NO_FLOAT
  virtual CORBA::FloatSeq* 	get_float_seq();
  virtual CORBA::DoubleSeq*     get_double_seq();
#endif
#ifdef HAS_LongLong
  virtual CORBA::LongLongSeq*   get_longlong_seq();
  virtual CORBA::ULongLongSeq*  get_ulonglong_seq();
#endif
#ifdef HAS_LongDouble
  virtual CORBA::LongDoubleSeq* get_longdouble_seq();
#endif
  virtual CORBA::WCharSeq*      get_wchar_seq();

  virtual CORBA::Boolean seek(CORBA::Long index);
  virtual void rewind();
  virtual CORBA::Boolean next();
  virtual CORBA::ULong component_count();
  virtual DynamicAny::DynAny_ptr current_component();

  virtual DynamicAny::DynAny_ptr get_discriminator();
  virtual void set_discriminator(DynamicAny::DynAny_ptr d);
  virtual void set_to_default_member();
  virtual void set_to_no_active_member();
  virtual CORBA::Boolean has_no_active_member();
  virtual CORBA::TCKind discriminator_kind();
  virtual DynamicAny::DynAny_ptr member();
  virtual char*  member_name();
  virtual CORBA::TCKind member_kind();
  virtual CORBA::Boolean is_set_to_default_member();

  /****************************
  * exposed private interface *
  ****************************/
  virtual int NP_nodetype() const;

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Must not hold DynAnyImplBase::refCountLock.

  virtual void* _ptrToObjRef(const char* repoId);

  static const char* _PD_repoId;
  // Fake repoId for use by _ptrToObjRef

  /***********
  * internal *
  ***********/
  virtual void set_to_initial_value();
  virtual int copy_to(cdrAnyMemoryStream& mbs);
  virtual int copy_from(cdrAnyMemoryStream& mbs);
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

  TypeCode_union* actualTc() const {
    return (TypeCode_union*) DynAnyImplBase::actualTc();
  }

private:
  cdrAnyMemoryStream& writeCurrent(CORBA::TCKind kind) {
    switch( pd_curr_index ) {
    case 0:
      if( kind != pd_disc_kind )  throw DynamicAny::DynAny::TypeMismatch();
      pd_disc->pd_buf.rewindPtrs();
      pd_disc->setValid();
      return pd_disc->pd_buf;
    case 1:
      if( pd_member_kind != kind )  throw DynamicAny::DynAny::TypeMismatch();
      pd_member->pd_buf.rewindPtrs();
      // Must be a DynAnyImpl ...
      ToDynAnyImpl(pd_member)->setValid();
      return pd_member->pd_buf;
    default:
      throw DynamicAny::DynAny::InvalidValue();
    }
#ifdef NEED_DUMMY_RETURN
    return pd_buf;
#endif
  }

  cdrAnyMemoryStream& readCurrent(CORBA::TCKind kind) {
    switch( pd_curr_index ) {
    case 0:
      if( kind != pd_disc_kind || !pd_disc->isValid() )
	throw DynamicAny::DynAny::TypeMismatch();
      pd_disc->pd_buf.rewindInputPtr();
      return pd_disc->pd_buf;
    case 1:
      // If !pd_member, then pd_member_kind == tk_null.
      if( pd_member_kind != kind || !ToDynAnyImpl(pd_member)->isValid() )
	throw DynamicAny::DynAny::TypeMismatch();
      pd_member->pd_buf.rewindInputPtr();
      return pd_member->pd_buf;
    default:
      throw DynamicAny::DynAny::InvalidValue();
    }
#ifdef NEED_DUMMY_RETURN
    return pd_buf;
#endif
  }

  void detachMember() {
    if( pd_member ) {
      pd_member->detach();
      pd_member->destroy();
      pd_member->_NP_decrRefCount();
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
			public DynamicAny::DynSequence
{
public:
  DynSequenceImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynSequenceImpl();

  /*******************
  * public interface *
  *******************/
  virtual DynamicAny::DynAny_ptr copy();

  virtual CORBA::ULong get_length();
  virtual void set_length (CORBA::ULong len);
  virtual DynamicAny::AnySeq* get_elements();
  virtual void set_elements(const DynamicAny::AnySeq& value);
  virtual DynamicAny::DynAnySeq* get_elements_as_dyn_any();
  virtual void set_elements_as_dyn_any(const DynamicAny::DynAnySeq& value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual int NP_nodetype() const;

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Must not hold DynAnyImplBase::refCountLock.

  virtual void* _ptrToObjRef(const char* repoId);

  /***********
  * internal *
  ***********/
  virtual int copy_to(cdrAnyMemoryStream& mbs);
  virtual int copy_from(cdrAnyMemoryStream& mbs);
  virtual TypeCode_base* nthComponentTC(unsigned n);
  // Overrides DynAnyConstrBase

  virtual SeqLocation prepareSequenceWrite(CORBA::TCKind kind,
					   CORBA::ULong len);

  virtual SeqLocation prepareSequenceRead(CORBA::TCKind kind);

private:
  CORBA::ULong pd_bound;  // 0 if unbounded sequence, bound otherwise
};


//////////////////////////////////////////////////////////////////////
/////////////////////////// DynArrayImpl /////////////////////////////
//////////////////////////////////////////////////////////////////////

class DynArrayImpl : public DynAnyConstrBase,
		     public DynamicAny::DynArray
{
public:
  DynArrayImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynArrayImpl();

  /*******************
  * public interface *
  *******************/
  virtual DynamicAny::DynAny_ptr copy();

  virtual DynamicAny::AnySeq* get_elements();
  virtual void set_elements(const DynamicAny::AnySeq& value);
  virtual DynamicAny::DynAnySeq* get_elements_as_dyn_any();
  virtual void set_elements_as_dyn_any(const DynamicAny::DynAnySeq& value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual int NP_nodetype() const;

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Must not hold DynAnyImplBase::refCountLock.

  virtual void* _ptrToObjRef(const char* repoId);

  /***********
  * internal *
  ***********/
  virtual TypeCode_base* nthComponentTC(unsigned n);
  // Overrides DynAnyConstrBase

  virtual SeqLocation prepareSequenceWrite(CORBA::TCKind kind,
					   CORBA::ULong len);

  virtual SeqLocation prepareSequenceRead(CORBA::TCKind kind);
private:
};


//////////////////////////////////////////////////////////////////////
//////////////////////////// DynValueImpl ////////////////////////////
//////////////////////////////////////////////////////////////////////

//: DynAny for valuetypes

class DynValueImpl : public DynAnyConstrBase,
		     public DynamicAny::DynValue
{
public:
  DynValueImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynValueImpl();

  /*******************
  * public interface *
  *******************/
  virtual DynamicAny::DynAny_ptr copy();

  virtual CORBA::Boolean is_null();
  virtual void set_to_null();
  virtual void set_to_value();

  virtual char* current_member_name();
  virtual CORBA::TCKind current_member_kind();
  virtual DynamicAny::NameValuePairSeq* get_members();
  virtual void set_members(const DynamicAny::NameValuePairSeq& value);
  virtual DynamicAny::NameDynAnyPairSeq* get_members_as_dyn_any();
  virtual void set_members_as_dyn_any(const DynamicAny::NameDynAnyPairSeq& value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual void set_to_initial_value();
  virtual int copy_to(cdrAnyMemoryStream& mbs);
  virtual int copy_from(cdrAnyMemoryStream& mbs);
  virtual TypeCode_base* nthComponentTC(unsigned n);
  // Overrides DynAnyConstrBase

  virtual SeqLocation prepareSequenceWrite(CORBA::TCKind kind,
					   CORBA::ULong len);
  virtual SeqLocation prepareSequenceRead(CORBA::TCKind kind);

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Must not hold DynAnyImplBase::refCountLock.

  virtual void* _ptrToObjRef(const char* repoId);

private:
  omniTypedefs::TypeCodeSeq pd_componentTCs;
  CORBA::StringSeq          pd_componentNames;
  CORBA::Boolean            pd_null;
};



//////////////////////////////////////////////////////////////////////
//////////////////////////// DynValueBoxImpl /////////////////////////
//////////////////////////////////////////////////////////////////////

//: DynAny for valueboxes

class DynValueBoxImpl : public DynAnyConstrBase,
			public DynamicAny::DynValueBox
{
public:
  DynValueBoxImpl(TypeCode_base* tc, CORBA::Boolean is_root=1);
  virtual ~DynValueBoxImpl();

  /*******************
  * public interface *
  *******************/
  virtual DynamicAny::DynAny_ptr copy();

  virtual CORBA::Boolean is_null();
  virtual void set_to_null();
  virtual void set_to_value();

  virtual CORBA::Any* get_boxed_value();
  virtual void set_boxed_value(const CORBA::Any& value);
  virtual DynamicAny::DynAny_ptr get_boxed_value_as_dyn_any();
  virtual void set_boxed_value_as_dyn_any(DynamicAny::DynAny_ptr value);

  /****************************
  * exposed private interface *
  ****************************/
  virtual int NP_nodetype() const;

  /***********
  * internal *
  ***********/
  virtual void set_to_initial_value();
  virtual int copy_to(cdrAnyMemoryStream& mbs);
  virtual int copy_from(cdrAnyMemoryStream& mbs);
  virtual TypeCode_base* nthComponentTC(unsigned n);
  // Overrides DynAnyConstrBase

  virtual SeqLocation prepareSequenceWrite(CORBA::TCKind kind,
					   CORBA::ULong len);
  virtual SeqLocation prepareSequenceRead(CORBA::TCKind kind);

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();
  // Must not hold DynAnyImplBase::refCountLock.

  virtual void* _ptrToObjRef(const char* repoId);

private:
  CORBA::Boolean pd_null;
};


//////////////////////////////////////////////////////////////////////
////////////////////////////// Narrowing /////////////////////////////
//////////////////////////////////////////////////////////////////////

// <p> must not be nil.
inline DynAnyImplBase* ToDynAnyImplBase(DynamicAny::DynAny_ptr p)
{
  DynAnyImplBase* daib =
    (DynAnyImplBase*)p->_ptrToObjRef(DynAnyImplBase::_PD_repoId);
  OMNIORB_ASSERT(daib);
  return daib;
}

// <p> must not be nil.
inline DynAnyImpl* ToDynAnyImpl(DynamicAny::DynAny_ptr p)
{
  DynAnyImpl* dai = (DynAnyImpl*)p->_ptrToObjRef(DynAnyImpl::_PD_repoId);
  OMNIORB_ASSERT(dai);
  return dai;
}

// <p> must not be nil.
inline DynAnyConstrBase* ToDynAnyConstrBase(DynamicAny::DynAny_ptr p)
{
  DynAnyConstrBase* dacb = (DynAnyConstrBase*)p->
                                   _ptrToObjRef(DynAnyConstrBase::_PD_repoId);
  OMNIORB_ASSERT(dacb);
  return dacb;
}

// <p> must not be nil.
inline DynUnionImpl* ToDynUnionImpl(DynamicAny::DynAny_ptr p)
{
  DynUnionImpl* daui = (DynUnionImpl*)p->
                                  _ptrToObjRef(DynUnionImpl::_PD_repoId);
  OMNIORB_ASSERT(daui);
  return daui;
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// DynAnyFactoryImpl ////////////////////////
//////////////////////////////////////////////////////////////////////

class DynAnyFactoryImpl : public DynamicAny::DynAnyFactory
{
public:
  DynAnyFactoryImpl() :
    OMNIORB_BASE_CTOR(DynamicAny::)DynAnyFactory(0), pd_refCount(1) {}

  virtual ~DynAnyFactoryImpl();

  // IDL defined functions
  virtual DynamicAny::DynAny_ptr
  create_dyn_any(const CORBA::Any& value);

  virtual DynamicAny::DynAny_ptr
  create_dyn_any_from_type_code(CORBA::TypeCode_ptr type);

  // Internals
  virtual void* _ptrToObjRef(const char* repoId);

  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();

  static DynamicAny::DynAnyFactory_ptr theFactory();

private:
  int pd_refCount;
};


OMNI_NAMESPACE_END(omni)

#endif  // __DYNANYIMPL_H__
