// -*- Mode: C++; -*-
//                            Package   : omniORB2
// typecode.h                 Created on: 03/09/98
//                            Author1   : James Weatherall (jnw)
//                            Author2   : David Riddoch (djr)
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
//   Internal typecode classes.
//

/*
 * $Log$
 * Revision 1.7  1999/07/01 10:28:14  djr
 * Added two methods to TypeCode_pairlist.
 * Declare inline methods as inline.
 *
 * Revision 1.6  1999/06/18 21:00:31  sll
 * Updated to CORBA 2.3 mapping.
 *
 * Revision 1.5  1999/05/25 17:47:09  sll
 * Changed to use _CORBA_PseudoValue_Sequence.
 *
 * Revision 1.4  1999/03/11 16:25:59  djr
 * Updated copyright notice
 *
 * Revision 1.3  1999/02/18 15:49:30  djr
 * Simplified NP_namesEqualOrNull()
 *
 * Revision 1.2  1999/01/18 13:56:20  djr
 * Fixed bug in TypeCode_union.
 *
 * Revision 1.1  1999/01/07 17:12:37  djr
 * Initial revision
 *
 */

#ifndef __TYPECODE_H__
#define __TYPECODE_H__

#include <omniORB2/CORBA.h>
#include <omniutilities.h>


///////////////////////////
// Forward class defines //
///////////////////////////

// Buffer marshalling & unmarshalling
class TypeCode_offsetTable;
class TypeCode_offsetEntry;

// Helper class used by the NP_equal function
class TypeCode_pairlist;

// TypeCode internal marshalling class
// This class takes care of marshalling & unmarshalling recursive typecodes
class TypeCode_marshaller;

// TypeCode reference counting class
class TypeCode_collector;

// TypeCode implementation classes
class TypeCode_base;

class TypeCode_string;
class TypeCode_objref;
class TypeCode_alias;
class TypeCode_sequence;
class TypeCode_array;
class TypeCode_struct;
class TypeCode_except;
class TypeCode_enum;
class TypeCode_union;


// TypeCode parameter list types:
enum TypeCode_paramListType {
  plt_None,
  plt_Simple,
  plt_Complex
};

////////////
// Macros //
////////////
// to cast a CORBA::TypeCode_ptr into a TypeCode_base*

#define ToTcBase(a) ((TypeCode_base*)(CORBA::TypeCode_ptr)a)

inline TypeCode_base*
ToTcBase_Checked(CORBA::TypeCode_ptr a)
{
  if( CORBA::is_nil(a) )  throw CORBA::BAD_TYPECODE(0, CORBA::COMPLETED_NO);

  return (TypeCode_base*) a;
}

inline const TypeCode_base*
ToConstTcBase_Checked(const CORBA::TypeCode* a)
{
  if (CORBA::is_nil((CORBA::TypeCode_ptr)a))
    throw CORBA::BAD_TYPECODE(0, CORBA::COMPLETED_NO);

  return (const TypeCode_base*)a;
}

//////////////////////////////////////////////////////////////////////
///////////////////////// TypeCode_alignTable ////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_alignTable {
public:

  enum InfoType {
    it_simple,             // know size and alignment
    it_nasty               // data is variable length, or dodgy alignment
  };

  struct Alignment {
    InfoType type;
    union {
      struct {
	omni::alignment_t alignment;
	CORBA::ULong      size;
      } simple;
      struct {
	TypeCode_base*    tc;
      } nasty;
    };
  };

  inline TypeCode_alignTable() : pd_entries(0), pd_table(0), pd_owns(0) {}
  inline ~TypeCode_alignTable() {
    if( pd_table ) {
      for( unsigned i = 0; i < pd_entries; i++ )
	if( i_own(i) )  delete pd_table[i];
      delete[] pd_table;
      delete[] pd_owns;
    }
  }

  inline void setNumEntries(unsigned n) {
    pd_table = (new Alignment*[n]);
    unsigned owns_len = n & 31 ? (n>>5) + 1 : (n>>5);
    pd_owns = new CORBA::ULong[owns_len];
    for( unsigned i = 0; i < owns_len; i++ )  pd_owns[i] = 0;
    for( unsigned j = 0; j < n; j++ )         pd_table[j] = 0;
  }
  inline void add(const TypeCode_alignTable& at, unsigned ati) {
    pd_table[pd_entries++] = at.pd_table[ati];
  }
  inline void set(const TypeCode_alignTable& at) {
    setNumEntries(at.entries());
    for( unsigned i = 0; i < at.entries(); i++ )  add(at, i);
  }
  inline void addSimple(omni::alignment_t a, size_t s) {
    unsigned i = new_entry();
    pd_table[i]->type = it_simple;
    pd_table[i]->simple.alignment = a;
    pd_table[i]->simple.size = s;
  }
  inline void addNasty(TypeCode_base* tc) {
    unsigned i = new_entry();
    pd_table[i]->type = it_nasty;
    pd_table[i]->nasty.tc = tc;
  }

  inline CORBA::Boolean is_simple() const {
    return pd_entries == 1 && pd_table[0]->type == it_simple;
  }
  inline CORBA::Boolean has_only_simple() const {
    for( unsigned i = 0; i < pd_entries; i++ )
      if( pd_table[i]->type != it_simple )  return 0;
    return 1;
  }

  inline unsigned entries() const { return pd_entries; }
  inline const Alignment& operator [] (unsigned i) const {
    return *(pd_table[i]);
  }

