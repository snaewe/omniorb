// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_enum.cc             Created on: 12/08/1996
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
//   OMNI BE for the class AST_Enum
//

/*
  $Log$
  Revision 1.5  1997/12/09 19:55:06  sll
  *** empty log message ***

// Revision 1.4  1997/05/06  13:53:47  sll
// Public release.
//
  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

o2be_enum::o2be_enum(UTL_ScopedName *n, UTL_StrList *p)
       : AST_Enum(n, p),
	 AST_Decl(AST_Decl::NT_enum, n, p),
	 UTL_Scope(AST_Decl::NT_enum),
	 o2be_name(AST_Decl::NT_enum,n,p),
	 o2be_sequence_chain(AST_Decl::NT_enum,n,p)
{
  pd_hdr_produced_in_field = I_FALSE;
  pd_skel_produced_in_field = I_FALSE;
}

void
o2be_enum::produce_hdr(fstream &s)
{
  IND(s); s << "enum " << uqname() << " { ";
  {
    UTL_ScopeActiveIterator i(this, IK_decls);
    while (!(i.is_done())) {
      s << o2be_name::narrow_and_produce_uqname(i.item());
      i.next();
      if (!(i.is_done()))
	s << ", ";
    }
  }
  s << " };\n\n";

  IND(s); s << (!(defined_in()==idl_global->root())?"friend ":"")
	    << "inline void operator>>= (" << uqname() << " _e,NetBufferedStream &s) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "::operator>>=((CORBA::ULong)_e,s);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << (!(defined_in()==idl_global->root())?"friend ":"")
	    << "inline void operator<<= (" << uqname() << " &_e,NetBufferedStream &s) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::ULong __e;\n";
  IND(s); s << "__e <<= s;\n";
  IND(s); s << "switch (__e) {\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this, IK_decls);
    while (!(i.is_done())) {
      IND(s) s << "case " << o2be_name::narrow_and_produce_uqname(i.item()) << ":\n";
      i.next();
    }
  }
  INC_INDENT_LEVEL();
  IND(s); s << "_e = (" << uqname() << ") __e;\n";
  IND(s); s << "break;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "default:\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_CORBA_marshal_error();\n";
  DEC_INDENT_LEVEL();
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << (!(defined_in()==idl_global->root())?"friend ":"")
	    << "inline void operator>>= (" << uqname() << " _e,MemBufferedStream &s) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "::operator>>=((CORBA::ULong)_e,s);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << (!(defined_in()==idl_global->root())?"friend ":"")
	    << "inline void operator<<= (" << uqname() << " &_e,MemBufferedStream &s) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::ULong __e;\n";
  IND(s); s << "__e <<= s;\n";
  IND(s); s << "switch (__e) {\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this, IK_decls);
    while (!(i.is_done())) {
      IND(s) s << "case " << o2be_name::narrow_and_produce_uqname(i.item()) << ":\n";
      i.next();
    }
  }
  INC_INDENT_LEVEL();
  IND(s); s << "_e = (" << uqname() << ") __e;\n";
  IND(s); s << "break;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "default:\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_CORBA_marshal_error();\n";
  DEC_INDENT_LEVEL();
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  produce_seq_hdr_if_defined(s);
  return;
}

void
o2be_enum::produce_skel(fstream &s)
{
  return;
}

void
o2be_enum::produce_typedef_hdr(fstream &s, o2be_typedef *tdef)
{
  IND(s); s << "typedef " << unambiguous_name(tdef) 
	    << " " << tdef->uqname() << ";\n";
}


// Narrowing
IMPL_NARROW_METHODS1(o2be_enum, AST_Enum)
IMPL_NARROW_FROM_DECL(o2be_enum)
IMPL_NARROW_FROM_SCOPE(o2be_enum)


o2be_enum_val::o2be_enum_val(unsigned long v, 
			     UTL_ScopedName *n, 
			     UTL_StrList *p)
  : AST_Decl(AST_Decl::NT_enum_val, n, p),
    AST_Constant(AST_Expression::EV_ulong,
		 AST_Decl::NT_enum_val,
		 new AST_Expression(v),
		 n,
		 p),
    AST_EnumVal(v,n,p),
    o2be_name(AST_Decl::NT_enum_val,n,p)
{
  o2be_enum* enum_decl = o2be_enum::narrow_from_decl(ScopeAsDecl(defined_in()));
  set_scopename(enum_decl->scopename());
  set__scopename(enum_decl->_scopename());
  char* buf = new char [strlen(scopename())+strlen(uqname())+1];
  strcpy(buf,scopename());
  strcat(buf,uqname());
  set_fqname(buf);
  buf = new char [strlen(_scopename())+strlen(uqname())+1];
  strcpy(buf,_scopename());
  strcat(buf,uqname());
  set__fqname(buf);
}



IMPL_NARROW_METHODS1(o2be_enum_val, AST_EnumVal)
IMPL_NARROW_FROM_DECL(o2be_enum_val)

