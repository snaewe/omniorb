// -*- c++ -*-

//                          Package   : omniidl
// idlerr.h                 Created on: 1999/10/11
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
// Revision 1.1  1999/10/27 14:05:57  dpg1
// *** empty log message ***
//

#ifndef _idlerr_h_
#define _idlerr_h_

#include <idlutil.h>

extern int errorCount;
extern int warningCount;

// Error report and continuation
void IdlError(const char* file, int line, const char* fmt ...);
void IdlErrorCont(const char* file, int line, const char* fmt ...);

void IdlSyntaxError(const char* file, int line, const char* mesg);

// Warning report and continuation
void IdlWarning(const char* file, int line, const char* fmt ...);
void IdlWarningCont(const char* file, int line, const char* fmt ...);

_CORBA_Boolean IdlReportErrors();


#endif // _idlerr_h_
