// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_name.cc             Created on: 12/08/1996
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
//

/*
  $Log$
  Revision 1.6  1997/08/21 21:16:26  sll
  Minor cleanup.

// Revision 1.5  1997/05/06  14:01:18  sll
// Public release.
//
  */

#include "idl.hh"
#include "idl_extern.hh"
#include "o2be.h"
#include <iostream.h>

static char *internal_produce_scope_name(UTL_ScopedName *n,char *separator);
static char *internal_check_name_for_reserved_words(char *p);
static char *internal_produce_repositoryID(AST_Decl *decl,o2be_name *n);
static UTL_String* internal_search_pragma(AST_Decl* decl,char* p);

o2be_name::o2be_name(AST_Decl *decl)
{
  pd_scopename = internal_produce_scope_name(decl->name(),"::");
  pd__scopename = internal_produce_scope_name(decl->name(),"_");
  pd_uqname = internal_check_name_for_reserved_words(decl->local_name()
						     ->get_string());
  char *p = new char [strlen(pd_scopename)+strlen(pd_uqname)+1];
  strcpy(p,pd_scopename);
  strcat(p,pd_uqname);
  pd_fqname = p;
  p = new char [strlen(pd__scopename)+strlen(pd_uqname)+1];
  strcpy(p,pd__scopename);
  strcat(p,pd_uqname);
  pd__fqname = p;

  pd_repositoryID = internal_produce_repositoryID(decl,this);
  pd_decl = decl;
  return;
}

char*
o2be_name::repositoryID() {
  // Check the pragmas attached to this node to see if
  // pragma ID is defined to override the default repositoryID.
  UTL_String* id;
  if ((id = internal_search_pragma(pd_decl,"ID")) != 0) {
    return id->get_string();
  }
  else if ((id = internal_search_pragma(pd_decl,"version")) != 0) {
    // Check if pragma version is defined to override the
    // version number in the default repositoryID.
    char* p = strrchr(pd_repositoryID,':') + 1;
    char* result = new char[(p-pd_repositoryID)+strlen(id->get_string())+1];
    strncpy(result,pd_repositoryID,p-pd_repositoryID);
    result[p-pd_repositoryID] = '\0';
    strcat(result,id->get_string());
    return result;
  }
  else {
    return pd_repositoryID;
  }
}

static
char*
internal_produce_repositoryID(AST_Decl *decl,o2be_name *n)
{
  // find if any pragma prefix applies to this node. 
  UTL_String* prefix;
  UTL_Scope*  prefix_scope = 0;

  if (!(prefix = internal_search_pragma(decl,"prefix"))) {
    // no pragma prefix defined in this node, search the parents
    prefix_scope = decl->defined_in();
    while (prefix_scope) {
      if (!(prefix = internal_search_pragma(ScopeAsDecl(prefix_scope),
					    "prefix"))) {
	prefix_scope = ScopeAsDecl(prefix_scope)->defined_in();
      }
      else {
	break;
      }
    }
  }

  UTL_ScopedName* idlist;

  if (prefix) {
    idlist = new UTL_ScopedName(decl->local_name(),NULL);
    UTL_Scope* u = decl->defined_in();
    while (prefix_scope && prefix_scope != u) {
      idlist = new UTL_ScopedName(ScopeAsDecl(u)->local_name(),idlist);
      u = ScopeAsDecl(u)->defined_in();
    }
    idlist = new UTL_ScopedName(new Identifier(prefix->get_string()),idlist);

  }
  else {
    idlist = decl->name();
  }
  char* q = internal_produce_scope_name(idlist,"/");
  char* result = new char [strlen("IDL::1.0") + 
			   strlen(q) + 
			   strlen(n->uqname())+1];
  strcpy(result,"IDL:");
  strcat(result,q);
  strcat(result,n->uqname());
  strcat(result,":1.0");
  delete [] q;
  return result;
}

static
UTL_String*
internal_search_pragma(AST_Decl* decl,char* p) 
{
  UTL_String* result = 0;
  UTL_StrlistActiveIterator l(decl->pragmas());
  while (!l.is_done()) {
    if (strcmp(p,l.item()->get_string()) == 0) {
      l.next();
      result = l.item();
    }
    else {
      l.next();
    }
    l.next();
  }
  return result;
}


