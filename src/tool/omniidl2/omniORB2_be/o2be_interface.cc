// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_interface.cc        Created on: 12/08/1996
//			    Author    : Sai-Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
  Revision 1.39.6.11  1999/10/21 11:05:00  djr
  Added _core_attr to declarations of _PD_repoId in interfaces.

  Revision 1.39.6.10  1999/10/18 17:27:01  djr
  Work-around for MSVC scoping bug.

  Revision 1.39.6.9  1999/10/16 13:22:57  djr
  Changes to support compiling on MSVC.

  Revision 1.39.6.8  1999/10/13 15:18:01  djr
  Fixed problem with call descriptors shared between ops and attrs.

  Revision 1.39.6.7  1999/10/04 17:08:35  djr
  Some more fixes/MSVC work-arounds.

  Revision 1.39.6.6  1999/10/04 15:51:52  djr
  Various fixes/MSVC work-arounds.

  Revision 1.39.6.5  1999/09/29 13:51:03  djr
  Corrected tie implementation signatures.

  Revision 1.39.6.4  1999/09/29 13:12:03  djr
  Updated mapping of operation signatures for skeletons to use T_out types.
  Renamed all flags relating to backwards-compatiblity.

  Revision 1.39.6.3  1999/09/27 11:41:27  djr
  Generate old BOA-style tie templates.

  Revision 1.39.6.2  1999/09/24 15:35:13  djr
  Removed 'else if' nesting in ::_dispatch routine in stubs.  Makes life
  easier for brain-dead compilers.

  Revision 1.39.6.1  1999/09/24 10:05:24  djr
  Updated for omniORB3.

  Revision 1.37  1999/06/25 13:52:25  sll
  Updated any extraction operator to non-copy semantics but can be override
  by the value of omniORB_27_CompatibleAnyExtraction.

  Revision 1.36  1999/06/22 14:54:27  sll
  Changed stub code for any insertion and extraction. No longer keep a
  reference to objref member class in the tcdescriptor.

  Revision 1.35  1999/06/18 20:44:57  sll
  Updated to support new sequence object reference template.

  Revision 1.34  1999/06/03 17:11:06  sll
  Updated to CORBA 2.2

  Revision 1.33  1999/06/02 16:44:32  sll
  If module name is CORBA, use omniObjectKey instead of omniORB::objectKey.

  Revision 1.32  1999/05/26 10:43:35  sll
  Added connection calls to allow the generation of typecode constant for
  bounded string defined in the operation or attribute signature.

  Revision 1.31  1999/04/15 14:12:05  djr
  Fixed bug w TIE templates (wrong when using diamond shaped multiple
  inheritance.

  Revision 1.30  1999/03/11 16:26:13  djr
  Updated copyright notice

  Revision 1.29  1999/02/19 11:31:42  djr
  Fixed implementation of Any insertion/extraction code.

  Revision 1.28  1999/02/10 09:56:09  djr
  Fixed bug in which omniidl2 failed if constructed types were
  declared in an exception member declaration.

  Revision 1.27  1999/02/09 09:40:35  djr
  produce_decls_at_global_scope_in_hdr() now propagates properly
  through (nested) interfaces to structure and union types.

  Revision 1.26  1999/01/07 09:47:04  djr
  Changes to support new TypeCode/Any implementation, which is now
  placed in a new file ...DynSK.cc (by default).

  Revision 1.25  1998/08/26 18:28:44  sll
  Complete the previous fix for the LifeCycle support.

  Revision 1.24  1998/08/25 16:33:23  sll
  Workaround in the generated stub for a MSVC++ bug.
  Bug fix for the LifeCycle support.

  Revision 1.23  1998/08/19 15:52:44  sll
  New member functions void produce_binary_operators_in_hdr and the like
  are responsible for generating binary operators <<= etc in the global
  namespace.

  Revision 1.22  1998/08/13 22:43:16  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.21  1998/07/08 13:42:16  dpg1
  Fixed bug with interface inheritance in LifeCycle code generation.

  Revision 1.20  1998/05/20 18:23:58  sll
  New option (-t) enable the generation of tie implementation template.

  Revision 1.19  1998/04/07 18:48:03  sll
  Use std::fstream instead of fstream.
  Stub code modified to accommodate the use of namespace to represent module.

// Revision 1.18  1998/03/09  14:24:33  ewc
// Minor change - cast string literals
//
// Revision 1.17  1998/01/27  16:45:14  ewc
// Added support for type Any and TypeCode
//
  Revision 1.16  1997/12/23 19:26:13  sll
  Now generate correct typedefs for typedef interfaces.

  Revision 1.15  1997/12/10 11:35:41  sll
  Updated life cycle service stub.

  Revision 1.14  1997/12/09 19:54:07  sll
  *** empty log message ***

// Revision 1.13  1997/09/20  16:44:22  dpg1
// Added new is_cxx_type argument to _widenFromTheMostDerivedIntf().
// Added LifeCycle code generation.
//
  Revision 1.12  1997/08/27 17:54:31  sll
  Added _var typedef for IDL typedef Object.

  Revision 1.11  1997/08/22 12:43:23  sll
  Oh well, gcc does not like variable names starting with __, changed
  the prefix to _0RL_.

  Revision 1.10  1997/08/21 21:20:08  sll
  - Names of internal variables inside the stub code now all start with the
    prefix __ to avoid potential clash with identifiers defined in IDL.
  - fixed bug in the stub code generated for typedef Object X.

// Revision 1.9  1997/05/06  13:58:53  sll
// Public release.
//
  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <o2be_util.h>


#define PROXY_CLASS_PREFIX        "_objref_"
#define POF_CLASS_PREFIX          "_pof_"
#define IMPL_CLASS_PREFIX         "_impl_"
#define POA_SKELETON_PREFIX       "POA_"
#define BOA_SKELETON_PREFIX       "_sk_"
#define POA_TIE_CLASS_POSTFIX     "_tie"
#define BOA_TIE_CLASS_PREFIX      "_tie_"
#define HELPER_CLASS_POSTFIX      "_Helper"

#define FIELD_MEMBER_TEMPLATE     "_CORBA_ObjRef_Member"
#define SEQ_MEMBER_TEMPLATE       "_CORBA_ObjRef_Member"
#define ADPT_INOUT_CLASS_TEMPLATE "_CORBA_ObjRef_INOUT_arg"
#define ADPT_OUT_CLASS_TEMPLATE   "_CORBA_ObjRef_OUT_arg"



static void internal_produce_tie_templates(std::fstream& s,
					   o2be_interface* intf,
					   const char* class_name,
					   const char* skel_name);



o2be_interface::o2be_interface(UTL_ScopedName* n, AST_Interface** ih,
			       long nih, UTL_StrList* p)
            : AST_Interface(n, ih, nih, p),
	      AST_Decl(AST_Decl::NT_interface, n, p),
	      UTL_Scope(AST_Decl::NT_interface),
	      o2be_name(AST_Decl::NT_interface,n,p),
	      o2be_sequence_chain(AST_Decl::NT_interface,n,p)
{
  if (strcmp(fqname(),"Object") == 0)
    {
      // This node is for the psuedo object "Object" and should be mapped
      // to CORBA::Object.
      // Set the names to properly scoped under "CORBA::".
      set_uqname("CORBA::Object");
      set_fqname("CORBA::Object");
      set__fqname("CORBA_Object");
      set_scopename("");
      set__scopename("");

      set_tcname("CORBA::_tc_Object");
      set_fqtcname("CORBA::_tc_Object");
      set__fqtcname("CORBA__tc_Object");
      set__idname("CORBA_mObject");

      pd_objref_uqname = (char*) "CORBA::Object_ptr";
      pd_objref_fqname = (char*) "CORBA::Object_ptr";
      pd_proxy_uqname = (char*) "CORBA::Object";
      pd_proxy_fqname = (char*) "CORBA::Object";
      pd_server_uqname = 0;
      pd_server_fqname = 0;
      pd_fieldmem_uqname = (char*) "CORBA::Object_member";
      pd_fieldmem_fqname = (char*) "CORBA::Object_member";
      pd_seqmem_fqname = (char*) "CORBA::Object_member";
      pd_inout_adptarg_name = (char*) "CORBA::Object_INOUT_arg";
      pd_out_adptarg_name = (char*) "CORBA::Object_OUT_arg";
      return;
    }

  pd_objref_uqname = new char[strlen(uqname())+strlen("_ptr")+1];
  strcpy(pd_objref_uqname, uqname());
  strcat(pd_objref_uqname, "_ptr");

  pd_objref_fqname = new char[strlen(fqname())+strlen("_ptr")+1];
  strcpy(pd_objref_fqname, fqname());
  strcat(pd_objref_fqname, "_ptr");

  pd_proxy_uqname = new char[strlen(PROXY_CLASS_PREFIX) +
			    strlen(uqname()) + 1];
  strcpy(pd_proxy_uqname, PROXY_CLASS_PREFIX);
  strcat(pd_proxy_uqname, uqname());

  pd_server_uqname = new char[strlen(IMPL_CLASS_PREFIX) +
			     strlen(uqname()) + 1];
  strcpy(pd_server_uqname, IMPL_CLASS_PREFIX);
  strcat(pd_server_uqname, uqname());

  if( strlen(scopename()) ) {
    pd_proxy_fqname = new char[strlen(scopename())+
			      strlen(pd_proxy_uqname)+1];
    strcpy(pd_proxy_fqname, scopename());
    strcat(pd_proxy_fqname, pd_proxy_uqname);

    pd_server_fqname = new char[strlen(scopename())+
			       strlen(pd_server_uqname)+1];
    strcpy(pd_server_fqname, scopename());
    strcat(pd_server_fqname, pd_server_uqname);
  }
  else {
    pd_proxy_fqname = pd_proxy_uqname;
    pd_server_fqname = pd_server_uqname;
  }

  {
    StringBuf t;
    t += FIELD_MEMBER_TEMPLATE;
    t += '<';
    t += proxy_uqname();
    t += ", ";
    t += uqname();
    t += HELPER_CLASS_POSTFIX;
    t += '>';
    pd_fieldmem_uqname = t.release();
  }

  pd_fieldmem_fqname = 0;
  pd_seqmem_fqname = 0;

  pd_inout_adptarg_name = 0;
  pd_out_adptarg_name = 0;
  pd_have_produced_buildDesc_decls = 0;
}


o2be_interface_fwd::o2be_interface_fwd(UTL_ScopedName* n, UTL_StrList* p)
  : AST_InterfaceFwd(n, p),
    AST_Decl(AST_Decl::NT_interface_fwd, n, p),
    o2be_name(AST_Decl::NT_interface_fwd,n,p)
{
  pd_have_produced_buildDesc_decls = 0;
}


idl_bool
o2be_interface::check_opname_clash(o2be_interface* p, char* opname)
{
  {
    UTL_ScopeActiveIterator i(p,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	switch(d->node_type()) {
	case AST_Decl::NT_op:
	  if (strcmp(opname,o2be_operation::narrow_from_decl(d)->uqname())==0)
	    return I_TRUE;
	  break;
	case AST_Decl::NT_attr:
	  {
	    char *attrop = new char[strlen(o2be_attribute::narrow_from_decl(d)->uqname())+6];
	    strcpy(attrop,"_get_");
	    strcat(attrop,o2be_attribute::narrow_from_decl(d)->uqname());
	    if (strcmp(opname,attrop)==0)
	      {
		delete [] attrop;
		return I_TRUE;
	      }
	    strcpy(attrop,"_set_");
	    strcat(attrop,o2be_attribute::narrow_from_decl(d)->uqname());
	    if (strcmp(opname,attrop)==0)
	      {
		delete [] attrop;
		return I_TRUE;
	      }
	    delete [] attrop;
	  }
	  break;
	default:
	  break;
	}
	i.next();
      }
  }
  {
    int ni,j;
    AST_Interface **intftable;
    if ((ni = p->n_inherits()) != 0)
      {
	intftable = p->inherits();
	for (j=0; j< ni; j++)
	  {
	    o2be_interface * intf = o2be_interface::narrow_from_decl(intftable[j]);
	    if (o2be_interface::check_opname_clash(intf,opname))
	      return I_TRUE;
	  }
      }
  }
  return I_FALSE;
}


static inline
const char*
objectkeytype(o2be_interface* p) {
  if (strcmp(p->scopename(),"CORBA::") == 0) {
    // If this interface is defined within module CORBA, use omniObjectKey
    // because omniORB.h might not be included when the header is included.
    return "omniObjectKey";
  }
  else
    return "omniORB::objectKey";
}


void 
o2be_interface::produce_hdr(std::fstream& s)
{
  produce_helper_hdr(s);

  o2be_module* module = o2be_module::narrow_from_scope(defined_in());

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////////// foo ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  SimpleStringMap map;
  map.insert("foo", uqname());

  s << o2be_template(map,
   "class foo {\n"
   "public:\n"
   "  // Declarations for this interface type.\n"
   "  typedef foo@_ptr _ptr_type;\n"
   "  typedef foo@_var _var_type;\n\n"

   "  static _ptr_type _duplicate(_ptr_type);\n"
   "  static _ptr_type _narrow(CORBA::Object_ptr);\n"
   "  static _ptr_type _nil();\n\n"

   "  static inline size_t _alignedSize(_ptr_type, size_t);\n"
   "  static inline void _marshalObjRef(_ptr_type, NetBufferedStream&);\n"
   "  static inline void _marshalObjRef(_ptr_type, MemBufferedStream&);\n\n"

   "  static inline _ptr_type _unmarshalObjRef(NetBufferedStream& s) {\n"
   "    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);\n"
   "    _ptr_type result = _narrow(obj);\n"
   "    CORBA::release(obj);\n"
   "    return result;\n"
   "  }\n\n"

   "  static inline _ptr_type _unmarshalObjRef(MemBufferedStream& s) {\n"
   "    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);\n"
   "    _ptr_type result = _narrow(obj);\n"
   "    CORBA::release(obj);\n"
   "    return result;\n"
   "  }\n\n"

   "  static _core_attr const char* _PD_repoId;\n\n"

   "  // Other IDL defined within this scope.\n\n"
  );
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	switch(d->node_type()) {
	case AST_Decl::NT_op:
	case AST_Decl::NT_attr:
	case AST_Decl::NT_enum_val:
	  break;
	case AST_Decl::NT_const:
	  o2be_constant::narrow_from_decl(d)->produce_hdr(s);
	  break;
	case AST_Decl::NT_enum:
	  o2be_enum::narrow_from_decl(d)->produce_hdr(s);
	  break;
	case AST_Decl::NT_except:
	  o2be_exception::narrow_from_decl(d)->produce_hdr(s);
	  break;
	case AST_Decl::NT_struct:
	  o2be_structure::narrow_from_decl(d)->produce_hdr(s);
	  break;
	case AST_Decl::NT_typedef:
	  o2be_typedef::narrow_from_decl(d)->produce_hdr(s);
	  break;
	case AST_Decl::NT_union:
	  o2be_union::narrow_from_decl(d)->produce_hdr(s);
	  break;
	default:
	  throw o2be_internal_error(__FILE__, __LINE__,
				    "unexpected type under interface class");
	}
	i.next();
      }
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "};\n\n\n";

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// _objref_foo ////////////////////////////
  //////////////////////////////////////////////////////////////////////

  map.insert("proxy", proxy_uqname());

  s << o2be_template(map, "class proxy :\n");
  if( n_inherits() == 0 ) {
    IND(s); s << "  public virtual CORBA::Object, public virtual omniObjRef\n";
  }
  else {
    AST_Interface **intftable = inherits();
    int ni = n_inherits();
    for( int j = 0; j < ni; j++ ) {
      o2be_interface* intf = o2be_interface::narrow_from_decl(intftable[j]);
      s << "  public virtual " << intf->unambiguous_proxy_name(this);
      if( j + 1 < ni )  s << ",\n";
      else              s << "\n";
    }
  }
  s << o2be_verbatim("{\n"
		     "public:\n");
  INC_INDENT_LEVEL();
  {
    o2be_iterator<o2be_operation, AST_Decl::NT_op> i(this);
    while( !i.is_done() ) {
      IND(s); i.item()->produce_client_decl(s, module, 1, 1);
      s << ";\n";
      i.next();
    }
  }
  {
    o2be_iterator<o2be_attribute, AST_Decl::NT_attr> i(this);
    while( !i.is_done() ) {
      o2be_attribute* a = i.item();
      IND(s); a->produce_decl_rd(s, module);
      s << ' ' << a->uqname() << "();\n";
      if( !a->readonly() ) {
	IND(s); s << "void " << a->uqname() << '(';
	a->produce_decl_wr(s, module);
	s << ");\n";
      }
      i.next();
    }
  }
  s << "\n";
  DEC_INDENT_LEVEL();

  s << o2be_template(map,
   "  inline proxy() { _PR_setobj(0); }  // nil\n"
   "  proxy(const char*, IOP::TaggedProfileList*, omniIdentity*, "
           "omniLocalIdentity*);\n\n"

   "protected:\n"
   "  virtual ~proxy();\n\n"

   "private:\n"
   "  virtual void* _ptrToObjRef(const char*);\n\n"

   "  proxy(const proxy&);\n"
   "  proxy& operator = (const proxy&);\n"
   "  // not implemented\n"
   "};\n\n\n"
  );

  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// _pof_foo //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  StringBuf pof;
  pof += POF_CLASS_PREFIX;
  pof += uqname();
  map.insert("pof", pof);

  s << o2be_template(map,
   "class pof : public proxyObjectFactory {\n"
   "public:\n"
   "  inline pof() : proxyObjectFactory(foo::_PD_repoId) {}\n"
   "  virtual ~pof();\n\n"

   "  virtual omniObjRef* newObjRef(const char*, IOP::TaggedProfileList*,\n"
   "                                omniIdentity*, omniLocalIdentity*);\n"
   "  virtual _CORBA_Boolean is_a(const char*) const;\n"
   "};\n\n\n"
  );

  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// _impl_foo /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  StringBuf _impl_foo;
  _impl_foo += IMPL_CLASS_PREFIX;
  _impl_foo += uqname();
  map.insert("_impl_foo", _impl_foo);

  s << o2be_template(map, "class _impl_foo :\n");
  if( n_inherits() == 0 ) {
    s << o2be_verbatim("  public virtual omniServant\n");
  }
  else {
    AST_Interface** intftable = inherits();
    int ni = n_inherits();
    for( int j = 0; j < ni; j++ ) {
      o2be_interface* intf = o2be_interface::narrow_from_decl(intftable[j]);
      s << "  public virtual " << intf->unambiguous_server_name(this);
      if( j + 1 < ni )  s << ",\n";
      else              s << "\n";
    }
  }
  s << o2be_template(map,
   "{\n"
   "public:\n"
   "  virtual ~_impl_foo();\n\n"
  );

  // Declare user-defined members as pure virtual.
  INC_INDENT_LEVEL();
  {
    o2be_iterator<o2be_operation, AST_Decl::NT_op> i(this);
    while( !i.is_done() ) {
      o2be_operation* op = i.item();
      IND(s); s << "virtual ";
      if( idl_global->compile_flags() & IDL_BE_OLD_SKEL_SIGNATURES )
	op->produce_decl(s, module);
      else
	op->produce_client_decl(s, module, 0, 1);
      s << " = 0;\n";
      i.next();
    }
  }
  {
    o2be_iterator<o2be_attribute, AST_Decl::NT_attr> i(this);
    while( !i.is_done() ) {
      o2be_attribute* a = i.item();
      IND(s); s << "virtual ";
      a->produce_decl_rd(s, module);
      s << ' ' << a->uqname() << "() = 0;\n";
      if( !a->readonly() ) {
	IND(s); s << "virtual void " << a->uqname() << '(';
	a->produce_decl_wr(s, module);
	s << ") = 0;\n";
      }
      i.next();
    }
  }
  DEC_INDENT_LEVEL();

  s << o2be_verbatim(
   "\n"
   "protected:\n"
   "  virtual _CORBA_Boolean _dispatch(GIOP_S&);\n"
   "\n"
   "private:\n"
   "  virtual void* _ptrToInterface(const char*);\n"
   "  virtual const char* _mostDerivedRepoId();\n"
   "};\n\n\n"
  );

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// _sk_foo //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  if( idl_global->compile_flags() & IDL_BE_GENERATE_BOA_SKEL ) {

    StringBuf _sk_foo;
    _sk_foo += BOA_SKELETON_PREFIX;
    _sk_foo += uqname();
    map.insert("_sk_foo", _sk_foo);

    s << o2be_template(map,
     "class _sk_foo :\n"
     "    public virtual _impl_foo,\n"
     "    public virtual omniOrbBoaServant\n"
     "{\n"
     "public:\n"
     "  virtual ~_sk_foo();\n\n"

     "  inline foo::_ptr_type _this() {\n"
     "    return (foo::_ptr_type) omniOrbBoaServant::_this(foo::_PD_repoId);\n"
     "  }\n"
     "  inline void _obj_is_ready(CORBA::BOA_ptr) { omniOrbBoaServant::_obj_is_ready(); }\n"
     "  inline CORBA::BOA_ptr _boa() { return CORBA::BOA::getBOA(); }\n"
     "};\n\n\n"
    );
  }

  //////////////////////////////////////////////////////////////////////

  if (idl_global->compile_flags() & IDL_CF_ANY) {
    // TypeCode_ptr declaration
    IND(s); s << variable_qualifier()
	      << " _dyn_attr const CORBA::TypeCode_ptr "
	      << tcname() << ";\n\n\n";
  }

  produce_seq_hdr_if_defined(s);
}


