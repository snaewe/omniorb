// -*- Mode: C++; -*-
//                            Package   : omniORB2
// bufferedstream.h           Created on: 3/3/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.6  1997/04/23 14:11:38  sll
  Marshalling functions for the sequence templates are defined here.
  get_char_array and put_char_array are used to marshal sequence of
  primitive data types.

 * Revision 1.5  1997/03/09  12:59:52  sll
 * Minor cleanup.
 *
  Revision 1.4  1997/01/23 18:03:49  sll
  Fixed bug in marshalling of double. The alignment should be 8.

  Revision 1.3  1997/01/23 14:59:27  sll
  Now included template marshalling code for sequence template.

 * Revision 1.2  1997/01/08  19:10:53  ewc
 * Added typedef to work around VC++ 4.2 nested class bug
 *
 * Revision 1.1  1997/01/08  17:28:30  sll
 * Initial revision
 *
  */

#ifndef __BUFFEREDSTREAM_H__
#define __BUFFEREDSTREAM_H__

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

#ifndef MARSHAL
#define MARSHAL(s,type,align,arg) {\
    type *p = (type *) (s).align_and_put_bytes(align,sizeof(type)); \
    *p = arg; \
}
#else
#error "MARSHAL has already been defined"
#endif

#ifndef UMARSHAL
#define UMARSHAL(s,type,align,arg) {\
    type *p = (type *) (s).align_and_get_bytes(align,sizeof(type)); \
    arg = *p; \
}
#else
#error "UMARSHAL has already been defined"
#endif

class NetBufferedStream : public Strand_Sync {
public:
  NetBufferedStream(Strand *s,
		    _CORBA_Boolean RdLock=1,
		    _CORBA_Boolean WrLock=1,
		    size_t Bufsize=0);
  NetBufferedStream(Rope *r,
		    _CORBA_Boolean RdLock=1,
		    _CORBA_Boolean WrLock=1,
		    size_t Bufsize=0);
  ~NetBufferedStream();


  friend inline void operator>>= (const _CORBA_Char a,NetBufferedStream &s) {
    MARSHAL(s,_CORBA_Char,omni::ALIGN_1,a);
  }

  friend inline void operator<<= (_CORBA_Char &a,NetBufferedStream &s) {
    UMARSHAL(s,_CORBA_Char,omni::ALIGN_1,a);
  }

  friend inline void operator>>= (const _CORBA_Short a,NetBufferedStream &s) {
    MARSHAL(s,_CORBA_Short,omni::ALIGN_2,a);
  }

  friend inline void operator<<= (_CORBA_Short &a,NetBufferedStream &s) {
    if (s.RdMessageByteOrder() != omni::myByteOrder) {
      _CORBA_Short t;
      UMARSHAL(s,_CORBA_Short,omni::ALIGN_2,t);
      a = Swap16(t);
    }
    else {
      UMARSHAL(s,_CORBA_Short,omni::ALIGN_2,a);
    }
    return;
  }

  friend inline void operator>>= (const _CORBA_UShort a,NetBufferedStream &s) {
    MARSHAL(s,_CORBA_UShort,omni::ALIGN_2,a);
  }

  friend inline void operator<<= (_CORBA_UShort &a,NetBufferedStream &s) {
    if (s.RdMessageByteOrder() != omni::myByteOrder) {
      _CORBA_UShort t;
      UMARSHAL(s,_CORBA_UShort,omni::ALIGN_2,t);
      a = Swap16(t);
    }
    else {
      UMARSHAL(s,_CORBA_UShort,omni::ALIGN_2,a);
    }
    return;
  }


  friend inline void operator>>= (const _CORBA_Long a,NetBufferedStream &s) {
    MARSHAL(s,_CORBA_Long,omni::ALIGN_4,a);
  }

  friend inline void operator<<= (_CORBA_Long &a,NetBufferedStream &s) {
    if (s.RdMessageByteOrder() != omni::myByteOrder) {
      _CORBA_Long t;
      UMARSHAL(s,_CORBA_Long,omni::ALIGN_4,t);
      a = Swap32(t);
    }
    else {
      UMARSHAL(s,_CORBA_Long,omni::ALIGN_4,a);
    }
    return;
  }

