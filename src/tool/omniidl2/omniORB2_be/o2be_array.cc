// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_array.cc            Created on: 07/10/1996
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
  Revision 1.8  1997/12/23 19:27:51  sll
  Bug fixes.

  Revision 1.7  1997/12/18 17:28:43  sll
  *** empty log message ***

  Revision 1.6  1997/12/09 19:55:17  sll
  *** empty log message ***

// Revision 1.5  1997/05/06  13:49:08  sll
// Public release.
//
  */


#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#define ADPT_CLASS_TEMPLATE "_CORBA_Array_OUT_arg"

o2be_array::o2be_array(UTL_ScopedName *n,
		       unsigned long ndims,
		       UTL_ExprList *dims)
  : AST_Array(n, ndims, dims),
    AST_Decl(AST_Decl::NT_array, n, NULL),
    o2be_name(AST_Decl::NT_array,n,NULL)
{
}

idl_bool
o2be_array::isVariable()
{
  idl_bool isvar;

  AST_Decl *decl = getElementType();
  switch (decl->node_type())
    {
    case AST_Decl::NT_interface:
    case AST_Decl::NT_string:
    case AST_Decl::NT_sequence:
      isvar = I_TRUE;
      break;
    case AST_Decl::NT_union:
      isvar = o2be_union::narrow_from_decl(decl)->isVariable();
      break;
    case AST_Decl::NT_struct:
      isvar = o2be_structure::narrow_from_decl(decl)->isVariable();
      break;
    case AST_Decl::NT_pre_defined:
      if (o2be_predefined_type::narrow_from_decl(decl)->pt() ==
	  AST_PredefinedType::PT_any)
	isvar = I_TRUE;
      else
	isvar = I_FALSE;
      break;
    default:
      isvar = I_FALSE;
    }
  return isvar;
}

size_t
o2be_array::getSliceDim()
{
  AST_Expression **d = dims();
  AST_Expression::AST_ExprValue *v = d[0]->ev();
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
      throw o2be_internal_error(__FILE__,__LINE__,"unexpected type for array dimension");
    }
  return 0;
}

AST_Decl *
o2be_array::getElementType()
{
  AST_Decl *decl = base_type();
  
  while (1) {
    while (decl->node_type() == AST_Decl::NT_typedef)
      decl = o2be_typedef::narrow_from_decl(decl)->base_type();
    if (decl->node_type() == AST_Decl::NT_array) {
      return o2be_array::narrow_from_decl(decl)->getElementType();
    }
    else {
      return decl;
    }
  }
}

size_t
o2be_array::getNumOfElements()
{
  size_t dim = 1;
  AST_Expression **d = dims();
  unsigned long i;
  for (i=0; i < n_dims(); i++)
    {
      AST_Expression::AST_ExprValue *v = d[i]->ev();
      switch (v->et) 
	{
	case AST_Expression::EV_short:
	  dim = dim * v->u.sval;
	  break;
	case AST_Expression::EV_ushort:
	  dim = dim * v->u.usval;
	  break;
	case AST_Expression::EV_long:
	  dim = dim * v->u.lval;
	  break;
	case AST_Expression::EV_ulong:
	  dim = dim * v->u.ulval;
	  break;
	default:
	  throw o2be_internal_error(__FILE__,__LINE__,"unexpected type for array dimension");
	  break;
	}
    }
  AST_Decl *decl = base_type();
  while (1) {
    while (decl->node_type() == AST_Decl::NT_typedef)
      decl = o2be_typedef::narrow_from_decl(decl)->base_type();
    if (decl->node_type() == AST_Decl::NT_array) {
      dim = dim * o2be_array::narrow_from_decl(decl)->getNumOfElements();
      return dim;
    }
    else {
      return dim;
    }
  }
}

