//                          Package   : omniidl2
// o2be_module.cc           Created on: 8/8/1996
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
   Revision 1.5  1997/12/09 19:55:33  sll
   *** empty log message ***

// Revision 1.4  1997/05/06  13:59:35  sll
// Public release.
//
   */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#include <iostream.h>

o2be_module::o2be_module(UTL_ScopedName *n, UTL_StrList *p)
                 : AST_Decl(AST_Decl::NT_module, n, p),
		   UTL_Scope(AST_Decl::NT_module),
                   o2be_name(AST_Decl::NT_module,n,p) 
{
}

void
o2be_module::produce_hdr(fstream &s)
{
  if (!(in_main_file()))
    return;

  if (defined_in() != 0)
    {
      IND(s); s << "_CORBA_MODULE " << uqname() << " {\n";
      IND(s); s << "_CORBA_MODULE_PUBLIC\n\n";
      INC_INDENT_LEVEL();
    }

  UTL_ScopeActiveIterator  i(this,UTL_Scope::IK_decls);
  AST_Decl                 *decl;

  while (!(i.is_done()))
    {
      decl = i.item();
      if ((decl->in_main_file()))
	{
	  switch(decl->node_type()) {
	  case AST_Decl::NT_module:
	    o2be_module::narrow_from_decl(decl)->produce_hdr(s); 
	    break;
	  case AST_Decl::NT_interface:
	    o2be_interface::narrow_from_decl(decl)->produce_hdr(s);
	    break;
	  case AST_Decl::NT_except:
	    o2be_exception::narrow_from_decl(decl)->produce_hdr(s);
	    break;
	  case AST_Decl::NT_struct:
	    o2be_structure::narrow_from_decl(decl)->produce_hdr(s);
	    break;
	  case AST_Decl::NT_typedef:
	    o2be_typedef::narrow_from_decl(decl)->produce_hdr(s);
	    break;
	  case AST_Decl::NT_union:
	    o2be_union::narrow_from_decl(decl)->produce_hdr(s);
	    break;
	  case AST_Decl::NT_const:
	    o2be_constant::narrow_from_decl(decl)->produce_hdr(s);
	    break;
	  case AST_Decl::NT_interface_fwd:
	    o2be_interface_fwd::narrow_from_decl(decl)->produce_hdr(s);
	    break;
	  case AST_Decl::NT_enum:
	    o2be_enum::narrow_from_decl(decl)->produce_hdr(s);
	    break;
	  case AST_Decl::NT_enum_val:
	    break;
	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,"Unrecognised node type");
	  }
	}
      i.next();
    }

  if (defined_in() != 0)
    {
      DEC_INDENT_LEVEL();

      IND(s); s << "};\n\n";
    }

  return;
}

void
o2be_module::produce_skel(fstream &s)
{
  if (!(in_main_file()))
    return;

  UTL_ScopeActiveIterator  i(this,UTL_Scope::IK_decls);
  AST_Decl                 *decl;

  while (!(i.is_done()))
    {
      decl = i.item();
      if ((decl->in_main_file()))
	{
	  switch(decl->node_type()) {
	  case AST_Decl::NT_module:
	    o2be_module::narrow_from_decl(decl)->produce_skel(s); 
	    break;
	  case AST_Decl::NT_interface:
	    o2be_interface::narrow_from_decl(decl)->produce_skel(s);
	    break;
	  case AST_Decl::NT_except:
	    o2be_exception::narrow_from_decl(decl)->produce_skel(s);
	    break;
	  case AST_Decl::NT_struct:
	    o2be_structure::narrow_from_decl(decl)->produce_skel(s);
	    break;
	  case AST_Decl::NT_typedef:
	    o2be_typedef::narrow_from_decl(decl)->produce_skel(s);
	    break;
	  case AST_Decl::NT_union:
	    o2be_union::narrow_from_decl(decl)->produce_skel(s);
	    break;
	  case AST_Decl::NT_const:
	    o2be_constant::narrow_from_decl(decl)->produce_skel(s);
	    break;
	  case AST_Decl::NT_interface_fwd:
	    break;
	  case AST_Decl::NT_enum:
	    o2be_enum::narrow_from_decl(decl)->produce_skel(s);
	    break;
	  case AST_Decl::NT_enum_val:
	    break;
	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,"Unrecognised node type");
	  }
	}
      i.next();
    }
  return;
}

// Narrowing
IMPL_NARROW_METHODS1(o2be_module, AST_Module)
IMPL_NARROW_FROM_DECL(o2be_module)
IMPL_NARROW_FROM_SCOPE(o2be_module)

