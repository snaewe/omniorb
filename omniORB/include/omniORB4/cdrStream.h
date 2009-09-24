// -*- Mode: C++; -*-
//                            Package   : omniORB
// cdrStream.h                Created on: 11/1/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2009 Apasphere Ltd
//    Copyright (C) 1999      AT&T Laboratories, Cambridge
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
//	*** PROPRIETARY INTERFACE ***
//

#ifndef __CDRSTREAM_H__
#define __CDRSTREAM_H__

#include <limits.h>

class cdrStream;
class cdrStreamAdapter;
class cdrValueChunkStream;

OMNI_NAMESPACE_BEGIN(omni)
  class ValueIndirectionTracker {
  public:
    virtual ~ValueIndirectionTracker();
  };
OMNI_NAMESPACE_END(omni)


//
// cdrStream abstract base class

class cdrStream {
public:

  cdrStream();

  virtual ~cdrStream();

  //
  // Marshalling macros

#ifndef CdrMarshal
#define CdrMarshal(type,align,reserveAndMarshal,arg) do { \
  omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr,align);\
  omni::ptr_arith_t p2 = p1 + sizeof(type);\
  if ((void*)p2 <= pd_outb_end) {\
    pd_outb_mkr = (void*) p2;\
    if (!pd_marshal_byte_swap) {\
      *((type*)p1) = arg;\
    }\
    else {\
      *((type*)p1) = byteSwap(arg);\
    }\
  }\
  else {\
    reserveAndMarshal(arg);\
  }\
} while(0)
#else
#error "CdrMarshal has already been defined"
#endif

#ifndef CdrUnMarshal
#define CdrUnMarshal(type,align,fetchAndUnmarshal,arg) do { \
  omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,align);\
  omni::ptr_arith_t p2 = p1 + sizeof(type);\
  if ((void*)p2 <= pd_inb_end) {\
    pd_inb_mkr = (void*) p2;\
    if (!pd_unmarshal_byte_swap) {\
      arg = *((type*)p1);\
    }\
    else {\
      arg = byteSwap(*((type*)p1));\
    }\
  }\
  else {\
    arg = fetchAndUnmarshal();\
  }\
} while(0)
#else
#error "CdrUnMarshal has already been defined"
#endif

  //
  // Byte swapping functions

  static inline _CORBA_Short byteSwap(_CORBA_Short s) {
    return (((s & 0xff00) >> 8 |
	     (s & 0x00ff) << 8));
  }
  static inline _CORBA_UShort byteSwap(_CORBA_UShort s) {
    return (((s & 0xff00) >> 8 |
	     (s & 0x00ff) << 8));
  }
  static inline _CORBA_Long byteSwap(_CORBA_Long l) {
    return (((l & 0xff000000) >> 24) |
	    ((l & 0x00ff0000) >> 8)  |
	    ((l & 0x0000ff00) << 8)  |
	    ((l & 0x000000ff) << 24));
  }
  static inline _CORBA_ULong byteSwap(_CORBA_ULong l) {
    return (((l & 0xff000000) >> 24) |
	    ((l & 0x00ff0000) >> 8)  |
	    ((l & 0x0000ff00) << 8)  |
	    ((l & 0x000000ff) << 24));
  }

#ifdef HAS_LongLong
  static inline _CORBA_LongLong byteSwap(_CORBA_LongLong l) {
    return (((l & _CORBA_LONGLONG_CONST(0xff00000000000000)) >> 56) |
	    ((l & _CORBA_LONGLONG_CONST(0x00ff000000000000)) >> 40) |
	    ((l & _CORBA_LONGLONG_CONST(0x0000ff0000000000)) >> 24) |
	    ((l & _CORBA_LONGLONG_CONST(0x000000ff00000000)) >> 8)  |
	    ((l & _CORBA_LONGLONG_CONST(0x00000000ff000000)) << 8)  |
	    ((l & _CORBA_LONGLONG_CONST(0x0000000000ff0000)) << 24) |
	    ((l & _CORBA_LONGLONG_CONST(0x000000000000ff00)) << 40) |
	    ((l & _CORBA_LONGLONG_CONST(0x00000000000000ff)) << 56));
  }
  static inline _CORBA_ULongLong byteSwap(_CORBA_ULongLong l) {
    return (((l & _CORBA_LONGLONG_CONST(0xff00000000000000)) >> 56) |
	    ((l & _CORBA_LONGLONG_CONST(0x00ff000000000000)) >> 40) |
	    ((l & _CORBA_LONGLONG_CONST(0x0000ff0000000000)) >> 24) |
	    ((l & _CORBA_LONGLONG_CONST(0x000000ff00000000)) >> 8)  |
	    ((l & _CORBA_LONGLONG_CONST(0x00000000ff000000)) << 8)  |
	    ((l & _CORBA_LONGLONG_CONST(0x0000000000ff0000)) << 24) |
	    ((l & _CORBA_LONGLONG_CONST(0x000000000000ff00)) << 40) |
	    ((l & _CORBA_LONGLONG_CONST(0x00000000000000ff)) << 56));
  }
