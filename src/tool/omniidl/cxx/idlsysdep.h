// -*- c++ -*-
//                          Package   : omniidl
// idlsysdep.h              Created on: 1999/11/04
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
//   System dependencies

// $Id$
// $Log$
// Revision 1.2  1999/11/09 12:41:40  dpg1
// strcasecmp changes for NT, AIX.
//
// Revision 1.1  1999/11/05 09:43:01  dpg1
// Changes for NT.
//

#ifndef _idlsysdep_h_
#define _idlsysdep_h_

// Types
#include <CORBA_sysdep.h>    // ***
#include <CORBA_basetypes.h> // ***


// As usual, Windows manages to be different...
#ifdef __WIN32__
#define DLL_EXPORT _declspec(dllexport)
#define strcasecmp(X,Y) stricmp(X,Y)
#else
#define DLL_EXPORT
#include <strings.h>
#endif

#endif // _idlsysdep_h_
