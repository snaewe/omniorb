// -*- Mode: C++; -*-
//                            Package   : omniORB2
// cdrStream.h                Created on: 11/1/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1999 AT&T Laboratories, Cambridge
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
  Revision 1.1.2.20  2003/03/02 17:10:41  dgrisby
  AIX patches integrated in main tree.

  Revision 1.1.2.19  2002/03/14 14:39:44  dpg1
  Obscure bug in objref creation with unaligned buffers.

  Revision 1.1.2.18  2002/03/11 12:23:03  dpg1
  Tweaks to avoid compiler warnings.

  Revision 1.1.2.17  2001/11/14 17:13:41  dpg1
  Long double support.

  Revision 1.1.2.16  2001/10/17 16:33:27  dpg1
  New downcast mechanism for cdrStreams.

  Revision 1.1.2.15  2001/09/19 17:29:04  dpg1
  Cosmetic changes.

  Revision 1.1.2.14  2001/08/22 13:29:45  dpg1
  Re-entrant Any marshalling.

  Revision 1.1.2.13  2001/08/17 17:02:22  sll
  Removed static variables ncs_c, ncs_w, default_tcs_c, default_tcs_w.

  Revision 1.1.2.12  2001/07/31 16:32:02  sll
  Added virtual function is_giopStream to check if a cdrStream is a giopStream.
  That is, a poor man's substitute for dynamic_cast.

  Revision 1.1.2.11  2001/06/13 19:58:37  sll
  Added omni namespace scoping for omniCodeSet.

  Revision 1.1.2.10  2001/05/10 15:03:50  dpg1
  Update cdrStreamAdapter to modified cdrStream interface.

  Revision 1.1.2.9  2001/04/18 17:50:44  sll
  Big checkin with the brand new internal APIs.
  Scoped where appropriate with the omni namespace.

  Revision 1.1.2.8  2001/01/09 17:16:59  dpg1
  New cdrStreamAdapter class to allow omniORBpy to intercept buffer
  management.

  Revision 1.1.2.7  2000/12/05 17:39:31  dpg1
  New cdrStream functions to marshal and unmarshal raw strings.

  Revision 1.1.2.6  2000/11/22 14:37:58  dpg1
  Code set marshalling functions now take a string length argument.

  Revision 1.1.2.5  2000/11/15 17:16:23  sll
  Added char, wchar codeset convertor support to cdrStream.

  Revision 1.1.2.4  2000/11/09 12:27:49  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.1.2.3  2000/11/03 18:49:16  sll
  Separate out the marshalling of byte, octet and char into 3 set of distinct
  marshalling functions.
  Renamed put_char_array and get_char_array to put_octet_array and
  get_octet_array.
  New string marshal member functions.

  Revision 1.1.2.2  2000/10/10 10:14:27  sll
  Extra ctor for cdrEncapsulationStream which initialise the buffer by
  fetching data from the argument cdrStream.

  Revision 1.1.2.1  2000/09/27 16:54:08  sll
  *** empty log message ***

*/

#ifndef __CDRSTREAM_H__
#define __CDRSTREAM_H__

#include <limits.h>

#ifndef Swap16
#define Swap16(s) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff))
#else
#error "Swap16 has already been defined"
#endif

#ifndef Swap32
#define Swap32(l) ((((l) & 0xff000000) >> 24) | \
		   (((l) & 0x00ff0000) >> 8)  | \
		   (((l) & 0x0000ff00) << 8)  | \
		   (((l) & 0x000000ff) << 24))
#else
#error "Swap32 has already been defined"
#endif

#ifdef HAS_LongLong
#ifndef Swap64
#define Swap64(l) \
  ((((l) & _CORBA_LONGLONG_CONST(0xff00000000000000)) >> 56) | \
   (((l) & _CORBA_LONGLONG_CONST(0x00ff000000000000)) >> 40) | \
   (((l) & _CORBA_LONGLONG_CONST(0x0000ff0000000000)) >> 24) | \
   (((l) & _CORBA_LONGLONG_CONST(0x000000ff00000000)) >> 8)  | \
   (((l) & _CORBA_LONGLONG_CONST(0x00000000ff000000)) << 8)  | \
   (((l) & _CORBA_LONGLONG_CONST(0x0000000000ff0000)) << 24) | \
   (((l) & _CORBA_LONGLONG_CONST(0x000000000000ff00)) << 40) | \
   (((l) & _CORBA_LONGLONG_CONST(0x00000000000000ff)) << 56))
#else
#error "Swap64 has already been defined"
#endif
#endif

class cdrStreamAdapter;

class cdrStream {
public:

  cdrStream();

  virtual ~cdrStream() {}

#ifndef CdrMarshal
#define CdrMarshal(s,type,align,arg) do {\
   again: \
   omni::ptr_arith_t p1 =omni::align_to((omni::ptr_arith_t)s.pd_outb_mkr,align);\
   omni::ptr_arith_t p2 = p1 + sizeof(type);\
   if( (void*)p2 > s.pd_outb_end ) {\
     if (s.reserveOutputSpaceForPrimitiveType(align,sizeof(type)))\
       goto again;\
     else {\
       s.pd_outb_mkr = (void*) p2;\
       break;\
     }\
   }\
   s.pd_outb_mkr = (void*) p2;\
   *((type*)p1) = arg;\
} while(0)
#else
#error "CdrMarshal has already been defined"
#endif

