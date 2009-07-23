// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// omnipy.h                   Created on: 2000/02/24
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
//    Master header file for omnipy internals

#ifndef _omnipy_h_
#define _omnipy_h_

// $Log$
// Revision 1.3.2.14  2008/10/09 15:04:36  dgrisby
// Python exceptions occurring during unmarshalling were not properly
// handled. Exception state left set when at traceLevel 0 (thanks
// Morarenko Kirill).
//
// Revision 1.3.2.13  2007/01/19 11:11:09  dgrisby
// Avoid assertion failure if an unexpected C++ exception occurs during
// an invocation.
//
// Revision 1.3.2.12  2006/07/26 17:50:43  dgrisby
// Reuse existing omniIOR object when converting C++ object reference to Python.
//
// Revision 1.3.2.11  2006/07/19 09:40:39  dgrisby
// Track ORB core changes.
//
// Revision 1.3.2.10  2006/05/24 18:33:04  dgrisby
// Unlock interpreter lock before clearing value tracker in cdrMarshal /
// cdrUnmarshal.
//
// Revision 1.3.2.9  2006/05/15 10:26:11  dgrisby
// More relaxation of requirements for old-style classes, for Python 2.5.
//
// Revision 1.3.2.8  2006/01/19 17:28:44  dgrisby
// Merge from omnipy2_develop.
//
// Revision 1.3.2.7  2005/11/09 12:33:32  dgrisby
// Support POA LocalObjects.
//
// Revision 1.3.2.6  2005/06/24 17:36:08  dgrisby
// Support for receiving valuetypes inside Anys; relax requirement for
// old style classes in a lot of places.
//
// Revision 1.3.2.5  2005/04/25 18:27:41  dgrisby
// Maintain forwarded location when narrowing forwarded references.
//
// Revision 1.3.2.4  2005/01/07 00:22:32  dgrisby
// Big merge from omnipy2_develop.
//
// Revision 1.3.2.3  2003/07/10 22:13:25  dgrisby
// Abstract interface support.
//
// Revision 1.3.2.2  2003/05/20 17:10:23  dgrisby
// Preliminary valuetype support.
//
// Revision 1.3.2.1  2003/03/23 21:51:57  dgrisby
// New omnipy3_develop branch.
//
// Revision 1.2.4.20  2003/03/12 11:17:02  dgrisby
// Registration of external pseudo object creation functions.
//
// Revision 1.2.4.19  2002/11/27 00:18:25  dgrisby
// Per thread / per objref timeouts.
//
// Revision 1.2.4.18  2002/05/26 00:55:36  dgrisby
// C++ API to convert object references to/from Python.
//
// Revision 1.2.4.17  2002/03/18 12:40:38  dpg1
// Support overriding _non_existent.
//
// Revision 1.2.4.16  2002/01/18 15:49:44  dpg1
// Context support. New system exception construction. Fix None call problem.
//
// Revision 1.2.4.15  2001/10/18 15:48:39  dpg1
// Track ORB core changes.
//
// Revision 1.2.4.14  2001/09/24 10:48:25  dpg1
// Meaningful minor codes.
//
// Revision 1.2.4.13  2001/08/21 10:52:41  dpg1
// Update to new ORB core APIs.
//
// Revision 1.2.4.12  2001/08/15 10:37:14  dpg1
// Track ORB core object table changes.
//
// Revision 1.2.4.11  2001/06/11 13:06:26  dpg1
// Support for PortableServer::Current.
//
// Revision 1.2.4.10  2001/06/01 11:09:26  dpg1
// Make use of new omni::ptrStrCmp() and omni::strCmp().
//
// Revision 1.2.4.9  2001/05/29 17:10:14  dpg1
// Support for in process identity.
//
// Revision 1.2.4.8  2001/05/14 12:47:21  dpg1
// Fix memory leaks.
//
// Revision 1.2.4.7  2001/05/10 15:16:01  dpg1
// Big update to support new omniORB 4 internals.
//
// Revision 1.2.4.6  2001/04/09 15:22:15  dpg1
// Fixed point support.
//
// Revision 1.2.4.5  2001/03/13 10:38:07  dpg1
// Fixes from omnipy1_develop
//
// Revision 1.2.4.4  2000/12/04 18:57:23  dpg1
// Fix deadlock when trying to lock omniORB internal lock while holding
// the Python interpreter lock.
//

#if defined(__VMS)
#include <Python.h>
#else
#include PYTHON_INCLUDE
#endif

#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/minorCode.h>
#include <exceptiondefs.h>
#include <objectTable.h>
#include <orbParameters.h>
#include <omniORBpy.h>
#include "omnipy_sysdep.h"


OMNI_USING_NAMESPACE(omni)

////////////////////////////////////////////////////////////////////////////
// Data structure to manage C++ twins of Python objects                   //
////////////////////////////////////////////////////////////////////////////

