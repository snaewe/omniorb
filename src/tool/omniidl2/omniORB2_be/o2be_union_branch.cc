// -*- Mode: C++; -*-
//                          Package   : omniidl
// o2be_union_branch.cc     Created on: 12/08/1996
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

#include "idl.hh"
#include "idl_extern.hh"
#include "o2be.h"

o2be_union_branch::o2be_union_branch(AST_UnionLabel *lab, AST_Type *ft,
				     UTL_ScopedName *n, UTL_StrList *p)
                    : AST_Decl(AST_Decl::NT_union_branch, n, p),
                      AST_Field(AST_Decl::NT_union_branch, ft, n, p),
                      AST_UnionBranch(lab, ft, n, p),
		      o2be_field(ft,n,p),
		      o2be_name(this)
{
}

// Narrowing
IMPL_NARROW_METHODS1(o2be_union_branch, AST_UnionBranch)
IMPL_NARROW_FROM_DECL(o2be_union_branch)