#ifndef CdrUnMarshal
#define CdrUnMarshal(s,type,align,arg) do {\
  again: \
  omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)s.pd_inb_mkr,align);\
  omni::ptr_arith_t p2 = p1 + sizeof(type);\
  if ((void *)p2 > s.pd_inb_end) {\
    s.fetchInputData(align,sizeof(type));\
    goto again;\
  }\
  s.pd_inb_mkr = (void*) p2;\
  arg = *((type*)p1);\
} while(0)
#else
#error "CdrUnMarshal has already been defined"
#endif

  inline void marshalChar(_CORBA_Char a) {
    pd_ncs_c->marshalChar(*this,pd_tcs_c,a);
  }

  inline _CORBA_Char unmarshalChar() {
    return pd_ncs_c->unmarshalChar(*this,pd_tcs_c);
  }

  inline void marshalWChar(_CORBA_WChar a) {
    pd_ncs_w->marshalWChar(*this,pd_tcs_w,a);
  }

  inline _CORBA_WChar unmarshalWChar() {
    return pd_ncs_w->unmarshalWChar(*this,pd_tcs_w);
  }

  inline void marshalOctet(_CORBA_Octet a) {
    CdrMarshal((*this),_CORBA_Octet,omni::ALIGN_1,a);
  }

  inline _CORBA_Octet unmarshalOctet() {
    _CORBA_Octet a;
    CdrUnMarshal((*this),_CORBA_Octet,omni::ALIGN_1,a);
    return a;
  }

  inline void marshalBoolean(_CORBA_Boolean b) {
    _CORBA_Char c = b ? 1 : 0;
    CdrMarshal((*this),_CORBA_Char,omni::ALIGN_1,c);
  }

  inline _CORBA_Boolean unmarshalBoolean() {
    _CORBA_Char c;
    CdrUnMarshal((*this),_CORBA_Char,omni::ALIGN_1,c);
#ifdef HAS_Cplusplus_Bool
    return c ? true : false;
#else
    return c;
#endif
  }

#ifndef OMNI_NO_INLINE_FRIENDS
  friend inline void operator>>= (_CORBA_Short a, cdrStream& s) {
    if (s.pd_marshal_byte_swap) {
      _CORBA_Short t = Swap16(a);
      a = t;
    }
    CdrMarshal(s,_CORBA_Short,omni::ALIGN_2,a);
  }

  friend inline void operator<<= (_CORBA_Short& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_Short,omni::ALIGN_2,a);
    if (s.pd_unmarshal_byte_swap) {
      _CORBA_Short t = Swap16(a);
      a = t;
    }
  }

  friend inline void operator>>= (_CORBA_UShort a, cdrStream& s) {
    if (s.pd_marshal_byte_swap) {
      _CORBA_UShort t = Swap16(a);
      a = t;
    }
    CdrMarshal(s,_CORBA_UShort,omni::ALIGN_2,a);
  }

  friend inline void operator<<= (_CORBA_UShort& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_UShort,omni::ALIGN_2,a);
    if (s.pd_unmarshal_byte_swap) {
      _CORBA_UShort t = Swap16(a);
      a = t;
    }
  }

  friend inline void operator>>= (_CORBA_Long a, cdrStream& s) {
    if (s.pd_marshal_byte_swap) {
      _CORBA_Long t = Swap32(a);
      a = t;
    }
    CdrMarshal(s,_CORBA_Long,omni::ALIGN_4,a);
  }

  friend inline void operator<<= (_CORBA_Long& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_Long,omni::ALIGN_4,a);
    if (s.pd_unmarshal_byte_swap) {
      _CORBA_Long t = Swap32(a);
      a = t;
    }
  }

  friend inline void operator>>= (_CORBA_ULong a, cdrStream& s) {
    if (s.pd_marshal_byte_swap) {
      _CORBA_ULong t = Swap32(a);
      a = t;
    }
    CdrMarshal(s,_CORBA_ULong,omni::ALIGN_4,a);
  }

  friend inline void operator<<= (_CORBA_ULong& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_ULong,omni::ALIGN_4,a);
    if (s.pd_unmarshal_byte_swap) {
      _CORBA_ULong t = Swap32(a);
      a = t;
    }
  }

  friend inline void operator>>= (_CORBA_LongLong a, cdrStream& s) {
    if (s.pd_marshal_byte_swap) {
      _CORBA_LongLong t = Swap64(a);
      a = t;
    }
    CdrMarshal(s,_CORBA_LongLong,omni::ALIGN_8,a);
  }

  friend inline void operator<<= (_CORBA_LongLong& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_LongLong,omni::ALIGN_8,a);
    if (s.pd_unmarshal_byte_swap) {
      _CORBA_LongLong t = Swap64(a);
      a = t;
    }
  }

  friend inline void operator>>= (_CORBA_ULongLong a, cdrStream& s) {
    if (s.pd_marshal_byte_swap) {
      _CORBA_ULongLong t = Swap64(a);
      a = t;
    }
    CdrMarshal(s,_CORBA_ULongLong,omni::ALIGN_8,a);
  }

  friend inline void operator<<= (_CORBA_ULongLong& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_ULongLong,omni::ALIGN_8,a);
    if (s.pd_unmarshal_byte_swap) {
      _CORBA_ULongLong t = Swap64(a);
      a = t;
    }
  }