  friend inline void operator>>= (const _CORBA_ULong a,NetBufferedStream &s) {
    MARSHAL(s,_CORBA_ULong,omni::ALIGN_4,a);
  }

  friend inline void operator<<= (_CORBA_ULong &a,NetBufferedStream &s) {
    if (s.RdMessageByteOrder() != omni::myByteOrder) {
      _CORBA_ULong t;
      UMARSHAL(s,_CORBA_ULong,omni::ALIGN_4,t);
      a = Swap32(t);
    }
    else {
      UMARSHAL(s,_CORBA_ULong,omni::ALIGN_4,a);
    }
    return;
  }

#if !defined(NO_FLOAT)

  friend inline void operator>>= (const _CORBA_Float a,NetBufferedStream &s) {
    MARSHAL(s,_CORBA_Float,omni::ALIGN_4,a);
  }

  friend inline void operator<<= (_CORBA_Float &a,NetBufferedStream &s) {
    if (s.RdMessageByteOrder() != omni::myByteOrder) {
      _CORBA_Float t;
      UMARSHAL(s,_CORBA_Float,omni::ALIGN_4,t);
      _CORBA_ULong tl1 = *((_CORBA_ULong *)&t);
      _CORBA_ULong tl2 = Swap32(tl1);
      a = *((_CORBA_Float *) &tl2);
    }
    else {
      UMARSHAL(s,_CORBA_Float,omni::ALIGN_4,a);
    }
    return;
  }

  friend inline void operator>>= (const _CORBA_Double a,NetBufferedStream &s) {
    MARSHAL(s,_CORBA_Double,omni::ALIGN_8,a);
  }

  friend inline void operator<<= (_CORBA_Double &a,NetBufferedStream &s) {
    if (s.RdMessageByteOrder() != omni::myByteOrder) {
      _CORBA_Double t;
      UMARSHAL(s,_CORBA_Double,omni::ALIGN_8,t);
      _CORBA_ULong tl1 = ((_CORBA_ULong *)&t)[1];
      _CORBA_ULong tl2 = Swap32(tl1);
      ((_CORBA_ULong *)&a)[0] = tl2;
      tl1 = ((_CORBA_ULong *)&t)[0];
      tl2 = Swap32(tl1);
      ((_CORBA_ULong *)&a)[1] = tl2;
    }
    else {
      UMARSHAL(s,_CORBA_Double,omni::ALIGN_8,a);
    }
    return;
  }

#endif

  void put_char_array(const _CORBA_Char *b,int size);
  void get_char_array(_CORBA_Char *b,int size);

  void skip(_CORBA_ULong size);

  void flush();

  // Byte streams passing through the object can be separated into
  // messages. A message is simply a vector of bytes.  Both the incoming
  // and outgoing stream has an associated message size. The message size
  // is limited to the value of MaxMessageSize().
  //
  // The outgoing message size can be set by the protected member
  // function void WrMessageSize(size_t msgsize). This function can be
  // called at any time to reset the value of the outgoing message size.
  //
  // If the outgoing message size is set to 0, the message is of infinite
  // size. If the outgoing message size is set to N > 0, up to N bytes
  // can be written to the stream. Writing more than N bytes would cause
  // an exception (CORBA::MARSHAL).
  //
  // The public member functions WrMessageAlreadyWritten() and 
  // WrMessageSpaceLeft() return the number of bytes that have already
  // been written and the space left in a message respectively. The functions
  // would return 0 if the outgoing message size is 0.
  //
  // Similarly, the incoming message size can be set by the protected
  // member function void RdMessageSize(size_t msgsize). The
  // public member functions RdMessageAlreadyRead() and RdMessageUnRead()
  // return the number of bytes that have already been read and the
  // remaining bytes to be read respectively.

  size_t MaxMessageSize() const { return pd_strand->MaxMTU(); }