private:
  inline int i_own(unsigned i) const {
    return pd_owns[i >> 5] & (1 << (i & 31));
  }
  inline void set_i_own(unsigned i) { pd_owns[i >> 5] |= (1 << (i & 31)); }
  inline unsigned new_entry() {
    pd_table[pd_entries] = new Alignment;
    set_i_own(pd_entries);
    return pd_entries++;
  }

  unsigned      pd_entries;
  Alignment**   pd_table;
  CORBA::ULong* pd_owns;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// TypeCode_base ///////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_base : public CORBA::TypeCode {
public:

  TypeCode_base(CORBA::TCKind tck);

  virtual ~TypeCode_base();

  // omniORB2 marshalling routines specific to simple typecodes
  virtual void NP_marshalSimpleParams(NetBufferedStream& nbuf,
				      TypeCode_offsetTable*) const;
  virtual void NP_marshalSimpleParams(MemBufferedStream& nbuf,
				      TypeCode_offsetTable*) const;

  // omniORB2 marshalling routines specific to complex typecodes
  virtual void NP_marshalComplexParams(MemBufferedStream&,
				       TypeCode_offsetTable*) const;

  // omniORB2 recursive typecode & reference count handling
  virtual CORBA::Boolean NP_complete_recursive_sequences(TypeCode_base* tc,
							 CORBA::ULong offset) {
    return 1;
  }

  // omniORB2 functions to get the aligned size of the parameter data
  virtual size_t NP_alignedSimpleParamSize(size_t initialoffset,
					   TypeCode_offsetTable* otbl) const;
  virtual size_t NP_alignedComplexParamSize(size_t initialoffset,
					    TypeCode_offsetTable* otbl) const;

  // omniORB2 equality check support functions
  static const TypeCode_base* NP_expand(const TypeCode_base* tc);
  inline static CORBA::Boolean NP_namesEqual(const char* name1,
					     const char* name2) {
    // returns 1 if both strings are nil or both strings are non-nil and
    // with the same content.
    if (name1 && name2) {
      return (!strcmp(name1,name2));
    }
    else {
      return (name1 == name2);
    }
  }

  // omniORB2 internal versions of the OMG TypeCode interface
  inline CORBA::TCKind NP_kind() const { return pd_tck; }

  CORBA::Boolean NP_equal(const TypeCode_base* TCp,
			  CORBA::Boolean equivalent,
			  const TypeCode_pairlist* pl) const;
  // If equivalent == 1, perform equivalent test as defined in the
  //                     CORBA 2.3 TypeCode::equivalent() operation.
  // else
  //     perform equality test as defined in the CORBA 2.3 TypeCode::equal()
  //     operation.

  virtual CORBA::Boolean NP_extendedEqual(const TypeCode_base* TCp,
					  CORBA::Boolean equivalent,
					  const TypeCode_pairlist* pl) const;
  // If equivalent == 1, perform equivalent test as defined in the
  //                     CORBA 2.3 TypeCode::equivalent() operation.
  // else
  //     perform equality test as defined in the CORBA 2.3 TypeCode::equal()
  //     operation.

  virtual const char*    NP_id() const;
  virtual const char*    NP_name() const;
  virtual CORBA::ULong   NP_member_count() const;
  virtual const char*    NP_member_name(CORBA::ULong index) const;
  virtual TypeCode_base* NP_member_type(CORBA::ULong index) const;
  virtual CORBA::Any*    NP_member_label(CORBA::ULong index) const;
  virtual TypeCode_base* NP_discriminator_type() const;
  virtual CORBA::Long    NP_default_index() const;
  virtual CORBA::ULong   NP_length() const;
  virtual TypeCode_base* NP_content_type() const;
  virtual CORBA::Long    NP_param_count() const;
  virtual CORBA::Any*    NP_parameter(CORBA::Long) const;

  virtual CORBA::Boolean NP_is_nil() const;

  inline const TypeCode_alignTable& alignmentTable() const {
    return pd_alignmentTable;
  }

  virtual CORBA::Boolean NP_containsAnAlias();
  // Returns true if this TypeCode or any of its members
  // is an alias.

  virtual TypeCode_base* NP_aliasExpand(TypeCode_pairlist*);
  // Return a TypeCode equivalent to this, but with aliases expanded
  // to the actual type.  It is assumed that this will only be called
  // if necassary - ie. the instance it is invoked on really does
  // contain an alias.  This is necassary to reduce the number of
  // calls to NP_containsAnAlias that are necassary.

  TypeCode_base* NP_compactTc();
  // Return a TypeCode equivalent to this, but with the optional
  // name and member_names removed.

  virtual void removeOptionalNames();
  // This internal function removes any optional names and member_names
  // from the typecode

  static TypeCode_base* aliasExpand(TypeCode_base* tc);
  // Return a duplicate of <tc> with aliases expanded to
  // the true type.

  inline TypeCode_base* aliasExpandedTc() { return pd_aliasExpandedTc; }
  // Returns the alias expanded TypeCode if it has been generated.

  inline CORBA::Boolean complete() { return pd_complete; }