#endif


  //
  // Marshalling methods : char, wchar, bool, octet

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
    // No need to align here
    omni::ptr_arith_t p1 = (omni::ptr_arith_t)pd_outb_mkr;
    omni::ptr_arith_t p2 = p1 + sizeof(_CORBA_Octet);
    if ((void*)p2 <= pd_outb_end) {
      pd_outb_mkr = (void*)p2;
      *((_CORBA_Octet*)p1) = a;
    }
    else {
      reserveAndMarshalOctet(a);
    }
  }

  inline _CORBA_Octet unmarshalOctet() {
    _CORBA_Octet a;
    omni::ptr_arith_t p1 = (omni::ptr_arith_t)pd_inb_mkr;
    omni::ptr_arith_t p2 = p1 + sizeof(_CORBA_Octet);
    if ((void*)p2 <= pd_inb_end) {
      pd_inb_mkr = (void*)p2;
      a = *((_CORBA_Octet*)p1);
    }
    else {
      a = fetchAndUnmarshalOctet();
    }
    return a;
  }

  inline void marshalBoolean(_CORBA_Boolean b) {
    _CORBA_Octet o = b ? 1 : 0;
    marshalOctet(o);
  }

  inline _CORBA_Boolean unmarshalBoolean() {
    _CORBA_Octet o = unmarshalOctet();
#ifdef HAS_Cplusplus_Bool
    return o ? true : false;
#else
    return (_CORBA_Boolean)o;
#endif
  }


  //
  // Marshalling methods : integer types

#define intMarshalFns(type,align) \
  inline void marshal ## type(_CORBA_ ## type a) { \
    CdrMarshal(_CORBA_ ## type, omni::align, reserveAndMarshal ## type, a); \
  } \
  inline _CORBA_ ## type unmarshal ## type() { \
    _CORBA_ ## type a; \
    CdrUnMarshal(_CORBA_ ## type, omni::align, fetchAndUnmarshal ## type , a); \
    return a; \
  }

  intMarshalFns(Short,  ALIGN_2)
  intMarshalFns(UShort, ALIGN_2)
  intMarshalFns(Long,   ALIGN_4)
  intMarshalFns(ULong,  ALIGN_4)  

#ifdef HAS_LongLong
  intMarshalFns(LongLong,  ALIGN_8)
  intMarshalFns(ULongLong, ALIGN_8)  
#endif

#undef intMarshalFns


  //
  // Marshalling methods : float types

#ifndef NO_FLOAT

#  ifndef USING_PROXY_FLOAT

  inline void marshalFloat(_CORBA_Float a) {
    union { _CORBA_Float a; _CORBA_ULong l; } u;
    u.a = a;
    marshalULong(u.l);
  }
  inline _CORBA_Float unmarshalFloat() {
    union { _CORBA_Float a; _CORBA_ULong l; } u;
    u.l = unmarshalULong();
    return u.a;
  }

#    if defined(HAS_LongLong) && !defined(OMNI_MIXED_ENDIAN_DOUBLE)

  inline void marshalDouble(_CORBA_Double a) {
    union { _CORBA_Double a; _CORBA_ULongLong l; } u;
    u.a = a;
    marshalULongLong(u.l);
  }
  inline _CORBA_Double unmarshalDouble() {
    union { _CORBA_Double a; _CORBA_ULongLong l; } u;
    u.l = unmarshalULongLong();
    return u.a;
  }

