// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_constant.cc         Created on: 12/08/1996
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
  Revision 1.2  1997/05/06 13:52:40  sll
  Public release.

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

