// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyLocalObjects.cc          Created on: 2005/10/20
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2005-2008 Apasphere Ltd.
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//    This file is part of the omniORBpy library
//
//    The omniORBpy library is free software; you can redistribute it
//    and/or modify it under the terms of the GNU Lesser General
//    Public License as published by the Free Software Foundation;
//    either version 2.1 of the License, or (at your option) any later
//    version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
//    MA 02111-1307, USA
//
//
// Description:
//    Implementation of Python servant object

// $Id$

// $Log$
// Revision 1.1.2.2  2008/10/09 15:04:36  dgrisby
// Python exceptions occurring during unmarshalling were not properly
// handled. Exception state left set when at traceLevel 0 (thanks
// Morarenko Kirill).
//
// Revision 1.1.2.1  2005/11/09 12:33:32  dgrisby
// Support POA LocalObjects.
//

#include <omnipy.h>
#include <pyThreadCache.h>


//
// Local object classes
//

class Py_ServantActivatorObj :
  public virtual PortableServer::ServantActivator
{
public:
  Py_ServantActivatorObj(PyObject* pysa) : impl_(pysa), refcount_(1) { }

  virtual ~Py_ServantActivatorObj() { }

  PortableServer::Servant incarnate(const PortableServer::ObjectId& oid,
				    PortableServer::POA_ptr         poa)
  {
    return impl_.incarnate(oid, poa);
  }

  void etherealize(const PortableServer::ObjectId& oid,
		   PortableServer::POA_ptr         poa,
		   PortableServer::Servant         serv,
		   CORBA::Boolean                  cleanup_in_progress,
		   CORBA::Boolean                  remaining_activations)
  {
    impl_.etherealize(oid, poa, serv, cleanup_in_progress,
		      remaining_activations);
  }

  inline PyObject* pyobj() { return impl_.pyobj(); }

  void _add_ref();
  void _remove_ref();
  void* _ptrToObjRef(const char* id);

private:
  omniPy::Py_ServantActivator impl_;
  int refcount_;

  // Not implemented
  Py_ServantActivatorObj(const Py_ServantActivatorObj&);
  Py_ServantActivatorObj& operator=(const Py_ServantActivatorObj&);
};

void
Py_ServantActivatorObj::_add_ref()
{
  omnipyThreadCache::lock _t;
  OMNIORB_ASSERT(refcount_ > 0);
  ++refcount_;
}

void
Py_ServantActivatorObj::_remove_ref()
{
  omnipyThreadCache::lock _t;

  if (--refcount_ > 0) return;

  OMNIORB_ASSERT(refcount_ == 0);
  delete this;
}

