// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyomniFunc.cc              Created on: 2000/06/07
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2008 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//    omniORB API functions

// $Id$
// $Log$
// Revision 1.1.4.9  2008/10/09 15:04:36  dgrisby
// Python exceptions occurring during unmarshalling were not properly
// handled. Exception state left set when at traceLevel 0 (thanks
// Morarenko Kirill).
//
// Revision 1.1.4.8  2008/03/24 20:06:30  dgrisby
// Memory leak in minor code stringification.
//
// Revision 1.1.4.7  2006/01/17 17:38:21  dgrisby
// Expose omniORB.setClientConnectTimeout function.
//
// Revision 1.1.4.6  2005/11/09 12:33:32  dgrisby
// Support POA LocalObjects.
//
// Revision 1.1.4.5  2005/07/22 17:41:07  dgrisby
// Update from omnipy2_develop.
//
// Revision 1.1.4.4  2005/06/24 17:36:00  dgrisby
// Support for receiving valuetypes inside Anys; relax requirement for
// old style classes in a lot of places.
//
// Revision 1.1.4.3  2005/04/14 13:50:59  dgrisby
// New traceTime, traceInvocationReturns functions; removal of omniORB::logf.
//
// Revision 1.1.4.2  2005/01/07 00:22:33  dgrisby
// Big merge from omnipy2_develop.
//
// Revision 1.1.4.1  2003/03/23 21:51:56  dgrisby
// New omnipy3_develop branch.
//
// Revision 1.1.2.11  2002/11/27 00:18:25  dgrisby
// Per thread / per objref timeouts.
//
// Revision 1.1.2.10  2002/08/16 19:27:36  dgrisby
// Documentation update. Minor ORB updates to match docs.
//
// Revision 1.1.2.9  2002/05/26 00:56:22  dgrisby
// traceInvocations() function.
//
// Revision 1.1.2.8  2002/02/25 15:33:50  dpg1
// Expose omniORB logger to Python.
//
// Revision 1.1.2.7  2001/09/24 10:48:28  dpg1
// Meaningful minor codes.
//
// Revision 1.1.2.6  2001/08/21 12:48:27  dpg1
// Meaningful exception minor code strings.
//
// Revision 1.1.2.5  2001/08/21 10:52:42  dpg1
// Update to new ORB core APIs.
//
// Revision 1.1.2.4  2001/04/09 15:22:16  dpg1
// Fixed point support.
//
// Revision 1.1.2.3  2000/11/22 14:43:58  dpg1
// Support code set conversion and wchar/wstring.
//
// Revision 1.1.2.2  2000/11/06 17:07:17  dpg1
// maxTcpConnectionPerServer function
//
// Revision 1.1.2.1  2000/10/13 13:55:28  dpg1
// Initial support for omniORB 4.
//

#include <omnipy.h>
#include <pyThreadCache.h>
#include <omniORB4/giopEndpoint.h>


OMNI_USING_NAMESPACE(omni);


static PyObject* transientEHtuple   = 0;
static PyObject* commFailureEHtuple = 0;
static PyObject* systemEHtuple      = 0;


static CORBA::Boolean transientEH(void* cookie, CORBA::ULong retries,
				  const CORBA::TRANSIENT& ex)
{
  PyObject* tuple;

  if (cookie)
    tuple = (PyObject*)cookie;
  else
    tuple = transientEHtuple;

  OMNIORB_ASSERT(PyTuple_Check(tuple));

  PyObject* pyfn     = PyTuple_GET_ITEM(tuple, 0);
  PyObject* pycookie = PyTuple_GET_ITEM(tuple, 1);
  PyObject* r;
  CORBA::Boolean ret;

  {
    omnipyThreadCache::lock _t;
    PyObject* pyex = omniPy::createPySystemException(ex);
    r = PyObject_CallFunction(pyfn, (char*)"OiN", pycookie, retries, pyex);
    
    if (!r) {
      if (omniORB::trace(1)) {
	omniORB::logs(1, "Python TRANSIENT exception handler failed. "
		      "Traceback follows:");
	PyErr_Print();
      }
      else
	PyErr_Clear();
      return 0;
    }
    if (!PyInt_Check(r)) {
      omniORB::logs(1, "Python TRANSIENT exception handler returned an "
		    "invalid object.");
      Py_DECREF(r);
      return 0;
    }
    ret = PyInt_AS_LONG(r);
    Py_DECREF(r);
  }
  return ret;
}


