// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyMarshal.cc               Created on: 1999/07/05
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2008 Apasphere Ltd
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
//    Marshalling / unmarshalling of Python objects

// $Id$
// $Log$
// Revision 1.1.4.13  2009/05/05 14:26:54  dgrisby
// Tolerate unpickled enum items.
//
// Revision 1.1.4.12  2008/10/09 15:04:36  dgrisby
// Python exceptions occurring during unmarshalling were not properly
// handled. Exception state left set when at traceLevel 0 (thanks
// Morarenko Kirill).
//
// Revision 1.1.4.11  2007/07/25 15:00:10  dgrisby
// Sequence unmarshalling could incorrecly complain about the lack of
// data left in a sequence of null or void.
//
// Revision 1.1.4.10  2006/05/15 10:26:11  dgrisby
// More relaxation of requirements for old-style classes, for Python 2.5.
//
// Revision 1.1.4.9  2005/12/08 14:28:05  dgrisby
// Track ORB core changes.
//
// Revision 1.1.4.8  2005/11/09 12:33:32  dgrisby
// Support POA LocalObjects.
//
// Revision 1.1.4.7  2005/08/12 09:32:09  dgrisby
// Use Python bool type where available.
//
// Revision 1.1.4.6  2005/07/22 17:41:08  dgrisby
// Update from omnipy2_develop.
//
// Revision 1.1.4.5  2005/06/24 17:36:08  dgrisby
// Support for receiving valuetypes inside Anys; relax requirement for
// old style classes in a lot of places.
//
// Revision 1.1.4.4  2005/01/07 00:22:32  dgrisby
// Big merge from omnipy2_develop.
//
// Revision 1.1.4.3  2003/07/10 22:13:25  dgrisby
// Abstract interface support.
//
// Revision 1.1.4.2  2003/05/20 17:10:23  dgrisby
// Preliminary valuetype support.
//
// Revision 1.1.4.1  2003/03/23 21:51:57  dgrisby
// New omnipy3_develop branch.
//
// Revision 1.1.2.15  2003/01/27 11:56:58  dgrisby
// Correctly handle invalid returns from application code.
//
// Revision 1.1.2.14  2002/09/06 21:30:42  dgrisby
// Int/long unification.
//
// Revision 1.1.2.13  2002/03/18 17:05:50  dpg1
// Correct exceptions for lack of wstring in GIOP 1.0.
//
// Revision 1.1.2.12  2001/09/24 10:48:26  dpg1
// Meaningful minor codes.
//
// Revision 1.1.2.11  2001/09/20 14:51:24  dpg1
// Allow ORB reinitialisation after destroy(). Clean up use of omni namespace.
//
// Revision 1.1.2.10  2001/08/21 10:52:41  dpg1
// Update to new ORB core APIs.
//
// Revision 1.1.2.9  2001/07/03 11:29:03  dpg1
// Tweaks to compile on Solaris.
//
// Revision 1.1.2.8  2001/06/28 16:41:09  dpg1
// Properly scope omniCodeSet for Unicode support.
//
// Revision 1.1.2.7  2001/05/14 12:47:21  dpg1
// Fix memory leaks.
//
// Revision 1.1.2.6  2001/05/10 15:16:02  dpg1
// Big update to support new omniORB 4 internals.
//
// Revision 1.1.2.5  2001/04/09 15:22:16  dpg1
// Fixed point support.
//
// Revision 1.1.2.4  2000/11/22 14:43:58  dpg1
// Support code set conversion and wchar/wstring.
//
// Revision 1.1.2.3  2000/11/06 17:10:08  dpg1
// Update to cdrStream interface
//
// Revision 1.1.2.2  2000/11/01 15:29:00  dpg1
// Support for forward-declared structs and unions
// RepoIds in indirections are now resolved at the time of use
//
// Revision 1.1.2.1  2000/10/13 13:55:25  dpg1
// Initial support for omniORB 4.
//

#include <omnipy.h>
#include <pyFixed.h>


#ifdef Py_UNICODEOBJECT_H
#  include <codeSetUtil.h>
#  define PY_HAS_UNICODE
#endif

OMNI_USING_NAMESPACE(omni)


#if defined(__DECCXX)
// EDG based compaq cxx is having a problem with taking the address of static
// functions.
PyObject* omnipyCompaqCxxBug() {
  // Oddly, modules that invoke the following function don't have a problem.
  return omniPy::newTwin(0); // never call this.
}
#endif

// PyLongFromLongLong is broken in Python 1.5.2. Workaround here:
#ifdef HAS_LongLong
#  if !defined(PY_VERSION_HEX) || (PY_VERSION_HEX < 0X01050200)
#    error "omniidl requires Python 1.5.2 or higher"

#  elif (PY_VERSION_HEX < 0x02000000)

// Don't know when it was fixed -- certainly in 2.0.0

static inline PyObject* MyPyLong_FromLongLong(_CORBA_LongLong ll)
{
  if (ll >= 0) // Positive numbers work OK
    return PyLong_FromLongLong(ll);
  else {
    _CORBA_ULongLong ull = (~ll) + 1; // Hope integers are 2's complement...
    PyObject* p = PyLong_FromUnsignedLongLong(ull);
    PyObject* n = PyNumber_Negative(p);
    Py_DECREF(p);
    return n;
  }
}
#  else
#    define MyPyLong_FromLongLong(ll) PyLong_FromLongLong(ll)
#  endif
#endif

// Boolean type support
#if (PY_VERSION_HEX < 0x02030000)
#  define PyBool_FromLong(x) PyInt_FromLong(x ? 1 : 0)
#  define PyBool_Check(x) 0
#endif


// Small function to indicate whether a descriptor represents a type
// for which we have unrolled sequence marshalling code
static inline int
sequenceOptimisedType(PyObject* desc)
{
  static CORBA::Boolean optmap[] = {
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0
  };

  if (PyInt_Check(desc)) {
    CORBA::ULong tk = PyInt_AS_LONG(desc);
    OMNIORB_ASSERT(tk <= 33);
    return optmap[tk];
  }
  return 0;
}
	


static void
validateTypeNull(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus,
		 PyObject* track)
{
  if (a_o != Py_None)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
}

static void
validateTypeVoid(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus,
		 PyObject* track)
{
  if (a_o != Py_None)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
}