extern "C" {
  struct omnipyTwin {
    PyObject_HEAD
    void* ob_twin;
  };
}

// Twin attribute names
#define ORB_TWIN        omniPy::pyORB_TWIN
#define OBJREF_TWIN     omniPy::pyOBJREF_TWIN
#define SERVANT_TWIN    omniPy::pySERVANT_TWIN
#define POA_TWIN        omniPy::pyPOA_TWIN
#define POAMANAGER_TWIN omniPy::pyPOAMANAGER_TWIN
#define POACURRENT_TWIN omniPy::pyPOACURRENT_TWIN


// Useful macro
#define RAISE_PY_BAD_PARAM_IF(x,minor) \
  if (x) { \
    CORBA::BAD_PARAM _ex(minor, CORBA::COMPLETED_NO); \
    return omniPy::handleSystemException(_ex); \
  }

class omniPy {
public:

  ////////////////////////////////////////////////////////////////////////////
  // The global Python interpreter state                                    //
  ////////////////////////////////////////////////////////////////////////////

  static PyInterpreterState* pyInterpreter;

  ////////////////////////////////////////////////////////////////////////////
  // Global pointers to Python objects                                      //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject* py_omnipymodule;    	// _omnipy module
  static PyObject* pyCORBAmodule;      	// CORBA module
  static PyObject* pyCORBAsysExcMap;   	//  The system exception map
  static PyObject* pyCORBAAnyClass;    	//  Any class
  static PyObject* pyCORBATypeCodeClass;//  TypeCode class
  static PyObject* pyCORBAContextClass;	//  Context class
  static PyObject* pyCORBAValueBase;    //  ValueBase class
  static PyObject* pyCORBAValueBaseDesc;//  Descriptor for ValueBase
  static PyObject* pyomniORBmodule;    	// The omniORB module
  static PyObject* pyomniORBobjrefMap; 	//  The objref class map
  static PyObject* pyomniORBskeletonMap;//  The skeleton class map
  static PyObject* pyomniORBtypeMap;   	//  Type map
  static PyObject* pyomniORBvalueMap;  	//  Value factory map
  static PyObject* pyomniORBwordMap;   	//  Reserved word map
  static PyObject* pyomniORBpoaCache;   //  POA cache
  static PyObject* pyomniORBUnknownValueBase;
                                        //  Base class for unknown valuetypes
  static PyObject* pyPortableServerModule;
                                        // Portable server module
  static PyObject* pyServantClass;     	// Servant class
  static PyObject* pyCreateTypeCode;   	// Function to create a TypeCode object
  static PyObject* pyWorkerThreadClass;	// Worker thread class
  static PyObject* pyWorkerThreadDel;  	// Method to delete worker thread
  static PyObject* pyEmptyTuple;       	// Zero element tuple

  ////////////////////////////////////////////////////////////////////////////
  // Twin names and other 'static' strings                                  //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject* pyORB_TWIN;
  static PyObject* pyOBJREF_TWIN;
  static PyObject* pySERVANT_TWIN;
  static PyObject* pyPOA_TWIN;
  static PyObject* pyPOAMANAGER_TWIN;
  static PyObject* pyPOACURRENT_TWIN;
  static PyObject* pyNP_RepositoryId;

  ////////////////////////////////////////////////////////////////////////////
  // Constant strings to facilitate comparison by pointer                   //
  ////////////////////////////////////////////////////////////////////////////

  static const char* string_Py_omniObjRef;
  static const char* string_Py_omniServant;
  static const char* string_Py_ServantActivator;
  static const char* string_Py_ServantLocator;
  static const char* string_Py_AdapterActivator;


  ////////////////////////////////////////////////////////////////////////////
  // Pointer to the ORB                                                     //
  ////////////////////////////////////////////////////////////////////////////

  static CORBA::ORB_ptr orb;


  ////////////////////////////////////////////////////////////////////////////
  // C++ API object                                                         //
  ////////////////////////////////////////////////////////////////////////////

  static omniORBpyAPI cxxAPI;


  ////////////////////////////////////////////////////////////////////////////
  // Twin object handling                                                   //
  ////////////////////////////////////////////////////////////////////////////

  static PyObject* newTwin(void* twin);

  static
  inline void
  setTwin(PyObject* obj, void* twin, PyObject* name)
  {
    PyObject* ot = newTwin(twin);
    PyObject_SetAttr(obj, name, ot);
    Py_DECREF(ot);
  }

  static
  inline void
  setExistingTwin(PyObject* obj, PyObject* ot, PyObject* name)
  {
    PyObject_SetAttr(obj, name, ot);
    Py_DECREF(ot);
  }

