// -*- Mode: C++; -*-
//                            Package   : omniORB
// anyStream.h                Created on: 2004/06/21
//                            Author    : Duncan Grisby
//
//
//    Copyright (C) 2004 Apasphere Ltd.
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
//    cdrMemoryStream extension used by Anys.
//

#ifndef __ANYSTREAM_H__
#define __ANYSTREAM_H__

#include <omniORB4/omniTypedefs.hh>

#ifdef _dyn_attr
# error "A local CPP macro _dyn_attr has already been defined."
#endif

#if defined(_OMNIORB_DYNAMIC_LIBRARY)
#  define _dyn_attr
#else
#  define _dyn_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

class cdrAnyMemoryStream : public cdrMemoryStream {
public:
  cdrAnyMemoryStream();

  cdrAnyMemoryStream(const cdrAnyMemoryStream& s);

  ~cdrAnyMemoryStream();

  cdrAnyMemoryStream& operator=(const cdrMemoryStream&);

  virtual void* ptrToClass(int* cptr);
  static inline cdrAnyMemoryStream* downcast(cdrStream* s) {
    return (cdrAnyMemoryStream*)s->ptrToClass(&_classid);
  }
  static _dyn_attr int _classid;

  inline omniTypedefs::ValueBaseSeq& valueSeq()
  {
    if (pd_values.operator->() == 0)
      pd_values = new omniTypedefs::ValueBaseSeq;
    return pd_values;
  }

  inline void clearValueSeq()
  {
    pd_values = 0;
  }

private:
  // ValueTypes inside Anys cannot be stored inside the marshalled
  // stream like other types, because to do so would not have the
  // required sharing semantics. Instead, we store a sequence of
  // values here. Inside the stream, values are stored as a ulong
  // representing an index into this sequence. The sequence is only
  // allocated if there are values inside the Any.
  omniTypedefs::ValueBaseSeq_var pd_values;
};


OMNI_NAMESPACE_END(omni)

#undef _dyn_attr

#endif // __ANYSTREAM_H__
