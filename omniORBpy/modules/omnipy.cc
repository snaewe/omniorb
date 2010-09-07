// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// omnipy.cc                  Created on: 1999/06/01
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2007 Apasphere Ltd
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
//    Main entry points for _omnipy Python module

// $Id$

// $Log$
// Revision 1.1.4.20  2009/05/05 14:26:21  dgrisby
// Remove unused variable.
//
// Revision 1.1.4.19  2007/09/18 20:03:34  dgrisby
// Refcount error if cdrUnmarshal raised a Python exception.
//
// Revision 1.1.4.18  2007/01/19 11:11:09  dgrisby
// Avoid assertion failure if an unexpected C++ exception occurs during
// an invocation.
//
// Revision 1.1.4.17  2006/09/05 11:30:22  dgrisby
// Modify stub version check to be more specific.
//
// Revision 1.1.4.16  2006/09/01 13:44:06  dgrisby
// Hard-code version check rather than using pre-processor defines.
//
// Revision 1.1.4.15  2006/07/26 17:49:59  dgrisby
// Support unchecked_narrow.
//
// Revision 1.1.4.14  2006/05/24 18:33:04  dgrisby
// Unlock interpreter lock before clearing value tracker in cdrMarshal /
// cdrUnmarshal.
//
// Revision 1.1.4.13  2006/05/15 10:26:11  dgrisby
// More relaxation of requirements for old-style classes, for Python 2.5.
//
// Revision 1.1.4.12  2006/01/19 17:28:44  dgrisby
// Merge from omnipy2_develop.
//
// Revision 1.1.4.11  2005/11/09 12:33:32  dgrisby
// Support POA LocalObjects.
//
// Revision 1.1.4.10  2005/06/24 17:36:09  dgrisby
// Support for receiving valuetypes inside Anys; relax requirement for
// old style classes in a lot of places.
//
// Revision 1.1.4.9  2005/04/25 18:27:41  dgrisby
// Maintain forwarded location when narrowing forwarded references.
//
// Revision 1.1.4.8  2005/04/11 13:02:38  dgrisby
// Another merge.
//
// Revision 1.1.4.7  2005/03/02 13:39:17  dgrisby
// Another merge from omnipy2_develop.
//
// Revision 1.1.4.6  2005/01/07 00:22:32  dgrisby
// Big merge from omnipy2_develop.
//
// Revision 1.1.4.5  2004/02/16 10:14:17  dgrisby
// Use stream based copy for local calls.
//
// Revision 1.1.4.4  2003/11/06 12:00:34  dgrisby
// ValueType TypeCode support; track ORB core changes.
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
// Revision 1.1.2.20  2003/03/12 11:17:02  dgrisby
// Registration of external pseudo object creation functions.
//
// Revision 1.1.2.19  2002/05/26 00:55:36  dgrisby
// C++ API to convert object references to/from Python.
//
// Revision 1.1.2.18  2002/02/18 18:48:13  dpg1
// Autoconf support.
//
// Revision 1.1.2.17  2002/01/18 15:49:44  dpg1
// Context support. New system exception construction. Fix None call problem.
//
// Revision 1.1.2.16  2001/12/10 18:10:37  dpg1
// Segfault with narrow on pseudo object.
//
// Revision 1.1.2.15  2001/09/24 10:48:25  dpg1
// Meaningful minor codes.
//
// Revision 1.1.2.14  2001/09/20 14:51:24  dpg1
// Allow ORB reinitialisation after destroy(). Clean up use of omni namespace.
//
// Revision 1.1.2.13  2001/08/21 10:52:40  dpg1
// Update to new ORB core APIs.
//
// Revision 1.1.2.12  2001/08/15 10:37:13  dpg1
// Track ORB core object table changes.
//
// Revision 1.1.2.11  2001/06/22 10:29:26  dpg1
// Add distribution date to logging.
//
// Revision 1.1.2.10  2001/06/11 13:06:25  dpg1
// Support for PortableServer::Current.
//
// Revision 1.1.2.9  2001/06/01 11:09:25  dpg1
// Make use of new omni::ptrStrCmp() and omni::strCmp().
//
// Revision 1.1.2.8  2001/05/14 15:22:00  dpg1
// cdrMarshal() / cdrUnmarshal() are back.
//
// Revision 1.1.2.7  2001/05/14 12:47:20  dpg1
// Fix memory leaks.
//
// Revision 1.1.2.6  2001/05/10 15:16:01  dpg1
// Big update to support new omniORB 4 internals.
//
// Revision 1.1.2.5  2001/04/09 16:28:49  dpg1
// De-uglify ORB_init command line argument eating.
//
// Revision 1.1.2.4  2001/04/09 15:22:15  dpg1
// Fixed point support.
//
// Revision 1.1.2.3  2000/11/22 17:23:03  dpg1
// Twin strings pre-allocated.
//
// Revision 1.1.2.2  2000/11/06 17:06:38  dpg1
// Fix to prevent extra _is_a call after narrow
//
// Revision 1.1.2.1  2000/10/13 13:55:23  dpg1
// Initial support for omniORB 4.
//

