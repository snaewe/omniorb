// -*- Mode: C++; -*-
//                          Package   : omniidl
// o2be_struct.cc           Created on: 12/08/1996
//			    Author    : Sai-Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//   OMNI BE for the class AST_Struct
//

/*
  $Log$
  Revision 1.2  1997/01/13 15:24:23  sll
  New member function produce_typedef_hdr(). Called when a typedef declaration
  is encountered.

  Revision 1.1  1997/01/08 17:32:59  sll
  Initial revision

  */

/*
  Example:

  // IDL

  struct fixedlen {
     short a;
     long  b;
  };

  // C++
  struct fixedlen {
     CORBA::Short a;
     CORBA::Long  b;
  };

  class fixedlen_var {
  public:
     fixedlen_var();
     fixedlen_var(fixedlen *);
     fixedlen_var(const fixedlen_var &);
     ~fixedlen_var();
     fixedlen_var &operator=(fixedlen *);
     fixedlen_var &operator=(const fixedlen_var &);
     fixedlen *operator-> const ();

     // conversion operators to support parameter passing
     operator fixedlen &();
     operator fixedlen *&();
     operator const fixedlen *() const;
  };


  // IDL
  interface A {
     ...	
  };
  struct varlen {
     short  a;
     long   b;
     string c;
     A      d;
  };

  // C++
  struct varlen {
     CORBA::Short a;
     CORBA::Long  b;
     CORBA::String_Member c;
     A_Member             d;
  };

  class varlen_var {
  public:
     varlen_var();
     varlen_var(varlen *);
     varlen_var(const varlen_var &);
     ~varlen_var();
     varlen_var &operator=(varlen *);
     varlen_var &operator=(const varlen_var &);
     varlen *operator-> const ();

     // conversion operators to support parameter passing
     operator varlen &();
     operator varlen *&();
     operator const varlen *() const;
  };

  */

#include "idl.hh"
#include "idl_extern.hh"
#include "o2be.h"


o2be_structure::o2be_structure(UTL_ScopedName *n, UTL_StrList *p)
	    : AST_Decl(AST_Decl::NT_struct, n, p),
	      UTL_Scope(AST_Decl::NT_struct),
              o2be_name(this),
	      o2be_sequence_chain(this)
{
  pd_isvar = I_FALSE;
  pd_hdr_produced_in_field = I_FALSE;
  pd_skel_produced_in_field = I_FALSE;
}

AST_Field *
o2be_structure::add_field(AST_Field *f)
{
  // Check that the CFE operation succeeds. If it returns 0,
  // stop any further work.
  if (AST_Structure::add_field(f) == 0)
    return 0;

  // Now check if the field is of variable size.
  if (isVariable())
    return f;

  AST_Decl *decl  = f->field_type();
  while (decl->node_type() == AST_Decl::NT_typedef)
    decl = o2be_typedef::narrow_from_decl(decl)->base_type();

  switch (decl->node_type())
    {
    case AST_Decl::NT_struct:
      pd_isvar = o2be_structure::narrow_from_decl(decl)->isVariable();
      break;
    case AST_Decl::NT_union:
      pd_isvar = o2be_union::narrow_from_decl(decl)->isVariable();
      break;
    case AST_Decl::NT_string:
    case AST_Decl::NT_sequence:
    case AST_Decl::NT_interface:
      pd_isvar = I_TRUE;
      break;
    case AST_Decl::NT_pre_defined:
      if (o2be_predefined_type::narrow_from_decl(decl)->pt()
	  == AST_PredefinedType::PT_any) 
	{
	  pd_isvar = I_TRUE;
	}
      break;
    case AST_Decl::NT_array:
      pd_isvar = o2be_array::narrow_from_decl(decl)->isVariable();
      break;
    default:
      break;
    }
  return f;
}


