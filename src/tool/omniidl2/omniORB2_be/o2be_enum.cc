// -*- Mode: C++; -*-
//                          Package   : omniidl
// o2be_enum.cc             Created on: 12/08/1996
//			    Author    : Sai-Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//   OMNI BE for the class AST_Enum
//

/*
  $Log$
  Revision 1.2  1997/01/13 15:42:16  sll
  New member function produce_typdef_hdr(). It is called when a typedef
  declaration is encountered.

  Revision 1.1  1997/01/08 17:32:59  sll
  Initial revision

  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

o2be_enum::o2be_enum(UTL_ScopedName *n, UTL_StrList *p)
       : AST_Enum(n, p),
	 AST_Decl(AST_Decl::NT_enum, n, p),
	 UTL_Scope(AST_Decl::NT_enum),
	 o2be_name(this),
	 o2be_sequence_chain(this)
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
	    << "void operator>>= (" << uqname() << " _e,NetBufferedStream &s) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "::operator>>=((CORBA::ULong)_e,s);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << (!(defined_in()==idl_global->root())?"friend ":"")
	    << "void operator<<= (" << uqname() << " &_e,NetBufferedStream &s) {\n";
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
	    << "void operator>>= (" << uqname() << " _e,MemBufferedStream &s) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "::operator>>=((CORBA::ULong)_e,s);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << (!(defined_in()==idl_global->root())?"friend ":"")
	    << "void operator<<= (" << uqname() << " &_e,MemBufferedStream &s) {\n";
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
  IND(s); s << "typedef " << fqname() << " " << tdef->uqname() << ";\n";
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
    o2be_name(this)
{
}

IMPL_NARROW_METHODS1(o2be_enum_val, AST_EnumVal)
IMPL_NARROW_FROM_DECL(o2be_enum_val)