#else
  friend inline void operator>>= (_CORBA_Short      a, cdrStream& s);
  friend inline void operator<<= (_CORBA_Short&     a, cdrStream& s);
  friend inline void operator>>= (_CORBA_UShort     a, cdrStream& s);
  friend inline void operator<<= (_CORBA_UShort&    a, cdrStream& s);
  friend inline void operator>>= (_CORBA_Long       a, cdrStream& s);
  friend inline void operator<<= (_CORBA_Long&      a, cdrStream& s);
  friend inline void operator>>= (_CORBA_ULong      a, cdrStream& s);
  friend inline void operator<<= (_CORBA_ULong&     a, cdrStream& s);
  friend inline void operator>>= (_CORBA_LongLong   a, cdrStream& s);
  friend inline void operator<<= (_CORBA_LongLong&  a, cdrStream& s);
  friend inline void operator>>= (_CORBA_ULongLong  a, cdrStream& s);
  friend inline void operator<<= (_CORBA_ULongLong& a, cdrStream& s);
#endif

#if !defined(NO_FLOAT)

#ifndef OMNI_NO_INLINE_FRIENDS
  friend inline void operator>>= (_CORBA_Float a, cdrStream& s) {
    if (s.pd_marshal_byte_swap) {
      union {
 	_CORBA_Float f;
 	_CORBA_ULong l;
      } u;
      u.f = a;
      u.l = Swap32(u.l);
      a = u.f;
    }
    CdrMarshal(s,_CORBA_Float,omni::ALIGN_4,a);
  }

  friend inline void operator<<= (_CORBA_Float& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_Float,omni::ALIGN_4,a);
    if (s.pd_unmarshal_byte_swap) {
      union {
	_CORBA_Float f;
	_CORBA_ULong l;
      } u;
      u.f = a;
      u.l = Swap32(u.l);
      a = u.f;
    }
  }

  friend inline void operator>>= (_CORBA_Double a, cdrStream& s) {
    if (s.pd_marshal_byte_swap) {
      union {
	_CORBA_Double d;
	_CORBA_ULong l[2];
      } u, v;
      u.d = a;
      v.l[0] = Swap32(u.l[1]);
      v.l[1] = Swap32(u.l[0]);
      a = v.d;
    }
    CdrMarshal(s,_CORBA_Double,omni::ALIGN_8,a);
  }

  friend inline void operator<<= (_CORBA_Double& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_Double,omni::ALIGN_8,a);
    if (s.pd_unmarshal_byte_swap) {
      union {
	_CORBA_Double d;
	_CORBA_ULong l[2];
      } u, v;
      u.d = a;
      v.l[0] = Swap32(u.l[1]);
      v.l[1] = Swap32(u.l[0]);
      a = v.d;
    }
  }
#else
  friend inline void operator>>= (_CORBA_Float      a, cdrStream& s);
  friend inline void operator<<= (_CORBA_Float&     a, cdrStream& s);
  friend inline void operator>>= (_CORBA_Double     a, cdrStream& s);
  friend inline void operator<<= (_CORBA_Double&    a, cdrStream& s);
#endif
#endif

#ifdef HAS_LongDouble
#if SIZEOF_LONG_DOUBLE == 16

#ifndef OMNI_NO_INLINE_FRIENDS
  friend inline void operator>>= (_CORBA_LongDouble a, cdrStream& s) {
    if (s.pd_marshal_byte_swap) {
      union {
	_CORBA_LongDouble d;
	_CORBA_ULong l[4];
      } u, v;
      u.d = a;
      v.l[0] = Swap32(u.l[3]);
      v.l[1] = Swap32(u.l[2]);
      v.l[2] = Swap32(u.l[1]);
      v.l[3] = Swap32(u.l[0]);
      a = v.d;
    }
    CdrMarshal(s,_CORBA_LongDouble,omni::ALIGN_8,a);
  }

  friend inline void operator<<= (_CORBA_LongDouble& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_LongDouble,omni::ALIGN_8,a);
    if (s.pd_unmarshal_byte_swap) {
      union {
	_CORBA_LongDouble d;
	_CORBA_ULong l[4];
      } u, v;
      u.d = a;
      v.l[0] = Swap32(u.l[3]);
      v.l[1] = Swap32(u.l[2]);
      v.l[2] = Swap32(u.l[1]);
      v.l[3] = Swap32(u.l[0]);
      a = v.d;
    }
  }
#else
  friend inline void operator>>= (_CORBA_LongDouble  a, cdrStream& s);
  friend inline void operator<<= (_CORBA_LongDouble& a, cdrStream& s);
#endif
#else
  // Code for long double < 16 bytes is too painful to put inline
  friend void operator>>= (_CORBA_LongDouble  a, cdrStream& s);
  friend void operator<<= (_CORBA_LongDouble& a, cdrStream& s);