protected:
  TypeCode_alignTable pd_alignmentTable;

  // Flag indicating whether this typecode & all it's child typecodes
  // are 'complete' (i.e. they contain no unresolved recursive sequences)
  CORBA::Boolean pd_complete;

  // The garbage collection and marshalling classes require access to the
  // internal details of typecodes
  friend class TypeCode_collector;
  friend class TypeCode_marshaller;

  // Garbage-collection related fields.
  CORBA::Boolean pd_mark;
  CORBA::ULong   pd_ref_count;

  CORBA::Boolean pd_loop_member;
  CORBA::ULong   pd_internal_ref_count;
  CORBA::ULong   pd_internal_depth;

  // Placeholder for the cached on-the-wire form of the typecode's
  // parameter list. This is only used for complex typecodes that aren't
  // part of a loop.
  MemBufferedStream* pd_cached_paramlist;

  // A pointer to the alias expanded version of this typecode
  // if one has been generated, 0 otherwise. If the alias expanded
  // version == this, then the reference is not duplicated.
  TypeCode_base* pd_aliasExpandedTc;

  // A pointer to the compact version of this typecode if one has
  // been generated, 0 otherwise. If the compact version == this,
  // then the reference is not duplicated.
  // This compact version  has all the optional name and member_name 
  // field stripped.
  TypeCode_base* pd_compactTc;

private:
  TypeCode_base();

  // The Kind of this TypeCode object
  CORBA::TCKind pd_tck;
};

//////////////////////////////////////////////////////////////////////////
// Complex TypeCode sub-classes, used to generate typecodes for complex //
// types. The following typecode sub-classes all use TypeCode_base as   //
// their base, so that all they have to do is override the functions    //
// relevant to their type.                                              //
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_string //////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_string : public TypeCode_base {
public:

  // omniORB interface:
  TypeCode_string(CORBA::ULong maxLen);

  virtual ~TypeCode_string();

  virtual void NP_marshalSimpleParams(NetBufferedStream&,
				      TypeCode_offsetTable*) const;
  virtual void NP_marshalSimpleParams(MemBufferedStream&,
				      TypeCode_offsetTable*) const;

  static TypeCode_base* NP_unmarshalSimpleParams(NetBufferedStream& s,
						 TypeCode_offsetTable*);
  static TypeCode_base* NP_unmarshalSimpleParams(MemBufferedStream& s,
						 TypeCode_offsetTable*);

  virtual size_t NP_alignedSimpleParamSize(size_t initialoffset,
					   TypeCode_offsetTable* otbl) const;

  // OMG Interface:
  virtual CORBA::Boolean NP_extendedEqual(const TypeCode_base* TCp,
					  CORBA::Boolean equivalent,
					  const TypeCode_pairlist* tcpl) const;

  virtual CORBA::ULong NP_length() const;
  virtual CORBA::Long NP_param_count() const;
  virtual CORBA::Any* NP_parameter(CORBA::Long) const;