  size_t WrMessageSize() const { return pd_wrmsg_size; }
  // Return the current outgoing message size

  size_t RdMessageSize() const { return pd_rdmsg_size; }
  // Return the current incoming message size

  size_t RdMessageAlreadyRead() const;
  // Return the number of bytes that have already been read from the incoming
  // message.

  size_t RdMessageUnRead() const;
  // Return the number of bytes in the incoming message that have
  // not been read.

  size_t WrMessageAlreadyWritten() const;
  // Return the number of bytes that have already been written into the
  // outgoing message.

  size_t WrMessageSpaceLeft() const;
  // Return the number of bytes in the outgoing message that have not
  // been written.

  _CORBA_Boolean RdMessageByteOrder() const {
    return pd_rdmsg_byte_order;
  }

protected:

  void RdLock();
  void RdUnlock();
  void WrLock();
  void WrUnlock();

  inline _CORBA_Long newRequestID() { return pd_strand->sequenceNumber(); }

  void   WrMessageSize(size_t msgsize);
  // For an outgoing message, set the message size.

  void   RdMessageSize(size_t msgsize,_CORBA_Boolean byteorder);
  // For an incoming message, set what the message size.

private:
  Strand         *pd_strand;
  size_t          pd_ideal_buf_size;
  void           *pd_inb_end;
  void           *pd_outb_end;
  void           *pd_inb_mkr;
  void           *pd_outb_mkr;
  _CORBA_Boolean  pd_RdLock;
  _CORBA_Boolean  pd_WrLock;

  size_t          pd_rdmsg_size;
  size_t          pd_wrmsg_size;
  size_t          pd_read;
  size_t          pd_written;
  _CORBA_Boolean  pd_rdmsg_byte_order;

  void ensure_rdlocked();
  void ensure_wrlocked();
  void reserve(size_t minimum);
  void giveback_reserved(_CORBA_Boolean transmit = 0);
  void receive(size_t minimum);
  void giveback_received();
  int current_outb_alignment();
  int current_inb_alignment();
  void rewind_outb_mkr(int oldalignment);
  void rewind_inb_mkr(int oldalignment);

  inline void *align_and_put_bytes(omni::alignment_t align,size_t nbytes) {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_outb_mkr,align);
    omni::ptr_arith_t p2 = p1 + nbytes;
    if ((void *)p2 > pd_outb_end) {
      reserve(p2 - (omni::ptr_arith_t)pd_outb_mkr);
      return align_and_put_bytes(align,nbytes);
    }
    pd_outb_mkr = (void *) p2;
    return (void *) p1;
  }

  inline void *align_and_get_bytes(omni::alignment_t align,size_t nbytes) {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_inb_mkr,align);
    omni::ptr_arith_t p2 = p1 + nbytes;
    if ((void *)p2 > pd_inb_end) {
      receive(p2 - (omni::ptr_arith_t)pd_inb_mkr);
      return align_and_get_bytes(align,nbytes);
    }
    pd_inb_mkr = (void *) p2;
    return (void *)p1;
  }

  NetBufferedStream();
  NetBufferedStream(const NetBufferedStream&);
  NetBufferedStream &operator=(const NetBufferedStream&);
};



class MemBufferedStream {
public:
  MemBufferedStream(size_t initialBufsize=0);
  ~MemBufferedStream();
  
  MemBufferedStream(const MemBufferedStream&);
  MemBufferedStream &operator=(const MemBufferedStream&);

  friend inline void operator>>= (const _CORBA_Char a,MemBufferedStream &s) {
    MARSHAL(s,_CORBA_Char,omni::ALIGN_1,a);
  }

  friend inline void operator<<= (_CORBA_Char &a,MemBufferedStream &s) {
    UMARSHAL(s,_CORBA_Char,omni::ALIGN_1,a);
  }

  friend inline void operator>>= (const _CORBA_Short a,MemBufferedStream &s) {
    MARSHAL(s,_CORBA_Short,omni::ALIGN_2,a);
  }

