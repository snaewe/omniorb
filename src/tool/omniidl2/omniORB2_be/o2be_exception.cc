//                          Package   : omniidl
// o2be_exception.cc        Created on: 9/8/96
//			    Author    : Sai-Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:

/*
  $Log$
  Revision 1.1  1997/01/08 17:32:59  sll
  Initial revision

  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#define IRREPOID_POSTFIX          "_IntfRepoID"

o2be_exception::o2be_exception(UTL_ScopedName *n, UTL_StrList *p)
  : AST_Decl(AST_Decl::NT_except, n, p),
    AST_Structure(AST_Decl::NT_except, n, p),
    UTL_Scope(AST_Decl::NT_except),
    o2be_name(this)
{
  pd_repoid = new char[strlen(_fqname())+strlen(IRREPOID_POSTFIX)+1];
  strcpy(pd_repoid,_fqname());
  strcat(pd_repoid,IRREPOID_POSTFIX);
  pd_repoidsize = strlen(pd_repoid)+1;
}

void
o2be_exception::produce_hdr(fstream &s)
{
  IND(s); s << "#define " << repoIdConstName() <<" \""<< repositoryID() << "\"\n\n";
  IND(s); s << "class " << uqname() << " : public CORBA::UserException {\n";
  IND(s); s << "public:\n\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	AST_Decl *decl = AST_Field::narrow_from_decl(d)->field_type();
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     decl,						   
		     o2be_operation::wIN,mapping);

	IND(s);
	switch (ntype)
	  {
	  case o2be_operation::tString:
	    s << o2be_string::fieldMemberTypeName();
	    break;
	  case o2be_operation::tObjref:
	    {
	      while (decl->node_type() == AST_Decl::NT_typedef)
		decl = o2be_typedef::narrow_from_decl(decl)->base_type();
	      s << o2be_interface::narrow_from_decl(decl)->fieldMemberType_fqname();
	    }
	  break;
	  default:
	    o2be_operation::declareVarType(s,decl);
	  }
	s << " " << o2be_field::narrow_from_decl(d)->uqname() << ";\n";
	i.next();
      }
  }

  IND(s); s << "\n";
  IND(s); s << uqname() << "() {};\n";
  IND(s); s << uqname() << "(const " << uqname() << " &);\n";
  IND(s); s << uqname() << "(";
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	AST_Decl *decl = AST_Field::narrow_from_decl(d)->field_type();
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     decl,						   
		     o2be_operation::wIN,mapping);

	s << ((mapping.is_const) ? "const ":"");
	if (ntype == o2be_operation::tObjref) {
	  s << o2be_interface::narrow_from_decl(decl)->objref_fqname();
	}
	else if (ntype == o2be_operation::tString) {
	  s << "char* ";
	}
	else {
	  s << o2be_name::narrow_and_produce_fqname(decl)
	    << ((mapping.is_arrayslice) ? "_slice":"")
	    << " "
	    << ((mapping.is_pointer)    ? "*":"")
	    << ((mapping.is_reference)  ? "&":"");
	}
	s << " _" << o2be_field::narrow_from_decl(d)->uqname();
	i.next();
	s << ((!i.is_done()) ? ", " : "");
      }
  }
  s << ");\n";
  IND(s); s << uqname() << " & operator=(const " << uqname() << " &);\n";
  IND(s); s << "virtual ~" << uqname() << "() {};\n";
  IND(s); s << "size_t NP_alignedSize(size_t initialoffset);\n";
  IND(s); s << "void operator>>= (NetBufferedStream &);\n";
  IND(s); s << "void operator<<= (NetBufferedStream &);\n";
  IND(s); s << "void operator>>= (MemBufferedStream &);\n";
  IND(s); s << "void operator<<= (MemBufferedStream &);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n";
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
	IND(s); s << d->uqname() << " = _s." << d->uqname() << ";\n";
	i.next();
      }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << fqname() << "::" << uqname()
	    << "(";
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	AST_Decl *decl = AST_Field::narrow_from_decl(d)->field_type();
	o2be_operation::argMapping mapping;
	o2be_operation::argType ntype = o2be_operation::ast2ArgMapping(
		     decl,						   
		     o2be_operation::wIN,mapping);

	s << ((mapping.is_const) ? "const ":"");
	if (ntype == o2be_operation::tObjref) {
	  s << o2be_interface::narrow_from_decl(decl)->objref_fqname();
	}
	else if (ntype == o2be_operation::tString) {
	  s << "char* ";
	}
	else {
	  s << o2be_name::narrow_and_produce_fqname(decl)
	    << ((mapping.is_arrayslice) ? "_slice":"")
	    << " "
	    << ((mapping.is_pointer)    ? "*":"")
	    << ((mapping.is_reference)  ? "&":"");
	}
	s << " _" << o2be_field::narrow_from_decl(d)->uqname();
	i.next();
	s << ((!i.is_done()) ? ", " : "");
      }
  }
  s << ")\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	o2be_field *d = o2be_field::narrow_from_decl(i.item());
	IND(s); s << d->uqname() << " = _" << d->uqname() << ";\n";
	i.next();
      }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << fqname() << " & " << fqname() 
	    << "::operator=(const " << fqname() << " &_s)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	o2be_field *d = o2be_field::narrow_from_decl(i.item());
	IND(s); s << d->uqname() << " = _s." << d->uqname() << ";\n";
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
	o2be_operation::produceSizeCalculation(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
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
	o2be_operation::produceMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
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
	o2be_operation::produceUnMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
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
	o2be_operation::produceMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
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
	o2be_operation::produceUnMarshalCode(
                     s,
		     AST_Field::narrow_from_decl(d)->field_type(),
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

}


IMPL_NARROW_METHODS1(o2be_exception, AST_Exception)
IMPL_NARROW_FROM_DECL(o2be_exception)
IMPL_NARROW_FROM_SCOPE(o2be_exception)

