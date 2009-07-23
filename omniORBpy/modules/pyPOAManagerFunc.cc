// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyPOAManagerFunc.cc        Created on: 2000/02/04
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2008 Apasphere Ltd
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
//    POAManager functions

// $Id$

// $Log$
// Revision 1.1.4.2  2008/04/03 09:05:25  dgrisby
// Leaks of some exception classes. Thanks Luke Deller.
//
// Revision 1.1.4.1  2003/03/23 21:51:57  dgrisby
// New omnipy3_develop branch.
//
// Revision 1.1.2.3  2001/06/11 13:07:27  dpg1
// Unduplicate exception throwing code.
//
// Revision 1.1.2.2  2000/12/04 18:57:24  dpg1
// Fix deadlock when trying to lock omniORB internal lock while holding
// the Python interpreter lock.
//
// Revision 1.1.2.1  2000/10/13 13:55:26  dpg1
// Initial support for omniORB 4.
//

#include <omnipy.h>


PyObject*
omniPy::createPyPOAManagerObject(const PortableServer::POAManager_ptr pm)
{
  if (CORBA::is_nil(pm)) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* pypm_class =
    PyObject_GetAttrString(omniPy::pyPortableServerModule,
			   (char*)"POAManager");
  OMNIORB_ASSERT(pypm_class);

  PyObject* pypm = PyEval_CallObject(pypm_class, omniPy::pyEmptyTuple);
  OMNIORB_ASSERT(pypm);

  omniPy::setTwin(pypm, (PortableServer::POAManager_ptr)pm, POAMANAGER_TWIN);
  omniPy::setTwin(pypm, (CORBA::Object_ptr)             pm, OBJREF_TWIN);
  return pypm;
}


static PyObject*
raiseAdapterInactive(PyObject* pyPM)
{
  PyObject* excc = PyObject_GetAttrString(pyPM, (char*)"AdapterInactive");
  OMNIORB_ASSERT(excc);
  PyObject* exci = PyEval_CallObject(excc, omniPy::pyEmptyTuple);
  PyErr_SetObject(excc, exci);
  Py_DECREF(exci);
  return 0;
}


extern "C" {

  static PyObject* pyPM_activate(PyObject* self, PyObject* args)
  {
    PyObject* pyPM;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyPM)) return NULL;

    PortableServer::POAManager_ptr pm =
      (PortableServer::POAManager_ptr)omniPy::getTwin(pyPM, POAMANAGER_TWIN);

    OMNIORB_ASSERT(pm);

    try {
      omniPy::InterpreterUnlocker _u;
      pm->activate();
    }
    catch (PortableServer::POAManager::AdapterInactive& ex) {
      return raiseAdapterInactive(pyPM);
    }
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject* pyPM_hold_requests(PyObject* self, PyObject* args)
  {
    PyObject* pyPM;
    int       wfc;
    if (!PyArg_ParseTuple(args, (char*)"Oi", &pyPM, &wfc)) return NULL;

    PortableServer::POAManager_ptr pm =
      (PortableServer::POAManager_ptr)omniPy::getTwin(pyPM, POAMANAGER_TWIN);

    OMNIORB_ASSERT(pm);

    try {
      omniPy::InterpreterUnlocker _u;
      pm->hold_requests(wfc);
    }
    catch (PortableServer::POAManager::AdapterInactive& ex) {
      return raiseAdapterInactive(pyPM);
    }
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject* pyPM_discard_requests(PyObject* self, PyObject* args)
  {
    PyObject* pyPM;
    int       wfc;
    if (!PyArg_ParseTuple(args, (char*)"Oi", &pyPM, &wfc)) return NULL;

    PortableServer::POAManager_ptr pm =
      (PortableServer::POAManager_ptr)omniPy::getTwin(pyPM, POAMANAGER_TWIN);

    OMNIORB_ASSERT(pm);

    try {
      omniPy::InterpreterUnlocker _u;
      pm->discard_requests(wfc);
    }
    catch (PortableServer::POAManager::AdapterInactive& ex) {
      return raiseAdapterInactive(pyPM);
    }
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject* pyPM_deactivate(PyObject* self, PyObject* args)
  {
    PyObject* pyPM;
    int       eo, wfc;
    if (!PyArg_ParseTuple(args, (char*)"Oii", &pyPM, &eo, &wfc)) return NULL;

    PortableServer::POAManager_ptr pm =
      (PortableServer::POAManager_ptr)omniPy::getTwin(pyPM, POAMANAGER_TWIN);

    OMNIORB_ASSERT(pm);

    try {
      omniPy::InterpreterUnlocker _u;
      pm->deactivate(eo, wfc);
    }
    catch (PortableServer::POAManager::AdapterInactive& ex) {
      return raiseAdapterInactive(pyPM);
    }
    Py_INCREF(Py_None); return Py_None;
  }

  static PyObject* pyPM_get_state(PyObject* self, PyObject* args)
  {
    PyObject* pyPM;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyPM)) return NULL;

    PortableServer::POAManager_ptr pm =
      (PortableServer::POAManager_ptr)omniPy::getTwin(pyPM, POAMANAGER_TWIN);

    OMNIORB_ASSERT(pm);

    PortableServer::POAManager::State s;
    {
      omniPy::InterpreterUnlocker _u;
      s = pm->get_state();
    }
    return PyInt_FromLong((int)s);
  }


  static PyObject* pyPM_releaseRef(PyObject* self, PyObject* args)
  {
    PyObject* pyPM;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyPM)) return NULL;

    PortableServer::POAManager_ptr pm =
      (PortableServer::POAManager_ptr)omniPy::getTwin(pyPM, POAMANAGER_TWIN);

    OMNIORB_ASSERT(pm);
    {
      omniPy::InterpreterUnlocker _u;
      CORBA::release(pm);
    }
    omniPy::remTwin(pyPM, POAMANAGER_TWIN);
    omniPy::remTwin(pyPM, OBJREF_TWIN);

    Py_INCREF(Py_None);
    return Py_None;
  }


  static PyMethodDef pyPM_methods[] = {
    {(char*)"activate",         pyPM_activate,         METH_VARARGS},
    {(char*)"hold_requests",    pyPM_hold_requests,    METH_VARARGS},
    {(char*)"discard_requests", pyPM_discard_requests, METH_VARARGS},
    {(char*)"deactivate",       pyPM_deactivate,       METH_VARARGS},
    {(char*)"get_state",        pyPM_get_state,        METH_VARARGS},

    {(char*)"releaseRef",       pyPM_releaseRef,       METH_VARARGS},
    {NULL,NULL}
  };
}

void
omniPy::initPOAManagerFunc(PyObject* d)
{
  PyObject* m = Py_InitModule((char*)"_omnipy.poamanager_func", pyPM_methods);
  PyDict_SetItemString(d, (char*)"poamanager_func", m);
}