  static
  inline void*
  getTwin(PyObject* obj, PyObject* name)
  {
    void* twin;
    PyObject* ot = PyObject_GetAttr(obj, name);
    if (ot) {
      twin = ((omnipyTwin*)ot)->ob_twin;
      Py_DECREF(ot);
    }
    else {
      PyErr_Clear();
      twin = 0;
    }
    return twin;
  }

  static
  inline void
  remTwin(PyObject* obj, PyObject* name)
  {
    PyObject_DelAttr(obj, name);
  }

  ////////////////////////////////////////////////////////////////////////////
  // Module initialisation functions                                        //
  ////////////////////////////////////////////////////////////////////////////

  static void initORBFunc        (PyObject* d);
  static void initPOAFunc        (PyObject* d);
  static void initPOAManagerFunc (PyObject* d);
  static void initPOACurrentFunc (PyObject* d);
  static void initInterceptorFunc(PyObject* d);
  static void initomniFunc       (PyObject* d);


  ////////////////////////////////////////////////////////////////////////////
  // Utility functions                                                      //
  ////////////////////////////////////////////////////////////////////////////

  // Set the Python execution state to handle a system exception.
  // Returns a NULL PyObject so you can say
  //   return handleSystemException(ex).
  static
  PyObject* handleSystemException(const CORBA::SystemException& ex);

  // Create a new Python object for the given system exception
  static
  PyObject* createPySystemException(const CORBA::SystemException& ex);

  // Throw a C++ system exception equivalent to the given Python exception
  static
  void produceSystemException(PyObject* eobj, PyObject* erepoId,
			      PyObject* etype, PyObject* etraceback);

  // Handle the current Python exception. An exception must have
  // occurred. Deals with system exceptions and
  // omniORB.LocationForward; all other exceptions print a traceback
  // and raise UNKNOWN.
  static
  void handlePythonException();

  // Handle the omniORB.LocationForward exception in the argument.
  static
  void handleLocationForward(PyObject* evalue);
  
  // Ensure there is an omni_thread associated with the calling thread.
  static
  omni_thread* ensureOmniThread();

  // IsInstance function for all Python versions.
  static inline
  CORBA::Boolean isInstance(PyObject* o, PyObject* c)
  {
#if PY_VERSION_HEX >= 0x02010000
    return PyObject_IsInstance(o,c);
#else
    if (!PyInstance_Check(a_o)) return 0;
    PyObject* acls = (PyObject*)((PyInstanceObject*)o)->in_class;
    return PyClass_IsSubclass(acls, c);
#endif
  }

  // IsSubclass function for all Python versions
  static inline
  CORBA::Boolean isSubclass(PyObject* o, PyObject* c)
  {
#if PY_VERSION_HEX >= 0x02010000
    return PyObject_IsSubclass(o,c);
#else
    return PyClass_IsSubclass(o, c);
#endif
  }

  ////////////////////////////////////////////////////////////////////////////
  // Fixed point                                                            //
  ////////////////////////////////////////////////////////////////////////////

  // Create a new omnipyFixedObject.
  static
  PyObject* newFixedObject(const CORBA::Fixed& f);

  // Version for CORBA.fixed() function
  static
  PyObject* newFixedObject(PyObject* self, PyObject* args);


  ////////////////////////////////////////////////////////////////////////////
  // Python object creation functions                                       //
  ////////////////////////////////////////////////////////////////////////////

  static
  PyObject* createPyPOAObject(const PortableServer::POA_ptr poa);

  static
  PyObject* createPyPOAManagerObject(const PortableServer::POAManager_ptr pm);

  static
  PyObject* createPyPOACurrentObject(const PortableServer::Current_ptr pc);


  ////////////////////////////////////////////////////////////////////////////
  // Object reference functions                                             //
  ////////////////////////////////////////////////////////////////////////////

  // Create the Python object relating to a CORBA object reference
  //
  // Caller must hold the Python interpreter lock.
  static
  PyObject* createPyCorbaObjRef(const char* targetRepoId,
				const CORBA::Object_ptr objref);

  static
  PyObject* createPyPseudoObjRef(const CORBA::Object_ptr objref);


  // Functions which mirror omni::createObjRef(). These versions don't
  // look for C++ proxy factories, and spot local Python servants.
  //
  // Caller must NOT hold the Python interpreter lock.
  static
  omniObjRef* createObjRef(const char*        targetRepoId,
			   omniIOR*           ior,
			   CORBA::Boolean     locked,
			   omniIdentity*      id = 0,
			   CORBA::Boolean     type_verified = 0,
			   CORBA::Boolean     is_forwarded = 0);

  static
  omniObjRef* createLocalObjRef(const char*         mostDerivedRepoId,
				const char*         targetRepoId,
				omniObjTableEntry*  entry,
				omniObjRef*         orig_ref,
				CORBA::Boolean      type_verified = 0);