void 
o2be_interface::produce_helper_hdr(std::fstream& s)
{
  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// foo_Helper /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  SimpleStringMap map;
  map.insert("objref_foo", proxy_uqname());
  map.insert("impl_foo", server_uqname());
  map.insert("foo_ptr", objref_uqname());
  map.insert("foo", uqname());
  map.insert("out_template", out_adptarg_name(this));
  StringBuf helper;
  helper += uqname();
  helper += HELPER_CLASS_POSTFIX;
  map.insert("helper", helper);

  s << "#ifndef __" << _idname() << "__\n";
  s << "#define __" << _idname() << "__\n\n";

  s << o2be_template(map,
   "class foo;\n"
   "class objref_foo;\n"
   "class impl_foo;\n"
   "typedef objref_foo* foo_ptr;\n"
   "typedef foo_ptr foo@Ref;\n\n"

   "class helper {\n"
   "public:\n"
   "  typedef foo_ptr _ptr_type;\n\n"

   "  static _ptr_type _nil();\n"
   "  static _CORBA_Boolean is_nil(_ptr_type);\n"
   "  static void release(_ptr_type);\n"
   "  static void duplicate(_ptr_type);\n"
   "  static size_t NP_alignedSize(_ptr_type, size_t);\n"
   "  static void marshalObjRef(_ptr_type, NetBufferedStream&);\n"
   "  static _ptr_type unmarshalObjRef(NetBufferedStream&);\n"
   "  static void marshalObjRef(_ptr_type, MemBufferedStream&);\n"
   "  static _ptr_type unmarshalObjRef(MemBufferedStream&);\n"
   "};\n\n"

   "typedef _CORBA_ObjRef_Var<objref_foo, helper> foo@_var;\n"
   "typedef out_template foo@_out;\n\n"
  );

  s << "#endif\n\n\n";
}