#    else  // No longlong or mixed endian

  inline void marshalDouble(_CORBA_Double a) {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr,
					  omni::ALIGN_8);
    omni::ptr_arith_t p2 = p1 + sizeof(_CORBA_Double);
    if ((void*)p2 <= pd_outb_end) {
      pd_outb_mkr = (void*) p2;

      struct LongArray2 { _CORBA_ULong l[2]; };
      union { _CORBA_Double a; LongArray2 l; } u;
      u.a = a;

#ifdef OMNI_MIXED_ENDIAN_DOUBLE
      {
	_CORBA_ULong v = u.l.l[0];
	u.l.l[0] = u.l.l[1];
	u.l.l[1] = v;
      }
#endif
      if (!pd_marshal_byte_swap) {
	*((LongArray2*)p1) = u.l;
      }
      else {
	LongArray2 m;
	m.l[0] = byteSwap(u.l.l[1]);
	m.l[1] = byteSwap(u.l.l[0]);
	*((LongArray2*)p1) = m;
      }
    }
    else {
      reserveAndMarshalDouble(a);
    }
  }

  inline _CORBA_Double unmarshalDouble() {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,
					  omni::ALIGN_8);
    omni::ptr_arith_t p2 = p1 + sizeof(_CORBA_Double);
    if ((void*)p2 <= pd_inb_end) {
      pd_inb_mkr = (void*) p2;

      struct LongArray2 { _CORBA_ULong l[2]; };
      union { _CORBA_Double a; LongArray2 l; } u;

      if (!pd_unmarshal_byte_swap) {
	u.l = *((LongArray2*)p1);
      }
      else {
	LongArray2 m = *((LongArray2*)p1);
	u.l.l[0] = byteSwap(m.l[1]);
	u.l.l[1] = byteSwap(m.l[0]);
      }

#ifdef OMNI_MIXED_ENDIAN_DOUBLE
      {
	_CORBA_ULong v = u.l.l[0];
	u.l.l[0] = u.l.l[1];
	u.l.l[1] = v;
      }
#endif
      return u.a;
    }
    else {
      return fetchAndUnmarshalDouble();
    }
  }

#    endif    // No longlong or mixed endian

#  else  // USING_PROXY_FLOAT

  // _CORBA_Float and _CORBA_Double are classes with constructors, so
  // we cannot use a union to convert it to an array of ulong. We use
  // reinterpret_cast instead.

  inline void marshalFloat(_CORBA_Float a) {
    _CORBA_ULong l = OMNI_REINTERPRET_CAST(_CORBA_ULong, a);
    marshalULong(l);
  }
  inline _CORBA_Float unmarshalFloat() {
    _CORBA_ULong l = unmarshalULong();
    return OMNI_REINTERPRET_CAST(_CORBA_Float, l);
  }

  inline void marshalDouble(_CORBA_Double a) {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr,
					  omni::ALIGN_8);
    omni::ptr_arith_t p2 = p1 + sizeof(_CORBA_Double);
    if ((void*)p2 <= pd_outb_end) {
      pd_outb_mkr = (void*) p2;

      struct LongArray2 { _CORBA_ULong l[2]; };

      LongArray2 l = OMNI_REINTERPRET_CAST(LongArray2, a);
      
      if (!pd_marshal_byte_swap) {
	*((LongArray2*)p1) = l;
      }
      else {
	LongArray2 m;
	m[0] = byteSwap(l[1]);
	m[1] = byteSwap(l[0]);
	*((LongArray2*)p1) = m;
      }
    }
    else {
      reserveAndMarshalDouble(a);
    }
  }

  inline _CORBA_Double unmarshalDouble() {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,
					  omni::ALIGN_8);
    omni::ptr_arith_t p2 = p1 + sizeof(_CORBA_Double);
    if ((void*)p2 <= pd_inb_end) {
      pd_inb_mkr = (void*) p2;

      struct LongArray2 { _CORBA_ULong l[2]; };

      LongArray2 l;

      if (!pd_unmarshal_byte_swap) {
	l = *((LongArray2*)p1);
      }
      else {
	LongArray2 m = *((LongArray2*)p1);
	l.l[0] = byteSwap(m.l[1]);
	l.l[1] = byteSwap(m.l[0]);
      }
      return OMNI_REINTERPRET_CAST(_CORBA_Double, l);
    }
    else {
      return fetchAndUnmarshalDouble();
    }
  }

#  endif  // USING_PROXY_FLOAT


