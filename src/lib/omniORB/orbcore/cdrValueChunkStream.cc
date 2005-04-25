// -*- Mode: C++; -*-
//                            Package   : omniORB
// cdrValueChunkStream.cc     Created on: 2003/03/26
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2003 Apasphere Ltd.
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
//    cdrStream wrapper that implements the evils of valuetype chunked
//    encoding.
//

// $Log$
// Revision 1.1.2.7  2005/04/25 17:42:25  dgrisby
// Bug in marshalling nested chunks on a buffer boundary.
//
// Revision 1.1.2.6  2005/04/13 09:11:04  dgrisby
// peekChunkTag forgot to byteswap when it needed to.
//
// Revision 1.1.2.5  2005/01/13 21:55:56  dgrisby
// Turn off -g debugging; suppress some compiler warnings.
//
// Revision 1.1.2.4  2004/10/13 17:58:25  dgrisby
// Abstract interfaces support; values support interfaces; value bug fixes.
//
// Revision 1.1.2.3  2003/11/06 11:56:57  dgrisby
// Yet more valuetype. Plain valuetype and abstract valuetype are now working.
//
// Revision 1.1.2.2  2003/07/10 21:52:31  dgrisby
// Value chunks should start after URL / repoids.
//
// Revision 1.1.2.1  2003/05/20 16:53:16  dgrisby
// Valuetype marshalling support.
//


#include <omniORB4/CORBA.h>

#ifndef Swap32
#define Swap32(l) ((((l) & 0xff000000) >> 24) | \
		   (((l) & 0x00ff0000) >> 8)  | \
		   (((l) & 0x0000ff00) << 8)  | \
		   (((l) & 0x000000ff) << 24))
#else
#error "Swap32 has already been defined"
#endif

OMNI_USING_NAMESPACE(omni)


cdrValueChunkStream::
~cdrValueChunkStream()
{
  if (!pd_exception) {
    if (pd_reader && pd_nestLevel > 0) {
      endInputValue();
    }
    OMNIORB_ASSERT(pd_nestLevel == 0);
  }
  copyStateToActual();
  pd_valueTracker = 0;
}

ValueIndirectionTracker::~ValueIndirectionTracker()
{
}

void
cdrValueChunkStream::initialiseInput()
{
  OMNIORB_ASSERT(pd_nestLevel == 0);
  pd_reader    = 1;
  pd_nestLevel = 1;
  pd_inHeader  = 1;
}


void
cdrValueChunkStream::startOutputChunk()
{
  omniORB::logs(25, "Start writing value chunk");

  OMNIORB_ASSERT(!pd_reader);
  OMNIORB_ASSERT(!pd_inChunk);
  OMNIORB_ASSERT(pd_nestLevel > 0);

  // We have to marshal the chunk length manually, so we can keep a
  // record of its address in the buffer.
  omni::ptr_arith_t p1, p2;

  copyStateToActual();
  while (1) {
    p1 = omni::align_to((omni::ptr_arith_t)pd_actual.pd_outb_mkr,
			omni::ALIGN_4);
    p2 = p1 + sizeof(_CORBA_Long);
    if ((void*)p2 > pd_actual.pd_outb_end) {
      if (pd_actual.reserveOutputSpaceForPrimitiveType(omni::ALIGN_4,
						       sizeof(_CORBA_Long))) {
	continue;
      }
      else {
	// Cannot reserve space, most likely because this is a
	// counting stream. We cannot continue.
	OMNIORB_THROW(MARSHAL, MARSHAL_CannotReserveOutputSpace,
		      (CORBA::CompletionStatus)completion());
      }
    }
    break;
  }
  pd_actual.pd_outb_mkr = (void*)p2;
  pd_lengthPtr  = (CORBA::Long*)p1;
  *pd_lengthPtr = 0;

  copyStateFromActual();
  pd_inChunk   = 1;
  pd_justEnded = 0;
}

void
cdrValueChunkStream::endOutputChunk()
{
  OMNIORB_ASSERT(!pd_reader);
  OMNIORB_ASSERT(pd_inChunk);
  OMNIORB_ASSERT(pd_lengthPtr);
    
  // Calculate length of the chunk we're ending
  omni::ptr_arith_t start = (omni::ptr_arith_t)pd_lengthPtr + 4;
  omni::ptr_arith_t end   = (omni::ptr_arith_t)pd_outb_mkr;

  *pd_lengthPtr = (CORBA::Long)(end - start);

  OMNIORB_ASSERT(*pd_lengthPtr > 0);

  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "End writing value chunk. Length = " << *pd_lengthPtr << "\n";
  }
  pd_lengthPtr = 0;
  pd_inChunk   = 0;

  copyStateToActual();
}