void 
o2be_interface::produce_poa_hdr(std::fstream& s)
{
  StringBuf pskel;
  if( defined_in() == o2be_global::root() )
    pskel += POA_SKELETON_PREFIX;
  pskel += uqname();

  SimpleStringMap map;
  map.insert("POA_foo", pskel);
  map.insert("foo", fqname());
  map.insert("foo_ptr", objref_fqname());
  map.insert("_impl_foo", server_fqname());

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// POA_foo //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  s << o2be_template(map,
   "class POA_foo :\n"
   "    public virtual _impl_foo,\n"
   "    public virtual PortableServer::ServantBase\n"
   "{\n"
   "public:\n"
   "  virtual ~POA_foo();\n\n"

   "  inline foo_ptr _this() {\n"
   "    return (foo_ptr) _do_this(foo::_PD_repoId);\n"
   "  }\n"
   "};\n\n\n"
  );

  if( idl_global->compile_flags() & IDL_BE_GENERATE_TIE ) {
    StringBuf cn;
    cn += pskel;
    cn += POA_TIE_CLASS_POSTFIX;
    internal_produce_tie_templates(s, this, cn, pskel);
  }
}


void
o2be_interface_fwd::produce_hdr(std::fstream& s)
{
  o2be_interface* intf = o2be_interface::narrow_from_decl(full_definition());
  intf->produce_helper_hdr(s);
}


void
o2be_interface_fwd::produce_skel(std::fstream &s)
{
}


void
o2be_interface_fwd::produce_dynskel(std::fstream &s)
{
}


void
o2be_interface_fwd::produce_buildDesc_decls(std::fstream& s,
					    idl_bool even_if_in_main_file)
{
  if( pd_have_produced_buildDesc_decls )
    return;
  pd_have_produced_buildDesc_decls = 1;

  s << "extern void _0RL_buildDesc" << canonical_name()
    << "(tcDescriptor &, const " << FIELD_MEMBER_TEMPLATE
    << '<' << fqname() << HELPER_CLASS_POSTFIX ">&);\n";
}


