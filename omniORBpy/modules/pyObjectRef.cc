// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyObjectRef.cc             Created on: 1999/07/29
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
//    Versions of ORB object ref functions which deal with Python
//    objects, rather than C++ objects

// $Id$
// $Log$
// Revision 1.1.4.8  2008/10/09 15:04:36  dgrisby
// Python exceptions occurring during unmarshalling were not properly
// handled. Exception state left set when at traceLevel 0 (thanks
// Morarenko Kirill).
//
// Revision 1.1.4.7  2006/07/26 17:50:43  dgrisby
// Reuse existing omniIOR object when converting C++ object reference to Python.
//
// Revision 1.1.4.6  2006/07/19 09:40:39  dgrisby
// Track ORB core changes.
//
// Revision 1.1.4.5  2006/05/15 10:26:11  dgrisby
// More relaxation of requirements for old-style classes, for Python 2.5.
//
// Revision 1.1.4.4  2005/06/24 17:36:01  dgrisby
// Support for receiving valuetypes inside Anys; relax requirement for
// old style classes in a lot of places.
//
// Revision 1.1.4.3  2005/04/25 18:27:41  dgrisby
// Maintain forwarded location when narrowing forwarded references.
//
// Revision 1.1.4.2  2005/01/07 00:22:32  dgrisby
// Big merge from omnipy2_develop.
//
// Revision 1.1.2.21  2004/04/05 09:06:42  dgrisby
// Bidirectional servers didn't work.
//
// Revision 1.1.2.20  2004/03/02 15:33:57  dgrisby
// Support persistent server id.
//
// Revision 1.1.2.19  2003/07/28 15:44:21  dgrisby
// Unlock interpreter during string_to_object.
//
// Revision 1.1.2.18  2003/03/14 15:28:43  dgrisby
// Use Python 1.5.2 sequence length function.
//
// Revision 1.1.2.17  2003/03/12 11:17:03  dgrisby
// Registration of external pseudo object creation functions.
//
// Revision 1.1.2.16  2002/08/02 13:33:49  dgrisby
// C++ API didn't allow ORB to be passed from C++ to Python, and required
// Python to have imported omniORB.
//
// Revision 1.1.2.15  2001/10/18 15:48:39  dpg1
// Track ORB core changes.
//
// Revision 1.1.2.14  2001/09/24 10:48:27  dpg1
// Meaningful minor codes.
//
// Revision 1.1.2.13  2001/09/20 14:51:25  dpg1
// Allow ORB reinitialisation after destroy(). Clean up use of omni namespace.
//
// Revision 1.1.2.12  2001/08/15 10:37:14  dpg1
// Track ORB core object table changes.
//
// Revision 1.1.2.11  2001/06/15 10:59:26  dpg1
// Apply fixes from omnipy1_develop.
//
// Revision 1.1.2.10  2001/06/11 13:06:26  dpg1
// Support for PortableServer::Current.
//
// Revision 1.1.2.9  2001/06/01 11:09:26  dpg1
// Make use of new omni::ptrStrCmp() and omni::strCmp().
//
// Revision 1.1.2.8  2001/05/29 17:10:14  dpg1
// Support for in process identity.
//
// Revision 1.1.2.7  2001/05/14 12:47:22  dpg1
// Fix memory leaks.
//
// Revision 1.1.2.6  2001/05/10 15:16:03  dpg1
// Big update to support new omniORB 4 internals.
//
// Revision 1.1.2.5  2001/03/13 10:38:07  dpg1
// Fixes from omnipy1_develop
//
// Revision 1.1.2.4  2001/01/10 12:00:07  dpg1
// Release the Python interpreter lock when doing potentially blocking
// stream calls.
//
// Revision 1.1.2.3  2000/12/04 18:57:23  dpg1
// Fix deadlock when trying to lock omniORB internal lock while holding
// the Python interpreter lock.
//
// Revision 1.1.2.2  2000/11/22 14:42:56  dpg1
// Fix segfault in string_to_object and resolve_initial_references with
// nil objref.
//
// Revision 1.1.2.1  2000/10/13 13:55:26  dpg1
// Initial support for omniORB 4.
//

#include <omnipy.h>
#include <omniORBpy.h>

// Internal omniORB interfaces
#include <objectTable.h>
#include <remoteIdentity.h>
#include <inProcessIdentity.h>
#include <objectAdapter.h>
#include <omniORB4/omniURI.h>
#include <giopStrand.h>
#include <giopStream.h>
#include <omniCurrent.h>
#include <poaimpl.h>

