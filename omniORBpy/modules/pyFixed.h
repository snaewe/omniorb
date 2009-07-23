// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyFixed.h                  Created on: 2001/03/30
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2005 Apasphere Ltd
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//
//    This file is part of the omniORBpy library
//
//    The omniORBpy library is free software; you can redistribute it
//    and/or modify it under the terms of the GNU Lesser General
//    Public License as published by the Free Software Foundation;
//    either version 2.1 of the License, or (at your option) any later
//    version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
//    MA 02111-1307, USA
//
//
// Description:
//    Implementation of Fixed type for Python

// $Id$
// $Log$
// Revision 1.1.4.2  2005/01/07 00:22:32  dgrisby
// Big merge from omnipy2_develop.
//
// Revision 1.1.4.1  2003/03/23 21:51:57  dgrisby
// New omnipy3_develop branch.
//
// Revision 1.1.2.1  2001/04/09 15:22:16  dpg1
// Fixed point support.
//

#if defined(__cygwin__) && defined(DL_IMPORT)
# undef DL_IMPORT
# define DL_IMPORT(RTYPE) RTYPE
#endif

extern "C" {

  struct omnipyFixedObject {
    PyObject_HEAD
    CORBA::Fixed* ob_fixed;
  };

  extern DL_IMPORT(PyTypeObject) omnipyFixed_Type;

#define omnipyFixed_Check(op) ((op)->ob_type == &omnipyFixed_Type)

}
