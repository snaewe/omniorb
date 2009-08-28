// -*- Mode: C++; -*-
//                            Package   : omniORB
// seqTemplatedefns.h         Created on: 14/5/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2009 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library.
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

#ifndef __SEQTEMPLATEDEFNS_H__
#define __SEQTEMPLATEDEFNS_H__

//////////////////////////////////////////////////////////////////////
template <class T>
inline void
_CORBA_Unbounded_Sequence<T>::operator>>= (cdrStream& s) const
{
  s.marshalULong(this->pd_len);
  for (_CORBA_ULong i = 0; i < this->pd_len; i++)
    this->pd_buf[i] >>= s;
}


//////////////////////////////////////////////////////////////////////
template <class T>
inline void
_CORBA_Unbounded_Sequence<T>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  for (_CORBA_ULong i = 0; i < l; i++)
    this->pd_buf[i] <<= s;
}

//////////////////////////////////////////////////////////////////////
template <class T,int max>
inline void
_CORBA_Bounded_Sequence<T,max>::operator>>= (cdrStream& s) const
{
  s.marshalULong(this->pd_len);
  for (_CORBA_ULong i = 0; i < this->pd_len; i++)
    this->pd_buf[i] >>= s;
}


//////////////////////////////////////////////////////////////////////
template <class T,int max>
inline void
_CORBA_Bounded_Sequence<T,max>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (l > max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  for (_CORBA_ULong i = 0; i < l; i++)
    this->pd_buf[i] <<= s;
}

//////////////////////////////////////////////////////////////////////
template <class T,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::operator>>= (cdrStream& s) const
{
  if (s.marshal_byte_swap()) {
    Base_T_seq::operator>>=(s);
    return;
  }
  _CORBA_ULong l = Base_T_seq::length();
  l >>= s;
  if (l==0) return;
  s.put_octet_array((_CORBA_Octet*)Base_T_seq::NP_data(),
		    (int)l*elmSize,
		    (omni::alignment_t)elmAlignment);
}


//////////////////////////////////////////////////////////////////////
template <class T,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(elmSize,l)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  Base_T_seq::length(l);
  if (l==0) return;
  s.get_octet_array((_CORBA_Octet*)Base_T_seq::NP_data(),
		    (int)l*elmSize,
		    (omni::alignment_t)elmAlignment);

  if (s.unmarshal_byte_swap() && elmAlignment != 1) {
    if (elmSize == 2) {
      _CORBA_UShort* data = (_CORBA_UShort*)Base_T_seq::NP_data();
      
      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
    }
    else if (elmSize == 4) {
      _CORBA_ULong* data = (_CORBA_ULong*)Base_T_seq::NP_data();

      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
    }
    else if (elmSize == 8) {

#ifdef HAS_LongLong
      _CORBA_ULongLong* data = (_CORBA_ULongLong*)Base_T_seq::NP_data();

      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
#else
      l *= 2;
      _CORBA_ULong* data = (_CORBA_ULong*)Base_T_seq::NP_data();
      _CORBA_ULong temp;

      for (_CORBA_ULong i=0; i<l; i+=2) {
	temp      = cdrStream::byteSwap(data[i+i]);
	data[i+1] = cdrStream::byteSwap(data[i]);
	data[i]   = temp;
      }
#endif
    }
  }
}

//////////////////////////////////////////////////////////////////////
template <class T,int max,int elmSize,int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator>>= (cdrStream& s) const
{
  if (s.marshal_byte_swap()) {
    Base_T_seq::operator>>=(s);
    return;
  }
  _CORBA_ULong l = Base_T_seq::length();
  l >>= s;
  if (l==0) return;
  s.put_octet_array((_CORBA_Octet*)Base_T_seq::NP_data(),
		    (int)l*elmSize,
		    (omni::alignment_t)elmAlignment);
}


//////////////////////////////////////////////////////////////////////
template <class T,int max,int elmSize,int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(elmSize,l) || (l > max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  Base_T_seq::length(l);
  if (l==0) return;
  s.get_octet_array((_CORBA_Octet*)Base_T_seq::NP_data(),
		    (int)l*elmSize,
		    (omni::alignment_t)elmAlignment);

  if (s.unmarshal_byte_swap() && elmAlignment != 1) {
    if (elmSize == 2) {
      _CORBA_UShort* data = (_CORBA_UShort*)Base_T_seq::NP_data();
      
      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
    }
    else if (elmSize == 4) {
      _CORBA_ULong* data = (_CORBA_ULong*)Base_T_seq::NP_data();

      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
    }
    else if (elmSize == 8) {

#ifdef HAS_LongLong
      _CORBA_ULongLong* data = (_CORBA_ULongLong*)Base_T_seq::NP_data();

      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
#else
      l *= 2;
      _CORBA_ULong* data = (_CORBA_ULong*)Base_T_seq::NP_data();
      _CORBA_ULong temp;

      for (_CORBA_ULong i=0; i<l; i+=2) {
	temp      = cdrStream::byteSwap(data[i+i]);
	data[i+1] = cdrStream::byteSwap(data[i]);
	data[i]   = temp;
      }
#endif
    }
  }
}