static
char *
internal_produce_scope_name(UTL_ScopedName *n,char *separator)
{
#define DEFAULT_FQN_SIZE 128
  int  nsep = (separator == NULL) ? 0 : strlen(separator);
  int  n_alloc = DEFAULT_FQN_SIZE;
  int  n_used  = 1;
  char *p = new char[n_alloc];
  *p = '\0';

  UTL_ScopedNameActiveIterator iter(n);
  Identifier *id;
  char *q;

  Identifier *last = n->last_component();

  id = iter.item();
  while (!iter.is_done() && id != last)
    {
      q = internal_check_name_for_reserved_words(id->get_string());
      if (strlen(q) != 0)
	{
	  n_used += strlen(q) + nsep;
	  while (n_used > n_alloc)
	    {
	      n_alloc += DEFAULT_FQN_SIZE;
	      char *t = new char[n_alloc];
	      strcpy(t,p);
	      delete [] p;
	      p = t;
	    }
	  strcat(p,q);
	  if (nsep)
	    strcat(p,separator);
	  if (q != id->get_string())
	    delete [] q;
	}
      iter.next();
      id = iter.item();
    }
  return p;
}

// reserved words are the keywords of C++ minus the keywords of CORBA IDL.
static char *reserved_words[] = {
  "and", "and_eq", "asm", "auto", 
  "bitand", "bitor", "break", 
  "catch",  "class", "compl", "const_cast", "continue",
  "delete", "do", "dynamic_cast",
  "else", "explicit", "extern",
  "false", "for", "friend", 
  "goto", "if", "inline", "int", 
  "mutable", 
  "namespace", "new", "not", "not_eq",
  "operator", "or", "or_eq",
  "private", "protected", "public",
  "register", "reinterpret_cast", "return",
  "signed", "sizeof", "static", "static_cast",
  "template", "this", "throw", "true", "try", "typeid", "typename",
  "using", 
  "virtual", "volatile", "wchar_t", "while",
  "xor", "xor_eq",
  NULL
};

char *
internal_check_name_for_reserved_words(char *keyw)
{
  char **rp = reserved_words;

  while (*rp != NULL)
    {
      if (!strcmp(*rp,keyw))
	{
	  // This is a reserved word, prefix it by '_'
	  char *str = new char[strlen(keyw)+2];
	  strcpy(str,"_");
	  strcat(str,keyw);
	  return str;
	}
      rp++;
    }
  return keyw;
}


char *
o2be_name::narrow_and_produce_fqname(AST_Decl *decl)
{
  switch(decl->node_type())
    {
    case AST_Decl::NT_module:
      return o2be_module::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_root:
      return o2be_root::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_interface:
      return o2be_interface::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_interface_fwd:
      return o2be_interface_fwd::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_const:
      return o2be_constant::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_except:
      return o2be_exception::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_attr:
      return o2be_attribute::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_op:
      return o2be_operation::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_argument:
      return o2be_argument::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_union:
      return o2be_union::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_union_branch:
      return o2be_union_branch::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_struct:
      return o2be_structure::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_field:
      return o2be_field::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_enum:
      return o2be_enum::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_enum_val:
      return o2be_enum_val::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_string:
      return o2be_string::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_array:
      return o2be_array::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_sequence:
      return o2be_sequence::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_typedef:
      return o2be_typedef::narrow_from_decl(decl)->fqname();
    case AST_Decl::NT_pre_defined:
      return o2be_predefined_type::narrow_from_decl(decl)->fqname();
    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unrecognised argument type");
    }
return 0; // For MSVC++ 4.2
}