  friend inline void operator<<= (_CORBA_Short &a,MemBufferedStream &s) {
    if (s.byteOrder() != omni::myByteOrder) {
      _CORBA_Short t;
      UMARSHAL(s,_CORBA_Short,omni::ALIGN_2,t);
      a = Swap16(t);
    }
    else {
      UMARSHAL(s,_CORBA_Short,omni::ALIGN_2,a);
    }
    return;
  }

  friend inline void operator>>= (const _CORBA_UShort a,MemBufferedStream &s) {
    MARSHAL(s,_CORBA_UShort,omni::ALIGN_2,a);
  }

  friend inline void operator<<= (_CORBA_UShort &a,MemBufferedStream &s) {
    if (s.byteOrder() != omni::myByteOrder) {
      _CORBA_UShort t;
      UMARSHAL(s,_CORBA_UShort,omni::ALIGN_2,t);
      a = Swap16(t);
    }
    else {
      UMARSHAL(s,_CORBA_UShort,omni::ALIGN_2,a);
    }
    return;
  }


  friend inline void operator>>= (const _CORBA_Long a,MemBufferedStream &s) {
    MARSHAL(s,_CORBA_Long,omni::ALIGN_4,a);
  }

  friend inline void operator<<= (_CORBA_Long &a,MemBufferedStream &s) {
    if (s.byteOrder() != omni::myByteOrder) {
      _CORBA_Long t;
      UMARSHAL(s,_CORBA_Long,omni::ALIGN_4,t);
      a = Swap32(t);
    }
    else {
      UMARSHAL(s,_CORBA_Long,omni::ALIGN_4,a);
    }
    return;
  }

  friend inline void operator>>= (const _CORBA_ULong a,MemBufferedStream &s) {
    MARSHAL(s,_CORBA_ULong,omni::ALIGN_4,a);
  }

  friend inline void operator<<= (_CORBA_ULong &a,MemBufferedStream &s) {
    if (s.byteOrder() != omni::myByteOrder) {
      _CORBA_ULong t;
      UMARSHAL(s,_CORBA_ULong,omni::ALIGN_4,t);
      a = Swap32(t);
    }
    else {
      UMARSHAL(s,_CORBA_ULong,omni::ALIGN_4,a);
    }
    return;
  }

#if !defined(NO_FLOAT)

  friend inline void operator>>= (const _CORBA_Float a,MemBufferedStream &s) {
    MARSHAL(s,_CORBA_Float,omni::ALIGN_4,a);
  }

  friend inline void operator<<= (_CORBA_Float &a,MemBufferedStream &s) {
    if (s.byteOrder() != omni::myByteOrder) {
      _CORBA_Float t;
      UMARSHAL(s,_CORBA_Float,omni::ALIGN_4,t);
      _CORBA_ULong tl1 = *((_CORBA_ULong *)&t);
      _CORBA_ULong tl2 = Swap32(tl1);
      a = *((_CORBA_Float *) &tl2);
    }
    else {
      UMARSHAL(s,_CORBA_Float,omni::ALIGN_4,a);
    }
    return;
  }

  friend inline void operator>>= (const _CORBA_Double a,MemBufferedStream &s) {
    MARSHAL(s,_CORBA_Double,omni::ALIGN_8,a);
  }

  friend inline void operator<<= (_CORBA_Double &a,MemBufferedStream &s) {
    if (s.byteOrder() != omni::myByteOrder) {
      _CORBA_Double t;
      UMARSHAL(s,_CORBA_Double,omni::ALIGN_8,t);
      _CORBA_ULong tl1 = ((_CORBA_ULong *)&t)[1];
      _CORBA_ULong tl2 = Swap32(tl1);
      ((_CORBA_ULong *)&a)[0] = tl2;
      tl1 = ((_CORBA_ULong *)&t)[0];
      tl2 = Swap32(tl1);
      ((_CORBA_ULong *)&a)[1] = tl2;
    }
    else {
      UMARSHAL(s,_CORBA_Double,omni::ALIGN_8,a);
    }
    return;
  }

#endif

