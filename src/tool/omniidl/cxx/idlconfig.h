// -*- c++ -*-
//                          Package   : omniidl
// idlconfig.h              Created on: 2000/03/06
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//   Global configuration for omniidl

// $Id$
// $Log$
// Revision 1.4  2001/02/21 14:12:08  dpg1
// Merge from omni3_develop for 3.0.3 release.
//
// Revision 1.1.2.4  2000/11/30 11:40:47  dpg1
// Add -nc option to omniidl to accept invalid IDL with identifiers
// differing only by case.
//
// Revision 1.1.2.3  2000/10/24 09:53:29  dpg1
// Clean up omniidl system dependencies. Replace use of _CORBA_ types
// with IDL_ types.
//
// Revision 1.1.2.2  2000/06/05 18:13:27  dpg1
// Comments can be attached to subsequent declarations (with -K). Better
// idea of most recent decl in operation declarations
//
// Revision 1.1.2.1  2000/03/06 15:03:48  dpg1
// Minor bug fixes to omniidl. New -nf and -k flags.
//

#ifndef _idlconfig_h_
#define _idlconfig_h_

#include <idlutil.h>


class Config {
public:
  static IDL_Boolean quiet;           // Don't make any output
  static IDL_Boolean forwardWarning;  // Warn about unresolved forwards
  static IDL_Boolean keepComments;    // Keep comments from source
  static IDL_Boolean commentsFirst;   // Comments come before declarations
  static IDL_Boolean caseSensitive;   // Do not treat identifiers differing
                                      //  only in case as errors
};


#endif // _idlconfig_h_