OMNI_USING_NAMESPACE(omni)

#if defined(HAS_Cplusplus_Namespace)
using omniORB::operator==;
#endif


class Py_omniObjRef : public virtual CORBA::Object,
		      public virtual omniObjRef
{
public:
  Py_omniObjRef(const char*        repoId,
		omniIOR*           ior,
		omniIdentity*      id)

    : omniObjRef(repoId, ior, id)
  {
    _PR_setobj(this);
  }
  virtual ~Py_omniObjRef() { }

  virtual const char* _localServantTarget();

private:
  virtual void* _ptrToObjRef(const char* target);

  // Not implemented:
  Py_omniObjRef(const Py_omniObjRef&);
  Py_omniObjRef& operator=(const Py_omniObjRef&);
};

const char*
Py_omniObjRef::_localServantTarget()
{
  return omniPy::string_Py_omniServant;
}

void*
Py_omniObjRef::_ptrToObjRef(const char* target)
{
  if (omni::ptrStrMatch(target, omniPy::string_Py_omniObjRef))
    return (Py_omniObjRef*)this;

  if (omni::ptrStrMatch(target, CORBA::Object::_PD_repoId))
    return (CORBA::Object_ptr)this;

  return 0;
}


PyObject*
omniPy::createPyCorbaObjRef(const char*             targetRepoId,
			    const CORBA::Object_ptr objref)
{
  if (CORBA::is_nil(objref)) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  if (objref->_NP_is_pseudo())
    return createPyPseudoObjRef(objref);

  omniObjRef* ooref = objref->_PR_getobj();

  const char*    actualRepoId = ooref->_mostDerivedRepoId();
  PyObject*      objrefClass;
  CORBA::Boolean fullTypeUnknown = 0;

  // Try to find objref class for most derived type:
  objrefClass = PyDict_GetItemString(pyomniORBobjrefMap, (char*)actualRepoId);

  if (targetRepoId &&
      !omni::ptrStrMatch(targetRepoId, actualRepoId) &&
      !omni::ptrStrMatch(targetRepoId, CORBA::Object::_PD_repoId)) {

    // targetRepoId is not plain CORBA::Object, and is different from
    // actualRepoId

    if (objrefClass) {
      // We've got an objref class for the most derived type. Is it a
      // subclass of the target type?
      PyObject* targetClass = PyDict_GetItemString(pyomniORBobjrefMap,
						   (char*)targetRepoId);

      if (!omniPy::isSubclass(objrefClass, targetClass)) {
	// Actual type is not derived from the target. Surprisingly
	// enough, this is valid -- the repoId in an object reference
	// is not necessarily that of the most derived type for the
	// object. If we are expecting interface A, and actually get
	// unrelated B, the object might actually have interface C,
	// derived from both A and B.
	//
	// In this situation, we must create an object reference of
	// the target type, not the object's claimed type.
	objrefClass     = targetClass;
	fullTypeUnknown = 1;
      }
    }
    else {
      // No objref class for the most derived type -- try to find one for
      // the target type:
      objrefClass     = PyDict_GetItemString(pyomniORBobjrefMap,
					     (char*)targetRepoId);
      fullTypeUnknown = 1;
    }
  }
  if (!objrefClass) {
    // No target type, or stub code bug:
    objrefClass     = PyObject_GetAttrString(pyCORBAmodule, (char*)"Object");
    fullTypeUnknown = 1;
  }

  OMNIORB_ASSERT(objrefClass); // Couldn't even find CORBA.Object!

  PyObject* pyobjref = PyEval_CallObject(objrefClass, omniPy::pyEmptyTuple);

  if (!pyobjref) {
    // Oh dear -- return the error to the program
    return 0;
  }

  if (fullTypeUnknown) {
    PyObject* idstr = PyString_FromString(actualRepoId);
    PyObject_SetAttrString(pyobjref, (char*)"_NP_RepositoryId", idstr);
    Py_DECREF(idstr);
  }
  omniPy::setTwin(pyobjref, (CORBA::Object_ptr)objref, OBJREF_TWIN);

  return pyobjref;
}


