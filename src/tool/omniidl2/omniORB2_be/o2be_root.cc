// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_root.cc             Created on: 06/08/1996
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
//

/*
  $Log$
  Revision 1.20  1999/06/02 16:43:53  sll
  Added support for -F flag.

  Revision 1.19  1999/04/12 09:07:04  djr
  Updated version.

  Revision 1.18  1999/03/11 16:26:09  djr
  Updated copyright notice

  Revision 1.17  1999/01/07 09:37:19  djr
  Separate stub skeleton into two files: fooSK.cc and fooDynSK.cc.
  The latter contains code for TypeCode and Any.

  Revision 1.16  1998/10/26 12:12:15  sll
  Added check for frontend error flagged by the backend.

  Revision 1.15  1998/08/19 15:55:12  sll
  Added a separate pass to generate binary operators <<= and the like.

  Revision 1.14  1998/08/13 22:39:34  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.13  1998/08/10 15:33:07  sll
  Now catch all errors instead of letting o2be_fatal_exception to cause
  core dump.

  Revision 1.12  1998/08/05 18:05:43  sll
  Version variable is changed to omniORB_2_6 (previously omniORB_2_5).

  Revision 1.11  1998/05/20 18:24:06  sll
  New option (-t) enable the generation of tie implementation template.

  Revision 1.10  1998/04/07 18:50:29  sll
  Use std::fstream instead of fstream.
  Stub code modified to accommodate the use of namespace to represent module.

// Revision 1.9  1998/01/27  16:52:59  ewc
// Incremented version to 2.5
//
// Revision 1.8  1998/01/20  19:13:46  sll
// Added support for OpenVMS.
//
  Revision 1.7  1997/12/12 20:02:33  sll
  Generate reference to version variable omniORB_x_y in skel file.

  Revision 1.6  1997/12/10 11:35:56  sll
  Updated life cycle service stub.

  Revision 1.5  1997/12/09 19:55:31  sll
  *** empty log message ***

// Revision 1.4  1997/09/20  16:48:34  dpg1
// Added new #include generation for LifeCycle support.
//
// Revision 1.3  1997/05/06  14:04:50  sll
// *** empty log message ***
//
  */

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#ifdef __WIN32__
# include <stdio.h>
#else
# include <unistd.h>
# if defined(__VMS) && __VMS_VER < 70000000
#  include <omniVms/unlink.hxx>
# endif
#endif
#include <ctype.h>

const char* defs_fragment_suffix      = "_defs";
const char* operators_fragment_suffix = "_operators";


// Do not forget to update the version number in omniORB_x_y below.
// The variable is defined in omniInternal.h.
#define OMNIORB_LIBRARY_VERSION    "omniORB_2_8"


o2be_root::o2be_root(UTL_ScopedName *n, UTL_StrList *p)
  : AST_Root(n,p),
    AST_Module(n,p),
    AST_Decl(AST_Decl::NT_module,n,p),
    UTL_Scope(AST_Decl::NT_module),
    o2be_module(n,p),
    o2be_name(AST_Decl::NT_module,n,p)
{
  o2be_global::set_root(this);
  set_in_main_file(I_TRUE);
  return;
}