#ifdef __WIN32__
#define DLL_EXPORT _declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#include "pydistdate.hh"
#include <omnipy.h>
#include <pyThreadCache.h>
#include <initialiser.h>

OMNI_USING_NAMESPACE(omni)

////////////////////////////////////////////////////////////////////////////
// Global pointers to Python objects                                      //
////////////////////////////////////////////////////////////////////////////

PyInterpreterState* omniPy::pyInterpreter;

PyObject* omniPy::py_omnipymodule;	// The _omnipy extension
PyObject* omniPy::pyCORBAmodule;	// The CORBA module
PyObject* omniPy::pyCORBAsysExcMap;	//  The system exception map
PyObject* omniPy::pyCORBAAnyClass;	//  Any class
PyObject* omniPy::pyCORBATypeCodeClass;	//  TypeCode class
PyObject* omniPy::pyCORBAContextClass;	//  Context class
PyObject* omniPy::pyCORBAValueBase;    	//  ValueBase class
PyObject* omniPy::pyCORBAValueBaseDesc;	//  ValueBase descriptor
PyObject* omniPy::pyomniORBmodule;	// The omniORB module
PyObject* omniPy::pyomniORBobjrefMap;	//  The objref class map
PyObject* omniPy::pyomniORBskeletonMap;	//  The skeleton class map
PyObject* omniPy::pyomniORBtypeMap;     //  The repoId to descriptor mapping
PyObject* omniPy::pyomniORBvalueMap;    //  The repoId to value factory mapping
PyObject* omniPy::pyomniORBwordMap;     //  Reserved word map
PyObject* omniPy::pyomniORBpoaCache;    //  POA cache
PyObject* omniPy::pyomniORBUnknownValueBase;
                                        //  Base class for unknown valuetypes
PyObject* omniPy::pyPortableServerModule;
                                        // Portable server module
PyObject* omniPy::pyServantClass;       // Servant class
PyObject* omniPy::pyCreateTypeCode;	// Function to create a TypeCode object
PyObject* omniPy::pyWorkerThreadClass;  // Worker thread class
PyObject* omniPy::pyWorkerThreadDel;    // Method to delete worker thread
PyObject* omniPy::pyEmptyTuple;         // Zero element tuple.

PyObject* omniPy::pyORB_TWIN;
PyObject* omniPy::pyOBJREF_TWIN;
PyObject* omniPy::pySERVANT_TWIN;
PyObject* omniPy::pyPOA_TWIN;
PyObject* omniPy::pyPOAMANAGER_TWIN;
PyObject* omniPy::pyPOACURRENT_TWIN;
PyObject* omniPy::pyNP_RepositoryId;


////////////////////////////////////////////////////////////////////////////
// Pointer to the ORB                                                     //
////////////////////////////////////////////////////////////////////////////

CORBA::ORB_ptr omniPy::orb;


////////////////////////////////////////////////////////////////////////////
// Constant strings to facilitate comparison by pointer                   //
////////////////////////////////////////////////////////////////////////////

const char* omniPy::string_Py_omniObjRef       = "Py_omniObjRef";
const char* omniPy::string_Py_omniServant      = "Py_omniServant";
const char* omniPy::string_Py_ServantActivator = "Py_ServantActivator";
const char* omniPy::string_Py_ServantLocator   = "Py_ServantLocator";
const char* omniPy::string_Py_AdapterActivator = "Py_AdapterActivator";


////////////////////////////////////////////////////////////////////////////
// Twin type                                                              //
////////////////////////////////////////////////////////////////////////////

extern "C" {

  static void
  omnipyTwin_dealloc(omnipyTwin* tp)
  {
    PyMem_DEL(tp);
  }

  static int
  omnipyTwin_cmp(omnipyTwin* t1, omnipyTwin* t2)
  {
    // Can't use simple t1->ob_twin - t2->ob_twin in case int is
    // smaller than a pointer
    if      (t1->ob_twin == t2->ob_twin) return 0;
    else if (t1->ob_twin >  t2->ob_twin) return 1;
    else                                 return -1;
  }

  static long
  omnipyTwin_hash(omnipyTwin* tp)
  {
    return (long)tp->ob_twin;
  }

  static PyTypeObject omnipyTwinType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    (char*)"omnipyTwin",
    sizeof(omnipyTwin),
    0,
    (destructor)omnipyTwin_dealloc,   /*tp_dealloc*/
    0,			              /*tp_print*/
    0,                                /*tp_getattr*/
    0,				      /*tp_setattr*/
    (cmpfunc)omnipyTwin_cmp,          /*tp_compare*/
    0,                                /*tp_repr*/
    0,				      /*tp_as_number*/
    0,                                /*tp_as_sequence*/
    0,				      /*tp_as_mapping*/
    (hashfunc)omnipyTwin_hash,	      /*tp_hash*/
    0,				      /*tp_call*/
    0,				      /*tp_str*/
    0,				      /*tp_getattro*/
    0,				      /*tp_setattro*/
    0,                                /*tp_as_buffer*/
    0,				      /*tp_flags*/
    0,                                /*tp_doc*/
  };
}