private:
  TypeCode_string();

  CORBA::ULong pd_length;
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_objref //////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_objref : public TypeCode_base {
public:

  TypeCode_objref(const char* repositoryId, const char* name);

  virtual ~TypeCode_objref();

  // omniORB2 marshalling routines specific to complex types
  virtual void NP_marshalComplexParams(MemBufferedStream&,
				       TypeCode_offsetTable*) const;

  static TypeCode_base* NP_unmarshalComplexParams(MemBufferedStream& s,
						  TypeCode_offsetTable*);

  virtual TypeCode_paramListType NP_paramListType() const;
  virtual size_t NP_alignedComplexParamSize(size_t initialoffset,
					    TypeCode_offsetTable* otbl) const;

  // OMG Interface:
  virtual CORBA::Boolean NP_extendedEqual(const TypeCode_base* TCp,
					  CORBA::Boolean equivalent,
					  const TypeCode_pairlist* tcpl) const;

  virtual const char* NP_id() const;
  virtual const char* NP_name() const;

  virtual CORBA::Long NP_param_count() const;
  virtual CORBA::Any* NP_parameter(CORBA::Long) const;

  virtual void removeOptionalNames();

private:
  TypeCode_objref();

  CORBA::String_member pd_repoId;
  CORBA::String_member pd_name;
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_alias ///////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_alias : public TypeCode_base {
public:

  TypeCode_alias(const char* repositoryId,
		 const char* name,
		 TypeCode_base* contentTC);

  virtual ~TypeCode_alias();

  // omniORB2 marshalling routines specific to complex types
  virtual void NP_marshalComplexParams(MemBufferedStream&,
				       TypeCode_offsetTable*) const;

  static TypeCode_base* NP_unmarshalComplexParams(MemBufferedStream& s,
						  TypeCode_offsetTable*);

  // omniORB2 recursive typecode handling
  virtual CORBA::Boolean NP_complete_recursive_sequences(TypeCode_base* tc,
							 CORBA::ULong offset);

  // omniORB2 parameter list handling
  virtual size_t NP_alignedComplexParamSize(size_t initialoffset,
					    TypeCode_offsetTable* otbl) const;

  // OMG Interface:
  virtual CORBA::Boolean NP_extendedEqual(const TypeCode_base* TCp,
					  CORBA::Boolean equivalent,
					  const TypeCode_pairlist* tcpl) const;

  virtual const char* NP_id() const;
  virtual const char* NP_name() const;

  virtual TypeCode_base* NP_content_type() const;

  virtual CORBA::Long NP_param_count() const;
  virtual CORBA::Any* NP_parameter(CORBA::Long) const;

  virtual CORBA::Boolean NP_containsAnAlias();
  virtual TypeCode_base* NP_aliasExpand(TypeCode_pairlist*);

  virtual void removeOptionalNames();

private:
  inline TypeCode_alias() : TypeCode_base(CORBA::tk_alias) {}

  CORBA::String_member pd_repoId;
  CORBA::String_member pd_name;
  CORBA::TypeCode_member pd_content;
};

//////////////////////////////////////////////////////////////////////
////////////////////////// TypeCode_sequence /////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_sequence : public TypeCode_base {
public:

  TypeCode_sequence(CORBA::ULong maxLen, TypeCode_base* contentTC);
  TypeCode_sequence(CORBA::ULong maxLen, CORBA::ULong offset);

  virtual ~TypeCode_sequence();

  // omniORB2 marshalling routines specific to complex types
  virtual void NP_marshalComplexParams(MemBufferedStream&,
				       TypeCode_offsetTable*) const;

  static TypeCode_base* NP_unmarshalComplexParams(MemBufferedStream& s,
						  TypeCode_offsetTable*);

  // omniORB2 recursive typecode handling
  virtual CORBA::Boolean NP_complete_recursive_sequences(TypeCode_base* tc,
							 CORBA::ULong offset);

  // omniORB2 parameter list handling
  virtual TypeCode_paramListType NP_paramListType() const;
  virtual size_t NP_alignedComplexParamSize(size_t initialoffset,
					    TypeCode_offsetTable* otbl) const;

  // OMG Interface:
  virtual CORBA::Boolean NP_extendedEqual(const TypeCode_base* TCp,
					  CORBA::Boolean equivalent,
					  const TypeCode_pairlist* tcpl) const;

  virtual CORBA::ULong   NP_length() const;
  virtual TypeCode_base* NP_content_type() const;
  virtual CORBA::Long    NP_param_count() const;
  virtual CORBA::Any*    NP_parameter(CORBA::Long) const;

  virtual CORBA::Boolean NP_containsAnAlias();
  virtual TypeCode_base* NP_aliasExpand(TypeCode_pairlist*);

  virtual void removeOptionalNames();

