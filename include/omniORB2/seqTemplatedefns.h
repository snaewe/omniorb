// -*- Mode: C++; -*-
//                            Package   : omniORB2
// seqTemplatedefns.h         Created on: 14/5/96
//                            Author    : Sai Lai Lo (sll)
//
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
//	*** PROPRIETORY INTERFACE ***

/*
  $Log$
  Revision 1.1.2.1  1999/09/15 20:35:43  sll
  *** empty log message ***


*/

#ifndef __SEQTEMPLATEDEFNS_H__
#define __SEQTEMPLATEDEFNS_H__

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

template <class T>
inline void
_CORBA_Sequence<T>::operator>>= (cdrStream& s) const
{
  ::operator>>=(_CORBA_ULong(pd_len), s);
  for( int i = 0; i < (int)pd_len; i++ )
    pd_buf[i] >>= s;
}


template <class T>
inline void
_CORBA_Sequence<T>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (pd_bounded && l > pd_max)) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(l);
  for( _CORBA_ULong i = 0; i < l; i++ )
    pd_buf[i] <<= s;
}

template <class T,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::operator>>= (cdrStream& s) const
{
  if (s.marshal_byte_swap()) {
    BaseT::operator>>=(s);
    return;
  }

  _CORBA_ULong l = BaseT::length();
  l >>= s;
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    s.alignOutput(omni::ALIGN_8);
  }
  s.put_char_array((_CORBA_Char*)BaseT::NP_data(),(int)l*elmSize);
}


template <class T,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_w_FixSizeElement<T,elmSize,elmAlignment>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(elmSize,l)) {
    _CORBA_marshal_error();
    // never reach here
  }
  BaseT::length(l);
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    s.alignInput(omni::ALIGN_8);
  }
  s.get_char_array((_CORBA_Char*)BaseT::NP_data(),(int)l*elmSize);
  if (s.unmarshal_byte_swap() && elmAlignment != 1) {
    if (elmSize == 2) {
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_UShort t = ((_CORBA_UShort*)BaseT::NP_data())[i];
	((_CORBA_UShort*)BaseT::NP_data())[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_ULong t = ((_CORBA_ULong*)BaseT::NP_data())[i];
	((_CORBA_ULong*)BaseT::NP_data())[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      l *= 2;
      for (_CORBA_ULong i=0; i<l; i+=2) {
	_CORBA_ULong tl1 = ((_CORBA_ULong*)BaseT::NP_data())[i+1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong*)BaseT::NP_data())[i];
	((_CORBA_ULong*)BaseT::NP_data())[i] = tl2;
	((_CORBA_ULong*)BaseT::NP_data())[i+1] = Swap32(tl1);
      }
    }
  }
}

template <class T,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator>>= (cdrStream& s) const
{
  if (s.marshal_byte_swap()) {
    BaseT::operator>>=(s);
    return;
  }

  _CORBA_ULong l = length();
  l >>= s;
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    s.alignOutput(omni::ALIGN_8);
  }
  s.put_char_array((_CORBA_Char*)BaseT::NP_data(),(int)l*elmSize);
}


template <class T,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(elmSize,l) || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  
  length(l);
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    s.alignInput(omni::ALIGN_8);
  }
  s.get_char_array((_CORBA_Char*)BaseT::NP_data(),(int)l*elmSize);
  if (s.unmarshal_byte_swap() && elmAlignment != 1) {
    if (elmSize == 2) {
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_UShort t = ((_CORBA_UShort*)BaseT::NP_data())[i];
	((_CORBA_UShort*)BaseT::NP_data())[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_ULong t = ((_CORBA_ULong*)BaseT::NP_data())[i];
	((_CORBA_ULong*)BaseT::NP_data())[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      l *= 2;
      for (_CORBA_ULong i=0; i<l; i+=2) {
	_CORBA_ULong tl1 = ((_CORBA_ULong*)BaseT::NP_data())[i+1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong*)BaseT::NP_data())[i];
	((_CORBA_ULong*)BaseT::NP_data())[i] = tl2;
	((_CORBA_ULong*)BaseT::NP_data())[i+1] = Swap32(tl1);
      }
    }
  }
}