#endif
#endif

  inline void marshalString(const char* s,int bounded=0) {
    OMNIORB_USER_CHECK(s);
    pd_ncs_c->marshalString(*this,pd_tcs_c,bounded,strlen(s),s);
  }

  inline char* unmarshalString(int bounded=0) {
    char* s;
    pd_ncs_c->unmarshalString(*this,pd_tcs_c,bounded,s);
    return s;
  }

  inline void marshalRawString(const char* s) {
    _CORBA_ULong len = strlen(s) + 1; len >>= *this;
    put_octet_array((const _CORBA_Octet*)s, len);
  }

  char* unmarshalRawString();
  // unmarshalRawString() can't be inline since it has to throw
  // MARSHAL exceptions.

  inline void marshalWString(const _CORBA_WChar* s,int bounded=0) {
    OMNIORB_USER_CHECK(s);
    pd_ncs_w->marshalWString(*this,pd_tcs_w,bounded,
			     _CORBA_WString_helper::len(s),s);
  }

  inline _CORBA_WChar* unmarshalWString(int bounded=0) {
    _CORBA_WChar* s;
    pd_ncs_w->unmarshalWString(*this,pd_tcs_w,bounded,s);
    return s;
  }

  virtual void put_octet_array(const _CORBA_Octet* b, int size,
			       omni::alignment_t align=omni::ALIGN_1) = 0;
  // <size> must be a multiple of <align>.
  // For instance, if <align> == omni::ALIGN_8 then <size> % 8 == 0.

  virtual void get_octet_array(_CORBA_Octet* b,int size,
			       omni::alignment_t align=omni::ALIGN_1) = 0;



  virtual void skipInput(_CORBA_ULong size) = 0;
  // Skip <size> bytes from the input stream.

  virtual _CORBA_Boolean checkInputOverrun(_CORBA_ULong itemSize,
				_CORBA_ULong nItems,
				omni::alignment_t align=omni::ALIGN_1) = 0;
  // Return TRUE(1) if the input stream contains data for at least one
  // <nitems> of size <itemSize>. The initial alignment of the data starts
  // at <align>. Return FALSE(0) otherwise.

  virtual _CORBA_ULong currentInputPtr() const = 0;
  // Return a value that represent the position of the next byte in the
  // input stream. Later bytes in the stream has a higher return value.
  // The absolute value of the return value has no meaning.
  // The only use of this function is to compute the distance between two
  // bytes in the stream.

  virtual _CORBA_ULong currentOutputPtr() const = 0;
  // Return a value that represent the position of the next byte in the
  // output stream. Later bytes in the stream has a higher return value.
  // The absolute value of the return value has no meaning.
  // The only use of this function is to compute the distance between two
  // bytes in the stream.


  virtual _CORBA_Boolean checkOutputOverrun(_CORBA_ULong itemSize,
				_CORBA_ULong nItems,
				omni::alignment_t align=omni::ALIGN_1) = 0;
  // Return TRUE(1) if data of at least  <nitems> of size <itemSize> can be
  // inserted to the output stream. The initial alignment of the data starts
  // at <align>. Return FALSE(0) otherwise.

  virtual void copy_to(cdrStream&,int size,
		       omni::alignment_t align=omni::ALIGN_1);
  // From this stream, copy <size> bytes to the argument stream.
  // The initial alignment starts at <align>.
  // Derived classes may provided more efficent implementation than the
  // default.


  inline
  void alignInput(omni::alignment_t align)
  // Align the buffer of the input stream to <align>.
    {
    again:
      omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,
					    align);
      if ((void*)p1 > pd_inb_end) {
	fetchInputData(align,0);
	goto again;
      }
      pd_inb_mkr = (void*)p1;
    }

  inline
  void alignOutput(omni::alignment_t align)
  // Align the buffer of the output stream to <align>.
    {
    again:
      omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr,
					    align);
      if ((void*)p1 > pd_outb_end) {
	if (reserveOutputSpaceForPrimitiveType(align,0))
	  goto again;
      }
      pd_outb_mkr = (void*)p1;
    }

  inline
  _CORBA_Boolean unmarshal_byte_swap() const { return pd_unmarshal_byte_swap; }
  // Return TRUE(1) if unmarshalled data have to be byte-swapped.

  inline
  _CORBA_Boolean marshal_byte_swap() const { return pd_marshal_byte_swap; }

  inline
  void* PR_get_outb_mkr() { return pd_outb_mkr; }
  // internal function used by cdrStream implementations

protected:

  _CORBA_Boolean pd_unmarshal_byte_swap;
  // TRUE(1) if unmarshalled data have to be byte-swapped.

  _CORBA_Boolean pd_marshal_byte_swap;
  // TRUE(1) if marshalled data have to be byte-swapped. I.e. data are not
  // going into the stream with the host endian.

  //  Input buffer pointers, the region (*p) containing valid data
  //  is pd_inb_mkr <= p < pd_inb_end.
  //  pd_inb_end and pd_inb_mkr are initialised by fetchInputData().
  //  pd_inb_mkr is also updated by the unmarshalling operators between
  //  calls to fetchInputData().
  void* pd_inb_end;
  void* pd_inb_mkr;

  virtual void fetchInputData(omni::alignment_t align,
			      size_t required)     = 0;
  // Fetch at least <required> bytes into the input buffer.
  // <required> must be no more than 8 bytes && align == required!!
  // The data block should start at alignment <align>.
  // If the space available is less than specified, raise a
  // MARSHAL system exception.

  //  Output buffer pointers, the region (*p) that can be written
  //  into is pd_outb_mkr <= p < pd_outb_end.
  //  pd_outb_end and pd_outb_mkr are initialised by reserveOutputSpace().
  //  pd_outb_mkr is also updated by the marshalling operators between
  //  calls to reserveOutputSpace().
  void* pd_outb_end;
  void* pd_outb_mkr;

  virtual
  _CORBA_Boolean reserveOutputSpaceForPrimitiveType(omni::alignment_t align,
						    size_t required) = 0;
  // Allocate at least <required> bytes in the output buffer.
  // The data block should start at alignment <align>.
  // <required> must be no more than 8 bytes && align == required!!
  // Returns TRUE(1) if at least <required> bytes with the starting
  // alignment as specified has been allocated.
  // Returns FALSE(0) if the required space cannot be allocated.
  // When the return value is FALSE, the cdrStream would skip writing
  // the current argument quietly.
  // If the derived class do not want the cdrStream to skip writing
  // quietly, it should raise a MARSHAL system exception instead of
  // returning FALSE.

  virtual _CORBA_Boolean maybeReserveOutputSpace(omni::alignment_t align,
						 size_t required) = 0;
  // Same as reserverOutputSpaceForPrimitiveType, except the following:
  // 1. The required size can be any size.
  // 2. The implementation of this function can throw CORBA::BAD_PARAM
  //    if the nature of the stream makes it impossible to fullfil this
  //    request. The caller should fall back to use put_octet_array or
  //    other means instead.

  _OMNI_NS(omniCodeSet::TCS_C)* pd_tcs_c;
  // Transmission code set convertor for char and string

  _OMNI_NS(omniCodeSet::TCS_W)* pd_tcs_w;
  // Transmission code set convertor for wchar and wstring

  _OMNI_NS(omniCodeSet::NCS_C)* pd_ncs_c;
  _OMNI_NS(omniCodeSet::NCS_W)* pd_ncs_w;