private:
  TypeCode_sequence();

  CORBA::ULong pd_length;
  CORBA::TypeCode_member pd_content;

  CORBA::ULong pd_offset;
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_array ///////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_array : public TypeCode_base {
public:

  TypeCode_array(CORBA::ULong maxLen, TypeCode_base* contentTC);

  virtual ~TypeCode_array();

  // omniORB2 marshalling routines specific to complex types
  virtual void NP_marshalComplexParams(MemBufferedStream&,
				       TypeCode_offsetTable*) const;

  static TypeCode_base* NP_unmarshalComplexParams(MemBufferedStream& s,
						  TypeCode_offsetTable*);

  // omniORB2 recursive typecode handling
  virtual CORBA::Boolean NP_complete_recursive_sequences(TypeCode_base* tc,
							 CORBA::ULong offset);

  // omniORB2 parameter list handling
  virtual size_t NP_alignedComplexParamSize(size_t initialoffset,
					    TypeCode_offsetTable* otbl) const;

  // OMG Interface:
  virtual CORBA::Boolean NP_extendedEqual(const TypeCode_base* TCp,
					  CORBA::Boolean equivalent,
					  const TypeCode_pairlist* tcpl) const;

  virtual CORBA::ULong NP_length() const;
  virtual TypeCode_base* NP_content_type() const;
  virtual CORBA::Long NP_param_count() const;
  virtual CORBA::Any* NP_parameter(CORBA::Long) const;

  virtual CORBA::Boolean NP_containsAnAlias();
  virtual TypeCode_base* NP_aliasExpand(TypeCode_pairlist*);

  virtual void removeOptionalNames();

private:
  inline TypeCode_array() : TypeCode_base(CORBA::tk_array) {}

  void generateAlignmentTable();

  CORBA::ULong pd_length;
  CORBA::TypeCode_member pd_content;
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_struct //////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_struct : public TypeCode_base {
public:

  TypeCode_struct(char* repositoryId, char* name,
		  CORBA::PR_structMember* members,
		  CORBA::ULong memberCount);
  // Consumes <repositoryId>, <name> and <members> (and all
  // the strings and TypeCodes in <members>). Assumes all
  // arguments are present and correct.

  virtual ~TypeCode_struct();

  // omniORB2 marshalling routines specific to complex types
  virtual void NP_marshalComplexParams(MemBufferedStream&,
				       TypeCode_offsetTable*) const;

  static TypeCode_base* NP_unmarshalComplexParams(MemBufferedStream&,
						  TypeCode_offsetTable*);

  // omniORB2 recursive typecode handling
  virtual CORBA::Boolean NP_complete_recursive_sequences(TypeCode_base* tc,
							 CORBA::ULong offset);

  // omniORB2 parameter list handling
  virtual size_t NP_alignedComplexParamSize(size_t initialoffset,
					    TypeCode_offsetTable* otbl) const;

  // OMG Interface:
  virtual CORBA::Boolean NP_extendedEqual(const TypeCode_base* TCp,
					  CORBA::Boolean equivalent,
					  const TypeCode_pairlist* tcpl) const;

  virtual const char* NP_id() const;
  virtual const char* NP_name() const;
  virtual CORBA::ULong NP_member_count() const;
  virtual const char*  NP_member_name(CORBA::ULong index) const;
  virtual TypeCode_base* NP_member_type(CORBA::ULong index) const;
  virtual CORBA::Long NP_param_count() const;
  virtual CORBA::Any* NP_parameter(CORBA::Long) const;

  virtual CORBA::Boolean NP_containsAnAlias();
  virtual TypeCode_base* NP_aliasExpand(TypeCode_pairlist*);

  virtual void removeOptionalNames();

private:
  inline TypeCode_struct()
    : TypeCode_base(CORBA::tk_struct),
      pd_members(0), pd_nmembers(0) {}
  // Private constructor - used when unmarshalling a TypeCode.

  void generateAlignmentTable();

  CORBA::String_member pd_repoId;
  CORBA::String_member pd_name;
  CORBA::PR_structMember* pd_members;
  CORBA::ULong pd_nmembers;
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_except //////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_except : public TypeCode_base {
public:

  TypeCode_except(char* repositoryId, char* name,
		  CORBA::PR_structMember* members,
		  CORBA::ULong memberCount);
  // Consumes <repositoryId>, <name> and <members> (and all
  // the strings and TypeCodes in <members>). Assumes all
  // arguments are present and correct.

  virtual ~TypeCode_except();

  // omniORB2 marshalling routines specific to complex types
  virtual void NP_marshalComplexParams(MemBufferedStream&,
				       TypeCode_offsetTable*) const;

  static TypeCode_base* NP_unmarshalComplexParams(MemBufferedStream&,
						  TypeCode_offsetTable*);

  // omniORB2 recursive typecode handling
  virtual CORBA::Boolean NP_complete_recursive_sequences(TypeCode_base* tc,
							 CORBA::ULong offset);

  // omniORB2 parameter list handling
  virtual size_t NP_alignedComplexParamSize(size_t initialoffset,
					    TypeCode_offsetTable* otbl) const;

  // OMG Interface:
  virtual CORBA::Boolean NP_extendedEqual(const TypeCode_base* TCp,
					  CORBA::Boolean equivalent,
					  const TypeCode_pairlist* tcpl) const;

  virtual const char* NP_id() const;
  virtual const char* NP_name() const;
  virtual CORBA::ULong NP_member_count() const;
  virtual const char* NP_member_name(CORBA::ULong index) const;
  virtual TypeCode_base* NP_member_type(CORBA::ULong index) const;
  virtual CORBA::Long NP_param_count() const;
  virtual CORBA::Any* NP_parameter(CORBA::Long) const;

  virtual CORBA::Boolean NP_containsAnAlias();
  virtual TypeCode_base* NP_aliasExpand(TypeCode_pairlist*);

  virtual void removeOptionalNames();