#  if defined(HAS_LongDouble) && defined(HAS_LongLong)

  // We only support LongDouble if we also have LongLong.

#    if SIZEOF_LONG_DOUBLE == 16

  inline void marshalLongDouble(_CORBA_LongDouble a) {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr,
					  omni::ALIGN_8);
    omni::ptr_arith_t p2 = p1 + sizeof(_CORBA_LongDouble);
    if ((void*)p2 <= pd_outb_end) {
      pd_outb_mkr = (void*) p2;

      struct LongLongArray2 { _CORBA_ULongLong l[2]; };
      union { _CORBA_LongDouble a; LongLongArray2 l; } u;
      u.a = a;

      if (!pd_marshal_byte_swap) {
	*((LongLongArray2*)p1) = u.l;
      }
      else {
	LongLongArray2 m;
	m.l[0] = byteSwap(u.l.l[1]);
	m.l[1] = byteSwap(u.l.l[0]);
	*((LongLongArray2*)p1) = m;
      }
    }
    else {
      reserveAndMarshalLongDouble(a);
    }
  }

  inline _CORBA_LongDouble unmarshalLongDouble() {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,
					  omni::ALIGN_8);
    omni::ptr_arith_t p2 = p1 + sizeof(_CORBA_Double);
    if ((void*)p2 <= pd_inb_end) {
      pd_inb_mkr = (void*) p2;

      struct LongLongArray2 { _CORBA_ULongLong l[2]; };
      union { _CORBA_LongDouble a; LongLongArray2 l; } u;

      if (!pd_unmarshal_byte_swap) {
	u.l = *((LongLongArray2*)p1);
      }
      else {
	LongLongArray2 m = *((LongLongArray2*)p1);
	u.l.l[0] = byteSwap(m.l[1]);
	u.l.l[1] = byteSwap(m.l[0]);
      }
      return u.a;
    }
    else {
      return fetchAndUnmarshalLongDouble();
    }
  }

#    else
  // Code for long double < 16 bytes is too painful to put inline
  void marshalLongDouble(_CORBA_LongDouble a);
  _CORBA_LongDouble unmarshalLongDouble();
#    endif

#  endif  // HAS_LongDouble

