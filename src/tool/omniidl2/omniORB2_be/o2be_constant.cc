// -*- Mode: C++; -*-
//                          Package   : omniidl
// o2be_constant.cc         Created on: 12/08/1996
//			    Author    : Sai-Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
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

o2be_constant::o2be_constant(AST_Expression::ExprType et,
			 AST_Expression *v,
			 UTL_ScopedName *n,
			 UTL_StrList *p)
	   : AST_Constant(et, v, n, p),
	     AST_Decl(AST_Decl::NT_const, n, p),
	     o2be_name(this)
{
}

void
o2be_constant::produce_hdr(fstream &s)
{
  IND(s); s << ((defined_in()== idl_global->root()) ? "extern const " 
		                                    : "static const ");
  AST_Expression::ExprType etype = et();
  switch (etype) {
  case AST_Expression::EV_short:
    s << "CORBA::Short";
    break;
  case AST_Expression::EV_ushort:
    s << "CORBA::UShort";
    break;
  case AST_Expression::EV_long:
    s <<  "CORBA::Long";
    break;
  case AST_Expression::EV_ulong:
    s << "CORBA::ULong";
    break;
  case AST_Expression::EV_float:
    s << "CORBA::Float";
    break;
  case AST_Expression::EV_double:
    s << "CORBA::Double";
    break;
  case AST_Expression::EV_char:
    s <<  "CORBA::Char";
    break;
  case AST_Expression::EV_octet:
    s << "CORBA::Octet";
    break;
  case AST_Expression::EV_bool:
    s << "CORBA::Boolean";
    break;
  case AST_Expression::EV_string:
    s << "char *";
    break;
  default:
    throw o2be_internal_error(__FILE__,__LINE__,"unexpected type under constant class");
    break;
  }
  s << " " << uqname() << ";\n";
  return;
}

void
o2be_constant::produce_skel(fstream &s)
{
  char *quote = NULL;

  IND(s); s << "const ";
  AST_Expression::ExprType etype = et();
  switch (etype) {
  case AST_Expression::EV_short:
    s << "CORBA::Short";
    break;
  case AST_Expression::EV_ushort:
    s << "CORBA::UShort";
    break;
  case AST_Expression::EV_long:
    s <<  "CORBA::Long";
    break;
  case AST_Expression::EV_ulong:
    s << "CORBA::ULong";
    break;
  case AST_Expression::EV_float:
    s << "CORBA::Float";
    break;
  case AST_Expression::EV_double:
    s << "CORBA::Double";
    break;
  case AST_Expression::EV_char:
    s <<  "CORBA::Char";
    quote = "'";
    break;
  case AST_Expression::EV_octet:
    s << "CORBA::Octet";
    break;
  case AST_Expression::EV_bool:
    s << "CORBA::Boolean";
    break;
  case AST_Expression::EV_string:
    s << "char *";
    quote = "\"";
    break;
  default:
    throw o2be_internal_error(__FILE__,__LINE__,"unexpected type under constant class");
    break;
  }
  s << " " << fqname() << " = " 
    << ((quote != NULL) ? quote : "");
  constant_value()->dump(s);
  s << ((quote != NULL) ? quote : "") << ";\n";
  return;
}

// Narrowing
IMPL_NARROW_METHODS1(o2be_constant, AST_Constant)
IMPL_NARROW_FROM_DECL(o2be_constant)