PyObject*
omniPy::newTwin(void* twin)
{
  omnipyTwin* ot = PyMem_NEW(omnipyTwin, 1);
  ot->ob_type = &omnipyTwinType;
  ot->ob_twin = (void*)twin;
  _Py_NewReference((PyObject*)ot);
  return (PyObject*)ot;
}

////////////////////////////////////////////////////////////////////////////
// Module inititaliser to hook orb creation and destruction               //
////////////////////////////////////////////////////////////////////////////

class omni_python_initialiser : public omniInitialiser {
public:
  void attach() {
    omniPy::registerInterceptors();
  }
  void detach() {
    omnipyThreadCache::shutdown();
    if (omniPy::orb) omniPy::orb = 0;
  }
};

static omni_python_initialiser the_omni_python_initialiser;


// Function to generate a list of system exception names
static PyObject*
generateExceptionList()
{
  int i = 0;
# define INCREMENT_COUNT(exc) i++;
  OMNIORB_FOR_EACH_SYS_EXCEPTION(INCREMENT_COUNT)
# undef INCREMENT_COUNT

  PyObject* excs = PyList_New(i);
  i = 0;

# define ADD_EXCEPTION_NAME(exc) \
  PyList_SetItem(excs, i++, PyString_FromString(#exc));
  OMNIORB_FOR_EACH_SYS_EXCEPTION(ADD_EXCEPTION_NAME)
# undef ADD_EXCEPTION_NAME

  return excs;
}



// Things visible to Python:

extern "C" {

  ////////////////////////////////////////////////////////////////////////////
  // omnipy private functions                                               //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject*
  omnipy_checkVersion(PyObject* self, PyObject* args)
  {
    int   maj, min;
    char* mod;

    if (!PyArg_ParseTuple(args, (char*)"iis", &maj, &min, &mod))
      return 0;

    if (maj != 3 || min != 0) {
      if (omniORB::trace(1)) {
	omniORB::logger l;
        l << "\n"
	  << "omniORBpy: WARNING! _omnipy module version "
	  << OMNIPY_MAJOR << "." << OMNIPY_MINOR
          << " expects stubs version 3.0.\n"
	  << "omniORBpy: Stubs in " << mod << " are version "
	  << maj << "." << min << ".\n"
	  << "omniORBpy: You may experience strange errors "
	  << "until you fix the mismatch\n";
      }
    }
    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  omnipy_coreVersion(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)"")) return 0;
    return Py_BuildValue((char*)"s", omniORB::versionString());
  }

#define OMNIPY_ATTR(x) \
  PyObject_GetAttrString(omniPy::pyomniORBmodule, (char*)x);

  static PyObject*
  omnipy_registerPyObjects(PyObject* self, PyObject* args)
  {
    PyObject* temp;

    // Get a pointer to the interpreter state
    PyThreadState* tstate = PyThreadState_Get();
    omniPy::pyInterpreter = tstate->interp;

    if (!PyArg_ParseTuple(args, (char*)"O", &omniPy::pyomniORBmodule))
      return 0;

    OMNIORB_ASSERT(PyModule_Check(omniPy::pyomniORBmodule));

    omniPy::pyCORBAmodule = OMNIPY_ATTR("CORBA");

    OMNIORB_ASSERT(omniPy::pyCORBAmodule &&
		   PyModule_Check(omniPy::pyCORBAmodule));

    omniPy::pyCORBAsysExcMap = OMNIPY_ATTR("sysExceptionMapping");

    omniPy::pyCORBAAnyClass =
      PyObject_GetAttrString(omniPy::pyCORBAmodule, (char*)"Any");

    omniPy::pyCORBATypeCodeClass =
      PyObject_GetAttrString(omniPy::pyCORBAmodule, (char*)"TypeCode");

    omniPy::pyCORBAContextClass =
      PyObject_GetAttrString(omniPy::pyCORBAmodule, (char*)"Context");

    omniPy::pyCORBAValueBase =
      PyObject_GetAttrString(omniPy::pyCORBAmodule, (char*)"ValueBase");

    omniPy::pyCORBAValueBaseDesc =
      PyObject_GetAttrString(omniPy::pyCORBAmodule, (char*)"_d_ValueBase");

    omniPy::pyomniORBobjrefMap        = OMNIPY_ATTR("objrefMapping");
    omniPy::pyomniORBtypeMap          = OMNIPY_ATTR("typeMapping");
    omniPy::pyomniORBwordMap          = OMNIPY_ATTR("keywordMapping");
    omniPy::pyomniORBpoaCache         = OMNIPY_ATTR("poaCache");
    omniPy::pyPortableServerModule    = OMNIPY_ATTR("PortableServer");
    omniPy::pyomniORBskeletonMap      = OMNIPY_ATTR("skeletonMapping");
    omniPy::pyomniORBvalueMap         = OMNIPY_ATTR("valueFactoryMapping");
    omniPy::pyomniORBUnknownValueBase = OMNIPY_ATTR("UnknownValueBase");

    OMNIORB_ASSERT(omniPy::pyPortableServerModule);
    OMNIORB_ASSERT(PyModule_Check(omniPy::pyPortableServerModule));

    omniPy::pyServantClass =
      PyObject_GetAttrString(omniPy::pyPortableServerModule, (char*)"Servant");

    temp = OMNIPY_ATTR("tcInternal");

    omniPy::pyCreateTypeCode = PyObject_GetAttrString(temp,
						      (char*)"createTypeCode");

    omniPy::pyWorkerThreadClass = OMNIPY_ATTR("WorkerThread");

    omniPy::pyWorkerThreadDel =
      PyObject_GetAttrString(omniPy::pyWorkerThreadClass, (char*)"delete");

    omniPy::pyEmptyTuple = OMNIPY_ATTR("_emptyTuple");

    OMNIORB_ASSERT(omniPy::pyCORBAsysExcMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyCORBAsysExcMap));
    OMNIORB_ASSERT(omniPy::pyCORBAAnyClass);
    OMNIORB_ASSERT(omniPy::pyCORBATypeCodeClass);
    OMNIORB_ASSERT(omniPy::pyCORBAContextClass);
    OMNIORB_ASSERT(omniPy::pyCORBAValueBaseDesc);
    OMNIORB_ASSERT(PyTuple_Check(omniPy::pyCORBAValueBaseDesc));
    OMNIORB_ASSERT(omniPy::pyCORBAValueBase);
    OMNIORB_ASSERT(omniPy::pyomniORBobjrefMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyomniORBobjrefMap));
    OMNIORB_ASSERT(omniPy::pyomniORBskeletonMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyomniORBskeletonMap));
    OMNIORB_ASSERT(omniPy::pyomniORBtypeMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyomniORBtypeMap));
    OMNIORB_ASSERT(omniPy::pyomniORBvalueMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyomniORBvalueMap));
    OMNIORB_ASSERT(omniPy::pyomniORBwordMap);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyomniORBwordMap));
    OMNIORB_ASSERT(omniPy::pyomniORBUnknownValueBase);
    OMNIORB_ASSERT(omniPy::pyomniORBpoaCache);
    OMNIORB_ASSERT(PyDict_Check(omniPy::pyomniORBpoaCache));
    OMNIORB_ASSERT(omniPy::pyServantClass);
    OMNIORB_ASSERT(omniPy::pyCreateTypeCode);
    OMNIORB_ASSERT(PyFunction_Check(omniPy::pyCreateTypeCode));
    OMNIORB_ASSERT(omniPy::pyWorkerThreadClass);
    OMNIORB_ASSERT(omniPy::pyWorkerThreadDel);
    OMNIORB_ASSERT(PyMethod_Check(omniPy::pyWorkerThreadDel));
    OMNIORB_ASSERT(omniPy::pyEmptyTuple);
    OMNIORB_ASSERT(PyTuple_Check(omniPy::pyEmptyTuple));

    omniPy::pyORB_TWIN        = OMNIPY_ATTR("_ORB_TWIN");
    omniPy::pyOBJREF_TWIN     = OMNIPY_ATTR("_OBJREF_TWIN");
    omniPy::pySERVANT_TWIN    = OMNIPY_ATTR("_SERVANT_TWIN");
    omniPy::pyPOA_TWIN        = OMNIPY_ATTR("_POA_TWIN");
    omniPy::pyPOAMANAGER_TWIN = OMNIPY_ATTR("_POAMANAGER_TWIN");
    omniPy::pyPOACURRENT_TWIN = OMNIPY_ATTR("_POACURRENT_TWIN");
    omniPy::pyNP_RepositoryId = OMNIPY_ATTR("_NP_RepositoryId");

    OMNIORB_ASSERT(omniPy::pyORB_TWIN);
    OMNIORB_ASSERT(omniPy::pyOBJREF_TWIN);
    OMNIORB_ASSERT(omniPy::pySERVANT_TWIN);
    OMNIORB_ASSERT(omniPy::pyPOA_TWIN);
    OMNIORB_ASSERT(omniPy::pyPOAMANAGER_TWIN);
    OMNIORB_ASSERT(omniPy::pyPOACURRENT_TWIN);

    OMNIORB_ASSERT(PyString_Check(omniPy::pyORB_TWIN));
    OMNIORB_ASSERT(PyString_Check(omniPy::pyOBJREF_TWIN));
    OMNIORB_ASSERT(PyString_Check(omniPy::pySERVANT_TWIN));
    OMNIORB_ASSERT(PyString_Check(omniPy::pyPOA_TWIN));
    OMNIORB_ASSERT(PyString_Check(omniPy::pyPOAMANAGER_TWIN));
    OMNIORB_ASSERT(PyString_Check(omniPy::pyPOACURRENT_TWIN));

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  omnipy_need_ORB_init(PyObject* self, PyObject* args)
  {
    if (!PyArg_ParseTuple(args, (char*)""))
      return 0;

    if (omniPy::orb)
      return PyInt_FromLong(0);
    else
      return PyInt_FromLong(1);
  }

  ////////////////////////////////////////////////////////////////////////////
  // CORBA:: functions                                                      //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject*
  omnipy_ORB_init(PyObject* self, PyObject* args)
  {
    PyObject* pyorb;
    PyObject* pyargv;
    char*     orbid;
    int       argc;
    char**    argv;

    OMNIORB_ASSERT(omniPy::orb == 0);

    if (!PyArg_ParseTuple(args, (char*)"OOs", &pyorb, &pyargv, &orbid))
      return 0;

    if (!PyList_Check(pyargv)) {
      PyErr_SetString(PyExc_TypeError,
		      "argument 2: parameter must be an argument list");
      return 0;
    }

    argc = PyList_GET_SIZE(pyargv);
    argv = new char*[argc];

    PyObject* o;
    int i;
    for (i=0; i<argc; i++) {
      o = PyList_GET_ITEM(pyargv, i);
      if (!PyString_Check(o)) {
	PyErr_SetString(PyExc_TypeError,
			"argument 2: parameter must be a list of strings.");
	delete[] argv;
	return 0;
      }
      argv[i] = PyString_AS_STRING(o);
    }

    int orig_argc = argc;

    CORBA::ORB_ptr orb;
    try {
      orb = CORBA::ORB_init(argc, argv, orbid);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    if (omniORB::trace(2)) {
      omniORB::logger l;
      l << "omniORBpy distribution date: " OMNIORBPY_DIST_DATE "\n";
    }

    omniPy::orb = orb;

    // Remove eaten arguments from Python argv list
    if (argc < orig_argc) {
      int r;
      char *s, *t;
      for (i=0; i<argc; ++i) {
	s = argv[i];

	while (1) {
	  o = PyList_GetItem(pyargv, i); OMNIORB_ASSERT(o != 0);
	  t = PyString_AS_STRING(o);
	  if (s == t) break;
	  r = PySequence_DelItem(pyargv, i);
	  OMNIORB_ASSERT(r != -1);
	}
      }
      while (PyList_Size(pyargv) > argc) {
	// Delete -ORB arguments at end
	r = PySequence_DelItem(pyargv, i);
	OMNIORB_ASSERT(r != -1);
      }
    }
    delete [] argv;

    omniPy::setTwin(pyorb, orb, ORB_TWIN);

    // Initialise the thread state cache
    omnipyThreadCache::init();

    Py_INCREF(Py_None);
    return Py_None;
  }

  ////////////////////////////////////////////////////////////////////////////
  // CDR stream marshalling/unmarshalling                                   //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject*
  omnipy_cdrMarshal(PyObject* self, PyObject* args)
  {
    PyObject *desc, *data;
    int endian = -1;

    if (!PyArg_ParseTuple(args, (char*)"OO|i", &desc, &data, &endian))
      return 0;

    if (endian > 1 || endian < -1) {
      PyErr_SetString(PyExc_ValueError,
		      "argument 3: endian must be 0 or 1");
      return 0;
    }

    try {
      omniPy::validateType(desc, data, CORBA::COMPLETED_NO);

      if (endian == -1) {
	// Marshal into an encapsulation
	cdrEncapsulationStream stream;
        omniPy::ValueTrackerClearer vtc(stream);

	omniPy::marshalPyObject(stream, desc, data);

	return PyString_FromStringAndSize((char*)stream.bufPtr(),
					  stream.bufSize());
      }
      else {
	// Marshal into a raw buffer
	cdrMemoryStream stream;
        omniPy::ValueTrackerClearer vtc(stream);

	if (endian != omni::myByteOrder)
	  stream.setByteSwapFlag(endian);

	omniPy::marshalPyObject(stream, desc, data);

	return PyString_FromStringAndSize((char*)stream.bufPtr(),
					  stream.bufSize());
      }
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static inline PyObject* do_cdrUnmarshal(cdrStream& stream, PyObject* desc)
  {
    PyObject* r = omniPy::unmarshalPyObject(stream, desc);
    if (r && stream.checkInputOverrun(1, 1)) {
      // More data in stream -- must have used the wrong TypeCode
      Py_DECREF(r);
      OMNIORB_THROW(MARSHAL, MARSHAL_MessageTooLong, CORBA::COMPLETED_NO);
    }
    return r;
  }

  static PyObject*
  omnipy_cdrUnmarshal(PyObject* self, PyObject* args)
  {
    PyObject* desc;
    char*     encap;
    size_t    size;
    int       endian = -1;
    
    if (!PyArg_ParseTuple(args, (char*)"Os#|i",
			  &desc, &encap, &size, &endian))
      return 0;

    if (endian > 1 || endian < -1) {
      PyErr_SetString(PyExc_ValueError,
		      "argument 3: endian must be 0 or 1");
      return 0;
    }

    try {
      if (endian == -1) {
	// Encapsulation
	cdrEncapsulationStream stream((CORBA::Octet*)encap, size);
        omniPy::ValueTrackerClearer vtc(stream);
	return do_cdrUnmarshal(stream, desc);
      }
      else {
	// Simple buffer. Is it aligned ok?
	if ((omni::ptr_arith_t)encap ==
	    omni::align_to((omni::ptr_arith_t)encap, omni::ALIGN_8)) {

	  cdrMemoryStream stream((CORBA::Octet*)encap, size);
          omniPy::ValueTrackerClearer vtc(stream);

	  if (endian != omni::myByteOrder)
	    stream.setByteSwapFlag(endian);

	  return do_cdrUnmarshal(stream, desc);
	}
	else {
	  // Unfortunately, this is a common case, due to the way
	  // Python string objects are laid out.
	  cdrMemoryStream stream;
          omniPy::ValueTrackerClearer vtc(stream);

	  if (endian != omni::myByteOrder)
	    stream.setByteSwapFlag(endian);

	  stream.put_octet_array((CORBA::Octet*)encap, size);
	  return do_cdrUnmarshal(stream, desc);
	}
      }
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }


  ////////////////////////////////////////////////////////////////////////////
  // Functions operating on object references                               //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject*
  omnipy_invoke(PyObject* self, PyObject* args)
  {
    // Arg format
    //  (objref, op_name, (in_desc,out_desc,exc_desc [, ctxt [,values]]), args)
    //
    //  exc_desc is a dictionary containing a mapping from repoIds to
    //  tuples of the form (exception class, marshal desc., param count)

    PyObject *pyobjref, *in_d, *out_d, *exc_d, *ctxt_d, *op_args;
    char*  op;
    size_t op_len;

    PyObject* o;
    PyObject* desc;

    pyobjref = PyTuple_GET_ITEM(args,0);

    o        = PyTuple_GET_ITEM(args,1);
    op       = PyString_AS_STRING(o);
    op_len   = PyString_GET_SIZE(o);

    desc     = PyTuple_GET_ITEM(args,2);

    in_d     = PyTuple_GET_ITEM(desc,0);
    out_d    = PyTuple_GET_ITEM(desc,1);
    exc_d    = PyTuple_GET_ITEM(desc,2);

    int desclen = PyTuple_GET_SIZE(desc);

    if (desclen >= 4) {
      ctxt_d = PyTuple_GET_ITEM(desc,3);
      if (ctxt_d == Py_None)
	ctxt_d = 0;
    }
    else
      ctxt_d = 0;

    CORBA::Boolean contains_values = 0;

    if (desclen == 5) {
      PyObject* v = PyTuple_GET_ITEM(desc,4);
      if (v != Py_None)
	contains_values = 1;
    }

    op_args  = PyTuple_GET_ITEM(args,3);

    int arg_len = PyTuple_GET_SIZE(in_d) + (ctxt_d ? 1:0);

    if (PyTuple_GET_SIZE(op_args) != arg_len) {
      char* err = new char[80];
      sprintf(err, "Operation requires %d argument%s; %d given",
	      arg_len, (arg_len == 1) ? "" : "s",
	      (int)PyTuple_GET_SIZE(op_args));

      PyErr_SetString(PyExc_TypeError, err);
      delete [] err;
      return 0;
    }

    CORBA::Object_ptr cxxobjref =
      (CORBA::Object_ptr)omniPy::getTwin(pyobjref, OBJREF_TWIN);

    omniObjRef*    oobjref   = cxxobjref->_PR_getobj();
    CORBA::Boolean is_oneway = (out_d == Py_None);

    omniPy::Py_omniCallDescriptor call_desc(op, op_len + 1, is_oneway,
					    in_d, out_d, exc_d, ctxt_d,
					    op_args, 0);

    if (contains_values)
      call_desc.containsValues(1);

    try {
      call_desc.releaseInterpreterLock();
      oobjref->_invoke(call_desc);
      call_desc.reacquireInterpreterLock();
      if (!is_oneway) {
	return call_desc.result();
      }
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }
#ifdef HAS_Cplusplus_catch_exception_by_base
    catch (const CORBA::SystemException& ex) {
      // systemException() reacquires the interpreter lock if necessary
      call_desc.systemException(ex);
    }
#else
#define DO_CALL_DESC_SYSTEM_EXCEPTON(exc) \
    catch (const CORBA::exc& ex) { \
      call_desc.systemException(ex); \
    }
OMNIORB_FOR_EACH_SYS_EXCEPTION(DO_CALL_DESC_SYSTEM_EXCEPTON)
#undef DO_CALL_DESC_SYSTEM_EXCEPTON
#endif
    catch (omniPy::PyUserException& ex) {
      call_desc.reacquireInterpreterLock();
      ex.setPyExceptionState();
    }
    catch (...) {
      // This should not happen, but in case it does we reacquire the
      // interpreter lock to avoid an assertion failure from the call
      // descriptor's destructor.
      omniORB::logs(1, "Unexpected C++ exception during Python invocation.");
      call_desc.ensureInterpreterLock();
      throw;
    }
    return 0;
  }

  static PyObject*
  omnipy_releaseObjref(PyObject* self, PyObject* args)
  {
    PyObject* pyobjref;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyobjref))
      return 0;

    CORBA::Object_ptr cxxobjref =
      (CORBA::Object_ptr)omniPy::getTwin(pyobjref, OBJREF_TWIN);

    if (cxxobjref) {
      {
	omniPy::InterpreterUnlocker _u;
	CORBA::release(cxxobjref);
      }
      omniPy::remTwin(pyobjref, OBJREF_TWIN);
    }
    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  omnipy_isA(PyObject* self, PyObject* args)
  {
    PyObject* pyobjref;
    char*     repoId;

    if (!PyArg_ParseTuple(args, (char*)"Os", &pyobjref, &repoId))
      return 0;

    CORBA::Object_ptr cxxobjref =
      (CORBA::Object_ptr)omniPy::getTwin(pyobjref, OBJREF_TWIN);

    RAISE_PY_BAD_PARAM_IF(!cxxobjref, BAD_PARAM_WrongPythonType);

    try {
      omniPy::InterpreterUnlocker ul;
      CORBA::Boolean isa = cxxobjref->_is_a(repoId);
      return PyInt_FromLong(isa);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  omnipy_nonExistent(PyObject* self, PyObject* args)
  {
    PyObject* pyobjref;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyobjref))
      return 0;

    CORBA::Object_ptr cxxobjref =
      (CORBA::Object_ptr)omniPy::getTwin(pyobjref, OBJREF_TWIN);

    RAISE_PY_BAD_PARAM_IF(!cxxobjref, BAD_PARAM_WrongPythonType);

    try {
      omniPy::InterpreterUnlocker ul;
      CORBA::Boolean nex = cxxobjref->_non_existent();
      return PyInt_FromLong(nex);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  omnipy_isEquivalent(PyObject* self, PyObject* args)
  {
    PyObject* pyobjref1;
    PyObject* pyobjref2;

    if (!PyArg_ParseTuple(args, (char*)"OO", &pyobjref1, &pyobjref2))
      return 0;

    CORBA::Object_ptr cxxobjref1, cxxobjref2;
    cxxobjref1 = (CORBA::Object_ptr)omniPy::getTwin(pyobjref1, OBJREF_TWIN);
    cxxobjref2 = (CORBA::Object_ptr)omniPy::getTwin(pyobjref2, OBJREF_TWIN);

    RAISE_PY_BAD_PARAM_IF(!cxxobjref1 || !cxxobjref2,
			  BAD_PARAM_WrongPythonType);

    try {
      omniPy::InterpreterUnlocker ul;
      CORBA::Boolean ise = cxxobjref1->_is_equivalent(cxxobjref2);
      return PyInt_FromLong(ise);
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  }

  static PyObject*
  omnipy_hash(PyObject* self, PyObject* args)
  {
    PyObject*    pyobjref;
    CORBA::ULong max;

    if (!PyArg_ParseTuple(args, (char*)"Oi", &pyobjref, &max))
      return 0;

    CORBA::Object_ptr cxxobjref =
      (CORBA::Object_ptr)omniPy::getTwin(pyobjref, OBJREF_TWIN);

    RAISE_PY_BAD_PARAM_IF(!cxxobjref, BAD_PARAM_WrongPythonType);

    CORBA::ULong h = cxxobjref->_hash(max);
    return PyInt_FromLong(h);
  }


  static PyObject*
  omnipy_narrow(PyObject* self, PyObject* args)
  {
    PyObject* pysource;
    char*     repoId;
    int       checked;

    if (!PyArg_ParseTuple(args, (char*)"Osi", &pysource, &repoId, &checked))
      return 0;

    CORBA::Object_ptr cxxsource =
      (CORBA::Object_ptr)omniPy::getTwin(pysource, OBJREF_TWIN);

    RAISE_PY_BAD_PARAM_IF(!cxxsource, BAD_PARAM_WrongPythonType);

    CORBA::Boolean    isa;
    CORBA::Object_ptr cxxdest = 0;

    try {
      omniPy::InterpreterUnlocker ul;

      if (checked || cxxsource->_NP_is_pseudo())
	isa = cxxsource->_is_a(repoId);
      else
	isa = 1;

      if (isa) {
	if (!cxxsource->_NP_is_pseudo()) {
	  omniObjRef* oosource = cxxsource->_PR_getobj();
	  omniObjRef* oodest;
	  {
	    omni_tracedmutex_lock sync(*omni::internalLock);
	    oodest = omniPy::createObjRef(repoId, oosource->_getIOR(), 1,
					  oosource->_identity(), 1,
					  oosource->_isForwardLocation());
	  }
	  cxxdest = (CORBA::Object_ptr)
	                   (oodest->_ptrToObjRef(CORBA::Object::_PD_repoId));
	}
	else
	  cxxdest = CORBA::Object::_duplicate(cxxsource);
      }
    }
    OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS

    if (isa) {
      return omniPy::createPyCorbaObjRef(repoId, cxxdest);
    }
    else {
      Py_INCREF(Py_None);
      return Py_None;
    }
  }

  static PyObject*
  omnipy_ensureInit(PyObject* self, PyObject* args)
  {
    PyObject* m = PyImport_ImportModule((char*)"_omnipy");
    PyObject* o = PyObject_GetAttrString(m, (char*)"orb_func");
    PyObject* f = 0;
    
    if (o && PyModule_Check(o))
      f = PyObject_GetAttrString(o, (char*)"destroy");

    if (!o || !PyModule_Check(o) || !f || f == Py_None) {
      omniORB::logs(5, "Reinitialise omniORBpy sub-modules.");
      PyObject* d = PyModule_GetDict(m);
      omniPy::initORBFunc(d);
      omniPy::initPOAFunc(d);
      omniPy::initPOAManagerFunc(d);
      omniPy::initPOACurrentFunc(d);
      omniPy::initInterceptorFunc(d);
      omniPy::initomniFunc(d);
    }
    Py_XDECREF(o);
    Py_XDECREF(f);

    Py_INCREF(Py_None);
    return Py_None;
  }


  ////////////////////////////////////////////////////////////////////////////
  // Python method table                                                    //
  ////////////////////////////////////////////////////////////////////////////

  static PyMethodDef omnipy_methods[] = {

    // omnipy specific things:
    {(char*)"checkVersion",      omnipy_checkVersion,            METH_VARARGS},
    {(char*)"coreVersion",       omnipy_coreVersion,             METH_VARARGS},
    {(char*)"registerPyObjects", omnipy_registerPyObjects,       METH_VARARGS},
    {(char*)"cdrMarshal",        omnipy_cdrMarshal,              METH_VARARGS},
    {(char*)"cdrUnmarshal",      omnipy_cdrUnmarshal,            METH_VARARGS},
    {(char*)"need_ORB_init",     omnipy_need_ORB_init,           METH_VARARGS},
    {(char*)"ensureInit",        omnipy_ensureInit,              METH_VARARGS},

    // Wrappers for functions in CORBA::
    {(char*)"ORB_init",          omnipy_ORB_init,                METH_VARARGS},

    // Functions for CORBA objects:
    {(char*)"invoke",            omnipy_invoke,                  METH_VARARGS},
    {(char*)"releaseObjref",     omnipy_releaseObjref,           METH_VARARGS},
    {(char*)"isA",               omnipy_isA,                     METH_VARARGS},
    {(char*)"nonExistent",       omnipy_nonExistent,             METH_VARARGS},
    {(char*)"isEquivalent",      omnipy_isEquivalent,            METH_VARARGS},
    {(char*)"hash",              omnipy_hash,                    METH_VARARGS},
    {(char*)"narrow",            omnipy_narrow,                  METH_VARARGS},
    {0,0}
  };

  void DLL_EXPORT init_omnipy()
  {
    // Make sure Python is running multi-threaded
    PyEval_InitThreads();

    PyObject* m = Py_InitModule((char*)"_omnipy", omnipy_methods);
    PyObject* d = PyModule_GetDict(m);

    PyDict_SetItemString(d, (char*)"__version__",
			 PyString_FromString(OMNIPY_VERSION_STRING));

    PyDict_SetItemString(d, (char*)"omnipyTwinType",
			 (PyObject*)&omnipyTwinType);

    PyObject* excs = generateExceptionList();
    PyDict_SetItemString(d, (char*)"system_exceptions", excs);
    Py_DECREF(excs);

    omniPy::py_omnipymodule = m;
    omniPy::initORBFunc(d);
    omniPy::initPOAFunc(d);
    omniPy::initPOAManagerFunc(d);
    omniPy::initPOACurrentFunc(d);
    omniPy::initInterceptorFunc(d);
    omniPy::initomniFunc(d);

    // Set up the C++ API singleton
    PyObject* api = PyCObject_FromVoidPtr((void*)&omniPy::cxxAPI, 0);
    PyDict_SetItemString(d, (char*)"API", api);
    Py_DECREF(api);

    // Create an empty list for external modules to register
    // additional pseudo object creation functions.
    PyObject* pseudolist = PyList_New(0);
    PyDict_SetItemString(d, (char*)"pseudoFns", pseudolist);
    Py_DECREF(pseudolist);

    omniInitialiser::install(&the_omni_python_initialiser);
  }
}