void
o2be_interface::produce_skel(std::fstream &s)
{
  o2be_module* module = o2be_module::narrow_from_scope(defined_in());

  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	switch(d->node_type()) {
	case AST_Decl::NT_op:
	case AST_Decl::NT_attr:
	case AST_Decl::NT_enum_val:
	  break;
	case AST_Decl::NT_const:
	  o2be_constant::narrow_from_decl(d)->produce_skel(s);
	  break;
	case AST_Decl::NT_enum:
	  o2be_enum::narrow_from_decl(d)->produce_skel(s);
	  break;
	case AST_Decl::NT_except:
	  o2be_exception::narrow_from_decl(d)->produce_skel(s);
	  break;
	case AST_Decl::NT_struct:
	  o2be_structure::narrow_from_decl(d)->produce_skel(s);
	  break;
	case AST_Decl::NT_typedef:
	  o2be_typedef::narrow_from_decl(d)->produce_skel(s);
	  break;
	case AST_Decl::NT_union:
	  o2be_union::narrow_from_decl(d)->produce_skel(s);
	  break;
	default:
	  throw o2be_internal_error(__FILE__,__LINE__,"unexpected type under interface class");
	}
	i.next();
      }
  }

  SimpleStringMap map;
  StringBuf helper;
  helper += fqname();
  helper += HELPER_CLASS_POSTFIX;
  map.insert("helper", helper);
  map.insert("foo_ptr", objref_fqname());
  map.insert("foo", fqname());
  map.insert("proxy", proxy_uqname());
  map.insert("fqproxy", proxy_fqname());

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// foo_Helper /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  s << o2be_template(map,
   "foo_ptr helper::_nil() {\n"
   "  return foo::_nil();\n"
   "}\n\n"

   "CORBA::Boolean helper::is_nil(foo_ptr p) {\n"
   "  return CORBA::is_nil(p);\n"
   "}\n\n"

   "void helper::release(foo_ptr p) {\n"
   "  CORBA::release(p);\n"
   "}\n\n"

   "void helper::duplicate(foo_ptr p) {\n"
   "  if( p )  omni::duplicateObjRef(p);\n"
   "}\n\n"

   "size_t helper::NP_alignedSize(foo_ptr obj, size_t offset) {\n"
   "  return foo::_alignedSize(obj, offset);\n"
   "}\n\n"

   "void helper::marshalObjRef(foo_ptr obj, NetBufferedStream& s) {\n"
   "  foo::_marshalObjRef(obj, s);\n"
   "}\n\n"

   "foo_ptr helper::unmarshalObjRef(NetBufferedStream& s) {\n"
   "  return foo::_unmarshalObjRef(s);\n"
   "}\n\n"

   "void helper::marshalObjRef(foo_ptr obj, MemBufferedStream& s) {\n"
   "  foo::_marshalObjRef(obj, s);\n"
   "}\n\n"

   "foo_ptr helper::unmarshalObjRef(MemBufferedStream& s) {\n"
   "  return foo::_unmarshalObjRef(s);\n"
   "}\n\n\n"

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////////// foo ////////////////////////////////
  //////////////////////////////////////////////////////////////////////

   "foo_ptr\n"
   "foo::_duplicate(foo_ptr obj)\n"
   "{\n"
   "  if( obj )  omni::duplicateObjRef(obj);\n"
   "  return obj;\n"
   "}\n\n\n"


   "foo_ptr\n"
   "foo::_narrow(CORBA::Object_ptr obj)\n"
   "{\n"
   "  if( !obj || obj->_NP_is_nil() || obj->_NP_is_pseudo() ) return _nil();\n"
   "  _ptr_type e = (_ptr_type) obj->_PR_getobj()->_realNarrow(_PD_repoId);\n"
   "  return e ? e : _nil();\n"
   "}\n\n\n"


   "foo_ptr\n"
   "foo::_nil()\n"
   "{\n"
   "  static proxy* _the_nil_ptr = 0;\n"
   "  {\n"
   "    omni_tracedmutex_lock sync(omni::nilRefLock);\n"
   "    if( !_the_nil_ptr )  _the_nil_ptr = new proxy();\n"
   "  }\n"
   "  return _the_nil_ptr;\n"
   "}\n\n\n"
  );

  // _PD_repoId
  IND(s); s << "const char* " << fqname() << "::_PD_repoId = \""
	    << repositoryID() << "\";\n\n\n";

  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// _objref_foo ////////////////////////////
  //////////////////////////////////////////////////////////////////////

  s << o2be_template(map,
   "fqproxy::~proxy() {}\n\n\n"

   "fqproxy::proxy(const char* mdri,\n"
   "  IOP::TaggedProfileList* p, omniIdentity* id, omniLocalIdentity* lid) :\n"
  );
  {
    AST_Interface** intftable = inherits();
    int ni = n_inherits();
    for( int i = 0; i < ni; i++ ) {
      o2be_interface* intf = o2be_interface::narrow_from_decl(intftable[i]);
      //?? Does this need to be unambiguous to make MSVC happy?
#if 1
      IND(s); s << "   "
		<< intf->unambiguous_proxy_name(this)
		<< "(mdri, p, id, lid),\n";
#else
      IND(s); s << "   " << intf->proxy_fqname() << "(mdri, p, id, lid),\n";
#endif
    }
  }
  s << o2be_template(map,
   "   omniObjRef(foo::_PD_repoId, mdri, p, id, lid)\n"
   "{\n"
   "  _PR_setobj(this);\n"
   "}\n\n\n"

  // _ptrToObjRef()
   "void*\n"
   "fqproxy::_ptrToObjRef(const char* id)\n"
   "{\n"
   "  if( !strcmp(id, CORBA::Object::_PD_repoId) )\n"
   "    return (CORBA::Object_ptr) this;\n"
   "  if( !strcmp(id, foo::_PD_repoId) )\n"
   "    return (foo_ptr) this;\n"
  );
  INC_INDENT_LEVEL();
  {
    base_iterator i(this, 1);
    while( !i.is_done() ) {
      IND(s); s << "if( !strcmp(id, " << i.item()->fqname()
		<< "::_PD_repoId) )\n";
      IND(s); s << "  return (" << i.item()->objref_fqname() << ") this;\n";
      i.next();
    }
  }
  DEC_INDENT_LEVEL();
  s << o2be_verbatim(
   "\n"
   "  return 0;\n"
   "}\n\n\n"
  );

  // IDL declared members.
  {
    o2be_iterator<o2be_operation, AST_Decl::NT_op> i(this);
    while( !i.is_done() ) {
      i.item()->produce_proxy_skel(s, *this);
      i.next();
    }
  }
  {
    o2be_iterator<o2be_attribute, AST_Decl::NT_attr> i(this);
    while( !i.is_done() ) {
      o2be_attribute* a = i.item();
      a->produce_proxy_rd_skel(s, *this);
      if( !a->readonly() )  a->produce_proxy_wr_skel(s,*this);
      i.next();
    }
  }

  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// _pof_foo //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  StringBuf pof;
  pof += scopename();
  pof += POF_CLASS_PREFIX;
  pof += uqname();
  map.insert("pof", pof);

  StringBuf uqpof;
  uqpof += POF_CLASS_PREFIX;
  uqpof += uqname();
  map.insert("uqpof", uqpof);

  s << o2be_template(map,
   "pof::~uqpof() {}\n\n\n"

   "omniObjRef*\n"
   "pof::newObjRef(const char* mdri, IOP::TaggedProfileList* p,\n"
   "               omniIdentity* id, omniLocalIdentity* lid)\n"
   "{\n"
   "  return new fqproxy(mdri, p, id, lid);\n"
   "}\n\n\n"


   "CORBA::Boolean\n"
   "pof::is_a(const char* id) const\n"
   "{\n"
   "  if( !strcmp(id, foo::_PD_repoId) )\n"
   "    return 1;\n"
  );
  {
    base_iterator i(this, 1);
    while( !i.is_done() ) {
      IND(s); s << "  if( !strcmp(id, " << i.item()->fqname()
		<< "::_PD_repoId) )\n";
      IND(s); s << "    return 1;\n";
      i.next();
    }
  }
  s << o2be_verbatim(
   "\n"
   "  return 0;\n"
   "}\n\n\n"
  );

  IND(s); s << "const " << scopename() << POF_CLASS_PREFIX << uqname()
	    << " _the_pof_" << _idname() << ";\n\n\n";

  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// _impl_foo /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  if( o2be_global::mflag() ) {
    // MSVC {4.2,5.0} cannot deal with a call to a virtual member
    // of a base class using the member function's fully/partially
    // scoped name. Have to use the alias for the base class in the
    // global scope to refer to the virtual member function instead.
    //
    // We scan all the base interfaces to see if any of them has to
    // be referred to by their fully/partially qualified names. If
    // that is necessary, we generate a typedef to define an alias for
    // this base interface. This alias is used in the stub generated below

    AST_Interface** intftable = inherits();
    int ni = n_inherits();

    for( int j = 0; j < ni; j++ ) {
      o2be_interface* intf = o2be_interface::narrow_from_decl(intftable[j]);
      char* intf_name = intf->unambiguous_name(this);
      if( strcmp(intf_name, intf->uqname()) != 0 ) {
	s << "#ifndef __" << intf->_idname() << "__ALIAS__\n";
	s << "#define __" << intf->_idname() << "__ALIAS__\n";
	IND(s); s << "typedef " << intf->fqname() << " " 
		  << intf->_fqname() << ";\n";
	IND(s); s << "typedef " << intf->server_fqname() << " "
		  << intf->_scopename() << intf->server_uqname() << ";\n";
	s << "#endif\n\n";
      }
    }
  }

  map.insert("impl", server_uqname());
  map.insert("fqimpl", server_fqname());

  s << o2be_template(map,
   "fqimpl::~impl() {}\n\n\n"


   "CORBA::Boolean\n"
   "fqimpl::_dispatch(GIOP_S& giop_s)\n"
   "{\n"
  );
  INC_INDENT_LEVEL();
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while( !i.is_done() ) {
      AST_Decl* d = i.item();
      if( d->node_type() == AST_Decl::NT_op ) {
	IND(s); s << "if( !strcmp(giop_s.operation(), \""
		  << d->local_name()->get_string()
		  << "\") ) {\n";
	INC_INDENT_LEVEL();
	o2be_operation::narrow_from_decl(d)->produce_server_skel(s, module);
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
      }
      else if( d->node_type() == AST_Decl::NT_attr ) {
	o2be_attribute* a = o2be_attribute::narrow_from_decl(d);
	IND(s); s << "if( !strcmp(giop_s.operation(), \""
		  << "_get_" << a->local_name()->get_string()
		  << "\") ) {\n";
	INC_INDENT_LEVEL();
	a->produce_server_rd_skel(s, module);
	DEC_INDENT_LEVEL();
	IND(s); s << "}\n";
	if( !a->readonly() ) {
	  IND(s); s << "if( !strcmp(giop_s.operation(), \""
		    << "_set_" << a->local_name()->get_string()
		    << "\") ) {\n";
	  INC_INDENT_LEVEL();
	  a->produce_server_wr_skel(s, module);
	  DEC_INDENT_LEVEL();
	  IND(s); s << "}\n";
	}
      }
      i.next();
    }
    AST_Interface** intftable = inherits();
    int ni = n_inherits();
    for( int j = 0; j < ni; j++ ) {
      o2be_interface * intf = o2be_interface::narrow_from_decl(intftable[j]);
      char* intf_name = (char*)intf->unambiguous_server_name(this);
      if( o2be_global::mflag() ) {
	// MSVC {4.2,5.0} cannot deal with a call to a virtual member
	// of a base class using the member function's fully/partially
	// scoped name. Have to use the alias for the base class in the
	// global scope to refer to the virtual member function instead.
	if (strcmp(intf_name,intf->server_uqname()) != 0) {
	  if (strcmp(uqname(), intf->uqname()) != 0) {
	    intf_name = new char[strlen(intf->_scopename())+
				strlen(intf->server_uqname())+1];
	    intf_name[0] = '\0';
	  }
	  else {
	    // The interface has the same name as this interface,
	    // Another bug in MSVC {5.0?} causes the wrong dispatch
	    // function to be called.
	    // The workaround is to first cast the this pointer to the 
	    // base class.
	    intf_name = new char[strlen("((*)this)->") +
				strlen(intf->_scopename())*2+
				strlen(intf->server_uqname())*2+1];
	    strcpy(intf_name,"((");
	    strcat(intf_name,intf->_scopename());
	    strcat(intf_name,intf->server_uqname());
	    strcat(intf_name,"*)this)->");
	  }
	  strcat(intf_name,intf->_scopename());
	  strcat(intf_name,intf->server_uqname());
	}
      }
      IND(s); s << "if( " << intf_name << "::_dispatch(giop_s) ) {\n";
      INC_INDENT_LEVEL();
      IND(s); s << "return 1;\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
    }
    IND(s); s << "return 0;\n";
  }
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n\n";

  // _ptrToInterface()
  s << o2be_template(map,
   "void*\n"
   "fqimpl::_ptrToInterface(const char* id)\n"
   "{\n"
   "  if( !strcmp(id, CORBA::Object::_PD_repoId) )\n"
   "    return (void*) 1;\n"
   "  if( !strcmp(id, foo::_PD_repoId) )\n"
   "    return (impl*) this;\n"
  );
  {
    base_iterator i(this, 1);
    while( !i.is_done() ) {
      IND(s); s << "  if( !strcmp(id, " << i.item()->unambiguous_name(this)
		<< "::_PD_repoId) )\n";
      IND(s); s << "    return (" << i.item()->unambiguous_server_name(this)
		<< "*) this;\n";
      i.next();
    }
  }
  s << o2be_template(map,
   "\n"
   "  return 0;\n"
   "}\n\n\n"


   "const char*\n"
   "fqimpl::_mostDerivedRepoId()\n"
   "{\n"
   "  return foo::_PD_repoId;\n"
   "}\n\n\n"
  );

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// _sk_foo //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  if( idl_global->compile_flags() & IDL_BE_GENERATE_BOA_SKEL ) {

    StringBuf skel;
    skel += BOA_SKELETON_PREFIX;
    skel += uqname();

    StringBuf fqskel;
    fqskel += scopename();
    fqskel += skel;

    map.insert("skel", skel);
    map.insert("fqskel", fqskel);

    s << o2be_template(map, "fqskel::~skel() {}\n\n\n");
  }
}


