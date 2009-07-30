// -*- Mode: C++; -*-
//                            Package   : omniORB
// cdrStream.cc               Created on: 09/11/2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2006 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambrige
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
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.1.4.11  2006/06/05 11:25:30  dgrisby
  Move codeset initialisation code to a more logical source file.

  Revision 1.1.4.10  2006/05/21 17:43:24  dgrisby
  Remove obsolete pd_chunked flag.

  Revision 1.1.4.9  2006/05/15 10:13:00  dgrisby
  Data was overwritten when a chunk ended with an array; make
  declareArrayLength() virtual.

  Revision 1.1.4.8  2006/01/10 12:24:03  dgrisby
  Merge from omni4_0_develop pre 4.0.7 release.

  Revision 1.1.4.7  2005/12/08 14:22:31  dgrisby
  Better string marshalling performance; other minor optimisations.

  Revision 1.1.4.6  2005/03/30 23:36:10  dgrisby
  Another merge from omni4_0_develop.

  Revision 1.1.4.5  2005/01/25 11:17:46  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.4  2005/01/06 23:10:12  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.3  2003/11/06 11:56:57  dgrisby
  Yet more valuetype. Plain valuetype and abstract valuetype are now working.

  Revision 1.1.4.2  2003/05/20 16:53:15  dgrisby
  Valuetype marshalling support.

  Revision 1.1.4.1  2003/03/23 21:02:24  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.15  2003/02/03 16:53:14  dgrisby
  Force type in constructor argument to help confused compilers.

  Revision 1.1.2.14  2002/11/26 14:51:50  dgrisby
  Implement missing interceptors.

  Revision 1.1.2.13  2002/05/07 12:55:47  dgrisby
  Minor tweak to log message format.

  Revision 1.1.2.12  2002/04/25 23:13:30  dgrisby
  Minor tweak to codeset setting interceptors for GIOP 1.0.

  Revision 1.1.2.11  2001/11/14 17:13:43  dpg1
  Long double support.

  Revision 1.1.2.10  2001/10/17 16:33:28  dpg1
  New downcast mechanism for cdrStreams.

  Revision 1.1.2.9  2001/08/21 11:02:12  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.1.2.8  2001/08/17 17:12:35  sll
  Modularise ORB configuration parameters.

  Revision 1.1.2.7  2001/08/03 17:41:18  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.2.6  2001/07/31 17:42:11  sll
  Cleanup String_var usage.

  Revision 1.1.2.5  2001/07/31 16:32:02  sll
  Added virtual function is_giopStream to check if a cdrStream is a giopStream.
  That is, a poor man's substitute for dynamic_cast.

  Revision 1.1.2.4  2001/04/18 18:18:11  sll
  Big checkin with the brand new internal APIs.

  Revision 1.1.2.3  2000/11/20 11:59:43  dpg1
  API to configure code sets.

  Revision 1.1.2.2  2000/11/15 19:16:06  sll
  Changed default native wchar from UCS-4 to UTF-16.

  Revision 1.1.2.1  2000/11/15 17:17:20  sll
  *** empty log message ***

*/

#include <omniORB4/CORBA.h>
#include <giopStream.h>
#include <giopStrand.h>
#include <GIOP_S.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <orbParameters.h>

OMNI_USING_NAMESPACE(omni)

/////////////////////////////////////////////////////////////////////////////
cdrStream::cdrStream() : pd_unmarshal_byte_swap(0), pd_marshal_byte_swap(0),
			 pd_inb_end(0), pd_inb_mkr(0),
			 pd_outb_end(0), pd_outb_mkr(0),
			 pd_tcs_c(0), pd_tcs_w(0),
			 pd_ncs_c(orbParameters::nativeCharCodeSet),
			 pd_ncs_w(orbParameters::nativeWCharCodeSet),
			 pd_valueTracker(0)
{
}

/////////////////////////////////////////////////////////////////////////////
cdrStream::~cdrStream()
{
  if (pd_valueTracker) {
    delete pd_valueTracker;
  }
}

/////////////////////////////////////////////////////////////////////////////
void*
cdrStream::ptrToClass(int* cptr)
{
  if (cptr == &cdrStream::_classid) return (cdrStream*)this;
  return 0;
}

int cdrStream::_classid;

