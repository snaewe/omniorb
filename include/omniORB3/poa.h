// -*- Mode: C++; -*-
//                            Package   : omniORB
// poa.h                      Created on: 2/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
  Revision 1.1.2.7  2000/06/27 16:15:08  sll
  New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
  _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
  sequence of string and a sequence of object reference.

  Revision 1.1.2.6  1999/11/23 10:48:07  djr
  Added _ptr_type and _var_type to POA.

  Revision 1.1.2.5  1999/10/21 11:29:45  djr
  Added _core_attr to declaration of _PD_repoId in exceptions & interfaces.

  Revision 1.1.2.4  1999/10/18 17:28:18  djr
  Fixes for building MSVC dlls.

  Revision 1.1.2.3  1999/10/16 13:22:51  djr
  Changes to support compiling on MSVC.

  Revision 1.1.2.2  1999/09/28 09:47:57  djr
  Corrected declaration of ObjectId_to_string and string_to_ObjectId.

  Revision 1.1.2.1  1999/09/24 09:51:49  djr
  Moved from omniORB2 + some new files.

*/

#ifndef __OMNIPOA_H__
#define __OMNIPOA_H__

#ifndef __OMNISERVANT_H__
#include <omniORB3/omniServant.h>
#endif
#ifndef __OMNIOBJREF_H__
#include <omniORB3/omniObjRef.h>
#endif


class GIOP_S;
class omniOrbPOA;