//////////////////////////////////////////////////////////////////////

template <class T,class T_slice,class Telm,int dimension>
inline void
_CORBA_Sequence_Array<T,T_slice,Telm,dimension>::operator>>= (cdrStream& s) const
{
  pd_len >>= s;
  for (_CORBA_ULong i=0; i<pd_len; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      *((Telm*)(pd_buf[i])+j) >>= s;
    }
  }
  return;
}


template <class T,class T_slice,class Telm,int dimension>
inline void
_CORBA_Sequence_Array<T,T_slice,Telm,dimension>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (s.checkInputOverrun(1,l) || (pd_bounded && l > pd_max)) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      *((Telm*)(pd_buf[i])+j) <<= s;
    }
  }
  return;
}


template <class T,class T_slice,class Telm,int dimension,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,elmSize,elmAlignment>::operator>>= (cdrStream& s) const
{
  if (s.marshal_byte_swap()) {
    BaseT::operator>>=(s);
    return;
  }

  _CORBA_ULong l = BaseT::length();
  l >>= s;
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    s.alignOutput(omni::ALIGN_8);
  }
  s.put_char_array((_CORBA_Char*)BaseT::NP_data(),(int)l*dimension*elmSize);
}


template <class T,class T_slice,class Telm,int dimension,int elmSize,int elmAlignment>
inline
void
_CORBA_Unbounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,elmSize,elmAlignment>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(elmSize,l*dimension)) {
    _CORBA_marshal_error();
    // never reach here
  }
  BaseT::length(l);
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    s.alignInput(omni::ALIGN_8);
  }
  s.get_char_array((_CORBA_Char*)BaseT::NP_data(),(int)l*dimension*elmSize);
  if (s.unmarshal_byte_swap() && elmAlignment != 1) {
    if (elmSize == 2) {
      l *= dimension;
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_UShort t = ((_CORBA_UShort*)BaseT::NP_data())[i];
	((_CORBA_UShort*)BaseT::NP_data())[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      l *= dimension;
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_ULong t = ((_CORBA_ULong*)BaseT::NP_data())[i];
	((_CORBA_ULong*)BaseT::NP_data())[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      l *= 2*dimension;
      for (_CORBA_ULong i=0; i<l; i+=2) {
	_CORBA_ULong tl1 = ((_CORBA_ULong*)BaseT::NP_data())[i+1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong*)BaseT::NP_data())[i];
	((_CORBA_ULong*)BaseT::NP_data())[i] = tl2;
	((_CORBA_ULong*)BaseT::NP_data())[i+1] = Swap32(tl1);
      }
    }
  }
}

template <class T,class T_slice,class Telm,int dimension,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>::operator>>= (cdrStream& s) const
{
  if (s.marshal_byte_swap()) {
    BaseT::operator>>=(s);
    return;
  }

  _CORBA_ULong l = BaseT::length();
  l >>= s;
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    s.alignOutput(omni::ALIGN_8);
  }
  s.put_char_array((_CORBA_Char*)BaseT::NP_data(),(int)l*dimension*elmSize);
}