size_t
o2be_array::getNumOfDims()
{
  size_t d = n_dims();

  AST_Decl *decl = base_type();

  while (1) {
    while (decl->node_type() == AST_Decl::NT_typedef)
      decl = o2be_typedef::narrow_from_decl(decl)->base_type();
    if (decl->node_type() == AST_Decl::NT_array) {
      d += o2be_array::narrow_from_decl(decl)->getNumOfDims();
      return d;
    }
    else {
      return d;
    }
  }
}

o2be_array::
dim_iterator::dim_iterator(o2be_array *v)
{
  pd_ndim = v->getNumOfDims();
  pd_next = 0;
  pd_dims = new AST_Expression* [pd_ndim];

  int i;
  int ndim;
  AST_Expression **d;

  d = v->dims();
  ndim = v->n_dims();
  for (i=0; i < ndim; i++) {
    pd_dims[pd_next++] = d[i];
  }

  while (pd_next < pd_ndim) {
    AST_Decl *decl = v->base_type();
    while (decl->node_type() == AST_Decl::NT_typedef)
      decl = o2be_typedef::narrow_from_decl(decl)->base_type();
    d = o2be_array::narrow_from_decl(decl)->dims();
    ndim = o2be_array::narrow_from_decl(decl)->n_dims();
    for (i=0; i < ndim; i++) {
      pd_dims[pd_next++] = d[i];
    }
  };
    
  pd_next = 0;
  return;
}

size_t
o2be_array::
dim_iterator::operator() ()
{
  AST_Expression::AST_ExprValue *v;

  if (pd_next < pd_ndim)
    v = pd_dims[pd_next++]->ev();
  else
    return 0;

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
      throw o2be_internal_error(__FILE__,__LINE__,"unexpected type for array dimension");
    }
  return 0;
}