#endif // NO_FLOAT


  //
  // Marshalling methods : string types

  inline void marshalString(const char* s,int bounded=0) {
    OMNIORB_USER_CHECK(s);
    pd_ncs_c->marshalString(*this,pd_tcs_c,bounded,0,s);
  }

  inline char* unmarshalString(int bounded=0) {
    char* s;
    pd_ncs_c->unmarshalString(*this,pd_tcs_c,bounded,s);
    return s;
  }

  _CORBA_ULong marshalRawString(const char* s);
  // Marshal a raw string, with no code set conversion.

  char* unmarshalRawString();
  // Unmarshal a raw string.

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


  //
  // Pure virtual functions

  virtual void put_octet_array(const _CORBA_Octet* b, int size,
			       omni::alignment_t align=omni::ALIGN_1) = 0;
  // Align output then put array of octets.
  // <size> must be a multiple of <align>.
  // For instance, if <align> == omni::ALIGN_8 then <size> % 8 == 0.

  void put_small_octet_array(const _CORBA_Octet* b, int size);
  // Put a small octet array which must have ALIGN_1. Since it is
  // small, we expect it to fit in the stream's current buffer without
  // having to allocate more space.

  virtual void get_octet_array(_CORBA_Octet* b, int size,
			       omni::alignment_t align=omni::ALIGN_1) = 0;
  // Get array of octets.

  virtual void skipInput(_CORBA_ULong size) = 0;
  // Skip <size> bytes from the input stream.

  virtual _CORBA_Boolean checkInputOverrun(_CORBA_ULong itemSize,
				_CORBA_ULong nItems,
				 omni::alignment_t align=omni::ALIGN_1) = 0;
  // Return true if the input stream contains data for at least
  // <nitems> of size <itemSize>. The initial alignment of the data
  // starts at <align>. Return false otherwise.

  virtual _CORBA_ULong currentInputPtr() const = 0;
  // Return a value that represents the position of the next byte in
  // the input stream. Later bytes in the stream has a higher return
  // value. The absolute value of the return value has no meaning.
  // The only use of this function is to compute the distance between
  // two bytes in the stream.

  virtual _CORBA_ULong currentOutputPtr() const = 0;
  // Return a value that represents the position of the next byte in
  // the output stream. Later bytes in the stream have a higher return
  // value.  The absolute value of the return value has no meaning.
  // The only use of this function is to compute the distance between
  // two bytes in the stream.


  virtual _CORBA_Boolean checkOutputOverrun(_CORBA_ULong itemSize,
				_CORBA_ULong nItems,
				omni::alignment_t align=omni::ALIGN_1) = 0;
  // Return true if data of at least <nitems> of size <itemSize> can
  // be inserted to the output stream. The initial alignment of the
  // data starts at <align>. Return false otherwise.

  virtual void copy_to(cdrStream&,int size,
		       omni::alignment_t align=omni::ALIGN_1);
  // From this stream, copy <size> bytes to the argument stream.
  // The initial alignment starts at <align>.
  // Derived classes may provided more efficent implementation than the
  // default.


  inline void alignInput(omni::alignment_t align)
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

  inline void alignOutput(omni::alignment_t align)
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

  inline _CORBA_Boolean
  unmarshal_byte_swap() const { return pd_unmarshal_byte_swap; }
  // Return TRUE(1) if unmarshalled data have to be byte-swapped.

  inline _CORBA_Boolean
  marshal_byte_swap() const { return pd_marshal_byte_swap; }

  inline void*
  PR_get_outb_mkr() { return pd_outb_mkr; }
  // internal function used by cdrStream implementations


  //
  // Marshalling methods: array types

  inline void
  unmarshalArrayChar(_CORBA_Char* a, int length)
  {
    for (int i = 0; i < length; i++)
      a[i] = unmarshalChar();
  }

  inline void
  unmarshalArrayBoolean(_CORBA_Boolean* a, int length)
  {
#if !defined(HAS_Cplusplus_Bool) || (SIZEOF_BOOL == 1)
    get_octet_array((_CORBA_Char*)a, length, omni::ALIGN_1);
#else
    for (int i = 0; i < length; i++)
      a[i] = unmarshalBoolean();
#endif
  }

  inline void
  unmarshalArrayShort(_CORBA_Short* a, int length)
  {
    get_octet_array((_CORBA_Char*)a, length * 2, omni::ALIGN_2);

    if (unmarshal_byte_swap())
      for (int i = 0; i < length; i++)
	a[i] = byteSwap(a[i]);
  }


  inline void
  unmarshalArrayUShort(_CORBA_UShort* a, int length)
  {
    get_octet_array((_CORBA_Char*)a, length * 2, omni::ALIGN_2);

    if (unmarshal_byte_swap())
      for (int i = 0; i < length; i++)
	a[i] = byteSwap(a[i]);
  }

  inline void
  unmarshalArrayLong(_CORBA_Long* a, int length)
  {
    get_octet_array((_CORBA_Char*)a, length * 4, omni::ALIGN_4);

    if (unmarshal_byte_swap())
      for (int i = 0; i < length; i++)
	a[i] = byteSwap(a[i]);
  }

  inline void
  unmarshalArrayULong(_CORBA_ULong* a, int length)
  {
    get_octet_array((_CORBA_Char*)a, length * 4, omni::ALIGN_4);

    if (unmarshal_byte_swap())
      for (int i = 0; i < length; i++)
	a[i] = byteSwap(a[i]);
  }

#ifdef HAS_LongLong
  inline void
  unmarshalArrayLongLong(_CORBA_LongLong* a, int length)
  {
    get_octet_array((_CORBA_Char*)a, length * 8, omni::ALIGN_8);

    if (unmarshal_byte_swap())
      for (int i = 0; i < length; i++)
	a[i] = byteSwap(a[i]);
  }

  inline void
  unmarshalArrayULongLong(_CORBA_ULongLong* a, int length)
  {
    get_octet_array((_CORBA_Char*)a, length * 8, omni::ALIGN_8);

    if (unmarshal_byte_swap())
      for (int i = 0; i < length; i++)
	a[i] = byteSwap(a[i]);
  }
#endif

