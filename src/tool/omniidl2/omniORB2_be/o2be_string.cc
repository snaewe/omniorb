// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_string.cc           Created on: 12/08/1996
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
//   OMNI BE for the class AST_String
//

/*
  $Log$
  Revision 1.12  1999/05/26 10:42:12  sll
  Now generate a typecode constant for anonymous bounded string defined in
  operation signature.

  Revision 1.11  1999/03/11 16:26:08  djr
  Updated copyright notice

  Revision 1.10  1999/01/07 09:36:22  djr
  *** empty log message ***

  Revision 1.9  1998/08/13 22:45:25  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.8  1998/04/07 18:51:41  sll
  Use std::fstream instead of fstream.

// Revision 1.7  1998/01/27  16:49:27  ewc
//  Added support for type Any and TypeCode
//
  Revision 1.6  1997/12/09 19:55:28  sll
  *** empty log message ***

// Revision 1.5  1997/05/06  14:07:43  sll
// Public release.
//
  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

o2be_string::o2be_string(AST_Expression *v)
	 : AST_String(v),
	   AST_Decl(AST_Decl::NT_string,
		    new UTL_ScopedName(new Identifier("string", 1, 0, I_FALSE),
				       NULL),
		    NULL),
	   o2be_name(AST_Decl::NT_string,
		    new UTL_ScopedName(new Identifier("string", 1, 0, I_FALSE),
				       NULL),
		    NULL),
	   o2be_sequence_chain(AST_Decl::NT_string,
		    new UTL_ScopedName(new Identifier("string", 1, 0, I_FALSE),
				       NULL),
		    NULL)
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
  set_fqtcname("");
  set__fqtcname("");

  if (max_length()) {
    p = new char [32];
    *p = '\0';
    sprintf(p,"_tc_string_%d",max_length());
    set_tcname(p);
  }
  else {
    set_tcname("");
  }
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
	   o2be_name(AST_Decl::NT_string,
		    wide == 1
		    ? new UTL_ScopedName(new Identifier("string",1,0,I_FALSE),
					 NULL)
		    : new UTL_ScopedName(new Identifier("wstring_t",
                                                        1,
                                                        0,
                                                        I_FALSE),
					 NULL),
		    NULL),
	   o2be_sequence_chain(AST_Decl::NT_string,
		    wide == 1
		    ? new UTL_ScopedName(new Identifier("string",1,0,I_FALSE),
					 NULL)
		    : new UTL_ScopedName(new Identifier("wstring_t",
                                                        1,
                                                        0,
                                                        I_FALSE),
					 NULL),
		    NULL)
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

  set_tcname("");
  set_fqtcname("");
  set__fqtcname("");
}


const char*
o2be_string::fieldMemberTypeName()
{
  return "CORBA::String_member";
}


void
o2be_string::produce_typedef_hdr(std::fstream &s, o2be_typedef *tdef)
{
  IND(s); s << "typedef char* " << tdef->uqname() << ";\n";
  IND(s); s << "typedef CORBA::String_var " << tdef->uqname() << "_var;\n";
}


void
o2be_string::produce_buildDesc_support(std::fstream& s)
{
  // Bounded strings have different canonical names from each
  // other, and from unbounded strings - but can all use the
  // same buildDesc code. Thus we just use a MACRO for simplicity.

  size_t len = max_length();
  if( !len )  return;

  const char* canon_name = canonical_name();

  s << "#ifndef _0RL_buildDesc" << canon_name << "\n";
  s << "#define _0RL_buildDesc" << canon_name << " _0RL_buildDesc_cstring\n";
  s << "#endif\n\n";
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
  return 0; // For MSVC++ 4.2
}


IMPL_NARROW_METHODS1(o2be_string, AST_String)
IMPL_NARROW_FROM_DECL(o2be_string)