void
cdrValueChunkStream::maybeStartNewChunk(omni::alignment_t align, size_t size)
{
  OMNIORB_ASSERT(!pd_reader);
  OMNIORB_ASSERT(pd_inChunk);
  OMNIORB_ASSERT(pd_lengthPtr);
    
  // Calculate length of the chunk we're ending
  omni::ptr_arith_t start = (omni::ptr_arith_t)pd_lengthPtr + 4;
  omni::ptr_arith_t end   = (omni::ptr_arith_t)pd_outb_mkr;

  *pd_lengthPtr = (CORBA::Long)(end - start);

  if (*pd_lengthPtr > 0) {
    // OK to end here
    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "End writing value chunk. Length = " << *pd_lengthPtr << "\n";
    }
    pd_lengthPtr  = 0;
    pd_inChunk    = 0;

    copyStateToActual();
    startOutputChunk();
  }
  else {
    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "Cannot end value chunk with zero length; extending to "
	<< size << " octets\n";
    }
    OMNIORB_ASSERT(size);
    chunkStreamDeclareArrayLength(align, size);
  }
}



void
cdrValueChunkStream::startOutputValueHeader(_CORBA_Long valueTag)
{
  OMNIORB_ASSERT(valueTag >= 0x7fffff00); // Valid tag range
  OMNIORB_ASSERT(valueTag &  0x00000008); // Chunked encoding flag

  if (pd_inChunk)
    endOutputChunk();

  omniORB::logs(25, "Start output value header.");

  pd_inHeader = 1;

  // Marshal value tag for new value. Since we're currently outside a
  // chunk, marshal straight into the actual stream, to prevent our
  // virtual functions running if the buffer is full.
  valueTag >>= pd_actual;
  copyStateFromActual();
}

void
cdrValueChunkStream::startOutputValueBody()
{
  OMNIORB_ASSERT(pd_inHeader);

  pd_inHeader = 0;
  ++pd_nestLevel;

  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "Start writing chunked value body. Nest level = "
      << pd_nestLevel << "\n";
  }
  pd_outb_end = pd_outb_mkr;
}


void
cdrValueChunkStream::endOutputValue()
{
  OMNIORB_ASSERT(pd_nestLevel > 0);
  OMNIORB_ASSERT(!pd_remaining);

  if (pd_inChunk)
    endOutputChunk();

  if (pd_justEnded) {
    // Ending a nested value

    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "End writing nested chunked value. Nest level = "
	<< pd_nestLevel << "\n";
    }

    CORBA::Long* endp = (CORBA::Long*)((omni::ptr_arith_t)pd_outb_mkr - 4);
    OMNIORB_ASSERT(*endp == -(pd_nestLevel + 1));
    *endp = -pd_nestLevel;
  }
  else {
    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "End writing chunked value. Nest level = " << pd_nestLevel << "\n";
    }

    // Marshal the end tag into the actual stream
    CORBA::Long endTag = -pd_nestLevel;
    endTag >>= pd_actual;
    copyStateFromActual();
  }

  --pd_nestLevel;

  // Ensure next marshal results in a call to one of our virtual
  // functions, so we can start a new chunk.
  pd_outb_end  = pd_outb_mkr;
  pd_justEnded = 1;
}


void
cdrValueChunkStream::
startInputValueBody()
{
  OMNIORB_ASSERT(pd_inHeader);
  pd_inHeader = 0;
  startInputChunk();
}


