// -*- Mode: C++; -*-
//                            Package   : omniORB2
// seqTemplatedefns.h         Created on: 14/5/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2007 Apasphere Ltd
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
  Revision 1.1.4.4  2007/04/12 19:50:32  dgrisby
  A few cases of sizeof(bool) > 1 were not handled correctly.

  Revision 1.1.4.3  2006/04/28 18:40:46  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.4.2  2005/01/06 17:31:06  dgrisby
  Changes (mainly from omni4_0_develop) to compile on gcc 3.4.

  Revision 1.1.2.11  2003/05/22 13:41:39  dgrisby
  HPUX patches.

  Revision 1.1.4.1  2003/03/23 21:04:02  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.10  2002/10/15 23:25:45  dgrisby
  DOH!

  Revision 1.1.2.9  2002/10/15 23:23:29  dgrisby
  Forgot case of compilers with no bool.

  Revision 1.1.2.8  2002/10/14 15:10:09  dgrisby
  Cope with platforms where sizeof(bool) != 1.

  Revision 1.1.2.7  2001/08/03 17:41:16  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.2.6  2001/05/04 11:24:46  sll
  Wrong bound check in all sequence array templates.

  Revision 1.1.2.5  2000/11/20 18:56:39  sll
  Sequence templates were broken by the previous checkin. They are now fixed.

  Revision 1.1.2.4  2000/11/20 14:41:44  sll
  Simplified sequence template hierachy and added templates for sequence of
  wchar and sequence of array of wchar.

  Revision 1.1.2.3  2000/11/07 18:19:54  sll
  Revert to use explicit castings in the marshalling operators of sequence of
  arrays.

  Revision 1.1.2.2  2000/11/03 19:02:46  sll
  Separate out the marshalling of byte, octet and char into 3 set of distinct
  marshalling functions. Sequence of and sequence of array of these types
  updated correspondingly.

  Revision 1.1.2.1  2000/09/27 16:54:09  sll
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

//////////////////////////////////////////////////////////////////////
template <class T>
inline void
_CORBA_Unbounded_Sequence<T>::operator>>= (cdrStream& s) const
{
  ::operator>>=(_CORBA_ULong(this->pd_len), s);
  for( int i = 0; i < (int)this->pd_len; i++ )
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
  for( _CORBA_ULong i = 0; i < l; i++ )
    this->pd_buf[i] <<= s;
}

//////////////////////////////////////////////////////////////////////
template <class T,int max>
inline void
_CORBA_Bounded_Sequence<T,max>::operator>>= (cdrStream& s) const
{
  ::operator>>=(_CORBA_ULong(this->pd_len), s);
  for( int i = 0; i < (int)this->pd_len; i++ )
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
  for( _CORBA_ULong i = 0; i < l; i++ )
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
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_UShort t = ((_CORBA_UShort*)Base_T_seq::NP_data())[i];
	((_CORBA_UShort*)Base_T_seq::NP_data())[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_ULong t = ((_CORBA_ULong*)Base_T_seq::NP_data())[i];
	((_CORBA_ULong*)Base_T_seq::NP_data())[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      l *= 2;
      for (_CORBA_ULong i=0; i<l; i+=2) {
	_CORBA_ULong tl1 = ((_CORBA_ULong*)Base_T_seq::NP_data())[i+1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong*)Base_T_seq::NP_data())[i];
	((_CORBA_ULong*)Base_T_seq::NP_data())[i] = tl2;
	((_CORBA_ULong*)Base_T_seq::NP_data())[i+1] = Swap32(tl1);
      }
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
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_UShort t = ((_CORBA_UShort*)Base_T_seq::NP_data())[i];
	((_CORBA_UShort*)Base_T_seq::NP_data())[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_ULong t = ((_CORBA_ULong*)Base_T_seq::NP_data())[i];
	((_CORBA_ULong*)Base_T_seq::NP_data())[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      l *= 2;
      for (_CORBA_ULong i=0; i<l; i+=2) {
	_CORBA_ULong tl1 = ((_CORBA_ULong*)Base_T_seq::NP_data())[i+1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong*)Base_T_seq::NP_data())[i];
	((_CORBA_ULong*)Base_T_seq::NP_data())[i] = tl2;
	((_CORBA_ULong*)Base_T_seq::NP_data())[i+1] = Swap32(tl1);
      }
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
  if (!s.checkInputOverrun(1,l) || (this->pd_bounded && l > this->pd_max)) {
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
    if (elmSize == 2) {
      l *= dimension;
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_UShort t = ((_CORBA_UShort*)Base_T_seq::NP_data())[i];
	((_CORBA_UShort*)Base_T_seq::NP_data())[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      l *= dimension;
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_ULong t = ((_CORBA_ULong*)Base_T_seq::NP_data())[i];
	((_CORBA_ULong*)Base_T_seq::NP_data())[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      l *= 2*dimension;
      for (_CORBA_ULong i=0; i<l; i+=2) {
	_CORBA_ULong tl1 = ((_CORBA_ULong*)Base_T_seq::NP_data())[i+1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong*)Base_T_seq::NP_data())[i];
	((_CORBA_ULong*)Base_T_seq::NP_data())[i] = tl2;
	((_CORBA_ULong*)Base_T_seq::NP_data())[i+1] = Swap32(tl1);
      }
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
    if (elmSize == 2) {
      l *= dimension;
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_UShort t = ((_CORBA_UShort*)Base_T_seq::NP_data())[i];
	((_CORBA_UShort*)Base_T_seq::NP_data())[i] = Swap16(t);
      }
    }
    else if (elmSize == 4) {
      l *= dimension;
      for (_CORBA_ULong i=0; i<l; i++) {
	_CORBA_ULong t = ((_CORBA_ULong*)Base_T_seq::NP_data())[i];
	((_CORBA_ULong*)Base_T_seq::NP_data())[i] = Swap32(t);
      }
    }
    else if (elmSize == 8) {
      l *= 2*dimension;
      for (_CORBA_ULong i=0; i<l; i+=2) {
	_CORBA_ULong tl1 = ((_CORBA_ULong*)Base_T_seq::NP_data())[i+1];
	_CORBA_ULong tl2 = Swap32(tl1);
	tl1 = ((_CORBA_ULong*)Base_T_seq::NP_data())[i];
	((_CORBA_ULong*)Base_T_seq::NP_data())[i] = tl2;
	((_CORBA_ULong*)Base_T_seq::NP_data())[i+1] = Swap32(tl1);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////
template <class T, class ElemT,class T_Helper>
inline void
_CORBA_Sequence_ObjRef<T,ElemT,T_Helper>::operator>>= (cdrStream& s) const
{
  ::operator>>=(_CORBA_ULong(this->pd_len), s);
  for( int i = 0; i < (int)this->pd_len; i++ )
    T_Helper::marshalObjRef(pd_data[i],s);
}


//////////////////////////////////////////////////////////////////////
template <class T, class ElemT,class T_Helper>
inline void
_CORBA_Sequence_ObjRef<T,ElemT,T_Helper>::operator<<= (cdrStream& s)
{
  _CORBA_ULong l;
  l <<= s;
  if (!s.checkInputOverrun(1,l) || (this->pd_bounded && l > this->pd_max)) {
    _CORBA_marshal_sequence_range_check_error(s);
    // never reach here
  }
  this->length(l);
  for( _CORBA_ULong i = 0; i < l; i++ )
    operator[](i) = T_Helper::unmarshalObjRef(s);
}

#undef Swap16
#undef Swap32

#endif // __SEQTEMPLATEDEFNS_H__
