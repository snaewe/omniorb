//                          Package   : omniidl2
// o2be_exception.cc        Created on: 9/8/96
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

/*
  $Log$
  Revision 1.8  1998/01/27 16:35:52  ewc
   Added support for type any and TypeCode

  Revision 1.7  1997/12/23 19:27:31  sll
  Now generate the correct stub for exception with anonymous array member.

  Revision 1.6  1997/12/09 19:55:20  sll
  *** empty log message ***

  Revision 1.5  1997/08/13 09:23:38  sll
  o2be_exception::repoIdConstLen() now returns the correct length of the
  repository ID. Previously, it wrongly returns the length of the header macro
  name.

// Revision 1.4  1997/05/06  13:54:46  sll
// Public release.
//
  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#define IRREPOID_POSTFIX          "_IntfRepoID"

o2be_exception::o2be_exception(UTL_ScopedName *n, UTL_StrList *p)
  : AST_Decl(AST_Decl::NT_except, n, p),
    AST_Structure(AST_Decl::NT_except, n, p),
    UTL_Scope(AST_Decl::NT_except),
    o2be_name(AST_Decl::NT_except,n,p)
{
  pd_repoid = new char[strlen(_fqname())+strlen(IRREPOID_POSTFIX)+1];
  strcpy(pd_repoid,_fqname());
  strcat(pd_repoid,IRREPOID_POSTFIX);
  pd_repoidsize = strlen(repositoryID())+1;
  set_recursive_seq(I_FALSE);
}

void
o2be_exception::produce_hdr(fstream &s)
{
  s << "#define " << repoIdConstName() <<" \""<< repositoryID() << "\"\n\n";
  IND(s); s << "class " << uqname() << " : public CORBA::UserException {\n";
  IND(s); s << "public:\n\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
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
		s << o2be_typedef::narrow_from_decl(decl)->fieldMemberType_fqname(this);
	      s <<" "<< o2be_field::narrow_from_decl(d)->uqname() << ";\n";
	      break;
	    }
	  case AST_Decl::NT_interface:
	    {
	      if (decl->node_type() == AST_Decl::NT_interface)
		s << o2be_interface::narrow_from_decl(decl)->fieldMemberType_fqname(this);
	      else
		s << o2be_typedef::narrow_from_decl(decl)->fieldMemberType_fqname(this);
	      s <<" "<< o2be_field::narrow_from_decl(d)->uqname() << ";\n";
	      break;
	    }
	  case AST_Decl::NT_pre_defined:
	    {
	      if (AST_PredefinedType::narrow_from_decl(tdecl)->pt()
		  == AST_PredefinedType::PT_TypeCode)
		{
		  s << o2be_predefined_type::TypeCodeMemberName();
		}
	      else
		{
		  s << o2be_name::narrow_and_produce_unambiguous_name(decl,this);
		}		    
	      s << " " << o2be_field::narrow_from_decl(d)->uqname() << ";\n";
	      break;
	    }
	  case AST_Decl::NT_array:
	    {
	      if (decl->node_type() == AST_Decl::NT_array) {
		// Check if this is an anonymous array type, if so
		// generate the supporting typedef.
		if (decl->has_ancestor(this)) {
		  char* fname = o2be_field::narrow_from_decl(d)->uqname();
		  char * tmpname = new char [strlen(fname) + 2];
		  strcpy(tmpname,"_");
		  strcat(tmpname,fname);
		  o2be_array::narrow_from_decl(decl)->produce_typedef_in_union(s,tmpname,this);
		}
		o2be_array::narrow_from_decl(decl)->produce_struct_member_decl(s,d,this);
	      }
	      else {
		s << o2be_typedef::narrow_from_decl(decl)->unambiguous_name(this);
		s <<" "<< o2be_field::narrow_from_decl(d)->uqname() << ";\n";
	      }
	      break;
	    }
#ifdef USE_SEQUENCE_TEMPLATE_IN_PLACE
	  case AST_Decl::NT_sequence:
	    {
	      if (decl->node_type() == AST_Decl::NT_sequence) {
		s << o2be_sequence::narrow_from_decl(decl)->seq_template_name(this)
		  << " "
		  << o2be_field::narrow_from_decl(d)->uqname()
		  << ";\n";
	      }
	      else {
		s << o2be_typedef::narrow_from_decl(decl)->unambiguous_name(this);
		s <<" "<< o2be_field::narrow_from_decl(d)->uqname() << ";\n";
	      }
	      break;
	    }
#endif
	  default:
	    s << o2be_name::narrow_and_produce_unambiguous_name(decl,this)
	      << " " << o2be_field::narrow_from_decl(d)->uqname() << ";\n";
	  }

	i.next();
      }
  }

  IND(s); s << "\n";
  IND(s); s << uqname() << "() {};\n";
  IND(s); s << uqname() << "(const " << uqname() << " &);\n";
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    idl_bool first = I_TRUE;
    while (!i.is_done())
      {
	if (first) {
	  IND(s); s << uqname() << "(";
	  first = I_FALSE;
	}
	AST_Decl *d = i.item();
	AST_Decl *decl = AST_Field::narrow_from_decl(d)->field_type();
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     decl,						   
		     o2be_operation::wIN,mapping);

	s << ((mapping.is_const) ? "const ":"");

	switch (ntype) {
	case o2be_operation::tObjref:
	  s << o2be_interface::narrow_from_decl(decl)->unambiguous_objref_name(this);
	  break;
	case o2be_operation::tString:
	  s << "char* ";
	  break;
	case o2be_operation::tTypeCode:
	  s << "CORBA::TypeCode_ptr ";
	  break;
	case o2be_operation::tArrayFixed:
	case o2be_operation::tArrayVariable:
	  // Check if this is an anonymous array type, if so
	  // use the supporting typedef for the array
	  if (decl->node_type() == AST_Decl::NT_array &&
	      decl->has_ancestor(this))
	    {
	      s << "_0RL_" << o2be_field::narrow_from_decl(d)->uqname();
	    }
	  else
	    {
	      s << o2be_name::narrow_and_produce_unambiguous_name(decl,this);
	    }
	  break;
	case o2be_operation::tSequence:
#ifdef USE_SEQUENCE_TEMPLATE_IN_PLACE
	  if (decl->node_type() == AST_Decl::NT_sequence) {
	    s << o2be_sequence::narrow_from_decl(decl)->seq_template_name(this);
	  }
	  else {
	    s << o2be_name::narrow_and_produce_unambiguous_name(decl,this);
	  }
	  break;
#endif
	default:
	  s << o2be_name::narrow_and_produce_unambiguous_name(decl,this)
	    << ((mapping.is_arrayslice) ? "_slice":"")
	    << " "
	    << ((mapping.is_pointer)    ? "*":"")
	    << ((mapping.is_reference)  ? "&":"");
	  break;
	}
	s << " _" << o2be_field::narrow_from_decl(d)->uqname();
	i.next();
	s << ((!i.is_done()) ? ", " : "");
      }
    if (!first) {
      s << ");\n";
    };
  }
  IND(s); s << uqname() << " & operator=(const " << uqname() << " &);\n";
  IND(s); s << "virtual ~" << uqname() << "() {};\n";
  IND(s); s << "size_t NP_alignedSize(size_t initialoffset);\n";
  IND(s); s << "void operator>>= (NetBufferedStream &);\n";
  IND(s); s << "void operator<<= (NetBufferedStream &);\n";
  IND(s); s << "void operator>>= (MemBufferedStream &);\n";
  IND(s); s << "void operator<<= (MemBufferedStream &);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";

  if (idl_global->compile_flags() & IDL_CF_ANY) {
    if (check_recursive_seq() == I_FALSE) {
      set_recursive_seq(I_FALSE);
      // TypeCode_ptr declaration
      IND(s); s << ((defined_in() == idl_global->root()) ? "extern " : 
		    "static ") 
		<< "const CORBA::TypeCode_ptr " << tcname() << ";\n\n";

    // any insertion operators (inline definitions)
      IND(s); s << (!(defined_in() == idl_global->root()) ? "friend " : "")
		<< "inline void operator<<=(CORBA::Any& _a, const " << uqname() 
		<< "& _s) {\n";
      INC_INDENT_LEVEL();
      IND(s); s << "MemBufferedStream _0RL_mbuf;\n";
      IND(s); s << tcname() << "->NP_fillInit(_0RL_mbuf);\n";
      IND(s); s << uqname() << " _0RL_s = _s;\n";
      IND(s); s << "_0RL_s >>= _0RL_mbuf;\n";
      IND(s); s << "_a.NP_replaceData(" << tcname() << ",_0RL_mbuf);\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n\n";

      IND(s); s << (!(defined_in() == idl_global->root()) ? "friend " : "")
		<< "inline void operator<<=(CORBA::Any& _a, " << uqname() 
		<< "* _sp) {\n";
      INC_INDENT_LEVEL();
      IND(s); s << "::operator<<=(_a,*_sp);\n";
      IND(s); s << "delete _sp;\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n\n";

      // any extraction operator (declaration)
      IND(s); s << (!(defined_in() == idl_global->root()) ? "friend " : "")
		<< "CORBA::Boolean operator>>=(const CORBA::Any& _a, " 
		<< uqname() << "*& _sp);\n\n";
    }
    else set_recursive_seq(I_TRUE);
  }
}

void
o2be_exception::produce_skel(fstream &s)
{
  IND(s); s << fqname() << "::" << uqname()
	    << "(const " << fqname() << " &_s)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	o2be_field *d = o2be_field::narrow_from_decl(i.item());
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype;
	ntype =  o2be_operation::ast2ArgMapping(d->field_type(),
						o2be_operation::wIN,mapping);
	switch (ntype) 
	  {
	  case o2be_operation::tArrayFixed:
	  case o2be_operation::tArrayVariable:
	    {
	      IND(s); s << "{\n";
	      INC_INDENT_LEVEL();
	      AST_Decl* decl = d->field_type();
	      unsigned int ndim = 0;
	      unsigned int dimval;

	      while (decl->node_type() == AST_Decl::NT_typedef)
		decl = o2be_typedef::narrow_from_decl(decl)->base_type();

	      o2be_array::dim_iterator next(o2be_array::narrow_from_decl(decl));
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  dimval = next();
		  IND(s); s << "for (unsigned int _i" << ndim << " =0;"
			    << "_i" << ndim << " < " << dimval << ";"
			    << "_i" << ndim << "++) {\n";
		  INC_INDENT_LEVEL();
		  ndim++;
		}
	      IND(s); s << d->uqname();
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << " = _s." << d->uqname();
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << ";\n";
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  DEC_INDENT_LEVEL();
		  IND(s); s << "}\n";
		  ndim++;
		}
	      DEC_INDENT_LEVEL();
	      IND(s); s << "}\n";
	      break;
	    }
	  
	  default:
	    IND(s); s << d->uqname() << " = _s." << d->uqname() << ";\n";
	    break;
	  }
	i.next();
      }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    idl_bool first = I_TRUE;
    while (!i.is_done())
      {
	if (first) {
	  IND(s); s << fqname() << "::" << uqname() << "(";
	  first = I_FALSE;
	}
	AST_Decl *d = i.item();
	AST_Decl *decl = AST_Field::narrow_from_decl(d)->field_type();
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     decl,						   
		     o2be_operation::wIN,mapping);

	s << ((mapping.is_const) ? "const ":"");
	switch (ntype) {
	case o2be_operation::tObjref:
	  s << o2be_interface::narrow_from_decl(decl)->unambiguous_objref_name(this);
	  break;
	case o2be_operation::tString:
	  s << "char* ";
	  break;
	case o2be_operation::tTypeCode:
	  s << "CORBA::TypeCode_ptr ";
	  break;
	case o2be_operation::tArrayFixed:
	case o2be_operation::tArrayVariable:
	  // Check if this is an anonymous array type, if so
	  // use the supporting typedef for the array
	  if (decl->node_type() == AST_Decl::NT_array &&
	      decl->has_ancestor(this))
	    {
	      s << "_0RL_" << o2be_field::narrow_from_decl(d)->uqname();
	    }
	  else
	    {
	      s << o2be_name::narrow_and_produce_unambiguous_name(decl,this);
	    }
	  break;
	case o2be_operation::tSequence:
#ifdef USE_SEQUENCE_TEMPLATE_IN_PLACE
	  if (decl->node_type() == AST_Decl::NT_sequence) {
	    s << o2be_sequence::narrow_from_decl(decl)->seq_template_name(this);
	  }
	  else {
	    s << o2be_name::narrow_and_produce_unambiguous_name(decl,this);
	  }
	  break;
#endif
	default:
	  s << o2be_name::narrow_and_produce_unambiguous_name(decl,this)
	    << ((mapping.is_arrayslice) ? "_slice":"")
	    << " "
	    << ((mapping.is_pointer)    ? "*":"")
	    << ((mapping.is_reference)  ? "&":"");
	  break;
	}
	s << " _" << o2be_field::narrow_from_decl(d)->uqname();
	i.next();
	s << ((!i.is_done()) ? ", " : "");
      }
    if (!first) {
      s << ")\n";
      IND(s); s << "{\n";
      INC_INDENT_LEVEL();
      {
	UTL_ScopeActiveIterator ii(this,UTL_Scope::IK_decls);
	while (!ii.is_done())
	  {
	    o2be_field *dd = o2be_field::narrow_from_decl(ii.item());
	    o2be_operation::argMapping mapping;
	    o2be_operation::argType ntype;
	    ntype =  o2be_operation::ast2ArgMapping(dd->field_type(),
						    o2be_operation::wIN,mapping);
	    switch (ntype) 
	      {
	      case o2be_operation::tArrayFixed:
	      case o2be_operation::tArrayVariable:
		{
		  IND(s); s << "{\n";
		  INC_INDENT_LEVEL();
		  AST_Decl* decl = dd->field_type();
		  unsigned int ndim = 0;
		  unsigned int dimval;

		  while (decl->node_type() == AST_Decl::NT_typedef)
		    decl = o2be_typedef::narrow_from_decl(decl)->base_type();

		  o2be_array::dim_iterator next(o2be_array::narrow_from_decl(decl));
		  while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		    {
		      dimval = next();
		      IND(s); s << "for (unsigned int _i" << ndim << " =0;"
				<< "_i" << ndim << " < " << dimval << ";"
				<< "_i" << ndim << "++) {\n";
		      INC_INDENT_LEVEL();
		      ndim++;
		    }
		  IND(s); s << dd->uqname();
		  ndim = 0;
		  while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		    {
		      s << "[_i" << ndim << "]";
		      ndim++;
		    }
		  s << " = _" << dd->uqname();
		  ndim = 0;
		  while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		    {
		      s << "[_i" << ndim << "]";
		      ndim++;
		    }
		  s << ";\n";
		  ndim = 0;
		  while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		    {
		      DEC_INDENT_LEVEL();
		      IND(s); s << "}\n";
		      ndim++;
		    }
		  DEC_INDENT_LEVEL();
		  IND(s); s << "}\n";
		  break;
		}
	  
	      default:
		IND(s); s << dd->uqname() << " = _" << dd->uqname() << ";\n";
		break;
	      }
	    ii.next();
	  }
      }
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n\n";
    }
  }

  IND(s); s << fqname() << " & " << fqname() 
	    << "::operator=(const " << fqname() << " &_s)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	o2be_field *d = o2be_field::narrow_from_decl(i.item());
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype;
	ntype =  o2be_operation::ast2ArgMapping(d->field_type(),
						o2be_operation::wIN,mapping);
	switch (ntype) 
	  {
	  case o2be_operation::tArrayFixed:
	  case o2be_operation::tArrayVariable:
	    {
	      IND(s); s << "{\n";
	      INC_INDENT_LEVEL();
	      AST_Decl* decl = d->field_type();
	      unsigned int ndim = 0;
	      unsigned int dimval;

	      while (decl->node_type() == AST_Decl::NT_typedef)
		decl = o2be_typedef::narrow_from_decl(decl)->base_type();

	      o2be_array::dim_iterator next(o2be_array::narrow_from_decl(decl));
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  dimval = next();
		  IND(s); s << "for (unsigned int _i" << ndim << " =0;"
			    << "_i" << ndim << " < " << dimval << ";"
			    << "_i" << ndim << "++) {\n";
		  INC_INDENT_LEVEL();
		  ndim++;
		}
	      IND(s); s << d->uqname();
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << " = _s." << d->uqname();
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  s << "[_i" << ndim << "]";
		  ndim++;
		}
	      s << ";\n";
	      ndim = 0;
	      while (ndim < o2be_array::narrow_from_decl(decl)->getNumOfDims())
		{
		  DEC_INDENT_LEVEL();
		  IND(s); s << "}\n";
		  ndim++;
		}
	      DEC_INDENT_LEVEL();
	      IND(s); s << "}\n";
	      break;
	    }
	  
	  default:
	    IND(s); s << d->uqname() << " = _s." << d->uqname() << ";\n";
	    break;
	  }
	i.next();
      }
  }
  IND(s); s << "return *this;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "size_t\n";
  IND(s); s << fqname() << "::NP_alignedSize(size_t _initialoffset)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "size_t _msgsize = _initialoffset;\n";
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     AST_Field::narrow_from_decl(d)->field_type(),
		     o2be_operation::wIN,mapping);

	if (ntype == o2be_operation::tTypeCode) {
	  ntype = o2be_operation::tTypeCodeMember;
	  mapping.is_pointer = I_FALSE;
	}	      

	o2be_operation::produceSizeCalculation(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
		     ScopeAsDecl(defined_in()),
		     "",
		     "_msgsize",
		     o2be_field::narrow_from_decl(d)->uqname(),
		     ntype,
		     mapping);
	i.next();
      }
  }
  IND(s); s << "return _msgsize;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "void\n";
  IND(s); s << fqname() << "::operator>>= (NetBufferedStream &_n)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     AST_Field::narrow_from_decl(d)->field_type(),
		     o2be_operation::wIN,mapping);

	if (ntype == o2be_operation::tTypeCode) {
	  ntype = o2be_operation::tTypeCodeMember;
	  mapping.is_pointer = I_FALSE;
	}	      

	o2be_operation::produceMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     o2be_field::narrow_from_decl(d)->uqname(),
		     ntype,
		     mapping);
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
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     AST_Field::narrow_from_decl(d)->field_type(),
		     o2be_operation::wIN,mapping);

	if (ntype == o2be_operation::tTypeCode) {
	  ntype = o2be_operation::tTypeCodeMember;
	  mapping.is_pointer = I_FALSE;
	}	      

	o2be_operation::produceUnMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     o2be_field::narrow_from_decl(d)->uqname(),
		     ntype,
		     mapping);
	i.next();
      }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "void\n";
  IND(s); s << fqname() << "::operator>>= (MemBufferedStream &_n)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     AST_Field::narrow_from_decl(d)->field_type(),
		     o2be_operation::wIN,mapping);

	if (ntype == o2be_operation::tTypeCode) {
	  ntype = o2be_operation::tTypeCodeMember;
	  mapping.is_pointer = I_FALSE;
	}	      

	o2be_operation::produceMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     o2be_field::narrow_from_decl(d)->uqname(),
		     ntype,
		     mapping);
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
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     AST_Field::narrow_from_decl(d)->field_type(),
		     o2be_operation::wIN,mapping);

	if (ntype == o2be_operation::tTypeCode) {
	  ntype = o2be_operation::tTypeCodeMember;
	  mapping.is_pointer = I_FALSE;
	}	      

	o2be_operation::produceUnMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
		     ScopeAsDecl(defined_in()),
		     "_n",
		     o2be_field::narrow_from_decl(d)->uqname(),
		     ntype,
		     mapping,
		     I_TRUE);
	i.next();
      }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  if ((idl_global->compile_flags() & IDL_CF_ANY) && 
      recursive_seq() == I_FALSE) {
    // Produce code for types any and TypeCode
    this->produce_typecode_skel(s);

    IND(s); s << "const CORBA::TypeCode_ptr " << fqtcname() << " = & " 
	      << "_01RL_" << _fqtcname() << ";\n\n";
    
    IND(s); s << "void _03RL_" << _fqname() << "_delete(void* _data) {\n";
    INC_INDENT_LEVEL();
    IND(s); s << fqname() << "*" << " _0RL_t = (" << fqname() << "*) _data;\n";
    IND(s); s << "delete _0RL_t;\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n\n";
        
    IND(s); s << "CORBA::Boolean operator>>=(const CORBA::Any& _a, "
	      << fqname() << "*& _sp) {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "CORBA::TypeCode_var _0RL_any_tc = _a.type();\n";
    IND(s); s << "if (!_0RL_any_tc->NP_expandEqual(" << fqtcname() 
	      << ",1)) {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "_sp = 0;\n";
    IND(s); s << "return 0;\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";
    IND(s); s << "else {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "void* _0RL_data = _a.NP_data();\n\n";
    IND(s); s << "if (!_0RL_data) {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "MemBufferedStream _0RL_tmp_mbuf;\n";
    IND(s); s << "_a.NP_getBuffer(_0RL_tmp_mbuf);\n";
    IND(s); s << fqname() << "* _0RL_tmp = new " << fqname() << ";\n";
    IND(s); s << "*_0RL_tmp <<= _0RL_tmp_mbuf;\n";
    IND(s); s << "_0RL_data = (void*) _0RL_tmp;\n";
    IND(s); s << "_a.NP_holdData(_0RL_data,_03RL_" << _fqname() 
	      << "_delete);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n\n";
    IND(s); s << "_sp = (" << fqname() << "*) _0RL_data;\n";
    IND(s); s << "return 1;\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n\n";
  } 
}

void
o2be_exception::produce_typecode_skel(fstream &s)
{
  if (idl_global->compile_flags() & IDL_CF_ANY) {
    s << "#ifndef " << "__01RL_" << _fqtcname() << "__\n";
    s << "#define " << "__01RL_" << _fqtcname() << "__\n\n";

    {
      // Produce static TypeCodes for members that are not
      // defined in this file.
      UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
      while (!i.is_done())
	{
	  AST_Decl *d = i.item();
	  if (d->node_type() == AST_Decl::NT_field)
	    {    
	      AST_Decl *decl = AST_Field::narrow_from_decl(d)->field_type();
	      if (!decl->in_main_file() || 
		  decl->node_type() == AST_Decl::NT_array || 
		  decl->node_type() == AST_Decl::NT_sequence)
		o2be_name::narrow_and_produce_typecode_skel(decl,s);	       
	    }
	  i.next();
	}
    }

    
    unsigned long memberCount = this->nmembers();
    
    if (memberCount > 0)
      {
	IND(s); s << "static CORBA::PR_structMember _02RL_" << _fqtcname()  
		  << "[] = {\n";
    
	INC_INDENT_LEVEL();
	// Produce entries in PR_structMember for struct members
	// (name and TypeCode_ptr)

	UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
	while (!i.is_done())
	  {
	    AST_Decl *d = i.item();
	    if (d->node_type() == AST_Decl::NT_field)
	      {
		IND(s); s << "{\"" << o2be_field::narrow_from_decl(d)->uqname()
			  << "\", ";
		AST_Decl *decl = AST_Field::narrow_from_decl(d)->field_type();
		o2be_name::produce_typecode_member(decl,s);
		s << "}";
		i.next();
		if (!i.is_done()) s << ",\n";    
	      }
	    else i.next();
	  }
	s << " };\n";
	DEC_INDENT_LEVEL();
      }

    IND(s); s << "static CORBA::TypeCode _01RL_" << _fqtcname() 
	      << "(CORBA::tk_except, \"" << repositoryID() << "\", \""
	      << uqname() << "\", ";
    if (memberCount > 0) s << "_02RL_" << _fqtcname() << ", ";
    else s << "(CORBA::PR_structMember*) 0, ";
    s << memberCount << ");\n\n";

    s << "#endif\n\n";
  }   
  return;
}


idl_bool 
o2be_exception::check_recursive_seq()
{
  UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
  while (!i.is_done())
    {
      AST_Decl *d = i.item();
      if (d->node_type() == AST_Decl::NT_field)
	{    
	  AST_Decl *decl = AST_Field::narrow_from_decl(d)->field_type();
	  if (o2be_name::narrow_and_check_recursive_seq(decl) == I_TRUE)
	    return I_TRUE;
	}
      i.next();
    }
  return I_FALSE;
}

IMPL_NARROW_METHODS1(o2be_exception, AST_Exception)
IMPL_NARROW_FROM_DECL(o2be_exception)
IMPL_NARROW_FROM_SCOPE(o2be_exception)

