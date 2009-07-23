// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyServant.cc               Created on: 1999/07/29
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2008 Apasphere Ltd
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
// Revision 1.1.4.12  2009/05/06 16:50:28  dgrisby
// Updated copyright.
//
// Revision 1.1.4.11  2008/10/09 15:04:36  dgrisby
// Python exceptions occurring during unmarshalling were not properly
// handled. Exception state left set when at traceLevel 0 (thanks
// Morarenko Kirill).
//
// Revision 1.1.4.10  2008/02/01 16:29:17  dgrisby
// Error with implementation of operations with names clashing with
// Python keywords.
//
// Revision 1.1.4.9  2006/07/05 10:47:14  dgrisby
// Propagate exceptions out of _default_POA.
//
// Revision 1.1.4.8  2005/11/09 12:33:32  dgrisby
// Support POA LocalObjects.
//
// Revision 1.1.4.7  2005/07/22 17:41:07  dgrisby
// Update from omnipy2_develop.
//
// Revision 1.1.4.6  2005/06/24 17:36:00  dgrisby
// Support for receiving valuetypes inside Anys; relax requirement for
// old style classes in a lot of places.
//
// Revision 1.1.4.5  2005/04/14 13:50:59  dgrisby
// New traceTime, traceInvocationReturns functions; removal of omniORB::logf.
//
// Revision 1.1.4.4  2005/01/25 11:45:48  dgrisby
// Merge from omnipy2_develop; set RPM version.
//
// Revision 1.1.4.3  2005/01/07 00:22:33  dgrisby
// Big merge from omnipy2_develop.
//
// Revision 1.1.4.2  2003/05/20 17:10:24  dgrisby
// Preliminary valuetype support.
//
// Revision 1.1.4.1  2003/03/23 21:51:57  dgrisby
// New omnipy3_develop branch.
//
// Revision 1.1.2.13  2003/01/27 11:56:58  dgrisby
// Correctly handle invalid returns from application code.
//
// Revision 1.1.2.12  2002/03/18 12:40:38  dpg1
// Support overriding _non_existent.
//
// Revision 1.1.2.11  2002/03/11 15:40:04  dpg1
// _get_interface support, exception minor codes.
//
// Revision 1.1.2.10  2002/01/18 15:49:44  dpg1
// Context support. New system exception construction. Fix None call problem.
//
// Revision 1.1.2.9  2001/09/24 10:48:28  dpg1
// Meaningful minor codes.
//
// Revision 1.1.2.8  2001/06/15 10:59:26  dpg1
// Apply fixes from omnipy1_develop.
//
// Revision 1.1.2.7  2001/06/01 11:09:26  dpg1
// Make use of new omni::ptrStrCmp() and omni::strCmp().
//
// Revision 1.1.2.6  2001/05/29 17:10:14  dpg1
// Support for in process identity.
//
// Revision 1.1.2.5  2001/05/14 12:47:22  dpg1
// Fix memory leaks.
//
// Revision 1.1.2.4  2001/05/10 15:16:03  dpg1
// Big update to support new omniORB 4 internals.
//
// Revision 1.1.2.3  2001/03/13 10:38:08  dpg1
// Fixes from omnipy1_develop
//
// Revision 1.1.2.2  2000/12/04 18:57:24  dpg1
// Fix deadlock when trying to lock omniORB internal lock while holding
// the Python interpreter lock.
//
// Revision 1.1.2.1  2000/10/13 13:55:27  dpg1
// Initial support for omniORB 4.
//


#include <omnipy.h>
#include <pyThreadCache.h>

#include <omniORB4/callHandle.h>
#include <omniORB4/IOP_S.h>


// Implementation classes for ServantManagers and AdapterActivator