void 
o2be_interface::produce_poa_skel(std::fstream& s)
{
  StringBuf pskel;
  if( defined_in() == o2be_global::root() )
    pskel += POA_SKELETON_PREFIX;
  pskel += uqname();

  StringBuf fqpskel;
  fqpskel += POA_SKELETON_PREFIX;
  fqpskel += scopename();
  fqpskel += uqname();

  SimpleStringMap map;
  map.insert("fqPOA_foo", fqpskel);
  map.insert("POA_foo", pskel);
  map.insert("foo_ptr", objref_fqname());
  map.insert("foo", fqname());

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// POA_foo //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  s << o2be_template(map, "fqPOA_foo::~POA_foo() {}\n\n\n");
}


void
o2be_interface::produce_dynskel(std::fstream &s)
{
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	switch(d->node_type()) {
	case AST_Decl::NT_op:
	  o2be_operation::narrow_from_decl(d)->produce_dynskel(s);
	  break;
	case AST_Decl::NT_attr:
	  o2be_attribute::narrow_from_decl(d)->produce_dynskel(s);
	  break;
	case AST_Decl::NT_enum_val:
	  break;
	case AST_Decl::NT_const:
	  o2be_constant::narrow_from_decl(d)->produce_dynskel(s);
	  break;
	case AST_Decl::NT_enum:
	  o2be_enum::narrow_from_decl(d)->produce_dynskel(s);
	  break;
	case AST_Decl::NT_except:
	  o2be_exception::narrow_from_decl(d)->produce_dynskel(s);
	  break;
	case AST_Decl::NT_struct:
	  o2be_structure::narrow_from_decl(d)->produce_dynskel(s);
	  break;
	case AST_Decl::NT_typedef:
	  o2be_typedef::narrow_from_decl(d)->produce_dynskel(s);
	  break;
	case AST_Decl::NT_union:
	  o2be_union::narrow_from_decl(d)->produce_dynskel(s);
	  break;
	default:
	  throw o2be_internal_error(__FILE__, __LINE__,
				    "unexpected type under interface class");
	}
	i.next();
      }
  }

  if (defined_in() != idl_global->root() &&
      defined_in()->scope_node_type() == AST_Decl::NT_module)
    {
      s << "\n#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)\n";
      IND(s); s << "// MSVC++ does not give the constant external"
		" linkage otherwise.\n";
      AST_Decl* inscope = ScopeAsDecl(defined_in());
      char* scopename = o2be_name::narrow_and_produce_uqname(inscope);
      if (strcmp(scopename,o2be_name::narrow_and_produce_fqname(inscope)))
	{
	  scopename = o2be_name::narrow_and_produce__fqname(inscope);
	  IND(s); s << "namespace " << scopename << " = " 
		    << o2be_name::narrow_and_produce_fqname(inscope)
		    << ";\n";
	}
      IND(s); s << "namespace " << scopename << " {\n";
      INC_INDENT_LEVEL();
      IND(s); s << "const CORBA::TypeCode_ptr " << tcname() << " = "
		<< "CORBA::TypeCode::PR_interface_tc(\""
		<< repositoryID() << "\", \"" << uqname() << "\");\n\n";
      DEC_INDENT_LEVEL();
      IND(s); s << "}\n";
      s << "#else\n";
      IND(s); s << "const CORBA::TypeCode_ptr " << fqtcname() << " = " 
		<< "CORBA::TypeCode::PR_interface_tc(\""
		<< repositoryID() << "\", \"" << uqname() << "\");\n\n";
      s << "#endif\n\n";
    }
  else
    {
      IND(s); s << "const CORBA::TypeCode_ptr " << fqtcname() << " = " 
		<< "CORBA::TypeCode::PR_interface_tc(\""
		<< repositoryID() << "\", \"" << uqname() << "\");\n\n";
    }
}


void
o2be_interface::produce_decls_at_global_scope_in_hdr(std::fstream& s)
{
  UTL_ScopeActiveIterator i(this, UTL_Scope::IK_decls);

  while( !i.is_done() ) {
    AST_Decl* d = i.item();

    switch( d->node_type() ) {
    case AST_Decl::NT_union:
      o2be_union::narrow_from_decl(d)
	->produce_decls_at_global_scope_in_hdr(s);
      break;
    case AST_Decl::NT_struct:
      o2be_structure::narrow_from_decl(d)
	->produce_decls_at_global_scope_in_hdr(s);
      break;
    case AST_Decl::NT_except:
      o2be_exception::narrow_from_decl(d)
	->produce_decls_at_global_scope_in_hdr(s);
      break;
    case AST_Decl::NT_interface:
      o2be_interface::narrow_from_decl(d)
	->produce_decls_at_global_scope_in_hdr(s);
      break;
    case AST_Decl::NT_op:
      o2be_operation::narrow_from_decl(d)
	->produce_decls_at_global_scope_in_hdr(s);
      break;
    case AST_Decl::NT_attr:
      o2be_attribute::narrow_from_decl(d)
	->produce_decls_at_global_scope_in_hdr(s);
      break;
    default:
      break;
    }

    i.next();
  }
}


void
o2be_interface::produce_defns_at_global_scope_in_hdr(std::fstream& s)
{
  int repoIdLen = strlen(repositoryID()) + 1;

  IND(s); s << "inline size_t\n";
  IND(s); s << fqname() << "::_alignedSize(" << objref_fqname() << " obj, "
	    "size_t offset) {\n";
  IND(s); s << "  return CORBA::AlignedObjRef(obj, _PD_repoId, "
	    << repoIdLen << ", offset);\n";
  IND(s); s << "}\n\n";

  IND(s); s << "inline void\n";
  IND(s); s << fqname() << "::_marshalObjRef(" << objref_fqname() << " obj, "
	    "NetBufferedStream& s) {\n";
  IND(s); s << "  CORBA::MarshalObjRef(obj, _PD_repoId, "
	    << repoIdLen << ", s);\n";
  IND(s); s << "}\n\n";

  IND(s); s << "inline void\n";
  IND(s); s << fqname() << "::_marshalObjRef(" << objref_fqname() << " obj, "
	    "MemBufferedStream& s) {\n";
  IND(s); s << "  CORBA::MarshalObjRef(obj, _PD_repoId, "
	    << repoIdLen << ", s);\n";
  IND(s); s << "}\n\n\n";
}