void
o2be_array::produce_hdr (fstream &s, o2be_typedef *tdef)
{
  AST_Decl *decl = base_type();

  const char *elm_fqname;

  switch (decl->node_type())
    {
    case AST_Decl::NT_string:
      elm_fqname = o2be_string::fieldMemberTypeName();
      break;
    case AST_Decl::NT_interface:
      elm_fqname = o2be_interface::narrow_from_decl(decl)->fieldMemberType_fqname(tdef);
      break;
    case AST_Decl::NT_typedef:
      elm_fqname = o2be_typedef::narrow_from_decl(decl)->fieldMemberType_fqname(tdef);
      break;
#if USE_SEQUENCE_TEMPLATE_IN_PLACE
    case AST_Decl::NT_sequence:
      elm_fqname = o2be_sequence::narrow_from_decl(decl)->seq_template_name(tdef);
      break;
#endif
    default:
      elm_fqname = o2be_name::narrow_and_produce_unambiguous_name(decl,tdef);
      break;
    }

  s << "\n";
  IND(s); s << "typedef " << elm_fqname << " "
	    << tdef->uqname();
  {
    int nd = n_dims();
    AST_Expression **d = dims();
    int i;
    for (i=0; i < nd; i++)
      {
	s << "[";
	AST_Expression::AST_ExprValue *v = d[i]->ev();
	switch (v->et) 
	  {
	  case AST_Expression::EV_short:
	    s << v->u.sval;
	    break;
	  case AST_Expression::EV_ushort:
	    s << v->u.usval;
	    break;
	  case AST_Expression::EV_long:
	    s << v->u.lval;
	    break;
	  case AST_Expression::EV_ulong:
	    s << v->u.ulval;
	    break;
	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,"unexpected type in array expression");
	  }
	s << "]";
      }
  }	
  s << ";\n";

  IND(s); s << "typedef " << elm_fqname << " "
	    << tdef->uqname() << "_slice";
  {
    int nd = n_dims();
    AST_Expression **d = dims();
    int i;
    for (i=1; i < nd; i++)
      {
	s << "[";
	AST_Expression::AST_ExprValue *v = d[i]->ev();
	switch (v->et) 
	  {
	  case AST_Expression::EV_short:
	    s << v->u.sval;
	    break;
	  case AST_Expression::EV_ushort:
	    s << v->u.usval;
	    break;
	  case AST_Expression::EV_long:
	    s << v->u.lval;
	    break;
	  case AST_Expression::EV_ulong:
	    s << v->u.ulval;
	    break;
	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,"unexpected type in array expression");
	  }
	s << "]";
      }
  }	
  s << ";\n\n";

  if (defined_in() == idl_global->root())
    {
      // memory management functions are declared as externs in the global
      // scope. Their implementation cannot be generated inline.
      IND(s); s << "extern "
		<< tdef->uqname() << "_slice* "
		<< tdef->uqname() << "_alloc();\n";
      IND(s); s << "extern "
		<< tdef->uqname() << "_slice* "<< tdef->uqname() << "_dup(const "
		<< tdef->uqname() << "_slice* _s);\n";
      IND(s); s << "extern "
		<< "void " << tdef->uqname() << "_free("
		<< tdef->uqname() << "_slice* _s);\n";
    }
  else
    {
      IND(s); s << "static inline "
		<< tdef->uqname() << "_slice* "
		<< tdef->uqname() << "_alloc() {\n";
      INC_INDENT_LEVEL();
      IND(s); s << "return new " << tdef->uqname() << "_slice["
		<< getSliceDim()
		<< "];\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n\n";
      IND(s); s << "static inline "
		<< tdef->uqname() << "_slice* "<< tdef->uqname() << "_dup(const "
		<< tdef->uqname() << "_slice* _s) {\n";
      INC_INDENT_LEVEL();
      IND(s); s << "if (!_s) return 0;\n";
      IND(s); s << tdef->uqname() << "_slice *_data = "
		<< tdef->uqname() << "_alloc();\n";
      IND(s); s << "if (_data) {\n";
      INC_INDENT_LEVEL();
      {
	unsigned int ndim = 0;
	unsigned int dimval;
	o2be_array::dim_iterator next(this);
	while (ndim < getNumOfDims())
	  {
	    dimval = next();
	    IND(s); s << "for (unsigned int _i" << ndim << " =0;"
		      << "_i" << ndim << " < " << dimval << ";"
		      << "_i" << ndim << "++) {\n";
	    INC_INDENT_LEVEL();
	    ndim++;
	  }
	
	IND(s); s << "_data";
	ndim = 0;
	while (ndim < getNumOfDims())
	  {
	    s << "[_i" << ndim << "]";
	    ndim++;
	  }
	s << " = _s";
	ndim = 0;
	while (ndim < getNumOfDims())
	  {
	    s << "[_i" << ndim << "]";
	    ndim++;
	  }
	s << ";\n";
	ndim = 0;
	while (ndim < getNumOfDims())
	  {
	    DEC_INDENT_LEVEL();
	    IND(s); s << "}\n";
	    ndim++;
	  }
      }
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
      IND(s); s << "return _data;\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n\n";
      IND(s); s << "static inline "
		<< "void " << tdef->uqname() << "_free("
		<< tdef->uqname() << "_slice* _s) {\n";
      INC_INDENT_LEVEL();
      IND(s); s << "delete [] _s;\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n\n";
    }

  IND(s); s << "class " << tdef->uqname() << "_copyHelper {\n";
  IND(s); s << "public:\n";
  INC_INDENT_LEVEL();
  IND(s); s << "static inline " << tdef->uqname() << "_slice* alloc() { return "
	    << tdef->uqname() << "_alloc(); }\n";
  IND(s); s << "static inline " << tdef->uqname() << "_slice* dup(const "
	    << tdef->uqname() << "_slice* p) { return "
	    << tdef->uqname() << "_dup(p); }\n";
  IND(s); s << "static inline void free("
	    << tdef->uqname() << "_slice* p) { "
	    << tdef->uqname() << "_free(p); }\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";

  IND(s); s << "typedef _CORBA_Array_Var<" 
	    << tdef->uqname() << "_copyHelper,"
	    << tdef->uqname() << "_slice> "
	    << tdef->uqname() << "_var;\n";
  IND(s); s << "typedef _CORBA_Array_Forany<"
	    << tdef->uqname() << "_copyHelper,"
	    << tdef->uqname() << "_slice> "
	    << tdef->uqname() << "_forany;\n\n";
}