  static
  omniObjRef* createLocalObjRef(const char* 	    mostDerivedRepoId,
				const char* 	    targetRepoId,
				const _CORBA_Octet* key,
				int                 keysize,
				omniObjRef*         orig_ref,
				CORBA::Boolean      type_verified = 0);

  // When a POA creates a reference to a Python servant, it does not
  // have a proxy object factory for it, so it creates an
  // omniAnonObjRef. This function converts one of them into a
  // Py_omniObjRef with a reference to the local servant.
  //
  // Caller must NOT hold the Python interpreter lock.
  static
  CORBA::Object_ptr makeLocalObjRef(const char* targetRepoId,
				    const CORBA::Object_ptr objref);

  // Copy a Python object reference in an argument or return value.
  // Compares the type of the objref with the target type, and creates
  // a new objref of the target type if they are not compatible.
  // Throws BAD_PARAM if the Python object is not an object reference.
  //
  // Caller must hold the Python interpreter lock.
  static
  PyObject* copyObjRefArgument(PyObject*               pytargetRepoId,
			       PyObject*               pyobjref,
			       CORBA::CompletionStatus compstatus);

  // Mirror of omniURI::stringToObject(). Caller must hold the Python
  // interpreter lock.
  static
  CORBA::Object_ptr stringToObject(const char* uri);

  // Mirror of CORBA::UnMarshalObjRef(). Caller must hold the Python
  // interpreter lock.
  static
  CORBA::Object_ptr UnMarshalObjRef(const char* repoId, cdrStream& s);



  ////////////////////////////////////////////////////////////////////////////
  // Recursive marshalling functions                                        //
  ////////////////////////////////////////////////////////////////////////////
  
  // Helper function to return the TypeCode kind of a descriptor
  static inline
  CORBA::ULong descriptorToTK(PyObject* d_o)
  {
    if (PyInt_Check(d_o))
      return PyInt_AS_LONG(d_o);
    else
      return PyInt_AS_LONG(PyTuple_GET_ITEM(d_o, 0));
  }

  // Validate that the argument has the type specified by the
  // descriptor. If it has not, throw CORBA::BAD_PARAM with the given
  // completion status.
  //
  typedef void (*ValidateTypeFn)(PyObject*, PyObject*,
				 CORBA::CompletionStatus,
				 PyObject*);

  static const ValidateTypeFn validateTypeFns[];

  static void validateTypeIndirect(PyObject* d_o, PyObject* a_o,
				   CORBA::CompletionStatus compstatus,
				   PyObject* track);

  static inline
  void validateType(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus,
		    PyObject* track = 0)
  {
    CORBA::ULong tk = descriptorToTK(d_o);

    if (tk <= 33) { // tk_local_interface
      validateTypeFns[tk](d_o, a_o, compstatus, track);
    }
    else if (tk == 0xffffffff) { // Indirection
      validateTypeIndirect(d_o, a_o, compstatus, track);
    }
    else OMNIORB_THROW(BAD_TYPECODE, BAD_TYPECODE_UnknownKind, compstatus);
  }

  // Marshal the given argument object a_o, which has the type
  // specified by d_o. This function MUST NOT be called without having
  // first called validateType() with the same arguments, since it
  // performs no argument type checking.
  //
  typedef void (*MarshalPyObjectFn)(cdrStream& stream, PyObject*, PyObject*);

  static const MarshalPyObjectFn marshalPyObjectFns[];

  static void marshalPyObjectIndirect(cdrStream& stream,
				      PyObject* d_o, PyObject* a_o);

  static inline
  void marshalPyObject(cdrStream& stream, PyObject* d_o, PyObject* a_o)
  {
    CORBA::ULong tk = descriptorToTK(d_o);

    if (tk <= 33) { // tk_local_interface
      marshalPyObjectFns[tk](stream, d_o, a_o);
    }
    else if (tk == 0xffffffff) { // Indirection
      marshalPyObjectIndirect(stream, d_o, a_o);
    }
    else OMNIORB_ASSERT(0);
  }

  // Unmarshal a PyObject, which has the type specified by d_o.
  //
  typedef PyObject* (*UnmarshalPyObjectFn)(cdrStream& stream, PyObject*);

  static const UnmarshalPyObjectFn unmarshalPyObjectFns[];

  static PyObject* unmarshalPyObjectIndirect(cdrStream& stream, PyObject* d_o);

  static inline
  PyObject* unmarshalPyObject(cdrStream& stream, PyObject* d_o)
  {
    CORBA::ULong tk = descriptorToTK(d_o);

    if (tk <= 33) { // tk_local_interface
      PyObject* r = unmarshalPyObjectFns[tk](stream, d_o);
      if (!r) handlePythonException();
      return r;
    }
    else if (tk == 0xffffffff) { // Indirection
      return unmarshalPyObjectIndirect(stream, d_o);
    }
    else OMNIORB_THROW(BAD_TYPECODE,
		       BAD_TYPECODE_UnknownKind,
		       (CORBA::CompletionStatus)stream.completion());
    return 0;
  }