class Py_ServantActivatorSvt :
  public virtual POA_PortableServer::ServantActivator,
  public virtual omniPy::Py_omniServant
{
public:
  Py_ServantActivatorSvt(PyObject* pysa, PyObject* opdict, const char* repoId)
    : PY_OMNISERVANT_BASE(pysa, opdict, repoId), impl_(pysa) { }

  virtual ~Py_ServantActivatorSvt() { }

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

  void* _ptrToInterface(const char* repoId);

  CORBA::Boolean _is_a(const char* logical_type_id) {
    return PY_OMNISERVANT_BASE::_is_a(logical_type_id);
  }
  PortableServer::POA_ptr _default_POA() {
    return PY_OMNISERVANT_BASE::_default_POA();
  }
  const char* _mostDerivedRepoId() {
    return PY_OMNISERVANT_BASE::_mostDerivedRepoId();
  }
  CORBA::Boolean _dispatch(omniCallHandle& handle) {
    return PY_OMNISERVANT_BASE::_dispatch(handle);
  }

private:
  omniPy::Py_ServantActivator impl_;

  // Not implemented
  Py_ServantActivatorSvt(const Py_ServantActivatorSvt&);
  Py_ServantActivatorSvt& operator=(const Py_ServantActivatorSvt&);
};

class Py_ServantLocatorSvt :
  public virtual POA_PortableServer::ServantLocator,
  public virtual omniPy::Py_omniServant
{
public:
  Py_ServantLocatorSvt(PyObject* pysl, PyObject* opdict, const char* repoId)
    : PY_OMNISERVANT_BASE(pysl, opdict, repoId), impl_(pysl) { }

  virtual ~Py_ServantLocatorSvt() { }

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

  void* _ptrToInterface(const char* repoId);

  CORBA::Boolean _is_a(const char* logical_type_id) {
    return PY_OMNISERVANT_BASE::_is_a(logical_type_id);
  }
  PortableServer::POA_ptr _default_POA() {
    return PY_OMNISERVANT_BASE::_default_POA();
  }
  const char* _mostDerivedRepoId() {
    return PY_OMNISERVANT_BASE::_mostDerivedRepoId();
  }
  CORBA::Boolean _dispatch(omniCallHandle& handle) {
    return PY_OMNISERVANT_BASE::_dispatch(handle);
  }

private:
  omniPy::Py_ServantLocator impl_;

  // Not implemented
  Py_ServantLocatorSvt(const Py_ServantLocatorSvt&);
  Py_ServantLocatorSvt& operator=(const Py_ServantLocatorSvt&);
};


class Py_AdapterActivatorSvt :
  public virtual POA_PortableServer::AdapterActivator,
  public virtual omniPy::Py_omniServant
{
public:
  Py_AdapterActivatorSvt(PyObject* pyaa, PyObject* opdict, const char* repoId)
    : PY_OMNISERVANT_BASE(pyaa, opdict, repoId), impl_(pyaa) { }

  virtual ~Py_AdapterActivatorSvt() { }

  CORBA::Boolean unknown_adapter(PortableServer::POA_ptr parent,
				 const char*             name)
  {
    return impl_.unknown_adapter(parent, name);
  }

  void* _ptrToInterface(const char* repoId);

  CORBA::Boolean _is_a(const char* logical_type_id) {
    return PY_OMNISERVANT_BASE::_is_a(logical_type_id);
  }
  PortableServer::POA_ptr _default_POA() {
    return PY_OMNISERVANT_BASE::_default_POA();
  }
  const char* _mostDerivedRepoId() {
    return PY_OMNISERVANT_BASE::_mostDerivedRepoId();
  }
  CORBA::Boolean _dispatch(omniCallHandle& handle) {
    return PY_OMNISERVANT_BASE::_dispatch(handle);
  }

private:
  omniPy::Py_AdapterActivator impl_;

  // Not implemented
  Py_AdapterActivatorSvt(const Py_AdapterActivatorSvt&);
  Py_AdapterActivatorSvt& operator=(const Py_AdapterActivatorSvt&);
};



// Implementation of Py_omniServant

omniPy::
Py_omniServant::Py_omniServant(PyObject* pyservant, PyObject* opdict,
			       const char* repoId)
  : pyservant_(pyservant), opdict_(opdict), refcount_(1)
{
  repoId_ = CORBA::string_dup(repoId);

  OMNIORB_ASSERT(PyDict_Check(opdict));
  Py_INCREF(pyservant_);
  Py_INCREF(opdict_);

  pyskeleton_ = PyObject_GetAttrString(pyservant_, (char*)"_omni_skeleton");
  OMNIORB_ASSERT(pyskeleton_);

  omniPy::setTwin(pyservant, (omniPy::Py_omniServant*)this, SERVANT_TWIN);
}