_CORBA_Boolean
cdrValueChunkStream::
reserveOutputSpaceForPrimitiveType(omni::alignment_t align, size_t required)
{
  omni::ptr_arith_t p1, p2;

  for (int i=0; i < 5; i++) {

    if (pd_remaining) {
      // Some pre-reserved octets to go before finishing a chunk
      OMNIORB_ASSERT(!pd_inChunk);
      OMNIORB_ASSERT(!pd_inHeader);

      p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr, align);
      p2 = p1 + required;
      if (p2 <= (omni::ptr_arith_t)pd_outb_end) {
	// Not got to the end of the current buffer yet
	return 1;
      }
      copyStateToActual();
      if (!pd_actual.reserveOutputSpaceForPrimitiveType(align, required))
	OMNIORB_THROW(MARSHAL, MARSHAL_CannotReserveOutputSpace,
		      (CORBA::CompletionStatus)completion());
      copyStateFromActual();
      p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr, align);
      p2 = p1 + pd_remaining;
      if (p2 > (omni::ptr_arith_t)pd_outb_end) {
	pd_remaining = p2 - (omni::ptr_arith_t)pd_outb_end;
      }
      else {
	pd_outb_end  = (void*)p2;
	pd_remaining = 0;
      }
      return 1;
    }

    if (pd_inHeader) {
      OMNIORB_ASSERT(!pd_inChunk);
      copyStateToActual();
      if (!pd_actual.reserveOutputSpaceForPrimitiveType(align, required))
	OMNIORB_THROW(MARSHAL, MARSHAL_CannotReserveOutputSpace,
		      (CORBA::CompletionStatus)completion());
      copyStateFromActual();
      return 1;
    }

    if (!pd_inChunk) {
      // Start a new chunk
      OMNIORB_ASSERT(pd_nestLevel);
      OMNIORB_ASSERT(pd_lengthPtr == 0);
      startOutputChunk();
    }

    p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr, align);
    p2 = p1 + required;

    if (p2 <= (omni::ptr_arith_t)pd_outb_end) {
      // Enough space
      return 1;
    }
    maybeStartNewChunk(align, required);
  }
  // If we've been round five times without getting enough space, the
  // stream must be being awkward and allocating the precise amount we
  // ask it for each time. We could choose to preset the chunk length
  // to the required size, meaning we don't have to go back later to
  // end it. There's not much point, though, since doing that would
  // end up with one chunk per primitive type, which is so inefficient
  // as to be ridiculous. We throw a MARSHAL exception.
  OMNIORB_THROW(MARSHAL, MARSHAL_CannotReserveOutputSpace,
		(CORBA::CompletionStatus)completion());
  return 0;
}


_CORBA_Boolean
cdrValueChunkStream::
maybeReserveOutputSpace(omni::alignment_t align, size_t required)
{
  omni::ptr_arith_t p1, p2;

  if (pd_remaining) {
    // Some pre-reserved octets to go before finishing a chunk
    OMNIORB_ASSERT(!pd_inChunk);
    OMNIORB_ASSERT(!pd_inHeader);

    p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr, align);
    p2 = p1 + required;
    if (p2 <= (omni::ptr_arith_t)pd_outb_end) {
      // Not got to the end of the current buffer yet
      return 1;
    }
    copyStateToActual();
    if (!pd_actual.maybeReserveOutputSpace(align, required))
      OMNIORB_THROW(MARSHAL, MARSHAL_CannotReserveOutputSpace,
		    (CORBA::CompletionStatus)completion());
    copyStateFromActual();
    p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr, align);
    p2 = p1 + pd_remaining;
    if (p2 > (omni::ptr_arith_t)pd_outb_end) {
      pd_remaining = p2 - (omni::ptr_arith_t)pd_outb_end;
    }
    else {
      pd_outb_end  = (void*)p2;
      pd_remaining = 0;
    }
    return 1;
  }

  if (pd_inHeader) {
    OMNIORB_ASSERT(!pd_inChunk);
    copyStateToActual();
    CORBA::Boolean r = pd_actual.maybeReserveOutputSpace(align, required);
    copyStateFromActual();
    return r;
  }

  if (!pd_inChunk) {
    // Start a new chunk
    OMNIORB_ASSERT(pd_nestLevel);
    OMNIORB_ASSERT(pd_lengthPtr == 0);
    startOutputChunk();
  }

  p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr, align);
  p2 = p1 + required;

  if (p2 <= (omni::ptr_arith_t)pd_outb_end) {
    // Enough space already
    return 1;
  }

  omni::ptr_arith_t start = (omni::ptr_arith_t)pd_lengthPtr + 4;
  *pd_lengthPtr = (CORBA::Long)(p2 - start);
  pd_remaining  = required;
  pd_outb_mkr   = pd_outb_end = (void*)p2;

  // The call to maybeReserveOutputSpace can either succeed, in which
  // case we set our pointers to allow the caller to fill the reserved
  // space, or it can throw BAD_PARAM, in which case our pointers and
  // pd_remaining are set correctly for the caller to fill the stream
  // some other way.
  if (!pd_actual.maybeReserveOutputSpace(align, required))
    OMNIORB_THROW(MARSHAL, MARSHAL_CannotReserveOutputSpace,
		  (CORBA::CompletionStatus)completion());

  copyStateFromActual();
  p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr, align);
  p2 = p1 + required;

  OMNIORB_ASSERT(p2 <= (omni::ptr_arith_t)pd_outb_end);

  pd_outb_end  = (void*)p2;
  pd_remaining = 0;

  return 1;
}