static CORBA::Boolean commFailureEH(void* cookie, CORBA::ULong retries,
				    const CORBA::COMM_FAILURE& ex)
{
  PyObject* tuple;

  if (cookie)
    tuple = (PyObject*)cookie;
  else
    tuple = commFailureEHtuple;

  OMNIORB_ASSERT(PyTuple_Check(tuple));

  PyObject* pyfn     = PyTuple_GET_ITEM(tuple, 0);
  PyObject* pycookie = PyTuple_GET_ITEM(tuple, 1);
  PyObject* r;
  CORBA::Boolean ret;

  {
    omnipyThreadCache::lock _t;
    PyObject* pyex = omniPy::createPySystemException(ex);
    r = PyObject_CallFunction(pyfn, (char*)"OiN", pycookie, retries, pyex);
    
    if (!r) {
      if (omniORB::trace(1)) {
	omniORB::logs(1, "Python COMM_FAILURE exception handler failed. "
		      "Traceback follows:");
	PyErr_Print();
      }
      else
	PyErr_Clear();
      return 0;
    }
    if (!PyInt_Check(r)) {
      omniORB::logs(1, "Python COMM_FAILURE exception handler returned an "
		    "invalid object.");
      Py_DECREF(r);
      return 0;
    }
    ret = PyInt_AS_LONG(r);
    Py_DECREF(r);
  }
  return ret;
}

static CORBA::Boolean systemEH(void* cookie, CORBA::ULong retries,
			       const CORBA::SystemException& ex)
{
  PyObject* tuple;

  if (cookie)
    tuple = (PyObject*)cookie;
  else
    tuple = systemEHtuple;

  OMNIORB_ASSERT(PyTuple_Check(tuple));

  PyObject* pyfn     = PyTuple_GET_ITEM(tuple, 0);
  PyObject* pycookie = PyTuple_GET_ITEM(tuple, 1);
  PyObject* r;
  CORBA::Boolean ret;

  {
    omnipyThreadCache::lock _t;
    PyObject* pyex = omniPy::createPySystemException(ex);
    r = PyObject_CallFunction(pyfn, (char*)"OiN", pycookie, retries, pyex);
    
    if (!r) {
      if (omniORB::trace(1)) {
	omniORB::logs(1, "Python SystemException handler failed. "
		      "Traceback follows:");
	PyErr_Print();
      }
      else
	PyErr_Clear();
      return 0;
    }
    if (!PyInt_Check(r)) {
      omniORB::logs(1, "Python SystemException handler returned an "
		    "invalid object.");
      Py_DECREF(r);
      return 0;
    }
    ret = PyInt_AS_LONG(r);
    Py_DECREF(r);
  }
  return ret;
}

extern "C" {
  static void removeDummyOmniThread(void* vself) {
    if ((omni_thread*)vself == omni_thread::self()) {
      omniORB::logs(10, "Remove dummy omni thread.");
      omniPy::InterpreterUnlocker _u;
      omni_thread::release_dummy();
    }
    else
      omniORB::logs(5, "Unable to release dummy omni_thread.");
  }
}