PyObject*
omniPy::createPyPseudoObjRef(const CORBA::Object_ptr objref)
{
  {
    CORBA::ORB_var orbp = CORBA::ORB::_narrow(objref);
    if (!CORBA::is_nil(orbp)) {
      OMNIORB_ASSERT(omniPy::orb);
      return PyObject_GetAttrString(omniPy::pyomniORBmodule, (char*)"orb");
    }
  }
  {
    PortableServer::POA_var poa = PortableServer::POA::_narrow(objref);
    if (!CORBA::is_nil(poa)) return createPyPOAObject(poa);
  }
  {
    PortableServer::POAManager_var pm =
      PortableServer::POAManager::_narrow(objref);
    if (!CORBA::is_nil(pm)) return createPyPOAManagerObject(pm);
  }
  {
    PortableServer::Current_var pc = PortableServer::Current::_narrow(objref);
    if (!CORBA::is_nil(pc)) return createPyPOACurrentObject(pc);
  }
  do {
    // No built in converter. Try the list of registered external functions
    PyObject* fnlist = PyObject_GetAttrString(omniPy::py_omnipymodule,
					      (char*)"pseudoFns");
    if (!fnlist || !PySequence_Check(fnlist)) {
      PyErr_Clear();
      omniORB::logs(1, "WARNING: _omnipy.pseudoFns is not a sequence.");
      Py_XDECREF(fnlist);
      break;
    }
    int len = PySequence_Length(fnlist);
    for (int i=0; i < len; i++) {
      PyObject* pyf = PySequence_GetItem(fnlist, i);
      if (!PyCObject_Check(pyf)) {
	omniORB::logs(1, "WARNING: Entry in _omnipy.pseudoFns "
		      "is not a PyCObject.");
	continue;
      }
      omniORBpyPseudoFn f = (omniORBpyPseudoFn)PyCObject_AsVoidPtr(pyf);
      PyObject* ret = f(objref);
      if (ret) {
	Py_DECREF(fnlist);
	return ret;
      }
    }
    Py_DECREF(fnlist);

  } while (0);

  try {
    // Use OMNIORB_THROW to get a nice trace message
    OMNIORB_THROW(INV_OBJREF, INV_OBJREF_NoPythonTypeForPseudoObj,
		  CORBA::COMPLETED_NO);
  }
  OMNIPY_CATCH_AND_HANDLE_SYSTEM_EXCEPTIONS
  return 0;
}



omniObjRef*
omniPy::createObjRef(const char*    	targetRepoId,
		     omniIOR*       	ior,
		     CORBA::Boolean 	locked,
		     omniIdentity*  	id,
		     CORBA::Boolean     type_verified,
		     CORBA::Boolean     is_forwarded)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, locked);
  OMNIORB_ASSERT(targetRepoId);
  OMNIORB_ASSERT(ior);

  if (!id) {
    ior->duplicate();  // consumed by createIdentity
    id = omni::createIdentity(ior, omniPy::string_Py_omniServant, locked);
    if (!id) {
      ior->release();
      return 0;
    }
  }

  if (omniORB::trace(10)) {
    omniORB::logger l;
    l << "Creating Python ref to ";
    if      (omniLocalIdentity    ::downcast(id)) l << "local";
    else if (omniInProcessIdentity::downcast(id)) l << "in process";
    else if (omniRemoteIdentity   ::downcast(id)) l << "remote";
    else                                          l << "unknown";
    l << ": " << id << "\n"
      " target id      : " << targetRepoId << "\n"
      " most derived id: " << (const char*)ior->repositoryID() << "\n";
  }

  omniObjRef* objref = new Py_omniObjRef(targetRepoId, ior, id);

  if (!type_verified &&
      !omni::ptrStrMatch(targetRepoId, CORBA::Object::_PD_repoId)) {

    objref->pd_flags.type_verified = 0;
  }

  if (is_forwarded) {
    omniORB::logs(10, "Reference has been forwarded.");
    objref->pd_flags.forward_location = 1;
  }

  {
    omni_optional_lock sync(*omni::internalLock, locked, locked);
    id->gainRef(objref);
  }

  if (orbParameters::persistentId.length()) {
    // Check to see if we need to re-write the IOR.

    omniIOR::IORExtraInfoList& extra = ior->getIORInfo()->extraInfo();

    for (CORBA::ULong index = 0; index < extra.length(); index++) {

      if (extra[index]->compid == IOP::TAG_OMNIORB_PERSISTENT_ID)

	if (!id->inThisAddressSpace()) {

	  omniORB::logs(15, "Re-write local persistent object reference.");

	  omniObjRef* new_objref;
	  omniIORHints hints(0);
	  {
	    omni_optional_lock sync(*internalLock, locked, locked);

	    omniIOR* new_ior = new omniIOR(ior->repositoryID(),
					   id->key(), id->keysize(), hints);

	    new_objref = createObjRef(targetRepoId, new_ior,
				      1, 0, type_verified);
	  }
	  releaseObjRef(objref);
	  objref = new_objref;
	}
      break;
    }
  }
  return objref;
}


