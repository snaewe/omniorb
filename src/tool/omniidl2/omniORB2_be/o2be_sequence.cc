// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_sequence.cc         Created on: 12/08/1996
//			    Author    : Sai-Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
  Revision 1.23  1999/06/18 20:45:35  sll
  Updated to support CORBA 2.3 mapping.

  Revision 1.22  1999/06/03 17:10:58  sll
  Updated to CORBA 2.2

  Revision 1.21  1999/05/26 15:57:00  sll
  sequence of object now uses a separate template.

  Revision 1.20  1999/05/26 10:40:51  sll
  Each sequence is now generated as a separate type.

  Revision 1.19  1999/05/20 15:24:26  sll
  Explicitly cast a StringBuf to const char* when passed to stream <<
  operator. MSVC++ is confused otherwise.

  Revision 1.18  1999/04/21 13:04:54  djr
  sequence<string> now uses a pre-defined type.

  Revision 1.17  1999/03/11 16:26:09  djr
  Updated copyright notice

  Revision 1.16  1999/01/07 09:33:46  djr
  Changes to support new TypeCode/Any implementation, which is now
  placed in a new file ...DynSK.cc (by default).

  Revision 1.15  1998/08/19 19:12:09  sll
  Catch the illegal IDL: a sequence of exception, in the ctor. The frontend
  grammer let this go through. Should really fix the frontend.

  Revision 1.14  1998/08/19 15:53:57  sll
  New member functions void produce_binary_operators_in_hdr and the like
  are responsible for generating binary operators <<= etc in the global
  namespace.

  Revision 1.13  1998/08/13 22:44:56  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.12  1998/04/07 18:51:13  sll
  Use std::fstream instead of fstream.
  Stub code modified to accommodate the use of namespace to represent module.

// Revision 1.11  1998/02/19  12:13:56  ewc
// Fix to recursive unions.
//
// Revision 1.10  1998/01/27  16:48:28  ewc
// Added support for type Any and TypeCode
//
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

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <o2be_stringbuf.h>
#include <stdio.h>


#define SEQUENCE_TEMPLATE_UNBOUNDED \
  "_CORBA_Unbounded_Sequence"

#define SEQUENCE_TEMPLATE_BOUNDED \
  "_CORBA_Bounded_Sequence"

#define SEQUENCE_TEMPLATE_UNBOUNDED_W_FIXSIZEELEMENT \
  "_CORBA_Unbounded_Sequence_w_FixSizeElement"

#define SEQUENCE_TEMPLATE_UNBOUNDED__BOOLEAN \
  "_CORBA_Unbounded_Sequence__Boolean"

#define SEQUENCE_TEMPLATE_UNBOUNDED__OCTET \
  "_CORBA_Unbounded_Sequence__Octet"

#define SEQUENCE_TEMPLATE_UNBOUNDED__STRING \
  "_CORBA_Unbounded_Sequence__String"

#define SEQUENCE_TEMPLATE_BOUNDED_W_FIXSIZEELEMENT \
  "_CORBA_Bounded_Sequence_w_FixSizeElement"

#define SEQUENCE_TEMPLATE_BOUNDED__BOOLEAN \
  "_CORBA_Bounded_Sequence__Boolean"

#define SEQUENCE_TEMPLATE_BOUNDED__OCTET \
  "_CORBA_Bounded_Sequence__Octet"

#define SEQUENCE_TEMPLATE_BOUNDED__STRING \
  "_CORBA_Bounded_Sequence__String"

#define SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY \
  "_CORBA_Unbounded_Sequence_Array"

#define SEQUENCE_TEMPLATE_BOUNDED_ARRAY \
  "_CORBA_Bounded_Sequence_Array"

#define SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY_W_FIXSIZEELEMENT \
  "_CORBA_Unbounded_Sequence_Array_w_FixSizeElement"

#define SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY__BOOLEAN \
  "_CORBA_Unbounded_Sequence_Array__Boolean"

#define SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY__OCTET \
  "_CORBA_Unbounded_Sequence_Array__Octet"

#define SEQUENCE_TEMPLATE_BOUNDED_ARRAY_W_FIXSIZEELEMENT \
  "_CORBA_Bounded_Sequence_Array_w_FixSizeElement"

#define SEQUENCE_TEMPLATE_BOUNDED_ARRAY__BOOLEAN \
  "_CORBA_Bounded_Sequence_Array__Boolean"