_CORBA_MODULE PortableServer
_CORBA_MODULE_BEG

  //////////////////////////////////////////////////////////////////////
  //////////////////////// Forward Declarations ////////////////////////
  //////////////////////////////////////////////////////////////////////

  class POA;
  typedef POA* POA_ptr;
  typedef POA_ptr POARef;

  class POA_Helper {
  public:
    typedef POA_ptr _ptr_type;
 
    static _ptr_type _nil();
    static _CORBA_Boolean is_nil(_ptr_type);
    static void release(_ptr_type);
    static void duplicate(_ptr_type);
    static inline size_t NP_alignedSize(_ptr_type, size_t) { return 0; }
    static inline void marshalObjRef(_ptr_type, NetBufferedStream&) {}
    static inline _ptr_type unmarshalObjRef(NetBufferedStream&) { return 0; }
    static inline void marshalObjRef(_ptr_type, MemBufferedStream&) {}
    static inline _ptr_type unmarshalObjRef(MemBufferedStream&) { return 0; }
  };

  class ServantBase;
  typedef ServantBase* Servant;


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// POAList //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class POAList_var;
 
  class POAList : public _CORBA_Unbounded_Sequence_ObjRef<POA, _CORBA_ObjRef_Element<POA, POA_Helper>, POA_Helper> {
  public:
    typedef POAList_var _var_type;
    inline POAList() {}
    inline POAList(const POAList& s)
      : _CORBA_Unbounded_Sequence_ObjRef<POA, _CORBA_ObjRef_Element<POA, POA_Helper>, POA_Helper>(s) {}
    inline POAList(_CORBA_ULong _max)
      : _CORBA_Unbounded_Sequence_ObjRef<POA, _CORBA_ObjRef_Element<POA, POA_Helper>, POA_Helper>(_max) {}
    inline POAList(_CORBA_ULong _max, _CORBA_ULong _len, POA_ptr* _val, _CORBA_Boolean _rel=0)
      : _CORBA_Unbounded_Sequence_ObjRef<POA, _CORBA_ObjRef_Element<POA, POA_Helper>, POA_Helper>(_max, _len, _val, _rel) {}
    inline POAList& operator = (const POAList& s) {
      _CORBA_Unbounded_Sequence_ObjRef<POA, _CORBA_ObjRef_Element<POA, POA_Helper>, POA_Helper>::operator=(s);
      return *this;
    }
  };


  class POAList_var {
  public:
    typedef POAList T;
    typedef POAList_var T_var;
 
    inline POAList_var() : pd_seq(0) {}
    inline POAList_var(T* s) : pd_seq(s) {}
    inline POAList_var(const T_var& s) {
      if( s.pd_seq )  pd_seq = new T(*s.pd_seq);
      else             pd_seq = 0;
    }
    inline ~POAList_var() { if( pd_seq )  delete pd_seq; }
 
    inline T_var& operator = (T* s) {
      if( pd_seq )  delete pd_seq;
      pd_seq = s;
      return *this;
    }
    inline T_var& operator = (const T_var& s) {
      if( s.pd_seq ) {
        if( !pd_seq )  pd_seq = new T;
        *pd_seq = *s.pd_seq;
      } else if( pd_seq ) {
        delete pd_seq;
        pd_seq = 0;
      }
      return *this;
    }
 
    inline _CORBA_ObjRef_Element<POA, POA_Helper> operator [] (_CORBA_ULong s) {
      return (*pd_seq)[s];
    }
    inline T* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
    inline operator T& () const { return *pd_seq; }
#else
    inline operator const T& () const { return *pd_seq; }
    inline operator T& () { return *pd_seq; }
#endif
 
    inline const T& in() const { return *pd_seq; }
    inline T&       inout()    { return *pd_seq; }
    inline T*&      out() {
      if( pd_seq ) { delete pd_seq; pd_seq = 0; }
      return pd_seq;
    }
    inline T* _retn() { T* tmp = pd_seq; pd_seq = 0; return tmp; }
 
  private:
    T* pd_seq;
  };

  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// ObjectId //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ObjectId_var;

  class ObjectId : public _CORBA_Unbounded_Sequence__Octet {
  public:
    typedef ObjectId_var _var_type;
    inline ObjectId() {}
    inline ObjectId(const ObjectId& seq)
      : _CORBA_Unbounded_Sequence__Octet(seq) {}
    inline ObjectId(CORBA::ULong max)
      : _CORBA_Unbounded_Sequence__Octet(max) {}
    inline ObjectId(CORBA::ULong max, CORBA::ULong len, CORBA::Octet* val, CORBA::Boolean rel=0)
      : _CORBA_Unbounded_Sequence__Octet(max, len, val, rel) {}
    inline ObjectId& operator = (const ObjectId& seq) {
      _CORBA_Unbounded_Sequence__Octet::operator=(seq);
      return *this;
    }
  };

  class ObjectId_out;

  class ObjectId_var {
  public:
    typedef ObjectId _T;
    typedef ObjectId_var _T_var;

    inline ObjectId_var() : pd_seq(0) {}
    inline ObjectId_var(_T* s) : pd_seq(s) {}
    inline ObjectId_var(const _T_var& sv) {
      if( sv.pd_seq ) {
        pd_seq = new _T;
        *pd_seq = *sv.pd_seq;
      } else
        pd_seq = 0;
    }
    inline ~ObjectId_var() { if( pd_seq ) delete pd_seq; }

    inline _T_var& operator = (_T* s) {
      if( pd_seq )  delete pd_seq;
      pd_seq = s;
      return *this;
    }
    inline _T_var& operator = (const _T_var& sv) {
      if( sv.pd_seq ) {
        if( !pd_seq )  pd_seq = new _T;
        *pd_seq = *sv.pd_seq;
      } else if( pd_seq ) {
        delete pd_seq;
        pd_seq = 0;
      }
      return *this;
    }

    inline CORBA::Octet& operator [] (_CORBA_ULong i) {   return (*pd_seq)[i]; }
    inline _T* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
    inline operator _T& () const { return *pd_seq; }
#else
    inline operator const _T& () const { return *pd_seq; }
    inline operator _T& () { return *pd_seq; }
#endif

    inline const _T& in() const { return *pd_seq; }
    inline _T& inout() { return *pd_seq; }
    inline _T*& out() { if (pd_seq) { delete pd_seq; pd_seq = 0; } return pd_seq; }
    inline _T* _retn() { _T* tmp = pd_seq; pd_seq = 0; return tmp; }

    friend class ObjectId_out;

  private:
    _T* pd_seq;
  };

  class ObjectId_out {
  public:
    typedef ObjectId _T;
    typedef ObjectId_var _T_var;

    inline ObjectId_out(_T*& s) : _data(s) { _data = 0; }
    inline ObjectId_out(_T_var& sv)
      : _data(sv.pd_seq) { sv = (_T*) 0; }
    inline ObjectId_out(const ObjectId_out& s) : _data(s._data) { }
    inline ObjectId_out& operator=(const ObjectId_out& s) { _data = s._data; return *this; }
    inline ObjectId_out& operator=(_T* s) { _data = s; return *this; }
    inline operator _T*&() { return _data; }
    inline _T*& ptr() { return _data; }
    inline _T* operator->() { return _data; }
    inline CORBA::Octet& operator [] (_CORBA_ULong i) {   return (*_data)[i]; }
    _T*& _data;

  private:
    ObjectId_out();
    ObjectId_out operator=( const _T_var&);
  };

  //////////////////////////////////////////////////////////////////////
  ////////////////////////// Policy Interfaces /////////////////////////
  //////////////////////////////////////////////////////////////////////

  _CORBA_MODULE_VARINT const CORBA::ULong
    THREAD_POLICY_ID _init_in_decl_( = 16 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    LIFESPAN_POLICY_ID _init_in_decl_( = 17 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    ID_UNIQUENESS_POLICY_ID _init_in_decl_( = 18 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    ID_ASSIGNMENT_POLICY_ID _init_in_decl_( = 19 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    IMPLICIT_ACTIVATION_POLICY_ID _init_in_decl_( = 20 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    SERVANT_RETENTION_POLICY_ID _init_in_decl_( = 21 );

  _CORBA_MODULE_VARINT const CORBA::ULong
    REQUEST_PROCESSING_POLICY_ID _init_in_decl_( = 22 );


  enum ThreadPolicyValue {
    ORB_CTRL_MODEL,  // default
    SINGLE_THREAD_MODEL
  };

  enum LifespanPolicyValue {
    TRANSIENT,  // default
    PERSISTENT
  };

  enum IdUniquenessPolicyValue {
    UNIQUE_ID,  // default
    MULTIPLE_ID
  };

  enum IdAssignmentPolicyValue {
    USER_ID,
    SYSTEM_ID  // default
  };

  enum ImplicitActivationPolicyValue {
    IMPLICIT_ACTIVATION,
    NO_IMPLICIT_ACTIVATION  // default (but not root poa)
  };

  enum ServantRetentionPolicyValue {
    RETAIN,  // default
    NON_RETAIN
  };

  enum RequestProcessingPolicyValue {
    USE_ACTIVE_OBJECT_MAP_ONLY,  // default
    USE_DEFAULT_SERVANT,
    USE_SERVANT_MANAGER
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

  OMNIORB_DECLARE_POLICY_OBJECT(ThreadPolicy, THREAD_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(LifespanPolicy, LIFESPAN_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(IdUniquenessPolicy, ID_UNIQUENESS_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(IdAssignmentPolicy, ID_ASSIGNMENT_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(ImplicitActivationPolicy,
				IMPLICIT_ACTIVATION_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(ServantRetentionPolicy,
				SERVANT_RETENTION_POLICY_ID)
  OMNIORB_DECLARE_POLICY_OBJECT(RequestProcessingPolicy,
				REQUEST_PROCESSING_POLICY_ID)

#undef OMNIORB_DECLARE_POLICY_OBJECT

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// POAManager /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class POAManager;
  typedef POAManager* POAManager_ptr;
  typedef POAManager_ptr POAManagerRef;

  class POAManager : public CORBA::Object {
  public:
    OMNIORB_DECLARE_USER_EXCEPTION(AdapterInactive, _core_attr)

    virtual void activate() = 0;
    virtual void hold_requests(CORBA::Boolean wait_for_completion) = 0;
    virtual void discard_requests(CORBA::Boolean wait_for_completion) = 0;
    virtual void deactivate(CORBA::Boolean etherealize_objects,
			    CORBA::Boolean wait_for_completion) = 0;

    enum State { HOLDING, ACTIVE, DISCARDING, INACTIVE };

    virtual State get_state() = 0;


    // CORBA Object members
    static POAManager_ptr _duplicate(POAManager_ptr);
    static POAManager_ptr _narrow(CORBA::Object_ptr);
    static POAManager_ptr _nil();

    // omniORB internal.
    static _core_attr const char* _PD_repoId;

  protected:
    inline POAManager(int is_nil = 0) {
      _PR_setobj((omniObjRef*) (is_nil ? 0:1));
    }
    virtual ~POAManager();

  private:
    POAManager(const POAManager&);
    POAManager& operator=(const POAManager&);
  };

  typedef _CORBA_PseudoObj_Var<POAManager> POAManager_var;

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////////// POA ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class _objref_AdapterActivator;
  class _objref_ServantManager;


  class POA : public CORBA::Object {
  public:

    typedef POA*                      _ptr_type;
    typedef _CORBA_PseudoObj_Var<POA> _var_type;

    // Exceptions

    OMNIORB_DECLARE_USER_EXCEPTION(AdapterAlreadyExists, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(AdapterInactive, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(AdapterNonExistent, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(NoServant, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(ObjectAlreadyActive, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(ObjectNotActive, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(ServantAlreadyActive, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(ServantNotActive, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(WrongAdapter, _core_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(WrongPolicy, _core_attr)

    class InvalidPolicy : public CORBA::UserException {
    public:
      CORBA::UShort index;

      inline InvalidPolicy() {
        pd_insertToAnyFn    = insertToAnyFn;
        pd_insertToAnyFnNCP = insertToAnyFnNCP;
      }
      InvalidPolicy(const InvalidPolicy&);
      InvalidPolicy(CORBA::UShort  i_index);
      InvalidPolicy& operator=(const InvalidPolicy&);
      virtual ~InvalidPolicy();
      virtual void _raise();
      static InvalidPolicy* _downcast(CORBA::Exception*);
      static const InvalidPolicy* _downcast(const CORBA::Exception*);
      static inline InvalidPolicy* _narrow(CORBA::Exception* e) {
	return _downcast(e);
      }

      size_t _NP_alignedSize(size_t) const;
      void operator>>=(NetBufferedStream&) const;
      void operator>>=(MemBufferedStream&) const;
      void operator<<=(NetBufferedStream&);
      void operator<<=(MemBufferedStream&);

      static _core_attr insertExceptionToAny    insertToAnyFn;
      static _core_attr insertExceptionToAnyNCP insertToAnyFnNCP;

      static _core_attr const char* _PD_repoId;

    private:
      virtual CORBA::Exception* _NP_duplicate() const;
      virtual const char* _NP_typeId() const;
      virtual const char* _NP_repoId(int*) const;
      virtual void _NP_marshal(NetBufferedStream&) const;
      virtual void _NP_marshal(MemBufferedStream&) const;
    };

    // POA creation and destruction

    virtual POA_ptr create_POA(const char* adapter_name,
			       POAManager_ptr a_POAManager,
			       const CORBA::PolicyList& policies) = 0;

    virtual POA_ptr find_POA(const char* adapter_name,
			     CORBA::Boolean activate_it) = 0;

    virtual void destroy(CORBA::Boolean etherealize_objects,
			 CORBA::Boolean wait_for_completion) = 0;

    // Factories for Policy objects

    virtual ThreadPolicy_ptr
      create_thread_policy(ThreadPolicyValue v);
    virtual LifespanPolicy_ptr
      create_lifespan_policy(LifespanPolicyValue v);
    virtual IdUniquenessPolicy_ptr
      create_id_uniqueness_policy(IdUniquenessPolicyValue v);
    virtual IdAssignmentPolicy_ptr
      create_id_assignment_policy(IdAssignmentPolicyValue v);
    virtual ImplicitActivationPolicy_ptr
      create_implicit_activation_policy(ImplicitActivationPolicyValue v);
    virtual ServantRetentionPolicy_ptr
      create_servant_retention_policy(ServantRetentionPolicyValue v);
    virtual RequestProcessingPolicy_ptr
      create_request_processing_policy(RequestProcessingPolicyValue v);

    // POA attributes

    virtual char* the_name() = 0;
    virtual POA_ptr the_parent() = 0;
    virtual POAList* the_children() = 0;
    virtual POAManager_ptr the_POAManager() = 0;
    virtual _objref_AdapterActivator* the_activator() = 0;
    virtual void the_activator(_objref_AdapterActivator* aa) = 0;

    // Servant Manager registration

    virtual _objref_ServantManager* get_servant_manager() = 0;
    virtual void set_servant_manager(_objref_ServantManager* imgr) = 0;

    // Operations for the USE_DEFAULT_SERVANT policy

    virtual Servant get_servant() = 0;
    virtual void set_servant(Servant p_servant) = 0;

    // Object activation and deactivation

    virtual ObjectId* activate_object(Servant p_servant) = 0;
    virtual void activate_object_with_id(const ObjectId& id,
					 Servant p_servant) = 0;
    virtual void deactivate_object(const ObjectId& oid) = 0;

    // Reference creation operations

    virtual CORBA::Object_ptr create_reference(const char* intf) = 0;
    virtual CORBA::Object_ptr create_reference_with_id(const ObjectId& oid,
					       const char* intf) = 0;

    // Identity mapping operations

    virtual ObjectId* servant_to_id(Servant p_servant) = 0;
    virtual CORBA::Object_ptr servant_to_reference(Servant p_servant) = 0;
    virtual Servant reference_to_servant(CORBA::Object_ptr reference) = 0;
    virtual ObjectId* reference_to_id(CORBA::Object_ptr reference) = 0;
    virtual Servant id_to_servant(const ObjectId& oid) = 0;
    virtual CORBA::Object_ptr id_to_reference(const ObjectId& oid) = 0;

    //////////////////////////
    // CORBA Object members //
    //////////////////////////

    static POA_ptr _duplicate(POA_ptr);
    static POA_ptr _narrow(CORBA::Object_ptr);
    static POA_ptr _nil();

    // omniORB internal.
    static _core_attr const char* _PD_repoId;
    static POA_ptr _the_root_poa();

  protected:
    inline POA(int nil) { _PR_setobj((omniObjRef*) (nil ? 0:1)); }
    virtual ~POA();

  private:
    POA(const POA&);
    POA& operator=(const POA&);
  };

  typedef POA::_var_type                        POA_var;
  typedef _CORBA_PseudoObj_Member<POA, POA_var> POA_member;

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// Current //////////////////////////////
  //////////////////////////////////////////////////////////////////////
#if 0
  class Current : public CORBA::Current {
  public:
    OMNIORB_DECLARE_USER_EXCEPTION(NoContext, _core_attr)

    virtual POA_ptr get_POA() = 0;
    virtual ObjectId* get_object_id() = 0;

    // CORBA Object members
    static Current_ptr _duplicate(Current_ptr);
    static Current_ptr _narrow(CORBA::Object_ptr);
    static Current_ptr _nil();

    // omniORB internal.
    static _core_attr const char* _PD_repoId;

  protected:
    inline Current() {}
    virtual ~Current();

  private:
    Current(const Current&);
    Current& operator=(const Current&);
  };
#endif
  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// ServantBase ////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ServantBase : public virtual omniServant {
  public:
    virtual ~ServantBase();

    virtual POA_ptr _default_POA();
    // May be overriden by user-defined servants to specify a
    // default POA for a particular servant.
    //  Returns the root POA by default.

    virtual CORBA::_objref_InterfaceDef* _get_interface();
    // May be overriden by user-defined servants.  Should
    // return a reference to a CORBA::InterfaceDef object
    // representing the most-derived interface of this object.
    //  The default version attempts to lookup the interface
    // in the interface repository returned by
    // CORBA::ORB::resolve_initial_references().

    //virtual CORBA::Boolean _is_a(const char* logical_type_id);
    //virtual CORBA::Boolean _non_existent();
    // These are provided by omniServant.  May be overridden
    // by application defined servants.

    virtual void _add_ref();
    virtual void _remove_ref();

  protected:
    inline ServantBase() {}
    inline ServantBase(const ServantBase&) {}
    inline ServantBase& operator = (const ServantBase&) { return *this; }

    void* _do_this(const char* repoId);

  private:
    virtual omniObjRef* _do_get_interface();
    virtual void* _downcast();
    // Overrides omniServant.
  };

  //////////////////////////////////////////////////////////////////////
  ///////////////////////// RefCountServantBase ////////////////////////
  //////////////////////////////////////////////////////////////////////

  class RefCountServantBase : public virtual ServantBase {
  public:
    virtual ~RefCountServantBase();

    virtual void _add_ref();
    virtual void _remove_ref();

  protected:
    inline RefCountServantBase() : pd_refCount(1) {}
    inline RefCountServantBase(const RefCountServantBase&) : pd_refCount(1) {}
    inline RefCountServantBase& operator = (const RefCountServantBase&)
      { return *this; }

  private:
    int pd_refCount;
  };

  //////////////////////////////////////////////////////////////////////
  /////////////////////////// ServantBase_var //////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ServantBase_var {
  public:
    typedef ServantBase*    T_ptr;
    typedef ServantBase_var T_var;

    inline ServantBase_var() : pd_data(0) {}
    inline ServantBase_var(T_ptr p) : pd_data(p) {}
    inline ServantBase_var(const T_var& v) : pd_data(v.pd_data)
      { if( pd_data )  pd_data->_add_ref(); }

    inline ~ServantBase_var() { if( pd_data )  pd_data->_remove_ref(); }

    inline T_var& operator = (T_ptr p) {
      if( pd_data )  pd_data->_remove_ref();
      pd_data = p;
      return *this;
    }
    inline T_var& operator = (T_var v) {
      if( pd_data )  pd_data->_remove_ref();
      if( (pd_data = v.pd_data) )  pd_data->_add_ref();
      return *this;
    }

    inline operator T_ptr () const    { return pd_data; }
    inline T_ptr operator -> () const { return pd_data; }

    inline T_ptr in() const { return pd_data; }
    inline T_ptr& inout() { return pd_data; }
    inline T_ptr& out() {
      if( pd_data ) { pd_data->_remove_ref();  pd_data = 0; }
      return pd_data;
    }
    inline T_ptr _retn() { T_ptr tmp = pd_data;  pd_data = 0;  return tmp; }

  private:
    T_ptr pd_data;
  };

  //////////////////////////////////////////////////////////////////////
  //////////////////////// DynamicImplementation ///////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynamicImplementation : public virtual ServantBase {
  public:
    virtual ~DynamicImplementation();

    CORBA::Object_ptr _this();

    virtual void invoke(CORBA::ServerRequest_ptr request) = 0;
    virtual char* _primary_interface(const ObjectId& oid, POA_ptr poa) = 0;

    //virtual CORBA::_objref_InterfaceDef* _get_interface();
    virtual CORBA::Boolean _is_a(const char* logical_type_id);
    // These default implementations use _primary_interface(),
    // but may be overridden by subclasses.

    // omniORB internal.
    virtual _CORBA_Boolean _dispatch(GIOP_S&);
    // DJR - NB. probably should not pass the standard object
    // ops up to the invoke routine...

  private:
    virtual omniObjRef* _do_get_interface();
  };

  //////////////////////////////////////////////////////////////////////
  //////////////////////// C++ Mapping Specific ////////////////////////
  //////////////////////////////////////////////////////////////////////

  _CORBA_MODULE_FN char* ObjectId_to_string(const ObjectId& id);
  _CORBA_MODULE_FN ObjectId* string_to_ObjectId(const char* s);


#include <omniORB3/poa_defs.h>
// This brings in the declarations for:
//  AdapterActivator
//  ServantManager
//  ServantActivator
//  ServantLocator


_CORBA_MODULE_END  // PortableServer


#include <omniORB3/poa_operators.h>
#include <omniORB3/poa_poa.h>
// This brings in the skeletons for:
//  AdapterActivator
//  ServantActivator
//  ServantLocator


#endif  // __OMNIPOA_H__
