// -*- Mode: C++; -*-
//                          Package   : omniidl
// o2be_name.cc             Created on: 12/08/1996
//			    Author    : Sai-Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
//
// Description:
//

/*
  $Log$
  Revision 1.1  1997/01/08 17:32:59  sll
  Initial revision

  */

#include "idl.hh"
#include "idl_extern.hh"
#include "o2be.h"
#include <iostream.h>

static char *internal_produce_scope_name(UTL_ScopedName *n,char *separator);
static char *internal_check_name_for_reserved_words(char *p);

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

  char *q = internal_produce_scope_name(decl->name(),"/");
  p = new char [strlen("IDL::1.0") + strlen(q) + strlen(uqname()) + 1];
  strcpy(p,"IDL:");
  strcat(p,q);
  strcat(p,uqname());
  strcat(p,":1.0");
  delete [] q;
  pd_repositoryID = p;
  return;
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

  id = iter.item();
  while (!iter.is_done())
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
      id = iter.item();
      iter.next();
    }
  return p;
}

// reserved words are the keywords of C++ minus the keywords of CORBA IDL.
static char *reserved_words[] = {
  "asm", "auto", "break", 
  "catch",  "class", "continue",
  "delete", "do",
  "else", "extern",
  "for", "friend", 
  "goto", "if", "inline", "int", "new", "operator",
  "private", "protected", "public",
  "register", "return",
  "signed", "sizeof", "static",
  "template", "this", "throw", "try",
  "virtual", "volatile", "while",
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
}