void
o2be_interface::produce_binary_operators_in_hdr(std::fstream& s)
{
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	switch(d->node_type()) {
	case AST_Decl::NT_enum:
	  o2be_enum::
	    narrow_from_decl(d)->produce_binary_operators_in_hdr(s);
	  break;
	case AST_Decl::NT_except:
	  o2be_exception::
	    narrow_from_decl(d)->produce_binary_operators_in_hdr(s);
	  break;
	case AST_Decl::NT_struct:
	  o2be_structure::
	    narrow_from_decl(d)->produce_binary_operators_in_hdr(s);
	  break;
	case AST_Decl::NT_typedef:
	  o2be_typedef::
	    narrow_from_decl(d)->produce_binary_operators_in_hdr(s);
	  break;
	case AST_Decl::NT_union:
	  o2be_union::
	    narrow_from_decl(d)->produce_binary_operators_in_hdr(s);
	  break;
	default:
	  break;
	}
	i.next();
      }
  }

  if (idl_global->compile_flags() & IDL_CF_ANY) {
    s << "\n";
    // any insertion and extraction operators
    IND(s); s << "void operator<<=(CORBA::Any& _a, " << objref_fqname()
	      << " _s);\n";
    IND(s); s << "void operator<<=(CORBA::Any& _a, " << objref_fqname() 
	      << "* _s);\n";
    IND(s); s << "CORBA::Boolean operator>>=(const CORBA::Any& _a, " 
	      << objref_fqname() 
	      << "& _s);\n\n";
  }
}


