// anyP.h                     Created on: 04/08/98
//                            Author    : James Weatherall (jnw)
//
//
//    Copyright (C) 1998 Olivetti & Oracle Research Laboratory
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


#include <omniORB2/CORBA.h>
#include <tcParser.h>


class AnyP {
public:

  // CREATION/DESTRUCTION

  AnyP(CORBA::TypeCode_ptr tc);
  // Constructor.  Creates a membufferedstream and duplicates & saves
  // the typecode passed in.

  AnyP(CORBA::TypeCode_ptr tc, void * value, CORBA::Boolean release);
  // Constructor.  Creates a read-only membufferedstream & duplicates &
  // saves the typecode.  The stream uses the given value to read from.
  // NEVER CALL THIS FUNCTION WITH A NULL value POINTER!

  AnyP(const AnyP *existing);
  // Constructor.  Copies the typecode from the supplied AnyP and copies
  // the MBufferedStream data into the new internal stream.

  ~AnyP();

  // METHODS

  inline CORBA::Boolean getData(CORBA::TypeCode_ptr tc, tcDescriptor &data) {
    if (!tc->equal(pd_parser->getTC()))
      return 0;
    pd_parser->copyTo(data);
    return 1;
  }
  // Routine to check the TypeCode and get the data from the mbuf.

  inline CORBA::Boolean getObjRef(tcDescriptor& data) {
    if( pd_parser->getTC()->kind() != CORBA::tk_objref )  return 0;
    pd_parser->copyTo(data);
    return 1;
  }
  // Routine to extract an object reference from the Any. Checks that we
  // contain an object ref, but not that the interface repo id is correct.
  // This is done further down the line ...

  inline void setData(CORBA::TypeCode_ptr tc, tcDescriptor &data) {
    setTC_and_reset(tc);
    pd_parser->copyFrom(data);
  }
  // Routine to change the TypeCode and fill the mbuf from the
  // given tcDescriptor.

  inline void setTC_and_reset(CORBA::TypeCode_ptr tc) {
    // Free the parser & buffer
    if (pd_parser != 0)  delete pd_parser;
    if (pd_mbuf != 0)    delete pd_mbuf;
    if (pd_releaseptr)   delete [] (char *) pd_dataptr;
    if (pd_cached_data_ptr != 0)
      pd_cached_data_destructor(pd_cached_data_ptr);
    pd_cached_data_ptr = 0;

    // Allocate a new membuffered stream & parser
    pd_mbuf = new MemBufferedStream();
    pd_releaseptr = 0;
    pd_parser = new tcParser(*pd_mbuf, tc);
  }
  // Change the TypeCode and reallocate the mbuf.

  inline tcParser *getTC_parser() { return pd_parser; }
  // Get a tcParser through which to manipulate the data.

  inline const void *getBuffer() {
    pd_mbuf->rewind_in_mkr();
    return pd_mbuf->data();
  }
  // Return the start of the data buffer.

  inline MemBufferedStream& getMemBufferedStream() { return *pd_mbuf; }
  // Get the internal stream.

  inline size_t alignedSize(size_t io) {
    return pd_parser->alignedSize(io);
  }
  // Return the size of the data contained in the buffer.

  inline void *getCachedData() { return pd_cached_data_ptr; }
  // Retrieve the cached data pointer.

  inline void setCachedData(void *data, void (*destructor)(void *)) {
    pd_cached_data_ptr = data;
    pd_cached_data_destructor = destructor;
  }
  // Set the cached data pointer and provide a destructor function.

private:
  // PRIVATE DATA

  MemBufferedStream* pd_mbuf;
  tcParser* pd_parser;

  // If the AnyP was created using a void* pointer and TypeCode.
  void * pd_dataptr;
  CORBA::Boolean pd_releaseptr;

  // Cached data pointer and a destructor for it. This is required
  // because when unmarshalling complex structures from Anys, the
  // Any has to handle the storage!
  void* pd_cached_data_ptr;
  void (*pd_cached_data_destructor)(void*);
};


#endif  // __ANYP_H__