//////////////////////////////////////////////////////////////////////
#ifdef OMNI_MIXED_ENDIAN_DOUBLE

// Template member function specializations to use the base
// marshalling functions for double, so the doubles are properly
// word-swapped.

template<>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Double,8,8>::operator>>= (cdrStream& s) const
{
  Base_T_seq::operator>>=(s);
}

template<>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Double,8,8>::operator<<= (cdrStream& s)
{
  Base_T_seq::operator<<=(s);
}

template<int max>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<_CORBA_Double,max,8,8>::operator>>= (cdrStream& s) const
{
  Base_T_seq::operator>>=(s);
}

template<int max>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<_CORBA_Double,max,8,8>::operator<<= (cdrStream& s)
{
  Base_T_seq::operator<<=(s);
}

#endif


//////////////////////////////////////////////////////////////////////
inline
void
_CORBA_Sequence_Char::operator>>= (cdrStream& s) const
{
  _CORBA_ULong l = Base_T_seq::length();
  l >>= s;
  for( _CORBA_ULong i = 0; i < l; i++ )
    s.marshalChar(this->pd_buf[i]);

}

//////////////////////////////////////////////////////////////////////
inline
void
_CORBA_Sequence_Char::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (this->pd_bounded && l > this->pd_max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  for( _CORBA_ULong i = 0; i < l; i++ )
    this->pd_buf[i] = s.unmarshalChar();
}

//////////////////////////////////////////////////////////////////////
inline
void
_CORBA_Sequence_Boolean::operator>>= (cdrStream& s) const
{
  _CORBA_ULong l = Base_T_seq::length();
  l >>= s;
  if (l==0) return;
# if !defined(HAS_Cplusplus_Bool) || (SIZEOF_BOOL == 1)
  s.put_octet_array((_CORBA_Octet*)this->pd_buf,l);
# else
  for ( _CORBA_ULong i = 0; i < l; i++ )
    s.marshalBoolean(this->pd_buf[i]);
# endif
}

//////////////////////////////////////////////////////////////////////
inline
void
_CORBA_Sequence_Boolean::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (pd_bounded && l > this->pd_max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  if (l==0) return;
# if !defined(HAS_Cplusplus_Bool) || (SIZEOF_BOOL == 1)
  s.get_octet_array((_CORBA_Octet*)this->pd_buf,l);
# else
  for ( _CORBA_ULong i = 0; i < l; i++ )
    this->pd_buf[i] = s.unmarshalBoolean();
# endif
}

//////////////////////////////////////////////////////////////////////
inline
void
_CORBA_Sequence_Octet::operator>>= (cdrStream& s) const
{
  _CORBA_ULong l = Base_T_seq::length();
  l >>= s;
  if (l==0) return;
  s.put_octet_array(this->pd_buf,l);
}

//////////////////////////////////////////////////////////////////////
inline
void
_CORBA_Sequence_Octet::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (this->pd_bounded && l > this->pd_max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  if (l==0) return;
  s.get_octet_array(this->pd_buf,l);
}

//////////////////////////////////////////////////////////////////////
inline
void
_CORBA_Sequence_WChar::operator>>= (cdrStream& s) const
{
  _CORBA_ULong l = Base_T_seq::length();
  l >>= s;
  for( _CORBA_ULong i = 0; i < l; i++ )
    s.marshalWChar(this->pd_buf[i]);
}

//////////////////////////////////////////////////////////////////////
inline
void
_CORBA_Sequence_WChar::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (this->pd_bounded && l > this->pd_max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  for( _CORBA_ULong i = 0; i < l; i++ )
    this->pd_buf[i] = s.unmarshalWChar();
}

//////////////////////////////////////////////////////////////////////
template <class T,class T_slice,class Telm,int dimension>
inline void
_CORBA_Unbounded_Sequence_Array<T,T_slice,Telm,dimension>::operator>>= (cdrStream& s) const
{
  this->pd_len >>= s;
  for (_CORBA_ULong i=0; i<this->pd_len; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      *((Telm*)(this->pd_buf[i]) + j) >>= s;
    }
  }
  return;
}


