// -*- Mode: C++; -*-
//                            Package   : omniORB
// valueType.cc               Created on: 2003/09/17
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2003-2004 Apasphere Ltd.
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
//
// Description:
//    Core valuetype marshalling.
//

/*
  $Log$
  Revision 1.1.2.5  2004/07/26 22:56:39  dgrisby
  Support valuetypes in Anys.

  Revision 1.1.2.4  2004/07/23 10:29:58  dgrisby
  Completely new, much simpler Any implementation.

  Revision 1.1.2.3  2004/07/04 23:53:37  dgrisby
  More ValueType TypeCode and Any support.

  Revision 1.1.2.2  2004/02/16 10:10:30  dgrisby
  More valuetype, including value boxes. C++ mapping updates.

  Revision 1.1.2.1  2003/09/26 16:12:54  dgrisby
  Start of valuetype support.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/valueType.h>
#include <omniORB4/anyStream.h>
#include <valueTrackerImpl.h>
#include <unknownValue.h>

//
// Tag constants
//

#define CODEBASE_URL  1
#define REPOID_SINGLE 2
#define REPOID_LIST   6
#define REPOID_MASK   6
#define CHUNKED       8

OMNI_USING_NAMESPACE(omni)


static void
marshalIndirection(cdrStream& stream, CORBA::Long pos)
{
  stream.declareArrayLength(omni::ALIGN_4, 8);
  CORBA::ULong indirect = 0xffffffff;
  indirect >>= stream;

  CORBA::Long offset = pos - stream.currentOutputPtr();
  OMNIORB_ASSERT(offset < -4);

  offset >>= stream;
}


static void
marshalHeaderAndBody(cdrStream& stream, cdrValueChunkStream* cstreamp,
		     OutputValueTracker* tracker,
		     const CORBA::ValueBase* val, CORBA::Long tag,
		     const char* valRepoId, CORBA::ULong valRepoIdHash,
		     const _omni_ValueIds* truncBases);

static CORBA::ValueBase*
unmarshalHeaderAndBody(cdrStream& stream, cdrValueChunkStream* cstreamp,
		       InputValueTracker* tracker, CORBA::Long pos,
		       CORBA::ULong tag, const char* targetId,
		       CORBA::ULong targetHash, CORBA::TypeCode_ptr tc);

void
omniValueType::
marshal(CORBA::ValueBase* val, const char* repoId, cdrStream& stream)
{
  CORBA::Long tag;

  if (val == 0) { // Nil
    tag = 0;
    tag >>= stream;
    return;
  }

  cdrAnyMemoryStream* astream = cdrAnyMemoryStream::downcast(&stream);
  if (astream) {
    // Value is being marshalled into an Any. We can't marshal it into
    // the Any's memory stream since that could break value sharing
    // semantics, so we put it into the any stream's list of values.
    // In the stream, we store the value's index in the array plus 1
    // (since zero is used for nil values).
    omniTypedefs::ValueBaseSeq& seq = astream->valueSeq();
    tag = seq.length() + 1;
    seq.length(tag);
    val->_add_ref();
    seq[tag - 1] = val;
    tag >>= stream;
    return;
  }

  if (!stream.valueTracker()) {
    stream.valueTracker(new OutputValueTracker());
  }
  OutputValueTracker* tracker = (OutputValueTracker*)stream.valueTracker();
  OMNIORB_ASSERT(tracker->valid());

  stream.alignOutput(omni::ALIGN_4);

  CORBA::Long pos = tracker->addValue(val, stream.currentOutputPtr());

  if (pos != -1) {
    marshalIndirection(stream, pos);
    return;
  }

  tag = 0x7fffff00;

  CORBA::ULong valRepoIdHash;
  const char* valRepoId = val->_NP_repositoryId(valRepoIdHash);

  const _omni_ValueIds* valTruncIds;

  if (omni::ptrStrMatch(repoId, valRepoId)) {
    // Value matches IDL / TypeCode type

    if (tracker->inTruncatable()) {
      // Nested inside a truncatable value, we always send the repoId(s)
      // in case the receiver truncates, then has a later indirection.

      valTruncIds = val->_NP_truncatableIds();

      if (valTruncIds)
	tag |= REPOID_LIST;
      else
	tag |= REPOID_SINGLE;
    }
    else {
      if (valRepoId[0] == 'R' &&
	  valRepoId[1] == 'M' &&
	  valRepoId[2] == 'I' &&
	  valRepoId[3] == ':') {
	// RMI: repository ids must always be sent
	tag |= REPOID_SINGLE;
      }
    }
  }
  else {
    // Value is derived from IDL / TypeCode type
    valTruncIds = val->_NP_truncatableIds();
    if (valTruncIds)
      tag |= REPOID_LIST;
    else
      tag |= REPOID_SINGLE;
  }

  // Chunking?
  cdrValueChunkStream* cstreamp = cdrValueChunkStream::downcast(&stream);

  if (cstreamp ||
      (tag & REPOID_MASK) == REPOID_LIST ||
      val->_NP_custom()) {

    tag |= CHUNKED;
  }

  if ((tag & CHUNKED) && !cstreamp) {
    // Create a new chunk stream
    cdrValueChunkStream cstream(stream);
    try {
      marshalHeaderAndBody(cstream, &cstream, tracker, val, tag,
			   valRepoId, valRepoIdHash, valTruncIds);
    }
    catch (...) {
      cstream.exceptionOccurred();
      throw;
    }
  }
  else {
    marshalHeaderAndBody(stream, cstreamp, tracker, val, tag,
			 valRepoId, valRepoIdHash, valTruncIds);
  }
}

static void
marshalHeaderAndBody(cdrStream& stream, cdrValueChunkStream* cstreamp,
		     OutputValueTracker* tracker,
		     const CORBA::ValueBase* val, CORBA::Long tag,
		     const char* valRepoId, CORBA::ULong valRepoIdHash,
		     const _omni_ValueIds* valTruncIds)
{
  if (cstreamp)
    cstreamp->startOutputValueHeader(tag);
  else
    tag >>= stream;

  CORBA::Long idflags = tag & REPOID_MASK;

  // Marshal repoId(s) if necessary
  if (idflags == REPOID_LIST) {
    OMNIORB_ASSERT(valTruncIds);

    CORBA::Long pos = tracker->addRepoIds(valTruncIds,
					  stream.currentOutputPtr());

    if (pos == -1) {
      valTruncIds->idcount >>= stream;
      for (CORBA::Long i=0; i < valTruncIds->idcount; i++) {
	_omni_ValueId* b = &(valTruncIds->repoIds[i]);
	pos = tracker->addRepoId(b->repoId, b->hashval,
				 stream.currentOutputPtr());
	if (pos == -1)
	  stream.marshalRawString(b->repoId);
	else
	  marshalIndirection(stream, pos);
      }
    }
    else {
      marshalIndirection(stream, pos);
    }
  }
  else if (idflags == REPOID_SINGLE) {
    OMNIORB_ASSERT(valRepoId);

    CORBA::Long pos = tracker->addRepoId(valRepoId, valRepoIdHash,
					 stream.currentOutputPtr());
    if (pos == -1)
      stream.marshalRawString(valRepoId);
    else
      marshalIndirection(stream, pos);
  }

  // Finally, we're ready to marshal the members
  if (cstreamp)
    cstreamp->startOutputValueBody();

  if (idflags == REPOID_LIST)
    tracker->startTruncatable();

  val->_PR_marshal_state(stream);

  if (idflags == REPOID_LIST)
    tracker->endTruncatable();

  if (cstreamp)
    cstreamp->endOutputValue();
}


static const char*
unmarshalRepoId(cdrStream& stream, InputValueTracker* tracker)
{
  // Unmarshal a raw string or an indirection to one

  CORBA::ULong len; len <<= stream;
  CORBA::Long  pos = stream.currentInputPtr();

  if (len == 0xffffffff) {
    CORBA::Long offset;
    offset <<= stream;
    if (offset >= -4) {
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIndirection,
		    (CORBA::CompletionStatus)stream.completion());
    }
    return tracker->lookupRepoId(pos + offset, pos - 4,
			   (CORBA::CompletionStatus)stream.completion());
  }
  if (!stream.checkInputOverrun(1, len))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)stream.completion());

  char* s = _CORBA_String_helper::alloc(len - 1);
  stream.get_octet_array((_CORBA_Octet*)s, len);

  if (s[len-1] != '\0')
    OMNIORB_THROW(MARSHAL,MARSHAL_StringNotEndWithNull,
		  (CORBA::CompletionStatus)stream.completion());

  tracker->addRepoId(s, pos-4);
  // The tracker now owns the string.

  return s;
}


CORBA::ValueBase*
omniValueType::
unmarshal(const char* repoId, CORBA::ULong hashval,
	  CORBA::TypeCode_ptr tc, cdrStream& stream)
{
  CORBA::ULong tag; // tag is really Long, but ULong is easier to handle
  tag <<= stream;

  if (tag == 0) {
    // nil
    return 0;
  }

  cdrAnyMemoryStream* astream = cdrAnyMemoryStream::downcast(&stream);
  if (astream) {
    // Value is in Any's value sequence. Tag is offset in sequence plus 1.
    omniTypedefs::ValueBaseSeq& seq = astream->valueSeq();
    CORBA::ValueBase* val = seq[tag - 1];
    val->_add_ref();
    return val;
  }

  if (!stream.valueTracker()) {
    stream.valueTracker(new InputValueTracker());
  }

  InputValueTracker* tracker = (InputValueTracker*)stream.valueTracker();
  OMNIORB_ASSERT(tracker->valid());

  CORBA::ValueBase* result;
  CORBA::Long pos = stream.currentInputPtr();

  if (tag == 0xffffffff) {
    // indirection
    CORBA::Long offset;
    offset <<= stream;

    if (offset >= -4) {
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidIndirection,
		    (CORBA::CompletionStatus)stream.completion());
    }
    result = tracker->lookupValue(pos + offset, pos-4,
				 (CORBA::CompletionStatus)stream.completion());
    CORBA::add_ref(result);
    return result;
  }
  if (tag < 0x7fffff00 || tag > 0x7fffffff) {
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidValueTag,
		  (CORBA::CompletionStatus)stream.completion());
  }

  cdrValueChunkStream* cstreamp = cdrValueChunkStream::downcast(&stream);
  if (tag & CHUNKED) {
    if (cstreamp) {
      result = unmarshalHeaderAndBody(stream, cstreamp, tracker,
				      pos-4, tag, repoId, hashval, tc);
    }
    else {
      cdrValueChunkStream cstream(stream);
      try {
	cstream.initialiseInput();
	result = unmarshalHeaderAndBody(cstream, &cstream, tracker,
					pos-4, tag, repoId, hashval, tc);
      }
      catch (...) {
	cstream.exceptionOccurred();
	throw;
      }
    }
  }
  else {
    // Not chunked.
    if (cstreamp) {
      // All values inside a chunk stream must be chunked.
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
		    (CORBA::CompletionStatus)stream.completion());
    }
    else {
      result = unmarshalHeaderAndBody(stream, 0, tracker, pos-4, tag,
				      repoId, hashval, tc);
    }
  }
  return result;
}


static
CORBA::ValueBase*
unmarshalHeaderAndBody(cdrStream&           stream,
		       cdrValueChunkStream* cstreamp,
		       InputValueTracker*   tracker,
		       CORBA::Long          pos,
		       CORBA::ULong         tag,
		       const char*          targetId,
		       CORBA::ULong         targetHash,
		       CORBA::TypeCode_ptr  tc)
{
  if (tag & CODEBASE_URL) { // Skip the codebase URL
    CORBA::ULong length;
    length <<= stream;
    if (length == 0xfffffff) // Indirection
      length <<= stream;
    else
      stream.skipInput(length);
  }

  CORBA::Boolean truncating = 0;
  const char* repoId;
  const _omni_ValueIds* repoIds = 0;

  CORBA::ValueBase* result;

  if ((tag & REPOID_MASK) == REPOID_LIST) {
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
      repoIds = tracker->lookupRepoIds(idpos + offset, idpos - 4,
				 (CORBA::CompletionStatus)stream.completion());
    }
    else {
      if (!stream.checkInputOverrun(4, count, omni::ALIGN_4)) {
	OMNIORB_THROW(MARSHAL, MARSHAL_SequenceIsTooLong,
		      (CORBA::CompletionStatus)stream.completion());
      }
      _omni_ValueIds* newIds = new _omni_ValueIds;
      _omni_ValueId* idList  = new _omni_ValueId[count];
      for (CORBA::ULong i=0; i < count; i++) {
	idList[i].repoId  = unmarshalRepoId(stream, tracker);
	idList[i].hashval = omniValueType::hash_id(idList[i].repoId);
      }
      newIds->idcount = count;
      newIds->repoIds = idList;
      newIds->hashval = omniValueType::hash_ids(newIds);
      tracker->addRepoIds(newIds, idpos-4);
      repoIds = newIds;
    }

    // Try to find a factory that can create the value.
    CORBA::Long i;
    for (i=0; i < repoIds->idcount; i++) {
      repoId = repoIds->repoIds[i].repoId;
      result = _omni_ValueFactoryManager::
	             create_for_unmarshal(repoId, repoIds->repoIds[i].hashval);

      if (result) {
	break;
      }
      else if (omni::ptrStrMatch(repoId, targetId)) {
	// We've reached the target id without finding a factory.
	// Break out here and raise MARSHAL below.
	break;
      }
    }
    if (i > 0)
      truncating = 1;
  }
  else if ((tag & REPOID_MASK) == REPOID_SINGLE) {
    repoId = unmarshalRepoId(stream, tracker);
    CORBA::ULong hashval = omniValueType::hash_id(repoId);
    result = _omni_ValueFactoryManager::create_for_unmarshal(repoId, hashval);
  }
  else {
    repoId = targetId;

    // No repoId marshalled -- it's the target type
    if (omni::ptrStrMatch(targetId, CORBA::ValueBase::_PD_repoId)) {
      // If we're expecting ValueBase, the sender must specify a repoId.
      OMNIORB_THROW(MARSHAL, MARSHAL_NoRepoIdInValueType,
		    (CORBA::CompletionStatus)stream.completion());
    }
    result = _omni_ValueFactoryManager::create_for_unmarshal(targetId,
							     targetHash);
  }

  // After all that, did we manage to create a value?
  if (!result) {

    if (tc && !CORBA::is_nil(tc) && omni::ptrStrMatch(repoId, tc->id())) {
      // Value is inside an Any so we create an UnknownValue. The
      // application won't be able to do anything with it except put
      // it in a DynAny or pass it on inside its Any.
      result = new UnknownValue(tc);
    }
    else {
      OMNIORB_THROW(MARSHAL, MARSHAL_NoValueFactory,
		    (CORBA::CompletionStatus)stream.completion());
    }
  }

  // If the value is chunked, tell the chunk stream we're about to
  // unmarshal the value body.
  if (cstreamp)
    cstreamp->startInputValueBody();

  tracker->addValue(result, pos);

  // Tell the value to unmarshal its members
  result->_PR_unmarshal_state(stream);

  // Skip remaining parts if we're truncating.
  if (truncating) {
    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "Truncating input value from '"
	<< repoIds->repoIds[0].repoId << "' to '" << repoId << "'.\n";
    }

    if (!cstreamp) {
      // Can't truncate if it's not a chunked stream.
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
		    (CORBA::CompletionStatus)stream.completion());
    }
    cdrValueChunkStream& cstream = *cstreamp;
    CORBA::Long level = cstream.nestLevel();

    CORBA::ValueBase* nested;

    while (cstream.skipToNestedValue(level)) {
      // Unmarshal a nested value, in case there's a later indirection to it.

      try {
	nested = omniValueType::unmarshal(CORBA::ValueBase::_PD_repoId,
					  0, 0, stream);
	CORBA::remove_ref(nested);
      }
      catch (CORBA::MARSHAL& ex) {
	omniORB::logs(25, "Ignore MARSHAL exception while truncating value.");
      }
    }
  }

  // The tracker holds the single reference to the value. Add one for
  // our caller.
  CORBA::add_ref(result);
  return result;
}
