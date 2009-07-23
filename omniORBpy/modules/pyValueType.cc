// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyValueType.cc             Created on: 2003/04/11
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2003-2006 Apasphere Ltd.
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
//    ValueType support

// $Log$
// Revision 1.1.2.14  2006/09/20 14:10:13  dgrisby
// Indirections could be wrong in valuebox marshalling, because alignment
// was not set before position calculation.
//
// Revision 1.1.2.13  2006/09/17 23:28:34  dgrisby
// Invalid assertion with indirections in counting streams.
//
// Revision 1.1.2.12  2006/05/15 10:26:11  dgrisby
// More relaxation of requirements for old-style classes, for Python 2.5.
//
// Revision 1.1.2.11  2006/02/28 12:42:00  dgrisby
// New _NP_postUnmarshal hook on valuetypes.
//
// Revision 1.1.2.10  2005/06/29 17:31:42  dgrisby
// Update valuetype examples; fix values in Anys.
//
// Revision 1.1.2.9  2005/06/24 17:36:00  dgrisby
// Support for receiving valuetypes inside Anys; relax requirement for
// old style classes in a lot of places.
//
// Revision 1.1.2.8  2005/01/17 15:19:11  dgrisby
// Minor changes to compile on Windows.
//
// Revision 1.1.2.7  2005/01/06 23:22:27  dgrisby
// Properly align output in valuetype marshalling.
//
// Revision 1.1.2.6  2004/03/24 22:28:50  dgrisby
// TypeCodes / truncation for inherited state members were broken.
//
// Revision 1.1.2.5  2004/02/16 10:14:18  dgrisby
// Use stream based copy for local calls.
//
// Revision 1.1.2.4  2003/11/06 12:00:36  dgrisby
// ValueType TypeCode support; track ORB core changes.
//
// Revision 1.1.2.3  2003/09/26 15:57:07  dgrisby
// Refactor repoId handling.
//
// Revision 1.1.2.2  2003/07/10 22:15:02  dgrisby
// Fix locking issues (merge from omnipy2_develop).
//
// Revision 1.1.2.1  2003/05/20 17:10:24  dgrisby
// Preliminary valuetype support.
//

#include <omnipy.h>
#include <pyThreadCache.h>

OMNI_USING_NAMESPACE(omni)

//
// Tag constants
//

#define CODEBASE_URL  1
#define REPOID_SINGLE 2
#define REPOID_LIST   6
#define REPOID_MASK   6
#define CHUNKED       8


//
// Value indirection trackers
//

class pyOutputValueTracker : public ValueIndirectionTracker {
public:
  inline pyOutputValueTracker() :
    magic_(MAGIC_), dict_(PyDict_New()), in_truncatable_(0)
  {
    omniORB::logs(25, "Create Python output value indirection tracker");
  }

  virtual ~pyOutputValueTracker()
  {
    omniORB::logs(25, "Delete Python output value indirection tracker");

    omnipyThreadCache::lock _t;
    Py_DECREF(dict_);
  }

  inline CORBA::Boolean valid()
  {
    return magic_ == MAGIC_;
  }

  CORBA::Long addValue(PyObject* obj, CORBA::Long current)
  {
    // Look to see if the value has been marshalled before. If so,
    // return its offset; if not, add it to the table and return -1.

    PyObject* key = PyLong_FromVoidPtr(obj); // id(obj)
    PyObject* val = PyDict_GetItem(dict_, key);

    if (val) {
      OMNIORB_ASSERT(PyInt_Check(val));
      CORBA::Long pos = PyInt_AS_LONG(val);
      Py_DECREF(key);
      return pos;
    }
    else {
      PyObject* val = PyInt_FromLong(current);
      PyDict_SetItem(dict_, key, val);
      Py_DECREF(val);
      Py_DECREF(key);
      return -1;
    }
  }

  CORBA::Long addRepoIds(PyObject* obj, CORBA::Long current)
  {
    // Caller is marshalling a repoid or a list of repoids stored in a
    // tuple.
    PyObject* val = PyDict_GetItem(dict_, obj);

    if (val) {
      OMNIORB_ASSERT(PyInt_Check(val));
      CORBA::Long pos = PyInt_AS_LONG(val);
      return pos;
    }
    else {
      val = PyInt_FromLong(current);
      PyDict_SetItem(dict_, obj, val);
      Py_DECREF(val);
      return -1;
    }
  }

