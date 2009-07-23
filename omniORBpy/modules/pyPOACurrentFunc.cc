// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyPOACurrentFunc.cc        Created on: 2001/06/11
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2005-2008 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//    PortableServer::Current functions

// $Id$
// $Log$
// Revision 1.1.4.3  2008/04/03 09:05:26  dgrisby
// Leaks of some exception classes. Thanks Luke Deller.
//
// Revision 1.1.4.2  2005/01/07 00:22:33  dgrisby
// Big merge from omnipy2_develop.
//
// Revision 1.1.4.1  2003/03/23 21:51:57  dgrisby
// New omnipy3_develop branch.
//
// Revision 1.1.2.2  2001/09/24 10:48:27  dpg1
// Meaningful minor codes.
//
// Revision 1.1.2.1  2001/06/11 13:06:26  dpg1
// Support for PortableServer::Current.
//

#include <omnipy.h>


PyObject*
omniPy::createPyPOACurrentObject(const PortableServer::Current_ptr pc)
{
  if (CORBA::is_nil(pc)) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* pypc_class =
    PyObject_GetAttrString(omniPy::pyPortableServerModule, (char*)"Current");
  OMNIORB_ASSERT(pypc_class);

  PyObject* pypc = PyEval_CallObject(pypc_class, omniPy::pyEmptyTuple);
  OMNIORB_ASSERT(pypc);

  omniPy::setTwin(pypc, (PortableServer::Current_ptr)pc, POACURRENT_TWIN);
  omniPy::setTwin(pypc, (CORBA::Object_ptr)          pc, OBJREF_TWIN);
  return pypc;
}

static PyObject*
raiseNoContext(PyObject* pyPC)
{
  PyObject* excc = PyObject_GetAttrString(pyPC, (char*)"NoContext");
  OMNIORB_ASSERT(excc);
  PyObject* exci = PyEval_CallObject(excc, omniPy::pyEmptyTuple);
  PyErr_SetObject(excc, exci);
  Py_DECREF(exci);
  return 0;
}


extern "C" {

  static PyObject* pyPC_get_POA(PyObject* self, PyObject* args)
  {
    PyObject* pyPC;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyPC)) return NULL;

    PortableServer::Current_ptr pc =
      (PortableServer::Current_ptr)omniPy::getTwin(pyPC, POACURRENT_TWIN);

    OMNIORB_ASSERT(pc);

    PortableServer::POA_ptr poa;
    try {
      omniPy::InterpreterUnlocker _u;
      poa = pc->get_POA();
    }
    catch (PortableServer::Current::NoContext& ex) {
      return raiseNoContext(pyPC);
    }
    return omniPy::createPyPOAObject(poa);
  }

  static PyObject* pyPC_get_object_id(PyObject* self, PyObject* args)
  {
    PyObject* pyPC;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyPC)) return NULL;

    PortableServer::Current_ptr pc =
      (PortableServer::Current_ptr)omniPy::getTwin(pyPC, POACURRENT_TWIN);

    OMNIORB_ASSERT(pc);

    PortableServer::ObjectId_var oid;
    try {
      omniPy::InterpreterUnlocker _u;
      oid = pc->get_object_id();
    }
    catch (PortableServer::Current::NoContext& ex) {
      return raiseNoContext(pyPC);
    }
    return PyString_FromStringAndSize((const char*)oid->NP_data(),
				      oid->length());
  }

  static PyObject* pyPC_get_reference(PyObject* self, PyObject* args)
  {
    PyObject* pyPC;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyPC)) return NULL;

    PortableServer::Current_ptr pc =
      (PortableServer::Current_ptr)omniPy::getTwin(pyPC, POACURRENT_TWIN);

    OMNIORB_ASSERT(pc);

    CORBA::Object_ptr lobjref;
    const char* mdri;

    try {
      omniPy::InterpreterUnlocker _u;
      CORBA::Object_var objref;
      objref  = pc->get_reference();
      mdri    = objref->_PR_getobj()->_mostDerivedRepoId();
      lobjref = omniPy::makeLocalObjRef(mdri, objref);
    }
    catch (PortableServer::Current::NoContext& ex) {
      return raiseNoContext(pyPC);
    }
    return omniPy::createPyCorbaObjRef(0, lobjref);
  }
      
  static PyObject* pyPC_get_servant(PyObject* self, PyObject* args)
  {
    PyObject* pyPC;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyPC)) return NULL;

    PortableServer::Current_ptr pc =
      (PortableServer::Current_ptr)omniPy::getTwin(pyPC, POACURRENT_TWIN);

    OMNIORB_ASSERT(pc);

    PortableServer::Servant  servant;
    omniPy::Py_omniServant*  pyos;
    try {
      omniPy::InterpreterUnlocker _u;
      servant = pc->get_servant();
      pyos = (omniPy::Py_omniServant*)servant->
                               _ptrToInterface(omniPy::string_Py_omniServant);
    }
    catch (PortableServer::Current::NoContext& ex) {
      return raiseNoContext(pyPC);
    }
    if (pyos) {
      PyObject* pyservant = pyos->pyServant();
      pyos->_locked_remove_ref();
      return pyservant;
    }
    else {
      // Oh dear -- the servant is C++, not Python. OBJ_ADAPTER
      // seems the most sensible choice of exception.
      {
	omniPy::InterpreterUnlocker _u;
	servant->_remove_ref();
      }
      CORBA::OBJ_ADAPTER ex(OBJ_ADAPTER_IncompatibleServant,
			    CORBA::COMPLETED_NO);
      return omniPy::handleSystemException(ex);
    }
  }

  static PyObject* pyPC_releaseRef(PyObject* self, PyObject* args)
  {
    PyObject* pyPC;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyPC)) return NULL;

    PortableServer::Current_ptr pc =
      (PortableServer::Current_ptr)omniPy::getTwin(pyPC, POACURRENT_TWIN);

    OMNIORB_ASSERT(pc);
    {
      omniPy::InterpreterUnlocker _u;
      CORBA::release(pc);
    }
    omniPy::remTwin(pyPC, POACURRENT_TWIN);
    omniPy::remTwin(pyPC, OBJREF_TWIN);

    Py_INCREF(Py_None);
    return Py_None;
  }


  static PyMethodDef pyPC_methods[] = {
    {(char*)"get_POA",          pyPC_get_POA,          METH_VARARGS},
    {(char*)"get_object_id",    pyPC_get_object_id,    METH_VARARGS},
    {(char*)"get_reference",    pyPC_get_reference,    METH_VARARGS},
    {(char*)"get_servant",      pyPC_get_servant,      METH_VARARGS},

    {(char*)"releaseRef",       pyPC_releaseRef,       METH_VARARGS},
    {NULL,NULL}
  };
}

void
omniPy::initPOACurrentFunc(PyObject* d)
{
  PyObject* m = Py_InitModule((char*)"_omnipy.poacurrent_func", pyPC_methods);
  PyDict_SetItemString(d, (char*)"poacurrent_func", m);
}
