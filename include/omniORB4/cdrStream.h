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



class cdrStream {
public:

  cdrStream() : pd_unmarshal_byte_swap(0), pd_marshal_byte_swap(0),
                pd_inb_end(0), pd_inb_mkr(0),
                pd_outb_end(0), pd_outb_mkr(0) {}
  virtual ~cdrStream() {}

#ifndef CdrMarshal
#define CdrMarshal(s,type,align,arg) do {\
   again: \
   omni::ptr_arith_t p1 =omni::align_to((omni::ptr_arith_t)s.pd_outb_mkr,align);\
   omni::ptr_arith_t p2 = p1 + sizeof(type);\
   if( (void*)p2 > s.pd_outb_end ) {\
     if (s.reserveOutputSpace(align,sizeof(type)))\
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

  friend inline void operator>>= (_CORBA_Char a, cdrStream& s) {
    CdrMarshal(s,_CORBA_Char,omni::ALIGN_1,a);
  }

  friend inline void operator<<= (_CORBA_Char& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_Char,omni::ALIGN_1,a);
  }

#ifdef HAS_Cplusplus_Bool

  friend inline void operator>>= (_CORBA_Boolean b, cdrStream& s) {
    _CORBA_Char c = b ? 1 : 0;
    CdrMarshal(s,_CORBA_Char,omni::ALIGN_1,c);
  }

  friend inline void operator<<= (_CORBA_Boolean& b, cdrStream& s) {
    _CORBA_Char c;
    CdrUnMarshal(s,_CORBA_Char,omni::ALIGN_1,c);
    b = c ? true : false;
  }

#endif

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

#if !defined(NO_FLOAT)

  friend inline void operator>>= (_CORBA_Float a, cdrStream& s) {
    if (s.pd_marshal_byte_swap) {
      _CORBA_ULong tl1 = *((_CORBA_ULong *)&a);
      _CORBA_ULong tl2 = Swap32(tl1);
      a = *((_CORBA_Float *) &tl2);
    }
    CdrMarshal(s,_CORBA_Float,omni::ALIGN_4,a);
  }

  friend inline void operator<<= (_CORBA_Float& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_Float,omni::ALIGN_4,a);
    if (s.pd_unmarshal_byte_swap) {
      _CORBA_ULong tl1 = *((_CORBA_ULong *)&a);
      _CORBA_ULong tl2 = Swap32(tl1);
      a = *((_CORBA_Float *) &tl2);
    }
  }

  friend inline void operator>>= (_CORBA_Double a, cdrStream& s) {
    if (s.pd_marshal_byte_swap) {
      _CORBA_Double t = a;
      _CORBA_ULong tl1 = ((_CORBA_ULong *)&t)[1];
      _CORBA_ULong tl2 = Swap32(tl1);
      ((_CORBA_ULong *)&a)[0] = tl2;
      tl1 = ((_CORBA_ULong *)&t)[0];
      tl2 = Swap32(tl1);
      ((_CORBA_ULong *)&a)[1] = tl2;
    }
    CdrMarshal(s,_CORBA_Double,omni::ALIGN_8,a);
  }

  friend inline void operator<<= (_CORBA_Double& a, cdrStream& s) {
    CdrUnMarshal(s,_CORBA_Double,omni::ALIGN_8,a);
    if (s.pd_unmarshal_byte_swap) {
      _CORBA_Double t = a;
      _CORBA_ULong tl1 = ((_CORBA_ULong *)&t)[1];
      _CORBA_ULong tl2 = Swap32(tl1);
      ((_CORBA_ULong *)&a)[0] = tl2;
      tl1 = ((_CORBA_ULong *)&t)[0];
      tl2 = Swap32(tl1);
      ((_CORBA_ULong *)&a)[1] = tl2;
    }
  }

