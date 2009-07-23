// -*- Mode: C++; -*-
//                            Package   : omniORB
// pyConnectionMgmt.cc        Created on: 2006/07/21
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2006 Apasphere Ltd.
//
//    This file is part of the omniORB library
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
// Description:
//    Proprietary omniORB connection management API

#ifdef __WIN32__
#define DLL_EXPORT _declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#include <omniORB4/CORBA.h>
#include <omniORB4/omniConnectionMgmt.h>

#if defined(__VMS)
#include <Python.h>
#else
#include PYTHON_INCLUDE
#endif

#include <omniORBpy.h>
#include "../omnipy.h"


static omniORBpyAPI* omnipyApi;


extern "C" {

  static char init_doc[] =
  "init()\n"
  "\n"
  "Initialise the omniConnectionMgmt module. Must be called before \n"
  "CORBA.ORB_init().\n";

  static PyObject* pyomniConnectionMgmt_init(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)""))
      return 0;

    omniConnectionMgmt::init();

    Py_INCREF(Py_None);
    return Py_None;
  }

  static char makeRestrictedReference_doc[] =
  "makeRestrictedReference(obj, connection_id, max_connections, max_threads,\n"
  "                        data_batch, permit_interleaved, server_hold_open)\n"
  "\n"
  "Given an object reference, construct a new reference that uses\n"
  "connections unique to references with <connection_id>. The client\n"
  "will open at most <max_connections> network connections to the\n"
  "server; the server will use at most <max_threads> concurrent\n"
  "threads to service each of those connections. If <data_batch> is\n"
  "true, the client will enable data batching on the connection if\n"
  "relevant (e.g. Nagle's algorithm). If <permit_interleaved> is\n"
  "true, multiple concurrent calls can be interleaved on a single\n"
  "connection. If <server_hold_open> is true, the server will keep\n"
  "the connection open until the client closes it.\n";

  static PyObject* pyomniConnectionMgmt_makeRestrictedReference(PyObject* self,
								PyObject* args)
  {
    PyObject* pyobj;
    int       connection_id, max_connections, max_threads;
    int       data_batch, permit_interleaved, server_hold_open;

    if (!PyArg_ParseTuple(args, (char*)"Oiiiiii",
			  &pyobj, &connection_id, &max_connections,
			  &max_threads, &data_batch, &permit_interleaved,
			  &server_hold_open))
      return 0;

    try {
      CORBA::Object_var orig_obj = omnipyApi->pyObjRefToCxxObjRef(pyobj, 1);
      CORBA::Object_var new_obj;

      {
	omniPy::InterpreterUnlocker _u;
	new_obj = omniConnectionMgmt::
	  makeRestrictedReference(orig_obj,
				  (CORBA::ULong)connection_id,
				  (CORBA::ULong)max_connections,
				  (CORBA::ULong)max_threads,
				  (CORBA::Boolean)data_batch,
				  (CORBA::Boolean)permit_interleaved,
				  (CORBA::Boolean)server_hold_open);
      }
      return omnipyApi->cxxObjRefToPyObjRef(new_obj, 1);
    }
#define api omnipyApi
    OMNIORBPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
#undef api
  }

  static PyMethodDef omniConnectionMgmt_methods[] = {
    {(char*)"init",
     pyomniConnectionMgmt_init, METH_VARARGS,
     init_doc},

    {(char*)"makeRestrictedReference",
     pyomniConnectionMgmt_makeRestrictedReference, METH_VARARGS,
     makeRestrictedReference_doc},

    {0,0}
  };

  void DLL_EXPORT init_omniConnMgmt()
  {
    PyObject* m = Py_InitModule((char*)"_omniConnMgmt",
				omniConnectionMgmt_methods);

    // Get hold of the omniORBpy C++ API.
    PyObject* omnipy = PyImport_ImportModule((char*)"_omnipy");
    PyObject* pyapi  = PyObject_GetAttrString(omnipy, (char*)"API");
    omnipyApi        = (omniORBpyAPI*)PyCObject_AsVoidPtr(pyapi);
    Py_DECREF(pyapi);
  }
};