omni_thread*
omniPy::ensureOmniThread()
{
  omni_thread* self = omni_thread::self();
  if (self)
    return self;

  omniORB::logs(10, "Create dummy omni thread.");

  // Get the threading.Thread object for this thread.
  PyObject* threading = PyImport_ImportModule((char*)"threading");
  if (!threading) {
    omniORB::logs(1, "Unable to import Python threading module.");
    return 0;
  }

  PyObject* current = PyObject_CallMethod(threading,
					  (char*)"currentThread",
					  (char*)"");
  if (!current) {
    omniORB::logs(1, "Unexpected exception calling threading.currentThread.");
    if (omniORB::trace(1)) PyErr_Print();
    PyErr_Clear();
    return 0;
  }

  // Create a dummy omni_thread
  self = omni_thread::create_dummy();

  // Create a CObject with a suitable destructor function and set it
  // as an attribute of the current thread.
  PyObject* cobj = PyCObject_FromVoidPtr(self, removeDummyOmniThread);
  PyObject_SetAttrString(current, (char*)"__omni_thread", cobj);

  // Use an evil hack to make sure the __omni_thread member is
  // released when the thread stops.
  PyObject* hook = PyObject_CallMethod(omniPy::pyomniORBmodule,
				       (char*)"omniThreadHook", (char*)"O",
				       current);
  if (!hook) {
    omniORB::logs(1, "Unexpected exception calling omniThreadHook.");
    if (omniORB::trace(1)) PyErr_Print();
    PyErr_Clear();
  }

  Py_XDECREF(hook);
  Py_DECREF(cobj);
  Py_DECREF(current);

  return self;
}



