// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_root.cc             Created on: 06/08/1996
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
//

/*
  $Log$
  Revision 1.4  1997/09/20 16:48:34  dpg1
  Added new #include generation for LifeCycle support.

// Revision 1.3  1997/05/06  14:04:50  sll
// *** empty log message ***
//
  */

#include "idl.hh"
#include "idl_extern.hh"
#include "o2be.h"

#include <iostream.h>

#ifdef __NT__
#include <stdio.h>
#else
#include <unistd.h>
#endif

o2be_root::o2be_root(UTL_ScopedName *n, UTL_StrList *p)
  : AST_Root(n,p),
    AST_Module(n,p),
    AST_Decl(AST_Decl::NT_module,n,p),
    UTL_Scope(AST_Decl::NT_module),
    o2be_module(n,p),
    o2be_name(this)
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

    stubfname = new char[baselen + o2be_global::suffixlen() + 1];
    strcpy(stubfname,basename);

    // open header 
    strcat(stubfname,o2be_global::hdrsuffix());
    pd_hdr.open(stubfname,ios::out|ios::trunc);
    if (! pd_hdr)
      {
	throw o2be_fileio_error("Can't open output header file");
      }
    stubfname[baselen] = '\0';

    // open server
    strcat(stubfname,o2be_global::skelsuffix());
    pd_skel.open(stubfname,ios::out|ios::trunc);
    if (! pd_skel)
      {
        throw o2be_fileio_error("Can't open output stub file");
      }
    stubfname[baselen] = '\0';

    produce_hdr(pd_hdr);
    produce_skel(pd_skel);
    pd_hdr.close();
    pd_skel.close();
    return;
  }
#if 0
  catch(...) {
#else
  catch(o2be_fileio_error &ex) {
#endif

#ifdef __NT__
	if (pd_hdr.is_open())
#else
    if (pd_hdr)
#endif
      {
	pd_hdr.close();
	stubfname[baselen] = '\0';
	strcat(stubfname,o2be_global::hdrsuffix());

#ifdef __NT__
	_unlink(stubfname);
#else
	unlink(stubfname);
#endif
	  }

#ifdef __NT__
	if (pd_skel.is_open())
#else
    if (pd_skel)
#endif
      {
	pd_skel.close();
	stubfname[baselen] = '\0';
	strcat(stubfname,o2be_global::skelsuffix());
#ifdef __NT__
	_unlink(stubfname);
#else
	unlink(stubfname);
#endif
      }
    throw;
  }
}


void
o2be_root::produce_hdr(fstream &hdr)
{
  hdr << "#ifndef __" << basename << "_hh__\n"
      << "#define __" << basename << "_hh__\n\n"
      << "#include <omniORB2/CORBA.h>\n\n";

  if (idl_global->compile_flags() & IDL_CF_LIFECYCLE) {
    hdr << "#include <omniORB2/omniLifeCycle.h>\n\n";
  }

  {
    // produce #include for all the included files
    String **filelist = idl_global->include_file_names();
    int nfiles = idl_global->n_include_file_names();
    int j;

    for (j=0; j<nfiles; j++)
      {
	char *bname = filelist[j]->get_string();
	char *ep    = strchr(bname,'.');
	char *filename;
	int blen = ((ep == NULL)? strlen(bname) : (ep-bname));
	filename = new char[blen+1+o2be_global::suffixlen()];
	strncpy(filename,bname,blen);
	filename[blen] = '\0';
	strcat(filename,o2be_global::hdrsuffix());
	hdr << "#include <" << filename << ">\n";
	delete [] filename;
      }
  }


  if (idl_global->indent() == NULL)
    idl_global->set_indent(new UTL_Indenter());

  o2be_sequence::produce_hdr_for_predefined_types(hdr);

  o2be_module::produce_hdr(hdr);

  hdr << "#endif // __" << basename << "_hh__" << endl;
  return;
}

void
o2be_root::produce_skel(fstream &skel)
{
  skel << "#include \""<<basename<<o2be_global::hdrsuffix()<<"\"\n\n" << endl;

  if (idl_global->indent() == NULL)
    idl_global->set_indent(new UTL_Indenter());

  o2be_module::produce_skel(skel);
  return;
}

AST_Sequence *
o2be_root::add_sequence(AST_Sequence *se)
{
  if (AST_Root::add_sequence(se) == NULL)
    return NULL;
  return o2be_sequence::attach_seq_to_base_type(se);
}

IMPL_NARROW_METHODS1(o2be_root, AST_Root)
IMPL_NARROW_FROM_DECL(o2be_root)
IMPL_NARROW_FROM_SCOPE(o2be_root)