omniObjRef*
omniPy::createLocalObjRef(const char*         mostDerivedRepoId,
			  const char*         targetRepoId,
			  omniObjTableEntry*  entry,
			  omniObjRef*         orig_ref,
			  CORBA::Boolean      type_verified)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(targetRepoId);
  OMNIORB_ASSERT(entry);

  // See if a suitable reference exists in the local ref list.
  // Suitable means having the same most-derived-intf-repo-id, and
  // also supporting the <targetRepoId>.
  {
    omniObjRef* objref;

    omnivector<omniObjRef*>::iterator i    = entry->objRefs().begin();
    omnivector<omniObjRef*>::iterator last = entry->objRefs().end();

    for (; i != last; i++) {
      objref = *i;

      if (omni::ptrStrMatch(mostDerivedRepoId, objref->_mostDerivedRepoId()) &&
	  objref->_ptrToObjRef(omniPy::string_Py_omniObjRef) &&
	  omni::ptrStrMatch(targetRepoId, objref->pd_intfRepoId)) {

	// We just need to check that the ref count is not zero here,
	// 'cos if it is then the objref is about to be deleted!
	// See omni::releaseObjRef().

	omni::objref_rc_lock->lock();
	int dying = objref->pd_refCount == 0;
	if( !dying )  objref->pd_refCount++;
	omni::objref_rc_lock->unlock();

	if( !dying ) {
	  omniORB::logs(15, "omniPy::createLocalObjRef -- reusing "
			"reference from local ref list.");
	  return objref;
	}
      }
    }
  }
  // Reach here if we have to create a new objref.
  omniIOR* ior = orig_ref->_getIOR();
  return omniPy::createObjRef(targetRepoId, ior, 1, entry, type_verified);
}

omniObjRef*
omniPy::createLocalObjRef(const char* 	      mostDerivedRepoId,
			  const char* 	      targetRepoId,
			  const _CORBA_Octet* key,
			  int                 keysize,
			  omniObjRef*         orig_ref,
			  CORBA::Boolean      type_verified)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(targetRepoId);
  OMNIORB_ASSERT(key && keysize);

  // See if there's a suitable entry in the object table
  CORBA::ULong hashv = omni::hash(key, keysize);

  omniObjTableEntry* entry = omniObjTable::locateActive(key, keysize,
							hashv, 0);

  if (entry)
    return createLocalObjRef(mostDerivedRepoId, targetRepoId,
			     entry, orig_ref, type_verified);

  omniIOR* ior = orig_ref->_getIOR();
  return createObjRef(targetRepoId,ior,1,0,type_verified);
}




CORBA::Object_ptr
omniPy::makeLocalObjRef(const char* targetRepoId,
			const CORBA::Object_ptr objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  omniObjRef* ooref = objref->_PR_getobj();
  omniObjRef* newooref;

  {
    omni_tracedmutex_lock sync(*omni::internalLock);
    
    omniObjTableEntry* entry = omniObjTableEntry::downcast(ooref->_identity());

    if (entry)
      newooref = omniPy::createLocalObjRef(ooref->_mostDerivedRepoId(),
					   targetRepoId, entry, ooref, 1);
    else
      newooref = omniPy::createLocalObjRef(ooref->_mostDerivedRepoId(),
					   targetRepoId,
					   ooref->_identity()->key(),
					   ooref->_identity()->keysize(),
					   ooref, 1);
  }
  return (CORBA::Object_ptr)newooref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}