template <class T,class T_slice,class Telm,int dimension,int max,int elmSize, int elmAlignment>
inline
void
_CORBA_Bounded_Sequence_Array_w_FixSizeElement<T,T_slice,Telm,dimension,max,elmSize,elmAlignment>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(elmSize,l*dimension) || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  
  BaseT::length(l);
  if (l==0) return;
  if ((int)elmAlignment == (int)omni::ALIGN_8) {
    s.alignInput(omni::ALIGN_8);
  }
  s.get_char_array((_CORBA_Char*)BaseT::NP_data(),(int)l*dimension*elmSize);
  if (s.unmarshal_byte_swap() && elmAlignment != 1) {
    if (elmSize == 2) {
      l *= dimension;
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_UShort t = ((_CORBA_UShort*)BaseT::NP_data())[i];
	((_CORBA_UShort*)BaseT::NP_data())[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      l *= dimension;
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_ULong t = ((_CORBA_ULong*)BaseT::NP_data())[i];
	((_CORBA_ULong*)BaseT::NP_data())[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      l *= 2*dimension;
      for (_CORBA_ULong i=0; i<l; i+=2) {
	_CORBA_ULong tl1 = ((_CORBA_ULong*)BaseT::NP_data())[i+1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong*)BaseT::NP_data())[i];
	((_CORBA_ULong*)BaseT::NP_data())[i] = tl2;
	((_CORBA_ULong*)BaseT::NP_data())[i+1] = Swap32(tl1);
      }
    }
  }
}

#ifdef HAS_Cplusplus_Bool
// Real C++ bool is used to represent CORBA::Boolean. The marshalling
// functions for sequence/sequence-array of fixed size elements cannot
// be used because the C++ bool may or may not be the same size as a
// CORBA::Char. Define the marshalling functions here:

void
_CORBA_Unbounded_Sequence__Boolean::operator>>= (cdrStream& s) const
{
  BaseT::operator>>=(s);
}


void
_CORBA_Unbounded_Sequence__Boolean::operator<<= (cdrStream& s)
{
  BaseT::operator<<=(s);
}


template <int max>
void
_CORBA_Bounded_Sequence__Boolean<max>::operator>>= (cdrStream& s) const
{
  BaseT::operator>>=(s);
}


template <int max>
void
_CORBA_Bounded_Sequence__Boolean<max>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  
  BaseT::length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    BaseT::NP_data()[i] <<= s;
  }
  return;
}

template<class T, class T_slice, int dimension>
void
_CORBA_Unbounded_Sequence_Array__Boolean<T,T_slice,dimension>::
    operator>>= (cdrStream& s) const
{
  BaseT::operator>>=(s);
}


template<class T, class T_slice, int dimension>
void
_CORBA_Unbounded_Sequence_Array__Boolean<T,T_slice,dimension>::
    operator<<= (cdrStream& s)
{
  BaseT::operator<<=(s);
}


template<class T, class T_slice, int dimension, int max>
void
_CORBA_Bounded_Sequence_Array__Boolean<T,T_slice,dimension,max>::
   operator>>= (cdrStream& s) const
{
  BaseT::operator>>=(s);
}


template<class T, class T_slice, int dimension, int max>
void
_CORBA_Bounded_Sequence_Array__Boolean<T,T_slice,dimension,max>::
    operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || l > max) {
    _CORBA_marshal_error();
    // never reach here
  }
  
  BaseT::length(l);
  for (_CORBA_ULong i=0; i<l; i++) {
    for (_CORBA_ULong j=0; j<dimension; j++) {
      *((_CORBA_Boolean*)(BaseT::NP_data()[i])+j) <<= s;
    }
  }
  return;
}

#endif


template <class T, class ElemT,class T_Helper>
inline void
_CORBA_Sequence_ObjRef<T,ElemT,T_Helper>::operator>>= (cdrStream& s) const
{
  ::operator>>=(_CORBA_ULong(pd_len), s);
  for( int i = 0; i < (int)pd_len; i++ )
    T_Helper::marshalObjRef(pd_data[i],s);
}


template <class T, class ElemT,class T_Helper>
inline void
_CORBA_Sequence_ObjRef<T,ElemT,T_Helper>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (pd_bounded && l > pd_max)) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(l);
  for( _CORBA_ULong i = 0; i < l; i++ )
    operator[](i) = T_Helper::unmarshalObjRef(s);
}

#undef Swap16
#undef Swap32

#endif // __SEQTEMPLATEDEFNS_H__