#endif

  virtual void put_char_array(const _CORBA_Char* b, int size,
			      omni::alignment_t align=omni::ALIGN_1) = 0;
  // <size> must be a multiple of <align>.
  // For instance, if <align> == omni::ALIGN_8 then <size> % 8 == 0.
    
  virtual void get_char_array(_CORBA_Char* b,int size,
			      omni::alignment_t align=omni::ALIGN_1) = 0;
  // <size> must be a multiple of <align>.
  // For instance, if <align> == omni::ALIGN_8 then <size> % 8 == 0.

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

  inline
  void copy_to(cdrStream& s, size_t size,
	       omni::alignment_t align=omni::ALIGN_1)
  // copy <size> bytes from this stream to the given stream. Both streams
  // are first moved on to the appropriate alignment.
  //
  // This function should be used only if the data block
  // consists of homogeneous elements of the same alignment as specified
  // in <align>. Otherwise, the data copied may end up with the wrong
  // alignment.
  {
    if (!checkInputOverrun(1,size,align) ||
	!s.checkOutputOverrun(1,size,align)) 
      {
	_CORBA_marshal_error();
	// Do not reach here
      }
    if (align != omni::ALIGN_1) {
      alignInput(align);
      s.alignOutput(align);
    }
    while(size) {
      size_t transfersz = size;
      if (transfersz > maxFetchInputData(omni::ALIGN_1))
	transfersz = maxFetchInputData(omni::ALIGN_1);
      if (transfersz > s.maxReserveOutputSpace(omni::ALIGN_1))
	transfersz = s.maxReserveOutputSpace(omni::ALIGN_1);
      if (!transfersz) _CORBA_marshal_error();
      fetchInputData(omni::ALIGN_1,transfersz);
      if (s.reserveOutputSpace(omni::ALIGN_1,transfersz)) {
	memcpy(s.pd_outb_mkr,pd_inb_mkr,transfersz);
      }
      s.pd_outb_mkr = (void*)((omni::ptr_arith_t)s.pd_outb_mkr + transfersz);
      pd_inb_mkr = (void*)((omni::ptr_arith_t)pd_inb_mkr + transfersz);
      size -= transfersz;
    }
  }

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
	if (reserveOutputSpace(align,0))
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
  // The data block should start at alignment <align>. 
  // If the space available is less than specified, raise a
  // MARSHAL system exception.

  virtual size_t maxFetchInputData(omni::alignment_t align) const = 0;
  // Return the maximum size that can be asked with fetchInputData().
  // The data block should start at alignment <align>.
  // This value is valid until the next fetchInputData().
  
  //  Output buffer pointers, the region (*p) that can be written
  //  into is pd_outb_mkr <= p < pd_outb_end.
  //  pd_outb_end and pd_outb_mkr are initialised by reserveOutputSpace().
  //  pd_outb_mkr is also updated by the marshalling operators between
  //  calls to reserveOutputSpace(). 
  void* pd_outb_end;
  void* pd_outb_mkr;

  virtual _CORBA_Boolean reserveOutputSpace(omni::alignment_t align,
					    size_t required) = 0;
  // Allocate at least <required> bytes in the output buffer.
  // The data block should start at alignment <align>.
  // Returns TRUE(1) if at least <required> bytes with the starting
  // alignment as specified has been allocated.
  // Returns FALSE(0) if the required space cannot be allocated.
  // When the return value is FALSE, the cdrStream would skip writing
  // the current argument quietly.
  // If the derived class do not want the cdrStream to skip writing
  // quietly, it should raise a MARSHAL system exception instead of 
  // returning FALSE.

  virtual size_t maxReserveOutputSpace(omni::alignment_t align) const = 0;
  // Return the maximum size that can be asked with reserveOutputSpace().
  // The data block should start at alignment <align>.


public:

  inline void
  unmarshalArrayShort(_CORBA_Short* a, int length)
  {
    get_char_array((_CORBA_Char*) a, length * 2, omni::ALIGN_2);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ )
	a[i] = Swap16(a[i]);
  }


  inline void
  unmarshalArrayUShort(_CORBA_UShort* a, int length)
  {
    get_char_array((_CORBA_Char*) a, length * 2, omni::ALIGN_2);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ )
	a[i] = Swap16(a[i]);
  }

  inline void
  unmarshalArrayLong(_CORBA_Long* a, int length)
  {
    get_char_array((_CORBA_Char*) a, length * 4, omni::ALIGN_4);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ )
	a[i] = Swap32(a[i]);
  }

  inline void
  unmarshalArrayULong(_CORBA_ULong* a, int length)
  {
    get_char_array((_CORBA_Char*) a, length * 4, omni::ALIGN_4);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ )
	a[i] = Swap32(a[i]);
  }