#if !defined(NO_FLOAT)
  inline void
  unmarshalArrayFloat(_CORBA_Float* a, int length)
  {
    get_octet_array((_CORBA_Char*)a, length * 4, omni::ALIGN_4);

    if (unmarshal_byte_swap()) {
      _CORBA_ULong* p=(_CORBA_ULong*)a;
      for( int i = 0; i < length; i++ ) {
	p[i] = byteSwap(p[i]);
      }
    }
  }


  inline void
  unmarshalArrayDouble(_CORBA_Double* a, int length)
  {
    get_octet_array((_CORBA_Char*) a, length * 8, omni::ALIGN_8);

    if (unmarshal_byte_swap()) {

#  if defined(Has_Longlong) && !defined (OMNI_MIXED_ENDIAN_DOUBLE)
      _CORBA_ULongLong* p=(_CORBA_ULongLong*)a;
      for( int i = 0; i < length; i++ ) {
	p[i] = byteSwap(p[i]);
      }
#  else
      struct LongArray2 {
        _CORBA_ULong l[2];
      };
      LongArray2* p=(LongArray2*)a;
      for (int i = 0; i < length; i++) {
        LongArray2 l;
        l.l[0] = byteSwap(p[i].l[1]);
        l.l[1] = byteSwap(p[i].l[0]);
        p[i] = l;
      }
#  endif
    }
#  ifdef OMNI_MIXED_ENDIAN_DOUBLE
    {
      struct LongArray2 {
        _CORBA_ULong l[2];
      };
      LongArray2* p=(LongArray2*)a;
      for( int i = 0; i < length; i++ ) {
        _CORBA_ULong v = p[i].l[0];
	p[i].l[0] = p[i].l[1];
	p[i].l[1] = v;
      }
    }
#  endif
  }
#endif

protected:

  _CORBA_Boolean pd_unmarshal_byte_swap;
  // TRUE(1) if unmarshalled data have to be byte-swapped.

  _CORBA_Boolean pd_marshal_byte_swap;
  // TRUE(1) if marshalled data have to be byte-swapped. i.e. data are not
  // going into the stream with the host endian.

  //  Input buffer pointers, the region (*p) containing valid data
  //  is pd_inb_mkr <= p < pd_inb_end.
  //  pd_inb_end and pd_inb_mkr are initialised by fetchInputData().
  //  pd_inb_mkr is also updated by the unmarshalling operators between
  //  calls to fetchInputData().
  void* pd_inb_end;
  void* pd_inb_mkr;

  virtual void fetchInputData(omni::alignment_t align, size_t required) = 0;
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
  // Same as reserveOutputSpaceForPrimitiveType, except the following:
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

  _OMNI_NS(ValueIndirectionTracker)* pd_valueTracker;
  // Object used to track offsets of indirections in valuetypes.


private:
  //
  // Marshalling functions used when there is not enough buffer space
  // for the inline versions.

#define fetchReserveMarshalFns(type) \
  void reserveAndMarshal ## type(_CORBA_ ## type a); \
  _CORBA_ ## type fetchAndUnmarshal ## type()

  fetchReserveMarshalFns(Octet);
  fetchReserveMarshalFns(Short);
  fetchReserveMarshalFns(UShort);
  fetchReserveMarshalFns(Long);
  fetchReserveMarshalFns(ULong);
#ifdef HAS_LongLong
  fetchReserveMarshalFns(LongLong);
  fetchReserveMarshalFns(ULongLong);
#endif
#ifndef NO_FLOAT
  fetchReserveMarshalFns(Double);
#  if defined(HAS_LongDouble) && defined(HAS_LongLong)
  fetchReserveMarshalFns(LongDouble);
#  endif
#endif

#undef fetchReserveMarshalFns

public:

  // Access functions to the char and wchar code set convertors
  inline _OMNI_NS(omniCodeSet::TCS_C)* TCS_C() const { return pd_tcs_c; }
  inline void TCS_C(_OMNI_NS(omniCodeSet::TCS_C)* c) { pd_tcs_c = c; }
  inline _OMNI_NS(omniCodeSet::TCS_W)* TCS_W() const { return pd_tcs_w; }
  inline void TCS_W(_OMNI_NS(omniCodeSet::TCS_W)* c) { pd_tcs_w = c; }


  //
  // Valuetype related methods

  inline _OMNI_NS(ValueIndirectionTracker)* valueTracker() const {
    return pd_valueTracker;
  }
  inline void valueTracker(_OMNI_NS(ValueIndirectionTracker)* v) {
    pd_valueTracker = v;
  }
  inline void clearValueTracker() {
    if (pd_valueTracker) {
      delete pd_valueTracker;
      pd_valueTracker = 0;
    }
  }

  virtual void declareArrayLength(omni::alignment_t align, size_t size);
  // Declare that an array of size <size> octets, alignment <align> is
  // about to be marshalled. This allows chunked streams to
  // pre-allocate a suitable sized chunk.


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
  friend class cdrValueChunkStream;
  // cdrStreamAdapter and cdrValueChunkStream need to access protected
  // pointers and virtual functions.
};


