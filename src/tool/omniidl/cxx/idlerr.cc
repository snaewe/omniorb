// -*- c++ -*-

//                          Package   : omniidl
// idlerr.cc                Created on: 1999/10/11
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
// Description:
//   
//   IDL compiler error handling

// $Id$
// $Log$
// Revision 1.5  2000/07/04 15:22:44  dpg1
// Merge from omni3_develop.
//
// Revision 1.3.2.1  2000/03/06 15:03:48  dpg1
// Minor bug fixes to omniidl. New -nf and -k flags.
//
// Revision 1.3  1999/11/02 17:07:27  dpg1
// Changes to compile on Solaris.
//
// Revision 1.2  1999/10/29 15:43:44  dpg1
// Error counts now reset when Report...() is called.
//
// Revision 1.1  1999/10/27 14:05:58  dpg1
// *** empty log message ***
//

#include <idlerr.h>
#include <idlutil.h>
#include <idlconfig.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

int errorCount    = 0;
int warningCount  = 0;

void
IdlError(const char* file, int line, const char* fmt ...)
{
  errorCount++;

  if (!Config::quiet) {
    fprintf(stderr, "%s:%d: ", file, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
  }
}

void
IdlErrorCont(const char* file, int line, const char* fmt ...)
{
  if (!Config::quiet) {
    fprintf(stderr, "%s:%d:  ", file, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
  }
}

void
IdlSyntaxError(const char* file, int line, const char* mesg)
{
  static char* lastFile = idl_strdup("");
  static int   lastLine = 0;
  static char* lastMesg = idl_strdup("");

  if (line != lastLine || strcmp(file, lastFile) || strcmp(mesg, lastMesg)) {
    lastLine = line;
    if (strcmp(file, lastFile)) {
      delete [] lastFile;
      lastFile = idl_strdup(file);
    }
    if (strcmp(mesg, lastMesg)) {
      delete [] lastMesg;
      lastMesg = idl_strdup(mesg);
    }
    IdlError(file, line, mesg);
  }
}

void IdlWarning(const char* file, int line, const char* fmt ...)
{
  warningCount++;

  if (!Config::quiet) {
    fprintf(stderr, "%s:%d: Warning: ", file, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
  }
}

void IdlWarningCont(const char* file, int line, const char* fmt ...)
{
  if (!Config::quiet) {
    fprintf(stderr, "%s:%d: Warning:  ", file, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
  }
}

_CORBA_Boolean
IdlReportErrors()
{
  if (!Config::quiet) {
    if (errorCount > 0 || warningCount > 0)
      fprintf(stderr, "omniidl: ");

    if (errorCount > 0)
      fprintf(stderr, "%d error%s", errorCount, errorCount == 1 ? "" : "s");

    if (errorCount > 0 && warningCount > 0)
      fprintf(stderr, " and ");

    if (warningCount > 0)
      fprintf(stderr, "%d warning%s", warningCount,
	      warningCount == 1 ? "" : "s");

    if (errorCount > 0 || warningCount > 0)
      fprintf(stderr, ".\n");
  }

  _CORBA_Boolean ret = (errorCount == 0);
  errorCount         = 0;
  warningCount       = 0;
  return ret;
}