#define SEQUENCE_TEMPLATE_BOUNDED_ARRAY__OCTET \
  "_CORBA_Bounded_Sequence_Array__Octet"

#define SEQUENCE_TEMPLATE_UNBOUNDED_OBJREF \
  "_CORBA_Unbounded_Sequence_ObjRef"

#define SEQUENCE_TEMPLATE_BOUNDED_OBJREF \
  "_CORBA_Bounded_Sequence_ObjRef"

#define SEQUENCE_TEMPLATE_ADPT_CLASS \
  "_CORBA_Sequence_OUT_arg"

#define UB_SEQUENCE_STRING_VAR \
  "_CORBA_Unbounded_Sequence__String_var"

#define UB_SEQUENCE_STRING_OUT \
  "_CORBA_Unbounded_Sequence__String_out"

#define B_SEQUENCE_STRING_VAR \
  "_CORBA_Bounded_Sequence__String_var"

#define B_SEQUENCE_STRING_OUT \
  "_CORBA_Bounded_Sequence__String_out"


o2be_sequence::o2be_sequence(AST_Expression* v, AST_Type* t)
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
  pd_have_produced_tcParser_buildDesc_code = I_FALSE;
  pd_have_calc_rec_seq_offset = I_FALSE;
  pd_out_adptarg_name = 0;
}


