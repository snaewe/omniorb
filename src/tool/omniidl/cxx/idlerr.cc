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
// Revision 1.1  1999/10/27 14:05:58  dpg1
// *** empty log message ***
//

#include <idlerr.h>
#include <idlutil.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int errorCount    = 0;
int warningCount  = 0;

void
IdlError(const char* file, int line, const char* fmt ...)
{
  errorCount++;
  fprintf(stderr, "%s:%d: ", file, line);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

void
IdlErrorCont(const char* file, int line, const char* fmt ...)
{
  fprintf(stderr, "%s:%d:  ", file, line);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

void
IdlSyntaxError(const char* file, int line, const char* mesg)
{
  static const char* lastFile = idl_strdup("");
  static int         lastLine = 0;
  static const char* lastMesg = idl_strdup("");

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
  fprintf(stderr, "%s:%d: Warning: ", file, line);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

void IdlWarningCont(const char* file, int line, const char* fmt ...)
{
  fprintf(stderr, "%s:%d: Warning:  ", file, line);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

_CORBA_Boolean
IdlReportErrors()
{
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
  
  if (errorCount > 0)
    return 0;
  else
    return 1;
}