  inline void startTruncatable() {
    in_truncatable_++;
  }
  inline void endTruncatable() {
    in_truncatable_--;
  }
  inline CORBA::Boolean inTruncatable() {
    return (in_truncatable_ > 0);
  }
  // Inside a truncatable value, repoIds of nested values must always
  // be sent, in case the receiver truncates the first instance of the
  // nested value away, but later receives an indirection to it.


private:
  static const CORBA::ULong MAGIC_;
  CORBA::ULong magic_;

  PyObject*    dict_;
  CORBA::ULong in_truncatable_;
};

const CORBA::ULong pyOutputValueTracker::MAGIC_ = 0x50594f56; // "PYOV"


class pyInputValueTracker : public ValueIndirectionTracker {
public:
  inline pyInputValueTracker() :
    magic_(MAGIC_), dict_(PyDict_New())
  {
    omniORB::logs(25, "Create Python input value indirection tracker");
  }

  virtual ~pyInputValueTracker()
  {
    omniORB::logs(25, "Delete Python input value indirection tracker");

    omnipyThreadCache::lock _t;
    Py_DECREF(dict_);
  }

  inline CORBA::Boolean valid()
  {
    return magic_ == MAGIC_;
  }
  
  void add(PyObject* obj, CORBA::Long pos)
  {
    // Add record of an unmarshalled value.
    PyObject* key = PyInt_FromLong(pos);
    PyDict_SetItem(dict_, key, obj);
    Py_DECREF(key);
  }

  PyObject* lookup(CORBA::Long pos, CORBA::CompletionStatus completion)
  {
    // Lookup specified position for a previously unmarshalled value.
    // If the value has been previously unmarshalled, return a
    // duplicated reference to it. Otherwise, throw a MARSHAL
    // exception.
    PyObject* key = PyInt_FromLong(pos);
    PyObject* ret = PyDict_GetItem(dict_, key);
    Py_DECREF(key);

    if (!ret)
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIndirection, completion);

    Py_INCREF(ret);
    return ret;
  }

private:
  static const CORBA::ULong MAGIC_;
  CORBA::ULong magic_;

  PyObject*    dict_;
};

const CORBA::ULong pyInputValueTracker::MAGIC_ = 0x50594956; // "PYIV"


//
// Marshalling functions
//

static void validateMembers(PyObject* d_o, PyObject* a_o,
			    CORBA::CompletionStatus compstatus,
			    PyObject* track);

void
omniPy::
validateTypeValue(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus,
		  PyObject* track)
{ // class, repoid, value name, valuemodifier, truncatable base repoids,
  // concrete base descr, [ member name, desc, visibility ] ...

  if (a_o == Py_None) // Nil value
    return;

  PyObject* idlRepoId = PyTuple_GET_ITEM(d_o, 2);

  // Detect cycles
  CORBA::Boolean track_alloc = 0;
  PyObject* key = PyTuple_New(2);
  Py_INCREF(idlRepoId);
  PyTuple_SET_ITEM(key, 0, PyLong_FromVoidPtr(a_o));
  PyTuple_SET_ITEM(key, 1, idlRepoId);

  if (track) {
    if (PyDict_GetItem(track, key)) {
      // Seen this value before.
      Py_DECREF(key);
      return;
    }
  }
  else {
    track_alloc = 1;
    track = PyDict_New();
  }
  PyDict_SetItem(track, key, Py_None);
  Py_DECREF(key);

  try {
    PyObject* actualRepoId = PyObject_GetAttr(a_o, omniPy::pyNP_RepositoryId);

    if (!actualRepoId) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
    }

    Py_DECREF(actualRepoId); // Safe because obj still holds a ref
  
    if (!omni::ptrStrMatch(PyString_AS_STRING(idlRepoId),
			   PyString_AS_STRING(actualRepoId))) {
      // Object doesn't match the value expected from the IDL. Check it
      // is derived from it.
      PyObject* cls = PyTuple_GET_ITEM(d_o, 1);

      if (!omniPy::isInstance(a_o, cls))
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);

      d_o = PyDict_GetItem(omniPy::pyomniORBtypeMap, actualRepoId);
    }

    // Check value modifier
    PyObject* pymod = PyTuple_GET_ITEM(d_o, 4);
    CORBA::ValueModifier mod = PyInt_AS_LONG(pymod);

    if (mod == CORBA::VM_ABSTRACT)
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_AttemptToMarshalAbstractValue,
		    compstatus);

    if (mod == CORBA::VM_CUSTOM)
      OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, compstatus);

    validateMembers(d_o, a_o, compstatus, track);

    if (track_alloc)
      Py_DECREF(track);
  }
  catch (...) {
    if (track_alloc)
      Py_DECREF(track);
    throw;
  }
}

