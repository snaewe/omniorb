// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyContext.cc               Created on: 2002/01/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2005 Apasphere Ltd
//    Copyright (C) 2002 AT&T Laboratories Cambridge
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
//    Context support

// $Id$
// $Log$
// Revision 1.1.4.3  2005/06/24 17:36:08  dgrisby
// Support for receiving valuetypes inside Anys; relax requirement for
// old style classes in a lot of places.
//
// Revision 1.1.4.2  2003/05/20 17:10:23  dgrisby
// Preliminary valuetype support.
//
// Revision 1.1.4.1  2003/03/23 21:51:57  dgrisby
// New omnipy3_develop branch.
//
// Revision 1.1.2.2  2003/03/14 15:29:22  dgrisby
// Remove const char* -> char* warnings.
//
// Revision 1.1.2.1  2002/01/18 15:49:44  dpg1
// Context support. New system exception construction. Fix None call problem.
//

#include <omnipy.h>

OMNI_USING_NAMESPACE(omni)


void
omniPy::validateContext(PyObject* c_o, CORBA::CompletionStatus compstatus)
{
  if (!isInstance(c_o, pyCORBAContextClass))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
}

void
omniPy::marshalContext(cdrStream& stream, PyObject* p_o, PyObject* c_o)
{
  PyObject* values = PyObject_CallMethod(c_o, (char*)"_get_values",
					 (char*)"O", p_o);
  if (values) {
    PyObject* items = PyDict_Items(values);
    CORBA::ULong count = PyList_GET_SIZE(items);
    CORBA::ULong mlen  = count * 2;
    mlen >>= stream;

    for (CORBA::ULong i=0; i < count; i++) {
      PyObject* item = PyList_GET_ITEM(items, i);
      omniPy::marshalRawPyString(stream, PyTuple_GET_ITEM(item, 0));
      omniPy::marshalRawPyString(stream, PyTuple_GET_ITEM(item, 1));
    }
    Py_DECREF(values);
  }
  else {
    if (omniORB::trace(1)) {
      {
	omniORB::logger l;
	l << "Exception trying to get Context values:\n";
      }
      PyErr_Print();
    }
    else
      PyErr_Clear();
    
    OMNIORB_THROW(TRANSIENT, TRANSIENT_PythonExceptionInORB,
		  CORBA::COMPLETED_NO);
  }
}

PyObject*
omniPy::unmarshalContext(cdrStream& stream)
{
  PyObject* dict = PyDict_New();
  CORBA::ULong mlen;
  mlen <<= stream;

  if (mlen % 2)
    OMNIORB_THROW(MARSHAL,
		  MARSHAL_InvalidContextList,
		  CORBA::COMPLETED_MAYBE);

  CORBA::ULong count = mlen / 2;

  for (CORBA::ULong i=0; i < count; i++) {
    PyObject* k = omniPy::unmarshalRawPyString(stream);
    PyObject* v = omniPy::unmarshalRawPyString(stream);
    PyDict_SetItem(dict, k, v);
    Py_DECREF(k);
    Py_DECREF(v);
  }
  PyObject* r = PyObject_CallFunction(omniPy::pyCORBAContextClass,
				      (char*)"sON", "", Py_None, dict);
  if (!r) {
    if (omniORB::trace(1)) {
      {
	omniORB::logger l;
	l << "Exception trying to construct Context:\n";
      }
      PyErr_Print();
    }
    else
      PyErr_Clear();

    OMNIORB_THROW(TRANSIENT, TRANSIENT_PythonExceptionInORB,
		  CORBA::COMPLETED_NO);
  }
  return r;
}


PyObject*
omniPy::filterContext(PyObject* p_o, PyObject* c_o)
{
  PyObject* values = PyObject_CallMethod(c_o, (char*)"_get_values",
					 (char*)"O", p_o);

  if (values) {
    PyObject* r = PyObject_CallFunction(omniPy::pyCORBAContextClass,
					(char*)"sON", "", Py_None, values);
    if (r) return r;
  }
  if (omniORB::trace(1)) {
    {
      omniORB::logger l;
      l << "Exception trying to filter Context:\n";
    }
    PyErr_Print();
  }
  else
    PyErr_Clear();

  OMNIORB_THROW(TRANSIENT, TRANSIENT_PythonExceptionInORB,
		CORBA::COMPLETED_NO);
  return 0;
}