void*
Py_ServantActivatorObj::_ptrToObjRef(const char* id)
{
  if( id == omniPy::string_Py_ServantActivator )
    return (Py_ServantActivatorObj*)this;
  
  if( id == PortableServer::ServantActivator::_PD_repoId )
    return (PortableServer::ServantActivator_ptr) this;

  if( id == PortableServer::ServantManager::_PD_repoId )
    return (PortableServer::ServantManager_ptr) this;

  if( id == CORBA::LocalObject::_PD_repoId )
    return (CORBA::LocalObject_ptr) this;

  if( id == CORBA::Object::_PD_repoId )
    return (CORBA::Object_ptr) this;

  if( omni::strMatch(id, omniPy::string_Py_ServantActivator) )
    return (Py_ServantActivatorObj*)this;

  if( omni::strMatch(id, PortableServer::ServantActivator::_PD_repoId) )
    return (PortableServer::ServantActivator_ptr) this;

  if( omni::strMatch(id, PortableServer::ServantManager::_PD_repoId) )
    return (PortableServer::ServantManager_ptr) this;

  if( omni::strMatch(id, CORBA::LocalObject::_PD_repoId) )
    return (CORBA::LocalObject_ptr) this;

  if( omni::strMatch(id, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


class Py_ServantLocatorObj :
  public virtual PortableServer::ServantLocator
{
public:
  Py_ServantLocatorObj(PyObject* pysl) : impl_(pysl), refcount_(1) { }

  virtual ~Py_ServantLocatorObj() { }

  PortableServer::Servant preinvoke(const PortableServer::ObjectId& oid,
				    PortableServer::POA_ptr         poa,
				    const char*                     operation,
				    void*&                          cookie)
  {
    return impl_.preinvoke(oid, poa, operation, cookie);
  }

  void postinvoke(const PortableServer::ObjectId& oid,
		  PortableServer::POA_ptr         poa,
		  const char*                     operation,
		  void*                           cookie,
		  PortableServer::Servant         serv)
  {
    impl_.postinvoke(oid, poa, operation, cookie, serv);
  }

  inline PyObject* pyobj() { return impl_.pyobj(); }

  void _add_ref();
  void _remove_ref();
  void* _ptrToObjRef(const char* id);

private:
  omniPy::Py_ServantLocator impl_;
  int refcount_;

  // Not implemented
  Py_ServantLocatorObj(const Py_ServantLocatorObj&);
  Py_ServantLocatorObj& operator=(const Py_ServantLocatorObj&);
};

void
Py_ServantLocatorObj::_add_ref()
{
  omnipyThreadCache::lock _t;
  OMNIORB_ASSERT(refcount_ > 0);
  ++refcount_;
}

void
Py_ServantLocatorObj::_remove_ref()
{
  omnipyThreadCache::lock _t;
  if (--refcount_ > 0) return;

  OMNIORB_ASSERT(refcount_ == 0);
  delete this;
}

void*
Py_ServantLocatorObj::_ptrToObjRef(const char* id)
{
  if( id == omniPy::string_Py_ServantLocator )
    return (Py_ServantLocatorObj*)this;
  
  if( id == PortableServer::ServantLocator::_PD_repoId )
    return (PortableServer::ServantLocator_ptr) this;

  if( id == PortableServer::ServantManager::_PD_repoId )
    return (PortableServer::ServantManager_ptr) this;

  if( id == CORBA::LocalObject::_PD_repoId )
    return (CORBA::LocalObject_ptr) this;

  if( id == CORBA::Object::_PD_repoId )
    return (CORBA::Object_ptr) this;

  if( omni::strMatch(id, omniPy::string_Py_ServantLocator) )
    return (Py_ServantLocatorObj*)this;

  if( omni::strMatch(id, PortableServer::ServantLocator::_PD_repoId) )
    return (PortableServer::ServantLocator_ptr) this;

  if( omni::strMatch(id, PortableServer::ServantManager::_PD_repoId) )
    return (PortableServer::ServantManager_ptr) this;

  if( omni::strMatch(id, CORBA::LocalObject::_PD_repoId) )
    return (CORBA::LocalObject_ptr) this;

  if( omni::strMatch(id, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


class Py_AdapterActivatorObj :
  public virtual PortableServer::AdapterActivator
{
public:
  Py_AdapterActivatorObj(PyObject* pyaa) : impl_(pyaa), refcount_(1) { }

  virtual ~Py_AdapterActivatorObj() { }

  CORBA::Boolean unknown_adapter(PortableServer::POA_ptr parent,
				 const char*             name)
  {
    return impl_.unknown_adapter(parent, name);
  }

  inline PyObject* pyobj() { return impl_.pyobj(); }

  void _add_ref();
  void _remove_ref();
  void* _ptrToObjRef(const char* id);

private:
  omniPy::Py_AdapterActivator impl_;
  int refcount_;

  // Not implemented
  Py_AdapterActivatorObj(const Py_AdapterActivatorObj&);
  Py_AdapterActivatorObj& operator=(const Py_AdapterActivatorObj&);
};


void
Py_AdapterActivatorObj::_add_ref()
{
  omnipyThreadCache::lock _t;
  OMNIORB_ASSERT(refcount_ > 0);
  ++refcount_;
}

void
Py_AdapterActivatorObj::_remove_ref()
{
  omnipyThreadCache::lock _t;
  if (--refcount_ > 0) return;

  OMNIORB_ASSERT(refcount_ == 0);
  delete this;
}

void*
Py_AdapterActivatorObj::_ptrToObjRef(const char* id)
{
  if( id == omniPy::string_Py_AdapterActivator )
    return (Py_AdapterActivatorObj*)this;
  
  if( id == PortableServer::AdapterActivator::_PD_repoId )
    return (PortableServer::AdapterActivator_ptr) this;

  if( id == CORBA::LocalObject::_PD_repoId )
    return (CORBA::LocalObject_ptr) this;

  if( id == CORBA::Object::_PD_repoId )
    return (CORBA::Object_ptr) this;

  if( omni::strMatch(id, omniPy::string_Py_AdapterActivator) )
    return (Py_AdapterActivatorObj*)this;

  if( omni::strMatch(id, PortableServer::AdapterActivator::_PD_repoId) )
    return (PortableServer::AdapterActivator_ptr) this;

  if( omni::strMatch(id, CORBA::LocalObject::_PD_repoId) )
    return (CORBA::LocalObject_ptr) this;

  if( omni::strMatch(id, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


//
// Function to create C++ local objects from Python local objects
//

CORBA::LocalObject_ptr
omniPy::getLocalObjectForPyObject(PyObject* pyobj)
{
  PyObject* pyrepoId = PyObject_GetAttrString(pyobj,(char*)"_NP_RepositoryId");

  if (!(pyrepoId && PyString_Check(pyrepoId)))
    return 0;

  PyRefHolder holder(pyrepoId);
  const char* repoId = PyString_AS_STRING(pyrepoId);

  if (omni::ptrStrMatch(repoId, PortableServer::ServantActivator::_PD_repoId))
    return new Py_ServantActivatorObj(pyobj);

  if (omni::ptrStrMatch(repoId, PortableServer::ServantLocator::_PD_repoId))
    return new Py_ServantLocatorObj(pyobj);

  if (omni::ptrStrMatch(repoId, PortableServer::AdapterActivator::_PD_repoId))
    return new Py_AdapterActivatorObj(pyobj);

  return 0;
}


//
// Function to return Python object for a local object
//

PyObject*
omniPy::getPyObjectForLocalObject(CORBA::LocalObject_ptr lobj)
{
  PyObject* pyobj;

  void* vp;

  if ((vp = lobj->_ptrToObjRef(omniPy::string_Py_ServantActivator)))
    pyobj = ((Py_ServantActivatorObj*)vp)->pyobj();

  else if ((vp = lobj->_ptrToObjRef(omniPy::string_Py_ServantLocator)))
    pyobj = ((Py_ServantLocatorObj*)vp)->pyobj();

  else if ((vp = lobj->_ptrToObjRef(omniPy::string_Py_AdapterActivator)))
    pyobj = ((Py_AdapterActivatorObj*)vp)->pyobj();

  else
    OMNIORB_THROW(INV_OBJREF, INV_OBJREF_NoPythonTypeForPseudoObj,
		  CORBA::COMPLETED_NO);

  Py_INCREF(pyobj);
  return pyobj;
}


//
// Underlying ServantManager and AdapterActivator implementations
//

PortableServer::Servant
omniPy::
Py_ServantActivator::incarnate(const PortableServer::ObjectId& oid,
			       PortableServer::POA_ptr         poa)
{
  PyObject *method, *argtuple, *pyservant;
  omnipyThreadCache::lock _t;

  method = PyObject_GetAttrString(pysa_, (char*)"incarnate");
  if (!method) {
    PyErr_Clear();
    OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_NoPythonMethod,
		  CORBA::COMPLETED_MAYBE);
  }
  PortableServer::POA::_duplicate(poa);
  argtuple = Py_BuildValue((char*)"s#N",
			   (const char*)oid.NP_data(), oid.length(),
			   omniPy::createPyPOAObject(poa));

  // Do the up-call
  pyservant = PyEval_CallObject(method, argtuple);
  Py_DECREF(method);
  Py_DECREF(argtuple);

  if (pyservant) {
    omniPy::Py_omniServant* servant = omniPy::getServantForPyObject(pyservant);
    Py_DECREF(pyservant);

    if (servant)
      return servant;
    else
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
  }
  else {
    // An exception of some sort was thrown
    PyObject *etype, *evalue, *etraceback;
    PyObject *erepoId = 0;
    PyErr_Fetch(&etype, &evalue, &etraceback);
    PyErr_NormalizeException(&etype, &evalue, &etraceback);
    OMNIORB_ASSERT(etype);

    if (evalue)
      erepoId = PyObject_GetAttrString(evalue, (char*)"_NP_RepositoryId");

    if (!(erepoId && PyString_Check(erepoId))) {
      PyErr_Clear();
      Py_XDECREF(erepoId);
      if (omniORB::trace(1)) {
	{
	  omniORB::logger l;
	  l << "Caught an unexpected Python exception during up-call.\n";
	}
	PyErr_Restore(etype, evalue, etraceback);
	PyErr_Print();
      }
      else {
        Py_DECREF(etype); Py_XDECREF(evalue); Py_XDECREF(etraceback);
      }
      OMNIORB_THROW(UNKNOWN, UNKNOWN_PythonException, CORBA::COMPLETED_MAYBE);
    }

    if (omni::strMatch(PyString_AS_STRING(erepoId),
		       PortableServer::ForwardRequest::_PD_repoId)) {
      Py_DECREF(erepoId); Py_DECREF(etype); Py_XDECREF(etraceback);
      PyObject* pyfr = PyObject_GetAttrString(evalue,
					      (char*)"forward_reference");
      Py_DECREF(evalue);
      if (pyfr) {
	CORBA::Object_ptr fr = (CORBA::Object_ptr)omniPy::getTwin(pyfr,
								  OBJREF_TWIN);
	if (fr) {
	  PortableServer::ForwardRequest ex(fr);
	  Py_DECREF(pyfr);
	  throw ex;
	}
      }
      else {
	PyErr_Clear();
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType,
		      CORBA::COMPLETED_NO);
      }
    }

    // Is it a LOCATION_FORWARD?
    if (omni::strMatch(PyString_AS_STRING(erepoId),
		       "omniORB.LOCATION_FORWARD")) {
      Py_DECREF(erepoId); Py_DECREF(etype); Py_XDECREF(etraceback);
      omniPy::handleLocationForward(evalue);
    }

    // System exception or unknown user exception
    omniPy::produceSystemException(evalue, erepoId, etype, etraceback);
  }
  OMNIORB_ASSERT(0); // Never reach here
  return 0;
}


void
omniPy::
Py_ServantActivator::etherealize(const PortableServer::ObjectId& oid,
				 PortableServer::POA_ptr poa,
				 PortableServer::Servant serv,
				 CORBA::Boolean          cleanup_in_progress,
				 CORBA::Boolean          remaining_activations)
{
  PyObject *method, *argtuple, *result;
  omnipyThreadCache::lock _t;

  omniPy::Py_omniServant* pyos;
  pyos = (omniPy::Py_omniServant*)serv->
                                _ptrToInterface(omniPy::string_Py_omniServant);
  if (!pyos) {
    omniPy::InterpreterUnlocker _u;
    serv->_remove_ref();
    OMNIORB_THROW(OBJ_ADAPTER, OBJ_ADAPTER_IncompatibleServant,
		  CORBA::COMPLETED_NO);
  }

  method = PyObject_GetAttrString(pysa_, (char*)"etherealize");
  if (!method) {
    PyErr_Clear();
    omniPy::InterpreterUnlocker _u;
    serv->_remove_ref();
    OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_NoPythonMethod,
		  CORBA::COMPLETED_NO);
  }
  PortableServer::POA::_duplicate(poa);
  argtuple = Py_BuildValue((char*)"s#NNii",
			   (const char*)oid.NP_data(), oid.length(),
			   omniPy::createPyPOAObject(poa),
			   pyos->pyServant(),
			   (int)cleanup_in_progress,
			   (int)remaining_activations);
  // Do the up-call
  result = PyEval_CallObject(method, argtuple);
  Py_DECREF(method);
  Py_DECREF(argtuple);

  pyos->_locked_remove_ref();

  if (result)
    Py_DECREF(result);
  else {
    omniORB::logs(5, "omniORBpy: Servant etherealization "
		  "raised an exception!");

    if (omniORB::trace(10)) {
      omniORB::logs(10, "omniORBpy: Traceback follows:");
      PyErr_Print();
    }
    else
      PyErr_Clear();
  }
}



PortableServer::Servant
omniPy::
Py_ServantLocator::preinvoke(const PortableServer::ObjectId& oid,
			     PortableServer::POA_ptr         poa,
			     const char*                     operation,
			     void*&                          cookie)
{
  PyObject *method, *argtuple, *rettuple, *pyservant, *pycookie;
  omnipyThreadCache::lock _t;

  method = PyObject_GetAttrString(pysl_, (char*)"preinvoke");
  if (!method) {
    PyErr_Clear();
    OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_NoPythonMethod,
		  CORBA::COMPLETED_NO);
  }
  PortableServer::POA::_duplicate(poa);
  argtuple = Py_BuildValue((char*)"s#Ns",
			   (const char*)oid.NP_data(), oid.length(),
			   omniPy::createPyPOAObject(poa),
			   operation);

  // Do the up-call
  rettuple = PyEval_CallObject(method, argtuple);
  Py_DECREF(method);
  Py_DECREF(argtuple);

  if (rettuple) {
    if (PyTuple_Size(rettuple) != 2) {
      Py_DECREF(rettuple);
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
    }
    pyservant = PyTuple_GET_ITEM(rettuple, 0);
    pycookie  = PyTuple_GET_ITEM(rettuple, 1);

    omniPy::Py_omniServant* servant = omniPy::getServantForPyObject(pyservant);

    if (servant) {
      Py_INCREF(pycookie);
      cookie = pycookie;
      Py_DECREF(rettuple);
      return servant;
    }
    else {
      Py_DECREF(rettuple);
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
    }
  }
  else {
    // An exception of some sort was thrown
    PyObject *etype, *evalue, *etraceback;
    PyObject *erepoId = 0;
    PyErr_Fetch(&etype, &evalue, &etraceback);
    PyErr_NormalizeException(&etype, &evalue, &etraceback);
    OMNIORB_ASSERT(etype);

    if (evalue)
      erepoId = PyObject_GetAttrString(evalue, (char*)"_NP_RepositoryId");

    if (!(erepoId && PyString_Check(erepoId))) {
      PyErr_Clear();
      Py_XDECREF(erepoId);
      if (omniORB::trace(1)) {
	{
	  omniORB::logger l;
	  l << "Caught an unexpected Python exception during up-call.\n";
	}
	PyErr_Restore(etype, evalue, etraceback);
	PyErr_Print();
      }
      else {
	Py_DECREF(etype); Py_XDECREF(evalue); Py_XDECREF(etraceback);
      }
      OMNIORB_THROW(UNKNOWN, UNKNOWN_PythonException, CORBA::COMPLETED_MAYBE);
    }

    if (omni::strMatch(PyString_AS_STRING(erepoId),
		       PortableServer::ForwardRequest::_PD_repoId)) {
      Py_DECREF(erepoId); Py_DECREF(etype); Py_XDECREF(etraceback);
      PyObject* pyfr = PyObject_GetAttrString(evalue,
					      (char*)"forward_reference");
      Py_DECREF(evalue);
      if (pyfr) {
	CORBA::Object_ptr fr = (CORBA::Object_ptr)omniPy::getTwin(pyfr,
								  OBJREF_TWIN);
	if (fr) {
	  PortableServer::ForwardRequest ex(fr);
	  Py_DECREF(pyfr);
	  throw ex;
	}
      }
      else {
	PyErr_Clear();
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType,
		      CORBA::COMPLETED_NO);
      }
    }
    // Is it a LOCATION_FORWARD?
    if (omni::strMatch(PyString_AS_STRING(erepoId),
		       "omniORB.LOCATION_FORWARD")) {
      Py_DECREF(erepoId); Py_DECREF(etype); Py_XDECREF(etraceback);
      omniPy::handleLocationForward(evalue);
    }

    // System exception or unknown user exception
    omniPy::produceSystemException(evalue, erepoId, etype, etraceback);
  }
  OMNIORB_ASSERT(0); // Never reach here
  return 0;
}


void
omniPy::
Py_ServantLocator::postinvoke(const PortableServer::ObjectId& oid,
			      PortableServer::POA_ptr         poa,
			      const char*                     operation,
			      void*                           cookie,
			      PortableServer::Servant         serv)
{
  PyObject *method, *argtuple, *result;
  omnipyThreadCache::lock _t;

  omniPy::Py_omniServant* pyos;
  pyos = (omniPy::Py_omniServant*)serv->
                                _ptrToInterface(omniPy::string_Py_omniServant);
  if (!pyos) {
    omniPy::InterpreterUnlocker _u;
    serv->_remove_ref();
    OMNIORB_THROW(OBJ_ADAPTER, OBJ_ADAPTER_IncompatibleServant,
		  CORBA::COMPLETED_NO);
  }

  method = PyObject_GetAttrString(pysl_, (char*)"postinvoke");
  if (!method) {
    PyErr_Clear();
    omniPy::InterpreterUnlocker _u;
    serv->_remove_ref();
    OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_NoPythonMethod,
		  CORBA::COMPLETED_NO);
  }
  PortableServer::POA::_duplicate(poa);
  argtuple = Py_BuildValue((char*)"s#NsNN",
			   (const char*)oid.NP_data(), oid.length(),
			   omniPy::createPyPOAObject(poa),
			   operation,
			   (PyObject*)cookie,
			   pyos->pyServant());
  // Do the up-call
  result = PyEval_CallObject(method, argtuple);
  Py_DECREF(method);
  Py_DECREF(argtuple);

  pyos->_locked_remove_ref();

  if (result) {
    Py_DECREF(result);
    return;
  }
  else {
    omniPy::handlePythonException();
  }
}


CORBA::Boolean
omniPy::
Py_AdapterActivator::unknown_adapter(PortableServer::POA_ptr parent,
				     const char*             name)
{
  PyObject *method, *argtuple, *pyresult;
  omnipyThreadCache::lock _t;

  method = PyObject_GetAttrString(pyaa_, (char*)"unknown_adapter");
  if (!method) {
    PyErr_Clear();
    OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_NoPythonMethod,
		  CORBA::COMPLETED_NO);
  }
  PortableServer::POA::_duplicate(parent);
  argtuple = Py_BuildValue((char*)"Ns",
			   omniPy::createPyPOAObject(parent), name);

  // Do the up-call
  pyresult = PyEval_CallObject(method, argtuple);
  Py_DECREF(method);
  Py_DECREF(argtuple);

  if (pyresult) {
    if (!PyInt_Check(pyresult)) {
      Py_DECREF(pyresult);
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
    }
    CORBA::Boolean result = PyInt_AS_LONG(pyresult);
    Py_DECREF(pyresult);
    return result;
  }
  else {
    omniORB::logs(5, "omniORBpy: AdapterActivator::unknown_adapter "
		  "raised an exception!");

    if (omniORB::trace(10)) {
      omniORB::logs(10, "omniORBpy: Traceback follows:");
      PyErr_Print();
    }
    else
      PyErr_Clear();
  }
  return 0;
}