/////////////////////////////////////////////////////////////////////////////
CORBA::ULong 
cdrStream::completion() {
  return CORBA::COMPLETED_NO;
}

/////////////////////////////////////////////////////////////////////////////
void
cdrStream::copy_to(cdrStream& s,int size,omni::alignment_t align) {
  try {
    alignInput(align);
    if (s.maybeReserveOutputSpace(align,size)) {
      omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)s.pd_outb_mkr,
					    align);
      get_octet_array((CORBA::Octet*)p1,size,align);
      s.pd_outb_mkr = (void*)(p1 + size);
    }
    else {
      skipInput(size);
    }
  }
  catch (const CORBA::BAD_PARAM&) {
    // Use a bounce buffer if the destination stream cannot satisfy
    // maybeReserveOutputSpace().
    cdrMemoryStream mbuf(size,0);
    mbuf.maybeReserveOutputSpace(align,size);
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)mbuf.pd_outb_mkr,
					  align);
    get_octet_array((CORBA::Octet*)p1,size,align);
    s.put_octet_array((const CORBA::Octet*)p1,size,align);
  }
}

void
cdrStream::declareArrayLength(omni::alignment_t, size_t)
{
  // Nothing to do.
}

void
cdrStream::put_small_octet_array(const _CORBA_Octet* b, int size)
{
  omni::ptr_arith_t p1 = (omni::ptr_arith_t)pd_outb_mkr;
  omni::ptr_arith_t p2 = p1 + size;

  if ((void*)p2 <= pd_outb_end) {
    while (p1 < p2) {
      *((_CORBA_Octet*)p1++) = *b++;
    }
    pd_outb_mkr = (void*)p2;
  }
  else {
    put_octet_array(b, size);
  }
}


/////////////////////////////////////////////////////////////////////////////
#ifdef HAS_LongDouble
#  if SIZEOF_LONG_DOUBLE == 12
#    ifndef __x86__
#      error "12-byte long double only supported for x86"
#    endif

// Intel x86 extended double is odd. Firstly, it's 80 bits, not 96, so
// the two most significant bytes are always zero. Secondly, the
// significand _includes_ the most significant bit. IEEE floating
// point always misses out the msb, as do the other floating point
// formats on x86. This means we have to do lots of bit shifting.
//
// This isn't the most efficient code in the world, but it's designed
// to be easy to understand.

void
cdrStream::marshalLongDouble(_CORBA_LongDouble a)
{
  _CORBA_Octet  mbuf[16];
  _CORBA_Octet* dbuf = (_CORBA_Octet*)&a;

  memset(mbuf, 0, 16);

  if (pd_marshal_byte_swap) { // big endian
    // Sign and exponent
    mbuf[0] = dbuf[9];
    mbuf[1] = dbuf[8];

    // significand
    mbuf[2] = (dbuf[7] << 1) | (dbuf[6] >> 7);
    mbuf[3] = (dbuf[6] << 1) | (dbuf[5] >> 7);
    mbuf[4] = (dbuf[5] << 1) | (dbuf[4] >> 7);
    mbuf[5] = (dbuf[4] << 1) | (dbuf[3] >> 7);
    mbuf[6] = (dbuf[3] << 1) | (dbuf[2] >> 7);
    mbuf[7] = (dbuf[2] << 1) | (dbuf[1] >> 7);
    mbuf[8] = (dbuf[1] << 1) | (dbuf[0] >> 7);
    mbuf[9] = (dbuf[0] << 1);
  }
  else { // little endian
    // Sign and exponent
    mbuf[15] = dbuf[9];
    mbuf[14] = dbuf[8];

    // significand
    mbuf[13] = (dbuf[7] << 1) | (dbuf[6] >> 7);
    mbuf[12] = (dbuf[6] << 1) | (dbuf[5] >> 7);
    mbuf[11] = (dbuf[5] << 1) | (dbuf[4] >> 7);
    mbuf[10] = (dbuf[4] << 1) | (dbuf[3] >> 7);
    mbuf[ 9] = (dbuf[3] << 1) | (dbuf[2] >> 7);
    mbuf[ 8] = (dbuf[2] << 1) | (dbuf[1] >> 7);
    mbuf[ 7] = (dbuf[1] << 1) | (dbuf[0] >> 7);
    mbuf[ 6] = (dbuf[0] << 1);
  }
  put_octet_array((_CORBA_Octet*)mbuf, 16, omni::ALIGN_8);
}

