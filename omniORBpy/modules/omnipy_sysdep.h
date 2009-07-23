// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// omnipy30_sysdep.h          Created on: 2000/03/07
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//    Additional system dependencies

#ifndef _omnipy_sysdep_h_
#define _omnipy_sysdep_h_

// Defaults for things we'd like to do

#define PY_OMNISERVANT_BASE omniPy::Py_omniServant


// Things that are broken

#if defined(_MSC_VER)
#  undef HAS_Cplusplus_catch_exception_by_base

#  undef  PY_OMNISERVANT_BASE
#  define PY_OMNISERVANT_BASE Py_omniServant

#endif

#endif // _omnipy_sysdep_h_