void
o2be_structure::produce_hdr(fstream &s)
{
  IND(s); s << "struct " << uqname() << " {\n";
  INC_INDENT_LEVEL();
  {
    // declare any constructor types defined in this scope
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_field)
	  {
	    AST_Decl *decl = AST_Field::narrow_from_decl(d)->field_type();
	    if (decl->has_ancestor(this))
	      {
		switch (decl->node_type())
		  {
		  case AST_Decl::NT_enum:
		    if (!o2be_enum::narrow_from_decl(decl)
			       ->get_hdr_produced_in_field()) 
		      {
			o2be_enum::narrow_from_decl(decl)
			       ->set_hdr_produced_in_field();
			o2be_enum::narrow_from_decl(decl)->produce_hdr(s);
		      }
		    break;
		  case AST_Decl::NT_struct:
		    if (!o2be_structure::narrow_from_decl(decl)
			       ->get_hdr_produced_in_field()) 
		      {
			o2be_structure::narrow_from_decl(decl)
			       ->set_hdr_produced_in_field();
			o2be_structure::narrow_from_decl(decl)->produce_hdr(s);
		      }
		    break;
		  case AST_Decl::NT_union:
		    if (!o2be_union::narrow_from_decl(decl)
			       ->get_hdr_produced_in_field()) 
		      {
			o2be_union::narrow_from_decl(decl)
			       ->set_hdr_produced_in_field();
			o2be_union::narrow_from_decl(decl)->produce_hdr(s);
		      }
		    break;
		  default:
		    break;
		  }
	      }
	  }
	i.next();
      }
  }

  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_field)
	  {
	    AST_Decl *decl = AST_Field::narrow_from_decl(d)->field_type();

	    AST_Decl *tdecl = decl;
	    while (tdecl->node_type() == AST_Decl::NT_typedef)
	      tdecl = o2be_typedef::narrow_from_decl(tdecl)->base_type();

	    IND(s);
	    switch (tdecl->node_type())
	      {
	      case AST_Decl::NT_string:
		{
		  if (decl->node_type() == AST_Decl::NT_string)
		    s << o2be_string::fieldMemberTypeName();
		  else
		    s << o2be_typedef::narrow_from_decl(decl)->fieldMemberType_fqname();
		  s <<" "<< o2be_field::narrow_from_decl(d)->uqname() << ";\n";
		  break;
		}
	      case AST_Decl::NT_interface:
		{
		  if (decl->node_type() == AST_Decl::NT_interface)
		    s << o2be_interface::narrow_from_decl(decl)->fieldMemberType_fqname();
		  else
		    s << o2be_typedef::narrow_from_decl(decl)->fieldMemberType_fqname();
		  s <<" "<< o2be_field::narrow_from_decl(d)->uqname() << ";\n";
		  break;
		}
	      case AST_Decl::NT_array:
		{
		  if (decl->node_type() == AST_Decl::NT_array)
		    o2be_array::narrow_from_decl(decl)->produce_struct_member_decl(s,d);
		  else {
		    s << o2be_typedef::narrow_from_decl(decl)->fqname();
		    s <<" "<< o2be_field::narrow_from_decl(d)->uqname() << ";\n";
		  }
		  break;
		}
#ifdef USE_SEQUENCE_TEMPLATE_IN_PLACE
	      case AST_Decl::NT_sequence:
		{
		  if (decl->node_type() == AST_Decl::NT_sequence) {
		    s << o2be_sequence::narrow_from_decl(decl)->seq_template_name()
		      << " "
		      << o2be_field::narrow_from_decl(d)->uqname()
		      << ";\n";
		  }
		  else {
		    s << o2be_typedef::narrow_from_decl(decl)->fqname();
		    s <<" "<< o2be_field::narrow_from_decl(d)->uqname() << ";\n";
		  }
		  break;
		}
#endif
	      default:
		s << o2be_name::narrow_and_produce_fqname(decl)
		  << " " << o2be_field::narrow_from_decl(d)->uqname() << ";\n";
	      }
	  }
	i.next();
      }
  }

  IND(s); s << "\n";
  IND(s); s << "size_t NP_alignedSize(size_t initialoffset) const;\n";
  IND(s); s << "void operator>>= (NetBufferedStream &) const;\n";
  IND(s); s << "void operator<<= (NetBufferedStream &);\n";
  IND(s); s << "void operator>>= (MemBufferedStream &) const;\n";
  IND(s); s << "void operator<<= (MemBufferedStream &);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";

  IND(s); s << "typedef _CORBA_ConstrType_"
	    << ((isVariable())?"Variable":"Fix")
	    << "_Var<" << uqname() << "> " 
	      << uqname() << "_var;\n\n";

  produce_seq_hdr_if_defined(s);
}