void
o2be_interface::produce_binary_operators_in_dynskel(std::fstream& s)
{
  {
    UTL_ScopeActiveIterator i(this,UTL_Scope::IK_decls);
    while (!i.is_done())
      {
	AST_Decl *d = i.item();
	switch(d->node_type()) {
	case AST_Decl::NT_enum:
	  o2be_enum::
	    narrow_from_decl(d)->produce_binary_operators_in_dynskel(s);
	  break;
	case AST_Decl::NT_except:
	  o2be_exception::
	    narrow_from_decl(d)->produce_binary_operators_in_dynskel(s);
	  break;
	case AST_Decl::NT_struct:
	  o2be_structure::
	    narrow_from_decl(d)->produce_binary_operators_in_dynskel(s);
	  break;
	case AST_Decl::NT_typedef:
	  o2be_typedef::
	    narrow_from_decl(d)->produce_binary_operators_in_dynskel(s);
	  break;
	case AST_Decl::NT_union:
	  o2be_union::
	    narrow_from_decl(d)->produce_binary_operators_in_dynskel(s);
	  break;
	default:
	  break;
	}
	i.next();
      }
  }

  //////////////////////////////////////////////////////////////////////
  //////////////////////// tcDescriptor generation /////////////////////
  //////////////////////////////////////////////////////////////////////

  IND(s); s << "static void\n";
  IND(s); s << "_0RL_tcParser_setObjectPtr_" << _idname()
	    << "(tcObjrefDesc *_desc, CORBA::Object_ptr _ptr)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << objref_fqname() << " _p = " << fqname()
	    << "::_narrow(_ptr);\n";
  IND(s); s << fqname() << "_ptr* pp = (" << fqname() 
	    << "_ptr*)_desc->opq_objref;\n";
  IND(s); s << "if (_desc->opq_release && !CORBA::is_nil(*pp)) CORBA::release(*pp);\n";
  IND(s); s << "*pp = _p;\n";
  IND(s); s << "CORBA::release(_ptr);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "static CORBA::Object_ptr\n";
  IND(s); s << "_0RL_tcParser_getObjectPtr_" << _idname()
	    << "(tcObjrefDesc *_desc)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "return (CORBA::Object_ptr) *"
	    << "((" << fqname()
	    << "_ptr*)_desc->opq_objref);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "void _0RL_buildDesc" << canonical_name()
	    << "(tcDescriptor& _desc, const "
	    << fieldMemberType_fqname(o2be_global::root()) << "& _data)\n";
  IND(s); s << "{\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_desc.p_objref.opq_objref = (void*) &_data._ptr;\n";
  IND(s); s << "_desc.p_objref.opq_release = _data.pd_rel;\n";
  IND(s); s << "_desc.p_objref.setObjectPtr = _0RL_tcParser_setObjectPtr_"
	    << _idname() << ";\n";
  IND(s); s << "_desc.p_objref.getObjectPtr = _0RL_tcParser_getObjectPtr_"
	    << _idname() << ";\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n\n";

  IND(s); s << "void _0RL_delete_" << _idname() << "(void* _data) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::release((" << objref_fqname() << ") _data);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";


  //////////////////////////////////////////////////////////////////////
  /////////////////////// Any insertion operator ///////////////////////
  //////////////////////////////////////////////////////////////////////

  IND(s); s << "void operator<<=(CORBA::Any& _a, "
	    << objref_fqname() << " _s) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "tcDescriptor tcd;\n";
  IND(s); s << fieldMemberType_fqname(o2be_global::root())
	    << " tmp(_s,0);\n";
  o2be_buildDesc::call_buildDesc(s, this, "tcd", "tmp");
  IND(s); s << "_a.PR_packFrom(" << fqtcname() << ", &tcd);\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  IND(s); s << "void operator<<=(CORBA::Any& _a, " << objref_fqname() 
	      << "* _sp) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_a <<= *_sp;\n";
  IND(s); s << "CORBA::release(*_sp);\n";
  IND(s); s << "*_sp = " << fqname() << "::_nil();\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n\n";

  //////////////////////////////////////////////////////////////////////
  //////////////////////// Any extraction operator /////////////////////
  //////////////////////////////////////////////////////////////////////

  IND(s); s << "CORBA::Boolean operator>>=(const CORBA::Any& _a, "
	    << objref_fqname() << "& _s) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << objref_fqname() << " sp = ("
	    << objref_fqname() << ") _a.PR_getCachedData();\n";
  IND(s); s << "if (sp == 0) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "tcDescriptor tcd;\n";
  IND(s); s << fieldMemberType_fqname(o2be_global::root())
	    << " tmp;\n";
  o2be_buildDesc::call_buildDesc(s, this, "tcd", "tmp");
  IND(s); s << "if( _a.PR_unpackTo(" << fqtcname()
	    << ", &tcd) ) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "if (!omniORB::omniORB_27_CompatibleAnyExtraction) {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "((CORBA::Any*)&_a)->PR_setCachedData((void*)tmp._ptr,"
	    << "_0RL_delete_" << _idname() << ");\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "_s = tmp._ptr;\n";
  IND(s); s << "tmp._ptr = " << fqname() << "::_nil(); return 1;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "} else {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "_s = " << fqname() << "::_nil(); return 0;\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  IND(s); s << "else {\n";
  INC_INDENT_LEVEL();
  IND(s); s << "CORBA::TypeCode_var tc = _a.type();\n";
  IND(s); s << "if (tc->equivalent(" << fqtcname() << ")) {\n";
  IND(s); s << "_s = sp; return 1;\n";
  IND(s); s << "}\n";
  IND(s); s << "else {\n";
  IND(s); s << "_s = " << fqname() << "::_nil(); return 0;\n";
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
  DEC_INDENT_LEVEL();
  IND(s); s << "}\n";
}


void
o2be_interface::produce_typedef_hdr(std::fstream& s, o2be_typedef* tdef)
{
  const char* uan = unambiguous_name(tdef);

  IND(s); s << "typedef " << uan << " " << tdef->uqname() << ";\n";
  IND(s); s << "typedef " << unambiguous_objref_name(tdef) 
	    << " " << tdef->uqname() << "_ptr;\n";
  IND(s); s << "typedef " << uan << "Ref " << tdef->uqname() << "Ref;\n";
  IND(s); s << "typedef " << unambiguous_server_name(tdef)
	    << " " IMPL_CLASS_PREFIX << tdef->uqname() << ";\n";
  if( strcmp(uqname(), "CORBA::Object") ) {
    IND(s); s << "typedef " << uan << HELPER_CLASS_POSTFIX " "
	      << tdef->uqname() << HELPER_CLASS_POSTFIX ";\n";
    IND(s); s << "typedef " << unambiguous_proxy_name(tdef)
	      << " " << PROXY_CLASS_PREFIX << tdef->uqname() << ";\n";
  }
  else {
    IND(s); s << "typedef CORBA::Object_Helper " 
	      << tdef->uqname() << HELPER_CLASS_POSTFIX ";\n";
  }
  IND(s); s << "typedef " << uan << "_var " << tdef->uqname() << "_var;\n";
  IND(s); s << "typedef " << uan << "_out " << tdef->uqname() << "_out;\n\n\n";
}


void
o2be_interface::produce_typedef_poa_hdr(std::fstream& s, o2be_typedef* tdef)
{
#if 0
  //  Ooops.  Actually fails in most interesting cases.
  // We can only put the poa prefix on at the outermost
  // scope!  Also, the interface may have been forward
  // declared.  We cannot unfortunately forward declare
  // things in a POA_foo module...so we are stuck.

  IND(s);
  if( tdef->defined_in() == o2be_global::root() ) {
    s << "typedef " << POA_SKELETON_PREFIX << scopename() << uqname()
      << " " POA_SKELETON_PREFIX << tdef->uqname() << ";\n\n";
  }
  else {
    // This will fail if the unambiguous scope name is
    // prefixed with ::.  So we bravely duck the issue
    // here.
    char* usn = unambiguous_scopename(tdef);
    if( usn[0] != ':' ) {
      s << "typedef " << POA_SKELETON_PREFIX << usn << uqname()
	<< " " << tdef->uqname() << ";\n\n";
    }
  }
#endif
}


void
o2be_interface::produce_tie_templates(std::fstream& s)
{
  StringBuf class_name;
  class_name += POA_SKELETON_PREFIX;
  class_name += _fqname();
  class_name += POA_TIE_CLASS_POSTFIX;

  StringBuf base_class;
  base_class += POA_SKELETON_PREFIX;
  base_class += fqname();

  internal_produce_tie_templates(s, this, class_name, base_class);

  if( idl_global->compile_flags() & IDL_BE_GENERATE_BOA_SKEL ) {
    class_name.clear();
    class_name += BOA_TIE_CLASS_PREFIX;
    class_name += _fqname();

    base_class.clear();
    base_class += scopename();
    base_class += BOA_SKELETON_PREFIX;
    base_class += uqname();

    internal_produce_tie_templates(s, this, class_name, base_class);
  }
}


void
o2be_interface::produce_buildDesc_decls(std::fstream& s,
					idl_bool even_if_in_main_file)
{
  if( pd_have_produced_buildDesc_decls )
    return;
  pd_have_produced_buildDesc_decls = 1;

  s << "extern void _0RL_buildDesc" << canonical_name()
    << "(tcDescriptor &, const "
    << fieldMemberType_fqname(o2be_global::root()) << "&);\n";
}


const char*
o2be_interface::fieldMemberType_fqname(AST_Decl* used_in)
{
  if (!pd_fieldmem_fqname) {
    pd_fieldmem_fqname = new char[strlen(proxy_fqname())+
				 strlen(fqname())+strlen(HELPER_CLASS_POSTFIX)+
				 strlen(FIELD_MEMBER_TEMPLATE)+4];
    strcpy(pd_fieldmem_fqname,FIELD_MEMBER_TEMPLATE);
    strcat(pd_fieldmem_fqname,"<");
    strcat(pd_fieldmem_fqname,proxy_fqname());
    strcat(pd_fieldmem_fqname,",");
    strcat(pd_fieldmem_fqname,fqname());
    strcat(pd_fieldmem_fqname,HELPER_CLASS_POSTFIX);
    strcat(pd_fieldmem_fqname,">");
  }

  if( o2be_global::qflag() || !strcmp(fqname(), "CORBA::Object") ) {
    return pd_fieldmem_fqname;
  }
  else {
    const char* ubpname = unambiguous_proxy_name(used_in);
    if (strcmp(proxy_fqname(),ubpname) == 0)
      return pd_fieldmem_fqname;
    else {
      StringBuf t;
      t += FIELD_MEMBER_TEMPLATE;
      t += '<';
      t += ubpname;
      t += ", ";
      t += unambiguous_name(used_in);
      t += HELPER_CLASS_POSTFIX;
      t += '>';
      return t.release();
    }
  }
}

const char*
o2be_interface::seqMemberType_fqname(AST_Decl* used_in)
{
  if (!pd_seqmem_fqname) {
    pd_seqmem_fqname = new char[strlen(proxy_fqname())+
				 strlen(fqname())+strlen(HELPER_CLASS_POSTFIX)+
				 strlen(SEQ_MEMBER_TEMPLATE)+4];
    strcpy(pd_seqmem_fqname,SEQ_MEMBER_TEMPLATE);
    strcat(pd_seqmem_fqname,"<");
    strcat(pd_seqmem_fqname,proxy_fqname());
    strcat(pd_seqmem_fqname,",");
    strcat(pd_seqmem_fqname,fqname());
    strcat(pd_seqmem_fqname,HELPER_CLASS_POSTFIX);
    strcat(pd_seqmem_fqname,">");
  }

  if( o2be_global::qflag() || !strcmp(fqname(), "CORBA::Object") ) {
    return pd_seqmem_fqname;
  }
  else {
    const char* ubpname = unambiguous_proxy_name(used_in);
    if (strcmp(proxy_fqname(), ubpname) == 0)
      return pd_seqmem_fqname;
    else {
      StringBuf t;
      t += SEQ_MEMBER_TEMPLATE;
      t += '<';
      t += ubpname;
      t += ", ";
      t += unambiguous_name(used_in);
      t += HELPER_CLASS_POSTFIX;
      t += '>';
      return t.release();
    }
  }
}


const char*
o2be_interface::unambiguous_objref_name(AST_Decl* used_in,
					idl_bool use_fqname) const
{
  if (o2be_global::qflag()) {
    return objref_fqname();
  }
  else {
    const char* ubname = unambiguous_name(used_in,use_fqname);
    if (strcmp(fqname(),ubname) == 0) {
      return objref_fqname();
    }
    else {
      char* result = new char[strlen(ubname)+strlen("_ptr")+1];
      strcpy(result,ubname);
      strcat(result,"_ptr");
      return result;
    }
  }
}


const char*
o2be_interface::unambiguous_proxy_name(AST_Decl* used_in,
				       idl_bool use_fqname) const
{
  if (o2be_global::qflag()) {
    return proxy_fqname();
  }
  else {
    const char* ubsname = unambiguous_scopename(used_in,use_fqname);
    if (strcmp(scopename(),ubsname) == 0) {
      return proxy_fqname();
    }
    else {
      char* result = new char[strlen(ubsname)+strlen(pd_proxy_uqname)+1];
      strcpy(result,ubsname);
      strcat(result,pd_proxy_uqname);
      return result;
    }
  }
}


const char*
o2be_interface::unambiguous_server_name(AST_Decl* used_in,
					idl_bool use_fqname) const
{
  if( o2be_global::qflag() )
    return server_fqname();

  const char* ubsname = unambiguous_scopename(used_in, use_fqname);

  if( strcmp(scopename(), ubsname) == 0 )
    return server_fqname();

  char* result = new char[strlen(ubsname) + strlen(pd_server_uqname) + 1];
  strcpy(result, ubsname);
  strcat(result, pd_server_uqname);
  return result;
}


const char*
o2be_interface::inout_adptarg_name(AST_Decl* used_in)
{
  if (!pd_inout_adptarg_name) {
    pd_inout_adptarg_name = new char[strlen(ADPT_INOUT_CLASS_TEMPLATE)+
				    strlen("<, >")+
				    strlen(proxy_fqname())+
				    strlen(fqname())+strlen(HELPER_CLASS_POSTFIX)+1];
    strcpy(pd_inout_adptarg_name,ADPT_INOUT_CLASS_TEMPLATE);
    strcat(pd_inout_adptarg_name,"<");
    strcat(pd_inout_adptarg_name,proxy_fqname());
    strcat(pd_inout_adptarg_name,",");
    strcat(pd_inout_adptarg_name,fqname());
    strcat(pd_inout_adptarg_name,HELPER_CLASS_POSTFIX);
    strcat(pd_inout_adptarg_name," >");
  }

  if( o2be_global::qflag() ) {
    return pd_inout_adptarg_name;
  }
  else {
    const char* ubname = unambiguous_name(used_in);
    const char* uborname = unambiguous_proxy_name(used_in);
    if (strcmp(fqname(),ubname) == 0) {
      return pd_inout_adptarg_name;
    }
    else {
      const char* fm = fieldMemberType_fqname(used_in);
      char* result = new char[strlen(ADPT_INOUT_CLASS_TEMPLATE)+
			     strlen("<, >")+
			     strlen(uborname)+
			     strlen(ubname)+strlen(HELPER_CLASS_POSTFIX)+1];
      strcpy(result,ADPT_INOUT_CLASS_TEMPLATE);
      strcat(result,"<");
      strcat(result,uborname);
      strcat(result,",");
      strcat(result,ubname);
      strcat(result,HELPER_CLASS_POSTFIX);
      strcat(result," >");
      return result;
    }
  }
}


const char*
o2be_interface::out_adptarg_name(AST_Decl* used_in)
{
  if (!pd_out_adptarg_name) {
    pd_out_adptarg_name = new char[strlen(ADPT_OUT_CLASS_TEMPLATE)+
				  strlen("<, >")+
				  strlen(proxy_fqname())+
				  strlen(fqname())+strlen(HELPER_CLASS_POSTFIX)+1];
    strcpy(pd_out_adptarg_name,ADPT_OUT_CLASS_TEMPLATE);
    strcat(pd_out_adptarg_name,"<");
    strcat(pd_out_adptarg_name,proxy_fqname());
    strcat(pd_out_adptarg_name,",");
    strcat(pd_out_adptarg_name,fqname());
    strcat(pd_out_adptarg_name,HELPER_CLASS_POSTFIX);
    strcat(pd_out_adptarg_name," >");
  }

  if( o2be_global::qflag() ) {
    return pd_out_adptarg_name;
  }
  else {
    const char* ubname = unambiguous_name(used_in);
    const char* uborname = unambiguous_proxy_name(used_in);
    if (strcmp(fqname(),ubname) == 0) {
      return pd_out_adptarg_name;
    }
    else {
      const char* fm = fieldMemberType_fqname(used_in);
      char* result = new char[strlen(ADPT_OUT_CLASS_TEMPLATE)+
			     strlen("<, >")+
			     strlen(uborname)+
			     strlen(ubname)+strlen(HELPER_CLASS_POSTFIX)+1];
      strcpy(result,ADPT_OUT_CLASS_TEMPLATE);
      strcat(result,"<");
      strcat(result,uborname);
      strcat(result,",");
      strcat(result,ubname);
      strcat(result,HELPER_CLASS_POSTFIX);
      strcat(result," >");
      return result;
    }
  }
}


IMPL_NARROW_METHODS1(o2be_interface, AST_Interface)
IMPL_NARROW_FROM_DECL(o2be_interface)
IMPL_NARROW_FROM_SCOPE(o2be_interface)
IMPL_NARROW_METHODS1(o2be_interface_fwd, AST_InterfaceFwd)
IMPL_NARROW_FROM_DECL(o2be_interface_fwd)
IMPL_NARROW_FROM_SCOPE(o2be_interface_fwd)
IMPL_NARROW_METHODS1(o2be_argument, AST_Argument)
IMPL_NARROW_FROM_DECL(o2be_argument)

o2be_argument::o2be_argument(AST_Argument::Direction d, AST_Type *ft,
			     UTL_ScopedName *n, UTL_StrList *p)
	   : AST_Argument(d, ft, n, p),
	     AST_Field(AST_Decl::NT_argument, ft, n, p),
	     AST_Decl(AST_Decl::NT_argument, n, p),
             o2be_name(AST_Decl::NT_argument,n,p)
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Highly inefficient hack. Oh how we dream of the STL ...

class OpnList {
public:
  inline OpnList() {
    pd_size = 10;
    pd_ops = new char*[pd_size];
    pd_n = 0;
  };
  inline ~OpnList() {
    for( int i = 0; i < pd_n; i++ )  delete[] pd_ops[i];
    delete[] pd_ops;
  }

  void add(const char* op);
  int is_member(const char* op);

private:
  char** pd_ops;
  int pd_size;
  int pd_n;
};


void OpnList::add(const char* op)
{
  if( pd_n == pd_size ) {
    int newsize = pd_size * 3 / 2 + 1;
    char** newops = new char*[newsize];
    for( int i = 0; i < pd_n; i++ )  newops[i] = pd_ops[i];
    pd_size = newsize;
    delete[] pd_ops;
    pd_ops = newops;
  }
  pd_ops[pd_n] = new char[strlen(op) + 1];
  strcpy(pd_ops[pd_n++], op);
}


int OpnList::is_member(const char* op)
{
  for( int i = 0; i < pd_n; i++ )
    if( !strcmp(pd_ops[i], op) )
      return 1;
  return 0;
}


static void
internal_produce_tie_call_wrappers(o2be_interface* intf,
				   SimpleStringMap& opl,
				   std::fstream& s)
{
  {
    o2be_iterator<o2be_operation, AST_Decl::NT_op> i(intf);

    while( !i.is_done() ) {
      o2be_operation* op = i.item();
      i.next();
      if( opl[op->uqname()] )  continue;
      opl.insert(op->uqname(), "");
      IND(s);
      if( idl_global->compile_flags() & IDL_BE_OLD_SKEL_SIGNATURES )
	op->produce_decl(s, o2be_global::root());
      else
	op->produce_client_decl(s, o2be_global::root(), 0, 1);
      s << " { ";
      if( !op->return_is_void() )  s << "return ";
      s << "pd_obj->";
      op->produce_invoke(s);
      s << "; }\n";
    }
  }
  {
    o2be_iterator<o2be_attribute, AST_Decl::NT_attr> i(intf);

    while( !i.is_done() ) {
      o2be_attribute* a = i.item();
      i.next();
      if( opl[a->uqname()] )  continue;
      opl.insert(a->uqname(), "");
      IND(s);  a->produce_decl_rd(s, o2be_global::root());
      s << ' ' << a->uqname() << "()";
      s << " { return pd_obj->" << a->uqname() << "(); }\n";
      if( !a->readonly() ) {
	IND(s); s << "void " << a->uqname() << '(';
	a->produce_decl_wr(s, o2be_global::root());
	s << " _value) { pd_obj->" << a->uqname() << "(_value); }\n";
      }
    }
  }
  {
    int ni = intf->n_inherits();
    AST_Interface** intftable = intf->inherits();

    for( int j = 0; j < ni; j++ ) {
      o2be_interface* intfc = o2be_interface::narrow_from_decl(intftable[j]);
      internal_produce_tie_call_wrappers(intfc, opl, s);
    }
  }
}


static void
internal_produce_tie_templates(std::fstream& s, o2be_interface* intf,
			       const char* class_name, const char* skel_name)
{
  SimpleStringMap map;
  map.insert("tie", class_name);
  map.insert("skel_class", skel_name);

  s << o2be_template(map,
    "template <class T>\n"
    "class tie : public virtual skel_class\n"
    "{\n"
    "public:\n"
    "  tie(T& t)\n"
    "    : pd_obj(&t), pd_poa(0), pd_rel(0) {}\n"
    "  tie(T& t, PortableServer::POA_ptr p)\n"
    "    : pd_obj(&t), pd_poa(p), pd_rel(0) {}\n"
    "  tie(T* t, CORBA::Boolean r=1)\n"
    "    : pd_obj(t), pd_poa(0), pd_rel(r) {}\n"
    "  tie(T* t, PortableServer::POA_ptr p,CORBA::Boolean r=1)\n"
    "    : pd_obj(t), pd_poa(p), pd_rel(r) {}\n"
    "  ~tie() {\n"
    "    if( pd_poa )  CORBA::release(pd_poa);\n"
    "    if( pd_rel )  delete pd_obj;\n"
    "  }\n\n"

    "  T* _tied_object() { return pd_obj; }\n\n"

    "  void _tied_object(T& t) {\n"
    "    if( pd_rel )  delete pd_obj;\n"
    "    pd_obj = &t;\n"
    "    pd_rel = 0;\n"
    "  }\n\n"

    "  void _tied_object(T* t, CORBA::Boolean r=1) {\n"
    "    if( pd_rel )  delete pd_obj;\n"
    "    pd_obj = t;\n"
    "    pd_rel = r;\n"
    "  }\n\n"

    "  CORBA::Boolean _is_owner()        { return pd_rel; }\n"
    "  void _is_owner(CORBA::Boolean io) { pd_rel = io;   }\n\n"

    "  PortableServer::POA_ptr _default_POA() {\n"
    "    if( !pd_poa )  return PortableServer::POA::_the_root_poa();\n"
    "    else           return PortableServer::POA::_duplicate(pd_poa);\n"
    "  }\n\n"
  );

  INC_INDENT_LEVEL();
  SimpleStringMap opl;
  internal_produce_tie_call_wrappers(intf, opl, s);
  DEC_INDENT_LEVEL();

  s << o2be_verbatim(
    "\n"
    "private:\n"
    "  T*                      pd_obj;\n"
    "  PortableServer::POA_ptr pd_poa;\n"
    "  CORBA::Boolean          pd_rel;\n"
    "};\n\n\n"
   );
}

//////////////////////////////////////////////////////////////////////
//////////////////// o2be_interface::base_iterator ///////////////////
//////////////////////////////////////////////////////////////////////

o2be_interface::base_iterator::base_iterator(o2be_interface* intf, int rec)
{
  pd_size = intf->n_inherits();
  if( pd_size < 5 )  pd_size = 5;
  pd_n = 0;
  pd_intfs = new o2be_interface*[pd_size];

  {
    pd_n = intf->n_inherits();
    AST_Interface** intftbl = intf->inherits();
    for( int i = 0; i < pd_n; i++ )
      pd_intfs[i] = o2be_interface::narrow_from_decl(intftbl[i]);
  }

  if( rec ) {
    // Keep adding bases of those in pd_intfs, until nothing changes.
    int prev_n = 0;

    while( prev_n != pd_n ) {
      int curr_n = prev_n;
      prev_n = pd_n;

      for( ; curr_n < prev_n; curr_n++ ) {
	AST_Interface** inh = pd_intfs[curr_n]->inherits();
	int n_inh = pd_intfs[curr_n]->n_inherits();

	for( int i = 0; i < n_inh; i++ )  add(inh[i]);
      }
    }
  }

  pd_i = 0;
}


o2be_interface::base_iterator::~base_iterator()
{
  delete[] pd_intfs;
}


void
o2be_interface::base_iterator::add(AST_Interface* aintf)
{
  if( pd_n == pd_size ) {
    int newsize = pd_size * 2;
    o2be_interface** newintfs = new o2be_interface*[newsize];
    for( int i = 0; i < pd_n; i++ )  newintfs[i] = pd_intfs[i];
    delete[] pd_intfs;
    pd_intfs = newintfs;
    pd_size = newsize;
  }

  o2be_interface* intf = o2be_interface::narrow_from_decl(aintf);

  for( int i = 0; i < pd_n; i++ )
    if( pd_intfs[i] == intf )  return;

  pd_intfs[pd_n++] = intf;
}