_CORBA_LongDouble
cdrStream::unmarshalLongDouble()
{
  _CORBA_LongDouble a;
  _CORBA_Octet  mbuf[16];
  _CORBA_Octet* dbuf = (_CORBA_Octet*)&a;

  get_octet_array((_CORBA_Octet*)mbuf, 16, omni::ALIGN_8);

  dbuf[11] = dbuf[10] = 0;

  if (pd_unmarshal_byte_swap) { // big endian
    // Sign and exponent
    dbuf[9] = mbuf[0];
    dbuf[8] = mbuf[1];

    // significand
    dbuf[7] = (mbuf[0] == 0 && mbuf[1] == 0) ? 0 : 0x80 | (mbuf[2] >> 1);
    dbuf[6] = (mbuf[2] << 7) | (mbuf[3] >> 1);
    dbuf[5] = (mbuf[3] << 7) | (mbuf[4] >> 1);
    dbuf[4] = (mbuf[4] << 7) | (mbuf[5] >> 1);
    dbuf[3] = (mbuf[5] << 7) | (mbuf[6] >> 1);
    dbuf[2] = (mbuf[6] << 7) | (mbuf[7] >> 1);
    dbuf[1] = (mbuf[7] << 7) | (mbuf[8] >> 1);
    dbuf[0] = (mbuf[8] << 7) | (mbuf[9] >> 1);
  }
  else { // little endian
    // Sign and exponent
    dbuf[9] = mbuf[15];
    dbuf[8] = mbuf[14];

    // significand
    dbuf[7] = (mbuf[15] == 0 && mbuf[14] == 0) ? 0 : 0x80 | (mbuf[13] >> 1);
    dbuf[6] = (mbuf[13] << 7) | (mbuf[12] >> 1);
    dbuf[5] = (mbuf[12] << 7) | (mbuf[11] >> 1);
    dbuf[4] = (mbuf[11] << 7) | (mbuf[10] >> 1);
    dbuf[3] = (mbuf[10] << 7) | (mbuf[ 9] >> 1);
    dbuf[2] = (mbuf[ 9] << 7) | (mbuf[ 8] >> 1);
    dbuf[1] = (mbuf[ 8] << 7) | (mbuf[ 7] >> 1);
    dbuf[0] = (mbuf[ 7] << 7) | (mbuf[ 6] >> 1);
  }
  return a;
}

#  endif // SIZEOF_LONG_DOUBLE == 12
#endif // HAS_LongDouble


/////////////////////////////////////////////////////////////////////////////
#define fetchReserveMarshalFns(type, align)\
void cdrStream::reserveAndMarshal ## type(CORBA::type a) \
{ \
  if (reserveOutputSpaceForPrimitiveType(align, sizeof(CORBA::type))) \
    marshal ## type(a); \
} \
CORBA::type cdrStream::fetchAndUnmarshal ## type() \
{ \
  fetchInputData(align, sizeof(CORBA::type)); \
  return unmarshal ## type(); \
}

fetchReserveMarshalFns(Octet,      omni::ALIGN_1)
fetchReserveMarshalFns(Short,      omni::ALIGN_2)
fetchReserveMarshalFns(UShort,     omni::ALIGN_2)
fetchReserveMarshalFns(Long,       omni::ALIGN_4)
fetchReserveMarshalFns(ULong,      omni::ALIGN_4)

#ifdef HAS_LongLong
fetchReserveMarshalFns(LongLong,   omni::ALIGN_8)
fetchReserveMarshalFns(ULongLong,  omni::ALIGN_8)
#endif

#ifndef NO_FLOAT
fetchReserveMarshalFns(Double,     omni::ALIGN_8)

#  if defined(HAS_LongDouble) && defined(HAS_LongLong)
fetchReserveMarshalFns(LongDouble, omni::ALIGN_8)
#  endif

#endif


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////
OMNI_NAMESPACE_BEGIN(omni)

class omni_cdrStream_initialiser : public omniInitialiser {
public:

  void attach() {
  }
  void detach() {
  }
};


static omni_cdrStream_initialiser initialiser;

omniInitialiser& omni_cdrStream_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
