// anyP.h                     Created on: 04/08/98
//                            Author    : James Weatherall (jnw)
//
//
//    Copyright (C) 1998-1999 AT&T Laboratories Cambridge
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
//    Private class AnyP for the implementation of the Any interface.
//

#ifndef __ANYP_H__
#define __ANYP_H__

#include <tcParser.h>

OMNI_NAMESPACE_BEGIN(omni)

class AnyP {
public:

  // -=- CREATION/DESTRUCTION

  AnyP(const CORBA::TypeCode_ptr tc);
  // Constructor.  Duplicates and saves <tc>

  AnyP(const CORBA::TypeCode_ptr tc, void * value, CORBA::Boolean release);
  // Constructor.  Duplicates and saves <tc>.  Makes internal
  // mbuf read-only, linked to the supplied data <value>.
  // <value> is "owned" by anyP if <release> is set.
  // NEVER CALL THIS FUNCTION WITH A NULL value POINTER!

  AnyP(const AnyP *existing);
  // Constructor.  Copies typecode from existing anyP and
  // fills internal buffer with source anyP's contents.

  ~AnyP();

  // -=- RECREATION

  void setTC_and_reset(const CORBA::TypeCode_ptr tc);
  // Change the TypeCode and flush the buffer

  // -=- tcDescriptor MANIPULATION

  inline CORBA::Boolean getData(const CORBA::TypeCode_ptr tc, tcDescriptor &data) {
    if (!tc->equivalent(pd_tc))
      return 0;
    tcParser::copyMemStreamToTcDescriptor_rdonly(pd_tc, pd_mbuf, data);
    return 1;
  }
  // Check the TypeCode and fetch the data into a tcDescriptor

  inline CORBA::Boolean getObjRef(tcDescriptor& data) {
    if( pd_tc->kind() != CORBA::tk_objref )  return 0;
    tcParser::copyMemStreamToTcDescriptor_rdonly(pd_tc, pd_mbuf, data);
    return 1;
  }
  // Routine to extract an object reference from the Any. Checks that we
  // contain an object ref, but not that the interface repo id is correct.
  // This is done further down the line ...

  inline void setData(const CORBA::TypeCode_ptr tc, const tcDescriptor &data) {
    setTC_and_reset(tc);
    tcParser::copyTcDescriptorToMemStream_flush(pd_tc, data, pd_mbuf);
  }
  // Routine to change the TypeCode and fill the mbuf from the
  // given tcDescriptor.

  // -=- TYPECODE OPERATIONS

  inline const CORBA::TypeCode_ptr getTC() {
    return pd_tc;
  }
  // Return the actual typecode in use - NOT A DUPLICATE

  inline void replaceTC(const CORBA::TypeCode_ptr tc) {
    if (tc->equivalent(pd_tc))
      pd_tc = CORBA::TypeCode::_duplicate(tc);
    else
      throw CORBA::TypeCode::BadKind();
  }
  // If the tc is equivalent to pd_tc, use it instead of pd_tc

  // -=- BUFFER OPERATIONS

  inline const void *getBuffer() {
    return pd_mbuf.bufPtr();
  }
  // Return the start of the data buffer.

  inline cdrMemoryStream& getWRableMemoryStream() {
    pd_mbuf.rewindPtrs(); return pd_mbuf;
  }
  // Returns a reference to the internal MemoryStream.
  // NOTE: The stream is emptied before being returned.

  inline const cdrMemoryStream& theMemoryStream() {
    return pd_mbuf;
  }
  // Returns a reference to the internal MemoryStream. It may only be
  // read from, not written to.

  // -=- CACHED DATA HANDLING

  inline void *getCachedData() { return pd_cached_data_ptr; }
  // Retrieve the cached data pointer.

  inline void setCachedData(void *data, void (*destructor)(void *)) {
    pd_cached_data_ptr = data;
    pd_cached_data_destructor = destructor;
  }
  // Set the cached data pointer and provide a destructor function.

  // -=- MARSHALLING TO/FROM STREAMS

  inline void copyTo(cdrStream &s) {
    tcParser::copyMemStreamToStream_rdonly(pd_tc, pd_mbuf, s);
  }
  // Copy the internal buffer out to a stream (re-entrant)

  inline void copyFrom(cdrStream &s) {
    tcParser::copyStreamToMemStream_flush(pd_tc, s, pd_mbuf);
  }
  // Fill the internal buffer from a stream (replaces contents)

private:
  // PRIVATE DATA

  cdrMemoryStream pd_mbuf;
  CORBA::TypeCode_var pd_tc;

  // If the AnyP was created using a void* pointer and TypeCode.
  void * pd_dataptr;
  CORBA::Boolean pd_releaseptr;

  // Cached data pointer and a destructor for it. This is required
  // because when unmarshalling complex structures from Anys, the
  // Any has to handle the storage!
  void* pd_cached_data_ptr;
  void (*pd_cached_data_destructor)(void*);
};

OMNI_NAMESPACE_END(omni)

#endif  // __ANYP_H__
