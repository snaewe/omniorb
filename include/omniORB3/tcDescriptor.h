// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcDescriptor.h             Created on: 9/98
//                            Author    : James Weatherall (jnw)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
// TypeCode-oriented data parser.
//
//  The tcParser class is initialised with a TypeCode and
// a MemBufferedStream. The MemBufferedStream is used to
// store data of type described by the associated TypeCode.
//
//  The operations <copyTo> and <copyFrom> are used to
// insert and extract the data from the MemBufferedStream.
// Overloaded versions are provided to marshal the data
// into and out of Mem and Net BufferedStreams - this is
// used for (un)marshalling values of type Any.
//
//  In addition the data passed into and out of the internal
// MemBufferedStream may be described by a tcDescriptor.
// The user of the tcParser will setup a tcDescriptor to
// describe where the data to be copied to/from the
// MemBufferedStream is in memory. For simple types this is
// a pointer to the location in memory. For more complex
// types the tcDescriptor provides call-backs to provide
// additional information such as the length and data of
// a string, or to create a tcDescriptor for the members
// of a struct.
//

#ifndef __TCDESCRIPTOR_H__
#define __TCDESCRIPTOR_H__

#include <omniORB3/CORBA.h>


// Forward declarations
union tcDescriptor;

// Complex type descriptors
struct tcObjrefDesc;

// Constructed type descriptors
struct tcUnionDesc;
struct tcStructDesc;
struct tcSequenceDesc;
struct tcArrayDesc;


//////////////////////////////////////////////////////////////////////
////////////////////// TYPECODE DATA DESCRIPTORS /////////////////////
//////////////////////////////////////////////////////////////////////

//
// NB: tcDescriptor storage is managed by whoever created the
// descriptor (and will usually be allocated on the stack).
// Data storage is always handled by the calling application.
//
//  Each descriptor class must provide accessor functions
// and is provided with a void* which must be initialised
// to point to the implementation of the data.
// When accessor functions are called, the relevant
// tc***Desc structure is also passed, through which the
// void * storage pointer may then be accessed.

// BASIC DESCRIPTOR CLASSES
//
// For types using these descriptor classes, the data should
// be filled in before the class is passed into the
// tcDescriptor.

//////////////
// tcObjref //
//////////////

typedef void (*tcObjrefSetReferenceFn)(tcObjrefDesc*, CORBA::Object_ptr);

typedef CORBA::Object_ptr (*tcObjrefGetReferenceFn)(tcObjrefDesc*);

struct tcObjrefDesc
{
  // Set the object reference from a supplied Object_ptr
  tcObjrefSetReferenceFn setObjectPtr;
  // Retrieve the Object_ptr version of the reference
  tcObjrefGetReferenceFn getObjectPtr;

  // Data members for use only in the callbacks
  void * opq_objref;
  CORBA::Boolean opq_release;
};

// COMPLEX DESCRIPTOR CLASSES
//
// These classes must have their methods overridden to
// retrieve tcDescriptors for their elements.

/////////////
// tcUnion //
/////////////

typedef void (*tcUnionGetDiscriminatorFn)
  (tcUnionDesc*, tcDescriptor&, CORBA::PR_unionDiscriminator&);
typedef void (*tcUnionSetDiscriminatorFn)
  (tcUnionDesc*, CORBA::PR_unionDiscriminator, int is_default);
typedef CORBA::Boolean (*tcUnionGetValueDescFn)
  (tcUnionDesc*, tcDescriptor& data_desc);

// This type is used to allocate storage for the maximum
// possible size of discriminator.
union tcUnionDiscriminatorType
{
  CORBA::Long u_long;
  CORBA::Short u_short;
  CORBA::ULong u_ulong;
  CORBA::UShort u_ushort;
  CORBA::Char u_char;
  CORBA::Boolean u_boolean;
  CORBA::ULong u_enum;
};

struct tcUnionDesc
{
  // This callback must fill in the descriptor for
  // the discriminator value, and also pass back the
  // discriminator value itself.
  tcUnionGetDiscriminatorFn getDiscriminator;