extern "C" {

  static char transient_doc [] =
  "installTransientExceptionHandler(cookie, function [, object]) -> None\n"
  "\n"
  "Install a handler for TRANSIENT exceptions, for all objects or a\n"
  "specified object. The function must have the signature:\n"
  "\n"
  "  function(cookie, retries, exc) -> boolean\n"
  "\n"
  "where cookie is the object passed on installation, retries is the\n"
  "number of times this operation invocation has been retried, and exc is\n"
  "the TRANSIENT exception object. If the function returns TRUE, the\n"
  "operation invocation is retried; if it returns FALSE, the TRANSIENT\n"
  "exception is thrown to the application.\n";

  static PyObject* pyomni_installTransientExceptionHandler(PyObject* self,
							   PyObject* args)
  {
    PyObject *pycookie, *pyfn, *pyobjref = 0;

    if (!PyArg_ParseTuple(args, (char*)"OO|O", &pycookie, &pyfn, &pyobjref))
      return 0;


    RAISE_PY_BAD_PARAM_IF(!PyCallable_Check(pyfn), BAD_PARAM_WrongPythonType);

    if (pyobjref) {
      CORBA::Object_ptr objref =
	(CORBA::Object_ptr)omniPy::getTwin(pyobjref, OBJREF_TWIN);

      RAISE_PY_BAD_PARAM_IF(!objref, BAD_PARAM_WrongPythonType);

      PyObject* tuple = Py_BuildValue((char*)"OO", pyfn, pycookie);
      PyObject_SetAttrString(pyobjref, (char*)"__omni_transient", tuple);
      omniORB::installTransientExceptionHandler(objref, (void*)tuple,
						transientEH);
    }
    else {
      Py_XDECREF(transientEHtuple);
      transientEHtuple = Py_BuildValue((char*)"OO", pyfn, pycookie);
      OMNIORB_ASSERT(transientEHtuple);
      omniORB::installTransientExceptionHandler((void*)transientEHtuple,
						transientEH);
    }
    Py_INCREF(Py_None);
    return Py_None;
  }

  static char commfailure_doc [] =
  "installCommFailureExceptionHandler(cookie, function [, object]) -> None\n"
  "\n"
  "Install a handler for COMM_FAILURE exceptions, for all objects or a\n"
  "specified object. The function must have the signature:\n"
  "\n"
  "  function(cookie, retries, exc) -> boolean\n"
  "\n"
  "where cookie is the object passed on installation, retries is the\n"
  "number of times this operation invocation has been retried, and exc is\n"
  "the COMM_FAILURE exception object. If the function returns TRUE, the\n"
  "operation invocation is retried; if it returns FALSE, the COMM_FAILURE\n"
  "exception is thrown to the application.\n";

  static PyObject* pyomni_installCommFailureExceptionHandler(PyObject* self,
							     PyObject* args)
  {
    PyObject *pycookie, *pyfn, *pyobjref = 0;

    if (!PyArg_ParseTuple(args, (char*)"OO|O", &pycookie, &pyfn, &pyobjref))
      return 0;

    RAISE_PY_BAD_PARAM_IF(!PyCallable_Check(pyfn), BAD_PARAM_WrongPythonType);

    if (pyobjref) {
      CORBA::Object_ptr objref =
	(CORBA::Object_ptr)omniPy::getTwin(pyobjref, OBJREF_TWIN);

      RAISE_PY_BAD_PARAM_IF(!objref, BAD_PARAM_WrongPythonType);

      PyObject* tuple = Py_BuildValue((char*)"OO", pyfn, pycookie);
      PyObject_SetAttrString(pyobjref, (char*)"__omni_commfailure", tuple);
      omniORB::installCommFailureExceptionHandler(objref, (void*)tuple,
						  commFailureEH);
    }
    else {
      Py_XDECREF(commFailureEHtuple);
      commFailureEHtuple = Py_BuildValue((char*)"OO", pyfn, pycookie);
      OMNIORB_ASSERT(commFailureEHtuple);
      omniORB::installCommFailureExceptionHandler((void*)commFailureEHtuple,
						  commFailureEH);
    }
    Py_INCREF(Py_None);
    return Py_None;
  }

  static char system_doc [] =
  "installSystemExceptionHandler(cookie, function [, object]) -> None\n"
  "\n"
  "Install a handler for system exceptions other than TRANSIENT and\n"
  "COMM_FAILURE, for all objects or a specified object. The function must\n"
  "have the signature:\n"
  "\n"
  "  function(cookie, retries, exc) -> boolean\n"
  "\n"
  "where cookie is the object passed on installation, retries is the\n"
  "number of times this operation invocation has been retried, and exc is\n"
  "the SystemException object. If the function returns TRUE, the\n"
  "operation invocation is retried; if it returns FALSE, the exception is\n"
  "thrown to the application.\n";

  static PyObject* pyomni_installSystemExceptionHandler(PyObject* self,
							PyObject* args)
  {
    PyObject *pycookie, *pyfn, *pyobjref = 0;

    if (!PyArg_ParseTuple(args, (char*)"OO|O", &pycookie, &pyfn, &pyobjref))
      return 0;

    RAISE_PY_BAD_PARAM_IF(!PyCallable_Check(pyfn), BAD_PARAM_WrongPythonType);

    if (pyobjref) {
      CORBA::Object_ptr objref =
	(CORBA::Object_ptr)omniPy::getTwin(pyobjref, OBJREF_TWIN);

      RAISE_PY_BAD_PARAM_IF(!objref, BAD_PARAM_WrongPythonType);

      PyObject* tuple = Py_BuildValue((char*)"OO", pyfn, pycookie);
      PyObject_SetAttrString(pyobjref, (char*)"__omni_systemex", tuple);
      omniORB::installSystemExceptionHandler(objref, (void*)tuple,
					     systemEH);
    }
    else {
      Py_XDECREF(systemEHtuple);
      systemEHtuple = Py_BuildValue((char*)"OO", pyfn, pycookie);
      OMNIORB_ASSERT(systemEHtuple);
      omniORB::installSystemExceptionHandler((void*)systemEHtuple,
					     systemEH);
    }
    Py_INCREF(Py_None);
    return Py_None;
  }

  static char traceLevel_doc [] =
  "traceLevel(int) -> None\n"
  "traceLevel()    -> int\n"
  "\n"
  "Set or get the omniORB debug trace level.\n";

  static PyObject* pyomni_traceLevel(PyObject* self, PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      return PyInt_FromLong(omniORB::traceLevel);
    }
    else if (PyTuple_GET_SIZE(args) == 1) {
      PyObject* pytl = PyTuple_GET_ITEM(args, 0);

      if (PyInt_Check(pytl)) {
	omniORB::traceLevel = PyInt_AS_LONG(pytl);
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    PyErr_SetString(PyExc_TypeError,
		    (char*)"Operation requires a single integer argument");
    return 0;
  }

  static char traceExceptions_doc [] =
  "traceExceptions(int) -> None\n"
  "traceExceptions()    -> int\n"
  "\n"
  "Set or get the omniORB exception tracing flag.\n";

  static PyObject* pyomni_traceExceptions(PyObject* self, PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      return PyInt_FromLong(omniORB::traceExceptions);
    }
    else if (PyTuple_GET_SIZE(args) == 1) {
      PyObject* pytl = PyTuple_GET_ITEM(args, 0);

      if (PyInt_Check(pytl)) {
	omniORB::traceExceptions = PyInt_AS_LONG(pytl);
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    PyErr_SetString(PyExc_TypeError,
		    (char*)"Operation requires a single integer argument");
    return 0;
  }

  static char traceInvocations_doc [] =
  "traceInvocations(int) -> None\n"
  "traceInvocations()    -> int\n"
  "\n"
  "Set or get the omniORB invocation tracing flag.\n";

  static PyObject* pyomni_traceInvocations(PyObject* self, PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      return PyInt_FromLong(omniORB::traceInvocations);
    }
    else if (PyTuple_GET_SIZE(args) == 1) {
      PyObject* pytl = PyTuple_GET_ITEM(args, 0);

      if (PyInt_Check(pytl)) {
	omniORB::traceInvocations = PyInt_AS_LONG(pytl);
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    PyErr_SetString(PyExc_TypeError,
		    (char*)"Operation requires a single integer argument");
    return 0;
  }

  static char traceInvocationReturns_doc [] =
  "traceInvocationReturns(int) -> None\n"
  "traceInvocationReturns()    -> int\n"
  "\n"
  "Set or get the omniORB invocation return tracing flag.\n";

  static PyObject* pyomni_traceInvocationReturns(PyObject* self, PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      return PyInt_FromLong(omniORB::traceInvocationReturns);
    }
    else if (PyTuple_GET_SIZE(args) == 1) {
      PyObject* pytl = PyTuple_GET_ITEM(args, 0);

      if (PyInt_Check(pytl)) {
	omniORB::traceInvocationReturns = PyInt_AS_LONG(pytl);
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    PyErr_SetString(PyExc_TypeError,
		    (char*)"Operation requires a single integer argument");
    return 0;
  }

  static char traceThreadId_doc [] =
  "traceThreadId(int) -> None\n"
  "traceThreadId()    -> int\n"
  "\n"
  "Set or get the omniORB thread id tracing flag.\n";

  static PyObject* pyomni_traceThreadId(PyObject* self, PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      return PyInt_FromLong(omniORB::traceThreadId);
    }
    else if (PyTuple_GET_SIZE(args) == 1) {
      PyObject* pytl = PyTuple_GET_ITEM(args, 0);

      if (PyInt_Check(pytl)) {
	omniORB::traceThreadId = PyInt_AS_LONG(pytl);
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    PyErr_SetString(PyExc_TypeError,
		    (char*)"Operation requires a single integer argument");
    return 0;
  }

  static char traceTime_doc [] =
  "traceTime(int) -> None\n"
  "traceTime()    -> int\n"
  "\n"
  "Set or get the omniORB time tracing flag.\n";

  static PyObject* pyomni_traceTime(PyObject* self, PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      return PyInt_FromLong(omniORB::traceTime);
    }
    else if (PyTuple_GET_SIZE(args) == 1) {
      PyObject* pytl = PyTuple_GET_ITEM(args, 0);

      if (PyInt_Check(pytl)) {
	omniORB::traceTime = PyInt_AS_LONG(pytl);
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    PyErr_SetString(PyExc_TypeError,
		    (char*)"Operation requires a single integer argument");
    return 0;
  }


  static char log_doc [] =
  "log(level, string)\n"
  "\n"
  "Output to the omniORB log, if the traceLevel is >= level.\n";

  static PyObject* pyomni_log(PyObject* self, PyObject* args)
  {
    int level;
    char* str;
    if (!PyArg_ParseTuple(args, (char*)"is", &level, &str))
      return 0;

    {
      omniPy::InterpreterUnlocker _u;
      omniORB::logs(level, str);
    }
    Py_INCREF(Py_None);
    return Py_None;
  }


  static char nativeCharCodeSet_doc [] =
  "nativeCharCodeSet(string) -> None\n"
  "nativeCharCodeSet()       -> string\n"
  "\n"
  "Set or get the native code set used for char and string.\n";

  static PyObject* pyomni_nativeCharCodeSet(PyObject* self, PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 0) {
      omniCodeSet::NCS_C* ncs_c = orbParameters::nativeCharCodeSet;

      if (ncs_c) {
	const char* ncs = ncs_c->name();
	return PyString_FromString((char*)ncs);
      }
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
    else if (PyTuple_GET_SIZE(args) == 1) {
      PyObject* pyncs = PyTuple_GET_ITEM(args, 0);

      if (PyString_Check(pyncs)) {
	try {
	  omniCodeSet::NCS_C* ncs_c;
	  ncs_c = omniCodeSet::getNCS_C(PyString_AS_STRING(pyncs));
	  orbParameters::nativeCharCodeSet = ncs_c;
	}
	OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

        Py_INCREF(Py_None);
	return Py_None;
      }
    }
    PyErr_SetString(PyExc_TypeError,
		    (char*)"Operation requires a single string argument");
    return 0;
  }

  static char fixed_doc [] =
  "fixed(digits,scale,value)  -> fixed point object\n"
  "\n"
  "Create a new fixed point object, given digits and scale limits\n"
  "and the value as an int or long.\n"
  "\n"
  "e.g. fixed(5, 2, 12345L) -> 123.45\n"
  "     fixed(5, 5, 123L)   -> 0.00123\n"
  "\n"
  "omniORB specific:\n"
  "\n"
  "fixed(string)              -> fixed point object\n"
  "fixed(int)                 -> fixed point object\n"
  "fixed(long)                -> fixed point object\n"
  "fixed(digits,scale,string) -> fixed point object\n"
  "\n"
  "e.g. fixed(\"123.45\") -> 123.45\n";

  static PyObject*
  pyomni_fixed(PyObject* self, PyObject* args)
  {
    return omniPy::newFixedObject(self, args);
  }


  static char minorCodeToString_doc [] =
  "minorCodeToString(CORBA.SystemException) -> string\n"
  "\n"
  "Return a name indicating the meaning of a system exception's minor\n"
  "code. If there is no entry for the minor code, return None.\n";

  static PyObject* pyomni_minorCodeToString(PyObject* self, PyObject* args)
  {
    PyObject* pyexc;
    PyObject* pyrepoId;
    PyObject* pyminor;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyexc))
      return 0;

    pyrepoId = PyObject_GetAttrString(pyexc, (char*)"_NP_RepositoryId");
    omniPy::PyRefHolder repoid_holder(pyrepoId);
    if (!pyrepoId)
      PyErr_Clear();

    pyminor = PyObject_GetAttrString(pyexc, (char*)"minor");
    omniPy::PyRefHolder minor_holder(pyminor);
    if (!pyminor)
      PyErr_Clear();

    if (!(pyrepoId && PyString_Check(pyrepoId) &&
	  pyminor  && (PyInt_Check(pyminor) || PyLong_Check(pyminor)))) {
      Py_INCREF(Py_None);
      return Py_None;
    }
    const char* repoId = PyString_AS_STRING(pyrepoId);

    CORBA::ULong minor;
    if (PyInt_Check(pyminor))
      minor = PyInt_AS_LONG(pyminor);
    else {
      minor = PyLong_AsUnsignedLong(pyminor);
      if (minor == (CORBA::ULong)-1 && PyErr_Occurred())
	PyErr_Clear();
    }

    const char* str = 0;

    if (0) {
      // Empty case to allow us to have a big chain of else ifs due to
      // the macro expansion below.
    }
#define ToStringIfMatch(name) \
    else if (!strcmp(repoId, "IDL:omg.org/CORBA/" #name ":1.0")) \
      str = minorCode2String(name##_LookupTable, minor);

    OMNIORB_FOR_EACH_SYS_EXCEPTION(ToStringIfMatch)
#undef ToStringIfMatch

    if (str)
      return PyString_FromString(str);
    else {
      Py_INCREF(Py_None);
      return Py_None;
    }
  }

  static char setClientCallTimeout_doc [] =
  "setClientCallTimeout(millisecs)\n"
  "setClientCallTimeout(objref, millisecs)\n"
  "\n"
  "Set the global client call timeout, or set the timeout for a specific\n"
  "object reference.\n";

  static PyObject* pyomni_setClientCallTimeout(PyObject* self, PyObject* args)
  {
    if (PyTuple_GET_SIZE(args) == 1) {
      int timeout;
      if (!PyArg_ParseTuple(args, (char*)"i", &timeout))
	return 0;
      omniORB::setClientCallTimeout(timeout);
    }
    else {
      int timeout;
      PyObject* pyobjref;
      if (!PyArg_ParseTuple(args, (char*)"Oi", &pyobjref, &timeout))
	return 0;

      CORBA::Object_ptr objref =
	(CORBA::Object_ptr)omniPy::getTwin(pyobjref, OBJREF_TWIN);

      RAISE_PY_BAD_PARAM_IF(!objref, BAD_PARAM_WrongPythonType);

      omniORB::setClientCallTimeout(objref, timeout);
    }
    Py_INCREF(Py_None);
    return Py_None;
  }

  static char setClientThreadCallTimeout_doc [] =
  "setClientThreadCallTimeout(millisecs)\n"
  "\n"
  "Set the client call timeout for the calling thread.\n";

  static PyObject* pyomni_setClientThreadCallTimeout(PyObject* self,
						     PyObject* args)
  {
    int timeout;
    if (!PyArg_ParseTuple(args, (char*)"i", &timeout))
      return 0;

    try {
      omniPy::ensureOmniThread();
      omniORB::setClientThreadCallTimeout(timeout);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    Py_INCREF(Py_None);
    return Py_None;
  }

  static char setClientConnectTimeout_doc [] =
  "setClientConnectTimeout(millisecs)\n"
  "\n"
  "Set the client connection timeout.\n";

  static PyObject* pyomni_setClientConnectTimeout(PyObject* self,
						  PyObject* args)
  {
    int timeout;
    if (!PyArg_ParseTuple(args, (char*)"i", &timeout))
      return 0;

    omniORB::setClientConnectTimeout(timeout);

    Py_INCREF(Py_None);
    return Py_None;
  }

  static char myIPAddresses_doc [] =
  "myIPAddresses()\n"
  "\n"
  "Return a list of IP address strings for this host. Returns an empty list\n"
  "if called before the ORB is initialised.\n";

  static PyObject* pyomni_myIPAddresses(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)""))
      return 0;

    const omnivector<const char*>* ifaddrs
      = omni::giopTransportImpl::getInterfaceAddress("giop:tcp");

    PyObject* pyaddrs = PyList_New(ifaddrs->size());

    omnivector<const char*>::const_iterator i;
    int j;

    for (i = ifaddrs->begin(), j=0; i != ifaddrs->end(); i++, j++) {
      PyList_SetItem(pyaddrs, j, PyString_FromString(*i));
    }

    return pyaddrs;
  }

  static char setPersistentServerIdentifier_doc [] =
  "setPersistentServerIdentifier(ident)\n"
  "\n"
  "Sets an octet sequence used to persistently identify \"this\"\n"
  "server. Stored object references matching this identifier are\n"
  "re-written to use the current endpoint details.\n";

  static PyObject* pyomni_setPersistentServerIdentifier(PyObject* self,
							PyObject* args)
  {
    char* idstr;
    int   idlen;

    if (!PyArg_ParseTuple(args, (char*)"s#", &idstr, &idlen))
      return 0;

    CORBA::OctetSeq idseq(idlen, idlen, (CORBA::Octet*)idstr, 0);
    try {
      omniPy::InterpreterUnlocker _u;
      omniORB::setPersistentServerIdentifier(idseq);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    Py_INCREF(Py_None);
    return Py_None;
  }

  static char locationForward_doc [] =
  "locationForward(object_reference, new_location)\n"
  "\n"
  "Reconfigures the object reference to refer to the new location,\n"
  "as if a GIOP location forward message had been received.\n";

  static PyObject* pyomni_locationForward(PyObject* self, PyObject* args)
  {
    PyObject* pyold;
    PyObject* pynew;
    if (!PyArg_ParseTuple(args, (char*)"OO", &pyold, &pynew))
      return 0;

    CORBA::Object_ptr oldobj =
      (CORBA::Object_ptr)omniPy::getTwin(pyold, OBJREF_TWIN);

    RAISE_PY_BAD_PARAM_IF(!oldobj, BAD_PARAM_WrongPythonType);

    CORBA::Object_ptr newobj =
      (CORBA::Object_ptr)omniPy::getTwin(pynew, OBJREF_TWIN);

    RAISE_PY_BAD_PARAM_IF(!newobj, BAD_PARAM_WrongPythonType);

    omni::locationForward(oldobj->_PR_getobj(), newobj->_PR_getobj(), 0);

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyMethodDef pyomni_methods[] = {
    {(char*)"installTransientExceptionHandler",
     pyomni_installTransientExceptionHandler,
     METH_VARARGS, transient_doc},

    {(char*)"installCommFailureExceptionHandler",
     pyomni_installCommFailureExceptionHandler,
     METH_VARARGS, commfailure_doc},

    {(char*)"installSystemExceptionHandler",
     pyomni_installSystemExceptionHandler,
     METH_VARARGS, system_doc},

    {(char*)"traceLevel",
     pyomni_traceLevel,
     METH_VARARGS, traceLevel_doc},

    {(char*)"traceExceptions",
     pyomni_traceExceptions,
     METH_VARARGS, traceExceptions_doc},

    {(char*)"traceInvocations",
     pyomni_traceInvocations,
     METH_VARARGS, traceInvocations_doc},

    {(char*)"traceInvocationReturns",
     pyomni_traceInvocationReturns,
     METH_VARARGS, traceInvocationReturns_doc},

    {(char*)"traceThreadId",
     pyomni_traceThreadId,
     METH_VARARGS, traceThreadId_doc},

    {(char*)"traceTime",
     pyomni_traceTime,
     METH_VARARGS, traceTime_doc},

    {(char*)"log",
     pyomni_log,
     METH_VARARGS, log_doc},

    {(char*)"nativeCharCodeSet",
     pyomni_nativeCharCodeSet,
     METH_VARARGS, nativeCharCodeSet_doc},

    {(char*)"fixed",
     pyomni_fixed,
     METH_VARARGS, fixed_doc},

    {(char*)"minorCodeToString",
     pyomni_minorCodeToString,
     METH_VARARGS, minorCodeToString_doc},

    {(char*)"setClientCallTimeout",
     pyomni_setClientCallTimeout,
     METH_VARARGS, setClientCallTimeout_doc},

    {(char*)"setClientThreadCallTimeout",
     pyomni_setClientThreadCallTimeout,
     METH_VARARGS, setClientThreadCallTimeout_doc},

    {(char*)"setClientConnectTimeout",
     pyomni_setClientConnectTimeout,
     METH_VARARGS, setClientConnectTimeout_doc},

    {(char*)"myIPAddresses",
     pyomni_myIPAddresses,
     METH_VARARGS, myIPAddresses_doc},

    {(char*)"setPersistentServerIdentifier",
     pyomni_setPersistentServerIdentifier,
     METH_VARARGS, setPersistentServerIdentifier_doc},

    {(char*)"locationForward",
     pyomni_locationForward,
     METH_VARARGS, locationForward_doc},

    {NULL,NULL}
  };
}

void
omniPy::initomniFunc(PyObject* d)
{
  PyObject* m = Py_InitModule((char*)"_omnipy.omni_func", pyomni_methods);
  PyDict_SetItemString(d, (char*)"omni_func", m);
}