void
cdrValueChunkStream::
put_octet_array(const _CORBA_Octet* b, int size, omni::alignment_t align)
{
  omni::ptr_arith_t p1, p2;

  if (pd_remaining) {
    OMNIORB_ASSERT(!pd_inChunk);
    OMNIORB_ASSERT(!pd_inHeader);

    p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr, align);
    p2 = p1 + size;

    if (p2 > (omni::ptr_arith_t)pd_outb_end + (omni::ptr_arith_t)pd_remaining){
      // Not permitted to put an array longer that the reserved chunk size
      OMNIORB_THROW(MARSHAL, MARSHAL_MessageTooLong,
		    (CORBA::CompletionStatus)completion());
    }
    if (p2 <= (omni::ptr_arith_t)pd_outb_end) {
      memcpy((void*)p1, (const void*)b, size);
      pd_outb_mkr = (void*)p2;
    }
    else {
      copyStateToActual();
      pd_actual.put_octet_array(b, size, align);
      pd_remaining -= (p2 - (omni::ptr_arith_t)pd_outb_end);
      copyStateFromActual();
      pd_outb_end = pd_outb_mkr;
    }
    return;
  }

  if (pd_inHeader) {
    OMNIORB_ASSERT(!pd_inChunk);
    copyStateToActual();
    pd_actual.put_octet_array(b, size, align);
    copyStateFromActual();
    return;
  }

  if (!pd_inChunk) {
    // Start a new chunk
    OMNIORB_ASSERT(pd_nestLevel);
    OMNIORB_ASSERT(pd_lengthPtr == 0);
    startOutputChunk();
  }

  p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr, align);
  p2 = p1 + size;

  if (p2 <= (omni::ptr_arith_t)pd_outb_end) {
    // Enough space in buffer
    memcpy((void*)p1, (const void*)b, size);
    pd_outb_mkr = (void*)p2;
  }
  else {
    // End the chunk, setting its length to include the octet array
    p1 = (omni::ptr_arith_t)pd_lengthPtr + 4;
    OMNIORB_ASSERT(p1 < p2);
    *pd_lengthPtr = (CORBA::Long)(p2 - p1);

    pd_lengthPtr = 0;
    pd_inChunk   = 0;

    copyStateToActual();
    pd_actual.put_octet_array(b, size, align);
    copyStateFromActual();

    // Make sure the next insertion causes a new chunk to start.
    pd_outb_end = pd_outb_mkr;
  }
}

void
cdrValueChunkStream::
chunkStreamDeclareArrayLength(omni::alignment_t align, size_t size)
{
  if (pd_inHeader)
    return;

  if (!pd_inChunk) {
    // Start a new chunk
    OMNIORB_ASSERT(pd_nestLevel);
    OMNIORB_ASSERT(pd_lengthPtr == 0);
    startOutputChunk();
  }

  omni::ptr_arith_t start, cur, end;
  cur = omni::align_to((omni::ptr_arith_t)pd_outb_mkr, align);
  end = cur + size;

  if (end <= (omni::ptr_arith_t)pd_outb_end) {
    // Enough space in buffer
    return;
  }
  else {
    // End the chunk, setting its length to include the array
    pd_outb_mkr = (void*)cur;
    start = (omni::ptr_arith_t)pd_lengthPtr + 4;
    OMNIORB_ASSERT(start < end);
    *pd_lengthPtr = (CORBA::Long)(end - start);

    // Number of octets remainig is the number we required, minus the
    // number we can fit into the current buffer with the required
    // alignment.
    omni::ptr_arith_t mask = ~((ptr_arith_t)align - 1);
    pd_remaining = end - ((omni::ptr_arith_t)pd_outb_end & mask);

    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "End writing value chunk inside declareArrayLength. Length = "
	<< *pd_lengthPtr << ", remaining = " << pd_remaining << "\n";
    }
    pd_lengthPtr = 0;
    pd_inChunk   = 0;
  }
}


