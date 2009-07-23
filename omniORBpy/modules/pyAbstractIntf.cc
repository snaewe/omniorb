// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyAbstractIntf.cc          Created on: 2003/05/21
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2003-2005 Apasphere Ltd.
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
//    Abstract interface support

// $Log$
// Revision 1.1.2.2  2005/06/24 17:36:08  dgrisby
// Support for receiving valuetypes inside Anys; relax requirement for
// old style classes in a lot of places.
//
// Revision 1.1.2.1  2003/07/10 22:13:25  dgrisby
// Abstract interface support.
//

#include <omnipy.h>

OMNI_USING_NAMESPACE(omni)

void
omniPy::
validateTypeAbstractInterface(PyObject* d_o, PyObject* a_o,
			      CORBA::CompletionStatus compstatus,
			      PyObject* track)
{ // repoId, name

  // Nil?
  if (a_o == Py_None)
    return;

  // Object reference?
  if (omniPy::getTwin(a_o, OBJREF_TWIN))
    return;

  // Value?
  if (omniPy::isInstance(a_o, omniPy::pyCORBAValueBase)) {
    // Does it support the interface?
    PyObject* repoId    = PyTuple_GET_ITEM(d_o, 1);
    PyObject* skelclass = PyDict_GetItem(omniPy::pyomniORBskeletonMap, repoId);

    if (!skelclass)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

    if (!omniPy::isInstance(a_o, skelclass))
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

    // Check the instance matches the valuetype it claims to be.
    repoId = PyObject_GetAttr(a_o, omniPy::pyNP_RepositoryId);
    if (!repoId)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

    PyObject* valuedesc = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);

    Py_DECREF(repoId);

    if (!valuedesc)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

    omniPy::validateTypeValue(valuedesc, a_o, compstatus, track);
    return;
  }
  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
}



void
omniPy::
marshalPyObjectAbstractInterface(cdrStream& stream,
				 PyObject* d_o, PyObject* a_o)
{ // repoId, name

  if (a_o == Py_None) {
    // GIOP spec says treat as a null valuetype
    stream.marshalBoolean(0);
    CORBA::Long tag = 0;
    tag >>= stream;
    return;
  }

  // Object reference?
  CORBA::Object_ptr obj;
  obj = (CORBA::Object_ptr)omniPy::getTwin(a_o, OBJREF_TWIN);
  if (obj) {
    stream.marshalBoolean(1);
    CORBA::Object::_marshalObjRef(obj, stream);
    return;
  }

  // Valuetype
  stream.marshalBoolean(0);
  omniPy::marshalPyObjectValue(stream, omniPy::pyCORBAValueBaseDesc, a_o);
}

PyObject*
omniPy::
unmarshalPyObjectAbstractInterface(cdrStream& stream, PyObject* d_o)
{ // repoId, name

  CORBA::Boolean is_objref = stream.unmarshalBoolean();

  if (is_objref) {
    PyObject* pyrepoId = PyTuple_GET_ITEM(d_o, 1);
    const char* repoId = PyString_AS_STRING(pyrepoId);

    CORBA::Object_ptr obj = omniPy::UnMarshalObjRef(repoId, stream);
    return omniPy::createPyCorbaObjRef(repoId, obj);
  }
  else {
    return omniPy::unmarshalPyObjectValue(stream,omniPy::pyCORBAValueBaseDesc);
  }
}

PyObject*
omniPy::
copyArgumentAbstractInterface(PyObject* d_o, PyObject* a_o,
			      CORBA::CompletionStatus compstatus)
{
  if (a_o == Py_None) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  if (omniPy::getTwin(a_o, OBJREF_TWIN)) {
    return omniPy::copyObjRefArgument(PyTuple_GET_ITEM(d_o, 1),
				      a_o, compstatus);
  }
  if (omniPy::isInstance(a_o, omniPy::pyCORBAValueBase)) {
    // Does it support the interface?
    PyObject* repoId    = PyTuple_GET_ITEM(d_o, 1);
    PyObject* skelclass = PyDict_GetItem(omniPy::pyomniORBskeletonMap, repoId);

    if (!skelclass)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

    if (!omniPy::isInstance(a_o, skelclass))
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

    // Check the instance matches the valuetype it claims to be.
    repoId = PyObject_GetAttr(a_o, omniPy::pyNP_RepositoryId);
    if (!repoId)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

    PyObject* valuedesc = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);

    Py_DECREF(repoId);

    if (!valuedesc)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

    return omniPy::copyArgumentValue(valuedesc, a_o, compstatus);
  }
  OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  return 0;
}
