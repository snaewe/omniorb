// -*- Mode: C++; -*-
//                            Package   : omniORB
// poa_defs.h                 Created on: 8/6/99
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
  Revision 1.4  2001/02/21 14:12:21  dpg1
  Merge from omni3_develop for 3.0.3 release.

  Revision 1.1.2.5  2000/06/27 16:15:08  sll
  New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
  _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
  sequence of string and a sequence of object reference.

  Revision 1.1.2.4  1999/10/21 11:29:45  djr
  Added _core_attr to declaration of _PD_repoId in exceptions & interfaces.

  Revision 1.1.2.3  1999/10/16 13:22:52  djr
  Changes to support compiling on MSVC.

  Revision 1.1.2.2  1999/10/04 15:51:51  djr
  Various fixes/MSVC work-arounds.

  Revision 1.1.2.1  1999/09/24 09:51:50  djr
  Moved from omniORB2 + some new files.

*/

#ifndef __OMNIPOA_H__
#error poa_defs.h should only be included by poa.h
#endif

#ifndef __OMNI_POA_DEFS_H__
#define __OMNI_POA_DEFS_H__


//////////////////////////////////////////////////////////////////////
/////////////////////////// ForwardRequest ///////////////////////////
//////////////////////////////////////////////////////////////////////

class ForwardRequest : public CORBA::UserException {
public:
  CORBA::Object_Member forward_reference;

