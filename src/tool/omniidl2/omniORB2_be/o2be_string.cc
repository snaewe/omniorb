// -*- Mode: C++; -*-
//                          Package   : omniidl
// o2be_string.cc           Created on: 12/08/1996
//			    Author    : Sai-Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//   OMNI BE for the class AST_String
//

/*
  $Log$
  Revision 1.1  1997/01/08 17:32:59  sll
  Initial revision

  */

#include "idl.hh"
#include "idl_extern.hh"
#include "o2be.h"

o2be_string::o2be_string(AST_Expression *v)
	 : AST_String(v),
	   AST_Decl(AST_Decl::NT_string,
		    new UTL_ScopedName(new Identifier("string", 1, 0, I_FALSE),
				       NULL),
		    NULL),
	   o2be_name(this),
	   o2be_sequence_chain(this)
{
  char *p = new char [strlen(local_name()->get_string())+1];
  strcpy(p,local_name()->get_string());
  set_uqname(p);
  p = new char [strlen(local_name()->get_string())+1];
  strcpy(p,local_name()->get_string());
  set_fqname(p);
  p = new char [strlen(local_name()->get_string())+1];
  strcpy(p,local_name()->get_string());
  set__fqname(p);
  set_scopename("");
  set__scopename("");
}

o2be_string::o2be_string(AST_Expression *v, long wide)
	 : AST_String(v, wide),
	   AST_Decl(AST_Decl::NT_string,
		    wide == 1
		    ? new UTL_ScopedName(new Identifier("string",1,0,I_FALSE),
					 NULL)
		    : new UTL_ScopedName(new Identifier("wstring_t",
                                                        1,
                                                        0,
                                                        I_FALSE),
					 NULL),
		    NULL),
	   o2be_name(this),
	   o2be_sequence_chain(this)
{
  char *p = new char [strlen(local_name()->get_string())+1];
  strcpy(p,local_name()->get_string());
  set_uqname(p);
  p = new char [strlen(local_name()->get_string())+1];
  strcpy(p,local_name()->get_string());
  set_fqname(p);
  p = new char [strlen(local_name()->get_string())+1];
  strcpy(p,local_name()->get_string());
  set__fqname(p);
  set_scopename("");
  set__scopename("");
}

const char *
o2be_string::fieldMemberTypeName()
{
  return "CORBA::String_member";
}

size_t
o2be_string::max_length()
{
  AST_Expression *e = max_size();
  if (!e)
    return 0;
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
      throw o2be_internal_error(__FILE__,__LINE__,"unexpected type for string maximum size");
    }
}


IMPL_NARROW_METHODS1(o2be_string, AST_String)
IMPL_NARROW_FROM_DECL(o2be_string)