static void
validateTypeShort(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{
  long l;

  if (PyInt_Check(a_o))
    l = PyInt_AS_LONG(a_o);
  else if (PyLong_Check(a_o)) {
    l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  if (l < -0x8000 || l > 0x7fff)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
}

static void
validateTypeLong(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus,
		 PyObject* track)
{
  long l;

  if (PyInt_Check(a_o))
    l = PyInt_AS_LONG(a_o);
  else if (PyLong_Check(a_o)) {
    l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

#if SIZEOF_LONG > 4
  if (l < -0x80000000L || l > 0x7fffffffL)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
}

static void
validateTypeUShort(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{
  long l;

  if (PyInt_Check(a_o))
    l = PyInt_AS_LONG(a_o);
  else if (PyLong_Check(a_o)) {
    l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  if (l < 0 || l > 0xffff)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
}

static void
validateTypeULong(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{
  if (PyLong_Check(a_o)) {
    unsigned long ul = PyLong_AsUnsignedLong(a_o);
    if (ul == (unsigned long)-1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
#if SIZEOF_LONG > 4
    if (ul > 0xffffffffL)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
  }
  else if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
#if SIZEOF_LONG > 4
    if (l < 0 || l > 0xffffffffL)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
#else
    if (l < 0)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
}

static void
validateTypeFloat(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{
  if (PyFloat_Check(a_o) || PyInt_Check(a_o))
    return;

  if (PyLong_Check(a_o)) {
    double d = PyLong_AsDouble(a_o);
    if (d == -1.0 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
}

static void
validateTypeDouble(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{
  if (PyFloat_Check(a_o) || PyInt_Check(a_o))
    return;

  if (PyLong_Check(a_o)) {
    double d = PyLong_AsDouble(a_o);
    if (d == -1.0 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
}

static void
validateTypeBoolean(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus,
		    PyObject* track)
{
  if (!(PyInt_Check(a_o) || PyLong_Check(a_o)))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
}

static void
validateTypeChar(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus,
		 PyObject* track)
{
  if (!(PyString_Check(a_o) && (PyString_GET_SIZE(a_o) == 1)))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
}

static void
validateTypeOctet(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{
  long l;

  if (PyInt_Check(a_o))
    l = PyInt_AS_LONG(a_o);
  else if (PyLong_Check(a_o)) {
    l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  if (l < 0 || l > 0xff)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
}

static void
validateTypeAny(PyObject* d_o, PyObject* a_o,
		CORBA::CompletionStatus compstatus,
		PyObject* track)
{
  if (!omniPy::isInstance(a_o, omniPy::pyCORBAAnyClass))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  // Validate TypeCode
  PyObject* t_o = PyObject_GetAttrString(a_o, (char*)"_t");

  if (!t_o) {
    PyErr_Clear();
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
  Py_DECREF(t_o);

  if (!omniPy::isInstance(t_o, omniPy::pyCORBATypeCodeClass))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  PyObject* desc = PyObject_GetAttrString(t_o, (char*)"_d");
  Py_XDECREF(desc);
  if (!desc) {
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }

  // Any's contents
  t_o = PyObject_GetAttrString(a_o, (char*)"_v");
  if (!t_o) {
    PyErr_Clear();
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
  Py_DECREF(t_o);
  omniPy::validateType(desc, t_o, compstatus, track);
}

static void
validateTypeTypeCode(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus,
		     PyObject* track)
{
  if (!omniPy::isInstance(a_o, omniPy::pyCORBATypeCodeClass))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  PyObject* t_o = PyObject_GetAttrString(a_o, (char*)"_d");

  Py_XDECREF(t_o);
  if (!t_o)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
}

static void
validateTypePrincipal(PyObject* d_o, PyObject* a_o,
		      CORBA::CompletionStatus compstatus,
		      PyObject* track)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
}

static void
validateTypeObjref(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{ // repoId, name
  if (a_o != Py_None) {
    CORBA::Object_ptr obj =
      (CORBA::Object_ptr)omniPy::getTwin(a_o, OBJREF_TWIN);
    if (!obj)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
}

static void
validateTypeStruct(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{ // class, repoId, struct name, name, descriptor, ...

  // The descriptor tuple has twice the number of struct members,
  // plus 4 -- the typecode kind, the Python class, the repoId,
  // and the struct name
  int cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;

  PyObject* name;
  PyObject* value;

  int i, j;

  // Optimise for the fast case, where the object is a class
  // instance with all attributes in its own dictionary
  if (PyInstance_Check(a_o)) {

    PyObject* sdict = ((PyInstanceObject*)a_o)->in_dict;

    for (i=0,j=4; i < cnt; i++,j++) {
      name    = PyTuple_GET_ITEM(d_o, j++);
      OMNIORB_ASSERT(PyString_Check(name));
      value   = PyDict_GetItem(sdict, name);

      if (value) {
	omniPy::validateType(PyTuple_GET_ITEM(d_o, j), value,
			     compstatus, track);
      }
      else {
	// Not such a fast case after all
	value = PyObject_GetAttr(a_o, name);
	if (!value) {
	  PyErr_Clear();
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	// DECREF now in case validateType() throws an exception. Safe
	// because the struct object still holds a reference to the
	// value.
	Py_DECREF(value);
	omniPy::validateType(PyTuple_GET_ITEM(d_o, j), value,
			     compstatus, track);
      }
    }
  }
  else {
    for (i=0,j=4; i < cnt; i++,j++) {
      name    = PyTuple_GET_ITEM(d_o, j++);
      OMNIORB_ASSERT(PyString_Check(name));
      value   = PyObject_GetAttr(a_o, name);
      if (!value) {
	PyErr_Clear();
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
      }

      Py_DECREF(value);
      omniPy::validateType(PyTuple_GET_ITEM(d_o, j), value, compstatus, track);
    }
  }
}

static void
validateTypeUnion(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{ // class,
  // repoId,
  // name,
  // discriminant descr,
  // default used,
  // ((label value, member name, member descr), ...),
  // default (label, name, descr) or None,
  // {label: (label, name, descr), ...}

  if (!PyInstance_Check(a_o))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  PyObject* udict        = ((PyInstanceObject*)a_o)->in_dict;
  PyObject* discriminant = PyDict_GetItemString(udict, (char*)"_d");
  PyObject* value        = PyDict_GetItemString(udict, (char*)"_v");
  if (!(discriminant && value))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 4); // Discriminant descriptor
  omniPy::validateType(t_o, discriminant, compstatus, track);

  PyObject* cdict = PyTuple_GET_ITEM(d_o, 8);
  OMNIORB_ASSERT(PyDict_Check(cdict));

  t_o = PyDict_GetItem(cdict, discriminant);
  if (t_o) {
    // Discriminant found in case dictionary
    OMNIORB_ASSERT(PyTuple_Check(t_o));
    omniPy::validateType(PyTuple_GET_ITEM(t_o, 2), value, compstatus, track);
  }
  else {
    // Is there a default case?
    t_o = PyTuple_GET_ITEM(d_o, 7);
    if (t_o != Py_None) {
      OMNIORB_ASSERT(PyTuple_Check(t_o));
      omniPy::validateType(PyTuple_GET_ITEM(t_o, 2), value, compstatus, track);
    }

  }
}

static void
validateTypeEnum(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus,
		 PyObject* track)
{ // repoId, name, item list

  PyObject* ev = PyObject_GetAttrString(a_o, (char*)"_v");

  if (!(ev && PyInt_Check(ev))) {
    PyErr_Clear();
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
  Py_DECREF(ev);

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 3);
  long      e   = PyInt_AS_LONG(ev);

  if (e >= PyTuple_GET_SIZE(t_o))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_EnumValueOutOfRange, compstatus);

  if (PyTuple_GET_ITEM(t_o, e) != a_o) {
    // EnumItem object is not the one we expected -- are they equivalent?
    int cmp;
    if (PyObject_Cmp(PyTuple_GET_ITEM(t_o, e), a_o, &cmp) == -1)
      omniPy::handlePythonException();

    if (cmp != 0)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
}

static void
validateTypeString(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{ // max_length

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);
  OMNIORB_ASSERT(PyInt_Check(t_o));

  CORBA::ULong max_len = PyInt_AS_LONG(t_o);

  if (!PyString_Check(a_o))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  CORBA::ULong len = PyString_GET_SIZE(a_o);

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, compstatus);

  // Annoyingly, we have to scan the string to check there are no
  // nulls
  char* str = PyString_AS_STRING(a_o);
  for (CORBA::ULong i=0; i<len; i++)
    if (str[i] == '\0')
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_EmbeddedNullInPythonString, compstatus);
}


static void
validateTypeSequence(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus,
		     PyObject* track)
{ // element_desc, max_length

  PyObject*     t_o      = PyTuple_GET_ITEM(d_o, 2);
  OMNIORB_ASSERT(PyInt_Check(t_o));
  CORBA::ULong  max_len  = PyInt_AS_LONG(t_o);
  PyObject*     elm_desc = PyTuple_GET_ITEM(d_o, 1);

  CORBA::ULong  len, i;
  long          long_val;
  unsigned long ulong_val;

  if (sequenceOptimisedType(elm_desc)) {
    CORBA::ULong etk = PyInt_AS_LONG(elm_desc);

    if (etk == CORBA::tk_octet || etk == CORBA::tk_char) {
      // Mapping says octet and char use a string
      if (!PyString_Check(a_o))
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

      len = PyString_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);
      return;
    }
    else if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      switch (etk) {
      case CORBA::tk_short:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

	  if (long_val < -0x8000 || long_val > 0x7fff)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	}
	return;

      case CORBA::tk_long:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

#if SIZEOF_LONG > 4
	  if (long_val < -0x80000000L || long_val > 0x7fffffffL)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	}
	return;

      case CORBA::tk_ushort:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange,
			    compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

	  if (long_val < 0 || long_val > 0xffff)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	}
	return;

      case CORBA::tk_ulong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ulong_val = PyLong_AsUnsignedLong(t_o);
	    if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (ulong_val > 0xffffffffL)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
#if SIZEOF_LONG > 4
	    if (long_val < 0 || long_val > 0xffffffffL)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#else
	    if (long_val < 0)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_float:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (!(PyFloat_Check(t_o) || PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_double:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (!(PyFloat_Check(t_o) || PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_boolean:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (!(PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    CORBA::LongLong ll = PyLong_AsLongLong(t_o);
	    if (ll == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else if (!PyInt_Check(t_o)) {
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	  }
	}
	return;

      case CORBA::tk_ulonglong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    CORBA::ULongLong ull = PyLong_AsUnsignedLongLong(t_o);
	    if (ull == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else if (PyInt_Check(t_o)) {
	    long l = PyInt_AS_LONG(t_o);
	    if (l < 0)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else {
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	  }
	}
	return;
#else
      case 23:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);

      case 24:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
      default:
	OMNIORB_ASSERT(0);
      }
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      switch (etk) {
      case CORBA::tk_short:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

	  if (long_val < -0x8000 || long_val > 0x7fff)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	}
	return;

      case CORBA::tk_long:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

#if SIZEOF_LONG > 4
	  if (long_val < -0x80000000L || long_val > 0x7fffffffL)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	}
	return;

      case CORBA::tk_ushort:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange,
			    compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

	  if (long_val < 0 || long_val > 0xffff)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	}
	return;

      case CORBA::tk_ulong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ulong_val = PyLong_AsUnsignedLong(t_o);
	    if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (ulong_val > 0xffffffffL)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
#if SIZEOF_LONG > 4
	    if (long_val < 0 || long_val > 0xffffffffL)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#else
	    if (long_val < 0)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_float:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (!(PyFloat_Check(t_o) || PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_double:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (!(PyFloat_Check(t_o) || PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_boolean:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (!(PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    CORBA::LongLong ll = PyLong_AsLongLong(t_o);
	    if (ll == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else if (!PyInt_Check(t_o)) {
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	  }
	}
	return;

      case CORBA::tk_ulonglong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    CORBA::ULongLong ull = PyLong_AsUnsignedLongLong(t_o);
	    if (ull == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else if (PyInt_Check(t_o)) {
	    long l = PyInt_AS_LONG(t_o);
	    if (l < 0)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else {
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	  }
	}
	return;
#else
      case 23:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);

      case 24:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
      default:
	OMNIORB_ASSERT(0);
      }
    }
    else
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
  else { // Complex type
    if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);
	  
      for (i=0; i < len; i++) {
	omniPy::validateType(elm_desc, PyList_GET_ITEM(a_o, i),
			     compstatus, track);
      }
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      for (i=0; i < len; i++) {
	omniPy::validateType(elm_desc, PyTuple_GET_ITEM(a_o, i),
			     compstatus, track);
      }
    }
    else
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
}

static void
validateTypeArray(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{ // element_desc, length

  PyObject*    t_o      = PyTuple_GET_ITEM(d_o, 2);
  OMNIORB_ASSERT(PyInt_Check(t_o));
  CORBA::ULong arr_len  = PyInt_AS_LONG(t_o);
  PyObject*    elm_desc = PyTuple_GET_ITEM(d_o, 1);

  CORBA::ULong  len, i;
  long          long_val;
  unsigned long ulong_val;

  if (sequenceOptimisedType(elm_desc)) {
    CORBA::ULong etk = PyInt_AS_LONG(elm_desc);

    if (etk == CORBA::tk_octet || etk == CORBA::tk_char) {
      // Mapping says octet and char use a string
      if (!PyString_Check(a_o))
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

      len = PyString_GET_SIZE(a_o);
      if (len != arr_len)
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
    else if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (len != arr_len)
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);

      switch (etk) {
      case CORBA::tk_short:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

	  if (long_val < -0x8000 || long_val > 0x7fff)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	}
	return;

      case CORBA::tk_long:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

#if SIZEOF_LONG > 4
	  if (long_val < -0x80000000L || long_val > 0x7fffffffL)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	}
	return;

      case CORBA::tk_ushort:
	for (i=0; i<len; i++) {
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange,
			    compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

	  if (long_val < 0 || long_val > 0xffff)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	}
	return;

      case CORBA::tk_ulong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ulong_val = PyLong_AsUnsignedLong(t_o);
	    if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (ulong_val > 0xffffffffL)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
#if SIZEOF_LONG > 4
	    if (long_val < 0 || long_val > 0xffffffffL)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#else
	    if (long_val < 0)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_float:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (!(PyFloat_Check(t_o) || PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_double:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (!(PyFloat_Check(t_o) || PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_boolean:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (!(PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    CORBA::LongLong ll = PyLong_AsLongLong(t_o);
	    if (ll == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else if (!PyInt_Check(t_o)) {
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	  }
	}
	return;

      case CORBA::tk_ulonglong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    CORBA::ULongLong ull = PyLong_AsUnsignedLongLong(t_o);
	    if (ull == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else if (PyInt_Check(t_o)) {
	    long l = PyInt_AS_LONG(t_o);
	    if (l < 0)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else {
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	  }
	}
	return;
#else
      case 23:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);

      case 24:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
      default:
	OMNIORB_ASSERT(0);
      }
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (len != arr_len)
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);

      switch (etk) {
      case CORBA::tk_short:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

	  if (long_val < -0x8000 || long_val > 0x7fff)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	}
	return;

      case CORBA::tk_long:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

#if SIZEOF_LONG > 4
	  if (long_val < -0x80000000L || long_val > 0x7fffffffL)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	}
	return;

      case CORBA::tk_ushort:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o))
	    long_val = PyInt_AS_LONG(t_o);
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange,
			    compstatus);
	    }
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

	  if (long_val < 0 || long_val > 0xffff)
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	}
	return;

      case CORBA::tk_ulong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ulong_val = PyLong_AsUnsignedLong(t_o);
	    if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (ulong_val > 0xffffffffL)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
#if SIZEOF_LONG > 4
	    if (long_val < 0 || long_val > 0xffffffffL)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#else
	    if (long_val < 0)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_float:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (!(PyFloat_Check(t_o) || PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_double:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (!(PyFloat_Check(t_o) || PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

      case CORBA::tk_boolean:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (!(PyInt_Check(t_o) || PyLong_Check(t_o)))
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	return;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    CORBA::LongLong ll = PyLong_AsLongLong(t_o);
	    if (ll == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else if (!PyInt_Check(t_o)) {
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	  }
	}
	return;

      case CORBA::tk_ulonglong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    CORBA::ULongLong ull = PyLong_AsUnsignedLongLong(t_o);
	    if (ull == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	  }
	  else if (PyInt_Check(t_o)) {
	    long l = PyInt_AS_LONG(t_o);
	    if (l < 0)
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else {
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	  }
	}
	return;
#else
      case 23:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);

      case 24:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
      default:
	OMNIORB_ASSERT(0);
      }
    }
    else
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
  else { // Complex type
    if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (len != arr_len)
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);

      for (i=0; i < len; i++) {
	omniPy::validateType(elm_desc, PyList_GET_ITEM(a_o, i),
			     compstatus, track);
      }
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (len != arr_len)
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);

      for (i=0; i < len; i++) {
	omniPy::validateType(elm_desc, PyTuple_GET_ITEM(a_o, i),
			     compstatus, track);
      }
    }
    else
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
}

static void
validateTypeAlias(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{ // repoId, name, descr

  omniPy::validateType(PyTuple_GET_ITEM(d_o, 3), a_o, compstatus, track);
}

static void
validateTypeExcept(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{ // class, repoId, exc name, name, descriptor, ...

  // As with structs, the descriptor tuple has twice the number of
  // members plus 4.
  int cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;

  PyObject* name;
  PyObject* value;

  int i, j;
  for (i=0,j=4; i < cnt; i++) {
    name = PyTuple_GET_ITEM(d_o, j++);
    OMNIORB_ASSERT(PyString_Check(name));

    value = PyObject_GetAttr(a_o, name);
    if (!value)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

    Py_DECREF(value);

    omniPy::validateType(PyTuple_GET_ITEM(d_o, j++), value, compstatus, track);
  }
}

static void
validateTypeLongLong(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus,
		     PyObject* track)
{
#ifdef HAS_LongLong
  if (PyLong_Check(a_o)) {
    CORBA::LongLong ll = PyLong_AsLongLong(a_o);
    if (ll == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
  }
  else if (!PyInt_Check(a_o)) {
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
}

static void
validateTypeULongLong(PyObject* d_o, PyObject* a_o,
		      CORBA::CompletionStatus compstatus,
		      PyObject* track)
{
#ifdef HAS_LongLong
  if (PyLong_Check(a_o)) {
    CORBA::ULongLong ull = PyLong_AsUnsignedLongLong(a_o);
    if (ull == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
  }
  else if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
    if (l < 0)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
  }
  else {
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
}

static void
validateTypeLongDouble(PyObject* d_o, PyObject* a_o,
		       CORBA::CompletionStatus compstatus,
		       PyObject* track)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
}

static void
validateTypeWChar(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{
#ifdef PY_HAS_UNICODE
  if (!(PyUnicode_Check(a_o) && (PyUnicode_GET_SIZE(a_o) == 1)))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
}

static void
validateTypeWString(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus,
		    PyObject* track)
{ // max_length
#ifdef PY_HAS_UNICODE
  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);
  OMNIORB_ASSERT(PyInt_Check(t_o));

  CORBA::ULong max_len = PyInt_AS_LONG(t_o);

  if (!PyUnicode_Check(a_o))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  CORBA::ULong len = PyUnicode_GET_SIZE(a_o);

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, compstatus);

  // Annoyingly, we have to scan the string to check there are no
  // nulls
  Py_UNICODE* str = PyUnicode_AS_UNICODE(a_o);
  for (CORBA::ULong i=0; i<len; i++)
    if (str[i] == 0)
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_EmbeddedNullInPythonString, compstatus);
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
}

static void
validateTypeFixed(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{ // digits, scale
  if (!omnipyFixed_Check(a_o))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  PyObject* t_o;

  t_o = PyTuple_GET_ITEM(d_o, 1); int dlimit = PyInt_AS_LONG(t_o);
  t_o = PyTuple_GET_ITEM(d_o, 2); int slimit = PyInt_AS_LONG(t_o);

  int digits = ((omnipyFixedObject*)a_o)->ob_fixed->fixed_digits();
  int scale  = ((omnipyFixedObject*)a_o)->ob_fixed->fixed_scale();

  if (scale > slimit) {
    digits -= (scale - slimit);
    scale   = slimit;
  }
  if (digits > dlimit)
    OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_RangeError, compstatus);
}

// validateTypeValue and validateTypeValueBox are in pyValueType.cc

static void
validateTypeNative(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus,
		   PyObject* track)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
}

// validateTypeAbstractInterface is in pyAbstractIntf.cc

static void
validateTypeLocalInterface(PyObject* d_o, PyObject* a_o,
			   CORBA::CompletionStatus compstatus,
			   PyObject* track)
{
  OMNIORB_THROW(MARSHAL, MARSHAL_LocalObject, compstatus);
}

void
omniPy::
validateTypeIndirect(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus,
		     PyObject* track)
{
  PyObject* l = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(PyList_Check(l));
  PyObject* d = PyList_GET_ITEM(l, 0);

  if (PyString_Check(d)) {
    // Indirection to a repoId -- find the corresponding descriptor
    d = PyDict_GetItem(pyomniORBtypeMap, d);
    if (!d) OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_IncompletePythonType,
			  compstatus);
    Py_INCREF(d);
    PyList_SetItem(l, 0, d);
  }
  validateType(d, a_o, compstatus, track);
}


const omniPy::ValidateTypeFn omniPy::validateTypeFns[] = {
  validateTypeNull,
  validateTypeVoid,
  validateTypeShort,
  validateTypeLong,
  validateTypeUShort,
  validateTypeULong,
  validateTypeFloat,
  validateTypeDouble,
  validateTypeBoolean,
  validateTypeChar,
  validateTypeOctet,
  validateTypeAny,
  validateTypeTypeCode,
  validateTypePrincipal,
  validateTypeObjref,
  validateTypeStruct,
  validateTypeUnion,
  validateTypeEnum,
  validateTypeString,
  validateTypeSequence,
  validateTypeArray,
  validateTypeAlias,
  validateTypeExcept,
  validateTypeLongLong,
  validateTypeULongLong,
  validateTypeLongDouble,
  validateTypeWChar,
  validateTypeWString,
  validateTypeFixed,
  omniPy::validateTypeValue,
  omniPy::validateTypeValueBox,
  validateTypeNative,
  omniPy::validateTypeAbstractInterface,
  validateTypeLocalInterface
};


static void
marshalPyObjectNull(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
}

static void
marshalPyObjectVoid(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
}

static void
marshalPyObjectShort(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Short s;
  if (PyInt_Check(a_o))
    s = PyInt_AS_LONG(a_o);
  else
    s = PyLong_AsLong(a_o);
  s >>= stream;
}

static void
marshalPyObjectLong(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Long l;
  if (PyInt_Check(a_o))
    l = PyInt_AS_LONG(a_o);
  else
    l = PyLong_AsLong(a_o);
  l >>= stream;
}

static void
marshalPyObjectUShort(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::UShort us;
  if (PyInt_Check(a_o))
    us = PyInt_AS_LONG(a_o);
  else
    us = PyLong_AsLong(a_o);
  us >>= stream;
}

static void
marshalPyObjectULong(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::ULong ul;

  if (PyLong_Check(a_o))
    ul = PyLong_AsUnsignedLong(a_o);
  else // It's an int
    ul = PyInt_AS_LONG(a_o);

  ul >>= stream;
}

static void
marshalPyObjectFloat(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Float f;

  if (PyFloat_Check(a_o))
    f = (CORBA::Float)PyFloat_AS_DOUBLE(a_o);
  else if (PyInt_Check(a_o))
    f = (CORBA::Float)PyInt_AS_LONG(a_o);
  else
    f = (CORBA::Float)PyLong_AsDouble(a_o);

  f >>= stream;
}

static void
marshalPyObjectDouble(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Double d = 0; // Initialised to stop egcs complaining

  if (PyFloat_Check(a_o))
    d = (CORBA::Double)PyFloat_AS_DOUBLE(a_o);
  else if (PyInt_Check(a_o))
    d = (CORBA::Double)PyInt_AS_LONG(a_o);
  else
    d = (CORBA::Double)PyLong_AsDouble(a_o);

  d >>= stream;
}

static void
marshalPyObjectBoolean(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Boolean b;
  if (PyInt_Check(a_o))
    b = PyInt_AS_LONG(a_o);
  else
    b = PyLong_AsLong(a_o);
  stream.marshalBoolean(b);
}

static void
marshalPyObjectChar(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  char* str = PyString_AS_STRING(a_o);
  stream.marshalChar(str[0]);
}

static void
marshalPyObjectOctet(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  CORBA::Octet o;
  if (PyInt_Check(a_o))
    o = PyInt_AS_LONG(a_o);
  else
    o = PyLong_AsLong(a_o);
  stream.marshalOctet(o);
}

static void
marshalPyObjectAny(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  // TypeCode
  PyObject* t_o  = PyObject_GetAttrString(a_o, (char*)"_t");
  PyObject* desc = PyObject_GetAttrString(t_o, (char*)"_d");

  Py_DECREF(t_o); Py_DECREF(desc);

  omniPy::marshalTypeCode(stream, desc);

  // Any's contents
  t_o = PyObject_GetAttrString(a_o, (char*)"_v");
  Py_DECREF(t_o);
  omniPy::marshalPyObject(stream, desc, t_o);
}

static void
marshalPyObjectTypeCode(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  PyObject* t_o = PyObject_GetAttrString(a_o, (char*)"_d");
  Py_DECREF(t_o);
  omniPy::marshalTypeCode(stream, t_o);
}

static void
marshalPyObjectPrincipal(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  OMNIORB_ASSERT(0);
}

static void
marshalPyObjectObjref(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // repoId, name

  CORBA::Object_ptr obj;

  if (a_o == Py_None)
    obj = CORBA::Object::_nil();
  else
    obj = (CORBA::Object_ptr)omniPy::getTwin(a_o, OBJREF_TWIN);

  CORBA::Object::_marshalObjRef(obj, stream);
}

static void
marshalPyObjectStruct(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // class, repoId, struct name, {name, descriptor}

  int i, j;
  int       cnt   = (PyTuple_GET_SIZE(d_o) - 4) / 2;
  PyObject* name;
  PyObject* value;

  if (PyInstance_Check(a_o)) {
    PyObject* sdict = ((PyInstanceObject*)a_o)->in_dict;
    for (i=0,j=4; i < cnt; i++,j++) {
      name  = PyTuple_GET_ITEM(d_o, j++);
      value = PyDict_GetItem(sdict, name);

      if (value) {
	omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(d_o, j), value);
      }
      else {
	value = PyObject_GetAttr(a_o, name);
	Py_DECREF(value);
	omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(d_o, j), value);
      }
    }
  }
  else {
    for (i=0,j=4; i < cnt; i++,j++) {
      name  = PyTuple_GET_ITEM(d_o, j++);
      value = PyObject_GetAttr(a_o, name);
      Py_DECREF(value);
      omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(d_o, j), value);
    }
  }
}

static void
marshalPyObjectUnion(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // class,
  // repoId,
  // name,
  // discriminant descr,
  // default used,
  // ((label value, member name, member descr), ...),
  // default (label, name, descr) or None,
  // {label: (label, name, descr), ...}

  PyObject* udict = ((PyInstanceObject*)a_o)->in_dict;

  PyObject* discriminant = PyDict_GetItemString(udict, (char*)"_d");
  PyObject* value        = PyDict_GetItemString(udict, (char*)"_v");
  PyObject* t_o          = PyTuple_GET_ITEM(d_o, 4); // Discriminant descriptor
  PyObject* cdict        = PyTuple_GET_ITEM(d_o, 8);

  omniPy::marshalPyObject(stream, t_o, discriminant);

  t_o = PyDict_GetItem(cdict, discriminant);
  if (t_o) {
    // Discriminant found in case dictionary
    omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(t_o, 2), value);
  }
  else {
    // Is there a default case?
    t_o = PyTuple_GET_ITEM(d_o, 7);
    if (t_o != Py_None) {
      omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(t_o, 2), value);
    }
  }
}

static void
marshalPyObjectEnum(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // repoId, name, item list

  PyObject* ev = PyObject_GetAttrString(a_o, (char*)"_v");
  Py_DECREF(ev);
  CORBA::ULong e = PyInt_AS_LONG(ev);
  e >>= stream;
}

static void
marshalPyObjectString(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // max_length
  orbParameters::nativeCharCodeSet->marshalString(stream, stream.TCS_C(), 0,
						  PyString_GET_SIZE(a_o),
						  PyString_AS_STRING(a_o));
}

static void
marshalPyObjectSequence(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // element_desc, max_length

  PyObject* elm_desc = PyTuple_GET_ITEM(d_o, 1);
  PyObject* t_o;

  CORBA::ULong i, len;

  if (sequenceOptimisedType(elm_desc)) {
    CORBA::ULong etk = PyInt_AS_LONG(elm_desc);

    if (etk == CORBA::tk_octet) {
      len = PyString_GET_SIZE(a_o);
      len >>= stream;
      CORBA::Octet *l = (CORBA::Octet*)PyString_AS_STRING(a_o);
      stream.put_octet_array((const CORBA::Octet*)l, len);
    }
    else if (etk == CORBA::tk_char) {
      len = PyString_GET_SIZE(a_o);
      len >>= stream;
      CORBA::Char *l = (CORBA::Char*)PyString_AS_STRING(a_o);
      for (i=0; i<len; i++)
	stream.marshalChar(l[i]);
    }
    else if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      len >>= stream;

      switch (etk) {
      case CORBA::tk_short:
	{
	  CORBA::Short e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_long:
	{
	  CORBA::Long e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ushort:
	{
	  CORBA::UShort e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ulong:
	{
	  CORBA::ULong e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsUnsignedLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_float:
	{
	  CORBA::Float e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyFloat_Check(t_o))
	      e = PyFloat_AS_DOUBLE(t_o);
	    else if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsDouble(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_double:
	{
	  CORBA::Double e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyFloat_Check(t_o))
	      e = PyFloat_AS_DOUBLE(t_o);
	    else if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsDouble(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_boolean:
	{
	  CORBA::Boolean e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    stream.marshalBoolean(e);
	  }
	}
	break;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	{
	  CORBA::LongLong e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsLongLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ulonglong:
	{
	  CORBA::ULongLong e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsLongLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;
#endif
      default:
	OMNIORB_ASSERT(0);
      }
    }
    else {
      OMNIORB_ASSERT(PyTuple_Check(a_o));
      len = PyTuple_GET_SIZE(a_o);
      len >>= stream;

      switch (etk) {
      case CORBA::tk_short:
	{
	  CORBA::Short e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_long:
	{
	  CORBA::Long e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ushort:
	{
	  CORBA::UShort e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ulong:
	{
	  CORBA::ULong e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsUnsignedLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_float:
	{
	  CORBA::Float e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyFloat_Check(t_o))
	      e = PyFloat_AS_DOUBLE(t_o);
	    else if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsDouble(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_double:
	{
	  CORBA::Double e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyFloat_Check(t_o))
	      e = PyFloat_AS_DOUBLE(t_o);
	    else if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsDouble(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_boolean:
	{
	  CORBA::Boolean e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    stream.marshalBoolean(e);
	  }
	}
	break;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	{
	  CORBA::LongLong e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsLongLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ulonglong:
	{
	  CORBA::ULongLong e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsLongLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;
#endif
      default:
	OMNIORB_ASSERT(0);
      }
    }
  }
  else if (PyList_Check(a_o)) {
    len = PyList_GET_SIZE(a_o);
    len >>= stream;
    for (i=0; i < len; i++)
      omniPy::marshalPyObject(stream, elm_desc, PyList_GET_ITEM(a_o, i));
  }
  else {
    len = PyTuple_GET_SIZE(a_o);
    len >>= stream;
    for (i=0; i < len; i++)
      omniPy::marshalPyObject(stream, elm_desc, PyTuple_GET_ITEM(a_o, i));
  }
}


static void
marshalPyObjectArray(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // element_desc, length

  PyObject* elm_desc = PyTuple_GET_ITEM(d_o, 1);
  PyObject* t_o;

  CORBA::ULong i, len;

  if (sequenceOptimisedType(elm_desc)) {
    CORBA::ULong etk = PyInt_AS_LONG(elm_desc);

    if (etk == CORBA::tk_octet) {
      len = PyString_GET_SIZE(a_o);
      CORBA::Octet *l = (CORBA::Octet*)PyString_AS_STRING(a_o);
      stream.put_octet_array(l, len);
    }
    else if (etk == CORBA::tk_char) {
      len = PyString_GET_SIZE(a_o);
      CORBA::Octet *l = (CORBA::Octet*)PyString_AS_STRING(a_o);
      for (i=0; i<len; i++)
	stream.marshalChar(l[i]);
    }
    else if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);

      switch (etk) {
      case CORBA::tk_short:
	{
	  CORBA::Short e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_long:
	{
	  CORBA::Long e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ushort:
	{
	  CORBA::UShort e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ulong:
	{
	  CORBA::ULong e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsUnsignedLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_float:
	{
	  CORBA::Float e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyFloat_Check(t_o))
	      e = PyFloat_AS_DOUBLE(t_o);
	    else if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsDouble(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_double:
	{
	  CORBA::Double e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyFloat_Check(t_o))
	      e = PyFloat_AS_DOUBLE(t_o);
	    else if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsDouble(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_boolean:
	{
	  CORBA::Boolean e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    stream.marshalBoolean(e);
	  }
	}
	break;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	{
	  CORBA::LongLong e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsLongLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ulonglong:
	{
	  CORBA::ULongLong e;
	  for (i=0; i < len; i++) {
	    t_o = PyList_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsLongLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;
#endif
      default:
	OMNIORB_ASSERT(0);
      }
    }
    else {
      OMNIORB_ASSERT(PyTuple_Check(a_o));
      len = PyTuple_GET_SIZE(a_o);

      switch (etk) {
      case CORBA::tk_short:
	{
	  CORBA::Short e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_long:
	{
	  CORBA::Long e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ushort:
	{
	  CORBA::UShort e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ulong:
	{
	  CORBA::ULong e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsUnsignedLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_float:
	{
	  CORBA::Float e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyFloat_Check(t_o))
	      e = PyFloat_AS_DOUBLE(t_o);
	    else if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsDouble(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_double:
	{
	  CORBA::Double e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyFloat_Check(t_o))
	      e = PyFloat_AS_DOUBLE(t_o);
	    else if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsDouble(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_boolean:
	{
	  CORBA::Boolean e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyInt_Check(t_o))
	      e = PyInt_AS_LONG(t_o);
	    else
	      e = PyLong_AsLong(t_o);
	    stream.marshalBoolean(e);
	  }
	}
	break;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	{
	  CORBA::LongLong e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsLongLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;

      case CORBA::tk_ulonglong:
	{
	  CORBA::ULongLong e;
	  for (i=0; i < len; i++) {
	    t_o = PyTuple_GET_ITEM(a_o, i);
	    if (PyLong_Check(t_o))
	      e = PyLong_AsLongLong(t_o);
	    else
	      e = PyInt_AS_LONG(t_o);
	    e >>= stream;
	  }
	}
	break;
#endif
      default:
	OMNIORB_ASSERT(0);
      }
    }
  }
  else if (PyList_Check(a_o)) {
    len = PyList_GET_SIZE(a_o);
    for (i=0; i < len; i++)
      omniPy::marshalPyObject(stream, elm_desc, PyList_GET_ITEM(a_o, i));
  }
  else {
    len = PyTuple_GET_SIZE(a_o);
    for (i=0; i < len; i++)
      omniPy::marshalPyObject(stream, elm_desc, PyTuple_GET_ITEM(a_o, i));
  }
}

static void
marshalPyObjectAlias(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // repoId, name, descr

  omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(d_o, 3), a_o);
}

static void
marshalPyObjectExcept(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // class, repoId, exc name, name, descriptor, ...

  PyObject*    t_o  = PyTuple_GET_ITEM(d_o, 2);
  CORBA::ULong slen = PyString_GET_SIZE(t_o) + 1;
  slen >>= stream;

  char* str = PyString_AS_STRING(t_o);
  stream.put_octet_array((const CORBA::Octet*)((const char*)str), slen);

  int cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;

  PyObject* name;
  PyObject* value;

  int i, j;
  for (i=0,j=4; i < cnt; i++) {
    name  = PyTuple_GET_ITEM(d_o, j++);
    value = PyObject_GetAttr(a_o, name);
    Py_DECREF(value);
    omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(d_o, j++), value);
  }
}

static void
marshalPyObjectLongLong(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
#ifdef HAS_LongLong
  CORBA::LongLong ll = 0; // Initialised to stop egcs complaining

  if (PyLong_Check(a_o))
    ll = PyLong_AsLongLong(a_o);
  else // It's an int
    ll = PyInt_AS_LONG(a_o);

  ll >>= stream;
#else
  OMNIORB_ASSERT(0);
#endif
}

static void
marshalPyObjectULongLong(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
#ifdef HAS_LongLong
  CORBA::ULongLong ull = 0; // Initialised to stop egcs complaining

  if (PyLong_Check(a_o))
    ull = PyLong_AsUnsignedLongLong(a_o);
  else // It's an int
    ull = PyInt_AS_LONG(a_o);

  ull >>= stream;
#else
  OMNIORB_ASSERT(0);
#endif
}

static void
marshalPyObjectLongDouble(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  OMNIORB_ASSERT(0);
}

static void
marshalPyObjectWChar(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
#ifdef PY_HAS_UNICODE
  OMNIORB_CHECK_TCS_W_FOR_MARSHAL(stream.TCS_W(), stream);
  Py_UNICODE* str = PyUnicode_AS_UNICODE(a_o);
  stream.TCS_W()->marshalWChar(stream, str[0]);
#else
  OMNIORB_ASSERT(0);
#endif
}

static void
marshalPyObjectWString(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
#ifdef PY_HAS_UNICODE
  OMNIORB_CHECK_TCS_W_FOR_MARSHAL(stream.TCS_W(), stream);

#  ifdef Py_UNICODE_WIDE
  PyObject* ustr = PyUnicode_AsUTF16String(a_o);
  if (!ustr) {
    // Now we're in trouble...
    if (omniORB::trace(1)) {
      PyErr_Print();
    }
    PyErr_Clear();
    OMNIORB_THROW(UNKNOWN, UNKNOWN_PythonException,
		  (CORBA::CompletionStatus)stream.completion());
  }
  omniPy::PyRefHolder h(ustr);
  OMNIORB_ASSERT(PyString_Check(ustr));

  char* str = PyString_AS_STRING(ustr) + 2; // Skip BOM

#  else
  Py_UNICODE* str = PyUnicode_AS_UNICODE(a_o);

#  endif
  stream.TCS_W()->marshalWString(stream,
				 0,
				 PyUnicode_GET_SIZE(a_o),
				 (const omniCodeSet::UniChar*)str);
#else
  OMNIORB_ASSERT(0);
#endif
}

static void
marshalPyObjectFixed(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // digits, scale
  PyObject* t_o;

  t_o = PyTuple_GET_ITEM(d_o, 1); int dlimit = PyInt_AS_LONG(t_o);
  t_o = PyTuple_GET_ITEM(d_o, 2); int slimit = PyInt_AS_LONG(t_o);

  CORBA::Fixed f(*(((omnipyFixedObject*)a_o)->ob_fixed));
  f.PR_setLimits(dlimit, slimit);
  f >>= stream;
}

// marshalPyObjectValue and marshalPyObjectValueBox are in pyValueType.cc

static void
marshalPyObjectNative(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  OMNIORB_ASSERT(0);
}

// marshalPyObjectAbstractInterface is in pyAbstractIntf.cc

static void
marshalPyObjectLocalInterface(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  OMNIORB_ASSERT(0);
}

void
omniPy::
marshalPyObjectIndirect(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{
  PyObject* l = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(PyList_Check(l));
  PyObject* d = PyList_GET_ITEM(l, 0);

  OMNIORB_ASSERT(!PyString_Check(d));
  marshalPyObject(stream, d, a_o);
}


const omniPy::MarshalPyObjectFn omniPy::marshalPyObjectFns[] = {
  marshalPyObjectNull,
  marshalPyObjectVoid,
  marshalPyObjectShort,
  marshalPyObjectLong,
  marshalPyObjectUShort,
  marshalPyObjectULong,
  marshalPyObjectFloat,
  marshalPyObjectDouble,
  marshalPyObjectBoolean,
  marshalPyObjectChar,
  marshalPyObjectOctet,
  marshalPyObjectAny,
  marshalPyObjectTypeCode,
  marshalPyObjectPrincipal,
  marshalPyObjectObjref,
  marshalPyObjectStruct,
  marshalPyObjectUnion,
  marshalPyObjectEnum,
  marshalPyObjectString,
  marshalPyObjectSequence,
  marshalPyObjectArray,
  marshalPyObjectAlias,
  marshalPyObjectExcept,
  marshalPyObjectLongLong,
  marshalPyObjectULongLong,
  marshalPyObjectLongDouble,
  marshalPyObjectWChar,
  marshalPyObjectWString,
  marshalPyObjectFixed,
  omniPy::marshalPyObjectValue,
  omniPy::marshalPyObjectValueBox,
  marshalPyObjectNative,
  omniPy::marshalPyObjectAbstractInterface,
  marshalPyObjectLocalInterface
};



static PyObject*
unmarshalPyObjectNull(cdrStream& stream, PyObject* d_o)
{
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
unmarshalPyObjectVoid(cdrStream& stream, PyObject* d_o)
{
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
unmarshalPyObjectShort(cdrStream& stream, PyObject* d_o)
{
  CORBA::Short s;
  s <<= stream;
  return PyInt_FromLong(s);
}

static PyObject*
unmarshalPyObjectLong(cdrStream& stream, PyObject* d_o)
{
  CORBA::Long l;
  l <<= stream;
  return PyInt_FromLong(l);
}

static PyObject*
unmarshalPyObjectUShort(cdrStream& stream, PyObject* d_o)
{
  CORBA::UShort us;
  us <<= stream;
  return PyInt_FromLong(us);
}

static PyObject*
unmarshalPyObjectULong(cdrStream& stream, PyObject* d_o)
{
  CORBA::ULong ul;
  ul <<= stream;
  return PyLong_FromUnsignedLong(ul);
}

static PyObject*
unmarshalPyObjectFloat(cdrStream& stream, PyObject* d_o)
{
  CORBA::Float f;
  f <<= stream;
  return PyFloat_FromDouble((double)f);
}

static PyObject*
unmarshalPyObjectDouble(cdrStream& stream, PyObject* d_o)
{
  CORBA::Double d;
  d <<= stream;
  return PyFloat_FromDouble(d);
}

static PyObject*
unmarshalPyObjectBoolean(cdrStream& stream, PyObject* d_o)
{
  CORBA::Boolean b = stream.unmarshalBoolean();
  return PyBool_FromLong(b);
}

static PyObject*
unmarshalPyObjectChar(cdrStream& stream, PyObject* d_o)
{
  CORBA::Char c   = stream.unmarshalChar();
  PyObject*   r_o = PyString_FromStringAndSize(0, 1);
  char*       str = PyString_AS_STRING(r_o);
  str[0]          = c;
  str[1]          = '\0';
  return r_o;
}

static PyObject*
unmarshalPyObjectOctet(cdrStream& stream, PyObject* d_o)
{
  CORBA::Octet o = stream.unmarshalOctet();
  return PyInt_FromLong(o);
}

static PyObject*
unmarshalPyObjectAny(cdrStream& stream, PyObject* d_o)
{
  // TypeCode
  PyObject* desc     = omniPy::unmarshalTypeCode(stream);
  PyObject* argtuple = PyTuple_New(1);
  PyTuple_SET_ITEM(argtuple, 0, desc);

  omniPy::PyRefHolder argtuple_holder(argtuple);

  PyObject* tcobj = PyEval_CallObject(omniPy::pyCreateTypeCode, argtuple);

  if (!tcobj) {
    // Return exception to caller
    return 0;
  }

  omniPy::PyRefHolder tcobj_holder(tcobj);

  PyObject* value = omniPy::unmarshalPyObject(stream, desc);

  argtuple = argtuple_holder.change(PyTuple_New(2));
  PyTuple_SET_ITEM(argtuple, 0, tcobj_holder.retn());
  PyTuple_SET_ITEM(argtuple, 1, value);

  PyObject* r_o = PyEval_CallObject(omniPy::pyCORBAAnyClass, argtuple);
  return r_o;
}

static PyObject*
unmarshalPyObjectTypeCode(cdrStream& stream, PyObject* d_o)
{
  PyObject* t_o      = omniPy::unmarshalTypeCode(stream);
  PyObject* argtuple = PyTuple_New(1);
  PyTuple_SET_ITEM(argtuple, 0, t_o);
  PyObject* r_o      = PyEval_CallObject(omniPy::pyCreateTypeCode, argtuple);
  Py_DECREF(argtuple);
  return r_o;
}

static PyObject*
unmarshalPyObjectPrincipal(cdrStream& stream, PyObject* d_o)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
}

static PyObject*
unmarshalPyObjectObjref(cdrStream& stream, PyObject* d_o)
{ // repoId, name

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);

  const char* targetRepoId;

  if (t_o == Py_None)
    targetRepoId = 0;
  else {
    OMNIORB_ASSERT(PyString_Check(t_o));
    targetRepoId = PyString_AS_STRING(t_o);
    if (targetRepoId[0] == '\0') { // Empty string => CORBA.Object
      targetRepoId = CORBA::Object::_PD_repoId;
    }
  }
  CORBA::Object_ptr obj = omniPy::UnMarshalObjRef(targetRepoId, stream);
  return omniPy::createPyCorbaObjRef(targetRepoId, obj);
}

static PyObject*
unmarshalPyObjectStruct(cdrStream& stream, PyObject* d_o)
{ // class, repoId, struct name, name, descriptor, ...

  PyObject* strclass = PyTuple_GET_ITEM(d_o, 1);
  int       cnt      = (PyTuple_GET_SIZE(d_o) - 4) / 2;
  PyObject* strtuple = PyTuple_New(cnt);

  omniPy::PyRefHolder strtuple_holder(strtuple);

  int i, j;
  for (i=0, j=5; i < cnt; i++, j+=2) {
    PyTuple_SET_ITEM(strtuple, i,
		     omniPy::unmarshalPyObject(stream,
					       PyTuple_GET_ITEM(d_o, j)));
  }
  return PyEval_CallObject(strclass, strtuple);
}

static PyObject*
unmarshalPyObjectUnion(cdrStream& stream, PyObject* d_o)
{ // class,
  // repoId,
  // name,
  // discriminant descr,
  // default used,
  // ((label value, member name, member descr), ...),
  // default (label, name, descr) or None,
  // {label: (label, name, descr), ...}

  PyObject* unclass      = PyTuple_GET_ITEM(d_o, 1);
  PyObject* t_o          = PyTuple_GET_ITEM(d_o, 4);
  PyObject* discriminant = omniPy::unmarshalPyObject(stream, t_o);
  PyObject* value;
  PyObject* cdict        = PyTuple_GET_ITEM(d_o, 8);

  omniPy::PyRefHolder discriminant_holder(discriminant);

  t_o = PyDict_GetItem(cdict, discriminant);
  if (t_o) {
    // Discriminant found in case dictionary
    OMNIORB_ASSERT(PyTuple_Check(t_o));
    value = omniPy::unmarshalPyObject(stream, PyTuple_GET_ITEM(t_o, 2));
  }
  else {
    // Is there a default case?
    t_o = PyTuple_GET_ITEM(d_o, 7);
    if (t_o != Py_None) {
      OMNIORB_ASSERT(PyTuple_Check(t_o));
      value = omniPy::unmarshalPyObject(stream, PyTuple_GET_ITEM(t_o, 2));
    }
    else {
      Py_INCREF(Py_None);
      value = Py_None;
    }
  }
  PyObject* untuple = PyTuple_New(2);
  PyTuple_SET_ITEM(untuple, 0, discriminant_holder.retn());
  PyTuple_SET_ITEM(untuple, 1, value);

  PyObject* r_o = PyEval_CallObject(unclass, untuple);
  Py_DECREF(untuple);
  return r_o;
}

static PyObject*
unmarshalPyObjectEnum(cdrStream& stream, PyObject* d_o)
{ // repoId, name, item list

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 3);

  OMNIORB_ASSERT(PyTuple_Check(t_o));

  CORBA::ULong e;
  e <<= stream;

  if (e >= (CORBA::ULong)PyTuple_GET_SIZE(t_o))
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidEnumValue,
		  (CORBA::CompletionStatus)stream.completion());

  PyObject* ev = PyTuple_GET_ITEM(t_o, e);
  Py_INCREF(ev);
  return ev;
}

static PyObject*
unmarshalPyObjectString(cdrStream& stream, PyObject* d_o)
{ // max_length

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);

  OMNIORB_ASSERT(PyInt_Check(t_o));

  CORBA::ULong max_len = PyInt_AS_LONG(t_o);

  char* s;
  CORBA::ULong len =
    orbParameters::nativeCharCodeSet->unmarshalString(stream, stream.TCS_C(),
						      max_len, s);

  PyObject* r_o = PyString_FromStringAndSize(s, len);
  _CORBA_String_helper::free(s);
  return r_o;
}

static PyObject*
unmarshalPyObjectSequence(cdrStream& stream, PyObject* d_o)
{ // element_desc, max_length

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 2);
  PyObject* r_o = 0;

  OMNIORB_ASSERT(PyInt_Check(t_o));

  CORBA::ULong max_len = PyInt_AS_LONG(t_o);
  CORBA::ULong len;
  len <<= stream;

  PyObject* elm_desc = PyTuple_GET_ITEM(d_o, 1);

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong,
		  (CORBA::CompletionStatus)stream.completion());

  // If the sequence length field is greater than the number of
  // octets left in the message, the sequence length is invalid.
  if (!stream.checkInputOverrun(1, len)) {

    if (PyInt_Check(elm_desc) && PyInt_AS_LONG(elm_desc) <= 1) {
      // Sequence is a bizarre sequence of void or null, meaning that
      // the data takes up no space!  The overrun is therefore not an
      // error.
    }
    else {
      OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
                    (CORBA::CompletionStatus)stream.completion());
    }
  }

  CORBA::ULong i;

  if (sequenceOptimisedType(elm_desc)) {
    CORBA::ULong etk = PyInt_AS_LONG(elm_desc);

    if (etk == CORBA::tk_octet) {
      r_o = PyString_FromStringAndSize(0, len);
      omniPy::PyRefHolder r_o_holder(r_o);
      CORBA::Octet* c = (CORBA::Octet*)PyString_AS_STRING(r_o);
      stream.get_octet_array(c, len);
      return r_o_holder.retn();
    }
    else if (etk == CORBA::tk_char) {
      r_o = PyString_FromStringAndSize(0, len);
      omniPy::PyRefHolder r_o_holder(r_o);
      CORBA::Char* c = (CORBA::Char*)PyString_AS_STRING(r_o);

      for (i=0; i<len; i++) c[i] = stream.unmarshalChar();

      return r_o_holder.retn();
    }
    else {
      r_o = PyList_New(len); omniPy::PyRefHolder r_o_holder(r_o);

      switch(etk) {
      case CORBA::tk_short:
	{
	  CORBA::Short e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyInt_FromLong(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_long:
	{
	  CORBA::Long e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyInt_FromLong(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_ushort:
	{
	  CORBA::UShort e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyInt_FromLong(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_ulong:
	{
	  CORBA::ULong e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyLong_FromUnsignedLong(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_float:
	{
	  CORBA::Float e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyFloat_FromDouble(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_double:
	{
	  CORBA::Double e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyFloat_FromDouble(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_boolean:
	{
	  CORBA::Boolean e;
	  for (i=0; i < len; i++) {
	    e = stream.unmarshalBoolean();
	    PyList_SET_ITEM(r_o, i, PyBool_FromLong(e));
	  }
	}
	return r_o_holder.retn();
	    
#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	{
	  CORBA::LongLong e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, MyPyLong_FromLongLong(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_ulonglong:
	{
	  CORBA::ULongLong e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyLong_FromUnsignedLongLong(e));
	  }
	}
	return r_o_holder.retn();
#else
      case 23:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		      (CORBA::CompletionStatus)stream.completion());
	return 0;

      case 24:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		      (CORBA::CompletionStatus)stream.completion());
	return 0;
#endif
      default:
	OMNIORB_ASSERT(0);
	return 0;
      }
    }
  }
  else {
    r_o = PyList_New(len); omniPy::PyRefHolder r_o_holder(r_o);

    for (i=0; i < len; i++)
      PyList_SET_ITEM(r_o, i, omniPy::unmarshalPyObject(stream, elm_desc));

    return r_o_holder.retn();
  }
  OMNIORB_ASSERT(0);
  return 0;
}

static PyObject*
unmarshalPyObjectArray(cdrStream& stream, PyObject* d_o)
{ // element_desc, length

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 2);
  PyObject* r_o = 0;

  OMNIORB_ASSERT(PyInt_Check(t_o));

  CORBA::ULong len      = PyInt_AS_LONG(t_o);
  PyObject*    elm_desc = PyTuple_GET_ITEM(d_o, 1);
  CORBA::ULong i;

  if (sequenceOptimisedType(elm_desc)) {
    CORBA::ULong etk = PyInt_AS_LONG(elm_desc);

    if (etk == CORBA::tk_octet) {
      r_o = PyString_FromStringAndSize(0, len);
      omniPy::PyRefHolder r_o_holder(r_o);
      CORBA::Octet* c = (CORBA::Octet*)PyString_AS_STRING(r_o);
      stream.get_octet_array(c, len);
      return r_o_holder.retn();
    }
    else if (etk == CORBA::tk_char) {
      r_o = PyString_FromStringAndSize(0, len);
      omniPy::PyRefHolder r_o_holder(r_o);
      CORBA::Char* c = (CORBA::Char*)PyString_AS_STRING(r_o);

      for (i=0; i<len; i++) c[i] = stream.unmarshalChar();

      return r_o_holder.retn();
    }
    else {
      r_o = PyList_New(len); omniPy::PyRefHolder r_o_holder(r_o);

      switch(etk) {
      case CORBA::tk_short:
	{
	  CORBA::Short e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyInt_FromLong(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_long:
	{
	  CORBA::Long e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyInt_FromLong(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_ushort:
	{
	  CORBA::UShort e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyInt_FromLong(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_ulong:
	{
	  CORBA::ULong e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyLong_FromUnsignedLong(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_float:
	{
	  CORBA::Float e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyFloat_FromDouble(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_double:
	{
	  CORBA::Double e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyFloat_FromDouble(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_boolean:
	{
	  CORBA::Boolean e;
	  for (i=0; i < len; i++) {
	    e = stream.unmarshalBoolean();
	    PyList_SET_ITEM(r_o, i, PyBool_FromLong(e));
	  }
	}
	return r_o_holder.retn();
	    
#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	{
	  CORBA::LongLong e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, MyPyLong_FromLongLong(e));
	  }
	}
	return r_o_holder.retn();

      case CORBA::tk_ulonglong:
	{
	  CORBA::ULongLong e;
	  for (i=0; i < len; i++) {
	    e <<= stream;
	    PyList_SET_ITEM(r_o, i, PyLong_FromUnsignedLongLong(e));
	  }
	}
	return r_o_holder.retn();
#else
      case 23:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		      (CORBA::CompletionStatus)stream.completion());
	return 0;
	
      case 24:
	OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		      (CORBA::CompletionStatus)stream.completion());
	return 0;
#endif
      default:
	OMNIORB_ASSERT(0);
	return 0;
      }
    }
  }
  else {
    r_o = PyList_New(len); omniPy::PyRefHolder r_o_holder(r_o);

    for (i=0; i < len; i++)
      PyList_SET_ITEM(r_o, i, omniPy::unmarshalPyObject(stream, elm_desc));

    return r_o_holder.retn();
  }
  OMNIORB_ASSERT(0);
  return 0;
}


static PyObject*
unmarshalPyObjectAlias(cdrStream& stream, PyObject* d_o)
{ // repoId, name, descr

  return omniPy::unmarshalPyObject(stream, PyTuple_GET_ITEM(d_o, 3));
}

static PyObject*
unmarshalPyObjectExcept(cdrStream& stream, PyObject* d_o)
{ // class, repoId, exc name, name, descriptor, ...

  // Throw away the repoId. By the time we get here, we already
  // know it.
  // *** Should maybe check to see if it's what we're expecting
  CORBA::ULong len; len <<= stream;
  stream.skipInput(len);

  PyObject* strclass = PyTuple_GET_ITEM(d_o, 1);
  int       cnt      = (PyTuple_GET_SIZE(d_o) - 4) / 2;
  PyObject* strtuple = PyTuple_New(cnt);

  omniPy::PyRefHolder strtuple_holder(strtuple);

  int i, j;
  for (i=0, j=5; i < cnt; i++, j+=2) {
    PyTuple_SET_ITEM(strtuple, i,
		     omniPy::unmarshalPyObject(stream,
					       PyTuple_GET_ITEM(d_o, j)));
  }
  return PyEval_CallObject(strclass, strtuple);
}

static PyObject*
unmarshalPyObjectLongLong(cdrStream& stream, PyObject* d_o)
{
#ifdef HAS_LongLong
  CORBA::LongLong ll;
  ll <<= stream;
  return MyPyLong_FromLongLong(ll);
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
#endif
}

static PyObject*
unmarshalPyObjectULongLong(cdrStream& stream, PyObject* d_o)
{
#ifdef HAS_LongLong
  CORBA::ULongLong ull;
  ull <<= stream;
  return PyLong_FromUnsignedLongLong(ull);
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
#endif
}

static PyObject*
unmarshalPyObjectLongDouble(cdrStream& stream, PyObject* d_o)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
}

static PyObject*
unmarshalPyObjectWChar(cdrStream& stream, PyObject* d_o)
{
#ifdef PY_HAS_UNICODE
  OMNIORB_CHECK_TCS_W_FOR_UNMARSHAL(stream.TCS_W(), stream);

  Py_UNICODE  c   = stream.TCS_W()->unmarshalWChar(stream);
  PyObject*   r_o = PyUnicode_FromUnicode(0, 1);
  Py_UNICODE* str = PyUnicode_AS_UNICODE(r_o);
  str[0]          = c;
  str[1]          = 0;
  return r_o;
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
#endif
}

static PyObject*
unmarshalPyObjectWString(cdrStream& stream, PyObject* d_o)
{ // max_length

#ifdef PY_HAS_UNICODE
  OMNIORB_CHECK_TCS_W_FOR_UNMARSHAL(stream.TCS_W(), stream);

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);

  OMNIORB_ASSERT(PyInt_Check(t_o));

  CORBA::ULong max_len = PyInt_AS_LONG(t_o);

  omniCodeSet::UniChar* us;
  CORBA::ULong len = stream.TCS_W()->unmarshalWString(stream, max_len, us);

#  ifdef Py_UNICODE_WIDE
#    if _OMNIORB_HOST_BYTE_ORDER_ == 0
  int byteorder = 1;  // Big endian
#    else
  int byteorder = -1; // Little endian
#    endif
  PyObject* r_o = PyUnicode_DecodeUTF16((const char*)us, len*2, 0, &byteorder);

#  else
  PyObject* r_o = PyUnicode_FromUnicode((Py_UNICODE*)us, len);
#  endif
  omniCodeSetUtil::freeU(us);
  return r_o;
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
#endif
}

static PyObject*
unmarshalPyObjectFixed(cdrStream& stream, PyObject* d_o)
{ // digits, scale
  PyObject* t_o;

  t_o = PyTuple_GET_ITEM(d_o, 1); int dlimit = PyInt_AS_LONG(t_o);
  t_o = PyTuple_GET_ITEM(d_o, 2); int slimit = PyInt_AS_LONG(t_o);

  CORBA::Fixed f;
  f.PR_setLimits(dlimit, slimit);

  f <<= stream;

  return omniPy::newFixedObject(f);
}

// unmarshalPyObjectValue is in pyValueType.cc. It does both values
// and valueboxes.

static PyObject*
unmarshalPyObjectNative(cdrStream& stream, PyObject* d_o)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
}

// unmarshalPyObjectAbstractInterface is in pyAbstractIntf.cc

static PyObject*
unmarshalPyObjectLocalInterface(cdrStream& stream, PyObject* d_o)
{
  OMNIORB_THROW(MARSHAL, MARSHAL_LocalObject,
		(CORBA::CompletionStatus)stream.completion());
  return 0;
}

PyObject*
omniPy::
unmarshalPyObjectIndirect(cdrStream& stream, PyObject* d_o)
{
  PyObject* l = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(PyList_Check(l));
  PyObject* d = PyList_GET_ITEM(l, 0);

  if (PyString_Check(d)) {
    // Indirection to a repoId -- find the corresponding descriptor
    d = PyDict_GetItem(pyomniORBtypeMap, d);
    if (!d)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_IncompletePythonType,
		    (CORBA::CompletionStatus)stream.completion());
    Py_INCREF(d);
    PyList_SetItem(l, 0, d);
  }
  return unmarshalPyObject(stream, d);
}


const omniPy::UnmarshalPyObjectFn omniPy::unmarshalPyObjectFns[] = {
  unmarshalPyObjectNull,
  unmarshalPyObjectVoid,
  unmarshalPyObjectShort,
  unmarshalPyObjectLong,
  unmarshalPyObjectUShort,
  unmarshalPyObjectULong,
  unmarshalPyObjectFloat,
  unmarshalPyObjectDouble,
  unmarshalPyObjectBoolean,
  unmarshalPyObjectChar,
  unmarshalPyObjectOctet,
  unmarshalPyObjectAny,
  unmarshalPyObjectTypeCode,
  unmarshalPyObjectPrincipal,
  unmarshalPyObjectObjref,
  unmarshalPyObjectStruct,
  unmarshalPyObjectUnion,
  unmarshalPyObjectEnum,
  unmarshalPyObjectString,
  unmarshalPyObjectSequence,
  unmarshalPyObjectArray,
  unmarshalPyObjectAlias,
  unmarshalPyObjectExcept,
  unmarshalPyObjectLongLong,
  unmarshalPyObjectULongLong,
  unmarshalPyObjectLongDouble,
  unmarshalPyObjectWChar,
  unmarshalPyObjectWString,
  unmarshalPyObjectFixed,
  omniPy::unmarshalPyObjectValue,
  omniPy::unmarshalPyObjectValue, // Same function as value
  unmarshalPyObjectNative,
  omniPy::unmarshalPyObjectAbstractInterface,
  unmarshalPyObjectLocalInterface
};



static PyObject*
copyArgumentNull(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus)
{
  if (a_o != Py_None)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
copyArgumentVoid(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus)
{
  if (a_o != Py_None)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
copyArgumentShort(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
  if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
    if (l < -0x8000 || l > 0x7fff)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    Py_INCREF(a_o); return a_o;
  }
  else if (PyLong_Check(a_o)) {
    long l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
    else if (l < -0x8000 || l > 0x7fff)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);

    return PyInt_FromLong(l);
  }
  else OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  return 0;
}

static PyObject*
copyArgumentLong(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus)
{
  if (PyInt_Check(a_o)) {
#if SIZEOF_LONG > 4
    long l = PyInt_AS_LONG(a_o);
    if (l < -0x80000000L || l > 0x7fffffffL)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
    Py_INCREF(a_o); return a_o;
  }
  else if (PyLong_Check(a_o)) {
    long l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
#if SIZEOF_LONG > 4
    if (l < -0x80000000L || l > 0x7fffffffL)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
    return PyInt_FromLong(l);
  }
  else OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  return 0;
}

static PyObject*
copyArgumentUShort(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{
  if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
    if (l < 0 || l > 0xffff)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    Py_INCREF(a_o); return a_o;
  }
  else if (PyLong_Check(a_o)) {
    long l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
    else if (l < 0 || l > 0xffff)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);

    return PyInt_FromLong(l);
  }
  else OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  return 0;
}

static PyObject*
copyArgumentULong(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
  if (PyLong_Check(a_o)) {
    unsigned long ul = PyLong_AsUnsignedLong(a_o);
    if (ul == (unsigned long)-1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
#if SIZEOF_LONG > 4
    if (ul > 0xffffffffL)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
    Py_INCREF(a_o); return a_o;
  }
  else if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
#if SIZEOF_LONG > 4
    if (l < 0 || l > 0xffffffffL)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
#else
    if (l < 0)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
    return PyLong_FromLong(l);
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  return 0;
}

static PyObject*
copyArgumentFloat(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
  // *** This accepts values that are too big to fit in a float. It
  // *** should complain.

  if (PyFloat_Check(a_o)) {
    Py_INCREF(a_o); return a_o;
  }
  else if (PyInt_Check(a_o)) {
    return PyFloat_FromDouble((double)(PyInt_AS_LONG(a_o)));
  }
  else if (PyLong_Check(a_o)) {
    double d;
    d = PyLong_AsDouble(a_o);
    if (d == -1.0 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
    return PyFloat_FromDouble(d);
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  return 0;
}

static PyObject*
copyArgumentDouble(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{
  if (PyFloat_Check(a_o)) {
    Py_INCREF(a_o); return a_o;
  }
  else if (PyInt_Check(a_o)) {
    return PyFloat_FromDouble((double)(PyInt_AS_LONG(a_o)));
  }
  else if (PyLong_Check(a_o)) {
    double d;
    d = PyLong_AsDouble(a_o);
    if (d == -1.0 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
    return PyFloat_FromDouble(d);
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  return 0;
}

static PyObject*
copyArgumentBoolean(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus)
{
  long l;
  
  if (PyBool_Check(a_o)) {
    Py_INCREF(a_o);
    return a_o;
  }
  else if (PyInt_Check(a_o)) {
    l = PyInt_AS_LONG(a_o);
  }
  else if (PyLong_Check(a_o)) {
    l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred())
      PyErr_Clear(); // Too big for long, but we consider it true
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

#if (PY_VERSION_HEX >= 0x02030000)
  if (l) {
    Py_INCREF(Py_True);
    return Py_True;
  }
  else {
    Py_INCREF(Py_False);
    return Py_False;
  }
#else
  return PyInt_FromLong(l ? 1 : 0);
#endif
}

static PyObject*
copyArgumentChar(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus)
{
  if ((PyString_Check(a_o) && (PyString_GET_SIZE(a_o) == 1))) {
    Py_INCREF(a_o); return a_o;
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  return 0;
}

static PyObject*
copyArgumentOctet(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
  if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
    if (l < 0 || l > 0xff)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    Py_INCREF(a_o); return a_o;
  }
  else if (PyLong_Check(a_o)) {
    long l = PyLong_AsLong(a_o);
    if (l == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
    if (l < 0 || l > 0xff)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    return PyInt_FromLong(l);
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  return 0;
}

static PyObject*
copyArgumentAny(PyObject* d_o, PyObject* a_o,
		CORBA::CompletionStatus compstatus)
{
  if (!omniPy::isInstance(a_o, omniPy::pyCORBAAnyClass))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  // TypeCode
  PyObject* tc = PyObject_GetAttrString(a_o, (char*)"_t");

  if (!tc) {
    PyErr_Clear();
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
  Py_DECREF(tc);

  if (!omniPy::isInstance(tc, omniPy::pyCORBATypeCodeClass))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  PyObject* desc = PyObject_GetAttrString(tc, (char*)"_d");
  Py_XDECREF(desc);
  if (!desc) {
    PyErr_Clear();
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }

  // Any's contents
  PyObject* val = PyObject_GetAttrString(a_o, (char*)"_v");
  if (!val) {
    PyErr_Clear();
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
  Py_DECREF(val);

  // Copy contents
  PyObject* cval = omniPy::copyArgument(desc, val, compstatus);

  // Construct new Any
  PyObject* argtuple = PyTuple_New(2);
  Py_INCREF(tc);
  PyTuple_SET_ITEM(argtuple, 0, tc);
  PyTuple_SET_ITEM(argtuple, 1, cval);
  PyObject* r_o = PyEval_CallObject(omniPy::pyCORBAAnyClass, argtuple);
  Py_DECREF(argtuple);

  return r_o;
}

static PyObject*
copyArgumentTypeCode(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus)
{
  if (!omniPy::isInstance(a_o, omniPy::pyCORBATypeCodeClass))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  PyObject* desc = PyObject_GetAttrString(a_o, (char*)"_d");
  Py_XDECREF(desc);

  if (!desc)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  Py_INCREF(a_o); return a_o;
}

static PyObject*
copyArgumentPrincipal(PyObject* d_o, PyObject* a_o,
		      CORBA::CompletionStatus compstatus)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
  return 0;
}

static PyObject*
copyArgumentObjref(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{ // repoId, name

  return omniPy::copyObjRefArgument(PyTuple_GET_ITEM(d_o, 1),
				    a_o, compstatus);
}

static PyObject*
copyArgumentStruct(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{ // class, repoId, struct name, name, descriptor, ...

  // The descriptor tuple has twice the number of struct members,
  // plus 4 -- the typecode kind, the Python class, the repoId,
  // and the struct name
  int cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;

  PyObject* t_o;
  PyObject* name;
  PyObject* value;
  PyObject* argtuple = PyTuple_New(cnt);
  omniPy::PyRefHolder argtuple_holder(argtuple);

  int i, j;

  // Optimise for the fast case, where the object is a class
  // instance with all attributes in its own dictionary
  if (PyInstance_Check(a_o)) {

    PyObject* sdict = ((PyInstanceObject*)a_o)->in_dict;

    for (i=0,j=4; i < cnt; i++,j++) {
      name  = PyTuple_GET_ITEM(d_o, j++); OMNIORB_ASSERT(PyString_Check(name));
      value = PyDict_GetItem(sdict, name);

      if (value) {
	t_o = omniPy::copyArgument(PyTuple_GET_ITEM(d_o, j),
				   value, compstatus);
      }
      else {
	// Not such a fast case after all
	value = PyObject_GetAttr(a_o, name);
	if (value) {
	  // DECREF now in case copyArgument() throws an exception.
	  // Safe because the struct object still holds a reference to
	  // the value.
	  Py_DECREF(value);
	  t_o = omniPy::copyArgument(PyTuple_GET_ITEM(d_o, j),
				     value, compstatus);
	}
	else
	  PyErr_Clear();
      }
      if (value) {
	PyTuple_SET_ITEM(argtuple, i, t_o);
      }
      else
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
    }
  }
  else {
    for (i=0,j=4; i < cnt; i++,j++) {
      name  = PyTuple_GET_ITEM(d_o, j++); OMNIORB_ASSERT(PyString_Check(name));
      value = PyObject_GetAttr(a_o, name);

      if (value) {
	Py_DECREF(value);
	t_o = omniPy::copyArgument(PyTuple_GET_ITEM(d_o, j),
				   value, compstatus);
	PyTuple_SET_ITEM(argtuple, i, t_o);
      }
      else {
	PyErr_Clear();
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
      }
    }
  }
  return PyEval_CallObject(PyTuple_GET_ITEM(d_o, 1), argtuple);
}

static PyObject*
copyArgumentUnion(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{ // class,
  // repoId,
  // name,
  // discriminant descr,
  // default used,
  // ((label value, member name, member descr), ...),
  // default (label, name, descr) or None,
  // {label: (label, name, descr), ...}

  if (!PyInstance_Check(a_o))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  PyObject* udict  = ((PyInstanceObject*)a_o)->in_dict;
  PyObject* discr  = PyDict_GetItemString(udict, (char*)"_d");
  PyObject* value  = PyDict_GetItemString(udict, (char*)"_v");

  if (!(discr && value))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  PyObject* t_o    = PyTuple_GET_ITEM(d_o, 4);
  PyObject* cdiscr = omniPy::copyArgument(t_o, discr, compstatus);

  omniPy::PyRefHolder cdiscr_holder(cdiscr);

  PyObject* cvalue = 0;
  PyObject* cdict  = PyTuple_GET_ITEM(d_o, 8);
  t_o              = PyDict_GetItem(cdict, discr);
  if (t_o) {
    // Discriminant found in case dictionary
    OMNIORB_ASSERT(PyTuple_Check(t_o));
    cvalue = omniPy::copyArgument(PyTuple_GET_ITEM(t_o, 2), value, compstatus);
  }
  else {
    // Is there a default case?
    t_o = PyTuple_GET_ITEM(d_o, 7);
    if (t_o == Py_None) {
      // No default
      Py_INCREF(Py_None);
      cvalue = Py_None;
    }
    else {
      OMNIORB_ASSERT(PyTuple_Check(t_o));
      cvalue = omniPy::copyArgument(PyTuple_GET_ITEM(t_o, 2),
				    value, compstatus);
    }
  }
  t_o = PyTuple_New(2);
  PyTuple_SET_ITEM(t_o, 0, cdiscr_holder.retn());
  PyTuple_SET_ITEM(t_o, 1, cvalue);
  PyObject* r_o = PyEval_CallObject(PyTuple_GET_ITEM(d_o, 1), t_o);
  Py_DECREF(t_o);
  return r_o;
}

static PyObject*
copyArgumentEnum(PyObject* d_o, PyObject* a_o,
		 CORBA::CompletionStatus compstatus)
{ // repoId, name, item list

  PyObject* ev = PyObject_GetAttrString(a_o, (char*)"_v");

  if (!(ev && PyInt_Check(ev))) {
    PyErr_Clear();
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
  Py_DECREF(ev);

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 3);
  long      e   = PyInt_AS_LONG(ev);

  if (e >= PyTuple_GET_SIZE(t_o))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_EnumValueOutOfRange, compstatus);

  if (PyTuple_GET_ITEM(t_o, e) != a_o)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  Py_INCREF(a_o); return a_o;
}

static PyObject*
copyArgumentString(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{ // max_length

  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);
  OMNIORB_ASSERT(PyInt_Check(t_o));

  CORBA::ULong max_len = PyInt_AS_LONG(t_o);

  if (!PyString_Check(a_o))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  CORBA::ULong len = PyString_GET_SIZE(a_o);

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_StringIsTooLong, compstatus);

  // Annoyingly, we have to scan the string to check there are no
  // nulls
  char* str = PyString_AS_STRING(a_o);
  for (CORBA::ULong i=0; i<len; i++)
    if (str[i] == '\0')
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_EmbeddedNullInPythonString, compstatus);

  // After all that, we don't actually have to copy the string,
  // since they're immutable
  Py_INCREF(a_o);
  return a_o;
}

static PyObject*
copyArgumentSequence(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus)
{ // element_desc, max_length

  PyObject*    r_o;
  PyObject*    t_o      = PyTuple_GET_ITEM(d_o, 2);
  OMNIORB_ASSERT(PyInt_Check(t_o));
  CORBA::ULong max_len  = PyInt_AS_LONG(t_o);
  PyObject*    elm_desc = PyTuple_GET_ITEM(d_o, 1);

  CORBA::ULong len, i;

  if (sequenceOptimisedType(elm_desc)) { // Simple type
    CORBA::ULong etk = PyInt_AS_LONG(elm_desc);

    if (etk == CORBA::tk_octet || etk == CORBA::tk_char) {
      // Mapping says octet and char use a string
      if (!PyString_Check(a_o))
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

      len = PyString_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      Py_INCREF(a_o);
      return a_o;
    }
    else if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      r_o = PyList_New(len);
      omniPy::PyRefHolder r_o_holder(r_o);

      long             long_val;
      unsigned long    ulong_val;
      double           double_val;
#ifdef HAS_LongLong
      CORBA::LongLong  llong_val;
      CORBA::ULongLong ullong_val;
#endif

      switch (etk) {
      case CORBA::tk_short:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= -0x8000 && long_val <= 0x7fff) {
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange,
			    compstatus);
	    }
	    if (long_val >= -0x8000 && long_val <= 0x7fff) {
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_long:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
#if SIZEOF_LONG > 4
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= -0x80000000L && long_val <= 0x7fffffffL) {
#endif
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
#if SIZEOF_LONG > 4
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (long_val >= -0x80000000L && long_val <= 0x7fffffffL) {
#endif
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
#if SIZEOF_LONG > 4
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ushort:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= 0 && long_val <= 0xffff) {
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    if (long_val >= 0 && long_val <= 0xffff) {
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ulong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ulong_val = PyLong_AsUnsignedLong(t_o);
	    if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (ulong_val > 0xffffffffL) {
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#endif
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
#if SIZEOF_LONG > 4
	    if (long_val >= 0 && long_val <= 0xffffffffL) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
#else
	    if (long_val >= 0) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
#endif
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_float:
      case CORBA::tk_double:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyFloat_Check(t_o)) {
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o);
	  }
	  else if (PyInt_Check(t_o)) {
	    PyList_SET_ITEM(r_o, i,
			    PyFloat_FromDouble((double)
					       PyInt_AS_LONG(t_o)));
	  }
	  else if (PyLong_Check(t_o)) {
	    double_val = PyLong_AsDouble(t_o);
	    if (double_val == -1.0 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    PyList_SET_ITEM(r_o, i, PyFloat_FromDouble(double_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_boolean:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyBool_Check(t_o)) {
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o);
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    PyList_SET_ITEM(r_o, i, PyBool_FromLong(long_val));
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) PyErr_Clear();
	    PyList_SET_ITEM(r_o, i, PyBool_FromLong(long_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    llong_val = PyLong_AsLongLong(t_o);
	    if (llong_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ulonglong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ullong_val = PyLong_AsUnsignedLongLong(t_o);
	    if (ullong_val == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= 0) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;
#else
      case 23:
      case 24:
	{
	  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
	}
#endif
      default:
	OMNIORB_ASSERT(0);
      }
      return r_o_holder.retn();
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      r_o = PyList_New(len);
      omniPy::PyRefHolder r_o_holder(r_o);

      long             long_val;
      unsigned long    ulong_val;
      double           double_val;
#ifdef HAS_LongLong
      CORBA::LongLong  llong_val;
      CORBA::ULongLong ullong_val;
#endif

      switch (etk) {

      case CORBA::tk_short:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= -0x8000 && long_val <= 0x7fff) {
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    if (long_val >= -0x8000 && long_val <= 0x7fff) {
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_long:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
#if SIZEOF_LONG > 4
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= -0x80000000L && long_val <= 0x7fffffffL) {
#endif
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
#if SIZEOF_LONG > 4
	    }
#endif
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (long_val >= -0x80000000L && long_val <= 0x7fffffffL) {
#endif
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
#if SIZEOF_LONG > 4
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ushort:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= 0 && long_val <= 0xffff) {
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    if (long_val >= 0 && long_val <= 0xffff) {
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ulong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ulong_val = PyLong_AsUnsignedLong(t_o);
	    if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (ulong_val > 0xffffffffL) {
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#endif
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
#if SIZEOF_LONG > 4
	    if (long_val >= 0 && long_val <= 0xffffffffL) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
#else
	    if (long_val >= 0) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
#endif
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_float:
      case CORBA::tk_double:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyFloat_Check(t_o)) {
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o);
	  }
	  else if (PyInt_Check(t_o)) {
	    PyList_SET_ITEM(r_o, i,
			    PyFloat_FromDouble((double)
					       PyInt_AS_LONG(t_o)));
	  }
	  else if (PyLong_Check(t_o)) {
	    double_val = PyLong_AsDouble(t_o);
	    if (double_val == -1.0 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    PyList_SET_ITEM(r_o, i, PyFloat_FromDouble(double_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_boolean:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyBool_Check(t_o)) {
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o);
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    PyList_SET_ITEM(r_o, i, PyBool_FromLong(long_val));
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) PyErr_Clear();
	    PyList_SET_ITEM(r_o, i, PyBool_FromLong(long_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    llong_val = PyLong_AsLongLong(t_o);
	    if (llong_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ulonglong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ullong_val = PyLong_AsUnsignedLongLong(t_o);
	    if (ullong_val == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= 0) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;
#else
      case 23:
      case 24:
	{
	  OMNIORB_THROW(NO_IMPLEMENT, 0, compstatus);
	}
#endif
      default:
	OMNIORB_ASSERT(0);
      }
      return r_o_holder.retn();
    }
    else {
      // Not a list or a tuple
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
    }
  }
  else {
    // Complex type

    if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      r_o = PyList_New(len);
      omniPy::PyRefHolder r_o_holder(r_o);

      for (i=0; i < len; i++) {
	t_o = omniPy::copyArgument(elm_desc, PyList_GET_ITEM(a_o, i),
				   compstatus);
	PyList_SET_ITEM(r_o, i, t_o);
      }
      return r_o_holder.retn();
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (max_len > 0 && len > max_len)
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong, compstatus);

      r_o = PyList_New(len);
      omniPy::PyRefHolder r_o_holder(r_o);

      for (i=0; i < len; i++) {
	t_o = omniPy::copyArgument(elm_desc, PyTuple_GET_ITEM(a_o, i),
				   compstatus);
	PyList_SET_ITEM(r_o, i, t_o);
      }
      return r_o_holder.retn();
    }
    else
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
  OMNIORB_ASSERT(0);
  return 0; // For dumb compilers
}

static PyObject*
copyArgumentArray(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{ // element_desc, length

  PyObject*    r_o;
  PyObject*    t_o      = PyTuple_GET_ITEM(d_o, 2);
  OMNIORB_ASSERT(PyInt_Check(t_o));
  CORBA::ULong arr_len  = PyInt_AS_LONG(t_o);
  PyObject*    elm_desc = PyTuple_GET_ITEM(d_o, 1);

  CORBA::ULong len, i;

  if (sequenceOptimisedType(elm_desc)) { // Simple type
    CORBA::ULong etk = PyInt_AS_LONG(elm_desc);

    if (etk == CORBA::tk_octet || etk == CORBA::tk_char) {
      // Mapping says octet and char use a string
      if (!PyString_Check(a_o))
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

      len = PyString_GET_SIZE(a_o);
      if (len != arr_len)
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
      Py_INCREF(a_o);
      return a_o;
    }
    else if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (len != arr_len)
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);

      r_o = PyList_New(len);
      omniPy::PyRefHolder r_o_holder(r_o);

      long             long_val;
      unsigned long    ulong_val;
      double           double_val;
#ifdef HAS_LongLong
      CORBA::LongLong  llong_val;
      CORBA::ULongLong ullong_val;
#endif

      switch (etk) {
      case CORBA::tk_short:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= -0x8000 && long_val <= 0x7fff) {
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    if (long_val >= -0x8000 && long_val <= 0x7fff) {
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_long:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
#if SIZEOF_LONG > 4
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= -0x80000000L && long_val <= 0x7fffffffL) {
#endif
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
#if SIZEOF_LONG > 4
	    }
#endif
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (long_val >= -0x80000000L && long_val <= 0x7fffffffL) {
#endif
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
#if SIZEOF_LONG > 4
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ushort:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= 0 && long_val <= 0xffff) {
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    if (long_val >= 0 && long_val <= 0xffff) {
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ulong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ulong_val = PyLong_AsUnsignedLong(t_o);
	    if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (ulong_val > 0xffffffffL) {
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#endif
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
#if SIZEOF_LONG > 4
	    if (long_val >= 0 && long_val <= 0xffffffffL) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
#else
	    if (long_val >= 0) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
#endif
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_float:
      case CORBA::tk_double:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyFloat_Check(t_o)) {
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o);
	  }
	  else if (PyInt_Check(t_o)) {
	    PyList_SET_ITEM(r_o, i,
			    PyFloat_FromDouble((double)
					       PyInt_AS_LONG(t_o)));
	  }
	  else if (PyLong_Check(t_o)) {
	    double_val = PyLong_AsDouble(t_o);
	    if (double_val == -1.0 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    PyList_SET_ITEM(r_o, i, PyFloat_FromDouble(double_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_boolean:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyBool_Check(t_o)) {
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o);
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    PyList_SET_ITEM(r_o, i, PyBool_FromLong(long_val));
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) PyErr_Clear();
	    PyList_SET_ITEM(r_o, i, PyBool_FromLong(long_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    llong_val = PyLong_AsLongLong(t_o);
	    if (llong_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ulonglong:
	for (i=0; i<len; i++) {
	  t_o = PyList_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ullong_val = PyLong_AsUnsignedLongLong(t_o);
	    if (ullong_val == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= 0) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;
#else
      case 23:
      case 24:
	{
	  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
	}
#endif
      default:
	OMNIORB_ASSERT(0);
      }
      return r_o_holder.retn();
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (len != arr_len)
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);

      r_o = PyList_New(len);
      omniPy::PyRefHolder r_o_holder(r_o);

      long             long_val;
      unsigned long    ulong_val;
      double           double_val;
#ifdef HAS_LongLong
      CORBA::LongLong  llong_val;
      CORBA::ULongLong ullong_val;
#endif

      switch (etk) {

      case CORBA::tk_short:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= -0x8000 && long_val <= 0x7fff) {
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    if (long_val >= -0x8000 && long_val <= 0x7fff) {
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_long:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
#if SIZEOF_LONG > 4
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= -0x80000000L && long_val <= 0x7fffffffL) {
#endif
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
#if SIZEOF_LONG > 4
	    }
#endif
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (long_val >= -0x80000000L && long_val <= 0x7fffffffL) {
#endif
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
#if SIZEOF_LONG > 4
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
#endif
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ushort:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= 0 && long_val <= 0xffff) {
	      Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    if (long_val >= 0 && long_val <= 0xffff) {
	      PyList_SET_ITEM(r_o, i, PyInt_FromLong(long_val)); continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ulong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ulong_val = PyLong_AsUnsignedLong(t_o);
	    if (ulong_val == (unsigned long)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#if SIZEOF_LONG > 4
	    if (ulong_val > 0xffffffffL) {
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
#endif
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
#if SIZEOF_LONG > 4
	    if (long_val >= 0 && long_val <= 0xffffffffL) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
#else
	    if (long_val >= 0) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
#endif
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_float:
      case CORBA::tk_double:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyFloat_Check(t_o)) {
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o);
	  }
	  else if (PyInt_Check(t_o)) {
	    PyList_SET_ITEM(r_o, i,
			    PyFloat_FromDouble((double)
					       PyInt_AS_LONG(t_o)));
	  }
	  else if (PyLong_Check(t_o)) {
	    double_val = PyLong_AsDouble(t_o);
	    if (double_val == -1.0 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    PyList_SET_ITEM(r_o, i, PyFloat_FromDouble(double_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_boolean:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyBool_Check(t_o)) {
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o);
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    PyList_SET_ITEM(r_o, i, PyBool_FromLong(long_val));
	  }
	  else if (PyLong_Check(t_o)) {
	    long_val = PyLong_AsLong(t_o);
	    if (long_val == -1 && PyErr_Occurred()) PyErr_Clear();
	    PyList_SET_ITEM(r_o, i, PyBool_FromLong(long_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

#ifdef HAS_LongLong

      case CORBA::tk_longlong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    llong_val = PyLong_AsLongLong(t_o);
	    if (llong_val == -1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	  }
	  else
	    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;

      case CORBA::tk_ulonglong:
	for (i=0; i<len; i++) {
	  t_o = PyTuple_GET_ITEM(a_o, i);
	  if (PyLong_Check(t_o)) {
	    ullong_val = PyLong_AsUnsignedLongLong(t_o);
	    if (ullong_val == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
	      PyErr_Clear();
	      OMNIORB_THROW(BAD_PARAM,
			    BAD_PARAM_PythonValueOutOfRange, compstatus);
	    }
	    Py_INCREF(t_o); PyList_SET_ITEM(r_o, i, t_o); continue;
	  }
	  else if (PyInt_Check(t_o)) {
	    long_val = PyInt_AS_LONG(t_o);
	    if (long_val >= 0) {
	      PyList_SET_ITEM(r_o, i, PyLong_FromLong(long_val));
	      continue;
	    }
	    OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_PythonValueOutOfRange, compstatus);
	  }
	  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
	}
	break;
#else
      case 23:
      case 24:
	{
	  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
	}
#endif
      default:
	OMNIORB_ASSERT(0);
      }
      return r_o_holder.retn();
    }
    else {
      // Not a list or tuple
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
    }
  }
  else {
    // Complex type

    if (PyList_Check(a_o)) {
      len = PyList_GET_SIZE(a_o);
      if (len != arr_len)
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);

      r_o = PyList_New(len);
      omniPy::PyRefHolder r_o_holder(r_o);

      for (i=0; i < len; i++) {
	t_o = omniPy::copyArgument(elm_desc, PyList_GET_ITEM(a_o, i),
				   compstatus);
	PyList_SET_ITEM(r_o, i, t_o);
      }
      return r_o_holder.retn();
    }
    else if (PyTuple_Check(a_o)) {
      len = PyTuple_GET_SIZE(a_o);
      if (len != arr_len)
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);

      r_o = PyList_New(len);
      omniPy::PyRefHolder r_o_holder(r_o);

      for (i=0; i < len; i++) {
	t_o = omniPy::copyArgument(elm_desc, PyTuple_GET_ITEM(a_o, i),
				   compstatus);
	PyList_SET_ITEM(r_o, i, t_o);
      }
      return r_o_holder.retn();
    }
    else
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }
  OMNIORB_ASSERT(0);
  return 0;
}

static PyObject*
copyArgumentAlias(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{ // repoId, name, descr

  return omniPy::copyArgument(PyTuple_GET_ITEM(d_o, 3), a_o, compstatus);
}

static PyObject*
copyArgumentExcept(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{ // class, repoId, exc name, name, descriptor, ...

  // As with structs, the descriptor tuple has twice the number of
  // members plus 4.
  int cnt = (PyTuple_GET_SIZE(d_o) - 4) / 2;

  PyObject* t_o;
  PyObject* name;
  PyObject* value;
  PyObject* argtuple = PyTuple_New(cnt);
  omniPy::PyRefHolder argtuple_holder(argtuple);

  int i, j;
  for (i=0,j=4; i < cnt; i++) {
    name    = PyTuple_GET_ITEM(d_o, j++);
    OMNIORB_ASSERT(PyString_Check(name));

    value = PyObject_GetAttr(a_o, name);
    if (!value)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

    Py_DECREF(value);

    t_o = omniPy::copyArgument(PyTuple_GET_ITEM(d_o, j++), value, compstatus);

    PyTuple_SET_ITEM(argtuple, i, t_o);
  }
  return PyEval_CallObject(PyTuple_GET_ITEM(d_o, 1), argtuple);
}

static PyObject*
copyArgumentLongLong(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus)
{
#ifdef HAS_LongLong
  if (PyLong_Check(a_o)) {
    CORBA::LongLong ll = PyLong_AsLongLong(a_o);
    if (ll == -1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
    Py_INCREF(a_o); return a_o;
  }
  else if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
    return PyLong_FromLong(l);
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
  return 0;
}

static PyObject*
copyArgumentULongLong(PyObject* d_o, PyObject* a_o,
		      CORBA::CompletionStatus compstatus)
{
#ifdef HAS_LongLong
  if (PyLong_Check(a_o)) {
    CORBA::ULongLong ll = PyLong_AsUnsignedLongLong(a_o);
    if (ll == (CORBA::ULongLong)-1 && PyErr_Occurred()) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    }
    Py_INCREF(a_o); return a_o;
  }
  else if (PyInt_Check(a_o)) {
    long l = PyInt_AS_LONG(a_o);
    if (l < 0)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PythonValueOutOfRange, compstatus);
    return PyLong_FromLong(l);
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
  return 0;
}

static PyObject*
copyArgumentLongDouble(PyObject* d_o, PyObject* a_o,
		       CORBA::CompletionStatus compstatus)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
  return 0;
}

static PyObject*
copyArgumentWChar(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
#ifdef PY_HAS_UNICODE
  if (PyUnicode_Check(a_o) && (PyUnicode_GET_SIZE(a_o) == 1)) {
    Py_INCREF(a_o); return a_o;
  }
  else
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
#endif
  return 0;
}

static PyObject*
copyArgumentWString(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus)
{ // max_length
#ifdef PY_HAS_UNICODE
  PyObject* t_o = PyTuple_GET_ITEM(d_o, 1);
  OMNIORB_ASSERT(PyInt_Check(t_o));

  CORBA::ULong max_len = PyInt_AS_LONG(t_o);

  if (!PyUnicode_Check(a_o))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  CORBA::ULong len = PyUnicode_GET_SIZE(a_o);

  if (max_len > 0 && len > max_len)
    OMNIORB_THROW(MARSHAL, MARSHAL_WStringIsTooLong, compstatus);

  // Annoyingly, we have to scan the string to check there are no
  // nulls
  Py_UNICODE* str = PyUnicode_AS_UNICODE(a_o);
  for (CORBA::ULong i=0; i<len; i++)
    if (str[i] == 0)
      OMNIORB_THROW(BAD_PARAM,
		    BAD_PARAM_EmbeddedNullInPythonString, compstatus);

  // After all that, we don't actually have to copy the string,
  // since they're immutable
  Py_INCREF(a_o);
  return a_o;
#else
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
  return 0;
#endif
}

static PyObject*
copyArgumentFixed(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
  if (!omnipyFixed_Check(a_o))
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

  PyObject* t_o;

  t_o = PyTuple_GET_ITEM(d_o, 1); int dlimit = PyInt_AS_LONG(t_o);
  t_o = PyTuple_GET_ITEM(d_o, 2); int slimit = PyInt_AS_LONG(t_o);

  CORBA::Fixed f(*(((omnipyFixedObject*)a_o)->ob_fixed));
  f.PR_setLimits(dlimit, slimit);
  return omniPy::newFixedObject(f);
}

static PyObject*
copyArgumentNative(PyObject* d_o, PyObject* a_o,
		   CORBA::CompletionStatus compstatus)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);
  return 0;
}

// copyArgumentAbstractInterface is in pyAbstractIntf.cc

static PyObject*
copyArgumentLocalInterface(PyObject* d_o, PyObject* a_o,
			   CORBA::CompletionStatus compstatus)
{
  Py_INCREF(a_o);
  return a_o;
}

PyObject*
omniPy::
copyArgumentIndirect(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus)
{
  PyObject* l = PyTuple_GET_ITEM(d_o, 1); OMNIORB_ASSERT(PyList_Check(l));
  PyObject* d = PyList_GET_ITEM(l, 0);

  if (PyString_Check(d)) {
    // Indirection to a repoId -- find the corresponding descriptor
    d = PyDict_GetItem(pyomniORBtypeMap, d);
    if (!d) OMNIORB_THROW(BAD_PARAM,
			  BAD_PARAM_IncompletePythonType,
			  compstatus);

    Py_INCREF(d);
    PyList_SetItem(l, 0, d);
  }
  return copyArgument(d, a_o, compstatus);
}


const omniPy::CopyArgumentFn omniPy::copyArgumentFns[] = {
  copyArgumentNull,
  copyArgumentVoid,
  copyArgumentShort,
  copyArgumentLong,
  copyArgumentUShort,
  copyArgumentULong,
  copyArgumentFloat,
  copyArgumentDouble,
  copyArgumentBoolean,
  copyArgumentChar,
  copyArgumentOctet,
  copyArgumentAny,
  copyArgumentTypeCode,
  copyArgumentPrincipal,
  copyArgumentObjref,
  copyArgumentStruct,
  copyArgumentUnion,
  copyArgumentEnum,
  copyArgumentString,
  copyArgumentSequence,
  copyArgumentArray,
  copyArgumentAlias,
  copyArgumentExcept,
  copyArgumentLongLong,
  copyArgumentULongLong,
  copyArgumentLongDouble,
  copyArgumentWChar,
  copyArgumentWString,
  copyArgumentFixed,
  omniPy::copyArgumentValue,
  omniPy::copyArgumentValueBox,
  copyArgumentNative,
  omniPy::copyArgumentAbstractInterface,
  copyArgumentLocalInterface
};



//
// PyUnlockingCdrStream
//

void
omniPy::
PyUnlockingCdrStream::put_octet_array(const _CORBA_Octet* b, int size,
				      omni::alignment_t align)
{
  omniPy::InterpreterUnlocker _u;
  cdrStreamAdapter::put_octet_array(b, size, align);
}

void
omniPy::
PyUnlockingCdrStream::get_octet_array(_CORBA_Octet* b,int size,
				      omni::alignment_t align)
{
  omniPy::InterpreterUnlocker _u;
  cdrStreamAdapter::get_octet_array(b, size, align);
}
  
void
omniPy::
PyUnlockingCdrStream::skipInput(_CORBA_ULong size)
{
  omniPy::InterpreterUnlocker _u;
  cdrStreamAdapter::skipInput(size);
}

void
omniPy::
PyUnlockingCdrStream::copy_to(cdrStream& stream, int size,
			      omni::alignment_t align)
{
  omniPy::InterpreterUnlocker _u;
  cdrStreamAdapter::copy_to(stream, size, align);
}

void
omniPy::
PyUnlockingCdrStream::fetchInputData(omni::alignment_t align, size_t required)
{
  omniPy::InterpreterUnlocker _u;
  cdrStreamAdapter::fetchInputData(align, required);
}

_CORBA_Boolean
omniPy::
PyUnlockingCdrStream::
reserveOutputSpaceForPrimitiveType(omni::alignment_t align, size_t required)
{
  omniPy::InterpreterUnlocker _u;
  return cdrStreamAdapter::reserveOutputSpaceForPrimitiveType(align, required);
}

_CORBA_Boolean
omniPy::
PyUnlockingCdrStream::
maybeReserveOutputSpace(omni::alignment_t align, size_t required)
{
  omniPy::InterpreterUnlocker _u;
  return cdrStreamAdapter::maybeReserveOutputSpace(align, required);
}