public:

  // Access functions to the char and wchar code set convertors
  inline _OMNI_NS(omniCodeSet::TCS_C)* TCS_C() const { return pd_tcs_c; }
  inline void TCS_C(_OMNI_NS(omniCodeSet::TCS_C)* c) { pd_tcs_c = c; }
  inline _OMNI_NS(omniCodeSet::TCS_W)* TCS_W() const { return pd_tcs_w; }
  inline void TCS_W(_OMNI_NS(omniCodeSet::TCS_W)* c) { pd_tcs_w = c; }


  inline void
  unmarshalArrayChar(_CORBA_Short* a, int length)
  {
    for( int i = 0; i < length; i++ )
      a[i] = unmarshalChar();
  }

  inline void
  unmarshalArrayShort(_CORBA_Short* a, int length)
  {
    get_octet_array((_CORBA_Char*) a, length * 2, omni::ALIGN_2);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ )
	a[i] = Swap16(a[i]);
  }


  inline void
  unmarshalArrayUShort(_CORBA_UShort* a, int length)
  {
    get_octet_array((_CORBA_Char*) a, length * 2, omni::ALIGN_2);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ )
	a[i] = Swap16(a[i]);
  }

  inline void
  unmarshalArrayLong(_CORBA_Long* a, int length)
  {
    get_octet_array((_CORBA_Char*) a, length * 4, omni::ALIGN_4);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ )
	a[i] = Swap32(a[i]);
  }

  inline void
  unmarshalArrayULong(_CORBA_ULong* a, int length)
  {
    get_octet_array((_CORBA_Char*) a, length * 4, omni::ALIGN_4);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ )
	a[i] = Swap32(a[i]);
  }

#ifdef HAS_LongLong
  inline void
  unmarshalArrayLongLong(_CORBA_LongLong* a, int length)
  {
    get_octet_array((_CORBA_Char*) a, length * 8, omni::ALIGN_8);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ )
	a[i] = Swap64(a[i]);
  }

  inline void
  unmarshalArrayULongLong(_CORBA_ULongLong* a, int length)
  {
    get_octet_array((_CORBA_Char*) a, length * 8, omni::ALIGN_8);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ )
	a[i] = Swap64(a[i]);
  }
#endif

#if !defined(NO_FLOAT)
  inline void
  unmarshalArrayFloat(_CORBA_Float* a, int length)
  {
    get_octet_array((_CORBA_Char*) a, length * 4, omni::ALIGN_4);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ ) {
	union {
	  _CORBA_Float f;
	  _CORBA_ULong l;
	} u;
	u.f = a[i];
	u.l = Swap32(u.l);
	a[i] = u.f;
      }
  }


  inline void
  unmarshalArrayDouble(_CORBA_Double* a, int length)
  {
    get_octet_array((_CORBA_Char*) a, length * 8, omni::ALIGN_8);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ ) {
	union {
	  _CORBA_Double d;
	  _CORBA_ULong l[2];
	} u, v;
	u.d = a[i];
	v.l[0] = Swap32(u.l[1]);
	v.l[1] = Swap32(u.l[0]);
	a[i] = v.d;
      }
  }
#endif

public:
  /////////////////////////////////////////////////////////////////////
  virtual _CORBA_ULong completion();
  // If an error occurs when a value is marshalled or unmarshalled, a
  // system exception will be raised. The "completed" member of the
  // exception must be taken from the return value of this function.
  // Note: the return value is really of type CORBA::CompletionStatus.
  // Since this declaration must appear before the CORBA declaration,
  // we have to live with returning a _CORBA_ULong.

  /////////////////////////////////////////////////////////////////////
  virtual void* ptrToClass(int* cptr);
  // Yet another dynamic casting mechanism to cope with compilers
  // without dynamic_cast<>. Each class derived from cdrStream has a
  // static int declared in it. The address of this int is used to
  // compare the required class with the target object. Apart from the
  // use of the integer pointer, it works just like _ptrToInterface
  // and _ptrToObjRef in omniObjRef etc.

  static inline cdrStream* downcast(cdrStream* s) {
    return (cdrStream*)s->ptrToClass(&_classid);
  }
  // Not really necessary in the base class, but here for consistency.

  static _core_attr int _classid;

private:
  cdrStream(const cdrStream&);
  cdrStream& operator=(const cdrStream&);

  friend class cdrStreamAdapter;
  // cdrStreamAdapter needs to access protected pointers and virtual
  // functions.
};