  // Parser passes descriptor indicating the discriminator
  // value it wishes the union to have.
  tcUnionSetDiscriminatorFn setDiscriminator;

  // Callback must fill in the descriptor for the
  // data member currently selected. If the current
  // value of the discriminant is invalid, returns
  // 0, otherwise 1.
  tcUnionGetValueDescFn getValueDesc;

  void * opq_union;
};

//////////////
// tcStruct //
//////////////

typedef CORBA::Boolean (*tcStructGetMemberDescFn)
  (tcStructDesc *, CORBA::ULong, tcDescriptor&);
typedef CORBA::ULong (*tcStructGetMemberCountFn)
  (tcStructDesc *);

struct tcStructDesc
{
  tcStructGetMemberDescFn getMemberDesc;
  tcStructGetMemberCountFn getMemberCount;
  void * opq_struct;
};

////////////////
// tcSequence //
////////////////

typedef CORBA::Boolean (*tcSeqGetElementDescFn)
  (tcSequenceDesc *, CORBA::ULong, tcDescriptor&);
typedef CORBA::ULong (*tcSeqGetElementCountFn)
  (tcSequenceDesc *);
typedef void (*tcSeqSetElementCountFn)
  (tcSequenceDesc *, CORBA::ULong);

struct tcSequenceDesc
{
  // Get a descriptor for a particular element
  tcSeqGetElementDescFn getElementDesc;
  // Get the sequence length
  tcSeqGetElementCountFn getElementCount;
  // Set the sequence length
  tcSeqSetElementCountFn setElementCount;
  void * opq_seq;
};

/////////////
// tcArray //
/////////////

typedef CORBA::Boolean (*tcArrayGetElementDescFn)
  (tcArrayDesc*, CORBA::ULong, tcDescriptor&);

struct tcArrayDesc
{
  tcArrayGetElementDescFn getElementDesc;
  void * opq_array;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// tcDescriptor ////////////////////////////
//////////////////////////////////////////////////////////////////////

union tcDescriptor {

  // BASIC types
  // appendItem() will read in the data pointed to
  // fetchItem() will overwrite the data pointed to with new data
  CORBA::Short*   p_short;
  CORBA::Long*    p_long;
  CORBA::UShort*  p_ushort;
  CORBA::ULong*   p_ulong;
  CORBA::Float*   p_float;
  CORBA::Double*  p_double;
  CORBA::Boolean* p_boolean;
  CORBA::Char*    p_char;
  CORBA::Octet*   p_octet;
  CORBA::ULong*   p_enum;

  CORBA::Any*     p_any;

  // COMPLEX types

  // appendItem() will read in the objects from the _ptrs pointed to
  // fetchItem() will overwrite the _ptr to point to a new object -
  //   if the _ptr is not nil, it will be released
  CORBA::TypeCode_member* p_TypeCode;

  // appendItem() will read in the objects from the _ptrs pointed to
  // fetchItem() will overwrite the _ptr to point to a new object - 
  //   if the _ptr still points to valid storage then that storage is lost
  CORBA::PrincipalID*  p_Principal;

  // appendItem() will read in the string from string pointer
  // fetchItem() will overwrite the string pointer to a new string - 
  //   if *p_string is not nil, it will be freed.
  char**               p_string;

  // CONSTRUCTED types
  // These types have manager classes to help handle them, since their
  // internal details are not generally known to the tcParser
  tcObjrefDesc p_objref;