void
o2be_structure::produce_skel(fstream &s)
{
  {
    // declare any constructor types defined in this scope
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_field)
	  {
	    AST_Decl *decl = AST_Field::narrow_from_decl(d)->field_type();
	    if (decl->has_ancestor(this))
	      {
		switch (decl->node_type())
		  {
		  case AST_Decl::NT_enum:
		    if (!o2be_enum::narrow_from_decl(decl)
			       ->get_skel_produced_in_field()) 
		      {
			o2be_enum::narrow_from_decl(decl)
			       ->set_skel_produced_in_field();
			o2be_enum::narrow_from_decl(decl)->produce_skel(s);
		      }
		    break;
		  case AST_Decl::NT_struct:
		    if (!o2be_structure::narrow_from_decl(decl)
			       ->get_skel_produced_in_field()) 
		      {
			o2be_structure::narrow_from_decl(decl)
			       ->set_skel_produced_in_field();
			o2be_structure::narrow_from_decl(decl)->produce_skel(s);
		      }
		    break;
		  case AST_Decl::NT_union:
		    if (!o2be_union::narrow_from_decl(decl)
			       ->get_skel_produced_in_field()) 
		      {
			o2be_union::narrow_from_decl(decl)
			       ->set_skel_produced_in_field();
			o2be_union::narrow_from_decl(decl)->produce_skel(s);
		      }
		    break;
		  default:
		    break;
		  }
	      }
	  }
	i.next();
      }
  }

  IND(s); s << "size_t\n";
  IND(s); s << fqname() << "::NP_alignedSize(size_t _initialoffset) const\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::ULong _msgsize = _initialoffset;\n";
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_field)
	  {
	    o2be_operation::argMapping mapping;
	    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     AST_Field::narrow_from_decl(d)->field_type(),
		     o2be_operation::wIN,mapping);
	    if (ntype == o2be_operation::tString) {
	      ntype = o2be_operation::tStringMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    else if (ntype == o2be_operation::tObjref) {
	      ntype = o2be_operation::tObjrefMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    o2be_operation::produceSizeCalculation(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
		     "",
		     "_msgsize",
		     o2be_field::narrow_from_decl(d)->uqname(),
		     ntype,
		     mapping);
	  }
	i.next();
      }
  }
  IND(s); s << "return _msgsize;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "void\n";
  IND(s); s << fqname() << "::operator>>= (NetBufferedStream &_n) const\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_field)
	  {
	    o2be_operation::argMapping mapping;
	    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     AST_Field::narrow_from_decl(d)->field_type(),
		     o2be_operation::wIN,mapping);
	    if (ntype == o2be_operation::tString) {
	      ntype = o2be_operation::tStringMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    else if (ntype == o2be_operation::tObjref) {
	      ntype = o2be_operation::tObjrefMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    o2be_operation::produceMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
		     "_n",
		     o2be_field::narrow_from_decl(d)->uqname(),
		     ntype,
		     mapping);
	  }
	i.next();
      }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "void\n";
  IND(s); s << fqname() << "::operator<<= (NetBufferedStream &_n)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_field)
	  {
	    o2be_operation::argMapping mapping;
	    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     AST_Field::narrow_from_decl(d)->field_type(),
		     o2be_operation::wIN,mapping);
	    if (ntype == o2be_operation::tString) {
	      ntype = o2be_operation::tStringMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    else if (ntype == o2be_operation::tObjref) {
	      ntype = o2be_operation::tObjrefMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    o2be_operation::produceUnMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
		     "_n",
		     o2be_field::narrow_from_decl(d)->uqname(),
		     ntype,
		     mapping);
	  }
	i.next();
      }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "void\n";
  IND(s); s << fqname() << "::operator>>= (MemBufferedStream &_n) const\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_field)
	  {
	    o2be_operation::argMapping mapping;
	    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     AST_Field::narrow_from_decl(d)->field_type(),
		     o2be_operation::wIN,mapping);
	    if (ntype == o2be_operation::tString) {
	      ntype = o2be_operation::tStringMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    else if (ntype == o2be_operation::tObjref) {
	      ntype = o2be_operation::tObjrefMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    o2be_operation::produceMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
		     "_n",
		     o2be_field::narrow_from_decl(d)->uqname(),
		     ntype,
		     mapping);
	  }
	i.next();
      }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "void\n";
  IND(s); s << fqname() << "::operator<<= (MemBufferedStream &_n)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	if (d->node_type() == AST_Decl::NT_field)
	  {
	    o2be_operation::argMapping mapping;
	    o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     AST_Field::narrow_from_decl(d)->field_type(),
		     o2be_operation::wIN,mapping);
	    if (ntype == o2be_operation::tString) {
	      ntype = o2be_operation::tStringMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    else if (ntype == o2be_operation::tObjref) {
	      ntype = o2be_operation::tObjrefMember;
	      mapping.is_pointer = I_FALSE;
	    }
	    o2be_operation::produceUnMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
		     "_n",
		     o2be_field::narrow_from_decl(d)->uqname(),
		     ntype,
		     mapping,
		     I_TRUE);
	  }
	i.next();
      }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";


}


void
o2be_structure::produce_typedef_hdr(fstream &s, o2be_typedef *tdef)
{
  IND(s); s << "typedef " << fqname() << " " << tdef->uqname() << ";\n";
  IND(s); s << "typedef " << fqname() << "_var " << tdef->uqname() << "_var;\n";
}

IMPL_NARROW_METHODS1(o2be_structure, AST_Structure)
IMPL_NARROW_FROM_DECL(o2be_structure)
IMPL_NARROW_FROM_SCOPE(o2be_structure)