static void validateMembers(PyObject* d_o, PyObject* a_o,
			    CORBA::CompletionStatus compstatus,
			    PyObject* track)
{
  PyObject* t_o = PyTuple_GET_ITEM(d_o, 0);
  OMNIORB_ASSERT(PyInt_Check(t_o) && PyInt_AS_LONG(t_o) == CORBA::tk_value);

  // Check base
  t_o = PyTuple_GET_ITEM(d_o, 6);
  if (PyTuple_Check(t_o))
    validateMembers(t_o, a_o, compstatus, track);

  // The descriptor has three times the number of value members, plus 7.
  int members = (PyTuple_GET_SIZE(d_o) - 7) / 3;

  PyObject* name;
  PyObject* value;

  int i, j;

  for (i=0,j=7; i < members; i++, j+=3) {
    name    = PyTuple_GET_ITEM(d_o, j);
    OMNIORB_ASSERT(PyString_Check(name));
    value   = PyObject_GetAttr(a_o, name);
    if (!value) {
      PyErr_Clear();
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType, compstatus);
    }

    Py_DECREF(value); // Safe to DECREF now because object still holds a ref
    omniPy::validateType(PyTuple_GET_ITEM(d_o, j+1), value,
			 compstatus, track);
  }
}


void
omniPy::
validateTypeValueBox(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus,
		     PyObject* track)
{ // class, repoid, valuebox name, boxed type desc

  if (a_o == Py_None) // Nil value
    return;

  omniPy::validateType(PyTuple_GET_ITEM(d_o, 4), a_o, compstatus, track);
}



static void
marshalIndirection(cdrStream& stream, CORBA::Long pos)
{
  stream.declareArrayLength(omni::ALIGN_4, 8);
  CORBA::ULong indirect = 0xffffffff;
  indirect >>= stream;

  CORBA::Long offset = pos - stream.currentOutputPtr();

  OMNIORB_ASSERT(offset < -4 || stream.currentOutputPtr() == 0);
  // In a counting stream, the currentOutputPtr is always zero.

  offset >>= stream;
}


static void
marshalMembers(cdrValueChunkStream& stream, PyObject* d_o, PyObject* a_o);