#ifdef OMNI_NO_INLINE_FRIENDS

inline void operator>>= (_CORBA_Short a, cdrStream& s) {
  if (s.pd_marshal_byte_swap) {
    _CORBA_Short t = Swap16(a);
    a = t;
  }
  CdrMarshal(s,_CORBA_Short,omni::ALIGN_2,a);
}

inline void operator<<= (_CORBA_Short& a, cdrStream& s) {
  CdrUnMarshal(s,_CORBA_Short,omni::ALIGN_2,a);
  if (s.pd_unmarshal_byte_swap) {
    _CORBA_Short t = Swap16(a);
    a = t;
  }
}

inline void operator>>= (_CORBA_UShort a, cdrStream& s) {
  if (s.pd_marshal_byte_swap) {
    _CORBA_UShort t = Swap16(a);
    a = t;
  }
  CdrMarshal(s,_CORBA_UShort,omni::ALIGN_2,a);
}

inline void operator<<= (_CORBA_UShort& a, cdrStream& s) {
  CdrUnMarshal(s,_CORBA_UShort,omni::ALIGN_2,a);
  if (s.pd_unmarshal_byte_swap) {
    _CORBA_UShort t = Swap16(a);
    a = t;
  }
}

inline void operator>>= (_CORBA_Long a, cdrStream& s) {
  if (s.pd_marshal_byte_swap) {
    _CORBA_Long t = Swap32(a);
    a = t;
  }
  CdrMarshal(s,_CORBA_Long,omni::ALIGN_4,a);
}

inline void operator<<= (_CORBA_Long& a, cdrStream& s) {
  CdrUnMarshal(s,_CORBA_Long,omni::ALIGN_4,a);
  if (s.pd_unmarshal_byte_swap) {
    _CORBA_Long t = Swap32(a);
    a = t;
  }
}

inline void operator>>= (_CORBA_ULong a, cdrStream& s) {
  if (s.pd_marshal_byte_swap) {
    _CORBA_ULong t = Swap32(a);
    a = t;
  }
  CdrMarshal(s,_CORBA_ULong,omni::ALIGN_4,a);
}

inline void operator<<= (_CORBA_ULong& a, cdrStream& s) {
  CdrUnMarshal(s,_CORBA_ULong,omni::ALIGN_4,a);
  if (s.pd_unmarshal_byte_swap) {
    _CORBA_ULong t = Swap32(a);
    a = t;
  }
}

inline void operator>>= (_CORBA_LongLong a, cdrStream& s) {
  if (s.pd_marshal_byte_swap) {
    _CORBA_LongLong t = Swap64(a);
    a = t;
  }
  CdrMarshal(s,_CORBA_LongLong,omni::ALIGN_8,a);
}

inline void operator<<= (_CORBA_LongLong& a, cdrStream& s) {
  CdrUnMarshal(s,_CORBA_LongLong,omni::ALIGN_8,a);
  if (s.pd_unmarshal_byte_swap) {
    _CORBA_LongLong t = Swap64(a);
    a = t;
  }
}

inline void operator>>= (_CORBA_ULongLong a, cdrStream& s) {
  if (s.pd_marshal_byte_swap) {
    _CORBA_ULongLong t = Swap64(a);
    a = t;
  }
  CdrMarshal(s,_CORBA_ULongLong,omni::ALIGN_8,a);
}

inline void operator<<= (_CORBA_ULongLong& a, cdrStream& s) {
  CdrUnMarshal(s,_CORBA_ULongLong,omni::ALIGN_8,a);
  if (s.pd_unmarshal_byte_swap) {
    _CORBA_ULongLong t = Swap64(a);
    a = t;
  }
}

#if !defined(NO_FLOAT)

inline void operator>>= (_CORBA_Float a, cdrStream& s) {
  if (s.pd_marshal_byte_swap) {
    union {
      _CORBA_Float f;
      _CORBA_ULong l;
    } u;
    u.f = a;
    u.l = Swap32(u.l);
    a = u.f;
  }
  CdrMarshal(s,_CORBA_Float,omni::ALIGN_4,a);
}

inline void operator<<= (_CORBA_Float& a, cdrStream& s) {
  CdrUnMarshal(s,_CORBA_Float,omni::ALIGN_4,a);
  if (s.pd_unmarshal_byte_swap) {
    union {
      _CORBA_Float f;
      _CORBA_ULong l;
    } u;
    u.f = a;
    u.l = Swap32(u.l);
    a = u.f;
  }
}

inline void operator>>= (_CORBA_Double a, cdrStream& s) {
  if (s.pd_marshal_byte_swap) {
    union {
      _CORBA_Double d;
      _CORBA_ULong l[2];
    } u, v;
    u.d = a;
    v.l[0] = Swap32(u.l[1]);
    v.l[1] = Swap32(u.l[0]);
    a = v.d;
  }
  CdrMarshal(s,_CORBA_Double,omni::ALIGN_8,a);
}

inline void operator<<= (_CORBA_Double& a, cdrStream& s) {
  CdrUnMarshal(s,_CORBA_Double,omni::ALIGN_8,a);
  if (s.pd_unmarshal_byte_swap) {
    union {
      _CORBA_Double d;
      _CORBA_ULong l[2];
    } u, v;
    u.d = a;
    v.l[0] = Swap32(u.l[1]);
    v.l[1] = Swap32(u.l[0]);
    a = v.d;
  }
}