PyObject*
omniPy::copyObjRefArgument(PyObject* pytargetRepoId, PyObject* pyobjref,
			   CORBA::CompletionStatus compstatus)
{
  if (pyobjref == Py_None) {
    // Nil objref
    Py_INCREF(Py_None);
    return Py_None;
  }
  CORBA::Object_ptr objref = (CORBA::Object_ptr)getTwin(pyobjref, OBJREF_TWIN);
  if (!objref) {
    // Not an objref
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
  }

  // To copy an object reference, we have to take a number of things
  // into account. When the C++ object reference was created, it was
  // initialised with a most-derived repoId and a target repoId. If we
  // knew that the most-derived interface is compatible with the
  // target, then the Python objref is of the most derived type. If we
  // did not know the most-derived interface, or we did know it and
  // believed it to be incompatible with the target, then the Python
  // objref is of the target type, and it has a string attribute named
  // "_NP_RepositoryId" containing the most derived repoId.
  //
  // Now, as we are copying this objref, we have a target repoId,
  // which is possibly different from the objref's original target.
  // It's also possible that some time after we created the Python
  // objref, some new stubs were imported, so we now know about the
  // objref's most derived type when before we didn't.
  //
  // So, to copy the reference, we first see if the Python objref has
  // an attribute named "_NP_RepositoryId". If it does, all bets are
  // off, and we have to create a new C++ objref from scratch. If it
  // doesn't have the attribute, we look to see if the objref's class
  // is a subclass of the target objref class (or the same class). If
  // so, we can just incref the existing Python objref and return it;
  // if not, we have to build a new C++ objref.

  if (!PyObject_HasAttrString(pyobjref, (char*)"_NP_RepositoryId")) {

    PyObject* targetClass = PyDict_GetItem(pyomniORBobjrefMap,
					   pytargetRepoId);
    OMNIORB_ASSERT(targetClass);

    if (omniPy::isInstance(pyobjref, targetClass)) {
      Py_INCREF(pyobjref);
      return pyobjref;
    }
  }
  // Create new C++ and Python objrefs with the right target type
  omniObjRef* ooref        = objref->_PR_getobj();
  const char* targetRepoId = PyString_AS_STRING(pytargetRepoId);

  if (targetRepoId[0] == '\0') targetRepoId = CORBA::Object::_PD_repoId;

  omniObjRef* newooref;
  {
    omniPy::InterpreterUnlocker _u;
    newooref = omniPy::createObjRef(targetRepoId, ooref->_getIOR(), 0, 0);
  }
  PyObject* r = createPyCorbaObjRef(targetRepoId,
				    (CORBA::Object_ptr)newooref->
				      _ptrToObjRef(CORBA::Object::_PD_repoId));
  if (!r) {
    if (omniORB::trace(1)) {
      {
	omniORB::logger l;
	l <<
	  "Caught an unexpected Python exception trying to create an "
	  "object reference.\n";
      }
      PyErr_Print();
    }
    PyErr_Clear();
    OMNIORB_THROW(INTERNAL, 0, compstatus);
  }
  return r;
}


CORBA::Object_ptr
omniPy::stringToObject(const char* uri)
{
  CORBA::Object_ptr cxxobj;
  omniObjRef* objref;

  {
    omniPy::InterpreterUnlocker _u;
    cxxobj = omniURI::stringToObject(uri);

    if (CORBA::is_nil(cxxobj) || cxxobj->_NP_is_pseudo()) {
      return cxxobj;
    }
    omniObjRef* cxxobjref = cxxobj->_PR_getobj();

    objref = omniPy::createObjRef(CORBA::Object::_PD_repoId,
				  cxxobjref->_getIOR(), 0, 0);
    CORBA::release(cxxobj);
  }
  return (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}


CORBA::Object_ptr
omniPy::UnMarshalObjRef(const char* repoId, cdrStream& s)
{
  CORBA::String_var           id;
  IOP::TaggedProfileList_var  profiles;

  id = IOP::IOR::unmarshaltype_id(s);

  profiles = new IOP::TaggedProfileList();
  (IOP::TaggedProfileList&)profiles <<= s;

  if (profiles->length() == 0 && strlen(id) == 0) {
    // Nil object reference
    return CORBA::Object::_nil();
  }
  else {
    omniPy::InterpreterUnlocker _u;

    // It is possible that we reach here with the id string = '\0'.
    // That is alright because the actual type of the object will be
    // verified using _is_a() at the first invocation on the object.
    //
    // Apparently, some ORBs such as ExperSoft's do that. Furthermore,
    // this has been accepted as a valid behaviour in GIOP 1.1/IIOP 1.1.
    // 
    omniIOR* ior = new omniIOR(id._retn(),profiles._retn());

    giopStream* gs = giopStream::downcast(&s);
    if (gs) {
      giopStrand& g = (giopStrand&)*gs;
      if (g.biDir && !g.isClient()) {
	// Check the POA policy to see if the servant's POA is willing
	// to use bidirectional on its callback objects.
	omniCurrent* current = omniCurrent::get();
	omniCallDescriptor* desc = ((current)? current->callDescriptor() :0);

	if (desc && desc->poa() && desc->poa()->acceptBiDirectional()) {
	  const char* sendfrom = g.connection->peeraddress();
	  omniIOR::add_TAG_OMNIORB_BIDIR(sendfrom,*ior);
	}
      }
    }
    omniObjRef* objref = omniPy::createObjRef(repoId,ior,0);

    if (!objref) OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIOR,
			       (CORBA::CompletionStatus)s.completion());
    return 
      (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
  }
  return 0; // To shut GCC up
}