static void
real_marshalPyObjectValue(cdrValueChunkStream& stream,
			  PyObject* d_o, PyObject* a_o)
{ // class, repoid, value name, valuemodifier, truncatable base repoids,
  // concrete base descr, [ member name, desc, visibility ] ...

  CORBA::Long tag = 0x7fffff00 | CHUNKED;

  pyOutputValueTracker* tracker = (pyOutputValueTracker*)stream.valueTracker();

  PyObject* idlRepoId    = PyTuple_GET_ITEM(d_o, 2);
  PyObject* actualRepoId = PyObject_GetAttr(a_o, omniPy::pyNP_RepositoryId);

  OMNIORB_ASSERT(actualRepoId);
  Py_DECREF(actualRepoId); // Obj still holds a ref

  // Find descriptor of actual value being marshalled
  CORBA::Boolean derived = 0;
  if (!omni::ptrStrMatch(PyString_AS_STRING(idlRepoId),
			 PyString_AS_STRING(actualRepoId))) {

    d_o = PyDict_GetItem(omniPy::pyomniORBtypeMap, actualRepoId);
    derived = 1;
  }

  // Determine whether to send repoId(s)
  PyObject* baseIds = 0;

  if (derived) {
    baseIds = PyTuple_GET_ITEM(d_o, 5);

    if (baseIds == Py_None) {
      tag |= REPOID_SINGLE;
    }
    else {
      tag |= REPOID_LIST;
    }
  }
  else {
    // Not a derived value.

    if (tracker->inTruncatable()) {
      // Nested inside a truncatable value, we always send the repoId(s)
      // in case the receiver truncates, then has a later indirection.
      baseIds = PyTuple_GET_ITEM(d_o, 5);

      if (baseIds == Py_None) {
	tag |= REPOID_SINGLE;
      }
      else {
	tag |= REPOID_LIST;
      }
    }
    else {
      // RMI: repository ids must always be sent
      const char* id = PyString_AS_STRING(actualRepoId);
      if (id[0] == 'R' && id[1] == 'M' && id[2] == 'I' && id[3] == ':') {
	tag |= REPOID_SINGLE;
      }
    }
  }

  // Start the value header
  stream.startOutputValueHeader(tag);

  // Marshal repoId(s) if necessary
  if ((tag & REPOID_MASK) == REPOID_LIST) {

    OMNIORB_ASSERT(baseIds && baseIds != Py_None);

    CORBA::Long pos = tracker->addRepoIds(baseIds, stream.currentOutputPtr());

    if (pos != -1) {
      marshalIndirection(stream, pos);
    }
    else {
      CORBA::Long bases = PyTuple_GET_SIZE(baseIds);
      bases >>= stream;
      for (CORBA::Long i=0; i<bases; i++) {
	stream.alignOutput(omni::ALIGN_4);
	PyObject* id = PyTuple_GET_ITEM(baseIds, i);
	pos = tracker->addRepoIds(id, stream.currentOutputPtr());
	if (pos != -1)
	  marshalIndirection(stream, pos);
	else
	  omniPy::marshalRawPyString(stream, id);
      }
    }
  }
  else if ((tag & REPOID_MASK) == REPOID_SINGLE) {
    CORBA::Long pos = tracker->addRepoIds(actualRepoId,
					  stream.currentOutputPtr());
    if (pos != -1)
      marshalIndirection(stream, pos);
    else
      omniPy::marshalRawPyString(stream, actualRepoId);
  }

  // Finally, we marshal the members
  stream.startOutputValueBody();

  if ((tag & REPOID_MASK) == REPOID_LIST)
    tracker->startTruncatable();

  marshalMembers(stream, d_o, a_o);

  if ((tag & REPOID_MASK) == REPOID_LIST)
    tracker->endTruncatable();

  stream.endOutputValue();
}

static void
marshalMembers(cdrValueChunkStream& stream, PyObject* d_o, PyObject* a_o)
{
  PyObject* t_o = PyTuple_GET_ITEM(d_o, 6);
  if (PyTuple_Check(t_o))
    marshalMembers(stream, t_o, a_o);

  int members = (PyTuple_GET_SIZE(d_o) - 7) / 3;

  PyObject* name;
  PyObject* value;

  int i, j;

  for (i=0,j=7; i < members; i++, j+=3) {
    name    = PyTuple_GET_ITEM(d_o, j);
    value   = PyObject_GetAttr(a_o, name);

    Py_DECREF(value); // Safe to DECREF now because object still holds a ref
    omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(d_o, j+1), value);
  }
}  


void
omniPy::
marshalPyObjectValue(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // class, repoid, value name, valuemodifier, truncatable base repoids,
  // concrete base descr, [ member name, desc, visibility ] ...

  if (a_o == Py_None) { // Nil
    CORBA::Long tag = 0;
    tag >>= stream;
    return;
  }

  if (!stream.valueTracker()) {
    stream.valueTracker(new pyOutputValueTracker());
  }

  pyOutputValueTracker* tracker = (pyOutputValueTracker*)stream.valueTracker();
  
  OMNIORB_ASSERT(tracker->valid());

  stream.alignOutput(omni::ALIGN_4);

  CORBA::Long pos = tracker->addValue(a_o, stream.currentOutputPtr());

  if (pos != -1) {
    marshalIndirection(stream, pos);
    return;
  }

  // We choose to always send values chunked. That's much easier than
  // figuring out when we have to chunk.

  cdrValueChunkStream* cstreamp = cdrValueChunkStream::downcast(&stream);
  if (cstreamp) {
    real_marshalPyObjectValue(*cstreamp, d_o, a_o);
  }
  else {
    cdrValueChunkStream cstream(stream);
    try {
      real_marshalPyObjectValue(cstream, d_o, a_o);
    }
    catch (...) {
      cstream.exceptionOccurred();
      throw;
    }
  }
}