  // Take a descriptor and an argument object, and return a "copy" of
  // the argument. Immutable types need not be copied. If the argument
  // does not match the descriptor, throws BAD_PARAM.
  //
  typedef PyObject* (*CopyArgumentFn)(PyObject*, PyObject*,
				      CORBA::CompletionStatus);

  static const CopyArgumentFn copyArgumentFns[];

  static PyObject* copyArgumentIndirect(PyObject* d_o, PyObject* a_o,
					CORBA::CompletionStatus compstatus);

  static inline
  PyObject* copyArgument(PyObject* d_o, PyObject* a_o,
			 CORBA::CompletionStatus compstatus)
  {
    CORBA::ULong tk = descriptorToTK(d_o);

    if (tk <= 33) { // tk_local_interface
      PyObject* r = copyArgumentFns[tk](d_o, a_o, compstatus);
      if (!r) handlePythonException();
      return r;
    }
    else if (tk == 0xffffffff) { // Indirection
      return copyArgumentIndirect(d_o, a_o, compstatus);
    }
    else OMNIORB_THROW(BAD_TYPECODE, BAD_TYPECODE_UnknownKind, compstatus);
    return 0; // For dumb compilers
  }

  static inline
  void marshalRawPyString(cdrStream& stream, PyObject* pystring)
  {
    CORBA::ULong slen = PyString_GET_SIZE(pystring) + 1;
    slen >>= stream;
    char* str = PyString_AS_STRING(pystring);
    stream.put_octet_array((const CORBA::Octet*)((const char*)str), slen);
  }

  static inline PyObject*
  unmarshalRawPyString(cdrStream& stream)
  {
    CORBA::ULong len; len <<= stream;

    if (!stream.checkInputOverrun(1, len))
      OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		    (CORBA::CompletionStatus)stream.completion());

    PyObject* pystring = PyString_FromStringAndSize(0, len - 1);

