//                          Package   : omniidl
// o2be_field.cc            Created on: 9/8/96
//			    Author    : Sai-Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:

/*
  $Log$
  Revision 1.1  1997/01/08 17:32:59  sll
  Initial revision

  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

o2be_field::o2be_field(AST_Type *ft, UTL_ScopedName *n, UTL_StrList *p)
	: AST_Field(ft, n, p),
	  AST_Decl(AST_Decl::NT_field, n, p),
	  o2be_name(this)
{
}

IMPL_NARROW_METHODS1(o2be_field, AST_Field)
IMPL_NARROW_FROM_DECL(o2be_field)