void
cdrValueChunkStream::
fetchInputData(omni::alignment_t align,size_t required)
{
  OMNIORB_ASSERT(pd_reader);

  while (1) {
    omni::ptr_arith_t p1, p2;
    p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr, align);
    p2 = p1 + required;
    if (p2 <= (omni::ptr_arith_t)pd_inb_end)
      return;

    if (pd_inHeader) {
      OMNIORB_ASSERT(!pd_inChunk);
      copyStateToActual();
      pd_actual.fetchInputData(align, required);
      copyStateFromActual();
      return;
    }

    if (pd_inChunk) {
      if (pd_remaining) {
	// More data to come in this chunk
	OMNIORB_ASSERT(pd_inb_end == pd_actual.pd_inb_end);

	copyStateToActual();
	pd_actual.fetchInputData(align, required);
	copyStateFromActual();

	p1 = (omni::ptr_arith_t)pd_inb_mkr;
	p2 = (omni::ptr_arith_t)pd_inb_end;

	if ((omni::ptr_arith_t)pd_remaining > (p2 - p1)) {
	  pd_remaining = pd_remaining - (p2 - p1);
	}
	else {
	  pd_inb_end   = (void*)(p1 + pd_remaining);
	  pd_remaining = 0;
	}
	return;
      }
      else {
	// End of chunk. Peek into the stream to see what comes next.
	CORBA::Long tag = peekChunkTag();
	if (tag < 0) {
	  // End of one or more values
	  endInputValue();
	}
	else if (tag == 0) {
	  // Zero value is reserved for future use.
	  OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
			(CORBA::CompletionStatus)completion());
	}
	else if (tag < 0x7fffff00) {
	  // Start of another chunk in this value
	  startInputChunk();
	}
	else {
	  // Start tag for a new value. Set the stream pointers so the
	  // caller can retrieve just the tag before we are called
	  // again.
	
	  // We should only be in this situation if the caller is asking
	  // for 4 octets, since they should be reading a tag.
	  if (required != 4)
	    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
			  (CORBA::CompletionStatus)completion());

	  p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr, align);
	  p2 = p1 + required;

	  // Given that we have already read the tag value, there _must_
	  // be enough space in the stream.
	  OMNIORB_ASSERT(p2 <= (omni::ptr_arith_t)pd_inb_end);
	  pd_inb_end = (void*)p2;

	  pd_inChunk  = 0;
	  pd_inHeader = 1;
	  pd_nestLevel++;
	  return;
	}
      }
    }
    else {
      startInputChunk();
    }
  }
}


_CORBA_Boolean
cdrValueChunkStream::
skipToNestedValue(_CORBA_Long level)
{
  omni::ptr_arith_t p1, p2;

  OMNIORB_ASSERT(!pd_inHeader);

  while (1) {
    if (pd_nestLevel < level) {
      return 0;
    }
    pd_inb_mkr = pd_inb_end;
    if (pd_inChunk) {
      if (pd_remaining) {
	// More data to come in this chunk
	copyStateToActual();
	pd_actual.fetchInputData(omni::ALIGN_1, 1);
	copyStateFromActual();
	
	p1 = (omni::ptr_arith_t)pd_inb_mkr;
	p2 = (omni::ptr_arith_t)pd_inb_end;

	if ((omni::ptr_arith_t)pd_remaining > (p2 - p1)) {
	  pd_remaining = pd_remaining - (p2 - p1);
	}
	else {
	  pd_inb_end   = (void*)(p1 + pd_remaining);
	  pd_remaining = 0;
	}
      }
      else {
	// End of chunk. Peek into the stream to see what comes next.
	CORBA::Long tag = peekChunkTag();
	if (tag < 0) {
	  // End of one or more values
	  endInputValue();
	}
	else if (tag == 0) {
	  // Zero value is reserved for future use.
	  OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
			(CORBA::CompletionStatus)completion());
	}
	else if (tag < 0x7fffff00) {
	  // Start of another chunk in this value
	  startInputChunk();
	}
	else {
	  p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr, omni::ALIGN_4);
	  p2 = p1 + 4;

	  // Given that we have already read the tag value, there _must_
	  // be enough space in the stream.
	  OMNIORB_ASSERT(p2 <= (omni::ptr_arith_t)pd_inb_end);
	  pd_inb_end = (void*)p2;

	  pd_inChunk  = 0;
	  pd_inHeader = 1;
	  pd_nestLevel++;
	  return 1;
	}
      }
    }
    else {
      startInputChunk();
    }
  }
}