private:
  inline TypeCode_except()
    : TypeCode_base(CORBA::tk_except),
      pd_members(0), pd_nmembers(0) {}

  void generateAlignmentTable();

  CORBA::String_member pd_repoId;
  CORBA::String_member pd_name;
  CORBA::PR_structMember* pd_members;
  CORBA::ULong pd_nmembers;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// TypeCode_enum ///////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_enum : public TypeCode_base {
public:

  TypeCode_enum(const char* repositoryId,
		const char* name,
		const CORBA::EnumMemberSeq& members);

  virtual ~TypeCode_enum();

  // omniORB2 marshalling routines specific to complex types
  virtual void NP_marshalComplexParams(MemBufferedStream&,
				       TypeCode_offsetTable*) const;

  static TypeCode_base* NP_unmarshalComplexParams(MemBufferedStream& s,
						  TypeCode_offsetTable*);

  // omniORB2 parameter list handling
  virtual size_t NP_alignedComplexParamSize(size_t initialoffset,
					    TypeCode_offsetTable* otbl) const;

  // OMG Interface:
  virtual CORBA::Boolean NP_extendedEqual(const TypeCode_base* TCp,
					  CORBA::Boolean equivalent,
					  const TypeCode_pairlist* tcpl) const;

  virtual const char* NP_id() const;
  virtual const char* NP_name() const;
  virtual CORBA::ULong NP_member_count() const;
  virtual const char*  NP_member_name(CORBA::ULong index) const;
  virtual CORBA::Long NP_param_count() const;
  virtual CORBA::Any* NP_parameter(CORBA::Long) const;

  CORBA::Long NP_member_index(const char* name) const;
  // Return the index of the member with the given name,
  // or -1 if no such member exists.

  virtual void removeOptionalNames();

private:
  TypeCode_enum();

  CORBA::String_member pd_repoId;
  CORBA::String_member pd_name;

  CORBA::EnumMemberSeq pd_members;
  //?? Consider also storing a lookup array sorted on the name to
  // enable faster lookups.
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// TypeCode_union ///////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_union : public TypeCode_base {
public:
  // This type is used to store discriminator values for all types
  // of discriminator - so must be able to store values as large
  // as the largest discriminator type.
  typedef CORBA::PR_unionDiscriminator Discriminator;

  TypeCode_union(const char* repoId, const char* name,
		 TypeCode_base* discTC, const CORBA::UnionMemberSeq& members);

  TypeCode_union(const char* repoId, const char* name,
		 TypeCode_base* discTC, const CORBA::PR_unionMember* members,
		 CORBA::ULong memberCount, CORBA::Long deflt);

  virtual ~TypeCode_union();

  // omniORB2 marshalling routines specific to complex types
  virtual void NP_marshalComplexParams(MemBufferedStream&,
				       TypeCode_offsetTable*) const;

  static TypeCode_base* NP_unmarshalComplexParams(MemBufferedStream& s,
						  TypeCode_offsetTable*);

  // omniORB2 recursive typecode handling
  virtual CORBA::Boolean NP_complete_recursive_sequences(TypeCode_base* tc,
							 CORBA::ULong offset);

  // omniORB2 parameter list handling
  virtual size_t NP_alignedComplexParamSize(size_t initialoffset,
					    TypeCode_offsetTable* otbl) const;

  // OMG Interface:
  virtual CORBA::Boolean NP_extendedEqual(const TypeCode_base* TCp,
					  CORBA::Boolean equivalent,
					  const TypeCode_pairlist* tcpl) const;

  virtual const char*    NP_id() const;
  virtual const char*    NP_name() const;
  virtual CORBA::ULong   NP_member_count() const;
  virtual const char*    NP_member_name(CORBA::ULong index) const;
  virtual TypeCode_base* NP_member_type(CORBA::ULong index) const;
  virtual CORBA::Any*    NP_member_label(CORBA::ULong index) const;
  virtual TypeCode_base* NP_discriminator_type() const;
  virtual CORBA::Long    NP_default_index() const;
  virtual CORBA::Long    NP_param_count() const;
  virtual CORBA::Any*    NP_parameter(CORBA::Long) const;

  inline Discriminator NP_member_label_val(CORBA::ULong index) const {
    return pd_members[index].alabel;
  }
  CORBA::Long NP_index_from_discriminator(Discriminator) const;
  // Returns the index of the member corresponding to the given
  // discriminator value. Returns -1 if the value is invalid.
  // (This can only happen if there is no default).

  Discriminator NP_default_value();
  // Returns a value which may be used as either explicit or
  // implicit default for this union.
  //  Must not be called if the union has no default.

  virtual CORBA::Boolean NP_containsAnAlias();
  virtual TypeCode_base* NP_aliasExpand(TypeCode_pairlist*);

  virtual void removeOptionalNames();

