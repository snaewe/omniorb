// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// cxxAPI.cc                  Created on: 2002/05/25
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2002 Duncan Grisby
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
// Description:
//    omniORBpy C++ API.

#include <omnipy.h>
#include <pyThreadCache.h>
#include <omniORBpy.h>

static
PyObject*
lockedCxxObjRefToPyObjRef(const CORBA::Object_ptr cxx_obj)
{
  // Make sure the omniORB Python module has been imported
  if (!omniPy::pyomniORBmodule) {
    omniORB::logs(15, "Import Python omniORB module.");
    PyObject* pymod = PyImport_ImportModule((char*)"omniORB");
    if (!pymod) return 0;
    Py_DECREF(pymod);
  }

  // Make sure ORB_init has been called from Python
  if (!omniPy::orb) {
    omniORB::logs(15, "Call Python ORB_init().");
    PyObject* pyorb = PyObject_CallMethod(omniPy::pyCORBAmodule,
					  (char*)"ORB_init",
					  (char*)"");
    if (!pyorb) return 0;
    Py_DECREF(pyorb);
  }
  if (CORBA::is_nil(cxx_obj)) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  if (cxx_obj->_NP_is_pseudo()) {
    return omniPy::createPyPseudoObjRef(CORBA::Object::_duplicate(cxx_obj));
  }
  CORBA::Object_ptr py_obj;
  {
    omniPy::InterpreterUnlocker _u;
    omniObjRef* objref = cxx_obj->_PR_getobj();
    omniIOR* ior = objref->_getIOR();
    objref = omniPy::createObjRef(ior->repositoryID(), ior, 0);
    py_obj = (CORBA::Object_ptr)objref->
                                     _ptrToObjRef(CORBA::Object::_PD_repoId);
  }
  return omniPy::createPyCorbaObjRef(0, py_obj);
}


static
PyObject*
impl_cxxObjRefToPyObjRef(const CORBA::Object_ptr cxx_obj,
			 CORBA::Boolean hold_lock)
{
  if (hold_lock)
    return lockedCxxObjRefToPyObjRef(cxx_obj);
  else {
    omnipyThreadCache::lock _t;
    return lockedCxxObjRefToPyObjRef(cxx_obj);
  }
}

static
CORBA::Object_ptr
lockedPyObjRefToCxxObjRef(PyObject* py_obj)
{
  if (py_obj == Py_None) {
    return CORBA::Object::_nil();
  }
  CORBA::Object_ptr obj = (CORBA::Object_ptr)omniPy::getTwin(py_obj,
							     OBJREF_TWIN);
  if (!obj) {
    // The ORB doesn't have an OBJREF_TWIN. Perhaps that's what we're
    // dealing with...
    CORBA::ORB_ptr orb = (CORBA::ORB_ptr)omniPy::getTwin(py_obj, ORB_TWIN);
    obj = orb;
  }
  if (!obj) {
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, CORBA::COMPLETED_NO);
  }

  if (obj->_NP_is_pseudo()) {
    return CORBA::Object::_duplicate(obj);
  }

  {
    omniPy::InterpreterUnlocker _u;
    omniObjRef* objref = obj->_PR_getobj();
    omniIOR* ior = objref->_getIOR();
    objref = omni::createObjRef(CORBA::Object::_PD_repoId, ior, 0);
    return (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
  }
}

static
CORBA::Object_ptr
impl_pyObjRefToCxxObjRef(PyObject* py_obj, CORBA::Boolean hold_lock)
{
  if (hold_lock)
    return lockedPyObjRefToCxxObjRef(py_obj);
  else {
    omnipyThreadCache::lock _t;
    return lockedPyObjRefToCxxObjRef(py_obj);
  }
}

static
PyObject*
impl_handleCxxSystemException(const CORBA::SystemException& ex)
{
  return omniPy::handleSystemException(ex);
}

static
void
impl_handlePythonSystemException()
{
  omniPy::handlePythonException();
}

static void
impl_marshalPyObject(cdrStream& stream, PyObject* desc, PyObject* obj,
		     CORBA::Boolean hold_lock)
{
  if (hold_lock) {
    omniPy::validateType(desc, obj, CORBA::COMPLETED_NO);
    omniPy::marshalPyObject(stream, desc, obj);
  }
  else {
    omnipyThreadCache::lock _t;
    omniPy::validateType(desc, obj, CORBA::COMPLETED_NO);
    omniPy::marshalPyObject(stream, desc, obj);
  }
}    

static PyObject*
impl_unmarshalPyObject(cdrStream& stream, PyObject* desc,
		       CORBA::Boolean hold_lock)
{
  if (hold_lock) {
    return omniPy::unmarshalPyObject(stream, desc);
  }
  else {
    omnipyThreadCache::lock _t;
    return omniPy::unmarshalPyObject(stream, desc);
  }
}    

static void
impl_marshalTypeDesc(cdrStream& stream, PyObject* desc,
		     CORBA::Boolean hold_lock)
{
  if (hold_lock) {
    omniPy::marshalTypeCode(stream, desc);
  }
  else {
    omnipyThreadCache::lock _t;
    omniPy::marshalTypeCode(stream, desc);
  }
}

static PyObject*
impl_unmarshalTypeDesc(cdrStream& stream, CORBA::Boolean hold_lock)
{
  if (hold_lock) {
    return omniPy::unmarshalTypeCode(stream);
  }
  else {
    omnipyThreadCache::lock _t;
    return omniPy::unmarshalTypeCode(stream);
  }
}


omniORBpyAPI::omniORBpyAPI()
  : cxxObjRefToPyObjRef(impl_cxxObjRefToPyObjRef),
    pyObjRefToCxxObjRef(impl_pyObjRefToCxxObjRef),
    handleCxxSystemException(impl_handleCxxSystemException),
    handlePythonSystemException(impl_handlePythonSystemException),
    marshalPyObject(impl_marshalPyObject),
    unmarshalPyObject(impl_unmarshalPyObject),
    marshalTypeDesc(impl_marshalTypeDesc),
    unmarshalTypeDesc(impl_unmarshalTypeDesc)
{}


// The API Singleton
omniORBpyAPI omniPy::cxxAPI;
