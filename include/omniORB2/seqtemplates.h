// -*- Mode: C++; -*-
//                            Package   : omniORB2
// seqtemplates.h             Created on: 14/5/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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

/*
 $Log$
 Revision 1.10  1998/01/27 19:35:56  ewc
 Revised Octet and Boolean templates

 * Revision 1.9  1998/01/27  16:08:45  ewc
 * Added new classes and templates for sequence of Boolean and sequence
 * of Octet.
 *
 Revision 1.8  1997/12/18 17:35:52  sll
 _CORBA_Sequence copy ctor should allocate s.pd_max elements instead of
 s.pd_len elements.

 Revision 1.7  1997/12/09 20:42:25  sll
 Updated sequence array templates.

 Revision 1.6  1997/08/21 22:21:56  sll
 New templates to support sequence of array.

 * Revision 1.5  1997/05/06  16:11:10  sll
 * Public release.
 *
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
    if (!(pd_buf = allocbuf(s.pd_max))) {
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
  inline void NP_norelease() { pd_rel = 0; }
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

class _CORBA_Unbounded_Sequence__Boolean
   : public _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Boolean,1,1>
{
public:
  inline _CORBA_Unbounded_Sequence__Boolean() {}
  inline _CORBA_Unbounded_Sequence__Boolean(_CORBA_ULong max)
    : _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Boolean,1,1>(max) {}
  inline _CORBA_Unbounded_Sequence__Boolean(_CORBA_ULong max,
						    _CORBA_ULong length,
						    _CORBA_Boolean   *value,
						    _CORBA_Boolean release = 0)
    : _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Boolean,1,1>(max,length,value,release) {}
  inline _CORBA_Unbounded_Sequence__Boolean (const 
       _CORBA_Unbounded_Sequence__Boolean& s)
    :  _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Boolean,1,1>(s) {}

  inline ~_CORBA_Unbounded_Sequence__Boolean() {}
};

class _CORBA_Unbounded_Sequence__Octet
   : public _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Octet,1,1>
{
public:
  inline _CORBA_Unbounded_Sequence__Octet() {}
  inline _CORBA_Unbounded_Sequence__Octet(_CORBA_ULong max)
    :  _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Octet,1,1 >(max) {}
  inline _CORBA_Unbounded_Sequence__Octet(_CORBA_ULong max,
						    _CORBA_ULong length,
						    _CORBA_Octet   *value,
						    _CORBA_Boolean release = 0)
    : _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Octet,1,1 >(max,length,value,release) {}
  inline _CORBA_Unbounded_Sequence__Octet (const 
       _CORBA_Unbounded_Sequence__Octet& s)
    : _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Octet,1,1 >(s) {}

  inline ~_CORBA_Unbounded_Sequence__Octet() {}
};


template <class T,int max>
class _CORBA_Bounded_Sequence : public _CORBA_Sequence<T> {
public:
  inline _CORBA_Bounded_Sequence() : _CORBA_Sequence<T>(max) {}
  inline _CORBA_Bounded_Sequence(_CORBA_ULong length,
				 T           *value,
				 _CORBA_Boolean release = 0)
            : _CORBA_Sequence<T>(max,length,value,release) {}
  inline _CORBA_Bounded_Sequence(const _CORBA_Bounded_Sequence<T,max>& s)
            : _CORBA_Sequence<T>(s) {}
  inline ~_CORBA_Bounded_Sequence() {}
  inline _CORBA_Bounded_Sequence<T,max> &operator= (const _CORBA_Bounded_Sequence<T,max> &s);
  inline _CORBA_ULong length() const { return _CORBA_Sequence<T>::length(); }
  inline void length(_CORBA_ULong len);
  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);
};


template <class T,int max,int elmSize, int elmAlignment>
class _CORBA_Bounded_Sequence_w_FixSizeElement 
  : public _CORBA_Sequence<T> 
{
public:
  inline _CORBA_Bounded_Sequence_w_FixSizeElement() {}
  inline _CORBA_Bounded_Sequence_w_FixSizeElement(_CORBA_ULong length,
					   T           *value,
					   _CORBA_Boolean release = 0)
    : _CORBA_Sequence<T>(max,length,value,release) {}
  inline _CORBA_Bounded_Sequence_w_FixSizeElement(const 
      _CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>& s) 
    : _CORBA_Sequence<T>(s) {}
  inline ~_CORBA_Bounded_Sequence_w_FixSizeElement() {}
  inline _CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment> &
      operator= 
        (const 
          _CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>&
            s);
  inline _CORBA_ULong length() const;
  inline void length(_CORBA_ULong len);
  // omniORB2 extensions
  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);
};

template <int max>
class _CORBA_Bounded_Sequence__Boolean
   : public _CORBA_Bounded_Sequence_w_FixSizeElement<_CORBA_Boolean,max,1,1>
{
public:
  inline _CORBA_Bounded_Sequence__Boolean() {}
  inline _CORBA_Bounded_Sequence__Boolean(_CORBA_ULong length,
					   _CORBA_Boolean    *value,
					   _CORBA_Boolean release = 0)
    : _CORBA_Bounded_Sequence_w_FixSizeElement<_CORBA_Boolean,max,1,1>(length,value,release) {}
  inline _CORBA_Bounded_Sequence__Boolean(const 
			       _CORBA_Bounded_Sequence__Boolean<max>& s) 
    :  _CORBA_Bounded_Sequence_w_FixSizeElement<_CORBA_Boolean,max,1,1>(s) {}

  inline ~_CORBA_Bounded_Sequence__Boolean() {}
};

template <int max>
class _CORBA_Bounded_Sequence__Octet
   : public _CORBA_Bounded_Sequence_w_FixSizeElement<_CORBA_Octet,max,1,1>
{
public:
  inline _CORBA_Bounded_Sequence__Octet() {}
  inline _CORBA_Bounded_Sequence__Octet(_CORBA_ULong length,
					   _CORBA_Octet    *value,
					   _CORBA_Boolean release = 0)
    : _CORBA_Bounded_Sequence_w_FixSizeElement<_CORBA_Octet,max,1,1>(length,value,release) {}
  inline _CORBA_Bounded_Sequence__Octet(const 
		       		 _CORBA_Bounded_Sequence__Octet<max>& s) 
    :  _CORBA_Bounded_Sequence_w_FixSizeElement<_CORBA_Octet,max,1,1>(s) {}

  inline ~_CORBA_Bounded_Sequence__Octet() {}
};
////////////////////////////////////////////////////////////////////////////

template <class T,class T_slice,class Telm,int dimension>
class _CORBA_Sequence_Array {
public:
  inline _CORBA_Sequence_Array() : pd_max(0), pd_len(0), 
                                   pd_rel(1), pd_buf(0) { }
  inline _CORBA_Sequence_Array(_CORBA_ULong max) :
             pd_max(max), pd_len(0), pd_rel(1)
  {
    if (!(pd_buf = allocbuf(max))) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    return;
  }

  inline _CORBA_Sequence_Array(_CORBA_ULong max,
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

  inline _CORBA_Sequence_Array(const _CORBA_Sequence_Array<T,T_slice,Telm,dimension>& s)
              : pd_max(s.pd_max), 
		pd_len(s.pd_len),
		pd_rel(1)
  {
    if (!(pd_buf = allocbuf(s.pd_len))) {
      _CORBA_new_operator_return_null();
      // never reach here
    }
    for (_CORBA_ULong i=0; i < s.pd_len; i++) {
      for (_CORBA_ULong j=0; j < dimension; j++) {
	*((Telm*)(pd_buf[i]) + j) = *((Telm*)(s.pd_buf[i]) + j);
      }
    }
  }

  inline ~_CORBA_Sequence_Array() {
    if (pd_rel && pd_buf) freebuf(pd_buf);
    pd_buf = 0;
    return;
  }
  inline _CORBA_Sequence_Array<T,T_slice,Telm,dimension> &operator= (const _CORBA_Sequence_Array<T,T_slice,Telm,dimension> &s)
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
      for (_CORBA_ULong j=0; j < dimension; j++) {
	*((Telm*)(pd_buf[i]) + j) = *((Telm*)(s.pd_buf[i]) + j);
      }
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
	  for (_CORBA_ULong j=0; j < dimension; j++) {
	    *((Telm*)(newbuf[i]) + j) = *((Telm*)(pd_buf[i]) + j);
	  }
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
  inline T_slice* operator[] (_CORBA_ULong index)
  {
    if (index >= length()) {
      _CORBA_bound_check_error();
    }
    return (T_slice*)(pd_buf[index]);
  }
  inline const T_slice* operator[] (_CORBA_ULong index) const
  {
    if (index >= length()) {
      _CORBA_bound_check_error();
    }
    return (const T_slice*)(pd_buf[index]);
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
  inline void NP_norelease() { pd_rel = 0; }
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

template <class T,class T_slice,class Telm,int dimension>
class _CORBA_Unbounded_Sequence_Array : public _CORBA_Sequence_Array<T,T_slice,Telm,dimension> {
public:
  inline _CORBA_Unbounded_Sequence_Array() {}
  inline _CORBA_Unbounded_Sequence_Array(_CORBA_ULong max) : _CORBA_Sequence_Array<T,T_slice,Telm,dimension>(max) {}
  inline _CORBA_Unbounded_Sequence_Array(_CORBA_ULong max,
					 _CORBA_ULong length,
					 T           *value,
					 _CORBA_Boolean release = 0)
     : _CORBA_Sequence_Array<T,T_slice,Telm,dimension>(max,length,value,release) {}
  inline _CORBA_Unbounded_Sequence_Array(const _CORBA_Unbounded_Sequence_Array<T,T_slice,Telm,dimension>& s) 
     : _CORBA_Sequence_Array<T,T_slice,Telm,dimension>(s) {}
  inline ~_CORBA_Unbounded_Sequence_Array() {}
  inline _CORBA_Unbounded_Sequence_Array<T,T_slice,Telm,dimension> &operator= (const _CORBA_Unbounded_Sequence_Array<T,T_slice,Telm,dimension> &s) {
    _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::operator= (s);
    return *this;
  }
  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);
};

template <class T,class T_slice, class Telm,int dimension,int elmSize,int elmAlignment>
class _CORBA_Unbounded_Sequence_Array_w_FixSizeElement 
   : public _CORBA_Sequence_Array<T,T_slice,Telm,dimension> 
{
public:
  inline _CORBA_Unbounded_Sequence_Array_w_FixSizeElement() {}
  inline _CORBA_Unbounded_Sequence_Array_w_FixSizeElement(_CORBA_ULong max)
    : _CORBA_Sequence_Array<T,T_slice,Telm,dimension>(max) {}
  inline _CORBA_Unbounded_Sequence_Array_w_FixSizeElement(_CORBA_ULong max,
							  _CORBA_ULong length,
							  T           *value,
							  _CORBA_Boolean release = 0)
    : _CORBA_Sequence_Array<T,T_slice,Telm,dimension>(max,length,value,release) {}
  inline _CORBA_Unbounded_Sequence_Array_w_FixSizeElement (const 
       _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,elmSize,elmAlignment>& s)
    : _CORBA_Sequence_Array<T,T_slice,Telm,dimension>(s) {}
  inline ~_CORBA_Unbounded_Sequence_Array_w_FixSizeElement() {}
  inline _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,elmSize,elmAlignment> &
      operator= 
        (const 
	  _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,elmSize,elmAlignment> &
	 s) 
  {
    _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::operator= (s);
    return *this;
  }
  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);
};

template<class T, class T_slice, int dimension>
class _CORBA_Unbounded_Sequence_Array__Boolean
   : public _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Boolean,dimension,1,1>
{
public:
  inline _CORBA_Unbounded_Sequence_Array__Boolean() {}
  inline _CORBA_Unbounded_Sequence_Array__Boolean(_CORBA_ULong max)
    : _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Boolean,dimension,1,1>(max) {}
  inline _CORBA_Unbounded_Sequence_Array__Boolean(_CORBA_ULong max,
							 _CORBA_ULong length,
							 _CORBA_Boolean *value,
							 _CORBA_Boolean release = 0)
    :  _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Boolean,dimension,1,1>(max,length,value,release) {}
  inline _CORBA_Unbounded_Sequence_Array__Boolean (const 
       _CORBA_Unbounded_Sequence_Array__Boolean<T,T_slice,dimension>& s)
    :   _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Boolean,dimension,1,1>(s) {}

  inline ~_CORBA_Unbounded_Sequence_Array__Boolean() {}
};


template<class T, class T_slice, int dimension>
class _CORBA_Unbounded_Sequence_Array__Octet
   : public _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Octet,dimension,1,1>
{
public:
  inline _CORBA_Unbounded_Sequence_Array__Octet() {}
  inline _CORBA_Unbounded_Sequence_Array__Octet(_CORBA_ULong max)
    :   _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Octet,dimension,1,1>(max) {}
  inline _CORBA_Unbounded_Sequence_Array__Octet(_CORBA_ULong max,
							 _CORBA_ULong length,
							 _CORBA_Octet *value,
							 _CORBA_Boolean release = 0)
    :   _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Octet,dimension,1,1>(max,length,value,release) {}
  inline _CORBA_Unbounded_Sequence_Array__Octet (const 
       _CORBA_Unbounded_Sequence_Array__Octet<T,T_slice,dimension>& s)
    :   _CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Octet,dimension,1,1>(s) {}

  inline ~_CORBA_Unbounded_Sequence_Array__Octet() {}
};

template <class T,class T_slice,class Telm,int dimension,int max>
class _CORBA_Bounded_Sequence_Array : public _CORBA_Sequence_Array<T,T_slice,Telm,dimension> {
public:
  inline _CORBA_Bounded_Sequence_Array() : _CORBA_Sequence_Array<T,T_slice,Telm,dimension>(max) {}
  inline _CORBA_Bounded_Sequence_Array(_CORBA_ULong length,
				       T           *value,
				       _CORBA_Boolean release = 0)
            : _CORBA_Sequence_Array<T,T_slice,Telm,dimension>(max,length,value,release) {}
  inline _CORBA_Bounded_Sequence_Array(const _CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max>& s)
            : _CORBA_Sequence_Array<T,T_slice,Telm,dimension>(s) {}
  inline ~_CORBA_Bounded_Sequence_Array() {}
  inline _CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max> &operator= (const _CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max> &s);
  inline _CORBA_ULong length() const { return _CORBA_Sequence_Array<T,T_slice,Telm,dimension>::length(); }
  inline void length(_CORBA_ULong len);
  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);
};

template <class T,class T_slice,class Telm,int dimension,int max,int elmSize, int elmAlignment>
class _CORBA_Bounded_Sequence_Array_w_FixSizeElement 
  : public _CORBA_Sequence_Array<T,T_slice,Telm,dimension> 
{
public:
  inline _CORBA_Bounded_Sequence_Array_w_FixSizeElement() {}
  inline _CORBA_Bounded_Sequence_Array_w_FixSizeElement(_CORBA_ULong length,
							T           *value,
							_CORBA_Boolean release = 0)
    : _CORBA_Sequence_Array<T,T_slice,Telm,dimension>(max,length,value,release) {}
  inline _CORBA_Bounded_Sequence_Array_w_FixSizeElement(const 
      _CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>& s) 
    : _CORBA_Sequence_Array<T,T_slice,Telm,dimension>(s) {}
  inline ~_CORBA_Bounded_Sequence_Array_w_FixSizeElement() {}
  inline _CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment> &
      operator= 
        (const 
          _CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>&
            s);
  inline _CORBA_ULong length() const;
  inline void length(_CORBA_ULong len);
  // omniORB2 extensions
  inline size_t NP_alignedSize(size_t initialoffset) const;
  inline void operator>>= (NetBufferedStream &s) const;
  inline void operator<<= (NetBufferedStream &s);
  inline void operator>>= (MemBufferedStream &s) const;
  inline void operator<<= (MemBufferedStream &s);
};

template<class T, class T_slice, int dimension, int max>
class _CORBA_Bounded_Sequence_Array__Boolean
   : public _CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Boolean,dimension,max,1,1>
{
public:
  inline _CORBA_Bounded_Sequence_Array__Boolean() {}
  inline _CORBA_Bounded_Sequence_Array__Boolean(_CORBA_ULong length,
						        _CORBA_Boolean *value,
							_CORBA_Boolean release = 0)
    :  _CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Boolean,dimension,max,1,1>(length,value,release) {}
  inline _CORBA_Bounded_Sequence_Array__Boolean(const 
      _CORBA_Bounded_Sequence_Array__Boolean<T,T_slice,dimension,max>& s) 
    :  _CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Boolean,dimension,max,1,1>(s) {}

  inline ~_CORBA_Bounded_Sequence_Array__Boolean() {}
};

template<class T, class T_slice, int dimension, int max>
class _CORBA_Bounded_Sequence_Array__Octet
   : public _CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Octet,dimension,max,1,1>
{
public:
  inline _CORBA_Bounded_Sequence_Array__Octet() {}
  inline _CORBA_Bounded_Sequence_Array__Octet(_CORBA_ULong length,
						        _CORBA_Octet *value,
							_CORBA_Boolean release = 0)
    :  _CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Octet,dimension,max,1,1>(length,value,release) {}
  inline _CORBA_Bounded_Sequence_Array__Octet(const 
      _CORBA_Bounded_Sequence_Array__Octet<T,T_slice,dimension,max>& s) 
    :  _CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,_CORBA_Octet,dimension,max,1,1>(s) {}

  inline ~_CORBA_Bounded_Sequence_Array__Octet() {}
};


typedef _CORBA_Unbounded_Sequence__Octet _CORBA_Unbounded_Sequence_Octet;

#endif // __SEQTEMPLATES_H__





