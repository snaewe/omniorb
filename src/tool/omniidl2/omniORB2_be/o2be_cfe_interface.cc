// -*- Mode: C++; -*-
//                          Package   : omniidl2
// obe_cfe_interface.cc     Created on: 8/8/1996
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
//    API entry points to interface with CFE

/*
  $Log$
  Revision 1.9  1998/01/20 19:13:38  sll
  Added support for OpenVMS.

  Revision 1.8  1997/12/09 19:55:22  sll
  *** empty log message ***

// Revision 1.7  1997/09/20  16:37:24  dpg1
// Added new -l flag for LifeCycle code generation.
//
// Revision 1.6  1997/05/07  10:12:52  ewc
// Changed win32 usage() message.
//
// Revision 1.5  1997/05/06  17:28:38  sll
// Public release.
//
  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef __WIN32__
#include <unistd.h>
#endif

#include <idl.hh>
#include <idl_extern.hh>
#include <drv_private.hh>
#include <o2be.h>


#if defined(__WIN32__) || defined(__VMS) && __VMS_VER < 60200000

// NT doesn't have an implementation of getopt() - supply a getopt() for this program:

char* optarg;
int optind = 1;


int
getopt(int num_args, char* const* args, const char* optstring)
{
if (optind == num_args) return EOF;
char* buf_left = *(args+optind);

if ((*buf_left != '-' && *buf_left != '/') || buf_left == NULL ) return EOF;
else if ((optind < (num_args-1)) && strcmp(buf_left,"-") == 0 && strcmp(*(args+optind+1),"-") == 0)
	{
		optind+=2;
		return EOF;
	}
else if (strcmp(buf_left,"-") == 0)
	{
		optind++;
		return '?';
	}

for(int count = 0; count < strlen(optstring); count++)
	{
	if (optstring[count] == ':') continue;
	if (buf_left[1] == optstring[count])
		{
		 if(optstring[count+1] == ':')
		  {
			if (strlen(buf_left) > 2)
			 {
				optarg = (buf_left+2);
				optind++;
			 }
			else if (optind < (num_args-1))
			 {
				optarg = *(args+optind+1);
				optind+=2;
			 }
			else
			 {
				optind++;
				return '?';
			 }
		  }
		  else optind++;

		  return buf_left[1];
		}
	}
optind++;
return '?';
}

#endif


o2be_root *o2be_global::myself = NULL;
char *o2be_global::pd_hdrsuffix = DEFAULT_IDL_HDR_SUFFIX;
char *o2be_global::pd_skelsuffix = DEFAULT_IDL_SKEL_SUFFIX;
size_t o2be_global::pd_suffixlen = DEFAULT_IDL_SUFFIXLEN;
int o2be_global::pd_fflag = 0;
int o2be_global::pd_aflag = 0;
int o2be_global::pd_qflag = 0;
int o2be_global::pd_mflag = 1;

//
// Initialize the BE. The protocol requires only that this routine
// return an instance of AST_Generator (or a subclass thereof).
//
// Remember that none of the FE initialization has been done, when you
// add stuff here.
//
AST_Generator *
BE_init()
{
  AST_Generator	*g = new o2be_generator();	
  return g;
}

//
// Print out a version string for the BE
//
void
BE_version()
{
}

//
// Do the work of this BE.
//
void
BE_produce()
{
  try {
    o2be_global::root()->produce();
  }
  catch (o2be_fileio_error &ex) {
    cerr << "Error: " << ex.errmsg() << endl;
    idl_global->err_count();
  }
  catch (o2be_unsupported &ex) {
    cerr << "Error: " << ex.file() << "-" << ex.line()
         << " unsupported IDL syntax. " << ex.msg() << endl;
    idl_global->err_count();
  }
#if 0
  catch (o2be_internal_error &ex) {
    cerr << "omniORB2 back end internal error: " 
	 << ex.file() << ":" << ex.line() << "-" << ex.errmsg() << endl;
    idl_global->err_count();
  };
#endif
  return;
}

//
// Abort this run of the BE
//
void
BE_abort()
{
  return;
}

static
void
usage()
{
  cerr << GTDEVEL("usage: ")
       << idl_global->prog_name()
#ifdef __WIN32__
       << GTDEVEL(" [flag]* file\n");
#else
       << GTDEVEL(" [flag]* file [file]*\n");
#endif
  cerr << GTDEVEL("Legal flags:\n");

  cerr << GTDEVEL(" -Dname[=value]\t\tdefines name for preprocessor\n");
  cerr << GTDEVEL(" -E\t\t\truns preprocessor only, prints on stdout\n");
  cerr << GTDEVEL(" -Idir\t\t\tincludes dir in search path for preprocessor\n");
  cerr << GTDEVEL(" -Uname\t\t\tundefines name for preprocessor\n");
  cerr << GTDEVEL(" -V\t\t\tprints version info then exits\n");
  cerr << GTDEVEL(" -h suffix   Specify non-default suffix for the generated header file(s)\n");
  cerr << GTDEVEL(" -s suffix   Specify non-default suffix for the generated stub file(s)\n");
  cerr << GTDEVEL(" -u\t\t\tprints usage message and exits\n");

  cerr << GTDEVEL(" -v\t\t\ttraces compilation stages\n");
  cerr << GTDEVEL(" -w\t\t\tsuppresses IDL compiler warning messages\n");
  cerr << GTDEVEL(" -l\t\t\tgenerates code required by LifeCycle service\n");
  return;
}

void
BE_prep_arg(char *arg, idl_bool unknown_flag)
{
  return;
}

void
BE_parse_args(int argc, char **argv)
{
  int c;
  char *buffer;

#ifndef __WIN32__
  extern char *optarg;
  extern int optind;
#else	
 o2be_global::set_skelsuffix("SK.cpp");
#endif

  DRV_cpp_init();
  idl_global->set_prog_name(argv[0]);
  while ((c = getopt(argc,argv,"D:EI:U:Vuvwh:s:l")) != EOF)
    {
      switch (c) 
	{
	case 'D':
	case 'I':
	case 'U':
	  buffer = new char[strlen(optarg) + 3];
	  sprintf(buffer, "-%c%s", c, optarg);
	  DRV_cpp_putarg(buffer);
	  break;
	case 'E':
	  idl_global->set_compile_flags(idl_global->compile_flags() |
					IDL_CF_ONLY_PREPROC);
	  break;
	case 'V':
	  idl_global->set_compile_flags(idl_global->compile_flags() |
					IDL_CF_VERSION);
	  return;
	case 'h':
	  o2be_global::set_hdrsuffix(optarg);
	  break;
	case 's':
	  o2be_global::set_skelsuffix(optarg);
	  break;
	case 'u':
	  usage();
	  idl_global->set_compile_flags(idl_global->compile_flags() |
					IDL_CF_ONLY_USAGE);
	  return;
	case 'v':
	  idl_global->set_compile_flags(idl_global->compile_flags() |
					IDL_CF_INFORMATIVE);
	  break;
	case 'w':
	  idl_global->set_compile_flags(idl_global->compile_flags() |
					IDL_CF_NOWARNINGS);
	  break;
	case 'l':
	  // XXX -Life cycle compiler flag
	  idl_global->set_compile_flags(idl_global->compile_flags() |
					IDL_CF_LIFECYCLE);
	  break;
	case '?':
	  usage();
	  idl_global->set_compile_flags(idl_global->compile_flags() |
					IDL_CF_ONLY_USAGE);
	  return;
	}
    }
  for (; optind < argc; optind++)
    {
      DRV_files[DRV_nfiles++] = argv[optind];
    }


  if (DRV_nfiles == 0)
    {
      cerr << "No file specified.\n";
      usage();
      idl_global->set_compile_flags(idl_global->compile_flags() |
				    IDL_CF_ONLY_USAGE);
    }
  return;
}
