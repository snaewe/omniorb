// -*- Mode: C++; -*-
//                            Package   : omniORB2
// bufStream_templates.h      Created on: 19/6/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:


/*
 $Log$
 Revision 1.1  1997/01/08 17:28:30  sll
 Initial revision

 */
#ifndef __BUFSTREAM_TEMPLATES_H__
#define __BUFSTREAM_TEMPLATES_H__

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

template <class T,int max,int elmSize, int elmAlignment>
inline void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator<<= (NetBufferedStream &s)
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

template <class T,int max,int elmSize, int elmAlignment>
inline void
_CORBA_Bounded_Sequence_w_FixSizeElement<T,max,elmSize,elmAlignment>::operator<<= (MemBufferedStream &s)
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



#endif // __BUFSTREAM_TEMPLATES_H__