#if !defined(NO_FLOAT)
  inline void
  unmarshalArrayFloat(_CORBA_Float* a, int length)
  {
    get_char_array((_CORBA_Char*) a, length * 4, omni::ALIGN_4);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ ) {
	_CORBA_ULong tmp = Swap32(* (_CORBA_ULong*) (a + i));
	a[i] = * (_CORBA_Float*) &tmp;
      }
  }


  inline void
  unmarshalArrayDouble(_CORBA_Double* a, int length)
  {
    get_char_array((_CORBA_Char*) a, length * 8, omni::ALIGN_8);

    if( unmarshal_byte_swap() )
      for( int i = 0; i < length; i++ ) {
	_CORBA_ULong tmp0 = Swap32(((_CORBA_ULong*) (a + i))[0]);
	_CORBA_ULong tmp1 = Swap32(((_CORBA_ULong*) (a + i))[1]);
	((_CORBA_ULong*) (a + i))[1] = tmp0;
	((_CORBA_ULong*) (a + i))[0] = tmp1;
      }
  }
#endif

private:
  cdrStream(const cdrStream&);
  cdrStream& operator=(const cdrStream&);
};

#undef CdrMarshal
#undef CdrUnMarshal
#undef Swap16
#undef Swap32

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

  cdrMemoryStream(const cdrMemoryStream&);
  cdrMemoryStream& operator=(const cdrMemoryStream&);

  cdrMemoryStream(void* databuffer);
  cdrMemoryStream(void* databuffer, size_t maxLen);
  // Constructors for a read-only buffered stream.


protected:
  _CORBA_Boolean pd_readonly_and_external_buffer;
  _CORBA_Boolean pd_clear_memory;
  char  pd_inline_buffer[32];
  void* pd_bufp;

public:
  // The following implement the abstract functions defined in cdrStream
  void put_char_array(const _CORBA_Char* b, int size,
		      omni::alignment_t align=omni::ALIGN_1);
  void get_char_array(_CORBA_Char* b,int size,
		      omni::alignment_t align=omni::ALIGN_1);
  void skipInput(_CORBA_ULong size);
  _CORBA_Boolean checkInputOverrun(_CORBA_ULong itemSize, 
				   _CORBA_ULong nItems,
				   omni::alignment_t align=omni::ALIGN_1);
  _CORBA_Boolean checkOutputOverrun(_CORBA_ULong itemSize,
				    _CORBA_ULong nItems,
				    omni::alignment_t align=omni::ALIGN_1);
  void fetchInputData(omni::alignment_t,size_t);
  size_t maxFetchInputData(omni::alignment_t) const;
  _CORBA_Boolean reserveOutputSpace(omni::alignment_t,size_t);
  size_t maxReserveOutputSpace(omni::alignment_t) const;
  _CORBA_ULong currentInputPtr() const;
  _CORBA_ULong currentOutputPtr() const;
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
};

class cdrCountingStream : public cdrStream {
public:
  cdrCountingStream(size_t initialoffset = 0) : pd_total(initialoffset) {}
  ~cdrCountingStream() {}

  size_t total() { return pd_total; }

  // The following implements the abstract functions defined in cdrStream
  void put_char_array(const _CORBA_Char* b, int size,
		      omni::alignment_t align=omni::ALIGN_1);

  _CORBA_Boolean reserveOutputSpace(omni::alignment_t align,size_t required);

  size_t maxReserveOutputSpace(omni::alignment_t) const;

  _CORBA_Boolean checkOutputOverrun(_CORBA_ULong itemSize,
				    _CORBA_ULong nItems,
				    omni::alignment_t align=omni::ALIGN_1);

  void get_char_array(_CORBA_Char*,int,omni::alignment_t align=omni::ALIGN_1);

  void skipInput(_CORBA_ULong);

  _CORBA_Boolean checkInputOverrun(_CORBA_ULong,_CORBA_ULong,
                                   omni::alignment_t align=omni::ALIGN_1);

  void fetchInputData(omni::alignment_t,size_t);

  size_t maxFetchInputData(omni::alignment_t) const;

  _CORBA_ULong currentInputPtr() const;
  _CORBA_ULong currentOutputPtr() const;
private:
  size_t pd_total;
  
  cdrCountingStream(const cdrCountingStream&);
  cdrCountingStream& operator=(const cdrCountingStream&);
};





#endif /* __CDRSTREAM_H__ */