omniPy::
Py_omniServant::~Py_omniServant()
{
  omniPy::remTwin(pyservant_, SERVANT_TWIN);
  Py_DECREF(pyservant_);
  Py_DECREF(opdict_);
  Py_DECREF(pyskeleton_);
  CORBA::string_free(repoId_);
}


void
omniPy::
Py_omniServant::_add_ref()
{
  omnipyThreadCache::lock _t;
  OMNIORB_ASSERT(refcount_ > 0);
  ++refcount_;
}

void
omniPy::
Py_omniServant::_locked_add_ref()
{
  OMNIORB_ASSERT(refcount_ > 0);
  ++refcount_;
}

void
omniPy::
Py_omniServant::_remove_ref()
{
  omnipyThreadCache::lock _t;
  if (--refcount_ > 0) return;

  OMNIORB_ASSERT(refcount_ == 0);
  delete this;
}

void
omniPy::
Py_omniServant::_locked_remove_ref()
{
  if (--refcount_ > 0) return;

  OMNIORB_ASSERT(refcount_ == 0);
  delete this;
}


void*
omniPy::
Py_omniServant::_ptrToInterface(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if (omni::ptrStrMatch(repoId, omniPy::string_Py_omniServant))
    return (Py_omniServant*)this;

  if (omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId))
    return (void*)1;

  return 0;
}


const char*
omniPy::
Py_omniServant::_mostDerivedRepoId()
{
  return repoId_;
}


PortableServer::POA_ptr
omniPy::
Py_omniServant::_default_POA()
{
  {
    omnipyThreadCache::lock _t;
    PyObject* pyPOA = PyObject_CallMethod(pyservant_,
					  (char*)"_default_POA", 0);
    if (pyPOA) {
      PortableServer::POA_ptr poa =
	(PortableServer::POA_ptr)omniPy::getTwin(pyPOA, POA_TWIN);

      Py_DECREF(pyPOA);
      if (poa) {
	return PortableServer::POA::_duplicate(poa);
      }
      else {
        omniORB::logs(1, "Python servant returned an invalid object from "
                      "_default_POA.");
        OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType,
                      CORBA::COMPLETED_NO);
      }      
    }
    else {
      // The call raised a Python exception
      omniORB::logs(1, "Python servant raised an exception in _default_POA.");
      omniPy::handlePythonException();
    }
  }
  return 0;
}

CORBA::Boolean
omniPy::
Py_omniServant::_non_existent()
{
  omnipyThreadCache::lock _t;
  PyObject* result = PyObject_CallMethod(pyservant_,
					 (char*)"_non_existent", 0);
  if (!result) {
    if (omniORB::trace(1)) {
      {
	omniORB::logger l;
	l << "Exception trying to call _non_existent. Raising UNKNOWN.\n";
      }
      PyErr_Print();
    }
    else {
      PyErr_Clear();
    }
    OMNIORB_THROW(UNKNOWN, UNKNOWN_PythonException, CORBA::COMPLETED_NO);
  }

  if (!PyInt_Check(result))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);

  long i = PyInt_AS_LONG(result);
  Py_DECREF(result);
  return i ? 1 : 0;
}


PyObject*
omniPy::
Py_omniServant::py_this()
{
  CORBA::Object_ptr lobjref;
  {
    omniPy::InterpreterUnlocker _u;
    {
      CORBA::Object_var objref;
      objref  = (CORBA::Object_ptr)_do_this(CORBA::Object::_PD_repoId);
      lobjref = omniPy::makeLocalObjRef(repoId_, objref);
    }
  }
  return omniPy::createPyCorbaObjRef(repoId_, lobjref);
}