  void put_char_array(const _CORBA_Char *b,int size);
  void get_char_array(_CORBA_Char *b,int size);


  void rewind_inout_mkr();
  void rewind_in_mkr();

  _CORBA_Boolean byteOrder() const {
    return pd_byte_order;
  }

  void byteOrder(_CORBA_Boolean b) {
    pd_byte_order = b;
  }

  size_t alreadyRead() const {
    if (pd_in_mkr < pd_out_mkr)
      return ((omni::ptr_arith_t)pd_in_mkr - 
	      (omni::ptr_arith_t)startofstream());
    else
      return ((omni::ptr_arith_t)pd_out_mkr - 
	      (omni::ptr_arith_t)startofstream());
  }

  size_t unRead() const {
    if (pd_in_mkr < pd_out_mkr)
      return ((omni::ptr_arith_t)pd_out_mkr - 
	      (omni::ptr_arith_t)pd_in_mkr);
    else
      return 0;
  }

  size_t alreadyWritten() const {
    return ((omni::ptr_arith_t)pd_out_mkr - 
	    (omni::ptr_arith_t)startofstream());
  }

  void skip(_CORBA_ULong size);

  void *data() const {
    return pd_in_mkr;
  }

private:
  void     *pd_bufp;
  void     *pd_bufend;
  void     *pd_in_mkr;
  void     *pd_out_mkr;
#define MEMBUFFEREDSTREAM_INLINE_BUF_SIZE 32
  static const int pd_inline_buf_size;
  char      pd_buffer[MEMBUFFEREDSTREAM_INLINE_BUF_SIZE];
  _CORBA_Boolean pd_byte_order;

  inline void *align_and_put_bytes(omni::alignment_t align,size_t nbytes) {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_out_mkr,align);
    omni::ptr_arith_t p2 = p1 + nbytes;
    if ((void *)p2 > pd_bufend) {
      grow(p2 - (omni::ptr_arith_t)pd_out_mkr);
      return align_and_put_bytes(align,nbytes);
    }
    pd_out_mkr = (void *) p2;
    return (void *) p1;
  }

  inline void *align_and_get_bytes(omni::alignment_t align,size_t nbytes) {
    omni::ptr_arith_t p1 = omni::align_to((omni::ptr_arith_t)pd_in_mkr,align);
    pd_in_mkr = (void *)(p1 + nbytes);
    if (pd_in_mkr > pd_out_mkr) {
      return overrun_error();
    }
    return (void *)p1;
  }

  void * startofstream() const;
  size_t size();
  void grow(size_t minimum);
  void copy(const MemBufferedStream &);
  void *overrun_error();
};

#undef MARSHAL
#undef UMARSHAL

template <class T>
inline void
_CORBA_Sequence<T>::operator>>= (NetBufferedStream &s) const
{
  pd_len >>= s;
  for (int i=0; i<(int)pd_len; i++) {
    pd_buf[i] >>= s;
  }
  return;
}

template <class T>
inline void
_CORBA_Sequence<T>::operator<<= (NetBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l > s.RdMessageUnRead()) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    pd_buf[i] <<= s;
  }
  return;
}

template <class T>
inline void
_CORBA_Sequence<T>::operator>>= (MemBufferedStream &s) const
{
  pd_len >>= s;
  for (int i=0; i<(int)pd_len; i++) {
    pd_buf[i] >>= s;
  }
  return;
}

template <class T>
inline void
_CORBA_Sequence<T>::operator<<= (MemBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l > s.unRead()) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    pd_buf[i] <<= s;
  }
  return;
}


template <class T>
inline
size_t
_CORBA_Unbounded_Sequence<T>::NP_alignedSize(size_t initialoffset) const 
{
  size_t alignedsize = ((initialoffset+3) & ~((int)3))+sizeof(_CORBA_ULong);
  for (unsigned long i=0; i < length(); i++) {
    alignedsize = NP_data()[i].NP_alignedSize(alignedsize);
  }
  return alignedsize;
}