void
cdrValueChunkStream::
startInputChunk()
{
  CORBA::Long len = peekChunkTag();

  if (len <= 0)
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
		  (CORBA::CompletionStatus)completion());

  if (len >= 0x7fffff00) {
    // It's not the start of a chunk at all, but a nested value. Treat
    // it as if it was a zero-length chunk.
    len = 0;
    omniORB::logs(25, "Receive nested value instead of chunk length.");
  }
  else {
    len <<= pd_actual;

    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "Start reading value chunk. Length = " << len << "\n";
    }
  }
  copyStateFromActual();

  omni::ptr_arith_t start = (omni::ptr_arith_t)pd_inb_mkr;
  omni::ptr_arith_t end   = (omni::ptr_arith_t)pd_inb_end;

  if ((omni::ptr_arith_t)len > (end - start)) {
    pd_remaining = len - (end - start);
  }
  else {
    pd_inb_end   = (void*)(start + len);
    pd_remaining = 0;
  }
  pd_inChunk = 1;
}

void
cdrValueChunkStream::
endInputValue()
{
  copyStateToActual();
  CORBA::Long tag;
  tag <<= pd_actual;
  copyStateFromActual();

  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "End reading value chunk. Nest level = " << -tag << "\n";
  }

  if (tag >= 0)
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
		  (CORBA::CompletionStatus)completion());
  tag = -tag - 1;
  
  if (tag >= pd_nestLevel)
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
		  (CORBA::CompletionStatus)completion());

  pd_nestLevel = tag;
  pd_inChunk   = 0;
  pd_inb_end   = pd_inb_mkr;
}

_CORBA_Long
cdrValueChunkStream::
peekChunkTag()
{
  copyStateToActual();
  omni::ptr_arith_t p1, p2;
  while (1) {
    p1 = omni::align_to((omni::ptr_arith_t)pd_actual.pd_inb_mkr,
			omni::ALIGN_4);
    p2 = p1 + 4;
    if (p2 <= (omni::ptr_arith_t)pd_actual.pd_inb_end)
      break;
    pd_actual.fetchInputData(omni::ALIGN_4, 4);
  }
  copyStateFromActual();

  _CORBA_Long tag = *((_CORBA_Long*)p1);

  if (pd_unmarshal_byte_swap)
    return Swap32(tag);
  else
    return tag;
}



void
cdrValueChunkStream::
get_octet_array(_CORBA_Octet* b,int size, omni::alignment_t align)
{
  omni::ptr_arith_t p1, p2;

  p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr, align);
  p2 = p1 + size;
  if (p2 <= (omni::ptr_arith_t)pd_inb_end) {
    memcpy((void*)b, (const void*)p1, size);
    pd_inb_mkr = (void*)p2;
    return;
  }

  if (pd_inHeader) {
    OMNIORB_ASSERT(!pd_inChunk);
    copyStateToActual();
    pd_actual.get_octet_array(b, size, align);
    copyStateFromActual();
    return;
  }

  // Copy as much as possible out of the buffer
  CORBA::Long inbuf = (omni::ptr_arith_t)pd_inb_end - p1;
  if (inbuf) {
    memcpy((void*)b, (const void*)p1, inbuf);
    size -= inbuf;
    b += inbuf;
    pd_inb_mkr = pd_inb_end;
  }

  if (pd_remaining) {
    // More octets left in this chunk
    copyStateToActual();
    pd_actual.get_octet_array(b, pd_remaining, align);
    size -= pd_remaining;
    b += pd_remaining;
    pd_remaining = 0;
    copyStateFromActual();
  }

  copyStateToActual();

  while (size) {
    // More chunks to come
    CORBA::Long len;
    len <<= pd_actual;

    if (align == omni::ALIGN_8) {
      // If the alignment is 8, there may be 4 octets of padding after
      // the chunk length. In that case, we need to skip it and
      // subtract 4 from the chunk length that's available.
      p1 = (omni::ptr_arith_t)pd_actual.pd_inb_mkr;
      p2 = omni::align_to(p1, omni::ALIGN_8);
      if (p2 > p1)
	size -= 4;
    }

    if (len <= 0 || len >= 0x7fffff00)
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
		    (CORBA::CompletionStatus)completion());

    if (len < size) {
      pd_actual.get_octet_array(b, len, align);
      size -= len;
      b += len;
    }
    else {
      // Finish the read
      pd_actual.get_octet_array(b, size, align);
      size = 0;

      // Sort out the input pointer
      copyStateFromActual();
      len -= size;
      p1 = (omni::ptr_arith_t)pd_inb_mkr;
      p2 = p1 + len;
      if (p2 > (omni::ptr_arith_t)pd_inb_end)
	pd_remaining = p2 - (omni::ptr_arith_t)pd_inb_end;
      else
	pd_inb_end = (void*)p2;

      break;
    }
  }
}