//////////////////////////////////////////////////////////////////////
template <class T,class T_slice,class Telm,int dimension>
inline void
_CORBA_Unbounded_Sequence_Array<T,T_slice,Telm,dimension>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      *((Telm*)(this->pd_buf[i]) + j) <<= s;
    }
  }
  return;
}

//////////////////////////////////////////////////////////////////////
template <class T,class T_slice,class Telm,int dimension,int max>
inline void
_CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max>::operator>>= (cdrStream& s) const
{
  this->pd_len >>= s;
  for (_CORBA_ULong i=0; i<this->pd_len; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      *((Telm*)(this->pd_buf[i]) + j) >>= s;
    }
  }
  return;
}


//////////////////////////////////////////////////////////////////////
template <class T,class T_slice,class Telm,int dimension,int max>
inline void
_CORBA_Bounded_Sequence_Array<T,T_slice,Telm,dimension,max>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (l > max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      *((Telm*)(this->pd_buf[i]) + j) <<= s;
    }
  }
  return;
}

//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
inline void
_CORBA_Sequence_Array_Char<T,T_slice,dimension>::operator>>=(cdrStream& s) const
{
  this->pd_len >>= s;
  for (_CORBA_ULong i=0; i<this->pd_len; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      s.marshalChar(*((_CORBA_Char*)(this->pd_buf[i]) + j));
    }
  }
}

//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
inline void
_CORBA_Sequence_Array_Char<T,T_slice,dimension>::operator<<=(cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (this->pd_bounded && l > this->pd_max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      *((_CORBA_Char*)(this->pd_buf[i]) + j) =  s.unmarshalChar();
    }
  }
}

//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
inline void
_CORBA_Sequence_Array_Boolean<T,T_slice,dimension>::operator>>=(cdrStream& s) const
{
  this->pd_len >>= s;
  if (this->pd_len==0) return;
# if !defined(HAS_Cplusplus_Bool) || (SIZEOF_BOOL == 1)
  s.put_octet_array((_CORBA_Octet*)this->pd_buf,(int)this->pd_len*dimension);
# else
  for (_CORBA_ULong i=0; i<this->pd_len; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      s.marshalBoolean(*((_CORBA_Boolean*)(this->pd_buf[i]) + j));
    }
  }
# endif
}

//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
inline void
_CORBA_Sequence_Array_Boolean<T,T_slice,dimension>::operator<<=(cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (this->pd_bounded && l > this->pd_max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  if (l==0) return;
# if !defined(HAS_Cplusplus_Bool) || (SIZEOF_BOOL == 1)
  s.get_octet_array((_CORBA_Octet*)this->pd_buf,(int)l*dimension);
# else
  for (_CORBA_ULong i=0; i<l; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      *((_CORBA_Boolean*)(this->pd_buf[i]) + j) =  s.unmarshalBoolean();
    }
  }
# endif
}
//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
inline void
_CORBA_Sequence_Array_Octet<T,T_slice,dimension>::operator>>=(cdrStream& s) const
{
  this->pd_len >>= s;
  if (this->pd_len==0) return;
  s.put_octet_array((_CORBA_Octet*)this->pd_buf,(int)this->pd_len*dimension);
}

//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
inline void
_CORBA_Sequence_Array_Octet<T,T_slice,dimension>::operator<<=(cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (this->pd_bounded && l > this->pd_max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  if (l==0) return;
  s.get_octet_array((_CORBA_Octet*)this->pd_buf,(int)l*dimension);
}

//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
inline void
_CORBA_Sequence_Array_WChar<T,T_slice,dimension>::operator>>=(cdrStream& s) const
{
  this->pd_len >>= s;
  for (_CORBA_ULong i=0; i<this->pd_len; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      s.marshalWChar(*((_CORBA_WChar*)(this->pd_buf[i]) + j));
    }
  }
}

//////////////////////////////////////////////////////////////////////
template<class T, class T_slice, int dimension>
inline void
_CORBA_Sequence_Array_WChar<T,T_slice,dimension>::operator<<=(cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (this->pd_bounded && l > this->pd_max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      *((_CORBA_WChar*)(this->pd_buf[i]) + j) =  s.unmarshalWChar();
    }
  }
}

//////////////////////////////////////////////////////////////////////
template <class T,class T_slice,class Telm,int dimension,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,elmSize,elmAlignment>::operator>>= (cdrStream& s) const
{
  if (s.marshal_byte_swap()) {
    Base_T_seq::operator>>=(s);
    return;
  }

  _CORBA_ULong l = Base_T_seq::length();
  l >>= s;
  if (l==0) return;
  s.put_octet_array((_CORBA_Octet*)Base_T_seq::NP_data(),
		    (int)l*dimension*elmSize,
		    (omni::alignment_t)elmAlignment);
}


