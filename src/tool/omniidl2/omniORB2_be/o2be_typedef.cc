// -*- Mode: C++; -*-
//                          Package   : omniidl
// o2be_typedef.cc          Created on: 07/10/1996
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

#define OBJREF_MEMBER_TEMPLATE_NAME "_CORBA_ObjRef_Member"
#define STRING_MEMBER_NAME          "CORBA::String_member"
#define SEQUENCE_VAR_TEMPLATE	    "_CORBA_ConstrType_Variable_Var"

o2be_typedef::o2be_typedef(AST_Type *bt, UTL_ScopedName *n, UTL_StrList *p)
	  : AST_Typedef(bt, n, p),
	    AST_Decl(AST_Decl::NT_typedef, n, p),
	    o2be_name(this),
	    o2be_sequence_chain(this)
{
  AST_Decl *decl = base_type();
  const char *tname = o2be_name::narrow_and_produce_fqname(decl);

  while (decl->node_type() == AST_Decl::NT_typedef) {
    decl = o2be_typedef::narrow_from_decl(decl)->base_type();
  }

  switch (decl->node_type())
    {
    case AST_Decl::NT_interface:
      pd_fm_uqname = new char[strlen(OBJREF_MEMBER_TEMPLATE_NAME)+
			     strlen("<>") + strlen(uqname()) + 1];
      strcpy(pd_fm_uqname,OBJREF_MEMBER_TEMPLATE_NAME);
      strcat(pd_fm_uqname,"<");
      strcat(pd_fm_uqname,uqname());
      strcat(pd_fm_uqname,">");
      pd_fm_fqname = pd_fm_uqname;
      break;
    case AST_Decl::NT_string:
      pd_fm_uqname = new char[strlen(STRING_MEMBER_NAME)+1];
      strcpy(pd_fm_uqname,STRING_MEMBER_NAME);
      pd_fm_fqname = pd_fm_uqname;
      break;
    default:
      pd_fm_uqname = uqname();
      pd_fm_fqname = fqname();
      break;
    }
}

void
o2be_typedef::produce_hdr(fstream &s)
{
  AST_Decl *decl = base_type();
  const char *tname = o2be_name::narrow_and_produce_fqname(decl);

  while (decl->node_type() == AST_Decl::NT_typedef) {
    decl = o2be_typedef::narrow_from_decl(decl)->base_type();
  }

  switch (decl->node_type())
    {
    case AST_Decl::NT_pre_defined:
      if (o2be_predefined_type::narrow_from_decl(decl)->pt() ==
	  AST_PredefinedType::PT_any) {
	IND(s); s << "typedef " << tname << " " << uqname() << ";\n";
	IND(s); s << "typedef " << tname << "_var " << uqname() << "_var;\n\n";
      }
      else {
	IND(s); s << "typedef " << tname << " " << uqname() << ";\n\n";
      }
      break;
    case AST_Decl::NT_enum:
      IND(s); s << "typedef " << tname << " " << uqname() << ";\n\n";
      break;
    case AST_Decl::NT_interface:
      IND(s); s << "typedef " << tname << " " << uqname() << ";\n";
      IND(s); s << "typedef " << tname << "_ptr " << uqname() << "_ptr;\n";
      IND(s); s << "typedef " << tname << "Ref " << uqname() << "Ref;\n";
      IND(s); s << "typedef " << tname << "_var " << uqname() << "_var;\n\n";
      break;
    case AST_Decl::NT_union:
      IND(s); s << "typedef " << tname << " " << uqname() << ";\n";
      IND(s); s << "typedef " << tname << "_var " << uqname() << "_var;\n\n";
      break;
    case AST_Decl::NT_struct:
      IND(s); s << "typedef " << tname << " " << uqname() << ";\n";
      IND(s); s << "typedef " << tname << "_var " << uqname() << "_var;\n\n";
      break;
    case AST_Decl::NT_string:
      IND(s); s << "typedef " << tname << " " << uqname() << ";\n";
      IND(s); s << "typedef " << tname << "_var " << uqname() << "_var;\n\n";
      break;
    case AST_Decl::NT_sequence:
#ifdef USE_SEQUENCE_TEMPLATE_IN_PLACE
      IND(s); s << "typedef " 
		<< o2be_sequence::narrow_from_decl(decl)->seq_template_name()
		<< " " << uqname() << ";\n";
      IND(s); s << "typedef " 
		<< SEQUENCE_VAR_TEMPLATE
		<< "<" << uqname() << "> " << uqname() << "_var;\n\n";
#else
      IND(s); s << "typedef " << tname << " " << uqname() << ";\n";
      IND(s); s << "typedef " << tname << "_var " << uqname() << "_var;\n\n";
#endif
      break;
    case AST_Decl::NT_array:
      o2be_array::narrow_from_decl(decl)->produce_typedef_hdr(s,this);
      break;
    default:
      throw o2be_internal_error(__FILE__,__LINE__,"Unexpected argument type");
    }

}

void
o2be_typedef::produce_skel(fstream &s)
{
  AST_Decl *decl = base_type();

  if (decl->node_type() == AST_Decl::NT_array)
    o2be_array::narrow_from_decl(decl)->produce_typedef_skel(s,this);
}

IMPL_NARROW_METHODS1(o2be_typedef, AST_Typedef)
IMPL_NARROW_FROM_DECL(o2be_typedef)