#endif

#ifdef HAS_LongDouble
#if SIZEOF_LONG_DOUBLE == 16
inline void operator>>= (_CORBA_LongDouble a, cdrStream& s) {
  if (s.pd_marshal_byte_swap) {
    union {
      _CORBA_LongDouble d;
      _CORBA_ULong l[4];
    } u, v;
    u.d = a;
    v.l[0] = Swap32(u.l[3]);
    v.l[1] = Swap32(u.l[2]);
    v.l[2] = Swap32(u.l[1]);
    v.l[3] = Swap32(u.l[0]);
    a = v.d;
  }
  CdrMarshal(s,_CORBA_LongDouble,omni::ALIGN_8,a);
}

inline void operator<<= (_CORBA_LongDouble& a, cdrStream& s) {
  CdrUnMarshal(s,_CORBA_LongDouble,omni::ALIGN_8,a);
  if (s.pd_unmarshal_byte_swap) {
    union {
      _CORBA_LongDouble d;
      _CORBA_ULong l[4];
    } u, v;
    u.d = a;
    v.l[0] = Swap32(u.l[3]);
    v.l[1] = Swap32(u.l[2]);
    v.l[2] = Swap32(u.l[1]);
    v.l[3] = Swap32(u.l[0]);
    a = v.d;
  }
}
#endif
#endif
 
#endif

#undef CdrMarshal
#undef CdrUnMarshal
#undef Swap16
#undef Swap32
#undef Swap64

class cdrMemoryStream : public cdrStream {
public:
  cdrMemoryStream(_CORBA_ULong initialBufsize = 0,
		  _CORBA_Boolean clearMemory = 0);
  ~cdrMemoryStream();

  void rewindInputPtr();
  // Rewind the input pointer to the beginning of the buffer

  void rewindPtrs();
  // Rewind the both input and output pointers to the beginning of the buffer
  // bufSize() returns 0 after this call.

  _CORBA_ULong bufSize() const;
  // Returns the size of the buffer containing valid data.

  void* bufPtr() const;
  // Returns a pointer to the beginning of the buffer.

  void setByteSwapFlag(_CORBA_Boolean littleendian);
  // Data in the buffer is little-endian (<littleendian> = TRUE(1)) or
  // big-endian (<littleendian> = FALSE(0)). Setup the cdrStream
  // accordingly.

  inline _CORBA_Boolean readOnly() { return pd_readonly_and_external_buffer; }
  // Return true if the stream is read-only -- i.e. it is using an
  // external buffer.

  cdrMemoryStream(const cdrMemoryStream&, _CORBA_Boolean read_only = 0);
  // Copy a stream. If the source stream is read-only, or <read_only>
  // is true, the new stream uses the same underlying memory buffer as
  // the source, and assumes the buffer stays valid for the lifetime
  // of the new stream.

  cdrMemoryStream& operator=(const cdrMemoryStream&);

  cdrMemoryStream(void* databuffer);
  cdrMemoryStream(void* databuffer, size_t maxLen);
  // Constructors for a read-only buffered stream.

  virtual void* ptrToClass(int* cptr);
  static inline cdrMemoryStream* downcast(cdrStream* s) {
    return (cdrMemoryStream*)s->ptrToClass(&_classid);
  }
  static _core_attr int _classid;

protected:
  _CORBA_Boolean pd_readonly_and_external_buffer;
  _CORBA_Boolean pd_clear_memory;
  char  pd_inline_buffer[32];
  void* pd_bufp;

public:
  // The following implement the abstract functions defined in cdrStream
  void put_octet_array(const _CORBA_Octet* b, int size,
		       omni::alignment_t align=omni::ALIGN_1);
  void get_octet_array(_CORBA_Octet* b,int size,
		       omni::alignment_t align=omni::ALIGN_1);
  void skipInput(_CORBA_ULong size);
  _CORBA_Boolean checkInputOverrun(_CORBA_ULong itemSize,
				   _CORBA_ULong nItems,
				   omni::alignment_t align=omni::ALIGN_1);
  _CORBA_Boolean checkOutputOverrun(_CORBA_ULong itemSize,
				    _CORBA_ULong nItems,
				    omni::alignment_t align=omni::ALIGN_1);
  void copy_to(cdrStream&,int size,
	       omni::alignment_t align=omni::ALIGN_1);
  void fetchInputData(omni::alignment_t,size_t);

  _CORBA_Boolean reserveOutputSpaceForPrimitiveType(omni::alignment_t,size_t);
  _CORBA_Boolean maybeReserveOutputSpace(omni::alignment_t,size_t);

  _CORBA_ULong currentInputPtr() const;
  _CORBA_ULong currentOutputPtr() const;

private:
  _CORBA_Boolean reserveOutputSpace(omni::alignment_t,size_t);
};

class cdrEncapsulationStream : public cdrMemoryStream {
public:
  cdrEncapsulationStream(_CORBA_ULong initialBufsize = 0,
			 _CORBA_Boolean clearMemory = 0);

  cdrEncapsulationStream(const _CORBA_Octet* databuffer,
			 _CORBA_ULong bufsize,
			 _CORBA_Boolean allowAlign4 = 0);

  cdrEncapsulationStream(cdrStream& s,_CORBA_ULong fetchsize);
  // copy from <s> <fetchsize> bytes of data.


  void getOctetStream(_CORBA_Octet*& databuffer, _CORBA_ULong& max,
		      _CORBA_ULong& len);