void
omniPy::
marshalPyObjectValueBox(cdrStream& stream, PyObject* d_o, PyObject* a_o)
{ // class, repoid, valuebox name, boxed type desc

  if (a_o == Py_None) { // Nil
    CORBA::Long tag = 0;
    tag >>= stream;
    return;
  }

  if (!stream.valueTracker()) {
    stream.valueTracker(new pyOutputValueTracker());
  }

  pyOutputValueTracker* tracker = (pyOutputValueTracker*)stream.valueTracker();
  
  OMNIORB_ASSERT(tracker->valid());

  stream.alignOutput(omni::ALIGN_4);

  CORBA::Long pos = tracker->addValue(a_o, stream.currentOutputPtr());

  if (pos != -1) {
    marshalIndirection(stream, pos);
    return;
  }

  cdrValueChunkStream* cstreamp = cdrValueChunkStream::downcast(&stream);

  CORBA::Long tag = 0x7fffff00;

  PyObject* repoId = PyTuple_GET_ITEM(d_o, 2);

  // ValueBoxes are only sent chunked if they're nested inside chunked values.
  if (cstreamp)
    tag |= CHUNKED;

  if (tracker->inTruncatable()) {
    tag |= REPOID_SINGLE;
  }
  else {
    const char* id = PyString_AS_STRING(repoId);
    if (id[0] == 'R' && id[1] == 'M' && id[2] == 'I' && id[3] == ':') {
      tag |= REPOID_SINGLE;
    }
  }

  if (cstreamp)
    cstreamp->startOutputValueHeader(tag);
  else
    tag >>= stream;

  if (tag & REPOID_SINGLE) {
    CORBA::Long pos = tracker->addRepoIds(repoId, stream.currentOutputPtr());
    if (pos != -1)
      marshalIndirection(stream, pos);
    else
      omniPy::marshalRawPyString(stream, repoId);
  }

  // Marshal the boxed value
  if (cstreamp)
    cstreamp->startOutputValueBody();

  omniPy::marshalPyObject(stream, PyTuple_GET_ITEM(d_o, 4), a_o);

  if (cstreamp)
    cstreamp->endOutputValue();
}



//
// Unmarshalling functions
//

static PyObject*
unmarshalValueRepoId(cdrStream& stream, pyInputValueTracker* tracker)
{
  // Unmarshal a raw string, or an indirection to one

  CORBA::ULong len; len <<= stream;
  CORBA::Long  pos = stream.currentInputPtr();

  if (len == 0xffffffff) {
    CORBA::Long offset;
    offset <<= stream;
    if (offset >= -4) {
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIndirection,
		    (CORBA::CompletionStatus)stream.completion());
    }
    return tracker->lookup(pos + offset,
			   (CORBA::CompletionStatus)stream.completion());
  }
  if (!stream.checkInputOverrun(1, len))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)stream.completion());

  PyObject* pystring = PyString_FromStringAndSize(0, len - 1);

  stream.get_octet_array((_CORBA_Octet*)PyString_AS_STRING(pystring), len);

  tracker->add(pystring, pos-4);
  return pystring;
}

static void
unmarshalMembers(cdrStream& stream, PyObject* desc,
		 PyObject* instance, PyObject* member_list);

