//                          Package   : omniidl
// o2be_predefined_type.cc  Created on: 9/8/96
//			    Author    : Sai-Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//   omniORB2 BE for the class AST_PredefinedType
//

/*
  $Log$
  Revision 1.1  1997/01/08 17:32:59  sll
  Initial revision

  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

o2be_predefined_type::o2be_predefined_type(AST_PredefinedType::PredefinedType t,
				       UTL_ScopedName *sn, UTL_StrList *p)
		  : AST_PredefinedType(t, sn, p),
		    AST_Decl(AST_Decl::NT_pre_defined, sn, p),
		    o2be_name(this),
		    o2be_sequence_chain(this)
{
  const char *name;

  switch(pt())
    {
    case AST_PredefinedType::PT_long:
      name = "Long";
      break;
    case AST_PredefinedType::PT_ulong:
      name = "ULong";
      break;
    case AST_PredefinedType::PT_short:
      name = "Short";
      break;
    case AST_PredefinedType::PT_ushort:
      name = "UShort";
      break;
    case AST_PredefinedType::PT_float:
      name = "Float";
     break;
    case AST_PredefinedType::PT_double:
      name = "Double";
      break;
    case AST_PredefinedType::PT_char:
      name = "Char";
      break;
    case AST_PredefinedType::PT_boolean:
      name = "Boolean";
      break;
    case AST_PredefinedType::PT_octet:
      name = "Octet";
      break;
    case AST_PredefinedType::PT_any:
      name = "Any";
      break;
    case AST_PredefinedType::PT_void:
      name = "<void>";
      break;
    case AST_PredefinedType::PT_pseudo:
      name = "<pseudo>";
      break;
    case AST_PredefinedType::PT_longlong:
      name = "<longlong>";
      break;
    case AST_PredefinedType::PT_ulonglong:
      name = "<ulonglong>";
      break;
    case AST_PredefinedType::PT_longdouble:
      name = "<longdouble>";
      break;
    case AST_PredefinedType::PT_wchar:
      name = "<wchar>";
      break;
    default:
      name = "<unknown>";
      break;
    }

  char *n = new char [strlen((const char *)"CORBA::")+strlen(name)+1];
  strcpy(n,(const char *)"CORBA::");
  strcat(n,name);
  set_uqname(n);

  n = new char[strlen(uqname())+1];
  strcpy(n,uqname());
  set_fqname(n);

  n = new char[strlen((const char *)"CORBA_")+strlen(name)+1];
  strcpy(n,(const char *)"CORBA_");
  strcat(n,name);
  set__fqname(n);

  n = new char[strlen((const char *)"CORBA")+1];
  strcpy(n,(const char *)"CORBA");
  set_scopename(n);

  n = new char[strlen(scopename())+1];
  strcpy(n,scopename());
  set__scopename(n);
  return;
}

// Narrowing
IMPL_NARROW_METHODS1(o2be_predefined_type, AST_PredefinedType)
IMPL_NARROW_FROM_DECL(o2be_predefined_type)


