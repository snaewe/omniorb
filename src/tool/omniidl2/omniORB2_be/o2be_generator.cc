// -*- Mode: C++; -*-
//                          Package   : omniidl
// o2be_generator.cc        Created on: 06/08/1996
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

#include  "idl.hh"
#include  "idl_extern.hh"
#include  "o2be.h"

AST_Root  *
o2be_generator::create_root(UTL_ScopedName *n,UTL_StrList *p)
{
  return (AST_Root *) new o2be_root(n,p);
}

AST_PredefinedType *
o2be_generator::create_predefined_type(AST_PredefinedType::PredefinedType t,
				     UTL_ScopedName *n,
				     UTL_StrList *p)
{
  return (AST_PredefinedType *) new o2be_predefined_type(t, n, p);
}

AST_Module *
o2be_generator::create_module(UTL_ScopedName *n, UTL_StrList *p)
{
  return (AST_Module *) new o2be_module(n, p);
}

AST_Interface *
o2be_generator::create_interface(UTL_ScopedName *n,
			       AST_Interface **ih,
			       long nih,
			       UTL_StrList *p)
{
  return (AST_Interface *) new o2be_interface(n, ih, nih, p);
}

AST_InterfaceFwd *
o2be_generator::create_interface_fwd(UTL_ScopedName *n, UTL_StrList *p)
{
  return (AST_InterfaceFwd *) new o2be_interface_fwd(n, p);
}

AST_Exception *
o2be_generator::create_exception(UTL_ScopedName *n, UTL_StrList *p)
{
  return (AST_Exception *) new o2be_exception(n, p);
}

AST_Structure *
o2be_generator::create_structure(UTL_ScopedName *n, UTL_StrList *p)
{
  return (AST_Structure *) new o2be_structure(n, p);
}

AST_Enum *
o2be_generator::create_enum(UTL_ScopedName *n, UTL_StrList *p)
{
  return (AST_Enum *) new o2be_enum(n, p);
}

AST_Operation *
o2be_generator::create_operation(AST_Type *rt,
			       AST_Operation::Flags fl,
			       UTL_ScopedName *n,
			       UTL_StrList *p)
{
  return (AST_Operation *) new o2be_operation(rt, fl, n, p);
}

AST_Field *
o2be_generator::create_field(AST_Type *ft, UTL_ScopedName *n, UTL_StrList *p)
{
  return (AST_Field *) new o2be_field(ft, n, p);
}

AST_Argument *
o2be_generator::create_argument(AST_Argument::Direction d,
			      AST_Type *ft,
			      UTL_ScopedName *n,
			      UTL_StrList *p)
{
  return (AST_Argument *) new o2be_argument(d, ft, n, p);
}

AST_Attribute *
o2be_generator::create_attribute(idl_bool ro,
			       AST_Type *ft,
			       UTL_ScopedName *n,
			       UTL_StrList *p)
{
  return (AST_Attribute *) new o2be_attribute(ro, ft, n, p);
}

AST_Union *
o2be_generator::create_union(AST_ConcreteType *dt,
			   UTL_ScopedName *n,
			   UTL_StrList *p)
{
  return (AST_Union *) new o2be_union(dt, n, p);
}

AST_UnionBranch *
o2be_generator::create_union_branch(AST_UnionLabel *lab,
				  AST_Type *ft,
				  UTL_ScopedName *n,
				  UTL_StrList *p)
{
  return (AST_UnionBranch *) new o2be_union_branch(lab, ft, n, p);
}

AST_Constant *
o2be_generator::create_constant(AST_Expression::ExprType et,
			      AST_Expression *ev,
			      UTL_ScopedName *n,
			      UTL_StrList *p)
{
  return (AST_Constant *) new o2be_constant(et, ev, n, p);
}

AST_EnumVal *
o2be_generator::create_enum_val(unsigned long v,
			      UTL_ScopedName *n,
			      UTL_StrList *p)
{
  return (AST_EnumVal *) new o2be_enum_val(v, n, p);
}

AST_Array *
o2be_generator::create_array(UTL_ScopedName *n,
			   unsigned long ndims,
			   UTL_ExprList *dims)
{
  return (AST_Array *) new o2be_array(n, ndims, dims);
}

AST_Sequence *
o2be_generator::create_sequence(AST_Expression *v, AST_Type *bt)
{
  return (AST_Sequence *) new o2be_sequence(v, bt);
}

AST_String *
o2be_generator::create_string(AST_Expression *v)
{
  return (AST_String *) new o2be_string(v);
}

AST_String *
o2be_generator::create_wstring(AST_Expression *v)
{
  return (AST_String *) new o2be_string(v, sizeof(wchar_t));
}

AST_Typedef *
o2be_generator::create_typedef(AST_Type *bt, UTL_ScopedName *n, UTL_StrList *p)
{
  return (AST_Typedef *) new o2be_typedef(bt, n, p);
}