private:
  TypeCode_union();

  struct TcUnionMember {
    CORBA::String_member   aname;
    Discriminator          alabel;  // undefined if default member
    CORBA::TypeCode_member atype;
  };
  typedef _CORBA_PseudoValue_Sequence<TcUnionMember> TcUnionMemberSeq;

  CORBA::String_member   pd_repoId;
  CORBA::String_member   pd_name;
  CORBA::TypeCode_member pd_discrim_tc;

# define TYPECODE_UNION_IMPLICIT_DEFAULT  -1
# define TYPECODE_UNION_NO_DEFAULT        -2
  CORBA::Long pd_default;
  // Index of the default member in in <pd_members>.
  // TYPECODE_UNION_IMPLICIT_DEFAULT if there is no explicit
  // default but there is an implicit default value.
  // TYPECODE_UNION_NO_DEFAULT if all cases are given explicitly.

  TcUnionMemberSeq pd_members;

  // This is a value which may be used as an explicit or implicit
  // default value for the union.
  CORBA::Boolean pd_have_calculated_default_value;
  Discriminator  pd_default_value;
};

//////////////////////////////////////////////////////////////////////
//////////////////////// TypeCode_offsetEntry ////////////////////////
//////////////////////////////////////////////////////////////////////

// Internal buffer-offset to typecode mapping class

class TypeCode_offsetEntry
{
private:
  friend class TypeCode_offsetTable;

  TypeCode_offsetEntry* pd_next;
  CORBA::ULong pd_offset;
  TypeCode_base* pd_typecode;
};

//////////////////////////////////////////////////////////////////////
//////////////////////// TypeCode_offsetTable ////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_offsetTable
{
public:
  TypeCode_offsetTable();
  ~TypeCode_offsetTable();

  // Constructor to create a new offset table based on an existing one
  // When the new table is created, it assumes the currentOffset field
  // of the parent table to be the base location.
  // EXAMPLE:
  //  .1) The user creates an offsetTable
  //  .2) The user does some work, leaving the currentOffset at _n_.
  //  .3) The user now wants to deal with the offsetTable as if the current
  //      offset wasn't _n_ but was, instead _base_.
  //  .4) A new offsetTable, B, is created, with initial offset of _base_.
  //      Now, a call such as B->addEntry(_m_, _tc_) will be mapped to
  //                          A->addEntry(_m_ + (_n_ - _base_), _tc)
  //      i.e.  The offset passed to B->addEntry is relative to offset zero
  //            in table B.  The is automatically mapped to the corresponding
  //            offset in table A.
  TypeCode_offsetTable(TypeCode_offsetTable* parent, CORBA::Long base_offset);

  // Routine to add an offset->typecode mapping
  void addEntry(CORBA::Long offset, TypeCode_base* typecode);

  // Routines to retrieve typecode by offset or vica versa
  TypeCode_base* lookupOffset(CORBA::Long offset);
  CORBA::Boolean lookupTypeCode(const TypeCode_base* tc, CORBA::Long& offset);

  // Routine to retrieve the current buffer offset
  inline CORBA::Long currentOffset()   { return pd_curr_offset; }
  inline void setOffset(CORBA::Long i) { pd_curr_offset = i;    }

private:
  TypeCode_offsetEntry* pd_table;
  CORBA::Long pd_curr_offset;

  // Fields for use when this offsetTable is actually just a wrapper round
  // an existing offsetTable
  TypeCode_offsetTable* pd_parent_table;
  CORBA::Long pd_parent_base_offset;
};

//////////////////////////////////////////////////////////////////////
////////////////////////// TypeCode_pairlist /////////////////////////
//////////////////////////////////////////////////////////////////////

// Instances of this class are chained together on the stack and are used
// by NP_equal to avoid loops in typecode comparisons

