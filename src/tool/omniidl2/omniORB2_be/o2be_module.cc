//                          Package   : omniidl
// o2be_module.cc           Created on: 8/8/1996
//			    Author    : Sai-Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
//
// Description:
//

/* 
   $Log$
   Revision 1.2  1997/01/13 15:31:29  sll
   Added interface forward declaration as a valid node type.

   Revision 1.1  1997/01/08 17:32:59  sll
   Initial revision

   */

#include "idl.hh"
#include "idl_extern.hh"
#include "o2be.h"

#include <iostream.h>

void
o2be_module::produce_hdr(fstream &s)
{
  if (!(in_main_file()))
    return;

  if (defined_in() != 0)
    {
      IND(s); s << "class " << uqname() << " {\n";
      IND(s); s << "public:\n\n";
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
#if 1
	  case AST_Decl::NT_enum_val:
	    break;
	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,"Unrecognised node type");
#else
	  default:
	    break;
#endif
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
#if 1
	  case AST_Decl::NT_enum_val:
	    break;
	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,"Unrecognised node type");
#else
	  default:
	    break;
#endif
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