    stream.get_octet_array((_CORBA_Octet*)PyString_AS_STRING(pystring), len);
    return pystring;
  }


  ////////////////////////////////////////////////////////////////////////////
  // Valuetype / abstract interface marshalling functions                   //
  ////////////////////////////////////////////////////////////////////////////

  static void
  validateTypeValue(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus,
		    PyObject* track);

  static void
  validateTypeValueBox(PyObject* d_o, PyObject* a_o,
		       CORBA::CompletionStatus compstatus,
		       PyObject* track);

  static void
  validateTypeAbstractInterface(PyObject* d_o, PyObject* a_o,
				CORBA::CompletionStatus compstatus,
				PyObject* track);

  static void
  marshalPyObjectValue(cdrStream& stream, PyObject* d_o, PyObject* a_o);

  static void
  marshalPyObjectValueBox(cdrStream& stream, PyObject* d_o, PyObject* a_o);

  static void
  marshalPyObjectAbstractInterface(cdrStream& stream,
				   PyObject* d_o, PyObject* a_o);

  static PyObject*
  unmarshalPyObjectValue(cdrStream& stream, PyObject* d_o);
  // Shared by Value and ValueBox

  static PyObject*
  unmarshalPyObjectAbstractInterface(cdrStream& stream, PyObject* d_o);

  static PyObject*
  copyArgumentValue(PyObject* d_o, PyObject* a_o,
		    CORBA::CompletionStatus compstatus);

  static PyObject*
  copyArgumentValueBox(PyObject* d_o, PyObject* a_o,
		       CORBA::CompletionStatus compstatus);

  static PyObject*
  copyArgumentAbstractInterface(PyObject* d_o, PyObject* a_o,
				CORBA::CompletionStatus compstatus);


  ////////////////////////////////////////////////////////////////////////////
  // TypeCode and Any support functions                                     //
  ////////////////////////////////////////////////////////////////////////////

  // Marshal a type descriptor as a TypeCode:
  static
  void marshalTypeCode(cdrStream& stream, PyObject* d_o);

  // Unmarshal a TypeCode, returning a descriptor:
  static
  PyObject* unmarshalTypeCode(cdrStream& stream);

  ////////////////////////////////////////////////////////////////////////////
  // Context support functions                                              //
  ////////////////////////////////////////////////////////////////////////////

  // Validate a Context object.
  static
  void validateContext(PyObject* c_o, CORBA::CompletionStatus compstatus);

  // Marshal context c_o, filtered according to pattern list p_o.
  static
  void marshalContext(cdrStream& stream, PyObject* p_o, PyObject* c_o);

  // Unmarshal context. Trust the sender to correctly filter.
  static
  PyObject* unmarshalContext(cdrStream& stream);

  // Filter context c_o according to pattern list p_o. Returns a new Context.
  static
  PyObject* filterContext(PyObject* p_o, PyObject* c_o);


  ////////////////////////////////////////////////////////////////////////////
  // Interceptor functions                                                  //
  ////////////////////////////////////////////////////////////////////////////

  // Register ORB interceptors if need be
  static
  void registerInterceptors();


  ////////////////////////////////////////////////////////////////////////////
  // Proxy call descriptor object                                           //
  ////////////////////////////////////////////////////////////////////////////

  static
  void Py_localCallBackFunction(omniCallDescriptor* cd, omniServant* svnt);

  class Py_omniCallDescriptor : public omniCallDescriptor {
  public:

    inline Py_omniCallDescriptor(const char* op, int op_len,
				 CORBA::Boolean oneway,
				 PyObject* in_d, PyObject* out_d,
				 PyObject* exc_d, PyObject* ctxt_d,
				 PyObject* args, CORBA::Boolean is_upcall)

      : omniCallDescriptor(Py_localCallBackFunction, op, op_len,
			   oneway, 0, 0, is_upcall),
      in_d_(in_d),
      out_d_(out_d),
      exc_d_(exc_d),
      ctxt_d_(ctxt_d),
      args_(args),
      result_(0),
      in_marshal_(0)
    {
      OMNIORB_ASSERT(PyTuple_Check(in_d));
      tstate_ = 0;
      in_l_   = PyTuple_GET_SIZE(in_d_);
      if (oneway) {
	OMNIORB_ASSERT(out_d_ == Py_None);
	out_l_ = -1;
      }
      else {
	OMNIORB_ASSERT(PyTuple_Check(out_d));
	out_l_ = PyTuple_GET_SIZE(out_d_);
      }
      if (args_) {
	OMNIORB_ASSERT(!is_upcall);
	Py_INCREF(args_);
      }
    }

    virtual ~Py_omniCallDescriptor();

    inline void releaseInterpreterLock() {
      OMNIORB_ASSERT(!tstate_);
      tstate_ = PyEval_SaveThread();
    }

    inline void reacquireInterpreterLock() {
      OMNIORB_ASSERT(tstate_);
      PyEval_RestoreThread(tstate_);
      tstate_ = 0;
    }

    inline void ensureInterpreterLock() {
      if (tstate_) {
        PyEval_RestoreThread(tstate_);
        tstate_ = 0;
      }
    }

    inline PyObject* args() { return args_; }

    // Extract and take ownership of stored results
    inline PyObject* result() { PyObject* r = result_; result_ = 0; return r; }

    //
    // Client side methods

    virtual void initialiseCall(cdrStream&);
    virtual void marshalArguments(cdrStream& stream);
    virtual void unmarshalReturnedValues(cdrStream& stream);
    virtual void userException(cdrStream& stream, _OMNI_NS(IOP_C)* iop_client,
			       const char* repoId);

    inline void systemException(const CORBA::SystemException& ex) {
      if (tstate_) {
	PyEval_RestoreThread(tstate_);
	tstate_ = 0;
      }
      handleSystemException(ex);
    }

    //
    // Server side methods

    virtual void unmarshalArguments(cdrStream& stream);

    // Throws BAD_PARAM if result is bad. _Always_ consumes result.
    void         setAndValidateReturnedValues(PyObject* result);

    // Simply set the returned values
    void         setReturnedValues(PyObject* result) { result_ = result; }

    // Marshal the returned values, and release the stored result
    virtual void marshalReturnedValues(cdrStream& stream);

  public:
    PyObject*      in_d_;
    int            in_l_;
    PyObject*      out_d_;
    int            out_l_;
    PyObject*      exc_d_;
    PyObject*      ctxt_d_;

  private:
    PyObject*      args_;
    PyObject*      result_;

    PyThreadState* tstate_;
    CORBA::Boolean in_marshal_;

    Py_omniCallDescriptor(const Py_omniCallDescriptor&);
    Py_omniCallDescriptor& operator=(const Py_omniCallDescriptor&);
  };


  ////////////////////////////////////////////////////////////////////////////
  // Servant object                                                         //
  ////////////////////////////////////////////////////////////////////////////

  class Py_omniServant : public virtual PortableServer::ServantBase {

  public:

    Py_omniServant(PyObject* pyservant, PyObject* opdict, const char* repoId);

    virtual ~Py_omniServant();

    virtual CORBA::Boolean _dispatch(omniCallHandle& handle);

    void remote_dispatch(Py_omniCallDescriptor* pycd);
    void local_dispatch (Py_omniCallDescriptor* pycd);

    PyObject* py_this();

    virtual void*                   _ptrToInterface(const char* repoId);
    virtual const char*             _mostDerivedRepoId();
    virtual CORBA::Boolean          _is_a(const char* logical_type_id);
    virtual PortableServer::POA_ptr _default_POA();
    virtual CORBA::Boolean          _non_existent();

    inline PyObject* pyServant() { Py_INCREF(pyservant_); return pyservant_; }

    // _add_ref and _remove_ref lock the Python interpreter lock
    // _locked versions assume the interpreter lock is already locked
    virtual void                    _add_ref();
    virtual void                    _remove_ref();
    void                            _locked_add_ref();
    void                            _locked_remove_ref();

  private:
    PyObject* pyservant_;	// Python servant object
    PyObject* opdict_;		// Operation descriptor dictionary
    PyObject* pyskeleton_;	// Skeleton class object
    char*     repoId_;
    int       refcount_;

    // Not implemented:
    Py_omniServant(const Py_omniServant&);
    Py_omniServant& operator=(const Py_omniServant&);
  };

  // Function to find or create a Py_omniServant object for a Python
  // servant object. If the Python object is not an instance of a
  // class derived from PortableServer.Servant, returns 0.
  //
  // Caller must hold the Python interpreter lock.
  static Py_omniServant* getServantForPyObject(PyObject* pyservant);


  ////////////////////////////////////////////////////////////////////////////
  // ServantManager / AdapterActivator implementations                      //
  ////////////////////////////////////////////////////////////////////////////

  class Py_ServantActivator
  {
  public:
    Py_ServantActivator(PyObject* pysa) : pysa_(pysa) { Py_INCREF(pysa_); }
    ~Py_ServantActivator() { Py_DECREF(pysa_); }

    PortableServer::Servant incarnate(const PortableServer::ObjectId& oid,
				      PortableServer::POA_ptr         poa);

    void etherealize(const PortableServer::ObjectId& oid,
		     PortableServer::POA_ptr         poa,
		     PortableServer::Servant         serv,
		     CORBA::Boolean                  cleanup_in_progress,
		     CORBA::Boolean                  remaining_activations);

    inline PyObject* pyobj() { return pysa_; }

  private:
    PyObject* pysa_;

    // Not implemented
    Py_ServantActivator(const Py_ServantActivator&);
    Py_ServantActivator& operator=(const Py_ServantActivator&);
  };

  class Py_ServantLocator
  {
  public:
    Py_ServantLocator(PyObject* pysl) : pysl_(pysl) { Py_INCREF(pysl_); }
    ~Py_ServantLocator() { Py_DECREF(pysl_); }

    PortableServer::Servant preinvoke(const PortableServer::ObjectId& oid,
				      PortableServer::POA_ptr poa,
				      const char*             operation,
				      void*&                  cookie);

    void postinvoke(const PortableServer::ObjectId& oid,
		    PortableServer::POA_ptr         poa,
		    const char*                     operation,
		    void*                           cookie,
		    PortableServer::Servant         serv);

    inline PyObject* pyobj() { return pysl_; }

  private:
    PyObject* pysl_;

    // Not implemented
    Py_ServantLocator(const Py_ServantLocator&);
    Py_ServantLocator& operator=(const Py_ServantLocator&);
  };

  class Py_AdapterActivator
  {
  public:
    Py_AdapterActivator(PyObject* pyaa) : pyaa_(pyaa) { Py_INCREF(pyaa_); }
    ~Py_AdapterActivator() { Py_DECREF(pyaa_); }

    CORBA::Boolean unknown_adapter(PortableServer::POA_ptr parent,
				   const char*             name);

    inline PyObject* pyobj() { return pyaa_; }

  private:
    PyObject* pyaa_;

    // Not implemented
    Py_AdapterActivator(const Py_AdapterActivator&);
    Py_AdapterActivator& operator=(const Py_AdapterActivator&);
  };

  // Function to create a C++ local object for a Python object. If the
  // Python object is not an instance of a mapped local object,
  // returns 0.
  //
  // Caller must hold the Python interpreter lock.
  static CORBA::LocalObject_ptr getLocalObjectForPyObject(PyObject* pyobj);

  // Convert a LocalObject to the underlying Python object. If the
  // object is not a suitable Python LocalObject, throw INV_OBJREF.
  //
  // Caller must hold the Python interpreter lock.
  static PyObject* getPyObjectForLocalObject(CORBA::LocalObject_ptr lobj);


  ////////////////////////////////////////////////////////////////////////////
  // PyUserException is a special CORBA::UserException                      //
  ////////////////////////////////////////////////////////////////////////////

  class PyUserException : public CORBA::UserException {
  public:
    // Constructor used during unmarshalling
    PyUserException(PyObject* desc);

    // Constructor used during marshalling. Throws BAD_PARAM with the
    // given completion status if the exception object doesn't match
    // the descriptor.
    // Always consumes reference to exc.
    PyUserException(PyObject* desc, PyObject* exc,
		    CORBA::CompletionStatus comp_status);

    // Copy constructor
    PyUserException(const PyUserException& e);

    virtual ~PyUserException();

    // Set the Python exception state to the contents of this exception.
    // Caller must hold the Python interpreter lock.
    // Returns 0 so callers can do "return ex.setPyExceptionState()".
    PyObject* setPyExceptionState();

    // DECREF the contained Python exception object. Caller must huld
    // the Python interpreter lock.
    void decrefPyException();

    // Marshalling operators for exception body, not including
    // repository id:

    // Caller must not hold interpreter lock
    void operator>>=(cdrStream& stream) const;

    // Caller must hold interpreter lock
    void operator<<=(cdrStream& stream);

    // Inherited virtual functions
    virtual void              _raise() const;
    virtual const char*       _NP_repoId(int* size)          const;
    virtual void              _NP_marshal(cdrStream& stream) const;
    virtual CORBA::Exception* _NP_duplicate()         	     const;
    virtual const char*       _NP_typeId()            	     const;

  private:
    PyObject* 	   desc_;          // Descriptor tuple
    PyObject* 	   exc_;           // The exception object
    CORBA::Boolean decref_on_del_; // True if exc_ should be DECREF'd when
				   // this object is deleted.
  };


  ////////////////////////////////////////////////////////////////////////////
  // InterpreterUnlocker releases the Python interpreter lock               //
  ////////////////////////////////////////////////////////////////////////////

  class InterpreterUnlocker {
  public:
    inline InterpreterUnlocker() {
      tstate_ = PyEval_SaveThread();
    }
    inline ~InterpreterUnlocker() {
      PyEval_RestoreThread(tstate_);
    }
    inline void lock() {
      PyEval_RestoreThread(tstate_);
    }
    inline void unlock() {
      tstate_ = PyEval_SaveThread();
    }
  private:
    PyThreadState* tstate_;
  };

  ////////////////////////////////////////////////////////////////////////////
  // ValueTrackerClearer safely clears a ValueTracker                       //
  ////////////////////////////////////////////////////////////////////////////

  class ValueTrackerClearer {
  public:
    inline ValueTrackerClearer(cdrStream& s) : s_(s) {}
    inline ~ValueTrackerClearer() {
      if (s_.valueTracker()) {
        InterpreterUnlocker u;
        s_.clearValueTracker();
      }
    };
  private:
    cdrStream& s_;
  };


  ////////////////////////////////////////////////////////////////////////////
  // PyUnlockingCdrStream unlocks the interpreter lock around blocking calls//
  ////////////////////////////////////////////////////////////////////////////

  class PyUnlockingCdrStream : public cdrStreamAdapter {
  public:
    PyUnlockingCdrStream(cdrStream& stream)
      : cdrStreamAdapter(stream)
    {
    }

    ~PyUnlockingCdrStream() { }

    // Override virtual functions in cdrStreamAdapter
    void put_octet_array(const _CORBA_Octet* b, int size,
			 omni::alignment_t align=omni::ALIGN_1);
    void get_octet_array(_CORBA_Octet* b,int size,
			 omni::alignment_t align=omni::ALIGN_1);
    void skipInput(_CORBA_ULong size);

    void copy_to(cdrStream&, int size, omni::alignment_t align=omni::ALIGN_1);

    void fetchInputData(omni::alignment_t,size_t);
    _CORBA_Boolean reserveOutputSpaceForPrimitiveType(omni::alignment_t align,
						      size_t required);
    _CORBA_Boolean maybeReserveOutputSpace(omni::alignment_t align,
					   size_t required);
  };

  ////////////////////////////////////////////////////////////////////////////
  // PyRefHolder holds a references to a Python object                      //
  ////////////////////////////////////////////////////////////////////////////

  class PyRefHolder {
  public:
    inline PyRefHolder(PyObject* obj) : obj_(obj) {}

    inline ~PyRefHolder() {
      Py_XDECREF(obj_);
    }
    inline PyObject* retn() {
      PyObject* r = obj_; obj_ = 0; return r;
    }
    inline PyObject* change(PyObject* o) {
      Py_XDECREF(obj_); obj_ = o; return o;
    }
  private:
    PyObject* obj_;
  };

};

#ifdef HAS_Cplusplus_catch_exception_by_base

#define OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS \
catch (const CORBA::SystemException& ex) { \
  return omniPy::handleSystemException(ex); \
}
#else

#define OMNIPY_CATCH_AND_HANDLE_SPECIFIED_EXCEPTION(exc) \
catch (const CORBA::exc& ex) { \
  return omniPy::handleSystemException(ex); \
}
#define OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS \
  OMNIORB_FOR_EACH_SYS_EXCEPTION(OMNIPY_CATCH_AND_HANDLE_SPECIFIED_EXCEPTION)

#endif


#endif // _omnipy_h_