void
o2be_array::produce_skel (fstream &s, o2be_typedef *tdef)
{
  if (defined_in() == idl_global->root())
    {
      // memory management functions are declared as externs in the global
      // scope. Generate their implemenation here.
      IND(s); s << "extern "
		<< tdef->uqname() << "_slice* "
		<< tdef->uqname() << "_alloc() {\n";
      INC_INDENT_LEVEL();
      IND(s); s << "return new " << tdef->uqname() << "_slice["
		<< getSliceDim()
		<< "];\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n\n";
      IND(s); s << "extern "
		<< tdef->uqname() << "_slice* "<< tdef->uqname() << "_dup(const "
		<< tdef->uqname() << "_slice* _s) {\n";
      INC_INDENT_LEVEL();
      IND(s); s << "if (!_s) return 0;\n";
      IND(s); s << tdef->uqname() << "_slice *_data = "
		<< tdef->uqname() << "_alloc();\n";
      IND(s); s << "if (_data) {\n";
      INC_INDENT_LEVEL();
      {
	unsigned int ndim = 0;
	unsigned int dimval;
	o2be_array::dim_iterator next(this);
	while (ndim < getNumOfDims())
	  {
	    dimval = next();
	    IND(s); s << "for (unsigned int _i" << ndim << " =0;"
		      << "_i" << ndim << " < " << dimval << ";"
		      << "_i" << ndim << "++) {\n";
	    INC_INDENT_LEVEL();
	    ndim++;
	  }
	
	IND(s); s << "_data";
	ndim = 0;
	while (ndim < getNumOfDims())
	  {
	    s << "[_i" << ndim << "]";
	    ndim++;
	  }
	s << " = _s";
	ndim = 0;
	while (ndim < getNumOfDims())
	  {
	    s << "[_i" << ndim << "]";
	    ndim++;
	  }
	s << ";\n";
	ndim = 0;
	while (ndim < getNumOfDims())
	  {
	    DEC_INDENT_LEVEL();
	    IND(s); s << "}\n";
	    ndim++;
	  }
      }
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
      IND(s); s << "return _data;\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n\n";
      IND(s); s << "extern "
		<< "void " << tdef->uqname() << "_free("
		<< tdef->uqname() << "_slice* _s) {\n";
      INC_INDENT_LEVEL();
      IND(s); s << "delete [] _s;\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n\n";
    }
}

void
o2be_array::produce_typedef_hdr(fstream &s, o2be_typedef *tdef1,
				o2be_typedef *tdef2)
{
  IND(s); s << "typedef " << tdef2->unambiguous_name(tdef1) 
	    << " " << tdef1->uqname() << ";\n";
  IND(s); s << "typedef " << tdef2->unambiguous_name(tdef1) 
	    << "_slice " << tdef1->uqname() 
	    << "_slice;\n";
  IND(s); s << "typedef " << tdef2->unambiguous_name(tdef1) 
	    << "_copyHelper " << tdef1->uqname() << "_copyHelper;\n";
  IND(s); s << "typedef " << tdef2->unambiguous_name(tdef1) 
	    << "_var " << tdef1->uqname() << "_var;\n";
  IND(s); s << "typedef " << tdef2->unambiguous_name(tdef1) 
	    << "_forany " << tdef1->uqname() << "_forany;\n";
  IND(s); s << ((!(tdef1->defined_in()==idl_global->root()))?"static ":"extern ")
	    << tdef1->uqname() << "_slice* "<< tdef1->uqname() << "_alloc() ";
  if (!(tdef1->defined_in()==idl_global->root())) {
    s << "{ return " << tdef2->unambiguous_name(tdef1) << "_alloc(); }\n";
  }
  else {
    s << ";\n";
  }
  IND(s); s << ((!(tdef1->defined_in()==idl_global->root()))?"static ":"extern ")
	    << tdef1->uqname() << "_slice* "<< tdef1->uqname() << "_dup(const "
	    << tdef1->uqname() << "_slice* p) ";
  if (!(tdef1->defined_in()==idl_global->root())) {
    s << "{ return " << tdef2->unambiguous_name(tdef1) 
      << "_dup(p); }\n";
  }
  else {
    s << ";\n";
  }
  IND(s); s << ((!(tdef1->defined_in()==idl_global->root()))?"static ":"extern ")
	    << "void " << tdef1->uqname() << "_free( "
	    << tdef1->uqname() << "_slice* p) ";
  if (!(tdef1->defined_in()==idl_global->root())) {
    s << "{ " << tdef2->unambiguous_name(tdef1) 
      << "_free(p); }\n\n";
  }
  else {
    s << ";\n\n";
  }
}