char *
o2be_name::narrow_and_produce__fqname(AST_Decl *decl)
{
  switch(decl->node_type())
    {
    case AST_Decl::NT_module:
      return o2be_module::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_root:
      return o2be_root::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_interface:
      return o2be_interface::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_interface_fwd:
      return o2be_interface_fwd::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_const:
      return o2be_constant::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_except:
      return o2be_exception::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_attr:
      return o2be_attribute::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_op:
      return o2be_operation::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_argument:
      return o2be_argument::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_union:
      return o2be_union::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_union_branch:
      return o2be_union_branch::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_struct:
      return o2be_structure::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_field:
      return o2be_field::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_enum:
      return o2be_enum::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_enum_val:
      return o2be_enum_val::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_string:
      return o2be_string::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_array:
      return o2be_array::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_sequence:
      return o2be_sequence::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_typedef:
      return o2be_typedef::narrow_from_decl(decl)->_fqname();
    case AST_Decl::NT_pre_defined:
      return o2be_predefined_type::narrow_from_decl(decl)->_fqname();
    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unrecognised argument type");
    }
return 0; // For MSVC++ 4.2
}

char *
o2be_name::narrow_and_produce_scopename(AST_Decl *decl)
{
  switch(decl->node_type())
    {
    case AST_Decl::NT_module:
      return o2be_module::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_root:
      return o2be_root::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_interface:
      return o2be_interface::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_interface_fwd:
      return o2be_interface_fwd::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_const:
      return o2be_constant::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_except:
      return o2be_exception::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_attr:
      return o2be_attribute::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_op:
      return o2be_operation::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_argument:
      return o2be_argument::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_union:
      return o2be_union::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_union_branch:
      return o2be_union_branch::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_struct:
      return o2be_structure::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_field:
      return o2be_field::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_enum:
      return o2be_enum::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_enum_val:
      return o2be_enum_val::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_string:
      return o2be_string::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_array:
      return o2be_array::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_sequence:
      return o2be_sequence::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_typedef:
      return o2be_typedef::narrow_from_decl(decl)->scopename();
    case AST_Decl::NT_pre_defined:
      return o2be_predefined_type::narrow_from_decl(decl)->scopename();
    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unrecognised argument type");
    }
return 0; // For MSVC++ 4.2
}

char *
o2be_name::narrow_and_produce__scopename(AST_Decl *decl)
{
  switch(decl->node_type())
    {
    case AST_Decl::NT_module:
      return o2be_module::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_root:
      return o2be_root::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_interface:
      return o2be_interface::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_interface_fwd:
      return o2be_interface_fwd::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_const:
      return o2be_constant::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_except:
      return o2be_exception::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_attr:
      return o2be_attribute::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_op:
      return o2be_operation::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_argument:
      return o2be_argument::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_union:
      return o2be_union::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_union_branch:
      return o2be_union_branch::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_struct:
      return o2be_structure::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_field:
      return o2be_field::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_enum:
      return o2be_enum::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_enum_val:
      return o2be_enum_val::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_string:
      return o2be_string::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_array:
      return o2be_array::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_sequence:
      return o2be_sequence::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_typedef:
      return o2be_typedef::narrow_from_decl(decl)->_scopename();
    case AST_Decl::NT_pre_defined:
      return o2be_predefined_type::narrow_from_decl(decl)->_scopename();
    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unrecognised argument type");
    }
return 0; // For MSVC++ 4.2
}


char *
o2be_name::narrow_and_produce_uqname(AST_Decl *decl)
{
  switch(decl->node_type())
    {
    case AST_Decl::NT_module:
      return o2be_module::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_root:
      return o2be_root::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_interface:
      return o2be_interface::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_interface_fwd:
      return o2be_interface_fwd::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_const:
      return o2be_constant::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_except:
      return o2be_exception::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_attr:
      return o2be_attribute::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_op:
      return o2be_operation::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_argument:
      return o2be_argument::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_union:
      return o2be_union::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_union_branch:
      return o2be_union_branch::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_struct:
      return o2be_structure::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_field:
      return o2be_field::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_enum:
      return o2be_enum::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_enum_val:
      return o2be_enum_val::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_string:
      return o2be_string::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_array:
      return o2be_array::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_sequence:
      return o2be_sequence::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_typedef:
      return o2be_typedef::narrow_from_decl(decl)->uqname();
    case AST_Decl::NT_pre_defined:
      return o2be_predefined_type::narrow_from_decl(decl)->uqname();
    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unrecognised argument type");
    }
return 0; // For MSVC++ 4.2
}
