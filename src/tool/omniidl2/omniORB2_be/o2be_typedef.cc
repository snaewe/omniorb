// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_typedef.cc          Created on: 07/10/1996
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
  Revision 1.3  1997/05/06 14:09:04  sll
  Public release.

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
			     strlen(uqname()) +
			     strlen(uqname()) + strlen("_Helper") + 4];
      strcpy(pd_fm_uqname,OBJREF_MEMBER_TEMPLATE_NAME);
      strcat(pd_fm_uqname,"<");
      strcat(pd_fm_uqname,uqname());
      strcat(pd_fm_uqname,",");
      strcat(pd_fm_uqname,uqname());
      strcat(pd_fm_uqname,"_Helper");
      strcat(pd_fm_uqname,">");

      pd_fm_fqname = new char[strlen(OBJREF_MEMBER_TEMPLATE_NAME)+
			     strlen(fqname()) +
			     strlen(fqname()) + strlen("_Helper") + 4];
      strcpy(pd_fm_fqname,OBJREF_MEMBER_TEMPLATE_NAME);
      strcat(pd_fm_fqname,"<");
      strcat(pd_fm_fqname,fqname());
      strcat(pd_fm_fqname,",");
      strcat(pd_fm_fqname,fqname());
      strcat(pd_fm_fqname,"_Helper");
      strcat(pd_fm_fqname,">");
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
      o2be_predefined_type::narrow_from_decl(decl)->produce_typedef_hdr(s,this);
      break;
    case AST_Decl::NT_enum:
      o2be_enum::narrow_from_decl(decl)->produce_typedef_hdr(s,this);
      break;
    case AST_Decl::NT_interface:
      o2be_interface::narrow_from_decl(decl)->produce_typedef_hdr(s,this);
      break;
    case AST_Decl::NT_union:
      o2be_union::narrow_from_decl(decl)->produce_typedef_hdr(s,this);
      break;
    case AST_Decl::NT_struct:
      o2be_structure::narrow_from_decl(decl)->produce_typedef_hdr(s,this);
      break;
    case AST_Decl::NT_string:
      o2be_string::produce_typedef_hdr(s,this);
      break;
    case AST_Decl::NT_sequence:
      o2be_sequence::narrow_from_decl(decl)->produce_typedef_hdr(s,this);
      break;
    case AST_Decl::NT_array:
      if (base_type()->node_type() == AST_Decl::NT_array)
	o2be_array::narrow_from_decl(decl)->produce_hdr(s,this);
      else
	o2be_array::produce_typedef_hdr(s,this,
					o2be_typedef::narrow_from_decl(base_type()));
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
    o2be_array::narrow_from_decl(decl)->produce_skel(s,this);
}

IMPL_NARROW_METHODS1(o2be_typedef, AST_Typedef)
IMPL_NARROW_FROM_DECL(o2be_typedef)