//
// Marshalling operators

#define marshallingOperators(type) \
inline void operator>>= (_CORBA_ ## type a, cdrStream& s) {\
  s.marshal ## type(a);\
} \
inline void operator<<= (_CORBA_ ## type& a, cdrStream& s) {\
  a = s.unmarshal ## type();\
}

marshallingOperators(Short)
marshallingOperators(UShort)
marshallingOperators(Long)
marshallingOperators(ULong)
#ifdef HAS_LongLong
marshallingOperators(LongLong)
marshallingOperators(ULongLong)
#endif
#if !defined(NO_FLOAT)
marshallingOperators(Float)
marshallingOperators(Double)
#  if defined(HAS_LongDouble) && defined(HAS_LongLong)
marshallingOperators(LongDouble)
#  endif
#endif

#undef marshallingOperators


//
// Memory buffered stream

class cdrMemoryStream : public cdrStream {
public:
  cdrMemoryStream(_CORBA_ULong initialBufsize = 0,
		  _CORBA_Boolean clearMemory = 0);
  virtual ~cdrMemoryStream();

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
  void* pd_bufp;
  void* pd_bufp_8;
  char  pd_inline_buffer[32];

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


//
// Specialisation of memory stream to handle CDR encapsulations

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


//
// Fake stream that counts how many octets are marshalled

class cdrCountingStream : public cdrStream {
public:
  cdrCountingStream(_OMNI_NS(omniCodeSet::TCS_C)* tcs_c,
		    _OMNI_NS(omniCodeSet::TCS_W)* tcs_w,
		    size_t initialoffset = 0) :
    pd_total(initialoffset) {
    pd_tcs_c = tcs_c;
    pd_tcs_w = tcs_w;
  }

  virtual ~cdrCountingStream() {}

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


//
// Stream adapter

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
    pd_valueTracker = 0;
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
    pd_inb_end      = pd_actual.pd_inb_end;
    pd_inb_mkr      = pd_actual.pd_inb_mkr;
    pd_outb_end     = pd_actual.pd_outb_end;
    pd_outb_mkr     = pd_actual.pd_outb_mkr;
    pd_valueTracker = pd_actual.pd_valueTracker;
  }
  inline void copyStateToActual() const
  {
    pd_actual.pd_inb_end      = pd_inb_end;
    pd_actual.pd_inb_mkr      = pd_inb_mkr;
    pd_actual.pd_outb_end     = pd_outb_end;
    pd_actual.pd_outb_mkr     = pd_outb_mkr;
    pd_actual.pd_valueTracker = pd_valueTracker;
  }

  virtual void* ptrToClass(int* cptr);
  static inline cdrStreamAdapter* downcast(cdrStream* s) {
    return (cdrStreamAdapter*)s->ptrToClass(&_classid);
  }
  static _core_attr int _classid;

private:
  cdrStream& pd_actual;
};


//
// Valuetype support

// cdrValueChunkStream is similar to cdrStreamAdapter. It implements
// chunked encoding of valuetypes by wrapping an existing stream.
//
// Use for reading and writing is slightly asymmetric: for writing,
// the stream wrapper must be created before marshalling the value tag
// indicating chunked encoding; for reading, the wrapper is created
// after unmarshalling the outer-most value tag (since it is not until
// then that the reader knows chunking is is use).

class cdrValueChunkStream : public cdrStream {
public:
  cdrValueChunkStream(cdrStream& stream) :
    pd_actual(stream), pd_nestLevel(0), pd_lengthPtr(0),
    pd_remaining(0), pd_inHeader(0), pd_inChunk(0), pd_justEnded(0),
    pd_reader(0), pd_exception(0)
  {
    pd_unmarshal_byte_swap = pd_actual.pd_unmarshal_byte_swap;
    pd_marshal_byte_swap   = pd_actual.pd_marshal_byte_swap;
    pd_tcs_c               = pd_actual.pd_tcs_c;
    pd_tcs_w               = pd_actual.pd_tcs_w;

    copyStateFromActual();
  }