char*
o2be_sequence::seq_template_name(AST_Decl* used_in)
{
  char* result;
  o2be_operation::argMapping mapping;
  o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(base_type(),
					        o2be_operation::wIN,mapping);
  size_t s_max = bound(); // non-zero means
                          // this is a bounded seq
  size_t dimension = 0;       // non-zero means this is a sequence of array
  size_t elmsize = 0;         // non-zero means this is a primitive type seq
  size_t alignment = 0;
  const char* baseclassname = seq_member_name(used_in);
  const char* elmclassname = 0; // non-zero if this is a sequence of array
  switch (ntype)
    {
    case o2be_operation::tBoolean:
	if (s_max) {
	  // bounded sequence
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_BOUNDED__BOOLEAN) + 13;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%d>",
		  SEQUENCE_TEMPLATE_BOUNDED__BOOLEAN,
		  (int)s_max);
	}
	else {
	  // unbounded sequence
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_UNBOUNDED__BOOLEAN) + 1;
	  result = new char[namesize];
	  strcpy(result,SEQUENCE_TEMPLATE_UNBOUNDED__BOOLEAN);
	}
	return result;
	break;

    case o2be_operation::tOctet:
	if (s_max) {
	  // bounded sequence
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_BOUNDED__OCTET) + 13;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%d>",
		  SEQUENCE_TEMPLATE_BOUNDED__OCTET,
		  (int)s_max);
	}
	else {
	  // unbounded sequence
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_UNBOUNDED__OCTET) + 1;
	  result = new char[namesize];
	  strcpy(result,SEQUENCE_TEMPLATE_UNBOUNDED__OCTET);
	}
	return result;
	break;

    case o2be_operation::tString:
	if (s_max) {
	  // bounded sequence
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_BOUNDED__STRING) + 13;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%d>",
		  SEQUENCE_TEMPLATE_BOUNDED__STRING,
		  (int) s_max);
	}
	else {
	  // unbounded sequence
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_UNBOUNDED__STRING) + 1;
	  result = new char[namesize];
	  strcpy(result,SEQUENCE_TEMPLATE_UNBOUNDED__STRING);
	}
	return result;
	break;

    case o2be_operation::tObjref:
      {
	char* iname;
	if (o2be_global::qflag()) {
	  iname = o2be_name::narrow_and_produce_fqname(base_type());
	}
	else {
	  iname = o2be_name::narrow_and_produce_unambiguous_name(base_type(),
								   used_in);
	}
	if (s_max) {
	  // bounded sequence
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_BOUNDED_OBJREF) + 
	                    strlen(iname)*2 + strlen(baseclassname) + 33;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%s,%s,%s_Helper,%d>",
		  SEQUENCE_TEMPLATE_BOUNDED_OBJREF,
		  iname,
		  baseclassname,
		  iname,
		  (int) s_max);
	}
	else {
	  // unbounded sequence
	  size_t namesize = strlen(SEQUENCE_TEMPLATE_UNBOUNDED_OBJREF) +
	                    strlen(iname)*2 + strlen(baseclassname) + 13;
	  result = new char[namesize];
	  sprintf(result,
		  "%s<%s,%s,%s_Helper >",
		  SEQUENCE_TEMPLATE_UNBOUNDED_OBJREF,
		  iname,
		  baseclassname,
		  iname);
	}
	return result;
	break;
      }
    case o2be_operation::tChar:
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
	AST_Decl* decl = base_type();
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
	    if (s_max) {
	      // bounded sequence of array
	      size_t namesize =
		strlen(SEQUENCE_TEMPLATE_BOUNDED_ARRAY__BOOLEAN)
		+strlen(baseclassname)*2+32;
	      result = new char[namesize];
	      sprintf(result,
		      "%s<%s,%s_slice,%d,%d>",
		      SEQUENCE_TEMPLATE_BOUNDED_ARRAY__BOOLEAN,
		      baseclassname,
		      baseclassname,
		      (int)dimension,
		      (int)s_max);
	    }
	    else {
	      size_t namesize =
		strlen(SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY__BOOLEAN)
		+ strlen(baseclassname)*2+24;
	      result = new char[namesize];
	      sprintf(result,
		      "%s<%s,%s_slice,%d>",
		      SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY__BOOLEAN,
		      baseclassname,
		      baseclassname,
		      (int)dimension);
	    }
	    return result;
	    break;

	  case o2be_operation::tOctet:
	    if (s_max) {
	      // bounded sequence of array
	      size_t namesize = strlen(SEQUENCE_TEMPLATE_BOUNDED_ARRAY__OCTET)
		+strlen(baseclassname)*2+32;
	      result = new char[namesize];
	      sprintf(result,
		      "%s<%s,%s_slice,%d,%d>",
		      SEQUENCE_TEMPLATE_BOUNDED_ARRAY__OCTET,
		      baseclassname,
		      baseclassname,
		      (int)dimension,
		      (int)s_max);
	    }
	    else {
	      size_t namesize = strlen(SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY__OCTET)
		+ strlen(baseclassname)*2+24;
	      result = new char[namesize];
	      sprintf(result,
		      "%s<%s,%s_slice,%d>",
		      SEQUENCE_TEMPLATE_UNBOUNDED_ARRAY__OCTET,
		      baseclassname,
		      baseclassname,
		      (int)dimension);
	    }
	    return result;
	    break;

	  case o2be_operation::tChar:
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
	AST_Decl* decl = base_type();
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
	  case o2be_operation::tTypeCode:
	    {
	      elmclassname = o2be_predefined_type::TypeCodeMemberName();
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
	AST_Decl* decl = base_type();
	while (decl->node_type() == AST_Decl::NT_typedef)
	  decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	baseclassname = o2be_interface::narrow_from_decl(decl)->seqMemberType_fqname(used_in);
	break;
      }
    case o2be_operation::tTypeCode:
      {
	baseclassname = o2be_predefined_type::TypeCodeMemberName();
	break;
      }
    case o2be_operation::tSequence:
      {
	AST_Decl* decl = base_type();
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

size_t
o2be_sequence::bound()
{
  AST_Expression::AST_ExprValue* v = max_size()->ev();

  switch( v->et ) {
  case AST_Expression::EV_short:
    return (size_t)v->u.sval;
  case AST_Expression::EV_ushort:
    return (size_t)v->u.usval;
  case AST_Expression::EV_long:
    return (size_t)v->u.lval;
  case AST_Expression::EV_ulong:
    return (size_t)v->u.ulval;
  default:
    throw o2be_internal_error(__FILE__,__LINE__,
			      "unexpected type for sequence bound");
  }
  return 0;
}


static size_t
calc_recursive_sequence_offset(AST_Decl* node, AST_Decl* base_type,
			       size_t offset = 0)
{
  if (node == base_type)
    return offset;
  if (node->defined_in() == NULL)
    return 0;
  return calc_recursive_sequence_offset(ScopeAsDecl(node->defined_in()),
					base_type, offset+1);
}

size_t
o2be_sequence::recursive_sequence_offset()
{
  if( !pd_have_calc_rec_seq_offset ) {
    pd_rec_seq_offset = calc_recursive_sequence_offset(this, base_type());
    pd_have_calc_rec_seq_offset = 1;
  }

  return pd_rec_seq_offset;
}


void
o2be_sequence::produce_hdr(std::fstream& s)
{
}


void
o2be_sequence::produce_skel(std::fstream& s)
{
}


void
o2be_sequence::produce_dynskel(std::fstream& s)
{
}


void
o2be_sequence::produce_binary_operators_in_hdr(std::fstream& s)
{
}


void
o2be_sequence::produce_binary_operators_in_dynskel(std::fstream& s)
{
}


void
o2be_sequence::produce_typecode_skel(std::fstream& s)
{
  // We are safe even if this is a recursive sequence, as there
  // is a guard in o2be_structure::produce_typecode_skel() (and
  // the like) to ensure the body is only executed once.
  o2be_name::narrow_and_produce_typecode_skel(base_type(), s);
}


void
o2be_sequence::produce_typecode_member(std::fstream& s)
{
  AST_Decl* base = base_type();
  size_t s_rec_offset = recursive_sequence_offset();

  if (s_rec_offset) {
    // Recursive sequence!
    s << "CORBA::TypeCode::PR_recursive_sequence_tc("
      << bound() << ", " << s_rec_offset << ")";
  } else {
    // Normal sequence!
    s << "CORBA::TypeCode::PR_sequence_tc(" << bound() << ", ";
    o2be_name::produce_typecode_member(base, s);
    s << ")";
  }
}


void
o2be_sequence::produce_typedef_hdr(std::fstream& s, o2be_typedef* tdef)
{
  {
    // gcc requires that the marshalling operators for the element
    // be declared before the sequence template is typedef'd.
    //  This is a problem for enums, as the marshalling operators
    // are not yet defined (and are not part of the type itself).

    AST_Decl* decl = base_type();
    while( decl->node_type() == AST_Decl::NT_typedef)
      decl = o2be_typedef::narrow_from_decl(decl)->base_type();

    if( decl->node_type() == AST_Decl::NT_enum ) {
      s << "// Need to declare <<= for elem type, as GCC expands templates"
	<< " early\n";
      s << "#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7\n";

      o2be_enum* e = o2be_enum::narrow_from_decl(decl);
      idl_bool in_root = tdef->defined_in() == idl_global->root();

      IND(s); s << (in_root ? "":"friend ")
		<< "inline void operator >>= (" << e->unambiguous_name(tdef)
		<< ", NetBufferedStream&);\n";

      IND(s); s << (in_root ? "":"friend ")
		<< "inline void operator <<= (" << e->unambiguous_name(tdef)
		<< "&, NetBufferedStream&);\n";

      IND(s); s << (in_root ? "":"friend ")
		<< "inline void operator >>= (" << e->unambiguous_name(tdef)
		<< ", MemBufferedStream&);\n";

      IND(s); s << (in_root ? "":"friend ")
		<< "inline void operator <<= (" << e->unambiguous_name(tdef)
		<< "&, MemBufferedStream&);\n";

      s << "#endif\n";
    }
  }

  // Compilers cannot distinguish identical template types. We need to
  // be able to distinguish between identical sequence types, so that
  // when we insert into an Any we get the correct repo Id. Thus we have
  // to have a separate C++ class for each typedef'ed sequence type.
  //  This class just needs to define the relevant constructors and
  // assignment operator.

  const char* template_name = seq_template_name(tdef);
  StringBuf t_star_constructor_arg, index_ret_type;
  int is_seq_of_array = 0;

  o2be_operation::argMapping mapping;
  o2be_operation::argType ntype =
    o2be_operation::ast2ArgMapping(base_type(), o2be_operation::wIN, mapping);

  switch( ntype ) {
  case o2be_operation::tString:
    t_star_constructor_arg += "char*";
    index_ret_type += "_CORBA_String_member";
    break;

  case o2be_operation::tObjref:
    t_star_constructor_arg +=
      o2be_name::narrow_and_produce_unambiguous_name(base_type(), tdef);
    t_star_constructor_arg += "_ptr";
    index_ret_type += seq_member_name(tdef);
    break;

  case o2be_operation::tArrayFixed:
  case o2be_operation::tArrayVariable:
    t_star_constructor_arg += seq_member_name(tdef);
    index_ret_type += seq_member_name(tdef);
    index_ret_type += "_slice*";
    is_seq_of_array = 1;
    break;

  default:
    t_star_constructor_arg += seq_member_name(tdef);
    index_ret_type += seq_member_name(tdef);
    index_ret_type += "&";
    break;
  }

  IND(s); s << "class " << tdef->uqname() << "_var;\n\n";

  IND(s); s << "class " << tdef->uqname() << " : public "
	    << template_name << " {\n";
  IND(s); s << "public:\n";
  INC_INDENT_LEVEL();

  IND(s); s << "typedef " << tdef->uqname() << "_var _var_type;\n";

  // Default constructor.
  IND(s); s << "inline " << tdef->uqname() << "() {}\n";
  // Copy constructor.
  IND(s); s << "inline " << tdef->uqname()
	    << "(const " << tdef->uqname() << "& seq)\n";
  IND(s); s << "  : " << template_name << "(seq) {}\n";
  if( bound() ) {
    // T* data constructor.
    IND(s); s << "inline " << tdef->uqname()
	      << "(CORBA::ULong len, "
	      << (const char*) t_star_constructor_arg 
	      << "* val, CORBA::Boolean rel=0)\n";
    IND(s); s << "  : " << template_name << "(len, val, rel) {}\n";
  } else {
    // Maximum constructor.
    IND(s); s << "inline " << tdef->uqname() << "(CORBA::ULong max)\n";
    IND(s); s << "  : " << template_name << "(max) {}\n";
    // T* data constructor.
    IND(s); s << "inline " << tdef->uqname()
	      << "(CORBA::ULong max, CORBA::ULong len, "
	      << (const char*) t_star_constructor_arg 
	      << "* val, CORBA::Boolean rel=0)\n";
    IND(s); s << "  : " << template_name << "(max, len, val, rel) {}\n";
  }
  // operator =
  IND(s); s << "inline " << tdef->uqname() << "& operator = (const "
	    << tdef->uqname() << "& seq) {\n";
  IND(s); s << "  " << template_name << "::operator=(seq);\n";
  IND(s); s << "  return *this;\n";
  IND(s); s << "};\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";

  StringBuf var_type;
  var_type += tdef->uqname();
  var_type += "_var";

  StringBuf out_type;
  out_type += tdef->uqname();
  out_type += "_out";

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////// T_var ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  IND(s); s << "class " << (const char*) out_type << ";\n\n";

  IND(s); s << "class " << (const char*) var_type << " {\n";
  IND(s); s << "public:\n";
  INC_INDENT_LEVEL();
  IND(s); s << "typedef " << tdef->uqname() << " _T;\n";
  IND(s); s << "typedef " << (const char*) var_type << " _T_var;\n\n";

  IND(s); s << "inline " << (const char*) var_type << "() : pd_seq(0) {}\n";
  IND(s); s << "inline " << (const char*) var_type << "(_T* s) : pd_seq(s) {}\n";
  IND(s); s << "inline " << (const char*) var_type << "(const _T_var& sv) {\n";
  IND(s); s << "  if( sv.pd_seq ) {\n";
  IND(s); s << "    pd_seq = new _T;\n";
  IND(s); s << "    *pd_seq = *sv.pd_seq;\n";
  IND(s); s << "  } else\n";
  IND(s); s << "    pd_seq = 0;\n";
  IND(s); s << "}\n";
  IND(s); s << "inline ~" << (const char*) var_type
	    << "() { if( pd_seq ) delete pd_seq; }\n\n";

  IND(s); s << "inline _T_var& operator = (_T* s) {\n";
  IND(s); s << "  if( pd_seq )  delete pd_seq;\n";
  IND(s); s << "  pd_seq = s;\n";
  IND(s); s << "  return *this;\n";
  IND(s); s << "}\n";
  IND(s); s << "inline _T_var& operator = (const _T_var& sv) {\n";
  IND(s); s << "  if( sv.pd_seq ) {\n";
  IND(s); s << "    if( !pd_seq )  pd_seq = new _T;\n";
  IND(s); s << "    *pd_seq = *sv.pd_seq;\n";
  IND(s); s << "  } else if( pd_seq ) {\n";
  IND(s); s << "    delete pd_seq;\n";
  IND(s); s << "    pd_seq = 0;\n";
  IND(s); s << "  }\n";
  IND(s); s << "  return *this;\n";
  IND(s); s << "}\n\n";

  IND(s); s << "inline " << (const char*) index_ret_type
	    << " operator [] (_CORBA_ULong i) { ";
  if( is_seq_of_array )
    s << "  return (" << (const char*) index_ret_type 
      << ") ((pd_seq->NP_data())[i]); }\n";
  else
    s << "  return (*pd_seq)[i]; }\n";
  IND(s); s << "inline _T* operator -> () { return pd_seq; }\n";
  s << "#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7\n";
  IND(s); s << "inline operator _T& () const { return *pd_seq; }\n";
  s << "#else\n";
  IND(s); s << "inline operator const _T& () const { return *pd_seq; }\n";
  IND(s); s << "inline operator _T& () { return *pd_seq; }\n";
  s << "#endif\n\n";

  IND(s); s << "inline const _T& in() const { return *pd_seq; }\n";
  IND(s); s << "inline _T& inout() { return *pd_seq; }\n";
  IND(s); s << "inline _T*& out() { if (pd_seq) { delete pd_seq; pd_seq = 0; } return pd_seq; }\n";
  IND(s); s << "inline _T* _retn() { _T* tmp = pd_seq; pd_seq = 0; "
	    "return tmp; }\n\n";

  IND(s); s << "friend class " << (const char*) out_type << ";\n\n";

  DEC_INDENT_LEVEL();
  IND(s); s << "private:\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_T* pd_seq;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////// T_out ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  IND(s); s << "class " << (const char*) out_type << " {\n";
  IND(s); s << "public:\n";
  INC_INDENT_LEVEL();
  IND(s); s << "typedef " << tdef->uqname() << " _T;\n";
  IND(s); s << "typedef " << (const char*) var_type << " _T_var;\n\n";

  IND(s); s << "inline " << (const char*) out_type << "(_T*& s) : _data(s) { _data = 0; }\n";
  IND(s); s << "inline " << (const char*) out_type << "(_T_var& sv)\n";
  IND(s); s << "  : _data(sv.pd_seq) { sv = (_T*) 0; }\n";
  IND(s); s << "inline " << (const char*) out_type << "(const "
	    << (const char*) out_type  << "& s) : _data(s._data) { }\n";
  IND(s); s << "inline " << (const char*) out_type << "& operator=(const "
	    << (const char*) out_type << "& s) { _data = s._data; return *this; }\n";
  IND(s); s << "inline " << (const char*) out_type << "& operator=(_T* s) { _data = s; return *this; }\n";
  IND(s); s << "inline operator _T*&() { return _data; }\n";
  IND(s); s << "inline _T*& ptr() { return _data; }\n";
  IND(s); s << "inline _T* operator->() { return _data; }\n";
  IND(s); s << "inline " << (const char*) index_ret_type
	    << " operator [] (_CORBA_ULong i) { ";
  if( is_seq_of_array )
    s << "  return (" << (const char*) index_ret_type 
      << ") ((_data->NP_data())[i]); }\n";
  else
    s << "  return (*_data)[i]; }\n";

  IND(s); s << "_T*& _data;\n\n";

  DEC_INDENT_LEVEL();
  IND(s); s << "private:\n";
  INC_INDENT_LEVEL();
  IND(s); s << (const char*) out_type << "();\n";
  IND(s); s << (const char*) out_type << " operator=( const _T_var&);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";
}


void
o2be_sequence::produce_typedef_binary_operators_in_hdr(std::fstream& s,
						       o2be_typedef* tdef)
{
  if( idl_global->compile_flags() & IDL_CF_ANY ) {

    // Generate the Any insertion and extraction operators.

    IND(s); s << "extern void operator <<= (CORBA::Any& a, const "
	      << tdef->fqname() << "& s);\n";

    IND(s); s << "inline void operator <<= (CORBA::Any& a, "
	      << tdef->fqname() << "* sp) {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "a <<= *sp;\n";
    IND(s); s << "delete sp;\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";

    IND(s); s << "extern CORBA::Boolean operator >>= (const CORBA::Any& a, "
	      << tdef->fqname() << "*& sp);\n";
    IND(s); s << "extern CORBA::Boolean operator >>= (const CORBA::Any& a, const "
	      << tdef->fqname() << "*& sp);\n\n";
  }
}


void
o2be_sequence::produce_typedef_binary_operators_in_dynskel(std::fstream& s,
							   o2be_typedef* tdef)
{
  const char* canon_name = canonical_name();
  const char* seq_type_name = seq_template_name(o2be_global::root());
  const char* tdef_idname = tdef->_idname();
  const char* tdef_fqname = tdef->fqname();

  produce_buildDesc_support(s);

  // Any insertion operator.

  IND(s); s << "void operator <<= (CORBA::Any& a, const "
	    << tdef->fqname() << "& s)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "tcDescriptor tcdesc;\n";
  o2be_buildDesc::call_buildDesc(s, this, "tcdesc", "s");
  IND(s); s << "a.PR_packFrom(" << tdef->fqtcname() << ", &tcdesc);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  // Helper to delete one of these, since the Any keeps the storage.

  IND(s); s << "void _0RL_seq_delete_" << tdef_idname
	    << "(void* data)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "delete (" << tdef_fqname << "*)data;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  // Any extraction operator.
  IND(s); s << "CORBA::Boolean operator >>= (const CORBA::Any& a, "
	    << tdef->fqname() << "*& s_out)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "return a >>= (const " << tdef->fqname() << "*&) s_out;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "CORBA::Boolean operator >>= (const CORBA::Any& a, const "
	    << tdef->fqname() << "*& s_out)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "s_out = 0;\n";
  IND(s); s << tdef_fqname << "* stmp = (" << tdef_fqname
	    <<"*) a.PR_getCachedData();\n";
  IND(s); s << "if( stmp == 0 ) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "tcDescriptor tcdesc;\n";
  IND(s); s << "stmp = new " << tdef_fqname << ";\n";
  o2be_buildDesc::call_buildDesc(s, this, "tcdesc", "*stmp");
  IND(s); s << "if( a.PR_unpackTo(" << tdef->fqtcname()
	    << ", &tcdesc)) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "((CORBA::Any*)&a)->PR_setCachedData((void*)stmp, "
	    << "_0RL_seq_delete_" << tdef_idname << ");\n";
  IND(s); s << "s_out = stmp;\n";
  IND(s); s << "return 1;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "} else {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "delete stmp;\n";
  IND(s); s << "return 0;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "} else {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::TypeCode_var tctmp = a.type();\n";
  IND(s); s << "if( tctmp->equivalent(" << tdef->fqtcname() << ") ) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "s_out = stmp;\n";
  IND(s); s << "return 1;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "} else {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "return 0;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";
}


const char*
o2be_sequence::out_adptarg_name(o2be_typedef* tdef, AST_Decl* used_in)
{
  if( pd_out_adptarg_name )  return pd_out_adptarg_name;

  StringBuf out_type;
  out_type += tdef->unambiguous_name(used_in);
  out_type += "_out";
  pd_out_adptarg_name = out_type.release();

  return pd_out_adptarg_name;
}


// The front end can produce multiple nodes for each pre-defined type and
// string type. We just keep an internal pointer to one instance of each type.
// produce_hdr_for_predefined_types() will use these pointers to invoke
// produce_seq_hdr_if_defined() for that pre-defined type.

static int                    pd_sizeof_predefined_type = 0;
static o2be_predefined_type** pd_predefined_type = NULL;
static o2be_string*           pd_string = NULL;

static
void
set_pd_predefined_type(o2be_predefined_type* s)
{
  if (pd_sizeof_predefined_type <= s->pt())
    {
      o2be_predefined_type** p = new (o2be_predefined_type* [s->pt()+1]);
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
o2be_predefined_type*
get_pd_predefined_type(AST_PredefinedType::PredefinedType type)
{
  if (pd_sizeof_predefined_type <= type)
    return NULL;
  else
    return pd_predefined_type[type];
}

static
void
set_pd_string(o2be_string* s)
{
  if (pd_string == NULL)
    {
      pd_string = s;
    }
}

static
o2be_string*
get_pd_string()
{
  return pd_string;
}

void
o2be_sequence::produce_hdr_for_predefined_types(std::fstream& s)
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


void
o2be_sequence::produce_buildDesc_support(std::fstream& s)
{
  // Ensure we only generate the code for this sequence type
  // once in this source file.
  if( pd_have_produced_tcParser_buildDesc_code )
    return;

  if( recursive_sequence_offset() != 0 )
    // For recursive sequences we must declare the buildDesc for
    // the base_type - even if it is in this file.
    o2be_buildDesc::produce_decls(s, base_type(), I_TRUE);
  else
    // If the component type is not declared in the same
    // file then we must declare the external buildDesc function for it.
    o2be_buildDesc::produce_decls(s, base_type());

  const char* canon_name = canonical_name();
  const char* seq_type_name = seq_template_name(o2be_global::root());

  s << std::endl;
  s << "#ifndef __0RL_tcParser_buildDesc" << canon_name << "__\n";
  s << "#define __0RL_tcParser_buildDesc" << canon_name << "__\n";

  // setElementCount
  IND(s); s << "static void\n";
  IND(s); s << "_0RL_tcParser_setElementCount" << canon_name
	    << "(tcSequenceDesc* _desc, CORBA::ULong _len)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "((" << seq_type_name << "*)_desc->opq_seq)->length(_len);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  // getElementCount
  IND(s); s << "static CORBA::ULong\n";
  IND(s); s << "_0RL_tcParser_getElementCount" << canon_name
	    << "(tcSequenceDesc* _desc)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "return ((" << seq_type_name
	    << "*)_desc->opq_seq)->length();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  // getElementDesc
  IND(s); s << "static CORBA::Boolean\n";
  IND(s); s << "_0RL_tcParser_getElementDesc" << canon_name
	    << "(tcSequenceDesc* _desc, CORBA::ULong _index, "
	    "tcDescriptor& _newdesc)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    char* tmp = new char[1 + 4 + strlen(seq_type_name) + 26];
    strcpy(tmp, "(*((");
    strcat(tmp, seq_type_name);
    strcat(tmp, "*)_desc->opq_seq))[_index]");
    o2be_buildDesc::call_buildDesc(s, base_type(), "_newdesc", tmp);
    delete[] tmp;
  }
  IND(s); s << "return 1;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  // buildDesc function
  IND(s); s << "static void\n";
  IND(s); s << "_0RL_buildDesc" << canon_name << "(tcDescriptor &_desc, ";
  s << "const " << seq_type_name << "& _data)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_desc.p_sequence.opq_seq = (void*) &_data;\n";
  IND(s); s << "_desc.p_sequence.setElementCount =\n";
  IND(s); s << "  _0RL_tcParser_setElementCount" << canon_name << ";\n";
  IND(s); s << "_desc.p_sequence.getElementCount =\n";
  IND(s); s << "  _0RL_tcParser_getElementCount" << canon_name << ";\n";
  IND(s); s << "_desc.p_sequence.getElementDesc =\n";
  IND(s); s << "  _0RL_tcParser_getElementDesc" << canon_name << ";\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";

  s << "#endif\n\n";

  pd_have_produced_tcParser_buildDesc_code = I_TRUE;
}


AST_Sequence*
o2be_sequence::attach_seq_to_base_type(AST_Sequence* se)
{
  AST_Decl* d = se->base_type();
  o2be_sequence* os = o2be_sequence::narrow_from_decl(se);
  switch(d->node_type())
    {
    case AST_Decl::NT_pre_defined:
      {
	// The front end can produce multiple nodes for each pre-defined
	// type. We just keep an internal pointer to one instance of these.
        // produce_hdr_for_predefined_types() will use this pointer to
	// invoke produce_seq_hdr_if_defined() for that pre-defined type.

	o2be_predefined_type* s = o2be_predefined_type::narrow_from_decl(d);
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

	o2be_string* s = o2be_string::narrow_from_decl(d);
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
      break;
    case AST_Decl::NT_typedef:
      o2be_typedef::narrow_from_decl(d)->set_seq_decl(os);
      break;
    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unexpected argument type");
      break;
    }
  return se;
}


void
o2be_sequence_chain::set_seq_decl(o2be_sequence* d)
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
	  AST_Decl* decl = AST_Sequence::narrow_from_decl(this)->base_type();
	  o2be_sequence* ns = o2be_sequence::narrow_from_decl(this);
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
		o2be_sequence_chain* p = o2be_sequence::narrow_from_decl(decl);
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
    o2be_sequence_chain* dd = d;
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
    o2be_sequence_chain* ddd;
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
o2be_sequence_chain::produce_seq_hdr_if_defined(std::fstream& s)
{
  return;
}


IMPL_NARROW_METHODS1(o2be_sequence, AST_Sequence)
IMPL_NARROW_FROM_DECL(o2be_sequence)
IMPL_NARROW_FROM_SCOPE(o2be_sequence)