void
o2be_root::produce()
{
  char *stubfname  = 0;

  try {
    char *bname      = idl_global->stripped_filename()->get_string();
    char *ep         = strchr(bname,'.');

    baselen    = (ep != NULL) ? ep - bname : strlen(bname);
    basename   = new char[baselen + 1];
    strncpy(basename,bname,baselen);
    basename[baselen] = '\0';

    stubfname = new char[baselen + o2be_global::suffixlen() + 64 + 1];
    strcpy(stubfname,basename);

    if (!(idl_global->compile_flags() & IDL_BE_GENERATE_FRAGMENT)) {
      // open header 
      strcat(stubfname,o2be_global::hdrsuffix());
      hdr.open(stubfname, std::ios::out | std::ios::trunc);
      if (! hdr)
	throw o2be_fileio_error("Can't open output header file");
      stubfname[baselen] = '\0';
    }
    else {
      // open header files
      strcat(stubfname,defs_fragment_suffix);
      strcat(stubfname,o2be_global::hdrsuffix());
      hdr_defs.open(stubfname, std::ios::out | std::ios::trunc);
      if (! hdr_defs)
	throw o2be_fileio_error("Can't open output header file");
      stubfname[baselen] = '\0';

      strcat(stubfname,operators_fragment_suffix);
      strcat(stubfname,o2be_global::hdrsuffix());
      hdr_opers.open(stubfname, std::ios::out | std::ios::trunc);
      if (! hdr_opers)
	throw o2be_fileio_error("Can't open output header file");
      stubfname[baselen] = '\0';

    }

    // open skeleton
    strcat(stubfname,o2be_global::skelsuffix());
    skel.open(stubfname, std::ios::out | std::ios::trunc);
    if (! skel)
      throw o2be_fileio_error("Can't open output stub file");
    stubfname[baselen] = '\0';

    if (idl_global->compile_flags() & IDL_CF_ANY) {
      // open dynamic skeleton
      strcat(stubfname, o2be_global::dynskelsuffix());
      dynskel.open(stubfname, std::ios::out | std::ios::trunc);
      if( !dynskel )
	throw o2be_fileio_error("Can't open output dynamic stub file");
      stubfname[baselen] = '\0';
    }

    if (!(idl_global->compile_flags() & IDL_BE_GENERATE_FRAGMENT)) {
      produce_hdr(hdr);
      if (idl_global->err_count() > 0)
	throw o2be_fe_error("Error detected when generating the header file.");
      hdr.close();
    }
    else {
      hdr_defs << "// This file is generated by omniidl2- "
	       << (const char*) OMNIORB_LIBRARY_VERSION << ". Do not edit.\n";
      hdr_defs << "#ifndef __" << basename << defs_fragment_suffix << "_hh__\n"
	       << "#define __" << basename << defs_fragment_suffix << "_hh__\n\n";
      produce_hdr_defs(hdr_defs);
      hdr_defs << "#endif" << std::endl;
      if (idl_global->err_count() > 0)
	throw o2be_fe_error("Error detected when generating the header file.");
      hdr_defs.close();

      hdr_opers << "// This file is generated by omniidl2- "
		<< (const char*) OMNIORB_LIBRARY_VERSION << ". Do not edit.\n";
      hdr_opers << "#ifndef __" << basename << operators_fragment_suffix << "_hh__\n"
		<< "#define __" << basename << operators_fragment_suffix << "_hh__\n\n";
      produce_hdr_operators(hdr_opers);
      hdr_opers << "#endif" << std::endl;
      if (idl_global->err_count() > 0)
	throw o2be_fe_error("Error detected when generating the header file.");
      hdr_opers.close();
    }

    produce_skel(skel);
    if (idl_global->err_count() > 0)
      throw o2be_fe_error("Error detected when generating the skeleton file.");
    skel.close();

    if (idl_global->compile_flags() & IDL_CF_ANY) {
      produce_dynskel(dynskel);
      if( idl_global->err_count() > 0 )
	throw o2be_fe_error("Error detected when generating the"
			    " dynamic skeleton file.");
      dynskel.close();
    }

    return;
  }
  catch(...) {

#ifdef __WIN32__
    if (hdr.is_open())
#else
    if (hdr)
#endif
    {
	hdr.close();
	stubfname[baselen] = '\0';
	strcat(stubfname,o2be_global::hdrsuffix());

#ifdef __WIN32__
	_unlink(stubfname);
#else
	unlink(stubfname);
#endif
     }

#ifdef __WIN32__
    if (hdr_defs.is_open())
#else
    if (hdr_defs)
#endif
    {
	hdr_defs.close();
	stubfname[baselen] = '\0';
	strcat(stubfname,defs_fragment_suffix);
	strcat(stubfname,o2be_global::hdrsuffix());

#ifdef __WIN32__
	_unlink(stubfname);
#else
	unlink(stubfname);
#endif
     }

#ifdef __WIN32__
    if (hdr_opers.is_open())
#else
    if (hdr_opers)
#endif
    {
	hdr_opers.close();
	stubfname[baselen] = '\0';
	strcat(stubfname,operators_fragment_suffix);
	strcat(stubfname,o2be_global::hdrsuffix());

#ifdef __WIN32__
	_unlink(stubfname);
#else
	unlink(stubfname);
#endif
     }

#ifdef __WIN32__
    if (skel.is_open())
#else
    if (skel)
#endif
      {
	skel.close();
	stubfname[baselen] = '\0';
	strcat(stubfname,o2be_global::skelsuffix());
#ifdef __WIN32__
	_unlink(stubfname);
#else
	unlink(stubfname);
#endif
      }

#ifdef __WIN32__
    if (dynskel.is_open())
#else
    if (dynskel)
#endif
      {
	dynskel.close();
	stubfname[baselen] = '\0';
	strcat(stubfname,o2be_global::skelsuffix());
#ifdef __WIN32__
	_unlink(stubfname);
#else
	unlink(stubfname);
#endif
      }

    throw;
  }
}