void
o2be_array::produce_typedef_skel(fstream &s, o2be_typedef *tdef1,
				 o2be_typedef *tdef2)
{
  if (tdef1->defined_in() == idl_global->root()) {
    IND(s); s << "extern " << tdef1->fqname() << "_slice* " 
	      << tdef1->fqname() << "_alloc() {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "return " << tdef2->fqname() << "_alloc();\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n\n";

    IND(s); s << "extern " << tdef1->fqname() << "_slice* " 
	      << tdef1->fqname() << "_dup(const " << tdef1->fqname()
	      << "_slice* p) {\n";
    INC_INDENT_LEVEL();
    IND(s); s << "return " << tdef2->fqname() << "_dup(p);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n\n";

    IND(s); s << "extern void " << tdef1->fqname() << "_free( " 
	      << tdef1->fqname() << "_slice* p) {\n";
    INC_INDENT_LEVEL();
    IND(s); s << tdef2->fqname() << "_free(p);\n";
    DEC_INDENT_LEVEL();
    IND(s); s << "}\n\n";
  }
}

void
o2be_array::_produce_member_decl (fstream &s, char *varname,AST_Decl* used_in)
{
  AST_Decl *decl = base_type();

  const char *elm_fqname;

  switch (decl->node_type())
    {
    case AST_Decl::NT_string:
      elm_fqname = o2be_string::fieldMemberTypeName();
      break;
    case AST_Decl::NT_interface:
      elm_fqname = o2be_interface::narrow_from_decl(decl)->fieldMemberType_fqname(used_in);
      break;
    case AST_Decl::NT_typedef:
      elm_fqname = o2be_typedef::narrow_from_decl(decl)->fieldMemberType_fqname(used_in);
      break;
#if USE_SEQUENCE_TEMPLATE_IN_PLACE
    case AST_Decl::NT_sequence:
      elm_fqname = o2be_sequence::narrow_from_decl(decl)->seq_template_name(used_in);
      break;
#endif
    default:
      elm_fqname = o2be_name::narrow_and_produce_unambiguous_name(decl,
								  used_in);
      break;
    }

  IND(s); s << elm_fqname << " " << varname;
  {
    int nd = n_dims();
    AST_Expression **d = dims();
    int i;
    for (i=0; i < nd; i++)
      {
	s << "[";
	AST_Expression::AST_ExprValue *v = d[i]->ev();
	switch (v->et) 
	  {
	  case AST_Expression::EV_short:
	    s << v->u.sval;
	    break;
	  case AST_Expression::EV_ushort:
	    s << v->u.usval;
	    break;
	  case AST_Expression::EV_long:
	    s << v->u.lval;
	    break;
	  case AST_Expression::EV_ulong:
	    s << v->u.ulval;
	    break;
	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,"unexpected type in array expression");
	  }
	s << "]";
      }
  }	
  s << ";\n";
}

void
o2be_array::produce_struct_member_decl (fstream &s, AST_Decl *fieldtype,
					AST_Decl* used_in)
{
  _produce_member_decl(s,o2be_name::narrow_and_produce_uqname(fieldtype),
		       used_in);
}

