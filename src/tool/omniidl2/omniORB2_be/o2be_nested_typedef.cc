// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_struct.cc           Created on: 12/08/1996
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
//   Helper class to generate type definition for nested type
//

/*
  $Log$
  Revision 1.1  1999/05/26 10:04:46  sll
  Initial revision

  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

static
void
iter_produce(std::fstream&s, AST_Decl* d,
	     void (*fn)(std::fstream& s, AST_Decl* def_in, AST_Decl* d))
{
  AST_Decl* decl;

  switch (d->node_type()) {

  case AST_Decl::NT_struct:
  case AST_Decl::NT_except:
    {    
      // declare any constructor types defined in this scope
      UTL_ScopeActiveIterator i(DeclAsScope(d),UTL_Scope::IK_decls);
      while (!i.is_done()) {
	decl = i.item();
	if (decl->node_type() == AST_Decl::NT_field) {
	  decl = AST_Field::narrow_from_decl(decl)->field_type();
	  fn(s,d,decl);
	}
	i.next();
      }
    }
    break; 

  case AST_Decl::NT_union:
    {
      // declare any constructor types defined in this scope
      UTL_ScopeActiveIterator i(DeclAsScope(d),UTL_Scope::IK_decls);
      while (!i.is_done()) {
	decl = i.item();
	if (decl->node_type() == AST_Decl::NT_union_branch) {
	    decl=AST_UnionBranch::narrow_from_decl(decl)->field_type();
	    fn(s,d,decl);
	}
	else if (decl->node_type() == AST_Decl::NT_enum) {
	  fn(s,d,decl);
	}
	i.next();
      }
    }
    break;

  default:
    break;
  }

}

#define CHECK_AND_PRODUCE(s,def_in,d,test_flag,set_flag,produce_fn,recurse_fn) \
do { \
  if ((d)->has_ancestor(def_in)) { \
\
    switch ((d)->node_type()) { \
\
    case AST_Decl::NT_enum: \
      if (!o2be_enum::narrow_from_decl(d)->test_flag()) { \
	o2be_enum::narrow_from_decl(d)->set_flag(); \
	o2be_enum::narrow_from_decl(d)->produce_fn(s); \
      } \
      break; \
\
    case AST_Decl::NT_struct: \
      if (!o2be_structure::narrow_from_decl(d)->test_flag()) { \
	o2be_structure::narrow_from_decl(d)->set_flag(); \
	o2be_structure::narrow_from_decl(d)->produce_fn(s); \
      } \
      break; \
\
    case AST_Decl::NT_union: \
      if (!o2be_union::narrow_from_decl(d)->test_flag()) { \
	o2be_union::narrow_from_decl(d)->set_flag(); \
	o2be_union::narrow_from_decl(d)->produce_fn(s); \
      } \
      break; \
\
    case AST_Decl::NT_array: \
      d = AST_Array::narrow_from_decl(d)->base_type(); \
      recurse_fn(s,def_in,d); \
      break; \
\
    default: \
      break; \
    } \
  } \
} while(0)


static
void
check_and_produce_hdr(std::fstream& s, AST_Decl* def_in, AST_Decl* d)
{
  CHECK_AND_PRODUCE(s,def_in,d,
		    get_hdr_produced_in_field,
		    set_hdr_produced_in_field,
		    produce_hdr,
		    check_and_produce_hdr);
}

void
o2be_nested_typedef::produce_hdr(std::fstream& s, AST_Decl* d)
{
  iter_produce(s,d,check_and_produce_hdr);
}

static
void
check_and_produce_skel(std::fstream& s, AST_Decl* def_in, AST_Decl* d)
{
  CHECK_AND_PRODUCE(s,def_in,d,
		    get_skel_produced_in_field,
		    set_skel_produced_in_field,
		    produce_skel,
		    check_and_produce_skel);
}

void
o2be_nested_typedef::produce_skel(std::fstream& s, AST_Decl* d)
{
  iter_produce(s,d,check_and_produce_skel);
}

static
void
check_and_produce_dynskel(std::fstream& s, AST_Decl* def_in, AST_Decl* d)
{
  CHECK_AND_PRODUCE(s,def_in,d,
		    get_dynskel_produced_in_field,
		    set_dynskel_produced_in_field,
		    produce_dynskel,
		    check_and_produce_dynskel);
}

void
o2be_nested_typedef::produce_dynskel(std::fstream& s, AST_Decl* d)
{
  iter_produce(s,d,check_and_produce_dynskel);
}


static
void
check_and_produce_binary_operators_in_hdr(std::fstream& s, AST_Decl* def_in, 
					  AST_Decl* d)
{
  CHECK_AND_PRODUCE(s,def_in,d,
		    get_binary_operators_hdr_produced_in_field,
		    set_binary_operators_hdr_produced_in_field,
		    produce_binary_operators_in_hdr,
		    check_and_produce_binary_operators_in_hdr);
}

void
o2be_nested_typedef::produce_binary_operators_in_hdr(std::fstream& s,
						     AST_Decl* d)
{
  iter_produce(s,d,check_and_produce_binary_operators_in_hdr);
}

static
void
check_and_produce_binary_operators_in_dynskel(std::fstream& s, 
					      AST_Decl* def_in, 
					      AST_Decl* d)
{
  CHECK_AND_PRODUCE(s,def_in,d,
		    get_binary_operators_skel_produced_in_field,
		    set_binary_operators_skel_produced_in_field,
		    produce_binary_operators_in_dynskel,
		    check_and_produce_binary_operators_in_dynskel);
}

void
o2be_nested_typedef::produce_binary_operators_in_dynskel(std::fstream& s,
							 AST_Decl* d)
{
  iter_produce(s,d,check_and_produce_binary_operators_in_dynskel);
}


static
void
check_and_produce_typecode_skel(std::fstream& s, 
				AST_Decl* def_in, 
				AST_Decl* d)
{
  // For typecode, we have to generate a static variable for
  // each of the types that are used within decl.
  o2be_name::narrow_and_produce_typecode_skel(d, s);
}

void
o2be_nested_typedef::produce_typecode_skel(std::fstream& s, AST_Decl* d)
{
  iter_produce(s,d,check_and_produce_typecode_skel);
}

#if 0

static
void
check_and_produce_decls_at_global_scope_in_hdr(std::fstream& s, 
					       AST_Decl* def_in, AST_Decl* d)
{
  CHECK_AND_PRODUCE(s,def_in,d,
		    have_produced_typecode_skel,
		    set_have_produced_typecode_skel,
		    produce_decls_at_global_scope_in_hdr,
		    check_and_produce_decls_at_global_scope_in_hdr);
}

void
o2be_nested_typedef::produce_decls_at_global_scope_in_hdr(std::fstream& s, 
							  AST_Decl* d)
{
  iter_produce(s,d,check_and_produce_decls_at_global_scope_in_hdr);
}

#endif