  virtual ~cdrValueChunkStream();

  void startOutputValueHeader(_CORBA_Long valueTag);
  // Start a new value with the given value tag. The value tag must
  // specify chunking. Must be called at least once before using the
  // stream for writing.

  void startOutputValueBody();
  // End the value header and start a chunk for the value body.

  void endOutputValue();
  // End the current value, decrementing the nesting level.

  void initialiseInput();
  // Initialise stream as an input stream.

  void startInputValueBody();
  // Start reading the value body.


  inline _CORBA_Long nestLevel() {
    return pd_nestLevel;
  }

  inline void exceptionOccurred()
  {
    pd_exception = 1;
  }

  _CORBA_Boolean skipToNestedValue(_CORBA_Long level);
  // Function used by truncation. Skip the remaining octets in the
  // value at the specified nesting level. If a nested value is
  // encountered, return true; if the end of the value is reached,
  // return false.
  //
  // We need this because we might truncate a value containing a
  // member that is a value we know about. Later on, an indirection
  // might point to that member.


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

  void fetchInputData(omni::alignment_t align,size_t required);

  _CORBA_Boolean reserveOutputSpaceForPrimitiveType(omni::alignment_t align,
						    size_t required);

  _CORBA_Boolean maybeReserveOutputSpace(omni::alignment_t align,
					 size_t required);

  _CORBA_ULong currentInputPtr() const;
  _CORBA_ULong currentOutputPtr() const;

  _CORBA_ULong completion();

  virtual void declareArrayLength(omni::alignment_t align, size_t size);

  inline void copyStateFromActual()
  {
    pd_inb_end      = pd_actual.pd_inb_end;
    pd_inb_mkr      = pd_actual.pd_inb_mkr;
    pd_outb_end     = pd_actual.pd_outb_end;
    pd_outb_mkr     = pd_actual.pd_outb_mkr;
    pd_valueTracker = pd_actual.pd_valueTracker;
  }
  inline void copyStateToActual() const
  {
    pd_actual.pd_inb_mkr      = pd_inb_mkr;
    pd_actual.pd_outb_mkr     = pd_outb_mkr;
    pd_actual.pd_valueTracker = pd_valueTracker;
  }

  static _core_attr int _classid;
  virtual void* ptrToClass(int* cptr);
  static inline cdrValueChunkStream* downcast(cdrStream* s) {
    return (cdrValueChunkStream*)s->ptrToClass(&_classid);
  }

private:
  void startOutputChunk();
  void endOutputChunk();

  void maybeStartNewChunk(omni::alignment_t align, size_t size);
  // Start a new chunk by doing the equivalent of endOutputChunk,
  // startOutputChunk, unless ending the chunk now would cause us to
  // output a zero length chunk. In that case, we use
  // declareArrayLength to reserve space in the chunk for an element
  // of the specified size.

  void startInputChunk();
  void endInputValue();

  _CORBA_Long peekChunkTag();
  // Retrieve a chunk tag from the stream without moving the pointers along.

  inline void setLength(_CORBA_ULong len)
  {
    *pd_lengthPtr = pd_marshal_byte_swap ? byteSwap(len) : len;
  }

  inline _CORBA_ULong getLength()
  {
    return pd_unmarshal_byte_swap ? byteSwap(*pd_lengthPtr) : *pd_lengthPtr;
  }

  cdrStream&     pd_actual;    // Stream being wrapped
  _CORBA_Long    pd_nestLevel; // The nesting level of chunks
  _CORBA_Long*   pd_lengthPtr; // Pointer to the chunk length field
  _CORBA_ULong   pd_remaining; // !=0 => octets remaining in chunk
  _CORBA_Boolean pd_inHeader;  // True if we're inside a value header
  _CORBA_Boolean pd_inChunk;   // True if we're inside a chunk
  _CORBA_Boolean pd_justEnded; // True if we've just ended a value
  _CORBA_Boolean pd_reader;    // True if we're a reader not a writer
  _CORBA_Boolean pd_exception; // Set true if an exception occurs, to
			       // prevent further exceptions during
			       // clean-up.
};


#undef CdrMarshal
#undef CdrUnMarshal

#endif /* __CDRSTREAM_H__ */