void
o2be_root::produce_hdr(std::fstream &s)
{
  s << "// This file is generated by omniidl2- "
    << (const char*) OMNIORB_LIBRARY_VERSION << ". Do not edit.\n";
  s << "#ifndef __" << basename << "_hh__\n"
      << "#define __" << basename << "_hh__\n\n"
      << "#ifndef USE_omniORB_logStream\n"
      << "#define USE_omniORB_logStream\n"
      << "#endif\n\n"
      << "#ifndef __CORBA_H_EXTERNAL_GUARD__\n"
      << "#define __CORBA_H_EXTERNAL_GUARD__\n"
      << "#include <omniORB2/CORBA.h>\n"
      << "#endif\n\n";

  if( idl_global->compile_flags() & IDL_CF_LIFECYCLE ) {
    s << "#ifndef __OMNILC_H_EXTERNAL_GUARD__\n"
	<< "#define __OMNILC_H_EXTERNAL_GUARD__\n"
	<< "#include <omniORB2/omniLC.h>\n"
	<< "#endif\n\n";
  }

  {
    // produce #include for all the included files
    String** filelist = idl_global->include_file_names();
    int nfiles = idl_global->n_include_file_names();
    int j;

    for( j = 0; j < nfiles; j++ ) {
      char* bname = filelist[j]->get_string();
      char* ep = strrchr(bname, '.');
      size_t blen = ep ? (ep - bname) : strlen(bname);
      char* filename = new char[blen + 1 + o2be_global::suffixlen()];
      strncpy(filename, bname, blen);
      filename[blen] = '\0';
      strcat(filename, o2be_global::hdrsuffix());

      bname = filename;
      char* guardname = new char[strlen(bname) + 1];
      char* d = guardname;
      while( *bname ) {
	if( isalnum(*bname) )  *d = *bname;
	else                   *d = '_';
	bname++;  d++;
      }
      *d = '\0';

      s << "#ifndef __" << guardname << "_EXTERNAL_GUARD__\n"
	  << "#define __" << guardname << "_EXTERNAL_GUARD__\n";
      s << "#include <" << filename << ">\n"
	  << "#endif\n\n";

      delete[] guardname;
      delete[] filename;
    }
  }

  s << "#ifdef _LC_attr\n"
      << "# error \"A local CPP macro _LC_attr has already been defined.\"\n"
      << "#else\n"
      << "# ifdef  USE_stub_in_nt_dll\n"
      << "#  define _LC_attr _OMNIORB_NTDLL_IMPORT\n"
      << "# else\n"
      << "#  define _LC_attr\n"
      << "# endif\n"
      << "#endif\n\n";

  if (idl_global->indent() == NULL)
    idl_global->set_indent(new UTL_Indenter());

  produce_hdr_defs(s);

  s << "\n#undef _LC_attr\n\n";

  produce_hdr_operators(s);

  s << "#endif // __" << basename << "_hh__" << std::endl;
}

void
o2be_root::produce_hdr_defs(std::fstream &s)
{
  o2be_sequence::produce_hdr_for_predefined_types(s);

  o2be_module::produce_decls_at_global_scope_in_hdr(s);

  o2be_module::produce_hdr(s);

  if (idl_global->compile_flags() & IDL_BE_GENERATE_TIE) {
    o2be_module::produce_tie_templates(s);
  }
}

void
o2be_root::produce_hdr_operators(std::fstream &s)
{
  o2be_module::produce_binary_operators_in_hdr(s);
}

void
o2be_root::produce_skel(std::fstream &s)
{
  s << "// This file is generated by omniidl2- "
    << (const char*) OMNIORB_LIBRARY_VERSION << ". Do not edit.\n";
  if (!(idl_global->compile_flags() & IDL_BE_GENERATE_FRAGMENT)) {

    s << "#include \"" << basename << o2be_global::hdrsuffix() << "\"\n";
    s << "#include <omniORB2/proxyCall.h>\n\n";
  
    s << "static const char* _0RL_library_version = "
	 << OMNIORB_LIBRARY_VERSION << ";\n\n\n";
  }

  if (idl_global->indent() == NULL)
    idl_global->set_indent(new UTL_Indenter());

  o2be_module::produce_skel(s);
}


void
o2be_root::produce_dynskel(std::fstream& s)
{
  s << "// This file is generated by omniidl2- "
    << (const char*) OMNIORB_LIBRARY_VERSION << ". Do not edit.\n";
  if (!(idl_global->compile_flags() & IDL_BE_GENERATE_FRAGMENT)) {
  
    s << "#include \"" << basename << o2be_global::hdrsuffix() << "\"\n";
    s << "#include <omniORB2/tcDescriptor.h>\n\n";

    s << "static const char* _0RL_library_version = "
	 << OMNIORB_LIBRARY_VERSION << ";\n\n\n";

    if( idl_global->indent() == NULL )
      idl_global->set_indent(new UTL_Indenter());
  }

  o2be_module::produce_dynskel(s);
  o2be_module::produce_binary_operators_in_dynskel(s);
}


AST_Sequence*
o2be_root::add_sequence(AST_Sequence *se)
{
  if (AST_Root::add_sequence(se) == NULL)
    return NULL;
  return o2be_sequence::attach_seq_to_base_type(se);
}


IMPL_NARROW_METHODS1(o2be_root, AST_Root)
IMPL_NARROW_FROM_DECL(o2be_root)
IMPL_NARROW_FROM_SCOPE(o2be_root)