template <class T>
inline 
void 
_CORBA_Unbounded_Sequence<T>::operator>>= (NetBufferedStream &s) const
{
  _CORBA_Sequence<T>::operator>>=(s);
}

template <class T>
inline
void
_CORBA_Unbounded_Sequence<T>::operator<<= (NetBufferedStream &s)
{
  _CORBA_Sequence<T>::operator<<=(s);
}

template <class T>
inline
void 
_CORBA_Unbounded_Sequence<T>::operator>>= (MemBufferedStream &s) const
{
  _CORBA_Sequence<T>::operator>>=(s);
}

template <class T>
inline
void 
_CORBA_Unbounded_Sequence<T>::operator<<= (MemBufferedStream &s)
{
  _CORBA_Sequence<T>::operator<<=(s);
}

template <class T,int max>
inline 
size_t
_CORBA_Bounded_Sequence<T,max>::NP_alignedSize(size_t initialoffset) const 
{
  size_t alignedsize = ((initialoffset+3) & ~((int)3))+sizeof(_CORBA_ULong);
  for (unsigned long i=0; i < length(); i++) {
    alignedsize = NP_data()[i].NP_alignedSize(alignedsize);
  }
  return alignedsize;
}

template <class T,int max>
inline 
void
_CORBA_Bounded_Sequence<T,max>::operator>>= (NetBufferedStream &s) const
{
  _CORBA_Sequence<T>::operator>>=(s);
}

template <class T,int max>
inline void
_CORBA_Bounded_Sequence<T,max>::operator<<= (NetBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l > s.RdMessageUnRead() || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  
  length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    NP_data()[i] <<= s;
  }
  return;
}

template <class T,int max>
inline void
_CORBA_Bounded_Sequence<T,max>::operator>>= (MemBufferedStream &s) const 
{
  _CORBA_Sequence<T>::operator>>=(s);
}

template <class T,int max>
inline void
_CORBA_Bounded_Sequence<T,max>::operator<<= (MemBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l > s.unRead() || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    NP_data()[i] <<= s;
  }
  return;
}

template <class T,int elmSize,int elmAlignment>
inline
size_t
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::NP_alignedSize(size_t initialoffset) const 
{
  size_t alignedsize = ((initialoffset+3) & ~((int)3))+sizeof(_CORBA_ULong);
  alignedsize = ((alignedsize+(elmAlignment-1)) & ~(elmAlignment-1));
  alignedsize += length() * elmSize;
  return alignedsize;
}

template <class T,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::operator>>= (NetBufferedStream &s) const
{
  _CORBA_ULong l = length();
  l >>= s;
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    CORBA::ULong padding = 0;
    if (s.WrMessageAlreadyWritten() & ((int)omni::ALIGN_8 - 1))
      padding >>= s;
  }
  s.put_char_array((_CORBA_Char*)NP_data(),(int)l*elmSize);
}

template <class T,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::operator<<= (NetBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l*elmSize > s.RdMessageUnRead()) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(l);
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    if (s.RdMessageAlreadyRead() & ((int)omni::ALIGN_8 - 1))
      s.skip(sizeof(_CORBA_ULong));
  }
  s.get_char_array((_CORBA_Char*)NP_data(),(int)l*elmSize);
  if (s.RdMessageByteOrder() != omni::myByteOrder && elmAlignment != 1) {
    if (elmSize == 2) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	NP_data()[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	NP_data()[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	_CORBA_ULong tl1 = ((_CORBA_ULong *)&t)[1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong *)&t)[0];
	((_CORBA_ULong *)&t)[0] = tl2;
	tl2 = Swap32(tl1);
	((_CORBA_ULong *)&t)[1] = tl2;
	NP_data()[i] = t;
      }
    }
  }
}

template <class T,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::operator>>= (MemBufferedStream &s) const
{
  _CORBA_ULong l = length();
  l >>= s;
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    CORBA::ULong padding = 0;
    if (s.alreadyWritten() & ((int)omni::ALIGN_8 - 1))
      padding >>= s;
  }
  s.put_char_array((_CORBA_Char*)NP_data(),(int)l*elmSize);
}