CORBA::Boolean
omniPy::
Py_omniServant::_is_a(const char* logical_type_id)
{
  if (omni::ptrStrMatch(logical_type_id, repoId_))
    return 1;
  else if (omni::ptrStrMatch(logical_type_id, CORBA::Object::_PD_repoId))
    return 1;
  else {
    omnipyThreadCache::lock _t;
    PyObject* pyisa = PyObject_CallMethod(omniPy::pyomniORBmodule,
					  (char*)"static_is_a", (char*)"Os",
					  pyskeleton_, logical_type_id);
    if (!pyisa) {
      if (omniORB::trace(1))
        PyErr_Print();
      else
        PyErr_Clear();

      OMNIORB_THROW(UNKNOWN, UNKNOWN_PythonException, CORBA::COMPLETED_NO);
    }

    OMNIORB_ASSERT(PyInt_Check(pyisa));

    CORBA::Boolean isa = PyInt_AS_LONG(pyisa);
    Py_DECREF(pyisa);

    if (isa)
      return 1;

    // Last resort -- does the servant have an _is_a method?
    if (PyObject_HasAttrString(pyservant_, (char*)"_is_a")) {

      pyisa = PyObject_CallMethod(pyservant_, (char*)"_is_a",
				  (char*)"s", logical_type_id);

      if (pyisa && PyInt_Check(pyisa)) {
	CORBA::Boolean isa = PyInt_AS_LONG(pyisa);
	Py_DECREF(pyisa);
	return isa;
      }
      if (!pyisa) {
	omniPy::handlePythonException();
      }
    }
  }
  return 0;
}


CORBA::Boolean
omniPy::
Py_omniServant::_dispatch(omniCallHandle& handle)
{
  int i;
  omnipyThreadCache::lock _t;

  const char* op   = handle.operation_name();
  PyObject*   desc = PyDict_GetItemString(opdict_, (char*)op);

  if (!desc) {
    if (omni::strMatch(op, "_interface")) {
      // Special case: _interface call maps to _get_interface
      //
      // If the IR stubs have been loaded, the descriptor for the
      // _interface call will be in the CORBA module. If they have not
      // been loaded, there will be no descriptor, and we fall back to
      // the ORB core version of _interface.

      desc = PyObject_GetAttrString(omniPy::pyCORBAmodule,
				    (char*)"_d_Object_interface");
      if (desc)
	Py_DECREF(desc);
      else
	PyErr_Clear();
    }
    if (!desc)
      return 0; // Unknown operation name
  }

  OMNIORB_ASSERT(PyTuple_Check(desc));

  PyObject* in_d  = PyTuple_GET_ITEM(desc,0);
  PyObject* out_d = PyTuple_GET_ITEM(desc,1);
  PyObject* exc_d = PyTuple_GET_ITEM(desc,2);
  PyObject* ctxt_d;

  if (PyTuple_GET_SIZE(desc) == 4)
    ctxt_d = PyTuple_GET_ITEM(desc,3);
  else
    ctxt_d = 0;

  Py_omniCallDescriptor call_desc(op, 0,
				  (out_d == Py_None),
				  in_d, out_d, exc_d, ctxt_d, 0, 1);
  try {
    omniPy::InterpreterUnlocker _u;
    handle.upcall(this, call_desc);
  }
  catch (omniPy::PyUserException& ex) {
    if (handle.iop_s()) {
      try {
	omniPy::InterpreterUnlocker _u;
	handle.iop_s()->SendException(&ex);
      }
      catch (...) {
	ex.decrefPyException();
	throw;
      }
      ex.decrefPyException();
    }
    else
      throw;
  }
  return 1;
}