  inline ForwardRequest() {
    pd_insertToAnyFn    = insertToAnyFn;
    pd_insertToAnyFnNCP = insertToAnyFnNCP;
  }
  ForwardRequest(const ForwardRequest&);
  ForwardRequest(CORBA::Object_ptr i_forward_reference);
  ForwardRequest& operator=(const ForwardRequest&);
  virtual ~ForwardRequest();
  virtual void _raise();
  static ForwardRequest* _downcast(CORBA::Exception*);
  static const ForwardRequest* _downcast(const CORBA::Exception*);
  static inline ForwardRequest* _narrow(CORBA::Exception* e) {
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


_CORBA_MODULE_VAR _dyn_attr const CORBA::TypeCode_ptr _tc_ForwardRequest;

//////////////////////////////////////////////////////////////////////
////////////////////////// AdapterActivator //////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef __PortableServer_mAdapterActivator__
#define __PortableServer_mAdapterActivator__

class _objref_AdapterActivator;
typedef _objref_AdapterActivator* AdapterActivator_ptr;
typedef AdapterActivator_ptr AdapterActivatorRef;

class AdapterActivator_Helper {
public:
  typedef AdapterActivator_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static size_t NP_alignedSize(_ptr_type, size_t);
  static void marshalObjRef(_ptr_type, NetBufferedStream&);
  static _ptr_type unmarshalObjRef(NetBufferedStream&);
  static void marshalObjRef(_ptr_type, MemBufferedStream&);
  static _ptr_type unmarshalObjRef(MemBufferedStream&);
};

typedef _CORBA_ObjRef_Var<_objref_AdapterActivator, AdapterActivator_Helper> AdapterActivator_var;
typedef _CORBA_ObjRef_OUT_arg<_objref_AdapterActivator,AdapterActivator_Helper > AdapterActivator_out;

#endif


class AdapterActivator {
public:
  // Declarations for this interface type.
  typedef AdapterActivator_ptr _ptr_type;
  typedef AdapterActivator_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(CORBA::Object_ptr);
  static _ptr_type _nil();

  static inline size_t _alignedSize(_ptr_type, size_t);
  static inline void _marshalObjRef(_ptr_type, NetBufferedStream&);
  static inline void _marshalObjRef(_ptr_type, MemBufferedStream&);

  static inline _ptr_type _unmarshalObjRef(NetBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static inline _ptr_type _unmarshalObjRef(MemBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static _core_attr const char* _PD_repoId;

  // Other IDL defined within this scope.

};


class _objref_AdapterActivator :
  public virtual CORBA::Object, public virtual omniObjRef
{
public:
  CORBA::Boolean unknown_adapter(POA_ptr parent, const char* name);

  inline _objref_AdapterActivator() { _PR_setobj(0); }  // nil
  _objref_AdapterActivator(const char*, IOP::TaggedProfileList*, omniIdentity*, omniLocalIdentity*);

protected:
  virtual ~_objref_AdapterActivator();

private:
  virtual void* _ptrToObjRef(const char*);

  _objref_AdapterActivator(const _objref_AdapterActivator&);
  _objref_AdapterActivator& operator = (const _objref_AdapterActivator&);
  // not implemented
};


class _pof_AdapterActivator : public proxyObjectFactory {
public:
  inline _pof_AdapterActivator() : proxyObjectFactory(AdapterActivator::_PD_repoId) {}
  virtual ~_pof_AdapterActivator();

  virtual omniObjRef* newObjRef(const char*, IOP::TaggedProfileList*,
                                omniIdentity*, omniLocalIdentity*);
  virtual _CORBA_Boolean is_a(const char*) const;
};


class _impl_AdapterActivator :
  public virtual omniServant
{
public:
  virtual ~_impl_AdapterActivator();

  virtual CORBA::Boolean unknown_adapter(POA_ptr parent, const char* name) = 0;

protected:
  virtual _CORBA_Boolean _dispatch(GIOP_S&);

private:
  virtual void* _ptrToInterface(const char*);
  virtual const char* _mostDerivedRepoId();
};


_CORBA_MODULE_VAR _dyn_attr const CORBA::TypeCode_ptr _tc_AdapterActivator;

//////////////////////////////////////////////////////////////////////
/////////////////////////// ServantManager ///////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef __PortableServer_mServantManager__
#define __PortableServer_mServantManager__

class _objref_ServantManager;
typedef _objref_ServantManager* ServantManager_ptr;
typedef ServantManager_ptr ServantManagerRef;

class ServantManager_Helper {
public:
  typedef ServantManager_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static size_t NP_alignedSize(_ptr_type, size_t);
  static void marshalObjRef(_ptr_type, NetBufferedStream&);
  static _ptr_type unmarshalObjRef(NetBufferedStream&);
  static void marshalObjRef(_ptr_type, MemBufferedStream&);
  static _ptr_type unmarshalObjRef(MemBufferedStream&);
};

typedef _CORBA_ObjRef_Var<_objref_ServantManager, ServantManager_Helper> ServantManager_var;
typedef _CORBA_ObjRef_OUT_arg<_objref_ServantManager,ServantManager_Helper > ServantManager_out;

#endif


class ServantManager {
public:
  // Declarations for this interface type.
  typedef ServantManager_ptr _ptr_type;
  typedef ServantManager_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(CORBA::Object_ptr);
  static _ptr_type _nil();

  static inline size_t _alignedSize(_ptr_type, size_t);
  static inline void _marshalObjRef(_ptr_type, NetBufferedStream&);
  static inline void _marshalObjRef(_ptr_type, MemBufferedStream&);

  static inline _ptr_type _unmarshalObjRef(NetBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static inline _ptr_type _unmarshalObjRef(MemBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static _core_attr const char* _PD_repoId;

  // Other IDL defined within this scope.

};


class _objref_ServantManager :
  public virtual CORBA::Object, public virtual omniObjRef
{
public:

  inline _objref_ServantManager() { _PR_setobj(0); }  // nil
  _objref_ServantManager(const char*, IOP::TaggedProfileList*, omniIdentity*, omniLocalIdentity*);

protected:
  virtual ~_objref_ServantManager();

private:
  virtual void* _ptrToObjRef(const char*);

  _objref_ServantManager(const _objref_ServantManager&);
  _objref_ServantManager& operator = (const _objref_ServantManager&);
  // not implemented
};


class _pof_ServantManager : public proxyObjectFactory {
public:
  inline _pof_ServantManager() : proxyObjectFactory(ServantManager::_PD_repoId) {}
  virtual ~_pof_ServantManager();

  virtual omniObjRef* newObjRef(const char*, IOP::TaggedProfileList*,
                                omniIdentity*, omniLocalIdentity*);
  virtual _CORBA_Boolean is_a(const char*) const;
};


class _impl_ServantManager :
  public virtual omniServant
{
public:
  virtual ~_impl_ServantManager();


protected:
  virtual _CORBA_Boolean _dispatch(GIOP_S&);

private:
  virtual void* _ptrToInterface(const char*);
  virtual const char* _mostDerivedRepoId();
};


_CORBA_MODULE_VAR _dyn_attr const CORBA::TypeCode_ptr _tc_ServantManager;

//////////////////////////////////////////////////////////////////////
////////////////////////// ServantActivator //////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef __PortableServer_mServantActivator__
#define __PortableServer_mServantActivator__

class _objref_ServantActivator;
typedef _objref_ServantActivator* ServantActivator_ptr;
typedef ServantActivator_ptr ServantActivatorRef;

class ServantActivator_Helper {
public:
  typedef ServantActivator_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static size_t NP_alignedSize(_ptr_type, size_t);
  static void marshalObjRef(_ptr_type, NetBufferedStream&);
  static _ptr_type unmarshalObjRef(NetBufferedStream&);
  static void marshalObjRef(_ptr_type, MemBufferedStream&);
  static _ptr_type unmarshalObjRef(MemBufferedStream&);
};

typedef _CORBA_ObjRef_Var<_objref_ServantActivator, ServantActivator_Helper> ServantActivator_var;
typedef _CORBA_ObjRef_OUT_arg<_objref_ServantActivator,ServantActivator_Helper > ServantActivator_out;

#endif


class ServantActivator {
public:
  // Declarations for this interface type.
  typedef ServantActivator_ptr _ptr_type;
  typedef ServantActivator_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(CORBA::Object_ptr);
  static _ptr_type _nil();

  static inline size_t _alignedSize(_ptr_type, size_t);
  static inline void _marshalObjRef(_ptr_type, NetBufferedStream&);
  static inline void _marshalObjRef(_ptr_type, MemBufferedStream&);

  static inline _ptr_type _unmarshalObjRef(NetBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static inline _ptr_type _unmarshalObjRef(MemBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static _core_attr const char* _PD_repoId;

  // Other IDL defined within this scope.

};


class _objref_ServantActivator :
  public virtual _objref_ServantManager
{
public:
  Servant incarnate(const ObjectId& oid, POA_ptr adapter);
  void etherealize(const ObjectId& oid, POA_ptr adapter, Servant serv, CORBA::Boolean cleanup_in_progress, CORBA::Boolean remaining_activations);

  inline _objref_ServantActivator() { _PR_setobj(0); }  // nil
  _objref_ServantActivator(const char*, IOP::TaggedProfileList*, omniIdentity*, omniLocalIdentity*);

protected:
  virtual ~_objref_ServantActivator();

private:
  virtual void* _ptrToObjRef(const char*);

  _objref_ServantActivator(const _objref_ServantActivator&);
  _objref_ServantActivator& operator = (const _objref_ServantActivator&);
  // not implemented
};


class _pof_ServantActivator : public proxyObjectFactory {
public:
  inline _pof_ServantActivator() : proxyObjectFactory(ServantActivator::_PD_repoId) {}
  virtual ~_pof_ServantActivator();

  virtual omniObjRef* newObjRef(const char*, IOP::TaggedProfileList*,
                                omniIdentity*, omniLocalIdentity*);
  virtual _CORBA_Boolean is_a(const char*) const;
};


class _impl_ServantActivator :
  public virtual _impl_ServantManager
{
public:
  virtual ~_impl_ServantActivator();

  virtual Servant incarnate(const ObjectId& oid, POA_ptr adapter) = 0;
  virtual void etherealize(const ObjectId& oid, POA_ptr adapter, Servant serv, CORBA::Boolean cleanup_in_progress, CORBA::Boolean remaining_activations) = 0;

protected:
  virtual _CORBA_Boolean _dispatch(GIOP_S&);

private:
  virtual void* _ptrToInterface(const char*);
  virtual const char* _mostDerivedRepoId();
};


_CORBA_MODULE_VAR _dyn_attr const CORBA::TypeCode_ptr _tc_ServantActivator;

//////////////////////////////////////////////////////////////////////
/////////////////////////// ServantLocator ///////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef __PortableServer_mServantLocator__
#define __PortableServer_mServantLocator__

class _objref_ServantLocator;
typedef _objref_ServantLocator* ServantLocator_ptr;
typedef ServantLocator_ptr ServantLocatorRef;

class ServantLocator_Helper {
public:
  typedef ServantLocator_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static size_t NP_alignedSize(_ptr_type, size_t);
  static void marshalObjRef(_ptr_type, NetBufferedStream&);
  static _ptr_type unmarshalObjRef(NetBufferedStream&);
  static void marshalObjRef(_ptr_type, MemBufferedStream&);
  static _ptr_type unmarshalObjRef(MemBufferedStream&);
};

typedef _CORBA_ObjRef_Var<_objref_ServantLocator, ServantLocator_Helper> ServantLocator_var;
typedef _CORBA_ObjRef_OUT_arg<_objref_ServantLocator,ServantLocator_Helper > ServantLocator_out;

#endif


class ServantLocator {
public:
  // Declarations for this interface type.
  typedef ServantLocator_ptr _ptr_type;
  typedef ServantLocator_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(CORBA::Object_ptr);
  static _ptr_type _nil();

  static inline size_t _alignedSize(_ptr_type, size_t);
  static inline void _marshalObjRef(_ptr_type, NetBufferedStream&);
  static inline void _marshalObjRef(_ptr_type, MemBufferedStream&);

  static inline _ptr_type _unmarshalObjRef(NetBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static inline _ptr_type _unmarshalObjRef(MemBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static _core_attr const char* _PD_repoId;

  // Other IDL defined within this scope.

  static _dyn_attr const CORBA::TypeCode_ptr _tc_Cookie;
  typedef void* Cookie;
};


class _objref_ServantLocator :
  public virtual _objref_ServantManager
{
public:
  Servant preinvoke(const ObjectId& oid, POA_ptr adapter, const char* operation, ServantLocator::Cookie& the_cookie);
  void postinvoke(const ObjectId& oid, POA_ptr adapter, const char* operation, ServantLocator::Cookie the_cookie, Servant the_servant);

  inline _objref_ServantLocator() { _PR_setobj(0); }  // nil
  _objref_ServantLocator(const char*, IOP::TaggedProfileList*, omniIdentity*, omniLocalIdentity*);

protected:
  virtual ~_objref_ServantLocator();

private:
  virtual void* _ptrToObjRef(const char*);

  _objref_ServantLocator(const _objref_ServantLocator&);
  _objref_ServantLocator& operator = (const _objref_ServantLocator&);
  // not implemented
};


class _pof_ServantLocator : public proxyObjectFactory {
public:
  inline _pof_ServantLocator() : proxyObjectFactory(ServantLocator::_PD_repoId) {}
  virtual ~_pof_ServantLocator();

  virtual omniObjRef* newObjRef(const char*, IOP::TaggedProfileList*,
                                omniIdentity*, omniLocalIdentity*);
  virtual _CORBA_Boolean is_a(const char*) const;
};


class _impl_ServantLocator :
  public virtual _impl_ServantManager
{
public:
  virtual ~_impl_ServantLocator();

  virtual Servant preinvoke(const ObjectId& oid, POA_ptr adapter, const char* operation, ServantLocator::Cookie& the_cookie) = 0;
  virtual void postinvoke(const ObjectId& oid, POA_ptr adapter, const char* operation, ServantLocator::Cookie the_cookie, Servant the_servant) = 0;

protected:
  virtual _CORBA_Boolean _dispatch(GIOP_S&);

private:
  virtual void* _ptrToInterface(const char*);
  virtual const char* _mostDerivedRepoId();
};


_CORBA_MODULE_VAR _dyn_attr const CORBA::TypeCode_ptr _tc_ServantLocator;


#endif  // __OMNI_POA_DEFS_H__
