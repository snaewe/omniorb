// -*- Mode: C++; -*-
//                            Package   : omniORB2
// seqtemplates.h             Created on: 14/5/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***

/*
 $Log$
 Revision 1.3  1997/03/09 13:00:28  sll
 Minor cleanup.

 Revision 1.2  1997/01/24 19:26:38  sll
 Moved the implementations of the inline members of _CORBA_Sequence<T> back
 into its class definition. This is to work around a bug in Sunpro C++ 4.0
 which make it fails to compile the runtime when optimisation is on.

 Revision 1.1  1997/01/23 15:19:58  sll
 Initial revision

 */

#ifndef __SEQTEMPLATES_H__
#define __SEQTEMPLATES_H__

class NetBufferedStream;
class MemBufferedStream;

template <class T>
class _CORBA_Sequence {
public:
  inline _CORBA_Sequence() : pd_max(0), pd_len(0), pd_rel(1), pd_buf(0) { }
  inline _CORBA_Sequence(_CORBA_ULong max) :
             pd_max(max), pd_len(0), pd_rel(1)
  {
    if (!(pd_buf = allocbuf(max))) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    return;
  }

  inline _CORBA_Sequence(_CORBA_ULong max,
			 _CORBA_ULong length,
			 T           *value,
			 _CORBA_Boolean release = 0) 
      : pd_max(max), 
	pd_len(length), 
	pd_rel(release),
	pd_buf(value)
  {
    if (length > max) {
      _CORBA_bound_check_error();
      // never reach here
    }
    return;
  }

  inline _CORBA_Sequence(const _CORBA_Sequence<T>& s)
              : pd_max(s.pd_max), 
		pd_len(s.pd_len),
		pd_rel(1)
  {
    if (!(pd_buf = allocbuf(s.pd_len))) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    for (_CORBA_ULong i=0; i < s.pd_len; i++) {
      pd_buf[i] = s.pd_buf[i];
    }
  }

  inline ~_CORBA_Sequence() {
    if (pd_rel && pd_buf) freebuf(pd_buf);
    pd_buf = 0;
    return;
  }
  inline _CORBA_Sequence<T> &operator= (const _CORBA_Sequence<T> &s)
  {
    if (pd_max < s.pd_max)
      {
	T *newbuf = allocbuf(s.pd_max);
	if (!newbuf) {
	  _CORBA_new_operator_return_null();
	  // never reach here
	}
	pd_max = s.pd_max;
	if (pd_rel && pd_buf) {
	  freebuf(pd_buf);
	}
	else {
	  pd_rel = 1;
	}
	pd_buf = newbuf;
      }
    pd_len = s.pd_len;
    for (unsigned long i=0; i < pd_len; i++) {
      pd_buf[i] = s.pd_buf[i];
    }
    return *this;
  }

  inline _CORBA_ULong maximum() const { return pd_max; }
  inline _CORBA_ULong length() const { return pd_len; }
  inline void length(_CORBA_ULong length)
  {
    if (length > pd_max)
      {
	T *newbuf = allocbuf(length);
	if (!newbuf) {
	  _CORBA_new_operator_return_null();
	  // never reach here
	}
	for (unsigned long i=0; i < pd_len; i++) {
	  newbuf[i] = pd_buf[i];
	}
	pd_max = length;
	if (pd_rel && pd_buf) {
	  freebuf(pd_buf);
	}
	else {
	  pd_rel = 1;
	}
	pd_buf = newbuf;
      }
    pd_len = length;
    return;
  }
  inline T &operator[] (_CORBA_ULong index)
  {
    if (index >= length()) {
      _CORBA_bound_check_error();
    }
    return pd_buf[index];
  }
  inline const T &operator[] (_CORBA_ULong index) const
  {
    if (index >= length()) {
      _CORBA_bound_check_error();
    }
    return pd_buf[index];
  }
  static inline T* allocbuf(_CORBA_ULong nelems)
  {
    return new T[nelems];
  }
  static inline void freebuf(T * b)
  {
    if (b) delete [] b; 
    return;
  }
  // omniORB2 extensions
  inline T *NP_data() const { return pd_buf; }
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);
private:
  _CORBA_ULong    pd_max;
  _CORBA_ULong    pd_len;
  _CORBA_Boolean  pd_rel;
  T              *pd_buf;
};

template <class T>
class _CORBA_Unbounded_Sequence : public _CORBA_Sequence<T> {
public:
  inline _CORBA_Unbounded_Sequence() {}
  inline _CORBA_Unbounded_Sequence(_CORBA_ULong max) : _CORBA_Sequence<T>(max) {}
  inline _CORBA_Unbounded_Sequence(_CORBA_ULong max,
				   _CORBA_ULong length,
				   T           *value,
				   _CORBA_Boolean release = 0)
     : _CORBA_Sequence<T>(max,length,value,release) {}
  inline _CORBA_Unbounded_Sequence(const _CORBA_Unbounded_Sequence<T>& s) 
     : _CORBA_Sequence<T>(s) {}
  inline ~_CORBA_Unbounded_Sequence() {}
  inline _CORBA_Unbounded_Sequence<T> &operator= (const _CORBA_Unbounded_Sequence<T> &s) {
    _CORBA_Sequence<T>::operator= (s);
    return *this;
  }
  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);
};

template <class T,int elmSize,int elmAlignment>
class _CORBA_Unbounded_Sequence_w_FixSizeElement 
   : public _CORBA_Sequence<T> 
{
public:
  inline _CORBA_Unbounded_Sequence_w_FixSizeElement() {}
  inline _CORBA_Unbounded_Sequence_w_FixSizeElement(_CORBA_ULong max)
    : _CORBA_Sequence<T>(max) {}
  inline _CORBA_Unbounded_Sequence_w_FixSizeElement(_CORBA_ULong max,
						    _CORBA_ULong length,
						    T           *value,
						    _CORBA_Boolean release = 0)
    : _CORBA_Sequence<T>(max,length,value,release) {}
  inline _CORBA_Unbounded_Sequence_w_FixSizeElement (const 
       _CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>& s)
    : _CORBA_Sequence<T>(s) {}
  inline ~_CORBA_Unbounded_Sequence_w_FixSizeElement() {}
  inline _CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment> &
      operator= 
        (const 
	  _CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment> &
	 s) 
  {
    _CORBA_Sequence<T>::operator= (s);
    return *this;
  }
  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);
};


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
  _CORBA_Sequence<T>::operator>>=(s);
}

template <class T,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::operator<<= (NetBufferedStream &s)
{
  _CORBA_Sequence<T>::operator<<=(s);
}

template <class T,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::operator>>= (MemBufferedStream &s) const
{
  _CORBA_Sequence<T>::operator>>=(s);
}

template <class T,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::operator<<= (MemBufferedStream &s)
{
  _CORBA_Sequence<T>::operator<<=(s);
}

typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Octet,1,1> _CORBA_Unbounded_Sequence_Octet;

#endif // __SEQTEMPLATES_H__

