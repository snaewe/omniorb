// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_sequence.cc         Created on: 12/08/1996
//			    Author    : Sai-Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
//
//  This file is part of omniidl2.
//
//  Omniidl2 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// Description:


/*
  $Log$
  Revision 1.9  1997/12/23 19:28:29  sll
  Now generate correct template argument for sequence<array of sequence>.

  Revision 1.8  1997/12/18 17:28:50  sll
  *** empty log message ***

  Revision 1.7  1997/12/09 19:55:14  sll
  *** empty log message ***

  Revision 1.6  1997/08/21 21:17:27  sll
  Added support for sequence of array. It was missing previously.

// Revision 1.5  1997/05/06  14:05:26  sll
// Public release.
//
  */

/*
  Example:

  // IDL
  typedef sequence<char,10> char_boundedseq;

  // C++
  class char_boundedseq {
  public:
     char_boundedseq();
     char_boundedseq(CORBA::ULong,CORBA::Char *,CORBA::Boolean rel=0);
     char_boundedseq(const char_boundedseq &);
     ~char_boundedseq();
     char_boundedseq &operator= (const char_boundedseq &);
     CORBA::ULong maximum() const;
     void length(CORBA::ULong);
     CORBA::ULong length() const;
     CORBA::Char &operator[] (CORBA::ULong);
     const CORBA::Char &operator[] (CORBA::ULong) const;

     static CORBA::Char *allocbuf(CORBA::ULong);
     static void freebuf(CORBA::Char *);
  };

  class char_boundedseq_var {
  public:
     char_boundedseq_var();
     char_boundedseq_var(char_boundedseq *);
     char_boundedseq_var(const char_boundedseq_var &);
     ~char_boundedseq_var();
     char_boundedseq_var &operator=(char_boundedseq *);
     char_boundedseq_var &operator=(const char_boundedseq_var &);
     char_boundedseq *operator-> const ();
     CORBA::Char & operator[] (CORBA::ULong index);
     const CORBA::Char & operator[] (CORBA::ULong index);

     // conversion operators to support parameter passing
     operator char_boundedseq *() const;
     operator const char_boundedseq &() const;
     operator char_boundedseq &();
     
  };


  // IDL
  typedef sequence<char>    char_unboundedseq;

  // C++
  class char_unboundedseq {
  public:
     char_unboundedseq();
     char_unboundedseq(CORBA::ULong);
     char_unboundedseq(CORBA::ULong,CORBA::ULong,
                       CORBA::Char *,CORBA::Boolean rel=0);
     char_unboundedseq(const char_unboundedseq &);
     ~char_unboundedseq();
     char_unboundedseq &operator= (const char_unboundedseq &);
     CORBA::ULong maximum() const;
     void length(CORBA::ULong);
     CORBA::ULong length() const;
     CORBA::Char &operator[] (CORBA::ULong);
     const CORBA::Char &operator[] (CORBA::ULong) const;

     static CORBA::Char *allocbuf(CORBA::ULong);
     static void freebuf(CORBA::Char *);
  };

  class char_unboundedseq_var {
  public:
     char_unboundedseq_var();
     char_unboundedseq_var(char_unboundedseq *);
     char_unboundedseq_var(const char_unboundedseq_var &);
     ~char_unboundedseq_var();
     char_unboundedseq_var &operator=(char_unboundedseq *);
     char_unboundedseq_var &operator=(const char_unboundedseq_var &);
     char_unboundedseq *operator-> const ();
     CORBA::Char & operator[] (CORBA::ULong index);
     const CORBA::Char & operator[] (CORBA::ULong index);

     // conversion operators to support parameter passing
     operator char_unboundedseq *() const;
     operator const char_unboundedseq &() const;
     operator char_unboundedseq &();
  };

  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>
#include <stdio.h>

#define SEQUENCE_TYPE_PREFIX "_IDL_SEQUENCE_"
#define SEQUENCE_TEMPLATE_UNBOUNDED                "_CORBA_Unbounded_Sequence"
#define SEQUENCE_TEMPLATE_BOUNDED                  "_CORBA_Bounded_Sequence"
#define SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT "_CORBA_Unbounded_Sequence_w_FixSizeElement"
#define SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT "_CORBA_Bounded_Sequence_w_FixSizeElement"
#define SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY    "_CORBA_Unbounded_Sequence_Array"
#define SEQUENCE_TEMPLATE_BOUNDED_ARRAY      "_CORBA_Bounded_Sequence_Array"
#define SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY_W_FIXSIZEELEMENT "_CORBA_Unbounded_Sequence_Array_w_FixSizeElement"
#define SEQUENCE_TEMPLATE_BOUNDED_ARRAY_W_FIXSIZEELEMENT "_CORBA_Bounded_Sequence_Array_w_FixSizeElement"
#define SEQUENCE_TEMPLATE_ADPT_CLASS "_CORBA_Sequence_OUT_arg"

static size_t astExpr2val(AST_Expression *v);

o2be_sequence::o2be_sequence(AST_Expression *v, AST_Type *t)
	   : AST_Sequence(v, t),
    	     AST_Decl(AST_Decl::NT_sequence,
		      new UTL_ScopedName(
				new Identifier("sequence", 1, 0, I_FALSE),
				NULL),
		      NULL),
	     o2be_name(AST_Decl::NT_sequence,
		      new UTL_ScopedName(
				new Identifier("sequence", 1, 0, I_FALSE),
				NULL),
		      NULL),
	     o2be_sequence_chain(AST_Decl::NT_sequence,
		      new UTL_ScopedName(
				new Identifier("sequence", 1, 0, I_FALSE),
				NULL),
		      NULL)
{

  // we want a sequence to be defined in the same scope as its elements.
  //    i.e. sequence<a::b>   ==>   a::_IDL_SEQUENCE_a_b
  // unless its elements are pre-defined types, in which case
  // the sequence should be defined in the global scope
  //    e.g. sequence <long>  ==>   _IDL_SEQUENCE_long

  // Sequences are always entered into idl_global->root()
  // Determine the correct scope by looking at its elements.
  AST_Decl *d = base_type();
  while (d->node_type() == AST_Decl::NT_sequence)
    {
      o2be_sequence *s = o2be_sequence::narrow_from_decl(d);
      d = s->base_type();
    }
  
  char *p,*q;
  // Don't look for the scope name if the type is pre-defined
  if (d->node_type() != AST_Decl::NT_pre_defined)
    {
      p = o2be_name::narrow_and_produce_scopename(d);
      q = new char [strlen(p)+1];
      strcpy(q,p);
      set_scopename(p);
      p = o2be_name::narrow_and_produce__scopename(d);
      q = new char [strlen(p)+1];
      strcpy(q,p);
      set__scopename(p);
    }
  else
    {
      set_scopename("");
      set__scopename("");
    }
  set_uqname(internal_produce_localname(o2be_sequence::IMMEDIATE_TYPE));
  pd_effname = internal_produce_localname(o2be_sequence::EFFECTIVE_TYPE);
  p = new char [strlen(scopename())+strlen(uqname())+1];
  strcpy(p,scopename());
  strcat(p,uqname());
  set_fqname(p);
  p = new char [strlen(_scopename())+strlen(uqname())+1];
  strcpy(p,_scopename());
  strcat(p,uqname());
  set__fqname(p);
}


char*
o2be_sequence::seq_template_name(AST_Decl* used_in)
{
  char* result;
  o2be_operation::argMapping mapping;
  o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(base_type(),
					        o2be_operation::wIN,mapping);
  size_t s_max = astExpr2val(max_size()); // non-zero means 
                                          // this is a bounded seq
  size_t dimension = 0;       // non-zero means this is a sequence of array
  size_t elmsize = 0;         // non-zero means this is a primitive type seq
  size_t alignment = 0;
  const char* baseclassname = seq_member_name(used_in);
  const char* elmclassname = 0; // non-zero if this is a sequence of array
  switch (ntype) 
    {
    case o2be_operation::tBoolean:
    case o2be_operation::tChar:
    case o2be_operation::tOctet:
      elmsize = 1;
      alignment = 1;
      break;
    case o2be_operation::tShort:
    case o2be_operation::tUShort:
      elmsize = 2;
      alignment = 2;
      break;
    case o2be_operation::tLong:
    case o2be_operation::tULong:
    case o2be_operation::tEnum:
      elmsize = 4;
      alignment = 4;
      break;
    case o2be_operation::tFloat:
      elmsize = 4;
      alignment = 4;
      break;
    case o2be_operation::tDouble:
      elmsize = 8;
      alignment = 8;
      break;
    case o2be_operation::tArrayFixed:
      {
	AST_Decl *decl = base_type();
	while (decl->node_type() == AST_Decl::NT_typedef)
	  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	dimension = o2be_array::narrow_from_decl(decl)->getNumOfElements();
	// look at the type of the elements in the array and see
	// if it is a primitive type
	decl = o2be_array::narrow_from_decl(decl)->getElementType();
	elmclassname = o2be_name::narrow_and_produce_unambiguous_name(decl,
								      used_in);
	ntype = o2be_operation::ast2ArgMapping(decl,
					       o2be_operation::wIN,mapping);
	switch (ntype)
	  {
	  case o2be_operation::tBoolean:
	  case o2be_operation::tChar:
	  case o2be_operation::tOctet:
	    elmsize = 1;
	    alignment = 1;
	    break;
	  case o2be_operation::tShort:
	  case o2be_operation::tUShort:
	    elmsize = 2;
	    alignment = 2;
	    break;
	  case o2be_operation::tLong:
	  case o2be_operation::tULong:
	  case o2be_operation::tEnum:
	    elmsize = 4;
	    alignment = 4;
	    break;
	  case o2be_operation::tFloat:
	    elmsize = 4;
	    alignment = 4;
	    break;
	  case o2be_operation::tDouble:
	    elmsize = 8;
	    alignment = 8;
	    break;
	  default:
	    break;
	  }
	break;
      }
    case o2be_operation::tArrayVariable:
      {
	AST_Decl *decl = base_type();
	while (decl->node_type() == AST_Decl::NT_typedef)
	  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	dimension = o2be_array::narrow_from_decl(decl)->getNumOfElements();
	decl = o2be_array::narrow_from_decl(decl)->getElementType();
	// look at the type of the elements in the array
	ntype = o2be_operation::ast2ArgMapping(decl,
					       o2be_operation::wIN,mapping);
	switch (ntype)
	  {
	  case o2be_operation::tObjref:
	    {
	      while (decl->node_type() == AST_Decl::NT_typedef)
		decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	      elmclassname = o2be_interface::narrow_from_decl(decl)->fieldMemberType_fqname(used_in);
	    }
	    break;
	  case o2be_operation::tString:
	    {
	      elmclassname = o2be_string::fieldMemberTypeName();
	    }
	    break;
	  case o2be_operation::tSequence:
	    {
	      elmclassname = o2be_sequence::narrow_from_decl(decl)->seq_template_name(used_in);
	    }
	    break;
	  default:
	    elmclassname = o2be_name::narrow_and_produce_unambiguous_name(decl,
								 used_in);
	  }
	break;
      }
    case o2be_operation::tAny:
    default:
    break;
  }

  if (s_max)
    {
      // bounded sequence
      if (!dimension) {
	if (elmsize) {
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT)
	                    + strlen(baseclassname) + 20;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%s,%d,%d,%d>",
		  SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT,
		  baseclassname,
		  (int)s_max,
		  (int)elmsize,
		  (int)alignment);
	}
	else {
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_BOUNDED)
   	                    + strlen(baseclassname) + 13;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%s,%d>",
		  SEQUENCE_TEMPLATE_BOUNDED,
		  baseclassname,
		  (int)s_max);
	}      
      }
      else {
	// bounded sequence of array
	if (elmsize) {
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_BOUNDED_ARRAY_W_FIXSIZEELEMENT)
	                    +strlen(baseclassname)*2+strlen(elmclassname)+39;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%s,%s_slice,%s,%d,%d,%d,%d>",
		  SEQUENCE_TEMPLATE_BOUNDED_ARRAY_W_FIXSIZEELEMENT,
		  baseclassname,
		  baseclassname,
		  elmclassname,
		  (int)dimension,
		  (int)s_max,
		  (int)elmsize,
		  (int)alignment);
	}
	else {
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_BOUNDED_ARRAY)
   	                    +strlen(baseclassname)*2+strlen(elmclassname)+32;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%s,%s_slice,%s,%d,%d>",
		  SEQUENCE_TEMPLATE_BOUNDED_ARRAY,
		  baseclassname,
		  baseclassname,
		  elmclassname,
		  (int)dimension,
		  (int)s_max);
	}      
      }
    }
  else
    {
      // unbounded sequence
      if (!dimension) {
	if (elmsize) {
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT)
	                    + strlen(baseclassname) + 7;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%s,%d,%d>",
		  SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT,
		  baseclassname,
		  (int)elmsize,
		  (int)alignment);
	}
	else {
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_UNBOUNDED)
	                    + strlen(baseclassname) + 4;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%s >",
		  SEQUENCE_TEMPLATE_UNBOUNDED,
		  baseclassname);
	}
      }
      else {
	// unbounded sequence of array
	if (elmsize) {
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY_W_FIXSIZEELEMENT)
	                    + strlen(baseclassname)*2+strlen(elmclassname)+27;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%s,%s_slice,%s,%d,%d,%d>",
		  SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY_W_FIXSIZEELEMENT,
		  baseclassname,
		  baseclassname,
		  elmclassname,
		  (int)dimension,
		  (int)elmsize,
		  (int)alignment);
	}
	else {
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY)
	                    + strlen(baseclassname)*2+strlen(elmclassname)+24;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%s,%s_slice,%s,%d>",
		  SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY,
		  baseclassname,
		  baseclassname,
		  elmclassname,
		  (int)dimension);
	}
      }
    }
  return result;
}

const char*
o2be_sequence::seq_member_name(AST_Decl* used_in)
{
  const char* baseclassname;
  if (o2be_global::qflag()) {
    baseclassname = o2be_name::narrow_and_produce_fqname(base_type());
  }
  else {
    baseclassname = o2be_name::narrow_and_produce_unambiguous_name(base_type(),
								   used_in);
  }
  o2be_operation::argMapping mapping;
  o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(base_type(),
					        o2be_operation::wIN,mapping);
  switch (ntype) 
    {
    case o2be_operation::tObjref:
      {
	AST_Decl *decl = base_type();
	while (decl->node_type() == AST_Decl::NT_typedef)
	  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	baseclassname = o2be_interface::narrow_from_decl(decl)->fieldMemberType_fqname(used_in);
	break;
      }
    case o2be_operation::tString:
      {
	baseclassname = o2be_string::fieldMemberTypeName();
	break;
      }
    case o2be_operation::tSequence:
      {
	AST_Decl *decl = base_type();
	while (decl->node_type() == AST_Decl::NT_typedef)
	  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	baseclassname = o2be_sequence::narrow_from_decl(decl)->seq_template_name(used_in);
	break;
      }
    default:
    break;
  }
  return baseclassname;
}

void
o2be_sequence::produce_hdr(fstream &s)
{
#ifdef USE_SEQUENCE_TEMPLATE_IN_PLACE
  return;
#else
  s << "#ifndef " << _fqname() << "_defined\n"
    << "#define " << _fqname() << "_defined\n\n";
  o2be_operation::argMapping mapping;
  o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(base_type(),
					        o2be_operation::wIN,mapping);

  size_t s_max = astExpr2val(max_size());
  switch (ntype)
    {
    case o2be_operation::tShort:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << s_max << ","
		    << "2,2> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << "2,2> ";
	}
	break;
      }
    case o2be_operation::tLong:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << s_max << ","
		    << "4,4> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << "4,4> ";
	}
	break;
      }
    case o2be_operation::tUShort:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << s_max << ","
		    << "2,2> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << "2,2> ";
	}
	break;
      }
    case o2be_operation::tULong:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << s_max << ","
		    << "4,4> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << "4,4> ";
	}
	break;
      }
    case o2be_operation::tFloat:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << s_max << ","
		    << "4,4> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << "4,4> ";
	}
	break;
      }
    case o2be_operation::tDouble:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << s_max << ","
		    << "8,8> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << "8,8> ";
	}
	break;
      }
    case o2be_operation::tBoolean:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << s_max << ","
		    << "1,1> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << "1,1> ";
	}
	break;
      }
    case o2be_operation::tChar:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << s_max << ","
		    << "1,1> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << "1,1> ";
	}
	break;
      }
    case o2be_operation::tOctet:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << s_max << ","
		    << "1,1> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << "1,1> ";
	}
	break;
      }
    case o2be_operation::tEnum:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << s_max << ","
		    << "4,4> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << "4,4> ";
	}
	break;
      }
    case o2be_operation::tObjref:
      {
	AST_Decl *decl = base_type();
	while (decl->node_type() == AST_Decl::NT_typedef)
	  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED << "<"
		    << o2be_interface::narrow_from_decl(decl)->fieldMemberType_fqname() << ","
		    << s_max << "> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED << "<"
		    << o2be_interface::narrow_from_decl(decl)->fieldMemberType_fqname() << " > ";
	}
	break;
      }
      break;
    case o2be_operation::tString:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED << "<"
		    << o2be_string::fieldMemberTypeName() << ","
		    << s_max << "> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED << "<"
		    << o2be_string::fieldMemberTypeName() << "> ";
	}
	break;
      }
    case o2be_operation::tAny:
    default:
      {
	if (s_max) {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_BOUNDED << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type()) << ","
		    << s_max << "> ";
	}
	else {
	  IND(s); s << "typedef " 
		    << SEQUENCE_TEMPLATE_UNBOUNDED << "<"
		    << o2be_name::narrow_and_produce_uqname(base_type())<<"> ";
	}
	break;
      }
    }
  s << uqname() << ";\n";

  IND(s); s << "typedef " << SEQUENCE_VAR_TEMPLATE << "<"
	    << uqname() << "> "  << uqname() << "_var;\n\n";
  s << "#endif\n";

  produce_seq_hdr_if_defined(s);
#endif
}

void
o2be_sequence::produce_skel(fstream &s)
{
  return;
}

void
o2be_sequence::produce_typedef_hdr(fstream &s, o2be_typedef *tdef)
{
#ifdef USE_SEQUENCE_TEMPLATE_IN_PLACE
  IND(s); s << "typedef " << seq_template_name(tdef) 
	    << " " << tdef->uqname() << ";\n";

  o2be_operation::argMapping mapping;
  switch (o2be_operation::ast2ArgMapping(base_type(),o2be_operation::wIN,
					 mapping))
    {
    case o2be_operation::tArrayFixed:
    case o2be_operation::tArrayVariable:
      IND(s); s << "typedef _CORBA_Sequence_Array_Var<"
		<< tdef->uqname() << ", " << seq_member_name(tdef) <<  "_slice > " 
		<< tdef->uqname() << "_var;\n\n";
      break;
    default:
      IND(s); s << "typedef _CORBA_Sequence_Var<"
		<< tdef->uqname() << ", " << seq_member_name(tdef) <<  " > " 
		<< tdef->uqname() << "_var;\n\n";
      break;
    }
#else
  IND(s); s << "typedef " << fqname() << " " << tdef->uqname() << ";\n";
  IND(s); s << "typedef " << fqname() << "_var " << tdef->uqname() << "_var;\n\n";
#endif
}

const char*
o2be_sequence::out_adptarg_name(o2be_typedef* tdef,AST_Decl* used_in) const
{
  const char* ubname;

  if (o2be_global::qflag()) {
    ubname = tdef->fqname();
  }
  else {
    ubname = tdef->unambiguous_name(used_in);
  }

  char* p = new char[strlen(SEQUENCE_TEMPLATE_ADPT_CLASS)+strlen("<, >")+
		     strlen(ubname)*2+strlen("_var")+1];
  strcpy(p,SEQUENCE_TEMPLATE_ADPT_CLASS);
  strcat(p,"<");
  strcat(p,ubname);
  strcat(p,",");
  strcat(p,ubname);
  strcat(p,"_var >");
  return p;
}


char *
o2be_sequence::internal_produce_seqname(AST_Decl *d,enum seqnametype stype)
{
  char *iname;
  switch(d->node_type())
    {
    case AST_Decl::NT_sequence:
      {
	o2be_sequence *s = o2be_sequence::narrow_from_decl(d);
	char *iiname = o2be_sequence::internal_produce_seqname(s->base_type(),
							      stype);
	iname = new char [strlen("sequence_")+strlen(iiname)+1];
	strcpy(iname,"sequence_");
	strcat(iname,iiname);
	delete [] iiname;
      }
      break;
    case AST_Decl::NT_pre_defined:
      {
	// pre-defined types are special because their uqname() actually
	// returns a scoped name, e.g. ULong --> CORBA::ULong.
	// Therefore, we use _fqname() instead.
	char *iiname = o2be_name::narrow_and_produce__fqname(d);
	iname = new char [strlen(iiname)+1];
	strcpy(iname,iiname);
	char *p = iname;
	while (*p != '\0')
	  {
	    if (*p == ' ')
	      *p = '_';
	    p++;
	  };
      }
      break;
    case AST_Decl::NT_string:
      {
	iname = new char [strlen("string") + 1];
	strcpy(iname,"string");
      }
      break;
    case AST_Decl::NT_typedef:
      {
	o2be_typedef *s = o2be_typedef::narrow_from_decl(d);
	if (stype == o2be_sequence::EFFECTIVE_TYPE)
	  {
	    AST_Decl *efftype = s->base_type();
	    if (efftype->node_type() != AST_Decl::NT_array)
	      {
		iname = o2be_sequence::internal_produce_seqname(efftype,stype);
		break;
	      }
	  }
	iname = new char [strlen(s->_fqname()) + 1];
	strcpy(iname,s->_fqname());
      }
      break;
    case AST_Decl::NT_enum:
      {
	o2be_enum *s = o2be_enum::narrow_from_decl(d);
	iname = new char [strlen(s->_fqname()) + 1];
	strcpy(iname,s->_fqname());
      }
      break;
    case AST_Decl::NT_struct:
      {
	o2be_structure *s = o2be_structure::narrow_from_decl(d);
	iname = new char [strlen(s->_fqname()) + 1];
	strcpy(iname,s->_fqname());

      }
      break;
    case AST_Decl::NT_union:
      {
	o2be_union *s = o2be_union::narrow_from_decl(d);
	iname = new char [strlen(s->_fqname()) + 1];
	strcpy(iname,s->_fqname());
      }
      break;
    case AST_Decl::NT_interface:
      {
	o2be_interface *s = o2be_interface::narrow_from_decl(d);
	iname = new char [strlen(s->_fqname()) + 1];
	strcpy(iname,s->_fqname());
      }
      break;
    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unexpected argument type");
      iname = 0; // Just to suppress the compiler's warning.
      break;
    }
  return iname;
}

char *
o2be_sequence::internal_produce_localname(enum seqnametype stype)
{
  char *iname = o2be_sequence::internal_produce_seqname(base_type(),stype);
  char *fullname = new char [strlen(SEQUENCE_TYPE_PREFIX)+strlen(iname)+1];
  strcpy(fullname,SEQUENCE_TYPE_PREFIX);
  strcat(fullname,iname);
  delete [] iname;
  return fullname;
}

#ifdef USE_NON_TEMPLATE_CODE
static void check_and_produce_effective_type_sequence(o2be_sequence *s);
#endif

// The front end can produce multiple nodes for each pre-defined type and
// string type. We just keep an internal pointer to one instance of each type.
// produce_hdr_for_predefined_types() will use these pointers to invoke
// produce_seq_hdr_if_defined() for that pre-defined type.

static int                   pd_sizeof_predefined_type = 0;
static o2be_predefined_type **pd_predefined_type = NULL;
static o2be_string *          pd_string = NULL;

static
void
set_pd_predefined_type(o2be_predefined_type *s)
{
  if (pd_sizeof_predefined_type <= s->pt())
    {
      o2be_predefined_type **p = new (o2be_predefined_type* [s->pt()+1]);
      int j;
      for (j=0; j<pd_sizeof_predefined_type; j++)
	p[j] = pd_predefined_type[j];
      for (; j<= s->pt(); j++)
	p[j] = NULL;
      pd_sizeof_predefined_type = s->pt()+1;
      if (pd_predefined_type) delete pd_predefined_type;
      pd_predefined_type = p;
    }
  if (pd_predefined_type[s->pt()] == NULL)
    {
      pd_predefined_type[s->pt()] = s;
    }
  return;
}

static
o2be_predefined_type *
get_pd_predefined_type(AST_PredefinedType::PredefinedType type)
{
  if (pd_sizeof_predefined_type <= type)
    return NULL;
  else
    return pd_predefined_type[type];
}

static
void
set_pd_string(o2be_string *s)
{
  if (pd_string == NULL)
    {
      pd_string = s;
    }
}

static
o2be_string *
get_pd_string()
{
  return pd_string;
}

void
o2be_sequence::produce_hdr_for_predefined_types(fstream &s)
{
  int j;
  for (j=0; j<pd_sizeof_predefined_type; j++)
    {
      if (pd_predefined_type[j] != NULL)
	{
	  pd_predefined_type[j]->produce_seq_hdr_if_defined(s);
	}
    }
  if (pd_string)
    {
      pd_string->produce_seq_hdr_if_defined(s);
    }
  return;
}


AST_Sequence *
o2be_sequence::attach_seq_to_base_type(AST_Sequence *se)
{
  AST_Decl *d = se->base_type();
  o2be_sequence *os = o2be_sequence::narrow_from_decl(se);
  switch(d->node_type())
    {
    case AST_Decl::NT_pre_defined:
      {
	// The front end can produce multiple nodes for each pre-defined
	// type. We just keep an internal pointer to one instance of these.
        // produce_hdr_for_predefined_types() will use this pointer to
	// invoke produce_seq_hdr_if_defined() for that pre-defined type.

	o2be_predefined_type *s = o2be_predefined_type::narrow_from_decl(d);
	set_pd_predefined_type(s);
	get_pd_predefined_type(s->pt())->set_seq_decl(os);
      }
      break;
    case AST_Decl::NT_string:
      {
	// The front end can produce multiple nodes of this type.
	// We just keep an internal pointer to one instance of these.
        // produce_hdr_for_predefined_types() will use this pointer to
	// invoke produce_seq_hdr_if_defined() for that pre-defined type.

	o2be_string *s = o2be_string::narrow_from_decl(d);
	set_pd_string(s);
	get_pd_string()->set_seq_decl(os);
      }
      break;
    case AST_Decl::NT_enum:
      o2be_enum::narrow_from_decl(d)->set_seq_decl(os);
      break;
    case AST_Decl::NT_struct:
      o2be_structure::narrow_from_decl(d)->set_seq_decl(os);
      break;
    case AST_Decl::NT_union:
      o2be_union::narrow_from_decl(d)->set_seq_decl(os);
      break;
    case AST_Decl::NT_interface:
      o2be_interface::narrow_from_decl(d)->set_seq_decl(os);
      break;
    case AST_Decl::NT_sequence:
      // The front end can produce multiple nodes of this type with the
      // same base type. set_seq_decl() will sort this out.

      o2be_sequence::narrow_from_decl(d)->set_seq_decl(os);
#ifdef USE_NON_TEMPLATE_CODE
      // If the effective type and the immediate type differs, i.e. the
      // base type is a typedef, we need to create sequence nodes for
      // the effective type as well
      check_and_produce_effective_type_sequence(os);
#endif
      break;
    case AST_Decl::NT_typedef:
      o2be_typedef::narrow_from_decl(d)->set_seq_decl(os);
#ifdef USE_NON_TEMPLATE_CODE
      // effective type and the immediate type differs, we may need to create
      // a sequence node for the effective type if it doesn't exist already.
      check_and_produce_effective_type_sequence(os);
#endif
      break;
    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unexpected argument type");
      break;
    }
  return se;
}

#ifdef USE_NON_TEMPLATE_CODE
static
void
check_and_produce_effective_type_sequence(o2be_sequence *s)
{
  // Get to the base type and determine how many levels of nesting
  int level = 0;
  AST_Decl *decl = s->base_type();
  while (decl->node_type() == AST_Decl::NT_sequence)
    {
      level++;
      decl = o2be_sequence::narrow_from_decl(decl)->base_type();
    }
  
  // If the base type is a typedef, get the effective type and create
  // sequence node for the effective type to the same level of nesting
  // as the base type.
  if (decl->node_type() == AST_Decl::NT_typedef)
    {
      o2be_sequence *sq,*eq;

      sq = o2be_typedef::narrow_from_decl(decl)->get_seq_decl();
      decl = o2be_typedef::narrow_from_decl(decl)->base_type();
      switch (decl->node_type())
	{
	case AST_Decl::NT_pre_defined:
	  {
	    set_pd_predefined_type(o2be_predefined_type::narrow_from_decl(decl));
	    o2be_predefined_type *p;
	    p = get_pd_predefined_type(o2be_predefined_type::narrow_from_decl(decl)->pt());
	    if (p->get_seq_decl() == NULL)
	      {
		AST_Sequence *newsq = idl_global->gen()
		  ->create_sequence(sq->max_size(),p);
		(void) o2be_root::narrow_from_decl(idl_global->root())
		  ->add_sequence(newsq);
	      }
	    eq = p->get_seq_decl();
	  }
	  break;
	case AST_Decl::NT_string:
	  {
	    set_pd_string(o2be_string::narrow_from_decl(decl));
	    o2be_string *p = get_pd_string();
	    if (p->get_seq_decl() == NULL)
	      {
		AST_Sequence *newsq = idl_global->gen()
		  ->create_sequence(sq->max_size(),p);
		(void) o2be_root::narrow_from_decl(idl_global->root())
		  ->add_sequence(newsq);
	      }
	    eq = p->get_seq_decl();
	  }
	  break;
	case AST_Decl::NT_enum:
	  {
	    o2be_enum *p = o2be_enum::narrow_from_decl(decl);
	    if (p->get_seq_decl() == NULL)
	      {
		AST_Sequence *newsq = idl_global->gen()
		  ->create_sequence(sq->max_size(),p);
		(void) o2be_root::narrow_from_decl(idl_global->root())
		  ->add_sequence(newsq);
	      }
	    eq = p->get_seq_decl();
	  }
	  break;
	case AST_Decl::NT_struct:
	  {
	    o2be_structure *p = o2be_structure::narrow_from_decl(decl);
	    if (p->get_seq_decl() == NULL)
	      {
		AST_Sequence *newsq = idl_global->gen()
		  ->create_sequence(sq->max_size(),p);
		(void) o2be_root::narrow_from_decl(idl_global->root())
		  ->add_sequence(newsq);
	      }
	    eq = p->get_seq_decl();
	  }
	  break;
	case AST_Decl::NT_union:
	  {
	    o2be_union *p = o2be_union::narrow_from_decl(decl);
	    if (p->get_seq_decl() == NULL)
	      {
		AST_Sequence *newsq = idl_global->gen()
		  ->create_sequence(sq->max_size(),p);
		(void) o2be_root::narrow_from_decl(idl_global->root())
		  ->add_sequence(newsq);
	      }
	    eq = p->get_seq_decl();
	  }
	  break;
	case AST_Decl::NT_interface:
	  {
	    o2be_interface *p = o2be_interface::narrow_from_decl(decl);
	    if (p->get_seq_decl() == NULL)
	      {
		AST_Sequence *newsq = idl_global->gen()
		  ->create_sequence(sq->max_size(),p);
		(void) o2be_root::narrow_from_decl(idl_global->root())
		  ->add_sequence(newsq);
	      }
	    eq = p->get_seq_decl();
	  }
	  break;
	case AST_Decl::NT_sequence:
	  {
	    o2be_sequence *p = o2be_sequence::narrow_from_decl(decl);
	    if (p->get_seq_decl() == NULL)
	      {
		AST_Sequence *newsq = idl_global->gen()
		  ->create_sequence(sq->max_size(),p);
		(void) o2be_root::narrow_from_decl(idl_global->root())
		  ->add_sequence(newsq);
	      }
	    eq = p->get_seq_decl();
	  }
	  break;
	case AST_Decl::NT_typedef:
	  {
	    o2be_typedef *p = o2be_typedef::narrow_from_decl(decl);
	    if (p->get_seq_decl() == NULL)
	      {
		AST_Sequence *newsq = idl_global->gen()
		  ->create_sequence(sq->max_size(),p);
		(void) o2be_root::narrow_from_decl(idl_global->root())
		  ->add_sequence(newsq);
	      }
	    eq = p->get_seq_decl();
	  }
	  break;
	case AST_Decl::NT_array:
	  return;      // nothing to do
	default:
	  throw o2be_internal_error(__FILE__,__LINE__,"Unexpected argument type");
	  return;
	}
      while (level-- > 0)
	{
	  sq = sq->get_seq_decl();
	  if (eq->get_seq_decl() == NULL)
	    {
	      AST_Sequence *newsq = idl_global->gen()
		->create_sequence(sq->max_size(),eq);
	      (void) o2be_root::narrow_from_decl(idl_global->root())
		->add_sequence(newsq);
	    }
	  eq = eq->get_seq_decl();
	}
    }
  return;
}
#endif


void
o2be_sequence_chain::set_seq_decl(o2be_sequence *d)
{
  // pd_seq_decl chains together obe_sequence nodes that represents 
  //      sequence<base_type>,
  //      sequence<sequence<base_type>>
  //      sequence<sequence<sequence<base_type>>> and so on.
  // The chain starts from the base type node.
  // A call to the member function produce_seq_hdr_if_defined() of the
  // base type node would produce declarations for the sequence types.
  // 
  // Unfortunately, the front end can produce multiple o2be_sequence nodes
  // that represent the same sequence constructs, e.g multiple nodes for
  // sequence<base_type>. This function filters out the duplicates and maintains
  // a single chain from the base type node. This chain must represent the
  // deepest level of nesting.
  //

  if (pd_seq_decl == NULL)
    {
      pd_seq_decl = d;
      if (node_type() == AST_Decl::NT_sequence)
	{
	  AST_Decl *decl = AST_Sequence::narrow_from_decl(this)->base_type();
	  o2be_sequence *ns = o2be_sequence::narrow_from_decl(this);
	  switch (decl->node_type())
	    {
	    case AST_Decl::NT_pre_defined:
	      o2be_predefined_type::narrow_from_decl(decl)->set_seq_decl(ns);
	      break;
	    case AST_Decl::NT_string:
	      o2be_string::narrow_from_decl(decl)->set_seq_decl(ns);
	      break;
	    case AST_Decl::NT_enum:
	      o2be_enum::narrow_from_decl(decl)->set_seq_decl(ns);
	      break;
	    case AST_Decl::NT_struct:
	      o2be_structure::narrow_from_decl(decl)->set_seq_decl(ns);
	      break;
	    case AST_Decl::NT_union:
	      o2be_union::narrow_from_decl(decl)->set_seq_decl(ns);
	      break;
	    case AST_Decl::NT_interface:
	      o2be_interface::narrow_from_decl(decl)->set_seq_decl(ns);
	      break;
	    case AST_Decl::NT_sequence:
	      {
		o2be_sequence_chain *p = o2be_sequence::narrow_from_decl(decl);
		p->pd_seq_decl = NULL;
	      // pd_seq_decl will be reset to its original value by the recursive
	      // call below. Setting it to NULL now would cause the recursion
	      // to terminate only when it reaches the base type.
	      }
	      o2be_sequence::narrow_from_decl(decl)->set_seq_decl(ns);
	      break;
	    case AST_Decl::NT_typedef:
	      o2be_typedef::narrow_from_decl(decl)->set_seq_decl(ns);
	      break;
	    default:
	      throw o2be_internal_error(__FILE__,__LINE__,"Unexpected argument type");
	      break;
	    }
	}
    }
  else {
    // Now we have a new sequence chain and an existing sequence chain.
    // Merge the two together to make the longest chain.
    // At each level of the chain, pick the o2be_sequence node with
    // its member function in_main_file() returns FALSE over the one that
    // returns TRUE.
    // Stub code would only be generated for those o2be_sequence 
    // nodes with in_main_file() returns TRUE. This would minimize the
    // generation of sequence typedefs defined in other idl files in
    // a subsequent call to produce_seq_hdr_if_defined()
    //
    int newlvl = 0;
    o2be_sequence_chain *dd = d; 
    while (dd != NULL)
      {
	newlvl++;
	dd = dd->pd_seq_decl;
      }
    int oldlvl = 0;
    dd = pd_seq_decl;
    while (dd != NULL)
      {
	oldlvl++;
	dd = dd->pd_seq_decl;
      }
    o2be_sequence_chain *ddd;
    if (newlvl > oldlvl) {
      dd = d;
      ddd = pd_seq_decl;
      pd_seq_decl = d;
    }
    else {
      dd = pd_seq_decl;
      ddd = d;
    }
    while (ddd) {
      if (!(ddd->in_main_file())) {
	dd->set_in_main_file(I_FALSE);
      }
      ddd = ddd->pd_seq_decl;
      dd = dd->pd_seq_decl;
    }
  }
  return;
}

void
o2be_sequence_chain::produce_seq_hdr_if_defined(fstream &s)
{
#ifdef USE_SEQUENCE_TEMPLATE_IN_PLACE
  return;
#else
  if (pd_seq_decl)
    {
      s << "\n";
      pd_seq_decl->produce_hdr(s);
    }
  return;
#endif
}

IMPL_NARROW_METHODS1(o2be_sequence, AST_Sequence)
IMPL_NARROW_FROM_DECL(o2be_sequence)
IMPL_NARROW_FROM_SCOPE(o2be_sequence)

static 
size_t
astExpr2val(AST_Expression *e)
{
  AST_Expression::AST_ExprValue *v = e->ev();
  switch (v->et) 
    {
    case AST_Expression::EV_short:
      return (size_t)v->u.sval;
    case AST_Expression::EV_ushort:
      return (size_t)v->u.usval;
    case AST_Expression::EV_long:
      return (size_t)v->u.lval;
    case AST_Expression::EV_ulong:
      return (size_t)v->u.ulval;
    default:
      throw o2be_internal_error(__FILE__,__LINE__,"unexpected type for array dimension");
    }
  return 0; // For MSVC++ 4.2
}