void
omniPy::
Py_omniServant::remote_dispatch(Py_omniCallDescriptor* pycd)
{
  const char* op     = pycd->op();
  PyObject*   method = PyObject_GetAttrString(pyservant_, (char*)op);

  if (!method) {
    PyErr_Clear();
    PyObject* word = PyDict_GetItemString(omniPy::pyomniORBwordMap, op);
    if (word) {
      // Keyword -- look up mangled name
      method = PyObject_GetAttr(pyservant_, word);
    }
    else if (omni::strMatch(op, "_interface")) {
      method = PyObject_GetAttrString(pyservant_, (char*)"_get_interface");
    }
    if (!method) {
      if (omniORB::trace(1)) {
	omniORB::logger l;
	l << "Python servant for `" << repoId_ << "' has no method named `"
	  << op << "'.\n";
      }
      PyErr_Clear();
      OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_NoPythonMethod,
		    CORBA::COMPLETED_NO);
    }
  }

  PyObject* args   = pycd->args();
  PyObject* result = PyEval_CallObject(method, args);
  Py_DECREF(method);

  if (result) {
    // No exception was thrown. Set the return value
    pycd->setAndValidateReturnedValues(result);
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

    PyObject* exc_d = pycd->exc_d_;

    // Is it a user exception?
    if (exc_d != Py_None) {
      OMNIORB_ASSERT(PyDict_Check(exc_d));

      PyObject* edesc = PyDict_GetItem(exc_d, erepoId);

      if (edesc) {
	Py_DECREF(erepoId); Py_DECREF(etype); Py_XDECREF(etraceback);
	PyUserException ex(edesc, evalue, CORBA::COMPLETED_MAYBE);
	ex._raise();
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
}


void
omniPy::
Py_omniServant::local_dispatch(Py_omniCallDescriptor* pycd)
{
  const char* op     = pycd->op();
  PyObject*   method = PyObject_GetAttrString(pyservant_, (char*)op);

  if (!method) {
    PyErr_Clear();
    PyObject* word = PyDict_GetItemString(omniPy::pyomniORBwordMap, op);
    if (word) {
      // Keyword -- look up mangled name
      method = PyObject_GetAttr(pyservant_, word);
    }
    else if (omni::strMatch(op, "_interface")) {
      method = PyObject_GetAttrString(pyservant_, (char*)"_get_interface");
    }
    if (!method) {
      if (omniORB::trace(1)) {
	omniORB::logger l;
	l << "Python servant for `" << repoId_ << "' has no method named `"
	  << op << "'.\n";
      }
      PyErr_Clear();
      OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_NoPythonMethod,
		    CORBA::COMPLETED_NO);
    }
  }

  PyObject* in_d   = pycd->in_d_;
  int       in_l   = pycd->in_l_;
  PyObject* out_d  = pycd->out_d_;
  int       out_l  = pycd->out_l_;
  PyObject* exc_d  = pycd->exc_d_;
  PyObject* ctxt_d = pycd->ctxt_d_;

  PyObject* args   = pycd->args();

  // Copy args which would otherwise have reference semantics
  PyObject* argtuple = PyTuple_New(in_l + (ctxt_d ? 1 : 0));
  PyObject* t_o;

  int i;

  try {
    for (i=0; i < in_l; ++i) {
      t_o = copyArgument(PyTuple_GET_ITEM(in_d, i),
			 PyTuple_GET_ITEM(args, i),
			 CORBA::COMPLETED_NO);
      OMNIORB_ASSERT(t_o);
      PyTuple_SET_ITEM(argtuple, i, t_o);
    }
    if (ctxt_d) {
      t_o = filterContext(ctxt_d, PyTuple_GET_ITEM(args, in_l));
      OMNIORB_ASSERT(t_o);
      PyTuple_SET_ITEM(argtuple, in_l, t_o);
    }
  }
  catch (...) {
    Py_DECREF(argtuple);
    Py_DECREF(method);
    throw;
  }

  //
  // Do the call

  PyObject* result = PyEval_CallObject(method, argtuple);
  Py_DECREF(method);
  Py_DECREF(argtuple);

  if (result) {
    PyObject* retval = 0;

    try {
      if (out_l == -1 || out_l == 0) {
	if (result == Py_None) {
	  pycd->setReturnedValues(result);
	  return;
	}
	else
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType,
			CORBA::COMPLETED_MAYBE);
      }
      else if (out_l == 1) {
	retval = copyArgument(PyTuple_GET_ITEM(out_d, 0),
			      result, CORBA::COMPLETED_MAYBE);
      }
      else {
	if (!PyTuple_Check(result) || PyTuple_GET_SIZE(result) != out_l)
	  OMNIORB_THROW(BAD_PARAM,
			BAD_PARAM_WrongPythonType,
			CORBA::COMPLETED_MAYBE);

	retval = PyTuple_New(out_l);
	
	for (i=0; i < out_l; ++i) {
	  t_o = copyArgument(PyTuple_GET_ITEM(out_d, i),
			     PyTuple_GET_ITEM(result, i),
			     CORBA::COMPLETED_MAYBE);

	  PyTuple_SET_ITEM(retval, i, t_o);
	}
      }
    }
    catch (...) {
      Py_DECREF(result);
      Py_XDECREF(retval);
      throw;
    }
    Py_DECREF(result);
    pycd->setReturnedValues(retval);
  }
  else {
    // The call raised a Python exception
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

    // Is it a user exception?
    if (exc_d != Py_None) {
      OMNIORB_ASSERT(PyDict_Check(exc_d));

      PyObject* edesc = PyDict_GetItem(exc_d, erepoId);

      if (edesc) {
	Py_DECREF(erepoId); Py_DECREF(etype); Py_XDECREF(etraceback);
	PyUserException ex(edesc, evalue, CORBA::COMPLETED_MAYBE);
	ex._raise();
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
}


// Py_ServantActivatorSvt

void*
Py_ServantActivatorSvt::_ptrToInterface(const char* repoId)
{
  if (omni::ptrStrMatch(repoId, PortableServer::ServantActivator::_PD_repoId))
    return (PortableServer::_impl_ServantActivator*)this;
  if (omni::ptrStrMatch(repoId, omniPy::string_Py_omniServant))
    return (omniPy::Py_omniServant*)this;
  if (omni::ptrStrMatch(repoId, PortableServer::ServantManager::_PD_repoId))
    return (PortableServer::_impl_ServantManager*)this;
  if (omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId))
    return (void*)1;

  return 0;
}

// Py_ServantLocatorSvt

void*
Py_ServantLocatorSvt::_ptrToInterface(const char* repoId)
{
  if (omni::ptrStrMatch(repoId, PortableServer::ServantLocator::_PD_repoId))
    return (PortableServer::_impl_ServantLocator*)this;
  if (omni::ptrStrMatch(repoId, omniPy::string_Py_omniServant))
    return (omniPy::Py_omniServant*)this;
  if (omni::ptrStrMatch(repoId, PortableServer::ServantManager::_PD_repoId))
    return (PortableServer::_impl_ServantManager*)this;
  if (omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId))
    return (void*)1;

  return 0;
}


// Py_AdapterActivatorSvt

void*
Py_AdapterActivatorSvt::_ptrToInterface(const char* repoId)
{
  if (omni::ptrStrMatch(repoId, PortableServer::AdapterActivator::_PD_repoId))
    return (PortableServer::_impl_AdapterActivator*)this;
  if (omni::ptrStrMatch(repoId, omniPy::string_Py_omniServant))
    return (omniPy::Py_omniServant*)this;
  if (omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId))
    return (void*)1;

  return 0;
}