  virtual void* ptrToClass(int* cptr);
  static inline cdrEncapsulationStream* downcast(cdrStream* s) {
    return (cdrEncapsulationStream*)s->ptrToClass(&_classid);
  }
  static _core_attr int _classid;
};

class cdrCountingStream : public cdrStream {
public:
  cdrCountingStream(_OMNI_NS(omniCodeSet::TCS_C)* tcs_c,
		    _OMNI_NS(omniCodeSet::TCS_W)* tcs_w,
		    size_t initialoffset = 0) :
    pd_total(initialoffset) {
    pd_tcs_c = tcs_c;
    pd_tcs_w = tcs_w;
  }

  ~cdrCountingStream() {}

  size_t total() { return pd_total; }

  // The following implements the abstract functions defined in cdrStream
  void put_octet_array(const _CORBA_Octet* b, int size,
		       omni::alignment_t align=omni::ALIGN_1);

  _CORBA_Boolean reserveOutputSpaceForPrimitiveType(omni::alignment_t align,
						    size_t required);
  _CORBA_Boolean maybeReserveOutputSpace(omni::alignment_t align,
					 size_t required);

  _CORBA_Boolean checkOutputOverrun(_CORBA_ULong itemSize,
				    _CORBA_ULong nItems,
				    omni::alignment_t align=omni::ALIGN_1);

  void copy_to(cdrStream&,int size,
	       omni::alignment_t align=omni::ALIGN_1);

  void get_octet_array(_CORBA_Octet* b,int size,
		       omni::alignment_t align=omni::ALIGN_1);

  void skipInput(_CORBA_ULong);

  _CORBA_Boolean checkInputOverrun(_CORBA_ULong,_CORBA_ULong,
                                   omni::alignment_t align=omni::ALIGN_1);

  void fetchInputData(omni::alignment_t,size_t);

  _CORBA_ULong currentInputPtr() const;
  _CORBA_ULong currentOutputPtr() const;

  virtual void* ptrToClass(int* cptr);
  static inline cdrCountingStream* downcast(cdrStream* s) {
    return (cdrCountingStream*)s->ptrToClass(&_classid);
  }
  static _core_attr int _classid;

private:
  size_t pd_total;

  cdrCountingStream(const cdrCountingStream&);
  cdrCountingStream& operator=(const cdrCountingStream&);
};


// In some circumstances, for example in omniORBpy, it is necessary to
// perform some extra work around operations which manage a
// cdrStream's buffers. cdrStreamAdapter provides a wrapper around a
// cdrStream object. A class derived from cdrStreamAdapter may do
// anything it likes in its implementations of the cdrStream virtual
// functions, as long as it also calls the cdrStreamAdapter versions.

class cdrStreamAdapter : public cdrStream {
protected:
  cdrStreamAdapter(cdrStream& stream) :
    pd_actual(stream)
  {
    pd_unmarshal_byte_swap = pd_actual.pd_unmarshal_byte_swap;
    pd_marshal_byte_swap   = pd_actual.pd_marshal_byte_swap;
    pd_tcs_c               = pd_actual.pd_tcs_c;
    pd_tcs_w               = pd_actual.pd_tcs_w;
    copyStateFromActual();
  }

  virtual ~cdrStreamAdapter()
  {
    copyStateToActual();
  }

  // Implementations of abstract functions...
  void put_octet_array(const _CORBA_Octet* b, int size,
		       omni::alignment_t align=omni::ALIGN_1);

  void get_octet_array(_CORBA_Octet* b,int size,
		       omni::alignment_t align=omni::ALIGN_1);

  void skipInput(_CORBA_ULong size);

  _CORBA_Boolean checkInputOverrun(_CORBA_ULong itemSize,
				   _CORBA_ULong nItems,
				   omni::alignment_t align=omni::ALIGN_1);

  _CORBA_Boolean checkOutputOverrun(_CORBA_ULong itemSize,
				    _CORBA_ULong nItems,
				    omni::alignment_t align=omni::ALIGN_1);

  void copy_to(cdrStream&, int size, omni::alignment_t align=omni::ALIGN_1);

  void fetchInputData(omni::alignment_t align,size_t required);

  _CORBA_Boolean reserveOutputSpaceForPrimitiveType(omni::alignment_t align,
						    size_t required);

  _CORBA_Boolean maybeReserveOutputSpace(omni::alignment_t align,
					 size_t required);

  _CORBA_ULong currentInputPtr() const;
  _CORBA_ULong currentOutputPtr() const;

  _CORBA_ULong completion();

public:
  inline void copyStateFromActual()
  {
    pd_inb_end  = pd_actual.pd_inb_end;
    pd_inb_mkr  = pd_actual.pd_inb_mkr;
    pd_outb_end = pd_actual.pd_outb_end;
    pd_outb_mkr = pd_actual.pd_outb_mkr;
  }
  inline void copyStateToActual() const
  {
    pd_actual.pd_inb_end  = pd_inb_end;
    pd_actual.pd_inb_mkr  = pd_inb_mkr;
    pd_actual.pd_outb_end = pd_outb_end;
    pd_actual.pd_outb_mkr = pd_outb_mkr;
  }

  virtual void* ptrToClass(int* cptr);
  static inline cdrStreamAdapter* downcast(cdrStream* s) {
    return (cdrStreamAdapter*)s->ptrToClass(&_classid);
  }
  static _core_attr int _classid;

private:
  cdrStream& pd_actual;
};

#endif /* __CDRSTREAM_H__ */