template <class T,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::operator<<= (MemBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l*elmSize > s.unRead()) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(l);
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    if (s.alreadyRead() & ((int)omni::ALIGN_8 - 1))
      s.skip(sizeof(_CORBA_ULong));
  }
  s.get_char_array((_CORBA_Char*)NP_data(),(int)l*elmSize);
  if (s.byteOrder() != omni::myByteOrder && elmAlignment != 1) {
    if (elmSize == 2) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	NP_data()[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	NP_data()[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	_CORBA_ULong tl1 = ((_CORBA_ULong *)&t)[1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong *)&t)[0];
	((_CORBA_ULong *)&t)[0] = tl2;
	tl2 = Swap32(tl1);
	((_CORBA_ULong *)&t)[1] = tl2;
	NP_data()[i] = t;
      }
    }
  }
}



template <class T,int max,int elmSize, int elmAlignment>
inline
size_t
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::NP_alignedSize(size_t initialoffset) const 
{
  size_t alignedsize = ((initialoffset+3) & ~((int)3))+sizeof(_CORBA_ULong);
  alignedsize = ((alignedsize+(elmAlignment-1)) & ~(elmAlignment-1));
  alignedsize += length() * elmSize;
  return alignedsize;
}


template <class T,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator>>= (NetBufferedStream &s) const
{
  _CORBA_ULong l = length();
  l >>= s;
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    CORBA::ULong padding = 0;
    if (s.WrMessageAlreadyWritten() & ((int)omni::ALIGN_8 - 1))
      padding >>= s;
  }
  s.put_char_array((_CORBA_Char*)NP_data(),(int)l*elmSize);
}

template <class T,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator<<= (NetBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l*elmSize > s.RdMessageUnRead() || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  
  length(l);
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    if (s.RdMessageAlreadyRead() & ((int)omni::ALIGN_8 - 1))
      s.skip(sizeof(_CORBA_ULong));
  }
  s.get_char_array((_CORBA_Char*)pd_buf,(int)l*elmSize);
  if (s.RdMessageByteOrder() != omni::myByteOrder && elmAlignment != 1) {
    if (elmSize == 2) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	NP_data()[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	NP_data()[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	_CORBA_ULong tl1 = ((_CORBA_ULong *)&t)[1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong *)&t)[0];
	((_CORBA_ULong *)&t)[0] = tl2;
	tl2 = Swap32(tl1);
	((_CORBA_ULong *)&t)[1] = tl2;
	NP_data()[i] = t;
      }
    }
  }
}

template <class T,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator>>= (MemBufferedStream &s) const
{
  _CORBA_ULong l = length();
  l >>= s;
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    CORBA::ULong padding = 0;
    if (s.alreadyWritten() & ((int)omni::ALIGN_8 - 1))
      padding >>= s;
  }
  s.put_char_array((_CORBA_Char*)NP_data(),(int)l*elmSize);
}

template <class T,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator<<= (MemBufferedStream &s)
{
  _CORBA_ULong l;
  l <<= s;
  if (l*elmSize > s.unRead() || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  
  length(l);
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    if (s.alreadyRead() & ((int)omni::ALIGN_8 - 1))
      s.skip(sizeof(_CORBA_ULong));
  }
  s.get_char_array((_CORBA_Char*)pd_buf,(int)l*elmSize);
  if (s.byteOrder() != omni::myByteOrder && elmAlignment != 1) {
    if (elmSize == 2) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	NP_data()[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	NP_data()[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      for (_CORBA_ULong i=0; i<l; i++) {
	T t = NP_data()[i];
	_CORBA_ULong tl1 = ((_CORBA_ULong *)&t)[1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong *)&t)[0];
	((_CORBA_ULong *)&t)[0] = tl2;
	tl2 = Swap32(tl1);
	((_CORBA_ULong *)&t)[1] = tl2;
	NP_data()[i] = t;
      }
    }
  }
}

#undef Swap16
#undef Swap32

#endif // __BUFFEREDSTREAM_H__