  tcUnionDesc    p_union;
  tcStructDesc   p_struct;
  tcStructDesc   p_except;
  tcSequenceDesc p_sequence;
  tcArrayDesc    p_array;
};

//////////////////////////////////////////////////////////////////////
////////////////////// Data Descriptor Functions /////////////////////
//////////////////////////////////////////////////////////////////////

//
// The _0RL_tcParser_buildDesc(tcDescriptor &desc, T &data) function is
// overloaded for each datatype, to fill out the supplied descriptor with
// the information required by the tcParser to parse the data.
//  These functions are required by the Any marshalling stubs generated by
// the IDL compiler, to avoid having to make them fill out descriptors
// manually.
//

inline void
_0RL_buildDesc_cboolean(tcDescriptor &desc, const CORBA::Boolean& data)
{
  desc.p_boolean = (CORBA::Boolean*)&data;
}

inline void
_0RL_buildDesc_coctet(tcDescriptor &desc, const CORBA::Octet& data)
{
  desc.p_octet = (CORBA::Octet*)&data;
}

inline void
_0RL_buildDesc_cchar(tcDescriptor &desc, const CORBA::Char &data)
{
  desc.p_char = (CORBA::Char *)&data;
}

inline void
_0RL_buildDesc_cshort(tcDescriptor &desc, const CORBA::Short &data)
{
  desc.p_short = (CORBA::Short *)&data;
}

inline void
_0RL_buildDesc_cunsigned_pshort(tcDescriptor &desc, const CORBA::UShort &data)
{
  desc.p_ushort = (CORBA::UShort *)&data;
}

inline void
_0RL_buildDesc_clong(tcDescriptor &desc, const CORBA::Long &data)
{
  desc.p_long = (CORBA::Long *)&data;
}

inline void
_0RL_buildDesc_cunsigned_plong(tcDescriptor &desc, const CORBA::ULong &data)
{
  desc.p_ulong = (CORBA::ULong *)&data;
}

#if !defined(NO_FLOAT)
inline void
_0RL_buildDesc_cfloat(tcDescriptor &desc, const CORBA::Float &data)
{
  desc.p_float = (CORBA::Float *)&data;
}

inline void
_0RL_buildDesc_cdouble(tcDescriptor &desc, const CORBA::Double &data)
{
  desc.p_double = (CORBA::Double *)&data;
}
#endif

/////////
// Any //
/////////

inline void
_0RL_buildDesc_cany(tcDescriptor &desc, const CORBA::Any& data)
{
  desc.p_any = (CORBA::Any *)&data;
}

///////////////////
// String        //
///////////////////

inline void
_0RL_buildDesc_cstring(tcDescriptor &desc,_CORBA_String_member const& data)
{
  desc.p_string = &data._ptr;
}

///////////////////
// Object_member //
///////////////////

extern void
_0RL_tcParser_objref_setObjectPtr(tcObjrefDesc* desc, CORBA::Object_ptr ptr);

extern CORBA::Object_ptr
_0RL_tcParser_objref_getObjectPtr(tcObjrefDesc* desc);

inline void
_0RL_buildDesc_cCORBA_sObject(tcDescriptor& desc, const CORBA::Object_member& data)
{
  desc.p_objref.opq_objref = (void*) &data._ptr;
  desc.p_objref.opq_release = data.pd_rel;
  desc.p_objref.setObjectPtr = _0RL_tcParser_objref_setObjectPtr;
  desc.p_objref.getObjectPtr = _0RL_tcParser_objref_getObjectPtr;
}

#if 0
extern void
_0RL_tcParser_objref2_setObjectPtr(tcObjrefDesc* desc, CORBA::Object_ptr ptr);

extern CORBA::Object_ptr
_0RL_tcParser_objref2_getObjectPtr(tcObjrefDesc* desc);

inline void
_0RL_buildDesc_cCORBA_sObject(tcDescriptor& desc, const _CORBA_ObjRef_Member<CORBA::Object, CORBA::Object_Helper>& data)
{
  desc.p_objref.opq_objref = (void*) &data._ptr;
  desc.p_objref.setObjectPtr = _0RL_tcParser_objref2_setObjectPtr;
  desc.p_objref.getObjectPtr = _0RL_tcParser_objref2_getObjectPtr;
}
#endif

/////////////////////
// TypeCode_member //
/////////////////////

inline void
_0RL_buildDesc_cTypeCode(tcDescriptor& desc, const CORBA::TypeCode_member& data)
{
  desc.p_TypeCode = (CORBA::TypeCode_member*) &data;
}


#endif  // __TCDESCRIPTOR_H__