void
o2be_array::produce_union_member_decl (fstream &s, AST_Decl *fieldtype,
				       AST_Decl* used_in)
{
  char * varname = new char[strlen("pd_")+strlen(o2be_name::narrow_and_produce_uqname(fieldtype))+1];
  strcpy(varname,"pd_");
  strcat(varname,o2be_name::narrow_and_produce_uqname(fieldtype));
  _produce_member_decl(s,varname,used_in);
  delete [] varname;
}

void
o2be_array::produce_typedef_in_union(fstream &s, const char *tname,
				     AST_Decl* used_in)
{
  AST_Decl *decl = base_type();

  const char *elm_fqname;

  switch (decl->node_type())
    {
    case AST_Decl::NT_string:
      elm_fqname = o2be_string::fieldMemberTypeName();
      break;
    case AST_Decl::NT_interface:
      elm_fqname = o2be_interface::narrow_from_decl(decl)->fieldMemberType_fqname(used_in);
      break;
    case AST_Decl::NT_typedef:
      elm_fqname = o2be_typedef::narrow_from_decl(decl)->fieldMemberType_fqname(used_in);
      break;
#if USE_SEQUENCE_TEMPLATE_IN_PLACE
    case AST_Decl::NT_sequence:
      elm_fqname = o2be_sequence::narrow_from_decl(decl)->seq_template_name(used_in);
      break;
#endif
    default:
      elm_fqname = o2be_name::narrow_and_produce_unambiguous_name(decl,
								  used_in);
      break;
    }

  IND(s); s << "typedef " << elm_fqname << " _0RL" << tname;
  {
    int nd = n_dims();
    AST_Expression **d = dims();
    int i;
    for (i=0; i < nd; i++)
      {
	s << "[";
	AST_Expression::AST_ExprValue *v = d[i]->ev();
	switch (v->et) 
	  {
	  case AST_Expression::EV_short:
	    s << v->u.sval;
	    break;
	  case AST_Expression::EV_ushort:
	    s << v->u.usval;
	    break;
	  case AST_Expression::EV_long:
	    s << v->u.lval;
	    break;
	  case AST_Expression::EV_ulong:
	    s << v->u.ulval;
	    break;
	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,"unexpected type in array expression");
	  }
	s << "]";
      }
  }	
  s << ";\n";


  IND(s); s << "typedef " << elm_fqname << " "
	    << tname << "_slice";
  {
    int nd = n_dims();
    AST_Expression **d = dims();
    int i;
    for (i=1; i < nd; i++)
      {
	s << "[";
	AST_Expression::AST_ExprValue *v = d[i]->ev();
	switch (v->et) 
	  {
	  case AST_Expression::EV_short:
	    s << v->u.sval;
	    break;
	  case AST_Expression::EV_ushort:
	    s << v->u.usval;
	    break;
	  case AST_Expression::EV_long:
	    s << v->u.lval;
	    break;
	  case AST_Expression::EV_ulong:
	    s << v->u.ulval;
	    break;
	  default:
	    throw o2be_internal_error(__FILE__,__LINE__,"unexpected type in array expression");
	  }
	s << "]";
      }
  }	
  s << ";\n\n";
}

const char*
o2be_array::out_adptarg_name(o2be_typedef* tdef,AST_Decl* used_in) const
{
  const char* ubname;

  if (o2be_global::qflag()) {
    ubname = tdef->fqname();
  }
  else {
    ubname = tdef->unambiguous_name(used_in);
  }

  char* p = new char[strlen(ADPT_CLASS_TEMPLATE)+strlen("<, >")+
		     strlen(ubname) + strlen("_slice") +
		     strlen(ubname) + strlen("_var")+1];
  strcpy(p,ADPT_CLASS_TEMPLATE);
  strcat(p,"<");
  strcat(p,ubname);
  strcat(p,"_slice");
  strcat(p,",");
  strcat(p,ubname);
  strcat(p,"_var >");
  return p;  
}


IMPL_NARROW_METHODS1(o2be_array, AST_Array)
IMPL_NARROW_FROM_DECL(o2be_array)