class TypeCode_pairlist
{
public:
  inline TypeCode_pairlist(const TypeCode_pairlist* next,
			   const TypeCode_base* tc1,
			   const TypeCode_base* tc2)
    : d_next(next), d_tc1(tc1), d_tc2(tc2) {}

  const TypeCode_pairlist* d_next;
  const TypeCode_base*     d_tc1;
  const TypeCode_base*     d_tc2;

  static int contains(const TypeCode_pairlist*,
		      const TypeCode_base*, const TypeCode_base*);
  // Returns true if the given list contains the given pair.

  static const TypeCode_base* search(const TypeCode_pairlist*,
				     const TypeCode_base*);
  // Search for given typecode in the pair list, and if
  // found as the second of a pair, return the first in
  // the same pair.  Otherwise return 0.
};

//////////////////////////////////////////////////////////////////////
///////////////////////// TypeCode_marshaller ////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_marshaller
{
public:

  // Marshal/unmarshal to NetBufferedStream
  static void marshal(TypeCode_base* obj,
		      NetBufferedStream& s,
		      TypeCode_offsetTable* otbl);
  static TypeCode_base* unmarshal(NetBufferedStream& s,
				   TypeCode_offsetTable* otbl);

  // Marshal/unmarshal to MemBufferedStream
  static void marshal(TypeCode_base* obj,
		      MemBufferedStream& s,
		      TypeCode_offsetTable* otbl);
  static TypeCode_base* unmarshal(MemBufferedStream& s,
				   TypeCode_offsetTable* otbl);

  // Calculate how much space the typecode will take up in a stream
  static size_t alignedSize(const TypeCode_base* obj,
			    size_t offset,
			    TypeCode_offsetTable* otbl);

  // Find out what kind of parameter list the given TypeCode Kind requires
  static TypeCode_paramListType paramListType(CORBA::ULong kind);
};

//////////////////////////////////////////////////////////////////////
///////////////////////// TypeCode_collector /////////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_collector
{
public:

  // Duplicate a typecode pointer
  static TypeCode_base* duplicateRef(TypeCode_base* tc);

  // Release a typecode pointer
  static void releaseRef(TypeCode_base* tc);

  // Traverse a typecode tree, marking all nodes that are part of a loop
  static void markLoopMembers(TypeCode_base* tc);

private:

  // Internal routines used by releaseRef

  static CORBA::ULong markLoops(TypeCode_base* tc, CORBA::ULong depth);

  static void countInternalRefs(TypeCode_base* tc);
  static CORBA::Boolean checkInternalRefs(TypeCode_base* tc,
					  CORBA::ULong depth);
};

//////////////////////////////////////////////////////////////////////
//////////////////////// TypeCode_union_helper ///////////////////////
//////////////////////////////////////////////////////////////////////

class TypeCode_union_helper
{
public:

  static TypeCode_union::Discriminator
    extractLabel(const CORBA::Any& discval, CORBA::TypeCode_ptr tc);
  // Extract a discriminator value from the given Any, and cast it to
  // the TypeCode_union::Discriminator type. Checks that the value in
  // <discval> is of type <tc>, and throws BAD_PARAM if not.
  // Also checks that <tc> is a legal union disciminator type,
  // and throws BAD_TYPECODE if not.

  static void insertLabel(CORBA::Any& label,
			  TypeCode_union::Discriminator,
			  CORBA::TypeCode_ptr tc);
  // Insert the discriminator value into the given Any, with the
  // required type.

  static void marshalLabel(TypeCode_union::Discriminator,
			   CORBA::TypeCode_ptr tc, MemBufferedStream& s);
  static void marshalLabel(TypeCode_union::Discriminator,
			   CORBA::TypeCode_ptr tc, NetBufferedStream& s);
  // Marshal a discriminator value (as the given type).

  static TypeCode_union::Discriminator
    unmarshalLabel(CORBA::TypeCode_ptr tc, MemBufferedStream& s);
  static TypeCode_union::Discriminator
    unmarshalLabel(CORBA::TypeCode_ptr tc, NetBufferedStream& s);
  // Unmarshal a discriminator value of the given type from the stream.
  // Throws CORBA::MARSHAL on error.

  static size_t labelAlignedSize(size_t initoffset,
				 CORBA::TypeCode_ptr tc);
  // Aligned size of this (discriminator) type for marshalling.

  static CORBA::Boolean has_implicit_default(TypeCode_base* tc);
  // True if the union has an implicit default - ie. no explicit
  // default given, and not all values of the discriminator type
  // have been used.
  //  This need only be called by the TypeCode_union constructor,
  // as it caches the result.
};


#endif // __TYPECODE_H__
