// -*- Mode: C++; -*-
//                            Package   : omniORB
// valueType.h                Created on: 2003/08/22
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2003 Apasphere Ltd.
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
//    ValueType definitions
//

/*
  $Log$
  Revision 1.1.2.2  2004/07/04 23:53:35  dgrisby
  More ValueType TypeCode and Any support.

  Revision 1.1.2.1  2003/09/26 16:12:53  dgrisby
  Start of valuetype support.

*/

#ifndef __OMNI_VALUETYPE_H__
#define __OMNI_VALUETYPE_H__

#include <omniORB4/valueFactoryManager.h>


struct _omni_ValueId {
  const char*  repoId;
  CORBA::ULong hashval;
};

struct _omni_ValueIds {
  CORBA::Long    idcount;
  CORBA::ULong   hashval;
  _omni_ValueId* repoIds;
};


class omniValueType {
public:
  static void marshal(CORBA::ValueBase* val,
		      const char* repoId,
		      cdrStream& stream);
  // Marshal the value, in a context where the IDL specifies the value
  // has type given by <repoId>.

  static CORBA::ValueBase* unmarshal(const char* repoId, CORBA::ULong hashval,
				     cdrStream& stream);
  // Unmarshal a value, in a context where the IDL specifies the value
  // has type given by <repoId>, with hash <hashval>.


  static inline CORBA::ULong hash_id(const char* id)
  {
    CORBA::ULong n = 0;
    while (*id)  n = ((n << 5) ^ (n >> 27)) ^ *id++;
    return n;
  }

  static inline CORBA::ULong hash_ids(const _omni_ValueIds* ids)
  {
    CORBA::ULong n = 0;
    
    for (CORBA::Long i=0; i < ids->idcount; i++)
      n = ((n << 5) ^ (n >> 27)) ^ ids->repoIds[i].hashval;

    return n;
  }
};


#endif // __OMNI_VALUETYPE_H__