//////////////////////////////////////////////////////////////////////
template <class T,class T_slice,class Telm,int dimension,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,elmSize,elmAlignment>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(elmSize,l*dimension)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  Base_T_seq::length(l);
  if (l==0) return;
  s.get_octet_array((_CORBA_Octet*)Base_T_seq::NP_data(),
		    (int)l*dimension*elmSize,
		    (omni::alignment_t)elmAlignment);

  if (s.unmarshal_byte_swap() && elmAlignment != 1) {
    l *= dimension;

    if (elmSize == 2) {
      _CORBA_UShort* data = (_CORBA_UShort*)Base_T_seq::NP_data();
      
      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
    }
    else if (elmSize == 4) {
      _CORBA_ULong* data = (_CORBA_ULong*)Base_T_seq::NP_data();

      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
    }
    else if (elmSize == 8) {

#ifdef HAS_LongLong
      _CORBA_ULongLong* data = (_CORBA_ULongLong*)Base_T_seq::NP_data();

      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
#else
      l *= 2;
      _CORBA_ULong* data = (_CORBA_ULong*)Base_T_seq::NP_data();
      _CORBA_ULong temp;

      for (_CORBA_ULong i=0; i<l; i+=2) {
	temp      = cdrStream::byteSwap(data[i+i]);
	data[i+1] = cdrStream::byteSwap(data[i]);
	data[i]   = temp;
      }
#endif
    }
  }
}

//////////////////////////////////////////////////////////////////////
template <class T,class T_slice,class Telm,int dimension,int max,int elmSize,int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>::operator>>= (cdrStream& s) const
{
  if (s.marshal_byte_swap()) {
    Base_T_seq::operator>>=(s);
    return;
  }

  _CORBA_ULong l = Base_T_seq::length();
  l >>= s;
  if (l==0) return;
  s.put_octet_array((_CORBA_Octet*)Base_T_seq::NP_data(),
		    (int)l*dimension*elmSize,
		    (omni::alignment_t)elmAlignment);
}


//////////////////////////////////////////////////////////////////////
template <class T,class T_slice,class Telm,int dimension,int max,int elmSize,int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(elmSize,l*dimension) || (l > max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  Base_T_seq::length(l);
  if (l==0) return;
  s.get_octet_array((_CORBA_Octet*)Base_T_seq::NP_data(),
		    (int)l*dimension*elmSize,
		    (omni::alignment_t)elmAlignment);

  if (s.unmarshal_byte_swap() && elmAlignment != 1) {
    l *= dimension;

    if (elmSize == 2) {
      _CORBA_UShort* data = (_CORBA_UShort*)Base_T_seq::NP_data();
      
      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
    }
    else if (elmSize == 4) {
      _CORBA_ULong* data = (_CORBA_ULong*)Base_T_seq::NP_data();

      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
    }
    else if (elmSize == 8) {

#ifdef HAS_LongLong
      _CORBA_ULongLong* data = (_CORBA_ULongLong*)Base_T_seq::NP_data();

      for (_CORBA_ULong i=0; i<l; i++) {
	data[i] = cdrStream::byteSwap(data[i]);
      }
#else
      l *= 2;
      _CORBA_ULong* data = (_CORBA_ULong*)Base_T_seq::NP_data();
      _CORBA_ULong temp;

      for (_CORBA_ULong i=0; i<l; i+=2) {
	temp      = cdrStream::byteSwap(data[i+i]);
	data[i+1] = cdrStream::byteSwap(data[i]);
	data[i]   = temp;
      }
#endif
    }
  }
}

//////////////////////////////////////////////////////////////////////
template <class T, class T_Elem,class T_Helper>
inline void
_CORBA_Sequence_ObjRef<T,T_Elem,T_Helper>::operator>>= (cdrStream& s) const
{
  s.marshalULong(this->pd_len);
  for( int i = 0; i < (int)this->pd_len; i++ )
    T_Helper::marshalObjRef(pd_buf[i],s);
}


//////////////////////////////////////////////////////////////////////
template <class T, class T_Elem,class T_Helper>
inline void
_CORBA_Sequence_ObjRef<T,T_Elem,T_Helper>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (this->pd_bounded && l > this->pd_max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }

  _CORBA_ULong i;

  if (this->pd_rel) {
    T* nil_ = T_Helper::_nil();
    for (i=0; i < this->pd_len; i++) {
      T_Helper::release(pd_buf[i]);
      pd_buf[i] = nil_;
    }
  }
  this->pd_len = 0;

  this->length(l);
  for (i = 0; i < l; i++)
    this->pd_buf[i] = T_Helper::unmarshalObjRef(s);
}


#endif // __SEQTEMPLATEDEFNS_H__