static PyObject*
real_unmarshalPyObjectValue(cdrStream& stream, cdrValueChunkStream* cstreamp,
			    PyObject* d_o, CORBA::ULong tag, CORBA::Long pos)
{ // class, repoid, value name, valuemodifier, truncatable base repoids,
  // concrete base descr, [ member name, desc, visibility ] ...
  //or:
  // class, repoid, valuebox name, boxed type desc

  pyInputValueTracker* tracker = (pyInputValueTracker*)stream.valueTracker();

  // Ignore any codebase URLs

  if (tag & CODEBASE_URL) {
    CORBA::ULong length;
    length <<= stream;
    if (length == 0xffffffff) { // Indirection. Skip it.
      length <<= stream;
    }
    else {
      stream.skipInput(length);
    }
  }

  CORBA::Boolean truncating = 0;
  int i;

  PyObject* instance  = 0;
  PyObject* factory   = 0;
  PyObject* desc      = 0;
  PyObject* idlrepoId = PyTuple_GET_ITEM(d_o, 2);
  PyObject* repoId;

  // Read any repoIds

  if ((tag & REPOID_MASK) == REPOID_LIST) {
    PyObject* repoIds;

    CORBA::ULong count;
    count <<= stream;

    CORBA::Long idpos = stream.currentInputPtr();

    if (count == 0xffffffff) { // Indirection
      CORBA::Long offset;
      offset <<= stream;

      if (offset >= -4) {
	OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIndirection,
		      (CORBA::CompletionStatus)stream.completion());
      }
      repoIds = tracker->lookup(idpos + offset,
				(CORBA::CompletionStatus)stream.completion());
    }
    else {
      if (!stream.checkInputOverrun(4, count, omni::ALIGN_4)) {
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong,
		      (CORBA::CompletionStatus)stream.completion());
      }
      repoIds = PyTuple_New(count);
      if (!repoIds)
	OMNIORB_THROW(NO_MEMORY, 0,
		      (CORBA::CompletionStatus)stream.completion());

      for (CORBA::ULong i=0; i<count; i++) {
	PyObject* repoId = unmarshalValueRepoId(stream, tracker);
	PyTuple_SET_ITEM(repoIds, i, repoId);
      }
    }
    tracker->add(repoIds, idpos - 4);

    for (i=0; i < PyTuple_GET_SIZE(repoIds); i++) {
      repoId  = PyTuple_GET_ITEM(repoIds, i);
      factory = PyDict_GetItem(omniPy::pyomniORBvalueMap, repoId);
      if (factory) {
	// Make sure we have a type descriptor
	if (omni::ptrStrMatch(PyString_AS_STRING(repoId),
			      PyString_AS_STRING(idlrepoId))) {
	  // Most derived id is target
	  desc = d_o;
	  break;
	}
	else {
	  desc = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);
	  if (desc)
	    break;
	}
      }
      else {
	if (omni::ptrStrMatch(PyString_AS_STRING(repoId),
			      PyString_AS_STRING(idlrepoId))) {
	  // RepoId matches the target, but we don't have a factory
	  // for it. We can't truncate any further. Break out here,
	  // and throw MARSHAL below.
	  desc = d_o;
	  break;
	}
      }
    }
    Py_INCREF(repoId);
    Py_DECREF(repoIds);
    if (i > 0) {
      // If the first repoId didn't match, we need to truncate.
      truncating = 1;
    }      
  }
  else if ((tag & REPOID_MASK) == REPOID_SINGLE) {
    repoId  = unmarshalValueRepoId(stream, tracker);
    factory = PyDict_GetItem(omniPy::pyomniORBvalueMap, repoId);

    if (omni::ptrStrMatch(PyString_AS_STRING(repoId),
			  PyString_AS_STRING(idlrepoId)))
      desc = d_o;
    else
      desc = PyDict_GetItem(omniPy::pyomniORBtypeMap, repoId);
  }
  else {
    // No repoId marshalled -- it's the target type
    if (d_o == omniPy::pyCORBAValueBaseDesc) {
      // If we're expecting ValueBase, the sender must specify a repoId.
      OMNIORB_THROW(MARSHAL, MARSHAL_NoRepoIdInValueType,
		    (CORBA::CompletionStatus)stream.completion());
    }
    Py_INCREF(idlrepoId);
    repoId  = idlrepoId;
    factory = PyDict_GetItem(omniPy::pyomniORBvalueMap, repoId);
    desc    = d_o;
  }

  // If the value is chunked, tell the chunk stream we're about to
  // unmarshal the value body.
  if (cstreamp)
    cstreamp->startInputValueBody();

  // Check there's a factory and a type descriptor for the chosen repoId.

  try {
    PyObject* member_list = 0;

    if (!factory || factory == Py_None) {
      if (desc) {
	PyObject* vclass = PyTuple_GET_ITEM(desc, 1);
	if (omniPy::isSubclass(vclass, omniPy::pyomniORBUnknownValueBase)) {
	  // Value is inside an Any, and has a TypeCode for which we
	  // have no static knowledge. We create an instance of the
	  // class created as the TypeCode was unmarshalled.
	  omniORB::logs(25, "Unmarshal unknown valuetype inside Any.");
	  factory = vclass;
	  member_list = PyList_New(0);
	}
      }
      if (!factory || factory == Py_None) {
	OMNIORB_THROW(MARSHAL, MARSHAL_NoValueFactory,
		      (CORBA::CompletionStatus)stream.completion());
      }
    }

    if (!desc)
      OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_NoValueImpl,
		    (CORBA::CompletionStatus)stream.completion());

    // Are we unmarshalling a valuetype or valuebox?
    CORBA::ULong dtype = PyInt_AS_LONG(PyTuple_GET_ITEM(desc, 0));

    if (dtype == CORBA::tk_value) {
      PyObject* target = PyTuple_GET_ITEM(d_o, 1);

      // Call the factory
      instance = PyObject_CallObject(factory, 0);

      if (!instance)
	omniPy::handlePythonException();

      if (!omniPy::isInstance(instance, target))
	OMNIORB_THROW(BAD_PARAM, BAD_PARAM_WrongPythonType,
		      (CORBA::CompletionStatus)stream.completion());

      tracker->add(instance, pos);

      // Finally, we have a blank value which we can unmarshal the members to.
      unmarshalMembers(stream, desc, instance, member_list);
      if (member_list) {
	PyObject_SetAttrString(instance, (char*)"_values", member_list);
	Py_DECREF(member_list);
      }

      // Allow the object to finish initialising itself after unmarshalling.
      PyObject* postHook = PyObject_GetAttrString(instance,
                                                  (char*)"_NP_postUnmarshal");
      if (postHook) {
        PyObject* hookResult = PyObject_CallObject(postHook, 0);
        if (!hookResult) {
          Py_DECREF(postHook);
          omniPy::handlePythonException();
        }
        Py_DECREF(postHook);
        Py_DECREF(instance);
        instance = hookResult;
      }
      else {
        PyErr_Clear();
      }
    }
    else if (dtype == CORBA::tk_value_box) {
      PyObject* boxedtype = PyTuple_GET_ITEM(desc, 4);
      instance = omniPy::unmarshalPyObject(stream, boxedtype);
      tracker->add(instance, pos);
    }
    else {
      OMNIORB_THROW(MARSHAL, MARSHAL_TypeIsNotAValueType,
		    (CORBA::CompletionStatus)stream.completion());
    }

    // Skip remaining parts if we're truncating.

    if (truncating) {
      if (omniORB::trace(25)) {
	omniORB::logger l;
	l << "Truncating input value to "
	  << PyString_AS_STRING(repoId) << "\n";
      }
      cdrValueChunkStream* cstreamp = cdrValueChunkStream::downcast(&stream);

      if (!cstreamp) {
	// Can't truncate if it's not a chunked stream.
	OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
		      (CORBA::CompletionStatus)stream.completion());
      }
      cdrValueChunkStream& cstream = *cstreamp;
      CORBA::Long level = cstream.nestLevel();

      PyObject* nested;

      while (cstream.skipToNestedValue(level)) {
	// Unmarshal a nested value, in case there's a later indirection to it.

	try {
	  nested = omniPy::unmarshalPyObjectValue(cstream,
					       omniPy::pyCORBAValueBaseDesc);
	  Py_DECREF(nested);
	}
	catch (CORBA::MARSHAL& ex) {
	  omniORB::logs(25,"Ignore MARSHAL exception while truncating value.");
	}
      }
    }
  }
  catch (...) {
    Py_DECREF(repoId);
    Py_XDECREF(instance);
    throw;
  }
  Py_DECREF(repoId);
  return instance;
}