void
cdrValueChunkStream::
skipInput(_CORBA_ULong size)
{
  omni::ptr_arith_t p1, p2;

  p1 = (omni::ptr_arith_t)pd_inb_mkr;
  p2 = p1 + size;
  if (p2 <= (omni::ptr_arith_t)pd_inb_end) {
    pd_inb_mkr = (void*)p2;
    return;
  }

  if (pd_inHeader) {
    OMNIORB_ASSERT(!pd_inChunk);
    copyStateToActual();
    pd_actual.skipInput(size);
    copyStateFromActual();
    return;
  }

  // Copy as much as possible out of the buffer
  CORBA::Long inbuf = (omni::ptr_arith_t)pd_inb_end - p1;
  if (inbuf) {
    size -= inbuf;
    pd_inb_mkr = pd_inb_end;
  }

  if (pd_remaining) {
    // More octets left in this chunk
    copyStateToActual();
    pd_actual.skipInput(pd_remaining);
    size -= pd_remaining;
    pd_remaining = 0;
    copyStateFromActual();
  }

  copyStateToActual();

  while (size) {
    // More chunks to come
    CORBA::Long len;
    len <<= pd_actual;

    if (len <= 0 || len >= 0x7fffff00)
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidChunkedEncoding,
		    (CORBA::CompletionStatus)completion());

    if ((CORBA::ULong)len < size) {
      pd_actual.skipInput(len);
      size -= len;
    }
    else {
      // Finish the read
      pd_actual.skipInput(size);
      size = 0;

      // Sort out the input pointer
      copyStateFromActual();
      len -= size;
      p1 = (omni::ptr_arith_t)pd_inb_mkr;
      p2 = p1 + len;
      if (p2 > (omni::ptr_arith_t)pd_inb_end)
	pd_remaining = p2 - (omni::ptr_arith_t)pd_inb_end;
      else
	pd_inb_end = (void*)p2;

      break;
    }
  }
}


_CORBA_Boolean
cdrValueChunkStream::
checkInputOverrun(_CORBA_ULong itemSize,
		  _CORBA_ULong nItems,
		  omni::alignment_t align)
{
  copyStateToActual();
  return pd_actual.checkInputOverrun(itemSize, nItems, align);
}

_CORBA_Boolean
cdrValueChunkStream::
checkOutputOverrun(_CORBA_ULong itemSize,
		   _CORBA_ULong nItems,
		   omni::alignment_t align)
{
  copyStateToActual();
  return pd_actual.checkOutputOverrun(itemSize, nItems, align);
}

_CORBA_ULong
cdrValueChunkStream::currentInputPtr() const
{
  copyStateToActual();
  return pd_actual.currentInputPtr();
}

_CORBA_ULong
cdrValueChunkStream::currentOutputPtr() const
{
  copyStateToActual();
  return pd_actual.currentOutputPtr();
}

_CORBA_ULong
cdrValueChunkStream::completion()
{
  return pd_actual.completion();
}

  
void*
cdrValueChunkStream::ptrToClass(int* cptr)
{
  if (cptr == &cdrValueChunkStream::_classid)
    return (cdrValueChunkStream*)this;

  if (cptr == &cdrStream::_classid)
    return (cdrStream*)this;

  return 0;
}

int cdrValueChunkStream::_classid;