// Functions to create Py_omniServant objects

static
omniPy::Py_omniServant*
newSpecialServant(PyObject* pyservant, PyObject* opdict, char* repoId)
{
  if (omni::ptrStrMatch(repoId, PortableServer::ServantActivator::_PD_repoId))
    return new Py_ServantActivatorSvt(pyservant, opdict, repoId);

  if (omni::ptrStrMatch(repoId, PortableServer::ServantLocator::_PD_repoId))
    return new Py_ServantLocatorSvt(pyservant, opdict, repoId);

  if (omni::ptrStrMatch(repoId, PortableServer::AdapterActivator::_PD_repoId))
    return new Py_AdapterActivatorSvt(pyservant, opdict, repoId);

  OMNIORB_ASSERT(0);
  return 0;
}


omniPy::Py_omniServant*
omniPy::getServantForPyObject(PyObject* pyservant)
{
  Py_omniServant* pyos;

  // Is there a Py_omniServant already?
  pyos = (omniPy::Py_omniServant*)omniPy::getTwin(pyservant, SERVANT_TWIN);
  if (pyos) {
    pyos->_locked_add_ref();
    return pyos;
  }

  // Is it an instance of the right class?
  if (!omniPy::isInstance(pyservant, omniPy::pyServantClass))
    return 0;

  PyObject* opdict = PyObject_GetAttrString(pyservant, (char*)"_omni_op_d");
  if (!(opdict && PyDict_Check(opdict)))
    return 0;

  PyObject* pyrepoId = PyObject_GetAttrString(pyservant,
					      (char*)"_NP_RepositoryId");
  if (!(pyrepoId && PyString_Check(pyrepoId))) {
    Py_DECREF(opdict);
    return 0;
  }
  if (PyObject_HasAttrString(pyservant, (char*)"_omni_special")) {

    pyos = newSpecialServant(pyservant, opdict, PyString_AS_STRING(pyrepoId));
  }
  else {
    pyos = new omniPy::Py_omniServant(pyservant, opdict,
				      PyString_AS_STRING(pyrepoId));
  }
  Py_DECREF(opdict);
  Py_DECREF(pyrepoId);

  return pyos;
}