static void
unmarshalMembers(cdrStream& stream, PyObject* desc,
		 PyObject* instance, PyObject* member_list)
{
  PyObject* t_o = PyTuple_GET_ITEM(desc, 6);
  if (PyTuple_Check(t_o))
    unmarshalMembers(stream, t_o, instance, member_list);

  int members = (PyTuple_GET_SIZE(desc) - 7) / 3;

  PyObject* name;
  PyObject* value;

  int i, j;

  for (i=0,j=7; i < members; i++, j+=3) {
    name  = PyTuple_GET_ITEM(desc, j);
    value = omniPy::unmarshalPyObject(stream, PyTuple_GET_ITEM(desc, j+1));

    if (member_list)
      PyList_Append(member_list, value);

    if (PyObject_SetAttr(instance, name, value) == -1) {
      // Error setting attribute, probably because the object has a
      // __setattr__ or __slots__.
      Py_DECREF(value);
      omniPy::handlePythonException();
    }
    Py_DECREF(value);
  }
}


PyObject*
omniPy::
unmarshalPyObjectValue(cdrStream& stream, PyObject* d_o)
{
  CORBA::ULong tag; // Spec says Long but it's easier to deal with ULong
  tag <<= stream;

  if (tag == 0) {
    // nil
    Py_INCREF(Py_None);
    return Py_None;
  }

  if (!stream.valueTracker()) {
    stream.valueTracker(new pyInputValueTracker());
  }

  pyInputValueTracker* tracker = (pyInputValueTracker*)stream.valueTracker();

  OMNIORB_ASSERT(tracker->valid());

  PyObject* result;
  CORBA::Long pos = stream.currentInputPtr();

  if (tag == 0xffffffff) {
    // indirection
    CORBA::Long offset;
    offset <<= stream;

    if (offset >= -4) {
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIndirection,
		    (CORBA::CompletionStatus)stream.completion());
    }
    result = tracker->lookup(pos + offset,
			     (CORBA::CompletionStatus)stream.completion());
    tracker->add(result, pos - 4);
    return result;
  }
  if (tag < 0x7fffff00 || tag > 0x7fffffff) {
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidValueTag,
		  (CORBA::CompletionStatus)stream.completion());
  }

  cdrValueChunkStream* cstreamp = cdrValueChunkStream::downcast(&stream);
  if (tag & CHUNKED) {
    if (cstreamp) {
      result = real_unmarshalPyObjectValue(stream, cstreamp, d_o, tag, pos-4);
    }
    else {
      cdrValueChunkStream cstream(stream);
      try {
	cstream.initialiseInput();
	result = real_unmarshalPyObjectValue(cstream, &cstream,
					     d_o, tag, pos-4);
      }
      catch (...) {
	cstream.exceptionOccurred();
	throw;
      }
    }
  }
  else {
    // Not chunked. Make sure we're not in a chunk stream
    if (cstreamp) {
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
		    (CORBA::CompletionStatus)stream.completion());
    }
    else {
      result = real_unmarshalPyObjectValue(stream, 0, d_o, tag, pos-4);
    }
  }
  return result;
}



PyObject*
omniPy::
copyArgumentValue(PyObject* d_o, PyObject* a_o,
		  CORBA::CompletionStatus compstatus)
{
  // *** This code does not actually copy the value. Normally
  // valuetype arguments are not "copied" using this code. Instead,
  // all the arguments are marshalled together to properly handle
  // cross-references between arguments. This code is only used if the
  // copyValuesInLocalCalls parameter is set false, meaning we have
  // chosen to use the wrong copy semantics, so we might as well do
  // the fastest thing.

  Py_INCREF(a_o);
  return a_o;
}

PyObject*
omniPy::
copyArgumentValueBox(PyObject* d_o, PyObject* a_o,
		     CORBA::CompletionStatus compstatus)
{
  // Same comment as copyArgumentValue above.

  Py_INCREF(a_o);
  return a_o;
}

